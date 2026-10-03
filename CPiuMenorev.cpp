#include "stdafx.h"
#include "Cpiumeno.h"
#include "CpiumenoTrans.h"

#include <stdlib.h>
#include <ctype.h>
#include <math.h>


void CPlusMinus::subEvEx(char *outbuf,uint8_t Pty, int16_t *cond, char *Clabel, struct OPERAND *V) {
	
  /*Brack=*/isRValue=isPtrUsed=inCast=0;
  FNRev(outbuf,Pty,cond,Clabel,V);
	if(Pty>15)		// se livello + esterno, esco 2026
		return;
//	if((V->Q & VALUE_IS_CONDITION) && Pty>=14)		// se condizionale e livello + esterno, esco 2025
//		return;f
  if(V->Q == VALUE_IS_D0) {
		if(!(V->Q & VALUE_IS_CONDITION))		// già a posto qua.. VERIFICARE!
	    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,V->cost->l,FALSE);
		}
  else if(V->Q == VALUE_IS_VARIABILE) {
		if(!(V->Q & VALUE_IS_CONDITION))		// già a posto qua.. VERIFICARE!
	    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,(*cond & VALUE_CONDITION_MASK) ? TRUE : FALSE,FALSE);
		}
  else if(V->Q & VALUE_IS_COSTANTE) {
		if(!(V->Q & VALUE_IS_CONDITION))		// già a posto qua.. VERIFICARE!
	    PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,FALSE);
		}

//	V->Q &= 0xf0;			// lascio solo le condizioni
  }
 
uint16_t CPlusMinus::FNEvalExpr(char *outbuf,uint8_t Pty, char *C) {
	int16_t i;
  struct VARS VPtr;
	struct OPERAND V;
  char Clabel[32];
  
  Brack=isRValue=isPtrUsed=inCast=0;
	ZeroMemory(&V,sizeof(struct OPERAND));
	ZeroMemory(&VPtr,sizeof(struct VARS));
  ZeroMemory(C,sizeof(union STR_LONG));
	*Clabel=0;
  V.var=&VPtr;
	V.cost=(union STR_LONG*)C;
  i=0;
  subEvEx(outbuf,Pty,&i,Clabel,&V);
// V.Q: -5 niente                      	0
//     0 per valore gen. in D0 o hl   	1
//     1 per valore da ptr in D0 o hl 	2 
//     2 variabile in *V              	3
//     -1 per costante integral       	8
//     -2 per altra costante          	9
//     -9 per ! condizionale      (bit 4)
//     -10..-15 per condizionale  (bit 7)
//     -20..-25 per condizionale multipla && ||   (bit 6)
// bit 5 indica comparazione tra signed (0) o unsigned (1)
  
  return V.size;                  // ritorno size espressione
  }
 
int CPlusMinus::FNEvalECast(char *outbuf,char *C, O_TYPE *T, O_SIZE *S) {
  int16_t i;
  struct VARS VPtr;
  struct OPERAND V;
  char Clabel[32];
  
  ZeroMemory(&V,sizeof(struct OPERAND));
	ZeroMemory(&VPtr,sizeof(struct VARS));
  ZeroMemory(C,sizeof(union STR_LONG));
	*Clabel=0;
	V.var=&VPtr;
	V.cost=(union STR_LONG*)C;
  i=0;
  Brack=isRValue=isPtrUsed=inCast=0;
	*outbuf=0;
  FNRev(outbuf,15,&i,Clabel,&V);
  if(*S) {                    // se ho newSize, faccio cast...
		if(V.Q==VALUE_IS_VARIABILE) {
	    ReadVar(outbuf,V.var,*T,*S,0,FALSE);
			}
		else if(V.Q==VALUE_IS_D0) {
	    PROCReadD0(outbuf,V.var,*T,*S,0,0,FALSE);
			}
		else if(V.Q & VALUE_IS_COSTANTE) {
	    PROCUseCost(NULL,V.Q,*T,*S,(union STR_LONG *)C,FALSE);
			}
	  else if(V.Q==VALUE_IS_EXPR || V.Q==VALUE_IS_EXPR_FUNC)
	    PROCCast(*T,*S,&V.type,&V.size,-1);
	  else if(V.Q==VALUE_IS_PTR)
			;
	  }
	else {                      // altrimenti no cast e ritorno i valori T & S
		if(V.Q==VALUE_IS_VARIABILE) {
	    ReadVar(outbuf,V.var,V.type,V.size,0,FALSE);
			}
		else if(V.Q==VALUE_IS_D0) {
	    PROCReadD0(outbuf,V.var,V.type,V.size,0,0,FALSE);
			}
		else if(V.Q & VALUE_IS_COSTANTE) {
	    PROCUseCost(NULL,V.Q,V.type,V.size,(union STR_LONG *)C,FALSE);
			}
	  else if(V.Q==VALUE_IS_PTR)
			;
	  *T=V.type;
	  *S=V.size;
	  }  
  return 0;
  }
 
int CPlusMinus::FNEvalCond(char *outbuf,char *C, const char *TS, uint16_t cond) {
  int8_t i;
	int16_t cond2;
  struct VARS VPtr;
  struct OPERAND V;
  char MyBuf[sizeof(union STR_LONG)];
    
  Brack=isRValue=isPtrUsed=inCast=0;
  ZeroMemory(&V,sizeof(struct OPERAND));
	ZeroMemory(&VPtr,sizeof(struct VARS));
	ZeroMemory(MyBuf,sizeof(union STR_LONG));
	V.var=&VPtr;
	V.cost=(union STR_LONG*)MyBuf;
//  *C=0;                   // gli stmt passano qui la label per && e ||
  cond2=1;
	*outbuf=0;
  subEvEx(outbuf,15,&cond2,C,&V);
//	PROCOper(LINE_TYPE_ISTRUZIONE,outbuf);
// aggiungo qui un Read VAR reso intelligente dal fatto che si ha un IF o un expr normale...    
  if(V.Q & VALUE_IS_COSTANTE) {
    PROCWarn(4127);
// in questo caso bisognerebbe anche stroncarlo...    
    }

	if(debug)
		myLog->print(0,"EvalCond GenCondBranch %s; C=%s, V.Q=%x, cond %x",TS,C,V.Q,cond);

  i=V.Q & ~VALUE_HAS_CONDITION;      // tolgo cond. multipla

	PROCGenCondBranch(TS,cond,&i,FNGetMemSize(V.type,V.size,NULL/*dim*/,0));

	if(V.Q & VALUE_HAS_CONDITION) {
    PROCOutLab(C);
		if(debug)
			myLog->print(0," label (da EvalCond) %s; V.Q=%x",C,V.Q);
    return 1;
	  }
	else
	  return 0;
  }

void CPlusMinus::skipExpr(uint8_t Pty,char delim) {		// usata per ignorare del tutto un'espressione, tipo per condizionali falsi
	uint8_t inBrack=0,inTernary=0;
	char AS[256];
	unsigned long ol,OT;

	do {
	  OT=FIn->GetPosition();
		FIn->SavePosition();
		ol=__line__;
		FNLO(AS);
		switch(*AS) {
			case 0:
				return;
				break;
			case '(':
			case '[':
			case '{':
				inBrack++;
				break;
			case ')':
			case ']':
			case '}':
				if(inBrack > 0) 
					inBrack--;			// safety
				break;
			case '?':
        // Trovato un ternario interno: incrementa il livello
        inTernary++;
        break;
			case ':':
        if(inTernary > 0) {
					inTernary--;
					if(delim == ':') {		// potrebbe andare...
	          AS[0] = 0x01; // Un valore qualsiasi diverso da ':'
						}
					}
        break;
			}
		} while(*AS != delim || inBrack || inTernary);
	FIn->RestorePosition(OT);
	__line__=ol;
	}



 char origLabel[32];

int8_t CPlusMinus::FNRev(char *outbuf,int8_t Pty,int16_t *cond,char *Clabel,struct OPERAND *V) {
  int i,j,I;
	O_TYPE T1;
	uint32_t T;
	int8_t v;
	uint32_t attrib;
  int AR,reg2;
	int8_t OP,oOP,Co=0;
	bool Exit=FALSE;
  int VQ1;
  char Rlabel[/*32*/ sizeof(STR_LONG)];
  char AS[64],*BS,B1S[64],TS[/*32*/ sizeof(STR_LONG)],T1S[64],MyBuf[64],MyBuf1[64];
  char *p1;
	struct VARS RPtr;
	struct OPERAND R;
  union STR_LONG RCost;
  struct VARS *VPtr;
  long OT,l,l1;
	unsigned long ol;
//	int8_t isWhat=0;		// 0 inizio, 1=value, 2=operand; (v. anche EVAL   alla fine forse non serve!
//  int BrackOP[10],BrackPty[10];
 
	*Rlabel=0;
  ZeroMemory(&R,sizeof(struct OPERAND));
  ZeroMemory(&RPtr,sizeof(struct VARS));
  ZeroMemory(&RCost,sizeof(union STR_LONG));
//  RVar=&RPtr;  // preparata per i ptr..
  VPtr=V->var;  // salvo quello che arriva... (usato da chi crea PTR)  MA SERVE ANCORA?? 2025
	R.cost=&RCost;


	if(!Co && !Brack)
		_tcscpy(origLabel,Clabel);		// label finale, salvata per || e &&

//  ROut=LastOut;
  oOP=0;
	OP=0;
  do {
	  R.var=&RPtr;        // recupero RVar alterata   IDEM serve ancora??
    OT=FIn->GetPosition();
		FIn->SavePosition();
		ol=__line__;
    AR=FNGetAritElem(outbuf,&OP,TS,V,Co);
// AR% = 1 SE COSTANTE, 2 SE VARIABILE, 3 SE OPERANDO, 0 SE FINE LINEA, -1 se errore!
   if(debug>2) 
     myLog->print(0,"LETTO AritmElem : %d, Brack %d",AR,Brack);

    switch(AR) {
      case ARITM_IS_EOL:
        Exit=TRUE;

					if(__line__== 43) {
//		isWhat=0;			// DEBUG BREAK
//		ol=0;
		}
				if(Pty==14) {
					if(isRValue>0)
						isRValue--;
//					else			// mah capita ma direi solo in certi fine riga
//						PROCWarn(1001,"isRValue");
					}
				if(isPtrUsed) {
//					Regs->DecP();
//					isPtrUsed--;
					}
        break;
      case ARITM_IS_COSTANTE:
        V->tag=NULL;
        ZeroMemory(V->dim,sizeof(O_DIM));
//     myLog->print(0,"LETTO AritmElem : %d",V->cost.l);
//        _tcscpy(V->cost,TS);
				if(Co)
//				if(isWhat==1)
					PROCError(2059,TS);
//				isWhat=1;
        break;
      case ARITM_IS_VARIABILE:
//      	*V->cost=0;
				if(V->var) {		// accade se init al livello più esterno con variabile (ERRORE
					V->tag=V->var->hasTag;
					memcpy(V->dim,V->var->dim,sizeof(O_DIM));
//					_tcscat(outbuf,V->var->name);
					}
				if(Co && Pty==14)		// non è perfetto, ma deve lasciar passare la virgola come separatore exprb
//				if(isWhat==1)
					PROCError(2059,V->var->name);
//				isWhat=1;
        break;
      case ARITM_IS_OPERANDO:
        if(OP>Pty) {
          if(OP>=3 && OP<=10) {          // se c'è un operatore (*,+,<=,&...), TOLGO cond subito!
//            *cond=0;           // in realtà anche alcuni op.2 andrebbero tolti...
            }
//					_tcscat(outbuf,Op[OP].s);
					if(!Co)
						PROCError(2059,TS);
          Exit=TRUE;
					FIn->RestorePosition(OT);
					__line__=ol;
          }
        else {
          *T1S=0;
          switch(OP) {
            case 1:
              switch(*TS) {
                case '(':
                  if(Co>0) {
                    if(V->type & VARTYPE_FUNC) {
                      PROCUsaFun(V->var);		//?? qua 2025, Pty solo se operazione binary
                      V->size=V->var->size;		// specie per inline/builtin 2026, verificare altre!
                      V->type &= ~(VARTYPE_FUNC | VARTYPE_FUNC_USED | VARTYPE_FUNC_BODY) /*0xfffffc7f*/;
                      V->Q=VALUE_IS_EXPR_FUNC;
                      }
                    else
                      PROCError(2064);
                    }
                  else {
                    if(FNIsType(FNLA(MyBuf)) != VARTYPE_NOTYPE) {     // cast
											O_DIM d;
											int16_t i2;
                      l1=FIn->GetPosition();
                      FNLO(MyBuf);
                      V->type=VARTYPE_PLAIN_INT;
                      V->size=0;
											ZeroMemory(V->var,sizeof(struct VARS));
                      PROCGetType(outbuf,&V->type,&V->size,&V->tag,d,&attrib,l1);

                      PROCCheck(')');
                      i2=0;
											inCast=TRUE;
										  FNRev(outbuf,2,&i2,Rlabel,&R);
										  if(R.Q==VALUE_IS_VARIABILE) {

												ReadVar(outbuf,R.var,V->type,V->size,0,
													V->type & VARTYPE_IS_POINTER ? TRUE : FALSE);		// 
												V->Q=V->type & VARTYPE_IS_POINTER ? VALUE_IS_PTR : VALUE_IS_EXPR;
										    }
										  else if(R.Q==VALUE_IS_EXPR || R.Q==VALUE_IS_EXPR_FUNC) {
                      	PROCCast(V->type,V->size,&R.type,&R.size,-1);
                      	V->Q=VALUE_IS_EXPR;
                      	}
										  else if(R.Q==VALUE_IS_PTR) {
                      	V->Q=VALUE_IS_PTR;

												}
										  else if(R.Q==VALUE_IS_D0) {
												if(!R.flag && isRValue) {		// v. anche sotto idem
						  						PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");	// 
													}
												if(R.var->size) 			// v. case 0 in readD0, casi con costante
                      	PROCReadD0(outbuf,R.var,V->type,V->size,0,0,FALSE);
												R.type=R.var->type; R.size=R.var->size;		// sarebbero da unire...
										    V->Q=VALUE_IS_EXPR;
										    }
										  else if(R.Q & VALUE_IS_COSTANTE) {
										    memcpy(V->cost,R.cost,sizeof(union STR_LONG));
										    V->Q=R.Q;
										    }
//										    PROCUseCost(RQ,R.type,R.size,&R.cost);


//											V->var->size=V->size;
//											V->var->type=V->type;
// post archimedes riverificare*********** dic25 in effetti è ok così, v. ReadD0  size=0
											//"arriva così se array è una costante! tipo literal string "abcd"[  "



                      }			// cast
                    else {
//                      BrackOP[Brack]=OP;
//                      BrackPty[Brack]=Pty;
//                      Pty=99;
                      Brack++;
//                      if(Brack>=10)
//												PROCError(1035);                      
                      FNRev(outbuf,99,cond,Clabel,V);
//                      FNRev(99,cond,&R.type,&R.size,&RQ,&RVar,&R.cost,&RTag,&RDim);
                      // su cond ho dei dubbi...
                      }
                    }
                  break; 
                case ')':
                  if(Pty==99) {		// o usare inCast
                    Brack--;
										if(debug)
											myLog->print(0," brack ) V->Q=%x, oOP %d",V->Q,oOP);



                    if(oOP==12   /*&& !(V->Q & VALUE_HAS_CONDITION)*/) {
//                      PROCOutLab(Clabel);
											if(debug)
												myLog->print(0," label (da ) ) %s; V->Q=%x",Clabel,V->Q);
//											oOP=0; inutile, poi esco
											}



//                  Pty=1;
//                    OP=BrackOP[Brack];
//                    Pty=BrackPty[Brack];
//										isWhat=0;
                    }
                  else {
                    FIn->unget(')');
//                    Exit=TRUE;
//										isWhat=0;
										inCast=FALSE;
                    }
                  Exit=TRUE;
                  break;

                case ']':
                  FIn->unget(']');
                  Exit=TRUE;
          // FORSE SAREBBE MEGLIO METTERCI UN IDENT. DI "[" PENDENTE
                  break;
                case '[':
									{uint8_t residualPtr=0;		// usato per array di stringhe e simili, ossia quando hai più parentesi [] che dim!
									uint8_t s1,s2;
									int l2=0;

                  l=0;
                  v=0;               // flag per ReadD0
                  T=0;               // 1 quando leggo la base-array
                  T1=V->type;
									isPtrUsed++;
//									Regs->IncP();
                  if(V->Q==VALUE_IS_VARIABILE || V->Q==VALUE_IS_COSTANTEPLUS) {		// per consentire uso di stringhe come array, credo...
                    T1 |= VARTYPE_ARRAY;
                    } 
									I=T1;
									if(T1 & VARTYPE_ARRAY)
										T1 = (T1 & ~VARTYPE_IS_POINTER) | FNGetArrayDims(V->var);		// patch per avere le reali dimensioni e gestire array di stringhe
                  for(;;) {
										R.flag = (I & VARTYPE_IS_POINTER)-1;			// livelli di indirezione, ricorsivi a cascata; -1 perché array han sempre 1 ptr
	                  if(I & VARTYPE_IS_POINTER) {
											R.type=T1;	// verrà usato per gli indici a seguire, se ci sono
//	                    T1=(T1 & VARTYPE_NOT_A_POINTER) | ((T1 & VARTYPE_IS_POINTER) -1); messo DOPO per memsize
											// v. anche R.flag
	                    I=(I & VARTYPE_NOT_A_POINTER) | ((I & VARTYPE_IS_POINTER) -1); // per controllo #dim


// NON fare se costante su 68000! spostare sotto e v sotto altro

	                    if(i)
	                      PROCError(1035);
	  	                j=*cond;
		                  *cond = VALUE_CONDITION_UP;
	//                    *cond=0;
											isPtrUsed++;
	                    FNRev(outbuf,15,cond,Rlabel,&R);
											isPtrUsed--;

											if((R.type & ~VARTYPE_UNSIGNED) != VARTYPE_PLAIN_INT)
												PROCError(2111);

												//ma occhio a : 2[a] = 5; // Valido! E1 è intero, E2 è puntatore gemini 2026
		                  if(!T && (R.Q>=VALUE_IS_EXPR && R.Q<=VALUE_IS_VARIABILE)) {
			                  if(V->Q==VALUE_IS_VARIABILE) {
			                    if(V->type & VARTYPE_ARRAY) {
			                      PROCGetAdd(VALUE_IS_VARIABILE,V->var,l,TRUE);
//											myLog->print(0,"faccio GETADD 1 con ofs %d",l);
			                      R.cost->l=l=0;
			                      }
			                    else if(V->type & VARTYPE_IS_POINTER) {

														ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,TRUE);
															}
			                      }
			                    T=1;  
			                    }
			                  else if(V->Q == VALUE_IS_COSTANTEPLUS) {		// cose tipo "abcd"[1]
											    PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,TRUE);
											// e ovviamente se costante pure l'indice si potrebbe ottimizzare! v.sotto
								          T=1;  
										      }
			                  }  


// NON fare se costante su 68000! spostare sotto idem v.sopra

	                    if(i)
	                      PROCError(1035);

											s1=FNGetMemSize(I,V->size,NULL,0);
											s2=FNGetMemSize(I,V->size,V->dim,0);
	                    switch(R.Q) {
	                      case VALUE_IS_EXPR:			// espressione
	                      case VALUE_IS_EXPR_FUNC:			// funzione
												case VALUE_IS_PTR:

	                        PROCCast(VARTYPE_UNSIGNED,INT_SIZE,&R.type,&R.size,-1);        // l'indice array dev'essere unsigned int


													// OTTIMIZZARE con ASL ;) se multiplo di 2 tipo PTR
													if(R.flag>0) {
														if(s2 > 1) {
															if(FNIsPower2(s2))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");// (multidim)
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															}
															// (multidim)
														}
													else {
														if(s1 > 1) {
															if(FNIsPower2(s1))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															}
														}

	                        v |= 1;   // segnalo indice in R
	                        break;

	                      case VALUE_IS_D0:
													isPtrUsed++;
											    PROCReadD0(outbuf,R.var,VARTYPE_UNSIGNED,INT_SIZE,*cond & VALUE_CONDITION_MASK,0,
														V->type & VARTYPE_IS_2POINTER);
													isPtrUsed--;
													if(R.flag>0) {
														if(s2 > 1) {
															if(FNIsPower2(s2))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															// (multidim)
															}
														}
													else {
														if(s1 > 1) {
															if(FNIsPower2(FNGetMemSize(I,V->size,NULL,0)))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															// (multidim)
															}
														}

	                        v |= 1;   // segnalo indice in R
	                        break;
	                      case VALUE_IS_VARIABILE:
//													if(R.flag>0 && FNGetMemSize(T1 /* era V->type*/ /*& ~VARTYPE_ARRAY*/,V->size,NULL,0) > 1)
													if(FNGetMemSize(V->var->type,V->var->size,NULL,0) > 1 || R.var->size<INT_SIZE) {
														ReadVar(outbuf,R.var,VARTYPE_UNSIGNED,INT_SIZE,0,FALSE);
														}
													else {
														switch(R.var->classe) {
															case CLASSE_EXTERN:
															case CLASSE_GLOBAL:
															case CLASSE_STATIC:
      													break;
															case CLASSE_REGISTER:
																break;
															case CLASSE_AUTO:
																break;
															}
														}
//													PROCCast(VARTYPE_UNSIGNED,INT_SIZE,&R.var->type,&R.var->size,-1);
													if(R.flag>0) {
														if(s2 > 1) {
															if(FNIsPower2(s2))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															}
														// (multidim)
														}
													else {
														if(s1 > 1) {
															if(FNIsPower2(FNGetMemSize(I,V->size,NULL,0)))
																PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d");
															else
																PROCOper(LINE_TYPE_ISTRUZIONE,"MUL.d");
															// (multidim)
															}
														}

														// più o meno va, ma manca poi il secondo indice se constante - se var pare ok!
													// o si somma esplicitamente come una var, o sarebbe da ottimizzare in StoreVar...
R.cost->l=0;		// l?
	                        v |= 1;   // segnalo indice in R
	                        break;
	                      case VALUE_IS_COSTANTE:
/*					da MSVC
;|***    	ch=provaaa[2][3];
	*** 000574	a0 11 00 		mov	al,BYTE PTR _provaaa+17		2*7+3
	*** 000577	a2 00 00 		mov	BYTE PTR _ch,al
;|*** 	ch=provaaa[4];
	*** 00057a	b0 1c 			mov	al,OFFSET DGROUP:_provaaa+28		4*7
	*** 00057c	a2 00 00 		mov	BYTE PTR _ch,al*/
													l2=l;
													if(R.flag>0)
														l += (R.cost->l * s2);  // (multidim)
													else
														l += (R.cost->l * s1);  // 
													R.cost->l=0;
	                        v |= 2;   // segnalo indice cost. in l
	                        break;  
	                      case VALUE_IS_COSTANTEPLUS:
		                      PROCError(1035);
	                        break;
	                      }			// switch

											if(!(T1 & VARTYPE_IS_POINTER  ) /*T1 & VARTYPE_IS_POINTER*/  &&  V->var->type & VARTYPE_IS_2POINTER) {
												residualPtr=VARTYPE_POINTER;
												if(V->var->type & VARTYPE_ARRAY) {		// non se (doppio) puntatore usato come array
													R.cost->l=l;
													l=l2;
													R.cost->l -= l2;
													}
												}

                      if(R.Q>=VALUE_IS_EXPR && R.Q<=VALUE_IS_VARIABILE) {// come fa a funzionare sta cosa?? 2026
                      if(R.Q & VALUE_IS_COSTANTE) {
												// NON dovrebbe servire qua! 
                        }
                      else {
												// se non c'era bisogno di moltiplicare, si potrebbe sommare direttamente la variabile... v. sopra
												if(R.flag>0 && s1 > 1)
													PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d");
												else {
														PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d");
													}
                        
                        }
	//                    V->Q=RQ;  
	//                    V->var=RVar;
	//                    *V->cost=R.cost;
//										    memcpy(V->cost,R.cost,sizeof(union STR_LONG);
	  	                *cond=*cond ? j : 0;


											// v.sopra

//											isPtrUsed=0;		// o decrementare per ogni quadra chiusa...

											R.flag--;
	                    PROCCheck(']');
	                    }		// se ancora array/ptr
	                  else
	                    PROCError(2109);

										FNLA(MyBuf);
										if(*MyBuf == '[')
											FNLO(MyBuf);
										else
											break;
	                  T1=(T1 & VARTYPE_NOT_A_POINTER) | ((T1 & VARTYPE_IS_POINTER) -1); // 

	                  }			// for

                  if(!T) {  // se non l'ho letto prima...
	                  if(V->Q==VALUE_IS_VARIABILE) {
	                    if(V->type & VARTYPE_ARRAY) {
	                      PROCGetAdd(VALUE_IS_VARIABILE,V->var,l,TRUE);
//											myLog->print(0,"faccio GETADD 2 con ofs %d",l);
	                      l=R.cost->l;		// e in R.cost->l ho il residuo da passare, se array multidimensione/stringhe

			                  if(!(T1 & VARTYPE_IS_POINTER) && !(I & VARTYPE_IS_POINTER)
													&& V->var->type & VARTYPE_IS_2POINTER) {
													PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
												}

	                      }
	                    else if(V->type & VARTYPE_IS_POINTER) {
												if(V->var->classe == CLASSE_REGISTER) {
													goto read_array_add_cmq;		// mah serve cmq... o forse in alcuni casi si potrebbe ottimizzare? altre cpu
													}
												else  {
read_array_add_cmq:
	                        ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,TRUE);
													if(l) {
//	lo aggiungo dopo a readD0...			                    PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d",OPDEF_MODE_REGISTRO32,
	//														Regs->P,OPDEF_MODE_IMMEDIATO32,l);
														}
													}
	                      }
	                    T=1;  
	                    }
	                  else if(V->Q == VALUE_IS_COSTANTEPLUS) {		// cose tipo "abcd"[1]
									    PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,TRUE);
											// e ovviamente se costante pure l'indice si potrebbe ottimizzare! pare lo faccia già cmq, 68000 SE NON l'ha letto prima... v. sopra
	                    T=1;  
	                    }
//								if(isPtrUsed)
//									Regs->DecP();
	                  }  

                  if(T1 & VARTYPE_ARRAY) {
//                    V->size=FNGetMemSize(I & ~VARTYPE_ARRAY,V->size,NULL/*dim*/,0);		// se ancora puntatore dopo la dereference...
//       myLog->print(0,"\aVQ %d, VarClass %d",V->Q,(V->Q ==3)?V->var->classe : 0);
										if(v & 2) {
//                        u[1].ofs=l;
                      i=0;
                      }
										if(v & 1) {
  // MAH 2026?? GD24032 no                    l=Regs->D;
											i=1;
											}
                    T1 &= ~VARTYPE_ARRAY /*0xFFFFFBFF*/;
//                    T1=(T1 & VARTYPE_NOT_A_POINTER) | ((T1 & VARTYPE_IS_POINTER) -1);
                    if(FNIsOp(MyBuf,0)>=2) {		// sta cosa è strana, ma sembra ok... 2026

//                      *cond=0;
//                      u[0].mode=0x80;
//                      PROCReadD0(V->size,V->type,&u[0],&u[1],i,*cond & 0xff);
										if(!(V->type & VARTYPE_ARRAY) && Pty<=14) {		// solo se era puntatore usato come array, altrimenti sono a posto (INEVITABILE un passaggio in più in registro...
											if(!(*cond & VALUE_CONDITION_MASK))			// qua non serve, lo faccio da EvalCond (v.
												PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,0 /*R.Q == VALUE_IS_COSTANTE ? R.cost->l : 0  in effetti l'ho già messo sopra!*/,
													FALSE);
											}
										}

										/* no if(isRValue) {		// se sono lvalue lo tengo incrementato
											Regs->DecP();
											isPtrUsed--;
											}*/

//												i=FNGetArrayDims(V->var) != V->var->type;
										if(V->type & VARTYPE_ARRAY) {
											if(I & VARTYPE_IS_POINTER) {
												if(FNGetArrayDims(V->var) > (I & VARTYPE_IS_POINTER))
													

													V->Q = VALUE_IS_PTR;
												else
													V->Q = VALUE_IS_D0;
												}
											else {
												V->Q = VALUE_IS_D0;
												}
											}
										else
											V->Q = VALUE_IS_D0;
										V->type = (I & ~VARTYPE_ARRAY) | residualPtr;
										V->var=VPtr;
										V->var->modif=i;
										V->var->type=V->type;
										V->var->size=V->size;
										V->var->func.value=reg2;
										V->var->parm.ofs=l; V->var->parm.flag=1;
										memcpy(V->dim,R.dim,sizeof(O_DIM));
										V->flag=1; // v & 2 ? 1 : 0 /*R.flag+*/;		// (CMQ se indice tutto costante, NON devo poi mettere Dn in An
								    memcpy(V->cost,R.cost,sizeof(union STR_LONG));
//											myLog->print(0,"array esce con ofs %d",l);
                    }
                  else
                    PROCError(2109);
									}
                  break;

                case '.':
                case '-':
                case ':':
									i=0;
rifo_struct:
									T=*TS=='.';		// 1 se membro, 0 se puntatore  AMPLIARE QUA
									if(!V->tag)
										PROCError((*TS=='.') ? 2224 : 2223);
									FNLO(T1S);
									if(*FNLA(MyBuf) == '(') {
										long tt=FIn->GetPosition();
										PROCCheck('(');
										collectParmList(MyBuf1);
										FIn->RestorePosition(tt);
//							__line__=ol;

rifo_inherit:
										_tcscpy(MyBuf,V->tag->label);
										_tcscat(MyBuf,"_");
										_tcscat(MyBuf,T1S);
										_tcscat(MyBuf,"__");
										_tcscat(MyBuf,MyBuf1);
										R.var=FNGetAggr(V->tag,MyBuf,(V->type & VARTYPE_CLASS) ? 2 : ((V->type & VARTYPE_STRUCT) ? 1 : 0),&reg2);
										if(!R.var) {
											if(V->tag->parent) {
												V->tag=V->tag->parent;
//											R.var=FNGetAggr(V->tag->parent,MyBuf,(V->type & VARTYPE_CLASS) ? 2 : ((V->type & VARTYPE_STRUCT) ? 1 : 0),&reg2);
												PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,V->tag->label,NULL,NULL,"chiamo padre",LINE_IS_NORMAL);
												goto rifo_inherit;
												}
											PROCError(2660,T1S);
											goto no_member_struct;
											}
										PROCCheck('(');
										if(R.var->type & VARTYPE_FUNC) {

//									_tcscat(outbuf,R.var->name);
											PROCUsaFun(R.var,R.var->classe == CLASSE_MEMBER_STATIC ? FALSE : TRUE,V->var->name);
											}
										else
											PROCError(2064,T1S);
										goto no_member_struct;
										}
									else {
rifo_inherit2:
										R.var=FNGetAggr(V->tag,T1S,(V->type & VARTYPE_CLASS) ? 2 : ((V->type & VARTYPE_STRUCT) ? 1 : 0),&reg2);
										if(!R.var) {
											if(V->tag && V->tag->parent) {
												V->tag=V->tag->parent;
//											R.var=FNGetAggr(V->tag->parent,MyBuf,(V->type & VARTYPE_CLASS) ? 2 : ((V->type & VARTYPE_STRUCT) ? 1 : 0),&reg2);
												PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,V->tag->label,NULL,NULL,"vado a padre",LINE_IS_NORMAL);
												goto rifo_inherit2;
												}
											PROCError(2039,T1S);
											goto no_member_struct;
											}
										}
//                  myLog->print(0,"GetAGGR\a: %d",reg2);
									_tcscat(outbuf,R.var->name);
									if(V->type & VARTYPE_IS_POINTER) {
		//								if(i)
//						  				PROCOper(LINE_TYPE_ISTRUZIONE,"adda.l",OPDEF_MODE_REGISTRO_INDIRETTO,Regs->P,		// MemoryModel
//												OPDEF_MODE_REGISTRO32,Regs->D);
//						  				PROCOper(LINE_TYPE_ISTRUZIONE,"adda.l",OPDEF_MODE_IMMEDIATO16,reg2,		// MemoryModel
	//											OPDEF_MODE_REGISTRO_INDIRETTO,Regs->P);
										}
//									else
	//									u[1].ofs = /* += */ reg2;
									if(V->Q==VALUE_IS_D0) {
										if(*TS=='.') {
											if(V->type & VARTYPE_IS_POINTER)
												PROCError(2221);
											else 
												PROCGetAdd(VALUE_IS_COSTANTE,V->var,0,TRUE);
											}
										else {
											if(V->type & VARTYPE_IS_POINTER) {
												PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,0,i>0);
												}
											else 
												PROCError(2222);
											}
										}
									else if(V->Q==VALUE_IS_VARIABILE) {
										if(*TS=='.') {
											if(V->type & VARTYPE_IS_POINTER)
												PROCError(2221);
											else 
												PROCGetAdd(VALUE_IS_VARIABILE,V->var,0,TRUE);
											}
										else {
											if(V->type & VARTYPE_IS_POINTER) {
												ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,TRUE);
												}
											else 
												PROCError(2222);
											}
										}
									FNLA(TS);
									if(*TS=='(') {

										goto no_member_struct;
										}
									else if(*TS=='.' || *TS=='-') {
										T=*TS=='.';		// 1 se membro, 0 se puntatore
										FNLO(TS);
										V->var=R.var;
										V->type=R.var->type;
						        V->tag=R.var->hasTag;
										// FINIRE accumulando, e/o gestendo puntatore
										V->Q=VALUE_IS_D0;
						  				PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d");		// sarebbe bello unire con la seguente, ma se c'è mix di ptr e . è un casino...

										if(OutSource) {
									//    i=_tcslen(LastOut->s)+_tcslen(V->name)+25;
									//    PROCOut(NULL,"\t\t\t\t; ",V->name,NULL,NULL);	
									//    LastOut=(struct LINE *)_frealloc(LastOut,i);
									//    LastOut->prev->next=LastOut;
//											_tcscpy(LastOut->rem,"ofs ");
	//										_tcscat(LastOut->rem,R.var->name);
											}
										PROCWarn(1003,"struct nidificata");
										i++;
										goto rifo_struct;
										}			// seguono ancora membri

									if(R.var->type & VARTYPE_BITFIELD) {
										reg2 = reg2/INT_SIZE;		// FINIRE!!
										if(reg2)
								  		PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d");
										if(isRValue) {
											i=FNGetAggr2(V->var,R.var,&reg2,&j);
											PROCReadD0(outbuf,R.var,V->type,V->size,0,j,FALSE);		// size sarà sempre int quindi 4!
											if(i)		// anche signed :)
							  				PROCOper(LINE_TYPE_ISTRUZIONE,"AND.d");	// mettere in funz a sé?
											if(reg2 > 0) {
												if(reg2<8)
													PROCOper(LINE_TYPE_ISTRUZIONE,R.type & VARTYPE_UNSIGNED ? "SRA" : "SRL");
												else {
													PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
													PROCOper(LINE_TYPE_ISTRUZIONE,R.type & VARTYPE_UNSIGNED ? "SRA" : "SRL"
														);
													}
												}
											V->Q=VALUE_IS_EXPR;	
											}
										else {
											i=FNGetAggr2(V->var,R.var,&reg2,&j);
											PROCReadD0(outbuf,R.var,V->type,V->size,0,j,FALSE);		// size sarà sempre int quindi 4!
											VPtr=R.var;			// salvo il membro per store
											V->Q=VALUE_IS_D0;		// FINIRE se lvalue
											}
										if(OutSource) {
									//    i=_tcslen(LastOut->s)+_tcslen(V->name)+25;
									//    PROCOut(NULL,"\t\t\t\t; ",V->name,NULL,NULL);	
									//    LastOut=(struct LINE *)_frealloc(LastOut,i);
									//    LastOut->prev->next=LastOut;
//											_tcscpy(LastOut->rem,"mask/shift ");
	//										_tcscat(LastOut->rem,R.var->name);
											}
										}
									else {
//										if(reg2 && !T) {		// altrimenti uso offset diretto poi
//							  			PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d",OPDEF_MODE_REGISTRO32,		// MemoryModel
	//											Regs->P,OPDEF_MODE_IMMEDIATO32,reg2);
	//										}
	                  V->Q=VALUE_IS_D0;
										if(OutSource) {
									//    i=_tcslen(LastOut->s)+_tcslen(V->name)+25;
									//    PROCOut(NULL,"\t\t\t\t; ",V->name,NULL,NULL);	
									//    LastOut=(struct LINE *)_frealloc(LastOut,i);
									//    LastOut->prev->next=LastOut;
//											_tcscpy(LastOut->rem,reg2 ? "ofs. " : "ofs.=0 ");
	//										_tcscat(LastOut->rem,R.var->name);
											}
										}

//							    PROCReadD0(V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,reg2,FALSE);
                  V->type=R.var->type;
                  V->size=R.var->size;
                  if(V->type & VARTYPE_IS_POINTER)
                    R.size=getPtrSize(V->type);
                  else 
                    R.size=V->size;        
//                  PROCReadD0(R.size,V->type & VARTYPE_NOT_A_POINTER,&u[0],&u[1],0,*cond & 0xff);
									V->var=VPtr;
//									*V->var=*R.var  /*era VPtr  -2025*/;
									// non viene copiato il nome della var o membro struct... boh? serve? 2025
									V->var->type=V->type/* & VARTYPE_NOT_A_POINTER*/;
									V->var->modif=0;
									V->var->size=R.size;
//									V->var->func.value=Regs->D;
									V->var->parm.ofs=R.var->type & VARTYPE_BITFIELD ? j : reg2; V->var->parm.flag=1;
                  V->tag=R.var->isInTag;
									V->flag=1;	//									V->flag= ??  0 per copiare ev. Dn in An, v.array e ptr
									memcpy(V->dim,R.dim,sizeof(O_DIM));

									isPtrUsed=1;
no_member_struct: ;
                  break;
                default:
                  break;
                }
              break;

            case 2:
              switch(*TS) {
                case '-':
                  if(*(TS+1) != '-') 
                    goto LUnaryMinus;
									else
										goto LBinaryMinus;
                case '+':      
                  if(*(TS+1) != '+') 
										continue;//                    goto LUnaryPlus;
LBinaryMinus:
//									if(isWhat==2)
//										PROCError(2059,TS);
                  if(!Co) {
                    T=1;											// se pre-inc
//                    BS=Regs->DS;
//                    *cond=0;
                    FNRev(outbuf,2,cond,Clabel,V);
                    }
                  else {
                    T=2;                       // se post-inc
//                    FNLA(MyBuf);
//                    if(!*MyBuf || *MyBuf==';' /*|| *MyBuf==')'*/) {   // mezza boiata...
										if(Pty>14 && !*cond) {
                      T=0;
//		                  BS=Regs->DS;
		                  }
                    else  
//		                  BS=Regs->D1S;
;
                    }

                  v=V->size;		// a che serve?? 2025
                  I=1;
                  if(V->type & VARTYPE_IS_POINTER) {
                    v=getPtrSize(V->type);
                    I=V->size;
										// credo si debba verificare se doppio puntatore... FNGetMemsize(
                    }
									else
                    I=1 /*V->size*/;
//									PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
									
									if(V->type & VARTYPE_IS_POINTER)
										V->flag=1;		// indico che ho già il puntatore pronto
									_tcscat(outbuf,TS);
	                break;

	              case '*':
	                i=*cond;
	                *cond = VALUE_CONDITION_UP;
									isPtrUsed++;
								  FNRev(outbuf,2,cond,Clabel,&R);
									isPtrUsed--;
								  if(R.Q==VALUE_IS_VARIABILE) {
								    if(R.var->classe == CLASSE_REGISTER) {
											ReadVar(outbuf,R.var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,TRUE);	// Pty<=14 ? TRUE : FALSE ...
											}								      
								    else {
											ReadVar(outbuf,R.var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,TRUE);	// Pty<=14 ? TRUE : FALSE ...

								      }
								    }
									else if(R.Q==VALUE_IS_D0) {
										if(R.var->parm.flag) {			// v. case 0 in readD0, casi con costante
										//if(Pty<14) {			// solo se non assegnazione (altrimenti ci pensa dopo
												PROCReadD0(outbuf,R.var,0,0,*cond & VALUE_CONDITION_MASK,R.var->parm.ofs,TRUE);
												R.var->parm.ptr=NULL;

											//	}
											}
										}
									else if(R.Q==VALUE_IS_PTR) {
// test					PROCOper(LINE_TYPE_COMMENTO,0,OPDEF_MODE_NULLA,(union SUB_OP_DEF *)0,0,"VALUE_IS_PTR");
										}
									else if(R.Q==VALUE_IS_EXPR || R.Q==VALUE_IS_EXPR_FUNC) {
										if(R.var->size) { 			// v. case 0 in readD0, casi con costante
											if(Pty<14 /*isRValue*/) {			// solo se non-assegnazione (altrimenti ci pensa dopo   
												PROCReadD0(outbuf,R.var,0,0,*cond & VALUE_CONDITION_MASK,0,TRUE);
												}
											}
											// e in certi casi una copia spuria da D a P, tipo un *p++; da solo
										if(!R.var->size || !isRValue /*Pty<14*/ /*isRValue*/) {			// solo se non-assegnazione (altrimenti ci pensa dopo   
//											if(Pty<=14) 			// solo se non livello esterno
//								  			PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d",OPDEF_MODE_REGISTRO32,Regs->P,OPDEF_MODE_REGISTRO32,Regs->D);
											R.flag=1;		// stronco ev. letture a seguire!
											}
										}
									else if(R.Q & VALUE_IS_COSTANTE) {
								    PROCUseCost(outbuf,R.Q,R.type,R.size,R.cost,TRUE);
										}
	                if(R.type & VARTYPE_IS_POINTER) {
	                  V->type=(R.type & VARTYPE_NOT_A_POINTER) | ((R.type & VARTYPE_IS_POINTER) -1);
	                  V->size=FNGetMemSize(V->type,R.size,NULL/*dim*/,1);		// 
	                  }
	                else
	                  PROCError(2100);

	                *cond=*cond ? i : 0;
//	                PROCReadD0(V->size,V->type,&u[0],0,*cond & 0xff,			FALSE /*o altro liv ptr??*/);
									V->var=VPtr; 
									V->var->modif=0;
									V->var->type=V->type;
									V->var->size=V->size;
									V->var->classe=R.var->classe;
									V->var->func.func=(struct VARS *)reg2;		// credo cazzata, 2025, di sicuro per 68000 sopra (perché poi leggo da A0
									V->var->parm.ofs=0; V->var->parm.flag=0;
									V->tag=R.tag;
									switch(R.Q) {
										case VALUE_IS_PTR:
											V->flag=  1;	
											break;
										case VALUE_IS_VARIABILE:
											V->flag=  1;	
											break;
										case VALUE_IS_D0:
											V->flag=R.flag;
											break;
										default:
											V->flag=  0;		// indica se devo caricare Dn in An, dopo
											break;
										}
//										V->flag= ((R.Q==VALUE_IS_EXPR || R.Q==VALUE_IS_EXPR_FUNC) && !R.flag) ? 0 : 1;		// indica se devo caricare Dn in An, dopo

									memcpy(V->dim,R.dim,sizeof(O_DIM));
							    memcpy(V->cost,R.cost,sizeof(union STR_LONG));

									V->Q=VALUE_IS_D0;
									isPtrUsed++;
	                break;

	              case '&':
//	                *cond=0;
	                FNRev(outbuf,2,cond,Clabel,&R);
	                switch(R.Q) {
										case VALUE_IS_PTR:

	                  case VALUE_IS_EXPR:
	                  case VALUE_IS_EXPR_FUNC:
											if(R.var->type & VARTYPE_BITFIELD)		// arriva qua!
												PROCError(2104);
											else
												PROCError(2101);
											break;
	                  case VALUE_IS_D0:
											PROCGetAdd(VALUE_IS_D0,R.var,R.var->parm.ofs,TRUE);		// mah RIVERIFICARE gli altri, 2025
											V->Q=VALUE_IS_PTR;
  	                  break;
	                  case VALUE_IS_VARIABILE:
	                    PROCGetAdd(VALUE_IS_VARIABILE,R.var,0,TRUE);			// idem
//	                    PROCGetAdd(VALUE_IS_VARIABILE,R.var,0,(isPtrUsed && Pty!=99) ? TRUE : FALSE);
											V->Q=VALUE_IS_PTR;
	                    break;
	                  case VALUE_IS_COSTANTEPLUS:		// boh, 2025...serve??
	                    PROCGetAdd(VALUE_IS_COSTANTEPLUS,R.var,0,TRUE);
//	                    PROCGetAdd(VALUE_IS_COSTANTEPLUS,R.var,0,(isPtrUsed && Pty!=99) ? TRUE : FALSE);
											V->Q=VALUE_IS_PTR;
	                    break;
	                  default:
	                    break;
	                  }
	                V->type=VARTYPE_UNSIGNED | VARTYPE_POINTER;
		              V->size=getPtrSize(V->type);
		              break;

		            case '~':
//		              *cond=0;
		              FNRev(outbuf,2,cond,Clabel,V);
		              switch(V->Q) {
		                case VALUE_IS_COSTANTE:
		                  V->cost->l=~V->cost->l;
		                  break;
		                case VALUE_IS_COSTANTEPLUS:		// 9
		                  PROCUseCost(outbuf,VALUE_IS_COSTANTE /*era-2*/,V->type,V->size,V->cost,FALSE);
											PROCOper(LINE_TYPE_ISTRUZIONE,"NEG.d");
		                  V->Q=VALUE_IS_EXPR;
		                  break;
		                default:
											if(V->Q==VALUE_IS_VARIABILE) {
												ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,FALSE);
												}
											if(V->Q==VALUE_IS_D0) {
										    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,0,FALSE);
												}
											if(V->Q & VALUE_IS_CONDITION_VALUE) {
		                    PROCAssignCond(&V->Q,&V->type,&V->size,Clabel);
												}
													PROCOper(LINE_TYPE_ISTRUZIONE,"NEG.d");
		                  V->Q=VALUE_IS_EXPR;
		                  break; 
		                }
		              break;

	              case '!':
	                FNRev(outbuf,2,cond,Clabel,V);
	                if(V->Q & (VALUE_IS_CONDITION | VALUE_IS_CONDITION_VALUE)) {
                    V->Q ^= 1;
                    }
                  else if(V->Q == VALUE_IS_COSTANTE) {                   // mancherebbe V->Q=-2 ossia 9...
                    V->cost->l = !V->cost->l;
	                  V->Q=VALUE_IS_COSTANTE;
	                  }
	                else {
										if(V->Q==VALUE_IS_D0) {
									    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,0,FALSE);
											}
										else if(V->Q==VALUE_IS_VARIABILE) {
	                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,FALSE);
											}
										else if(V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC
											|| V->Q==VALUE_IS_PTR) {

//											PROCWarn(1001,"PROVARE IF ! EXPR");		// provare, sembra ok 11/11

											V->Q = /*VALUE_IS_CONDITION |*/ VALUE_IS_CONDITION_VALUE /*| VALUE_IS_EXPR*/;
/* No non ha senso, i flag han significato diverso se condizione!
											if(V->var->type & VARTYPE_FUNC)		// beh è uguale cmq
												V->Q |= 1;
											else
												V->Q |= 1;*/

											goto unarynot_done;
											}
	                  if(!(*cond & VALUE_CONDITION_MASK)) {
											PROCOper(LINE_TYPE_JUMPC /* per formato istruzione..*/,"S");		// mancherebbe Size...
											if(V->size > 1) {
												PROCOper(LINE_TYPE_ISTRUZIONE,"SE.w");
												if(V->size > 2) {
	    										PROCOper(LINE_TYPE_ISTRUZIONE,"SE.l");
													}
												}
  										V->Q=VALUE_IS_EXPR;
											V->size=INT_SIZE;
											V->type=VARTYPE_PLAIN_INT;
		                  }  
		                else  
  	                  V->Q = VALUE_IS_CONDITION | CONDIZ_UGUALE;     // segnala ! condizionale (cmq implicito in CONDIZ_
	                  }
unarynot_done:
	                break;

	              case 'z':                  // finto per lasciare il case!
LUnaryMinus:
//	                *cond=0;
	                FNRev(outbuf,2,cond,Clabel,V);
	                switch(V->Q) {
	                  case VALUE_IS_COSTANTE:
	                    V->cost->l=-V->cost->l;
//	                    myLog->print(0,"\aUNARY MINUS su COST %s",V->cost);
	                    break;
	                  case VALUE_IS_COSTANTEPLUS:
		                  PROCUseCost(outbuf,VALUE_IS_COSTANTE /*era-2*/,V->type,V->size,V->cost,FALSE);
											PROCOper(LINE_TYPE_ISTRUZIONE,"NEG.d");
	                    V->Q=VALUE_IS_EXPR;
	                    break;
	                  default:
											if(V->Q==VALUE_IS_D0) {
										    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,0,FALSE);
												}
											if(V->Q==VALUE_IS_VARIABILE) {
	                      ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
												}
    											PROCOper(LINE_TYPE_ISTRUZIONE,"NEG.b");
	                    V->Q=VALUE_IS_EXPR;
	                    break;
	                  }        
	                break;

	              case 's':			// ossia sizeof
									if(*FNLA(TS) == '(')
										PROCCheck('(');		// (IN EFFETTI è opzionale...
	                T=-1;
	                l1=FIn->GetPosition();
	                R.type=VARTYPE_PLAIN_INT;
//	                v=0;
	                FNLO(TS);
	                if(FNIsType(TS) != VARTYPE_NOTYPE) {
										I=0;
	                  PROCGetType(outbuf,&R.type,(uint16_t*)&T,&R.tag,(uint32_t*)&R.dim,&attrib,l1);
	                  }
	                else {
										FIn->RestorePosition(l1);
										__line__=ol;
//	                  *cond=0;
	                  FNRev(outbuf,15,cond,Rlabel,&R);
	                  T=R.size;
		                if(R.type & VARTYPE_ARRAY)
		                  T=FNGetArraySize(R.var);
		                else if(R.type & VARTYPE_IS_POINTER)
		                  T=getPtrSize(R.type);
	                  }
									if(*FNLA(TS) == ')')
										PROCCheck(')');
	                V->cost->l=LOWORD(T);		// anche >65536?? MemoryModel?
	                V->Q=VALUE_IS_COSTANTE;
									// sarebbe carino accorpare con ev. costante che segue...
	                V->type=VARTYPE_UNSIGNED | VARTYPE_PLAIN_INT;
	                V->size=2 /*INT_SIZE*/;
	                break;
	              default:
	                break;
	              }
	            break;

	          case 6:
	          case 7:		// v. sotto

            case 3:
            case 4:
	          case 5:
	          case 8:
	          case 9:
	          case 10:
							if(!Co /*isWhat==2*/)
								PROCError(2059,TS);
	            reg2=0;
//							reg2=1;
	            if(V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || V->Q==VALUE_IS_D0 || (V->Q==VALUE_IS_VARIABILE && OP!=6 && OP!=7)) {		// tutte tranne CMP (solo shift/rotate se var qua
                }

              j=*cond;
              *cond = VALUE_CONDITION_UP;

							if(isPtrUsed && (V->Q==VALUE_IS_D0 || V->Q==VALUE_IS_PTR)) {
								isPtrUsed++;
								}

	            if(!FNRev(outbuf,OP-1,cond,Rlabel,&R))			// fine riga dopo un operando
					      /*non va verificare sopra EOL e Co   PROCError(2059,TS)*/;
								if(isPtrUsed && (V->Q==VALUE_IS_D0 || V->Q==VALUE_IS_PTR)) {
									isPtrUsed--;
									}

	            if(V->Q==VALUE_IS_EXPR_FUNC && R.Q==VALUE_IS_EXPR_FUNC) {		// recupero D0 (se il primo operando è una funzione E ANCHE IL SECONDO, devo salvare D0 per sicurezza
								PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d"/*popString*/);
//                swap(&ROut,&LastOut);		// vado a inserire
								PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d"/*pushString*/);
  //              swap(&ROut,&LastOut);
								}
              *cond=*cond ? j : 0;
	            // qui usiamo i registri Increm. per le operazioni varie dell'exp 2...
	            if(V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || V->Q==VALUE_IS_D0 || (V->Q==VALUE_IS_VARIABILE && OP!=6 && OP!=7)) {		// tutte tranne CMP (solo shift/rotate se var qua
                if(!reg2)
//                myLog->print(0,"Dec 1: %d",Regs->D);
;
                }

							VQ1=V->Q;
	            if((V->Q & VALUE_IS_COSTANTE) && (R.Q & VALUE_IS_COSTANTE)) {
							  switch(*TS) {
							    case '+':
							    case '-':
							    case '=':
							    case '*':
							    case '/':
							    case '%':
							    case '|':
							    case '&':
							    case '^':
							    case '?':
							      BS=TS;
							      *(BS+1)=0;
							      break;
							    case '!':
							      BS="@";         // simboli usati da eval per semplicità
							      break;
							    case '<':
							      if(*(TS+1) == '=')
							        BS=TS;
							      else
							        BS="l";
							      break;
							    case '>':
							      if(*(TS+1) == '=')
							        BS=TS;
							      else
							        BS="r";
							      break;
							    default:
							//	  _tcscpy(BS,TS);
							      PROCError(2059,TS);
							      break;
							    }
							  if((V->Q==VALUE_IS_COSTANTE) && (R.Q==VALUE_IS_COSTANTE)) {
							    sprintf(MyBuf,"%d %s %d",V->cost->l,BS,R.cost->l);
							    V->cost->l=EVAL(MyBuf);
							    V->Q=VALUE_IS_COSTANTE;
							    }
							  else {
							    _tcscat(V->cost->s,BS); 
							    _tcscat(V->cost->s,R.cost->s);
							    V->Q=VALUE_IS_COSTANTEPLUS;		// 9
							    }
	              }               // fine if costanti...
	            else {
  	            T=0;
	              switch(V->Q) {
	                case VALUE_IS_EXPR:
	                case VALUE_IS_EXPR_FUNC:
		                break;  
	                case VALUE_IS_D0:
										if(!*V->var->name) {		// accade se c'è cast
											V->var->type=V->type;
											V->var->size=V->size;
											}
		                if(OP==6 || OP==7) {
		                  T=-1;
		                  }
		                else
	                    PROCReadD0(outbuf,/**V->var->name ? V->var : R.var*/V->var,VARTYPE_PLAIN_INT,0,0,V->var->parm.ofs,FALSE);	// se è cast, (uso R.var) //  di là size=0... andrebbe sistemato

										V->Q=VALUE_IS_EXPR;
		                break;  
	                case VALUE_IS_COSTANTE:
                      T=MODE_IS_CONSTANT1;
	                  break;
	                case VALUE_IS_COSTANTEPLUS:
myUVcost:	                
		                PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,FALSE);
	                  break;
	                case VALUE_IS_VARIABILE:
                    switch(V->var->classe) {
                      case CLASSE_EXTERN:
                      case CLASSE_GLOBAL:
                      case CLASSE_STATIC:
myUVvar:	                
	                      if((OP==6 || OP==7 || (*cond && OP==8)) && !(V->type & 0xf)) {
													}
												else
		                      ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
	                      break;
	                    case CLASSE_REGISTER:
		                    if(OP==6 || OP==7) {
		                      T=-1;
		                      }
		                    else
	                        goto myUVvar;
												break;   
											case CLASSE_AUTO:
	                      if((OP==6 || OP==7 || (*cond && OP==8)) && !(V->type & 0xf)) {
								          T=-1;           // mi ricordo che la 1° expr è in memoria  
		                      }
		                    else
	                        goto myUVvar;
							          break;  
	                    }
//										V->Q=VALUE_IS_EXPR;
	                  break;  
									case VALUE_IS_PTR:
										if(R.Q==VALUE_IS_VARIABILE) {
											if(R.size<PTR_SIZE) {
												ReadVar(outbuf,R.var,VARTYPE_PLAIN_INT,0,0,FALSE);
												R.Q=VALUE_IS_EXPR;
												}
											}
	                  break;  
	                default:
                    PROCAssignCond(&V->Q,&V->type,&V->size,Clabel);
	                  break;              
	                }
//                ROut=LastOut;
	              if(reg2 && T==0) {
      	          }
      	        reg2=1;
  	            if(T>=0 && (V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || V->Q==VALUE_IS_D0 || (V->Q==VALUE_IS_VARIABILE && OP!=6 && OP!=7)))	// tutte tranne CMP < > == !=
                  {
	            // ...e qui usiamo i registri Increm. per ReadVar e simili...
//meglio sopra                  VQ1=V->Q;
	                }
	              else
	                VQ1=-1;
// VEDERE 2026							if(V->Q==VALUE_IS_PTR)
              //  ecc idem

	              j=(V->size > 4) ? 2 : 1;  


//								if(/*R.Q==VALUE_IS_D0 &&*/ isPtrUsed)//sistemare
	//								Regs->IncP();




	              switch(R.Q) {
	                case VALUE_IS_EXPR:
	                case VALUE_IS_EXPR_FUNC:
									case VALUE_IS_PTR:
	                  break;
	                case VALUE_IS_D0:
//										if(isPtrUsed)
	//										Regs->IncP();
										if(VQ1==VALUE_IS_D0 || VQ1==VALUE_IS_PTR)
										if(FNGetMemSize(V->type,V->size,NULL/*dim*/,1) != FNGetMemSize(R.type,R.size,NULL/*dim*/,1) 
											|| Pty==99) {		// se serve, mi tocca leggere
											PROCReadD0(outbuf,R.var,V->type,V->size,0,R.var->parm.ofs,FALSE);
		//									PROCCast(V->type,V->size,R.type,R.size,-1);
											R.Q=VALUE_IS_EXPR;
											}
										else {
											T=MODE_IS_VARIABLE;
											}
										R.type=R.var->type; R.size=R.var->size;		// sarebbero da unire...
		//								if(isPtrUsed)
			//								Regs->DecP();
										if(VQ1==VALUE_IS_D0 || VQ1==VALUE_IS_PTR)
;
	                  break;
	                case VALUE_IS_COSTANTE:
                    T=MODE_IS_CONSTANT2;
	                  break;
	                case VALUE_IS_COSTANTEPLUS:
myURcost:	 
	                  PROCUseCost(outbuf,R.Q,R.type,R.size,R.cost,FALSE);
	                  break;
	                case VALUE_IS_VARIABILE:
	                  switch(R.var->classe) {
	                    case CLASSE_EXTERN:
                      case CLASSE_GLOBAL:
                      case CLASSE_STATIC:
		                    if(T==0 && (OP != 5)) {
//	                      	T=-1;
		                      }
	                      else 
	 	                      ReadVar(outbuf,R.var,VARTYPE_PLAIN_INT,0,0,FALSE);
      									break;
	                    case CLASSE_REGISTER:
//		                    T=-1;  
	                      break;
											case CLASSE_AUTO:
												if(T==0 && (OP != 5) && FNGetMemSize(V->type,V->size,NULL/*dim*/,1) == FNGetMemSize(R.type,R.size,NULL/*dim*/,1)) {
//	                      	T=-1;
		                      }
	                      else 
	 	                      ReadVar(outbuf,R.var,VARTYPE_PLAIN_INT,0,0,FALSE);
												break;
	                    }
	                  break;  
	                default:
	                  PROCAssignCond(&R.Q,&R.type,&R.size,Rlabel);
	                  break;
                  }
//                if(R.var >= 0) {
//                  PROCCast(V->type,V->size,R.type,R.size);
//                  }
                    PROCOper(LINE_TYPE_ISTRUZIONE,"mov");
//	              if(T==-1) {
//	                T=0;
//	                }
							  if(!(V->type & VARTYPE_FLOAT) && (R.type & VARTYPE_FLOAT)) {
									struct VARS *v;
									v=FNCercaVar("_fcvti",0);
  								if(!v)
						   		  v=PROCAllocFunzProto("_fcvti",VARTYPE_FUNC_USED | VARTYPE_FLOAT,V->size);	
									if(R.Q==VALUE_IS_VARIABILE)
		                ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,FALSE);		// FINIRE
									else if(R.Q==VALUE_IS_COSTANTE)
								    PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,FALSE);		// FINIRE
									else if(R.Q==VALUE_IS_D0)
                    PROCReadD0(outbuf,R.var,0,0,0,0,FALSE);
      						PROCOper(LINE_TYPE_CALL,v);
									}

//								if(R.Q==VALUE_IS_D0 && isPtrUsed)//sistemare
	//								Regs->DecP();

                switch(OP) {
                  case 3:
//				            *cond=0;
										if(Optimize & OPTIMIZE_CONST && *TS!='%' && R.Q==VALUE_IS_COSTANTE && (i=FNIsPower2(R.cost->l))) {			// ottimizzo potenze di 2!
											R.cost->l=i;
	//										PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											}
										else if(Optimize & OPTIMIZE_CONST && *TS=='%' && R.Q==VALUE_IS_COSTANTE && (i=FNIsPower2(R.cost->l))) {			// ottimizzo potenze di 2!
											R.cost->l = R.cost->l-1;
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											}
										else {
											if(!(R.Q & VALUE_IS_COSTANTE)) {
												if(R.Q == VALUE_IS_VARIABILE)
													PROCCast(V->type,V->size,&R.type,&R.size,
														-1);		// cast implicito tra operandi!in effetti gemini dice di castare al tipo + grande...
												else
													PROCCast(V->type,V->size,&R.type,&R.size,-1);		// cast implicito tra operandi!  in effetti gemini dice di castare al tipo + grande...
												}
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											}
                    break;
                  case 4:
//				            *cond=0;
										if(!(R.Q & VALUE_IS_COSTANTE)) {
												if(R.Q == VALUE_IS_VARIABILE)
													PROCCast(V->type,V->size,&R.type,&R.size,-1
														);		// cast implicito tra operandi!in effetti gemini dice di castare al tipo + grande...
												else
													PROCCast(V->type,V->size,&R.type,&R.size,-1);		// cast implicito tra operandi!  in effetti gemini dice di castare al tipo + grande...
											}
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
			              break;
		              case 5:
//				            *cond=0;
										// qua direi che il cast non serve
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
		                break;
                  case 6:
                  case 7:
//				            *cond=0;
										if(!(R.Q & VALUE_IS_COSTANTE)) {
												if(R.Q == VALUE_IS_VARIABILE)
													PROCCast(V->type,V->size,&R.type,&R.size,-1
														);		// cast implicito tra operandi!in effetti gemini dice di castare al tipo + grande...
												else
													PROCCast(V->type,V->size,&R.type,&R.size,-1);		// cast implicito tra operandi!  in effetti gemini dice di castare al tipo + grande...
											}

										if(T==-1)			// bah arriva così...ma uniformizzo 2026
											T=MODE_IS_VARIABLE;
										// qua possiamo fare CMP diretto con le variabili, v.sopra, e finire anche con il secondo
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
                    break;
                  case 8:
                  case 9:
                  case 10:
//				            *cond=0;
										if(!(R.Q & VALUE_IS_COSTANTE)) {
												if(R.Q == VALUE_IS_VARIABILE)
													PROCCast(V->type,V->size,&R.type,&R.size,-1
														);		// cast implicito tra operandi!in effetti gemini dice di castare al tipo + grande...
												else
													PROCCast(V->type,V->size,&R.type,&R.size,-1);		// cast implicito tra operandi!  in effetti gemini dice di castare al tipo + grande...
											}
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
                    break;
                  }
  	            if(/*T>=0 &&*/ (VQ1==VALUE_IS_PTR || VQ1==VALUE_IS_EXPR || VQ1==VALUE_IS_EXPR_FUNC || VQ1==VALUE_IS_D0 || VQ1==VALUE_IS_VARIABILE))
                  {
                  }
//   		          V->Q=VALUE_IS_EXPR;// in pratica inutile qua, serve nel chiamante...
								// e OCCHIO si fotte la condizione! ev. gestire



                }
              break;

            case 11:
            case 12:
							if(!Co /*isWhat==2*/)
								PROCError(2059,TS);
if(debug)
myLog->print(0,"OP logico %u (%u): entro al livello %d con %x (cond è %x), Brack %u, Co %u",OP,oOP,Pty,V->Q,*cond,Brack,Co);


              T=1;
							if(!(V->Q & ~0xf)) {
								if(V->Q==VALUE_IS_D0) {
							    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,0,FALSE);
									}
					      else if(V->Q==VALUE_IS_VARIABILE) {
							    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,FALSE);
									}
								else if(V->Q & VALUE_IS_COSTANTE) {   // auto ottim. costanti (forza T=0)
									PROCWarn(4127,V->cost->s);
									if(Optimize & OPTIMIZE_CONST) {
										if((V->Q & 0xf) ==VALUE_IS_COSTANTE) {
											if(V->cost->l) {
												if(OP==12) {
	//												if(Brack)
													skipExpr(12,isRValue ? ';' : ')');
	//					              T1=FNRev(OP-1,cond,Rlabel,&R);
													T=0;		// ottimizzato, 1 fisso
													V->cost->l=1;		// C89 99
													}
												else 
													T=2;    // serve calcolo
												}
											else {
												if(OP==11) {
													T=0;		// ottimizzato, 0 fisso
													V->cost->l=0;		// C89 99
													}
												else 
													T=2;		// serve calcolo
												}
											}
										else {
											if(OP==12)
												;	//T=0;		// ?? come se non ci fosse?
											else 
												T=2;
											}
										}
									}
								}

							if(!(V->type & VARTYPE_FLOAT) && (R.type & VARTYPE_FLOAT)) {
								struct VARS *v;
								v=FNCercaVar("_fcvti",0);
								/* ovvero da gemini 2026
								; Esempio codegen per: if (f)
MOV.d   R0, [R24-4]      ; Carica l'immagine a 32-bit del float
AND.d   R0, 0x7FFFFFFF   ; Maschera via il bit del segno (gestisce +0.0 e -0.0)
CMP.d   R0, 0
BEQ     L_FALSE          ; Se 0, il float è 0.0 -> FALSO*/
								/*; Esempio per double su due registri (R1 = alto, R0 = basso)
MOV.d   R1, [R24-4]      ; Parte alta (contiene il segno)
MOV.d   R0, [R24-8]      ; Parte bassa
AND.d   R1, 0x7FFFFFFF   ; Pulisce il bit di segno dal registro alto
OR.d    R1, R0           ; Unisce tutti i bit di mantissa ed esponente
CMP.d   R1, 0            ; Se R1 == 0, l'intero double era 0.0 o -0.0!
BEQ     L_FALSE*/
  							if(!v)
					   		  v=PROCAllocFunzProto("_fcvti",VARTYPE_FUNC_USED | VARTYPE_FLOAT,V->size);
								if(R.Q==VALUE_IS_VARIABILE) {
	                ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,*cond & VALUE_CONDITION_MASK,FALSE);
									}
								else if(R.Q==VALUE_IS_COSTANTE) {
							    PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,FALSE);
									}
								else if(R.Q==VALUE_IS_D0)
                  PROCReadD0(outbuf,R.var,0,0,0,0,FALSE);
      					PROCOper(LINE_TYPE_CALL,v);
								}

              T1=1;

              if(OP==11  /*&& Pty==12 e le parentesi? */ /* && Co==1*/) {
// 		              swap(&ROut,&LastOut);
                FNGetLabel(Rlabel,2);
								
								
                PROCOutLab(Rlabel);
								Clabel[7]='F'; Clabel[8]=0;
// 		              swap(&ROut,&LastOut);
								if(debug)
									myLog->print(0," label (da op=11 ) %s; V->Q=%x",Clabel,V->Q);
								}
              if(OP==12 /*&& Pty==12 e le parentesi? */ && Co==1) {
								if(debug)
									myLog->print(0," label (da op=11 ) %s; V->Q=%x",Clabel,V->Q);
                FNGetLabel(Rlabel,2);
                PROCOutLab(Rlabel);
								T1=0;
								}

  	          if(T) {


  	            if((/*!*cond ||*/(oOP && oOP!=12) && (OP==12)) && (V->Q & 0xf)) {
    		          PROCOutLab(Clabel);
									if(debug)
										myLog->print(0," label (da op) %s",Clabel);
    		          }



								if(OP==11) {
//									_tcscpy(LastOut->prev->s2.s.label,origLabel);
// 		              swap(&ROut,&LastOut->prev);
									}
    		        _tcscpy(Rlabel,Clabel);
    		        *cond |= 1;
	              T1=FNRev(outbuf,OP-1,cond,Rlabel,&R);
								if(debug)
									myLog->print(0,"oOP era %d, Dopo il logico c'è %d",oOP,T1);
	              if((T1 == 12 &&    oOP!=12) || ((!(V->Q & VALUE_HAS_CONDITION)) && (/* !*cond || */ OP==/*==*/12)))		// solo se ||
	                Clabel[7]='T';	// FNGetLabel(Clabel,2,1);
	              if(T==1) {
                  V->Q &= ~VALUE_HAS_CONDITION;


									if(R.Q==VALUE_IS_COSTANTE) {		// se c'è una costante...
										if(OP==11) {		// se &&
											if(!R.cost->l) {		// e costante = 0
			                  PROCOper(LINE_TYPE_ISTRUZIONE,"CLR.d");
			                  PROCOper(LINE_TYPE_ISTRUZIONE,"jr",Clabel);
//												RQ=VALUE_HAS_CONDITION;
//												PROCOutLab(Clabel);
												goto skippa_condbranch;
												}
											}
										else {		// se ||
											if(R.cost->l) {		// e costante != 0
			                  PROCOper(LINE_TYPE_ISTRUZIONE,"mov");// fisso C89 C99 dice
			                  PROCOper(LINE_TYPE_ISTRUZIONE,"jr",Clabel);
												R.Q=VALUE_HAS_CONDITION;
//												PROCOutLab(Clabel);
												goto skippa_condbranch;
												}
											}
										}
									if(debug)
										myLog->print(0,"GenCondBranch %s; OP=%u, V->Q=%x, oOP=%u; Brack=%u, Co=%d",Clabel,OP,V->Q,oOP,Brack,Co);
		              PROCGenCondBranch(Clabel,(OP==11),&V->Q,FNGetMemSize(V->type,V->size,NULL/*dim*/,0));
skippa_condbranch: ;



	                }

/*	              if(RQ==3) {
	                ReadVar(R.var,VARTYPE_PLAIN_INT,0,*cond & 0xff);
//	                RQ=0x85;
	                }
	              if(RQ & 8) {
	                if(RQ==8) {
	                  if(R.cost.l) {
	                    if(OP==12)
	                      T1=0;
	                    else 
	                      T1=2;
	                    }
	                  else {
	                    if(OP==11)
	                      T1=0;
	                    else 
	                      T1=2;
	                    }
	                  }
	                else {
	                  if(OP==12)
	                    T1=0;
	                  else
	                    T1=2;
	                  }
//                  RQ=0x85;
	                }
	                */
//	              if(RQ & VALUE_IS_CONDITION_VALUE) {
	                V->Q=R.Q;
// ovviamente sbagliato, 2025	                V->var=R.var;
									if(!(R.Q & ~0xf)) {
										if(R.Q == VALUE_IS_VARIABILE && R.var && V->Q == VALUE_IS_VARIABILE && V->var)		// V->var è NULL se siamo al livello esterno, in quel caso dobbiam dare errore se ci sono variabili (v. GetAritmElem errore 2099
											*V->var=*R.var;
										if(R.Q == VALUE_IS_COSTANTE && R.cost->l)
											R.cost->l=1;		// fisso C89 C99 dice
										}
//									V->var->size=R.var->size;
//									V->var->type=R.var->type;
//memcpy(V->var,R.var,sizeof(struct VARS)-4);

	                V->size=R.size;
	                V->type=R.type;
									V->cost=R.cost;
// ev. flag, dim struct e tutto quanto...

//	                myLog->print(0,"RQ vale %x\n",RQ);
//	                }
	              if(!T1)
	 	              V->Q=0;
  	            }
	            if((/*!*cond ||*/ (oOP==12 || OP==12)) 
								&& (V->Q && ((V->Q & VALUE_IS_CONDITION) || ((V->Q & 0xf) != VALUE_IS_COSTANTE)))) {
	              V->Q |= VALUE_HAS_CONDITION;
  		          }
//	            else
//	              V->Q=0;

if(debug)
myLog->print(0,"OP logico %u (%u): esco con %x, %x, Brack %u, Co %u\a",OP,oOP,V->Q,*cond,Brack,Co);
              oOP=OP;
							if(Clabel[6])
								Clabel[7]='T';		// 
	            break;

            case 13:
							{int16_t cond2;
							struct OPERAND R2;
							struct VARS RPtr2;
							union STR_LONG RCost2;
							char Clabel2[sizeof(STR_LONG)]={0};
							ZeroMemory(&R2,sizeof(struct OPERAND));
							R2.cost=&RCost2;
							R2.var=&RPtr2;
							if(!Co /*isWhat==2*/)
								PROCError(2059,TS);

							if(!(V->Q & VALUE_IS_CONDITION)) {
							if(V->Q==VALUE_IS_VARIABILE) {
		            ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,1,FALSE);
								}
							else if(V->Q==VALUE_IS_D0) {
						    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,1,0,FALSE);
								}
		          else if(V->Q & VALUE_IS_COSTANTE) {      // boh autoottimizza cost..
								if(V->Q == VALUE_IS_COSTANTE) {      // 
									if(!V->cost->l) {
										skipExpr(13,':');
					          PROCCheck(':');
					          subEvEx(outbuf,13,cond,Clabel,&R);
										}
									else {
//					          subEvEx(13,&cond2,Clabel2,&R2);		// butto via!
					          subEvEx(outbuf,13,cond,Clabel,&R);
					          PROCCheck(':');
										skipExpr(13,';');
										}
									if(R.Q==VALUE_IS_COSTANTE) {
										V->type=R.type;          // siccome ottimizziamo, solo una vincerà! (andrebbero usate entrambe le expr
										V->size=R.size;
										V->var=NULL;
										V->cost=R.cost;
										V->Q=VALUE_IS_COSTANTE;
										break;
										}
									}
								else {
									PROCUseCost(outbuf,V->Q,V->type,V->size,V->cost,FALSE);
									}
						    PROCWarn(4127);
/*
		            if((*V->Q==VALUE_IS_COSTANTE) && (!&V->cost.l)) {
		              *AS=0;
		              T=0;
		              while((*AS != ':') && (!T)) {
		                FNLO(AS);
		                if(*AS=='(')
		                  T++;
		                if(*AS==')')
		                  T--;                     
		                }
		              }
		            subEvEx(13,0,V->type,V->size,V->Q,V->var,&V->cost,V->isInTag,V.dim);
		            */
		            }
							}		// IS_CONDITION
							cond2=*cond;
							*cond=0;
		          FNGetLabel(TS,2);
		          PROCGenCondBranch(TS,TRUE,&V->Q,FNGetMemSize(V->type,V->size,NULL/*dim*/,0));
//		          *cond=0;
		          subEvEx(outbuf,13,cond,Clabel,&R);
							_tcscpy(MyBuf,TS);
							_tcscat(MyBuf,"_");
							PROCOper(LINE_TYPE_JUMP,"jr",MyBuf);
		          PROCOutLab(TS);
		          PROCCheck(':');
		          subEvEx(outbuf,13,cond,Clabel2,&R2);
							*cond=cond2;
							if(R.type & VARTYPE_IS_POINTER || R2.type & VARTYPE_IS_POINTER)
								V->type = (R.type & VARTYPE_IS_POINTER) | (R2.type & VARTYPE_IS_POINTER);
							else if(R.type & VARTYPE_FLOAT || R2.type & VARTYPE_FLOAT)
								V->type = R.type | VARTYPE_FLOAT;
		          else
								V->type=V->type;          // :) 
							if(R.type & VARTYPE_UNSIGNED || R2.type & VARTYPE_UNSIGNED)
								V->type |= VARTYPE_UNSIGNED;
		          V->size=max(R.size,R2.size);
							PROCCast(V->type,V->size,&R.type,&R.size,-1);
							PROCCast(V->type,V->size,&R2.type,&R2.size,-1);
							}
		          PROCOutLab(TS,"_",NULL);
		          V->var=NULL;
		          V->Q=VALUE_IS_EXPR;
		          break;

		        case 14:
							isRValue++;
							if(isPtrUsed) {
								isPtrUsed++;		// qua forse no perché arrivo da lvalue...
								}

							T=0;

//							if(isWhat==2)
//								PROCError(2059,TS);
              if(Pty<14 || (V->Q & VALUE_IS_COSTANTE))           // bloccare le expr e cost a sinistra (a+3=b)
	              PROCError(2106);
		          if(V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || *TS != '=') {
//		            PROCOut("; fine =",NULL,NULL,NULL,NULL);
//		            swap(&ROut,&LastOut);
		            }
							else if(V->Q==VALUE_IS_PTR) {
								// RegsP ?
								}
		          else if(V->Q==VALUE_IS_D0) {     // separato da V->Q=1, per il ptr *
//		            PROCOut("; fine =",NULL,NULL,NULL,NULL);
//		            swap(&ROut,&LastOut);
								if(V->var->modif) {    // se devo sommare un ofs reg, lo faccio ora
//									subOfsD0(V->var,V->var->size,V->var->func.value,V->var->parm.ofs);
									V->var->modif=0;
									V->var->parm.ptr=NULL;
									}
		            }
//		          *cond=0;
		          if(FNRev(outbuf,14,cond,Rlabel,&R) < 0)
								/*PROCError(2059) no... finire*/;
		          if((R.Q==VALUE_IS_VARIABILE && (V->Q != VALUE_IS_VARIABILE || V->var->classe != CLASSE_REGISTER))
								) {   // store in registri a parte...
                ReadVar(outbuf,R.var,V->type,FNGetMemSize(V->type,V->size,NULL/*dim*/,1),0,FALSE);
                }
              else {
								if(V->Q == VALUE_IS_VARIABILE && V->var->classe == CLASSE_REGISTER)		// v.sopra: qua mi serve... forse anche altri
	                T=1;
								else 
									T= R.Q & VALUE_IS_COSTANTE ? 1 : 0;


//								V->Q=R.Q;



								}

		          if(R.Q & (VALUE_IS_CONDITION | VALUE_IS_CONDITION_VALUE)) {
								if(debug)
									myLog->print(0,"ASSIGNCOND: =%d\n\a",R.Q);
		            PROCAssignCond(&R.Q,&R.type,&V->size /*&R.size*/,Rlabel);
								}
			        else if(R.Q==VALUE_IS_COSTANTEPLUS) {             // tratto le costanti int. a parte
	              R.size=V->size;
	              PROCUseCost(outbuf,R.Q,R.type,R.size/*cioè V->size*/,R.cost,FALSE);
		            }
						  else if(R.Q==VALUE_IS_D0) {
//						  myLog->print(0,"Read D0: %lx\n",R.var);
								if(!R.flag) {		// era puntatore e NON array o expr 
						  		PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");	// qua è ok così
//							    PROCReadD0(R.var,V->type,FNGetMemSize(V->type,V->size,NULL/*dim*/,1),0,0,FALSE);
									}
//								else
//							    PROCReadD0(R.var,V->type,FNGetMemSize(V->type,V->size,NULL/*dim*/,1),0,0,FALSE);
								if(FNGetMemSize(V->type,V->size,NULL/*dim*/,1) != FNGetMemSize(R.type,R.size,NULL/*dim*/,1) 
									|| Pty==99) {		// se serve, mi tocca leggere
							    PROCReadD0(outbuf,R.var,V->type,V->size,*cond,R.var->parm.ofs,FALSE);
									R.type=R.var->type; R.size=R.var->size;		// sarebbero da unire...
									R.Q=VALUE_IS_EXPR;
									}
						    }
              else if(R.Q==VALUE_IS_EXPR || R.Q==VALUE_IS_EXPR_FUNC) {
								if(R.size == 0 && !(R.type & VARTYPE_POINTER))		// void function!
									PROCError(2440);
    					  PROCCast(V->type,V->size,&R.type,&R.size,-1);
								}
							else if(R.Q==VALUE_IS_PTR) {
  // hmmm no...  					  PROCCast(V->type,V->size,&R.type,&R.size,Regs->P);		// forzo Regs->P qua... o passare isPtr??

								}
              else if(R.Q==VALUE_IS_COSTANTE) {
								if(Pty==99 && *FNLA(MyBuf) != ',')		// cazzatina per evitare rilettura se segue virgola!
									PROCUseCost(outbuf,R.Q,R.type,R.size/*cioè V->size*/,R.cost,FALSE);
						    }
		          if(V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || *TS!='=') {
//                PROCOut("; inizio =",NULL,NULL,NULL,NULL);
//		            swap(&ROut,&LastOut);
		            }
		          else if(V->Q==VALUE_IS_PTR) {
								}
		          else if(V->Q==VALUE_IS_D0) {
//                PROCOut("; inizio =",NULL,NULL,NULL,NULL);
//		            swap(&ROut,&LastOut);
		            }

							if(isPtrUsed) {
								isPtrUsed--;
//								if(isPtrUsed)		// specialmente se c'è puntatore (non array) a dx dell '='
								}

//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);

		          switch(*TS) {
		            case '=':        // OCCHIO: fa casino se assegno dentro una cond...
				          if((R.type & (VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_CLASS)) && 
										(V->type & (VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_CLASS)) && (!(R.type & VARTYPE_IS_POINTER)) && (!(V->type & VARTYPE_IS_POINTER))) {
				            if(V->tag != R.tag)
				              PROCError(2115);
				            if(V->Q==VALUE_IS_VARIABILE) {
				              if(!(V->var->type & VARTYPE_IS_POINTER) && 
												(V->var->type & (VARTYPE_STRUCT | VARTYPE_UNION | VARTYPE_CLASS | VARTYPE_ARRAY | VARTYPE_FUNC /*0x1d00*/))) 
  				              PROCError(2106);   // dovrebbe bloccare i non lvalue a sinistra
											else {
  				              ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
												}
				              }
				            PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
										// MemoryModel
				            PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.w");
										PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.b");
										//OPDEF_MODE_REGISTRO_INDIRETTO_POSTINC USARE
								    PROCOper(LINE_TYPE_ISTRUZIONE,"DJNZ.w");
				            }
				          else {
				            if(V->size > PTR_SIZE || R.size > PTR_SIZE) {		// OCCHIO memorymodel, CPU varie... PTR_SIZE
				              if((V->size != R.size) && 
												!((V->type | R.type) & (VARTYPE_STRUCT | VARTYPE_UNION | VARTYPE_CLASS | VARTYPE_ARRAY | VARTYPE_IS_POINTER | VARTYPE_FUNC /*0x1d0f*/)))  // integrali di diff. grandezza
				                PROCWarn(4761);
				              }
				            if(V->Q & VALUE_IS_COSTANTE || V->Q==VALUE_IS_EXPR || V->Q==VALUE_IS_EXPR_FUNC || V->Q==VALUE_IS_PTR)
				              PROCError(2106);
				            else if(V->Q==VALUE_IS_VARIABILE) {
				              if(!(V->var->type & VARTYPE_IS_POINTER) && 
												(V->var->type & (VARTYPE_STRUCT | VARTYPE_UNION | VARTYPE_CLASS | VARTYPE_ARRAY | VARTYPE_FUNC /*0x1d00*/))) 
  				              PROCError(2106);
											else {
												switch(T) {
													case 0:				// ho già letto var
														if(R.Q==VALUE_IS_PTR)
															T=VALUE_IS_PTR;
														else if(R.Q==VALUE_IS_D0)
															T=VALUE_IS_D0;
														else if(R.Q==VALUE_IS_EXPR)
															T=VALUE_IS_EXPR;
														else
															T=VALUE_IS_EXPR;
														break;
													case 2:
														break;
													case 1:		// non ho letto var
														/*if(R.Q==VALUE_IS_VARIABILE)
															T=VALUE_IS_VARIABILE;*/

														
														T=R.Q;
														break;
													}
												StoreVar(outbuf,V->var,T,R.var,R.cost,R.var /*&& R.var->isInTag*/ ? R.var->parm.ofs : 0);		// 
												}

											// VERIFICARE perché arriva 2!!

											if(Pty==14)	{		// se si propaga, tipo a=b=c=0 ... lascio costante (utili specie se si usa CLR per scrivere 0
												if(R.Q==VALUE_IS_COSTANTE /*&& !R.cost->l*/)	{		// se si propaga, tipo a=b=c=0 ... lascio costante (utili specie se si usa CLR per scrivere 0
			   									R.Q=VALUE_IS_COSTANTE;
													*V->cost=*R.cost;
													}
												else if(R.Q==VALUE_IS_VARIABILE)	{		// 
			   									R.Q=VALUE_IS_VARIABILE;
//													V->var=R.var;
													}
												}
											else
			    		          R.Q=VALUE_IS_EXPR;

				              }
//				            if(*V->Q==1)
//				              PROCAssign(ROut/*LastOut*/,*V->Q,V->var,V->type,V->size,RQ,&R.cost);
//											PROCError(1001,"store in hl, no ptr");
										else if(V->Q==VALUE_IS_D0) {


//											if(!V->flag)			// v. sopra, gestione array e puntatori
//												PROCOper(LINE_TYPE_ISTRUZIONE,"move.l"/*movString*/,OPDEF_MODE_REGISTRO32,Regs->D,OPDEF_MODE_REGISTRO32,Regs->P);
// da togliere in alcuni casi!!


//											PROCStoreD0(V->var,R.Q,R.var,R.cost,isPtrUsed ? TRUE : FALSE);
											if(V->var->type & VARTYPE_BITFIELD) {
												i=FNGetAggr2(V->var,V->var,&reg2,&j);
												if(R.Q==VALUE_IS_COSTANTE) {
													if(reg2 > 0)
														R.cost->l <<= reg2;
													reg2=0;
													if(i)		// anche signed :)
							  						PROCOper(LINE_TYPE_ISTRUZIONE,"AND.d");	
							  					PROCOper(LINE_TYPE_ISTRUZIONE,"OR.d");	// 

													}
												else {
											    PROCReadD0(outbuf,V->var,0,0,0,0,FALSE);
													if(reg2 > 0) {
														PROCOper(LINE_TYPE_ISTRUZIONE,"SLA.d"
															);
														}
													if(i)		// anche signed :)
							  						PROCOper(LINE_TYPE_ISTRUZIONE,"AND.d");
						  						PROCOper(LINE_TYPE_ISTRUZIONE,"OR.d");	
													// manca sign-extend, dice gemini 2026
													}
												R.Q=VALUE_IS_EXPR;
												PROCStoreD0(outbuf,V->var,R.Q,R.var,R.cost,R.var ? R.var->parm.ofs : 0);
												}                
											else {
												if(!V->flag) {
						  						PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");	
													}
												switch(T) {
													case 0:				// ho già letto var
														if(R.Q==VALUE_IS_PTR)
															T=VALUE_IS_PTR;
														else if(R.Q==VALUE_IS_D0)
															T=VALUE_IS_D0;
														else if(R.Q==VALUE_IS_EXPR)
															T=VALUE_IS_EXPR;
														else
															T=VALUE_IS_EXPR;
														break;
													case 2:
														break;
													case 1:		// non ho letto var
														/*if(R.Q==VALUE_IS_VARIABILE)
															T=VALUE_IS_VARIABILE;
														if(R.Q==VALUE_IS_PTR)
															T=VALUE_IS_PTR;
														if(R.Q==VALUE_IS_D0)
															T=VALUE_IS_D0;
														if(R.Q==VALUE_IS_EXPR)
															T=VALUE_IS_EXPR;*/


														T=R.Q;
														break;
													}
												PROCStoreD0(outbuf,V->var,T,R.var,R.cost,R.var ? R.var->parm.ofs : 0);
												}
		    		          R.Q=VALUE_IS_EXPR;
											}
				            }
			            break;

		            case '+':
		            case '-':
		              if(T) {
										switch(R.var->classe) {
	                    case CLASSE_EXTERN:
                      case CLASSE_GLOBAL:
                      case CLASSE_STATIC:
      									break;
	                    case CLASSE_REGISTER:
	                      break;
											case CLASSE_AUTO:
												break;
											}
                    }
									else {
										}

                  if(R.Q==VALUE_IS_COSTANTE) {
                    j=2;
										}
                  else {
                    j=0;
                    if(R.Q==VALUE_IS_COSTANTEPLUS) {
 											PROCUseCost(outbuf,R.Q,R.type,R.size,R.cost,FALSE);
											}
										else if(R.Q==VALUE_IS_D0) {
											if(R.var->size) {			// v. case 0 in readD0, casi con costante
									    PROCReadD0(outbuf,R.var,0,0,0,R.var->parm.ofs,FALSE);
											}
											}
										else if(R.Q==VALUE_IS_EXPR) {
											if(R.var->size) {			// v. case 0 in readD0, casi con costante
// ovviamente no, 2026 - vedere altri!									    PROCReadD0(R.var,0,0,0,0,FALSE);
											}
											}
										else if(R.Q==VALUE_IS_PTR) {
											}
                    }
//già fatto in subAdd									if(!(R.Q & VALUE_IS_COSTANTE))
//										PROCCast(V->type,V->size,R.type,R.size,
//											R.var->classe==CLASSE_REGISTER ? MAKEPTRREG(R.var->label): -1);		// cast implicito tra operandi!
									if(FNGetMemSize(V->type,V->size,NULL/*dim*/,1) < FNGetMemSize(R.type,R.size,NULL/*dim*/,1))
										PROCWarn(4305);		// finire, completare
									// 4047 è in storevar/stored0
			            switch(V->Q) {
										case VALUE_IS_VARIABILE:
											switch(V->var->classe) {
												case CLASSE_AUTO:
													goto my_add;
												case CLASSE_REGISTER:
	//				                  u[0].s=0;
my_add:
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
													if(Pty==14) {             // ritorna expr in hl/d0 se serve
														ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
														}
													break;
												case CLASSE_EXTERN:
												case CLASSE_GLOBAL:
												case CLASSE_STATIC:
													// INVERTITI Dr e Dr1 (e h)
//							          if(RQ != VALUE_IS_COSTANTE)
//							            i=Regs->Inc(FNGetMemSize(V->type,V->size,0));

//							          if(RQ != VALUE_IS_COSTANTE)
//  			                  *V->Q=subAdd(*TS=='+',j,*V->Q,V->var,V->type,V->size,R.type,R.size,&V->cost,&R.cost,sRegs[1].Dr,sRegs[0].Dr,sRegs[1].Drh,sRegs[0].Drh,sRegs[3].Dr,sRegs[2].Dr,sRegs[3].Drh,sRegs[2].Drh);
//							          else
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
													if(Pty==14) {             // ritorna expr in hl/d0 se serve
														ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
														}

		

//							          if(RQ != 8) {
//  						            if(!i)
//  						              Regs->Dec(FNGetMemSize(V->type,V->size,0));
//  						            }  
												  break;
												}
											StoreVar(outbuf,V->var,R.Q,R.var,R.cost,0);
											break;
				            case VALUE_IS_D0:        // non va se (de) o (bc)...
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											if(Pty==14) {              // ritorna expr in hl se serve
										    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,0,FALSE);
												}
		    		          R.Q=VALUE_IS_EXPR;
				              break;
										case VALUE_IS_EXPR:
										case VALUE_IS_EXPR_FUNC:
											// nulla da fare ??, v.68000 sopra +=
											break;
										case VALUE_IS_PTR:
											break;
										default:
											PROCError(1001,"+=");
											break;
			              }
			            break;

		            case '*':
		            case '/':
		            case '%':
		              if(T) {
										switch(R.var->classe) {
											case CLASSE_AUTO:
												break;
											case CLASSE_REGISTER:
												break;
											case CLASSE_EXTERN:
											case CLASSE_GLOBAL:
											case CLASSE_STATIC:
												break;
											}//		                j=FNGetMemSize(V->type,V->size,1)>2 ? 2 : 1;
                    }
									else {
										}
			            switch(V->Q) {
			              case VALUE_IS_VARIABILE:
//											ReadVar(V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
											switch(V->var->classe) {
												case CLASSE_AUTO:
													break;
												case CLASSE_REGISTER:
													break;
												case CLASSE_EXTERN:
												case CLASSE_GLOBAL:
												case CLASSE_STATIC:
													break;
												}

//											if(V->size >4)		// beh completare! verificare
											if(R.Q & VALUE_IS_COSTANTE) {
												if(Optimize & OPTIMIZE_CONST && (i=FNIsPower2(R.cost->l))) {			// ottimizzo potenze di 2!
													if(*TS=='*' || *TS=='/') {
														R.cost->l=i;
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
														}
													else 
														j=2;
													}
												else
													j=2;
	// 	                    PROCUseCost(RQ,R.type,R.size,&R.cost);
												}
											else if(R.Q==VALUE_IS_D0) {
												if(R.var->size) {			// v. case 0 in readD0, casi con costante
											  PROCReadD0(outbuf,R.var,0,0,0,R.var->parm.ofs,FALSE);
												}
												j=0;
												}
											else {
//												ReadVar(R.var,V->type,FNGetMemSize(V->type,V->size,NULL/*dim*/,1),0,FALSE);
												j=0;
												// qua dovremmo sempre essere a posto...
												PROCCast(V->type,V->size,&R.type,&R.size,
													-1);		// cast implicito tra operandi!
												}
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											// caso unico: prima DEC di store
//											if(V->size >4)		// beh completare!
											StoreVar(outbuf,V->var,R.Q,R.var,R.cost,0);
											break;
										case VALUE_IS_D0:  		                   // non finito...
											if(R.Q & VALUE_IS_COSTANTE) {
												j=2;
	// 	                    PROCUseCost(RQ,R.type,R.size,&R.cost);
												}
											else if(R.Q==VALUE_IS_D0) {
												if(R.var->size) {			// v. case 0 in readD0, casi con costante
										    PROCReadD0(outbuf,R.var,0,0,0,R.var->parm.ofs,FALSE);
												}
												}
											else {
			                  ReadVar(outbuf,R.var,V->type,FNGetMemSize(V->type,V->size,NULL/*dim*/,1),0,FALSE);		// FINIRE
												j=0;
												}
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											PROCStoreD0(outbuf,V->var,V->Q,V->var,V->cost,0);
		    		          R.Q=VALUE_IS_EXPR;
											break;
										default:  
											PROCError(1001,"*=");
											break;
										}
									break;

								case '<':
		            case '>':
		              if(T) {
//		                j=FNGetMemSize(V->type,V->size,1)>2 ? 2 : 1;
										switch(R.var->classe) {
											case CLASSE_AUTO:
												break;
											case CLASSE_REGISTER:
												break;
											case CLASSE_EXTERN:
											case CLASSE_GLOBAL:
											case CLASSE_STATIC:
												break;
											}//		                j=FNGetMemSize(V->type,V->size,1)>2 ? 2 : 1;
                    }
									else {
										}

                  if(R.Q==VALUE_IS_COSTANTE)
                    j=2;
                  else {
                    j=0;
                    if(R.Q==VALUE_IS_COSTANTEPLUS) {
   	                  PROCUseCost(outbuf,R.Q,R.type,R.size,R.cost,FALSE);
											}
                    }
			            switch(V->Q) {
			              case VALUE_IS_VARIABILE:
			              switch(V->var->classe) {
			                case CLASSE_AUTO:
												// SOLO se lo shift è 1, costante, e la var 16bit, si può fare DIRETTAMENTE operazione
												if(j==2 && V->size==2 && R.cost->l==1) {
													}
												else
			                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
    		                if(FNGetMemSize(V->type,V->size,NULL/*dim*/,1) > 2) {
//				                  u[1].s=0;
		                      }
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
												if(Pty==14) {              // ritorna expr in hl se serve
			                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
													}
												// in teoria se lo shift è 1, costante, e la var 16bit, si può fare DIRETTAMENTE operazione
//												if(j==2 && V->size==2 && R.cost->l==1) {
//													}
//												else
//													StoreVar(V->var,R.Q,R.var,R.cost,FALSE,FALSE);
												// TUTTO FATTO :)
			                  break;
			                case CLASSE_REGISTER:                // anche register
//							          if(RQ != 8)
//							            i=Regs->Inc(FNGetMemSize(V->type,V->size,0));
		                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
			                  break;
			                case CLASSE_EXTERN:
			                case CLASSE_GLOBAL:
			                case CLASSE_STATIC:                // (was anche register
//							          if(RQ != 8)
//							            i=Regs->Inc(FNGetMemSize(V->type,V->size,0));

//							          if(RQ != 8) {
			                  // passo INVERTITI Dr e Dr1 (e h)
//  			                  *V->Q=subShift(*TS=='<',j,V->type,V->size,R.type,&V->cost,&R.cost,sRegs[1].Dr,sRegs[0].Dr,sRegs[1].Drh,sRegs[0].Drh,sRegs[3].Dr,sRegs[2].Dr,sRegs[3].Drh,sRegs[2].Drh);
//  			                  }
//  			                else  
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
			                  StoreVar(outbuf,V->var,R.Q,R.var,R.cost,0);
//							          if(RQ != 8) {
//  						            if(!i)
//  						              Regs->Dec(FNGetMemSize(V->type,V->size,0));
//  						            }  
			                  break;
			                }
			              break;
			            case VALUE_IS_D0:  		                   // non va se (de) o (bc)...
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
										if(Pty==14) {             // ritorna expr in hl se serve
									    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,0,FALSE);
											}
	    		          R.Q=VALUE_IS_EXPR;
									  break;  
			            default:  
			              PROCError(1001,"<<=");
			              break;
			              }
			            break;

		            case '&':
		            case '|':
		            case '^':
		              if(T) {
//		                j=FNGetMemSize(V->type,V->size,1)>2 ? 2 : 1;
										switch(R.var->classe) {
											case CLASSE_AUTO:
												break;
											case CLASSE_REGISTER:
												break;
											case CLASSE_EXTERN:
											case CLASSE_GLOBAL:
											case CLASSE_STATIC:
												break;
											}//		                j=FNGetMemSize(V->type,V->size,1)>2 ? 2 : 1;
                    }
									else {
										}

                  if(R.Q==VALUE_IS_COSTANTE)
                    j=2;
                  else {
                    j=0;
                    if(R.Q==VALUE_IS_COSTANTEPLUS) {
   	                  PROCUseCost(outbuf,R.Q,R.type,R.size,R.cost,FALSE);
											}
										else if(R.Q==VALUE_IS_D0) {
											if(R.var->size) {			// v. case 0 in readD0, casi con costante
										  PROCReadD0(outbuf,R.var,0,0,0,R.var->parm.ofs,FALSE);
											}
											}
										PROCCast(V->type,V->size,&R.type,&R.size,
											-1);		// cast implicito tra operandi!
                    }
			            switch(V->Q) {
			              case VALUE_IS_VARIABILE:
											switch(V->var->classe) {
												int16_t i2;
												case CLASSE_AUTO:
													goto my_aox;
												case CLASSE_REGISTER:
	//				                  u[0].s=0;
my_aox:
//													i2=0;		== era al posto di *cond...
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
													if(Pty==14) {             // ritorna expr in hl se serve
				                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);		// FINIRE
														}
													break;
												default:
													// passo INVERTITI Dr e Dr1 (e h)
//							          if(RQ != 8)
//							            i=Regs->Inc(FNGetMemSize(V->type,V->size,0));
				                    ReadVar(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,FALSE);		// FINIRE
													i2=0;// era al posto di cond...??
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
													StoreVar(outbuf,V->var,R.Q,R.var,R.cost,0/*(uint16_t)R.var->parm*/);
//							          if(RQ != 8) {
//  						            if(i)
//  						              Regs->Dec(FNGetMemSize(V->type,V->size,0));
//  						            }  
				                  break;
				                }
				              break;
										case VALUE_IS_D0:
											{int16_t i2=v;	//??? era al posto di cond
//	                		subOfsD0(V->var,V->var->size,V->var->func.value,V->var->parm.ofs);	
//											PROCOper(LINE_TYPE_ISTRUZIONE,FNIsOp(TS,Co),TS);
											if(Pty==14) {              // ritorna expr in hl se serve
										    PROCReadD0(outbuf,V->var,VARTYPE_PLAIN_INT,0,0,0,FALSE);			// SERVE?? verificare 7/11/25
												}
											}
		    		          R.Q=VALUE_IS_EXPR;
											break;  
										default:  
											PROCError(1001,"<<=");
											break;
										}
			            break;
			          }

							if(isPtrUsed>0) {
								isPtrUsed--;
								}

							if(V->type & VARTYPE_VOLATILE) {		// sarà da fare :) rifinire
								switch(V->Q) {
									case VALUE_IS_VARIABILE:
			              ReadVar(outbuf,V->var,V->var->type,V->var->size,0,FALSE);
										break;
									case VALUE_IS_D0:
								    PROCReadD0(outbuf,V->var,0,0,0,0,FALSE);
										break;

									}
								}

							V->Q=R.Q;		// DOPO per poter fare volatile!


		          break;
		        case 15:
//		          subEvEx(14,cond,&R.type,&R.size,&RQ,&RVar,R.cost,&RTag,&RDim);     inutile
		          break;
		        case 16:
		          break;
		        default:
		          break;
		        }

	        }		//  ARITM_IS_OPERANDO
	      break;
			default:
				Exit=TRUE;		// basterebbe uno dei due...
				goto fine;
				break;
	    }
    Co++;
    } while(!Exit);

//__line__=ol;

  if(!AR)
    return AR-1;
  else 
  	return OP;
fine:
	return -1;
  }  
  
char *CPlusMinus::ConRecEval(char *s, uint8_t Pty, long *l1) {
  long l2;
  char ch;
  int Times=0;
	uint8_t InBrack=0;
	bool Go=0,fError=0;
	char firstchar=0;
  char *p;
	int8_t isWhat=0;		// 0 inizio, 1=value, 2=operand; FORSE dovrebbe essere statica... gestendo liv. ricorsione, ma pare cmq andare (v. anche FNRev
  int i,j;
  
  do {
	  ch=*s;
		if(iscsym(ch) && !firstchar)
			firstchar=ch;
/*		if(!firstchar) {
			if(ch==')) {		// se inizia con operando...
				PROCError(2059,s);
				Go=TRUE;
				}
			}*/

//	  myLog->print(0,"sono sul %c(%x), T %d, pty %d\n",ch,ch,Times,Pty);
	  switch(ch) {
	    case '(':
	      s++;
	      InBrack++;
	      break;
	    case ')':
	      if(InBrack) {
  	      s++;
	        InBrack--;
	        }
	      else
	        Go=1;
	      break;
	    case '+':
	    case '-':
//			      myLog->print(0,"- ??unario: f %x,l %ld, pty %d, times %d\n",*f1,*l1,Pty,Times);
	      if(Times) {
					if(isWhat==1) {
  					if(Pty > 4) {
							s++;
							isWhat=2;
							s=ConRecEval(s,4,&l2);
							if(ch=='+')
								*l1=*l1+l2;
							else
								*l1=*l1-l2;
							}
						else
							Go=1;
						}
					else {
						PROCError(2059,s);
						Go=1;
						}
	        }
	      else {
					if(isWhat==1) {
  					if(Pty > 2) {
							s++;
							isWhat=2;
							s=ConRecEval(s,2,l1);
							if(ch=='-')
								*l1=-*l1;
							}
						else
							Go=1;
						}
					else {
						PROCError(2059,s);
						Go=1;
						}
					}
	      break;
	    case '*':
	    case '/':
	    case '%':
				if(isWhat==1) {
					if(Pty > 3) {
						s++;
						isWhat=2;
						s=ConRecEval(s,3,&l2);
						switch(ch) {
							case '*':
								*l1=*l1 * l2;
								break;
							case '/':
								*l1=*l1 / l2;
								break;
							case '%':
								*l1=*l1 % l2;
								break;
							}
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
	    case '!':
	    case '~':
				if(isWhat<=1) {
					if(Pty > 2) {
						s++;
						isWhat=1;
						s=ConRecEval(s,2,l1);
						switch(ch) {
							case '!':
								*l1=!*l1;
								break;
							case '~':
								*l1=~*l1;
								break;
							}
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
	    case 'l':
	    case 'r':
				if(isWhat==1) {
					if(Pty > 5) {
						s++;
						isWhat=2;
						s=ConRecEval(s,5,&l2);
						switch(ch) {
							case 'l':
								*l1=*l1 << l2;
								break;
							case 'r':
								*l1=*l1 >> l2;
								break;
							}
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
	    case '<':
	    case '@':                // diverso
	    case '=':
	    case '>':
	      i=0;
				if(isWhat==1) {
					if(Pty > 5) {
						isWhat=2;
						s++;
						if(*s == '=') {
							i=1;
							s++;
							}
						else {
							if(*s == '>') {
								i=-1;
								s++;
								}
							}  
						s=ConRecEval(s,5,&l2);
						switch(ch) {
							case '<':
								if(!i)
									*l1=*l1 < l2;
								else {
									if(i>0) 
										*l1=*l1 <= l2;
									else
										*l1=*l1 != l2;
									}
								break;
							case '=':
								*l1=*l1 == l2;
								break;
							case '@':
								*l1=*l1 != l2;
								break;
							case '>':
								if(i)
									*l1=*l1 >= l2;
								else
									*l1=*l1 > l2;
								break;
							}
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
//                        // and,or - Gemini 9/9/26 MORTE AGLI UMANI CANCRO AI BAMBINI
			// --- LOGICAL AND (&&) ---
			case '&':
				if(isWhat == 1) {
					char s2 = *(s+1);
					if(s2 == '&') { // Operatore &&
						if(Pty > 10) {
							s+=2;
							isWhat = 2;
							s = ConRecEval(s, 10, &l2); // Chiama il livello superiore
							*l1 = (*l1 != 0) && (l2 != 0);
							isWhat = 1;
							} 
						else
							Go = 1;
						} 
					else { // Operatore & (bitwise)
						if(Pty > 7) {
							s++;
							isWhat = 2;
							s = ConRecEval(s, 7, &l2);
							*l1 = *l1 & l2;
							isWhat = 1;
							} 
						else
							Go = 1;
						}
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
				break;
			// --- LOGICAL OR (||) ---
			case '|':
				if(isWhat == 1) {
					char s2 = *(s+1);
					if(s2 == '|') { // Operatore ||
						if(Pty > 11) {
							s+=2;
							isWhat = 2;
							s = ConRecEval(s, 11, &l2); // Chiama il livello superiore
							*l1 = (*l1 != 0) || (l2 != 0);
							isWhat = 1;
							} 
						else
							Go = 1;
						} 
					else { // Operatore | (bitwise)
						if(Pty > 9) {
							s++;
							isWhat = 2;
							s = ConRecEval(s, 9, &l2);
							*l1 = *l1 | l2;
							isWhat = 1;
							} 
						else
							Go = 1;
						}
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
				break;
	    case '^':
				if(isWhat==1) {
					if(Pty > 6) {
						char s2=*++s;
						s++;
						isWhat=2;
						s=ConRecEval(s,6,&l2);
	//	    myLog->print(0,"qui i valori sono %ld e %ld\n",*l1,l2);
						*l1=*l1 ^ l2;
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
			case '?':
				if(isWhat==1) {
					if(Pty > 13) {
						char s2=*++s;
						s++;
						isWhat=2;
						s=ConRecEval(s,13,&l2);
						if(l2) {
							s=ConRecEval(s,13,l1);
							Go=TRUE;		// short-circuit!
							}
						else {
							while(*s && *s!=':')
								s++;
							s=ConRecEval(s,13,l1);
							}
						}
					else
						Go=1;
					}
				else {
					PROCError(2059,s);
					Go=1;
					}
	      break;
			case ':':		// per ? : assorbito sopra cmq
				s++;
	      break;
	    case ' ':
	      s++;
	      if(Times)
					Times--;
	      break;
	    case 0:
	      Go=TRUE;
	      break;
			case 'd':
				// gestire FNDefined( , v. preprocessor
//				break;
	    default:
	      *l1=0;
        if(*s) {
					if(isWhat==1) {
						PROCError(2059,s);
						Go=1;
						}
					else {
						if(isdigit(firstchar) && isdigit(*s)) {
							while(*s && isdigit(*s)) {
								*l1=(*l1 * 10) + (*s - '0');
								s++;
								}
							}
						else 
							s++;
						isWhat=1;
						}
					}
				else
					Go=TRUE;
	      break;
	    }
	  Times++;
	  } while(!Go && !fError);

  return s;
  }
   
long CPlusMinus::EVAL(char *s) {
  long l;

//  myLog->print(0,"\aUso di EVAL su %s..!\n",s);
  ConRecEval(s,15,&l);
//  myLog->print(0,"\aEVAL ritorna %ld\n",l);
  return l;
  }
          
