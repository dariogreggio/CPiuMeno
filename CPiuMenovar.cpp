#include "stdafx.h"
#include "Cpiumeno.h"
#include "CpiumenoTrans.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>



enum CPlusMinus::ARITM_OP CPlusMinus::FNGetAritElem(char *outbuf,int8_t *OP, char *OS, struct OPERAND *O, int8_t Co) {
// O% (RISULTATO) = 1 SE COSTANTE
// 2 SE VARIABILE
// 3 SE OPERANDO
// 0 SE FINE LINEA
  int i,j;  
  long T1;
  char TS[128],T1S[64],MyBuf[64];
  char *p;
  
  FNLO(TS);
  switch(*TS) {
    case ':':
			if(TS[1]==':') {
        *OP=1;
        *(OS+1)=*OS=':';
        *(OS+2)=0;
				_tcscat(outbuf,OS);
        return ARITM_IS_OPERANDO;
			}

    case 0:
    case ';':
    case '}':
      FIn->unget(*TS);		//FIn->Seek(-1,CFile::current);
      return ARITM_IS_EOL;
      break;
    case ')':
      if(!Brack) {
	      FIn->unget(*TS);		//FIn->Seek(-1,CFile::current);
        return ARITM_IS_EOL;
        }
      else {
        *OP=1;
        *OS=')';
        *(OS+1)=0;
				_tcscat(outbuf,OS);
        return ARITM_IS_OPERANDO;
        }
      break;
    case '\'':
      i=_tcslen(TS);
      if(i>10)			// 8 char
        PROCError(2015,NULL);
      if(i==2) 
        PROCError(2137,NULL);
      T1=0;
      p=TS+1;
      while(*p && *p != '\'') {
				if(*p=='\\') {
					p++;
					if(toupper(*p)=='X') {
						T1=(T1<<8) | xtoi(p+1);
						p+=3;
						}
					else {//		PARE ARRIVINO GIA' espanse appunto ;)
						switch(toupper(*p)) {							// (GESTIRE! \n \t ecc , unire con altrove
							case 'A':
								T1=(T1<<8) | 7;
								break;
							case 'N':
								T1=(T1<<8) | 10;
								break;
							case 'R':
								T1=(T1<<8) | 13;
								break;
							case 'T':
								T1=(T1<<8) | 9;
								break;
							case 'C':
								T1=(T1<<8) | 12;
								break;
							default:
								T1=(T1<<8) | 0;
								PROCWarn(2017,p);
								break;
							}
						}
					}
				else {
					T1=(T1<<8) | ((unsigned char)*p++);
					}
        }
      O->type=0;
      O->size=FNGetSize((uint32_t)T1);
      O->cost->l=T1;
      O->Q=VALUE_IS_COSTANTE;
      return ARITM_IS_COSTANTE;
    case '\"':
      *T1S=0;
      p=TS;
      i=0;
      while(*p) {
        if(!isprint(*p)) {
					if(T1S[i-1] != '\"')
						wsprintf(T1S+i,"\",%u,\"",*p);
					else                           
						wsprintf(T1S+i-1,"%u,\"",*p);
          i=_tcslen(T1S);
          }
        else {
          T1S[i++]=*p;
					}
        p++;
        }  
      if(!_tcsncmp(T1S+i-2,"\"\"",2))
	      T1S[i-2]=0;
      T1S[i]=0;
      if(!i)
        i++;
      else
       	i--;
      if(T1S[i-1] && (T1S[i-1] !=',')) 
        _tcscat(T1S,",");
      _tcscat(T1S,"0");
      O->type=VARTYPE_POINTER;
      O->size=1;                 
      _tcscpy(O->cost->s,FNAllocCost(T1S,1,O->type)->label);
      O->Q=VALUE_IS_COSTANTEPLUS;		// 9
      return ARITM_IS_COSTANTE;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if(_tcschr(TS,'.')) {
				UseFloat=TRUE;		// mah sì diciamo
				O->type=VARTYPE_FLOAT;
				if(_tcslen(TS)<10) {			// diciamo :)
					float f;
					O->size=4;
					f=(float)atof(TS);
					*(float*)&O->cost->l=f;
					O->Q=VALUE_IS_COSTANTE;
					sprintf(MyBuf,"%g",f);
					}
				else {
					double f;
					O->size=8;
					f=atof(TS);			// boh verificare, trovare;  https://baseconvert.com/ieee-754-floating-point
	        sprintf(T1S,"%X%X ; LONG FLOAT",(uint32_t)((*(uint64_t*)&f) >> 32),(*(uint64_t*)&f) & 0xffffffff);		// (verificare %llu ... forse qua non va
					_tcscpy(O->cost->s,FNAllocCost(T1S,4)->label);
					O->Q=VALUE_IS_COSTANTEPLUS;		// (potrei evitare se faccio cost->l a 64bit...magari poi vediamo
//					O->Q=VALUE_IS_COSTANTE;
//					*(double*)&O->cost->l64=f;		 VA GESTITO IN MOLTI POSTI! e serve ulltoa(
					sprintf(MyBuf,"%g",f);
					}
				// prendere solo i 4 byte alti di un double non è esattamente uguale al float... 
				//	quindi servirà conversione, anche se inizializzazione (passare qua Tipo var
				_tcscat(outbuf,MyBuf);
        return ARITM_IS_COSTANTE;
        }
      else {
 	      i=_tcslen(TS)-1;
        if(*TS=='0' && i>1) {
          if(toupper(*(TS+1))=='X')
            //sscanf(TS,"%lx",&O->cost->l);
						O->cost->l=xtoi(TS+2);
          else 
            O->cost->l=FNGetOct(TS+1);
          }
 	      else
          O->cost->l=atol(TS);
//        myLog->print(0,"numero\a: %ld\n",VS->l);
 	      j=toupper(TS[i]);
 	      if(j=='L') {
//        myLog->print(0,"\atrovato un long\n");
	        TS[i]=0;
  	      O->size=4;
 	        }
 	      else {
					i=FNGetSize((uint32_t)O->cost->l);
// (si potrebbe auto-stabilire SIZE...
 	        O->size=INT_SIZE /*max(INT_SIZE,i)*/;
					}
	      O->type=VARTYPE_PLAIN_INT;
        O->Q=VALUE_IS_COSTANTE;
				sprintf(MyBuf,"%u",O->cost->l);
				_tcscat(outbuf,MyBuf);
	      return ARITM_IS_COSTANTE;
        }
      break;
    default:
      *OP=FNIsOp(TS,Co);
      if(*OP) {
        _tcscpy(OS,TS);
//				_tcscat(outbuf,OS);
        return ARITM_IS_OPERANDO;
        }
      else {
				struct VARS *v;
// PERCHE'?? 2025        _tcscpy(O->cost->s,TS);
				if(!CurrFunc)
					PROCError(2099,TS);
        v=FNCercaVar(TS,FALSE);
        if(!v) {
          if(*FNLA(MyBuf)=='(') {
//            O->var=PROCAllocFunzProto(TS,VARTYPE_PLAIN_INT,INT_SIZE);  // NO qua!
            PROCError(3861,TS);		// per le builtin non dovrebbe darlo... ma ok - mettere prototipi in un ovvio header!
						goto fine;
            }
          else {
						struct ENUMS *e;
		        e=FNCercaEnum(NULL,TS,FALSE);
						if(e) {
							O->cost->l=e->var.value;
							O->type=VARTYPE_PLAIN_INT;
							O->Q=VALUE_IS_COSTANTE;
							return ARITM_IS_COSTANTE;
							}
						else {
							struct TAGS *tag;
			        tag=FNCercaAggr(TS,FALSE);
							if(tag) {
								ZeroMemory(O->var,sizeof(struct VARS));
								_tcscpy(O->var->name,tag->label);
								O->var->hasTag=tag;
								O->type=VARTYPE_CLASS;		// solo per ::
								O->size=4   /*FNGetAggr()*/;
								O->Q=VALUE_IS_VARIABILE;
								return ARITM_IS_VARIABILE;
								}
							else {
								PROCError(2065,TS);
								goto fine;
								}
							}
						}
          }
				else {
					if(O->var)
						*O->var=*v;
					}		// O->var è NULL se siamo al livello esterno...
        O->size=O->var ? O->var->size : v->size;
        O->type=O->var ? O->var->type : v->type;
        O->Q=VALUE_IS_VARIABILE;
        return ARITM_IS_VARIABILE;
        }
      break;
    }
fine:
  return ARITM_IS_UNKNOWN;
  }

int CPlusMinus::lltoa(uint64_t num, char *str, /*int len, */uint8_t base) {
	uint64_t sum=num;
	uint8_t i=0;
	uint8_t digit;

//	if(len == 0)
//		return -1;
	do {
		digit = sum % base;
		if(digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
		} while (sum /*&& (i < (len - 1))*/);
//	if(i == (len - 1) && sum)
//		return -1;
	str[i] = '\0';
	strrev(str);
	return 0;
	}

int CPlusMinus::subGetType(O_TYPE *t, O_SIZE *s, O_DIM dim, long TT) {
  char AS[64],MyBuf[sizeof(union STR_LONG)];
	uint8_t J;
	long OT;
	uint8_t ndim=0;

	OT=FIn->GetPosition();
  FNLO(AS);
  switch(*FNLO(MyBuf)) {
    case '[':
			J=*t & VARTYPE_IS_POINTER;
      do {
        J++;
        *t= ((*t | VARTYPE_ARRAY) & ~VARTYPE_IS_POINTER) | J;
        if(*FNLA(MyBuf) != ']') {
					if(CurrFunc)
						;		// se C99 potremmo accettare variabile qua! solo se entro funzione ovviamente
					int32_t d=FNGetConst(MyBuf,0);		
					if(d<0)
			      PROCError(1001,"dimensione array negativa");		// 
          dim[ndim] = d;
					ndim++;
					if(ndim>MAX_DIM-1)
						PROCError(1002,"dimensioni max=4");
          }
        PROCCheck(']');
        } while(*FNLO(MyBuf)=='[');
      break;
    case '(':
      *t |= VARTYPE_FUNC;
      break;
    case ')':		// questo chiude un puntatore a funzione... o anche altro, ma cmq ok
      ;
			OT=FIn->GetPosition();
      break;
    case ':':
			if(isdigit(*FNLA(MyBuf))) {
				*t |= VARTYPE_BITFIELD;
				PROCWarn(2000,"bitfield");		// 
				}
			else
//				OT=FIn->GetPosition();
;
      break;
    default:
      break;
    }

	FIn->RestorePosition(OT);
	// e line qua??
	return 0;
  }

int CPlusMinus::PROCGetType(char *outbuf,O_TYPE *t, O_SIZE *s, struct TAGS **tag, O_DIM dim, uint32_t *attrib, long TT) {
  int I,J=0;  
	O_SIZE S;
  O_TYPE T=0;
  char AS[64],MyBuf[64];
  long OT;
	long ol;
  struct VARS *V;
	bool is_fun_ptr=FALSE;
  
  OT=FIn->GetPosition();
	FIn->Seek(TT,CFile::begin);
	FIn->SavePosition();
	ol=__line__;
  S=-1;

rifo:

  FNLO(AS);          // ATTENZIONE ALLE PARENTESI
	_tcscat(outbuf,AS);
//  *s=0;
//  *t=0l;
//  *tag=*dim=0;
  if(!_tcscmp(AS,"struct") || !_tcscmp(AS,"union") || !_tcscmp(AS,"class")) {
    if(!_tcscmp(AS,"struct"))
      T=VARTYPE_STRUCT;
    else if(!_tcscmp(AS,"union"))
      T=VARTYPE_UNION;
    else
      T=VARTYPE_CLASS;
    *tag=FNAllocAggr(T==VARTYPE_CLASS ? 2 : (T==VARTYPE_STRUCT ? 1 : 0));           // alloca tutta la class o struct o union (was: legge il nome o ne crea uno, poi è pronto per i membri
		_tcscat(outbuf," ");
		_tcscat(outbuf,(*tag)->label);

//#pragma warning		fare magari come in ASsembler, i membri delle struct metterli qua e non in VARS  2025


    FNLA(MyBuf);
    if(!*MyBuf)
      PROCError(2059,NULL);
    else if(*MyBuf==';') {
      *s=-2;		// marker per struct/union/class che hanno ; dopo la graffa
      }
/*    else if(*MyBuf == '#') {
			FNGoToEOL();
			}*/
    else {
      *s=0;
      V=Var;
      while(V) {
        if(V->isInTag==*tag) {
          I=V->size;
          if(V->type & VARTYPE_ARRAY) {
            I=FNGetArraySize(V);
            }
          else {
            if(V->type & VARTYPE_IS_POINTER)
              I=getPtrSize(V->type);
            }
          if(T & (VARTYPE_STRUCT | VARTYPE_CLASS)) {
	          if(V->type & VARTYPE_BITFIELD) {
							T |= VARTYPE_BITFIELD;		// me lo segno per dopo! (il padre NON ha l'attributo, anche perché possono coesistere - FINIRE
							}
						else {
							if(I>= StructPacking/*INT_SIZE*/) {
								*s=((*s+StructPacking-1) & -StructPacking) +I;
								}
							else
								*s += I;
							}
            }
          else {
            if(I > *s) 
              *s=I;
            }
          }
        V=V->next;
        }
/*      V=Var;                   // non dovrebbe servire, calcola dim struct
      while(V) {
        if(V->hasTag==*tag)
          V->size=*s;
        V=V->next;
        }
        */
      if(T & VARTYPE_BITFIELD) {
				S=(*s+8)/8;
				S = (S+INT_SIZE-1) & -INT_SIZE;
				}
			else
				S=*s;
      *t=T;
      OT=FIn->GetPosition();
      }
    }
  else {
    if(!_tcscmp(AS,"unsigned")) {
      *t |= VARTYPE_UNSIGNED;
      T=*t;
      if(FNIsType(FNLA(MyBuf)) != VARTYPE_NOTYPE) {
        FNLO(AS);
				_tcscat(outbuf," ");
				_tcscat(outbuf,AS);
        TT=FIn->GetPosition();
        OT=TT;
        }
      }
    if(!_tcscmp(AS,"signed")) {
      *t |= VARTYPE_SIGNED;		// :)
      T=*t;
      if(FNIsType(FNLA(MyBuf)) != VARTYPE_NOTYPE) {
        FNLO(AS);
				_tcscat(outbuf," ");
				_tcscat(outbuf,AS);
        TT=FIn->GetPosition();
        OT=TT;
        }
      }
    if(!_tcscmp(AS,"volatile")) {
      *t |= VARTYPE_VOLATILE;
			PROCWarn(1002,"volatile non completamente implementato");
      T=*t;
      if(FNIsType(FNLA(MyBuf)) != VARTYPE_NOTYPE) {
        FNLO(AS);
				_tcscat(outbuf," ");
				_tcscat(outbuf,AS);
        TT=FIn->GetPosition();
        OT=TT;
        }
      }

		if(!_tcscmp(AS,"const"))	{	// GESTIRE! usare DATA_CONST mettere da qualche parte; v. anche di là
			FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
			TT=FIn->GetPosition();
			OT=TT;
			goto rifo;
			}

	  for(I=0; I<MaxTypes; I++) {
	    if(!_tcscmp(AS,Types[I].s)) {
	      *s=Types[I].size;
	      *t=Types[I].type | *t;
	      *tag=Types[I].tag;
	      memcpy(dim,Types[I].dim,sizeof(dim));
	      break;
	      }
	    }
		if(!_tcscmp(AS,"short") /*|| !_tcscmp(AS,"signed")*/) {
			if(FNIsType(FNLA(AS)) != VARTYPE_NOTYPE) {		// solita PATCH per short int, v. di là e COMPLETARE
				FNLO(AS);
				_tcscat(outbuf," ");
				_tcscat(outbuf,AS);
				TT=FIn->GetPosition();
				OT=TT;
				}
			}
		FNLA(AS);
    if(!_tcscmp(AS,"far")) {
      *t |= VARTYPE_FAR;		// :)
      T=*t;
      FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
      TT=FIn->GetPosition();
      OT=TT;
      }
    if(!_tcscmp(AS,"__attribute__")) {		// GCC extension generica
      FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
			PROCCheck('(');
rifo_attr:
      FNLO(AS);
			if(!_tcscmp(AS,"noreturn")) {
				*attrib |= FUNC_ATTRIB_NORETURN; 
				}
			else if(!_tcscmp(AS,"naked")) {
				*attrib |= FUNC_ATTRIB_NAKED; 
				}
			else if(!_tcscmp(AS,"weak")) {
				*attrib |= FUNC_ATTRIB_WEAK; 
				}
			else if(!_tcscmp(AS,"packed")) {
				*attrib |= VAR_ATTRIB_PACKED;
				}
			else
				PROCWarn(4068,AS);		// finire :)
			if(*FNLA(AS) == ',') {
				PROCCheck(',');
				goto rifo_attr;
				}

			PROCCheck(')');
			TT=FIn->GetPosition();
			OT=TT;
			goto rifo;
			}
    S=*s;
    T=*t;
	  }
  if((int32_t)(int16_t)*s != -2) {		// marker per aggregato... MIGLIORARE
    *s=S;

	/*	if(*AS=='(') {		// potrebbero essercene più d'una... cmq non è perfetto, le parentesi possono circondare anche un Tipo qualsiasi
			T |= VARTYPE_FUNC_POINTER | VARTYPE_FUNC;
			*t = T;
			FNLO(AS);
      TT=FIn->GetPosition();
      OT=TT;
			}*/

		FIn->RestorePosition(OT);
		__line__=ol;

    FNLA(AS);
    if(*AS=='&') {
			T |= VARTYPE_IS_REFERENCE;		// occhio anche VARTYPE_RVALUE_REF, gestire
			*t |= VARTYPE_IS_REFERENCE;		// occhio anche VARTYPE_RVALUE_REF, gestire
	    FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
			}
/* no qua è sbagliato		else if(!InBlock && (*tag=FNCercaAggr(AS,FALSE))) {		// questo è per la dichiarazioni di robe delle classi al livello esterno
			PROCCheck("::");		// obbligatorio dunque!
			return 0;
			}*/
    else {
      long l2=FIn->GetPosition();
			FNLO(AS);
			FNLA(MyBuf);

			if(!_tcscmp(MyBuf,"::")) {
				PROCCheck("::");
	      OT=FIn->GetPosition();
				FNLO(MyBuf);
				_tcscat(outbuf," ");
				_tcscat(outbuf,AS);
				_tcscat(outbuf,"::");
				_tcscat(outbuf,MyBuf);
	//			goto was_class_static;
//				OT=TT;

		    FNLA(AS);
				if(*AS=='(') {		// posono esserci asterischi interni, o la ~ ... finire
					T |= VARTYPE_FUNC;
					}
				else if(*AS=='~') {		// 
					}
				else if(*AS=='*') {		// 
					T |= VARTYPE_IS_POINTER;
					}

//					subGetType(t, s, dim, TT);		// eventualmente
				*t = T;

				FIn->RestorePosition(l2);
				__line__=ol;

				return 1;

				}
			else
				FIn->RestorePosition(l2);

			}

    J=0;
    while(*FNLA(MyBuf)=='*') {
			if(T & VARTYPE_IS_REFERENCE) {		// l'opposto è lecito invece!
				PROCError(2528,MyBuf);
				break;
				}
      J++;
      FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
      *t=T + J;
			}
//    T=*t;
    T=*t;


    FNLA(AS);
		if(*AS=='(') {		// potrebbero essercene più d'una... cmq non è perfetto, le parentesi possono circondare anche un Tipo qualsiasi
			T |= VARTYPE_FUNC;
			if(T & VARTYPE_IS_POINTER)
				T |= VARTYPE_FUNC_POINTER;
			*t = T;
			FNLO(AS);
			_tcscat(outbuf," ");
			_tcscat(outbuf,AS);
			if(T & VARTYPE_FUNC_POINTER)
				PROCCheck('*');					// appunto, ma ok
      TT=FIn->GetPosition();
//      OT=TT;
			}
		
		OT=FIn->GetPosition();

		if(*t & VARTYPE_FUNC) {
			return 1;
			}
		else
			subGetType(t, s, dim, TT);

		if(*AS == ')')		// patch urfida perché trova una parentesi di espressione dopo un cast e la interpreta come funzione
			*t &= ~VARTYPE_FUNC;

		FIn->RestorePosition(OT);
		__line__=ol;
    }      
    
  return 0;
  }

long CPlusMinus::FNIsType(char *A) {
  int I;
  
  for(I=0; I<MaxTypes; I++) {
    if(!_tcscmp(A,Types[I].s)) { 
      return Types[I].type;     
      }
    }
  if(!_tcscmp(A,"struct"))
    return -2;
  if(!_tcscmp(A,"union"))
    return -3;
  if(!_tcscmp(A,"unsigned"))
    return -4;
  if(!_tcscmp(A,"signed"))		// sinonimi, diciamo
    return -4;
	if(!_tcscmp(A,"const"))	{	// GESTIRE! usare DATA_CONST mettere da qualche parte; v. anche di là
    return -6;
		}
	if(!_tcscmp(A,"volatile"))	{	
    return -8;
		}
  if(!_tcscmp(A,"__attribute__"))			// microchip extension, GCC 
    return -7;
  if(!_tcscmp(A,"class"))
    return -9;
  return VARTYPE_NOTYPE;
  }

struct VARS *CPlusMinus::FNGetAggr(struct TAGS *Tag, const char *TS, int8_t F, int *o) {  //F=2 per class, 1 per struct, 0 union
  O_SIZE S;
  O_TYPE T;
  struct VARS *V;
  
	if(o)
		*o=0;
  V=Var;
  while(V) {
//    myLog->print(0,"sono su %Fs..",V->name);
    if(V->isInTag==Tag) {
//    myLog->print(0,("ok\n");
      if(!_tcscmp(V->name,TS)) {
				if(V->type & VARTYPE_BITFIELD) {
					}
				else {
					if(((!T) && (S==INT_SIZE)) || ((T & VARTYPE_IS_POINTER) && !(T & VARTYPE_ARRAY))) {
						if(S>1) {
							if(o)
								*o=((*o+StructPacking-1) & -StructPacking);
							}
						// v. anche  attrib & VAR_ATTRIB_PACKED;
						}
					else {
						if(o)
							*o=((*o+StructPacking-1) & -StructPacking);
						}
					}
        break;
//      o%=(o%+3)&& &FFFC
	      }   
	    else {
	      T=V->type;
	      S=V->size;
	      if(T & VARTYPE_ARRAY) {
	        S = FNGetArraySize(V);
	        }
	      else {
	        if(T & VARTYPE_IS_POINTER)
	          S=getPtrSize(T);
	        }
		    switch(F) {
					case 2:		// classe
					case 1:		// struct
						if(V->type & VARTYPE_BITFIELD) {

							}
						else {
							if(((!T) && (S==INT_SIZE)) || ((T & VARTYPE_IS_POINTER) && !(T & VARTYPE_ARRAY))) {
		//		        *o=((*o+INT_SIZE-1) & -INT_SIZE) +S;
								if(S>1) {
									if(o)
										*o=((*o+StructPacking-1) & -StructPacking) +S;
									}
								else {
									if(o)
										*o=S;
									}
								// v. anche  attrib & VAR_ATTRIB_PACKED;
								}
							else {
								if(o)
		//		        *o += S;
									*o=((*o+StructPacking-1) & -StructPacking) +S;
								}
							}  
						break;
					case 0:		// union
						break;
					}  
	      }
      }
    V=V->next;
    }

  return V;
  }

struct VARS *CPlusMinus::FNGetAggr(struct TAGS *Tag, int8_t F) {  //F=2 per class, 1 per struct, 0 union
  O_SIZE S;
  O_TYPE T;
  struct VARS *V;
  
  V=Var;
  while(V) {
    if(V->hasTag==Tag) {
		  switch(F) {
				case 2:		// classe
					break;
				case 1:		// struct
					break;
				case 0:		// union
					break;
				}  
			break;
      }
    V=V->next;
    }

  return V;
  }

uint32_t CPlusMinus::FNGetAggr2(struct VARS *p, struct VARS *v, int *o, int *o2) {  // restituisce pos e mask per bitfield
	struct VARS *V;
	uint32_t i,j;
	O_TYPE T;
	char TS[64];

  i=0;
  V=Var;
  while(V) {
    if(V->isInTag==v->isInTag) {
			T=V->type;
			}
    if(V->isInTag==v->isInTag) {
			if(!_tcscmp(V->name,v->name)) {
				break;
				}   
			else {
				if(v->type & VARTYPE_BITFIELD) {
					// vedere se consentire mix bitfield e non! ovunque
					}
				if(T & VARTYPE_STRUCT) {
					if(i)
						;
					}  
				}
			}
    V=V->next;
    }

	*o=j=i;
	if(i) {		// safety :)
		uint32_t k= j ? (1 << j) : 1;
		j=0;
		do {
			j <<= 1;
			j |= k;
			} while(--i);
		}

	if(o2) {
		*o2=0;
		while(*o>=INT_SIZE*8) {		// 
			*o2+=INT_SIZE;
			*o-=INT_SIZE*8;
			}
		}

  return j;
  }

struct TAGS *CPlusMinus::subAllocTag(const char *TS,int8_t t) {
  struct TAGS *C;
  
  C=(struct TAGS*)malloc(sizeof(struct TAGS)); 
  if(!C) {
    PROCError(1001,"Fine memoria TAGS");
    }
  if(StrTag) {
    LTag->next=C;
    LTag=C;
    }
  else {
    LTag=StrTag=C;
    }
	C->func=CurrFunc;
  C->next=(struct TAGS*)NULL;
	C->member=(struct VARS*)NULL;
	C->parent=(struct TAGS*)NULL;
	C->friends=NULL;
  _tcsncpy(C->label,TS,MAX_NAME_LEN);
  C->label[MAX_NAME_LEN]=0;
	C->type=t;
	C->block=InBlock;
	C->blockId=OldTX[InBlock].id;
  return C;
  }

struct TAGS *CPlusMinus::FNAllocAggr(int8_t type) {
  bool Go=0;
	O_DIM dim;
	O_SIZE s;
  O_TYPE t;
	uint32_t attrib=0;
	int i;
  char MyBuf[sizeof(union STR_LONG)],TS[64],AS[64];
	char decor[128],outbuf[256];
  long OT;
  struct VARS *V;
  struct TAGS *C,*tag;
	bool is_ctor=FALSE,is_dtor=FALSE;
	enum VAR_CLASSES classe;

  if(*FNLA(MyBuf) != '{') {
    FNLO(TS);
    C=FNCercaAggr(TS,FALSE);
    if(!C) {
      C=subAllocTag(TS,type); 
      if(*FNLA(MyBuf) == ';') {		// forward declaration
				return C;		//Go=TRUE;
				}
      else if(*MyBuf == ':') {		// eredita
				if(type==0) // union no!
					PROCError(2652,TS);

				PROCCheck(':');
		    FNLO(AS);
		    tag=FNCercaAggr(AS,FALSE);		// MA se era stata fatta forward decl la trova MA è ERRORE! cercare membri o mettere flag
	      if(!tag)
					PROCError(2504,AS);
				if(!FNHasVar(tag))			// ecco
					PROCError(2504,AS);
//					PROCError(2651,TS);		// se la base class è union! fare
				C->parent=tag;
				PROCCheck('{');
				}
      else if(*MyBuf != '{') {		
        PROCError(2079,TS);
				Go=TRUE;
				}
      else 
        FNLO(TS);
      }
    else {
		  if(*FNLA(MyBuf) == ';') {
				PROCWarn(4091,TS);
				Go=TRUE;
				}
			else {
				if(C->type != type)
					PROCError(2371,TS);
				if(*MyBuf == '{') {
					if(FNHasVar(C))	{		// se era già stata definita
						_tcscpy(MyBuf,"tag ");
						_tcscat(MyBuf,TS);
						PROCError(2025,MyBuf);
						Go=TRUE;
						}
					else {
						PROCCheck('{');
						}
					}
				else
					Go=TRUE;
				}
      }
    }
  else {
    C=subAllocTag(FNGetLabel(MyBuf,3),type); 
    PROCCheck('{');
		wsprintf(MyBuf,"%s %s {",type==0 ? "union " : "struct ",C->label);
		PROCOper(LINE_TYPE_DATA_DEF_CONT,MyBuf);
    }
	if(C->parent) {
		wsprintf(MyBuf,"\tstruct %s __base;",C->parent->label,"class");
		PROCOper(LINE_TYPE_DATA_DEF,MyBuf);
		}

	if(type == 2)
	 	DefaultVisibility=VIS_PRIVATE;
	else
  	DefaultVisibility=VIS_PUBLIC;

  if(!Go) {                 // guardo i singoli membri
    do {
			OT=FIn->GetPosition();
			*outbuf=0;
      FNLO(TS);
			if(*TS=='#') {		// salto anche qua gli eventuali #line...
				FNGoToEOL();
				continue;
				}
      s=INT_SIZE;
      t=VARTYPE_PLAIN_INT;
      ZeroMemory(dim,sizeof(dim));
      tag=NULL;
			classe=CLASSE_MEMBER;

			if(!_tcscmp(TS,"protected")) {		// cmq anche in dichiarazioni ereditarietà...
  			DefaultVisibility=VIS_PROTECTED;
				PROCCheck(':');
				continue;
	  		}
			else if(!_tcscmp(TS,"private")) {
	 			DefaultVisibility=VIS_PRIVATE;
				PROCCheck(':');
				continue;
				}
			else if(!_tcscmp(TS,"public")) {		// cmq anche in dichiarazioni ereditarietà...
  			DefaultVisibility=VIS_PUBLIC;
				PROCCheck(':');
				continue;
				}
			else if(!_tcscmp(TS,"static")) {		// 
				classe=CLASSE_MEMBER_STATIC;
				OT=FIn->GetPosition();
	      FNLA(TS);
				if(!FNIsType(TS))
		      FNLO(TS);
				}
			else if(!_tcscmp(TS,"friend")) {		// 
				PROCCheck("class");		// secondo gemini in ultime versioni spec. può non esserci...
	      FNLO(TS);
				struct TAGS *f=FNCercaAggr(TS,FALSE),*c,*c2;
				c=c2=C;
				while(c->friends) {
					c=c->next;
					if(c)
						c2=c;
					}
				if(f) {
					c2=f;
					}
				else {
					c2=subAllocTag(TS,2);			// FINIRE! con inizializzazioni, e ev altre cose 
					c2->func;
					}
				}
			else if(!_tcscmp(TS,C->label)) {		// costruttore
  			is_ctor=TRUE;
				FIn->RestorePosition(OT);
				}
			else if(!_tcscmp(TS,"~")) {		// costruttore
  			is_dtor=TRUE;
				OT=FIn->GetPosition();
				FNLO(TS);
				if(_tcscmp(TS,C->label))
					PROCError(2523,TS);
				FIn->RestorePosition(OT);
				}
			else if(FNIsType(TS) == -1) {
				PROCError(2146,TS);
				break; //continue;		
				}

			// UNIRE con la dichiarazione al livello esterno in IsDecl!

			// credo che se ctor o dtor questo si possa saltare... prova!
//			if(!is_ctor && !is_dtor)
				PROCGetType(outbuf,&t,&s,&tag,dim,&attrib,OT);
	//		else
		//		FIn->RestorePosition(OT);

			if(t & VARTYPE_FUNC) {
				char funcType[128];
//				_tcscpy(AS,C->label);
				if(is_ctor)
					_tcscpy(decor,ctor);
				else if(is_dtor)
					_tcscpy(decor,dtor);
				else 
					_tcscpy(decor,C->label);

//				FIn->RestorePosition(OT);
//				subGetType(&t, &s, dim, OT);

				if(is_ctor || is_dtor) {
//					PROCError(2533); ctor
	//				PROCError(2524); dtor
					s=SIZE_NULL;
// NO, c'è funzione!					t=TYPE_NULL;
					}

				_tcscpy(MyBuf,C->label);
				OT=FIn->GetPosition();
				if(!is_ctor && !is_dtor)
					_tcscpy(funcType,TS);
				FNLO(TS);
				if(is_ctor || is_dtor) {
					_tcscat(MyBuf,decor);
					*funcType=*outbuf=0;
					}
				else {
					_tcscat(MyBuf,"_");
					_tcscat(MyBuf,TS);
					}
				_tcscat(MyBuf,"__");
//				FIn->RestorePosition(OT);
				PROCCheck('(');
				collectTypeList(MyBuf);
				V=FNCercaVar(MyBuf,FALSE);		// perché in teoria DclVar fa il controllo se esiste già, ma non conosce ancora il mangling..
//				__line__=ol;
				if(V && V->type & VARTYPE_FUNC_BODY)
					PROCError(2086,MyBuf);
				else {

				FNLO(TS);
				FNLA(TS);
				if(*TS == '{') {
					FIn->RestorePosition(OT);
					Declaring=TRUE;

					V=PROCDclVar(outbuf,classe,0,t,s,C,dim,attrib,FALSE,decor);

					if(is_ctor && C->parent) {		// 
						_tcscpy(AS,C->parent->label);
						_tcscat(AS,ctor);
						wsprintf(TS,"(%s*)&this->__base",C->parent->label);
						PROCOper(LINE_TYPE_CALL,AS,TS,NULL,"chiamo padre",LINE_IS_NORMAL);
						}
					is_ctor=FALSE; is_dtor=FALSE;

	/*				if(classe!=CLASSE_MEMBER_STATIC)
						wsprintf(MyBuf,"%s* this,%s",C->label, outbuf);
					else
						wsprintf(MyBuf,"%s",outbuf);*/
					PROCOper(LINE_TYPE_FUNCTION_DECLARATION,funcType,V->name,outbuf,NULL,LINE_IS_NORMAL);

					PROCCheck('{');
					PROCBlock();
					if(*FNLA(TS)=='}') {
						PROCCheck('}');
						wsprintf(MyBuf,"\t};\n");
						PROCOper(LINE_TYPE_DATA_DEF,MyBuf);
						goto fine_class;
						}
					}
				else if(*TS == ';') {
					PROCCheck(';');
					V=PROCAllocVar(MyBuf,VARTYPE_FUNC/*TYPE_NULL*/,CLASSE_MEMBER,0,SIZE_NULL,C,NULL);
					is_ctor=FALSE; is_dtor=FALSE;
					}
				*decor=0;
				continue;
				}

				}
			else {
				FNLA(AS);
				if(!_tcscmp(AS,"short")/* || !_tcscmp(T,"signed")*/) {
					FNLA(AS);
					if(FNIsType(AS)==VARTYPE_PLAIN_INT && !t) {		// PATCH rapida per "short int" ecc... MIGLIORARE
						FNLO(AS);
						}
					}
				}

			goto primogiro;		// perché ho già l'ev. ptr qua! v.sotto, migliorare

      do {



//        PROCGetType(&t,&s,&tag,dim,&attrib,OT);
//        myLog->print(0,"tipo %lx, size %d\n",t,s);
//			  FNLA(AS);




				t &= VARTYPE_NOT_A_POINTER;
				i=t & VARTYPE_ARRAY ? 0 : 0;		// gli array sono sempre anche puntatori, minimo
				while(*FNLA(MyBuf)=='*') {		// sarebbe da gestire in GetType... qua il * non appartiene al tipo dichiarato a inizio riga ma per ciascuno...
					FNLO(MyBuf);
					i++;
					}
				t=i;
				OT=FIn->GetPosition();

				subGetType(&t, &s, dim, OT);

primogiro:

				FNLO(AS);
				if(FNCercaVar(C,AS))
					PROCError(2011,AS);


	      V=PROCAllocVar(AS,t,classe,0,s,tag,dim);
//				FNLO(MyBuf);
				PROCOper(LINE_TYPE_DATA_DEF,"",outbuf,AS,AS,LINE_IS_NORMAL);


        V->isInTag=C;

        if(*FNLA(MyBuf) == ':') {
					FNLO(MyBuf);
					i=FNGetConst(MyBuf,0);
					if(!i)
						PROCError(2149);
					if(i>INT_SIZE*8) {
						PROCWarn(4309);		// vabbe' :)
						i=INT_SIZE*8;
						}
					}

				if(OutSource) {
					wsprintf(MyBuf,"|%5u| : .. %s %u",__line__,AS,V->size);
//					FNGetLine(OldTextp,MyBuf+10);		// complicato... lascio solo nomi
//					MyBuf[_tcslen(MyBuf)-2]=0;		// tolgo CR se no diventa doppio
					PROCOper(LINE_TYPE_COMMENTO,MyBuf);
					}

        while(*FNLA(MyBuf) == '[') {
          while(*FNLO(MyBuf) != ']')
						;
          }
        FNLO(TS);
        } while((*TS != ';') && (*TS));
      if(!*TS) {
				Go=TRUE;
        PROCError(2054,";");
				}
      if(*FNLA(MyBuf) == '}')
        Go=TRUE;
      } while(!Go);
    PROCCheck('}');
		wsprintf(MyBuf,"\t};\n");
		PROCOper(LINE_TYPE_DATA_DEF,MyBuf);
    }

fine_class:
	if(type == 2) {
		struct VARS *v=Var;
		i=0;
		while(v) {		// cerco almeno un costruttore
			if(v->isInTag==C) {
				if(strstr(v->name,ctor)) { 
					i=1;
					}
				}
			v=v->next;
			}
		if(!i) {	// se non c'è, lo creo
			_tcscpy(AS,C->label);
			_tcscat(AS,ctor);
			V=PROCAllocVar(AS,VARTYPE_FUNC | VARTYPE_FUNC_BODY/*TYPE_NULL*/,CLASSE_MEMBER,0,SIZE_NULL,C,NULL);
			V->isInTag=C;
			// e creare funzione vuota!
			_tcscpy(TS,C->label);
			_tcscat(TS," *this");
			PROCOper(LINE_TYPE_FUNCTION,NULL,AS,TS, NULL,LINE_IS_NORMAL);
//			PROCOper(LINE_TYPE_ISTRUZIONE,AS,"() {\n\t}",NULL,NULL,LINE_IS_NORMAL);
			if(C->parent) {		// 
				_tcscpy(AS,C->parent->label);
				_tcscat(AS,ctor);
				wsprintf(TS,"(%s*)&this->__base",C->parent->label);
				PROCOper(LINE_TYPE_CALL,AS,TS,NULL,"chiamo padre",LINE_IS_NORMAL);
				}
		  PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"\t}\n");
			}

		i=0;
		v=Var;
		while(v) {		// cerco almeno un distruttore
			if(v->isInTag==C) {
				if(strstr(v->name,dtor)) { 
					i=1;
					}
				}
			v=v->next;
			}
		if(!i) {	// se non c'è, lo creo
			_tcscpy(AS,C->label);		// il distruttore invece sempre, dice
			_tcscat(AS,dtor);
			V=PROCAllocVar(AS,VARTYPE_FUNC | VARTYPE_FUNC_BODY/*TYPE_NULL*/,CLASSE_MEMBER,0,SIZE_NULL,C,NULL);
			V->isInTag=C;
			// e creare funzione vuota!
			_tcscpy(TS,C->label);
			_tcscat(TS," *this");
			PROCOper(LINE_TYPE_FUNCTION,NULL,AS,TS, NULL,LINE_IS_NORMAL);
			if(C->parent) {		// inserire dtor base
				_tcscpy(AS,C->parent->label);
				_tcscat(AS,dtor);
				wsprintf(TS,"(%s*)&this->__base",C->parent->label);
				PROCOper(LINE_TYPE_CALL,AS,TS,NULL,"chiamo padre",LINE_IS_NORMAL);
				}
		  PROCOper(LINE_TYPE_ISTRUZIONE,"}\n");		// ci mette il ; , migliorare
			}
		}

  return C;
  }

int CPlusMinus::StoreVar(char *outbuf,struct VARS *V, int8_t RQ, struct VARS *RVar, union STR_LONG *RCost, uint16_t ofs) {
  O_SIZE S;
	O_TYPE T;
	int i;
  long l;
  char myBuf[64],myBuf2[256];
  
  if(debug)
    myLog->print(0,"STOREVAR in VAR# %X\n",V);
	
	_tcscpy(myBuf2,outbuf);
	if(V->classe>=CLASSE_MEMBER) {
		_tcscpy(outbuf,"this->");
		_tcscat(outbuf,V->name);
		}
	else
		_tcscpy(outbuf,V->name);
	_tcscat(outbuf,"=");
	_tcscat(outbuf,myBuf2);

  S=V->size;
  T=V->type;
  if(T & VARTYPE_ARRAY)
		PROCError(2106,NULL);
  if(T & VARTYPE_CONST)
		PROCError(2166,NULL);
  if((T & (VARTYPE_CLASS | VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_ARRAY | VARTYPE_FUNC)) && !(T & VARTYPE_IS_POINTER))
		PROCError(2106,NULL);
	if(RQ == VALUE_IS_VARIABILE) {
		if(RVar->size > V->size)
			PROCWarn(4244,NULL);
		}
	else if(RQ & VALUE_IS_COSTANTE) {
		if(FNGetSize((uint32_t)RCost->l) > V->size)
			PROCWarn(4244,NULL);
		}


	// anche su StoreD0
  if(RQ==VALUE_IS_VARIABILE) {		// SBAGLIATO Direi, servirebbe il tipo anche delle costanti!! passare type & size separatamente
//	if(RVar) {
		if((RVar->type & VARTYPE_IS_POINTER) && (V->type & VARTYPE_IS_POINTER)) {    // warning se puntatori diversi
			O_SIZE s1,s2;
			if((RVar->type & ~VARTYPE_ARRAY) != (V->type & ~VARTYPE_ARRAY))		
				PROCWarn(4047);

			// se array OCCHIO che mancano le dim... finire
			s1=FNGetMemSize(V->type,V->size,0,0);
			s2=FNGetMemSize(RVar->type,RVar->size,0,0);
			if(s1 != s2)		// !
				PROCWarn(4049);
			}
		}

	S=FNGetMemSize(T,S,0/*dim*/,1);
  if(T & VARTYPE_FLOAT) {               // float
  	PROCGetAdd(VALUE_IS_VARIABILE,V,0,TRUE);
		switch(S) {
			case 4:
				// bah qua penso non serva, basta copiare 1 o 2 long
				PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d",NULL,NULL,NULL,(V->type & VARTYPE_VOLATILE ? LINE_IS_VOLATILE : LINE_IS_NORMAL));
				break;
			case 8:
				PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d8",NULL,NULL,NULL,(V->type & VARTYPE_VOLATILE ? LINE_IS_VOLATILE : LINE_IS_NORMAL));
				break;
			}

    }
  else {  
		char storString2[16]={0};
		switch(S) {
			case 1:
				_tcscat(storString2,".b");
				break;
			case 2:
				_tcscat(storString2,".w");
				break;
			case 4:
				_tcscat(storString2,".d");
				break;
			}
		uint8_t srcReg;
		if(RQ==VALUE_IS_VARIABILE && RVar->classe==CLASSE_REGISTER)
			srcReg=1;
		else
			srcReg=0;
		} 

  if(OutSource) {
//    i=_tcslen(LastOut->s)+_tcslen(V->name)+20;
//    PROCOut(NULL,"\t\t\t\t; ",V->name,NULL,NULL);	
//    LastOut=(struct LINE *)_frealloc(LastOut,i);
//    LastOut->prev->next=LastOut;
//    _tcscpy(LastOut->rem,V->name);
    }
			   
  return 0;
  }

int CPlusMinus::ReadVar(char *outbuf,struct VARS *V,O_TYPE T,O_SIZE S,uint8_t isCond,bool asPtr) {   // m=0 se norm, 1 se condiz.
  int i,s,s1;                                           // nSize è SIZE per CAST... 0 se non voluto
  char myBuf[64];   

  if(!S) {
	  S=V->size;
    T=V->type;                    // preservo solo unsigned... (da rivedere)
	  }
	else {
	  T=(V->type & ~(VARTYPE_UNSIGNED | VARTYPE_IS_POINTER)) | (T & (VARTYPE_UNSIGNED | VARTYPE_IS_POINTER));		// e ROM??
	  }  
  s1=FNGetMemSize(T,S,NULL/*dim*/,1);
  s=FNGetMemSize(V,1);
  s1=__min(s,s1);

	if(V->classe>=CLASSE_MEMBER) {
		_tcscat(outbuf,"this->");
		_tcscat(outbuf,V->name);
		}
	else
		_tcscat(outbuf,V->name);

  if((T & VARTYPE_ARRAY) || 
		((T & (VARTYPE_CLASS | VARTYPE_UNION | VARTYPE_FLOAT | VARTYPE_STRUCT | VARTYPE_FUNC) /*0x3900*/) && 
		(!(T & VARTYPE_IS_POINTER)))) {
		PROCGetAdd(VALUE_IS_VARIABILE,V,0,asPtr);
		if(T & VARTYPE_FUNC) {		// questo serve se si usa il nome della funzione, senza chiamarla, per forzare EXTRN del simbolo
			FNCercaVar(V->name,FALSE)->type |= VARTYPE_FUNC_USED;		// ma devo cercare la Var vera!
			}
		}
  else {
		char movString2[16];
		_tcscpy(movString2,"mov");
		switch(s) {
			case 1:
				_tcscat(movString2,".b");
				break;
			case 2:
				_tcscat(movString2,".w");
				break;
			case 4:
				_tcscat(movString2,".d");
				break;
			}
		if(isCond)
			_tcscat(movString2,".f");

		if(s != FNGetMemSize(T,S,NULL/*dim*/,1))
			PROCCast(T,S,&V->type,&V->size,-1);
		}      

  if(OutSource) {
//    i=_tcslen(LastOut->s)+_tcslen(V->name)+20;
//    PROCOut(NULL,"\t\t\t\t; ",V->name,NULL,NULL);	
//    LastOut=(struct LINE *)_frealloc(LastOut,i);
//    LastOut->prev->next=LastOut;
//    _tcscat(LastOut->s,"\t\t; ");
//    _tcscpy(LastOut->rem,V->name);
    }

fine:    
  return 0;
  }



int CPlusMinus::PROCUseCost(char *outbuf,int8_t V, O_TYPE T, O_SIZE S, union STR_LONG *C,bool asPtr) {
  int i;
	char myBuf[64];

  if(V & VALUE_IS_COSTANTE) {
		if(T & VARTYPE_FLOAT) {		// fare, 2025
		  if(V == VALUE_IS_COSTANTEPLUS) {		// 9
				if(outbuf)
					_tcscat(outbuf,C->s);
				}
			else {
				sprintf(myBuf,"%g",C->l);
				if(outbuf)
					_tcscat(outbuf,myBuf);
				}
      }
		else if(T & VARTYPE_IS_POINTER) { 
		  if(V == VALUE_IS_COSTANTEPLUS) {		// 9
				if(outbuf)
					_tcscat(outbuf,C->s);
				}
			else {
				wsprintf(myBuf,"%d",C->l);
				if(outbuf)
					_tcscat(outbuf,myBuf);
				}
      }
		else {
		  if(V==VALUE_IS_COSTANTE) {			// (integer
				wsprintf(myBuf,"%d",C->l);
				if(outbuf)
					_tcscat(outbuf,myBuf);
				}
			else {		// array ecc
				if(outbuf)
					_tcscat(outbuf,C->s);
				}
			}
		}
	
  return 0;
  }

int CPlusMinus::FNIsOp(const char *A, int Co) {
  int I,T;
  
  I=0;
  do {
    do {
      if(I==48 /*(sizeof(Op)/sizeof(struct OPERANDO)) non lo prende @#$%*/)
        return 0;
      } while(_tcscmp(Op[I++].s,A));
    I--;
    T=Op[I].p;
    if(T==2) {
      switch(*A) {
        case '*':
        case '&':      
Lcase:        
          if(Co>0) {
            T=0;  
            I++;
            }
          break;
        case '+':		// unary plus e minus
        case '-':
          if(!*(A+1))
            goto Lcase;
          break;
        default:
          break;
        }
      }
    } while(!T);
  return T;
  }
  
/*enum VAR_CLASSES*/ int CPlusMinus::FNIsClass(const char *A) {

  if(!_tcscmp(A,"auto")) {
    return CLASSE_AUTO;
    }
  else if(!_tcscmp(A,"extern")) {
    return CLASSE_EXTERN;
    }
  else if(!_tcscmp(A,"register")) {
    return CLASSE_REGISTER;
    }
  else if(!_tcscmp(A,"static")) {
    return CLASSE_STATIC;
    }       
  else if(!_tcscmp(A,"interrupt")) {
    return CLASSE_INTERRUPT;
    }       
  else if(!_tcscmp(A,"fastcall")) {
    return CLASSE_FASTCALL;
    }       
  else if(!_tcscmp(A,"pascal")) {
    return CLASSE_PASCAL;
    }       
  else if(!_tcscmp(A,"inline")) {
    return CLASSE_INLINE;
    }       
  else if(!_tcscmp(A,"const")) {
    return CLASSE_GLOBAL /*CLASSE_CONST*/;			// v. anche di là, diciamo così
    }       
  else if(!_tcscmp(A,"virtual")) {
    return CLASSE_VIRTUAL;
    }       
  else
    return -1;
  }
 
O_SIZE CPlusMinus::FNGetMemSize(O_TYPE T, O_SIZE S, O_DIM d,uint8_t m) {
	int i;

  switch(m) {
    case 2:
			if(T & VARTYPE_ARRAY) {
		    if(T & VARTYPE_IS_2POINTER)
					return getPtrSize(T);
				else
		      return S;  
				}
	    else
	      return S;  
	    break;  
    case 1:
	    if(T & (VARTYPE_IS_POINTER | VARTYPE_FUNC_POINTER | VARTYPE_IS_REFERENCE))
	      return getPtrSize(T);
	    else
	      return S;  
	    break;  
	  case 0:  
			if(T & (VARTYPE_CLASS | VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_ARRAY | VARTYPE_FUNC | VARTYPE_IS_POINTER) /*0x1d0f*/) {
				if(T & VARTYPE_ARRAY) {
					if(T & VARTYPE_IS_2POINTER)	{	// 
						S=getPtrSize(T);
						}
					else if(T & VARTYPE_IS_POINTER) {
						i=MAX_DIM-1;
						if(d) {
							while(!d[i] && i>0)
								i--;
							if(i<0)
								PROCError(1001,"array dims");
							if(i>0 /*&& d[i]*/) {		// la prima non conta ai fini del calcolo! si parte da destra
								T &= VARTYPE_IS_POINTER;
								while(T--) {
									S *= d[i];
									i--;
									} 
								}
							else		// se non ci sono dimensioni, allora era un puntatore trattato come array...
								S=PTR_SIZE;
							}
						else
							S=S;		//:)
						}
					}
				else if(T & VARTYPE_IS_POINTER | VARTYPE_IS_REFERENCE)		// 
					S=getPtrSize(T);
				else {
					}
		    return S;
				}
	    else
	      return S;  
	    break;  
    }
  return 0;  
  }

O_SIZE CPlusMinus::FNGetMemSize(struct VARS *v,uint8_t m) {

	return FNGetMemSize(v->type,v->size,v->dim,m);
	}

O_TYPE CPlusMinus::FNGetPureType(struct VARS *v) {

	return v->type & ~(VARTYPE_FUNC_POINTER | VARTYPE_IS_REFERENCE | VARTYPE_INITIALIZED | VARTYPE_FUNC_USED);
	}

O_SIZE CPlusMinus::FNGetArraySize(struct VARS *v) {
	int i=0;
	uint8_t ptrlev=0;
	O_SIZE s;
	
	s=1;		// OVVIAMENTE non posso chiamare GetMemSize qua!
	while(v->dim[i] && i<MAX_DIM) {
		s *= v->dim[i];
		ptrlev++;
		i++;
		} 
	if((v->type & VARTYPE_IS_POINTER) > ptrlev)
		s *= getPtrSize(v->type);
	else
		s *= v->size;

	return s;
	}

uint8_t CPlusMinus::FNGetArrayDims(struct VARS *v) {
	int i=0;
	
	while(v->dim[i] && i<MAX_DIM)
		i++;

	return i;
	}

O_SIZE CPlusMinus::FNGetSize(uint32_t n) {
	O_SIZE s;

  s=1;;
  if(n & 0xffffff00)
    s=2;
  if(n & 0xffff0000)
    s=4;
	return s;
	}

O_SIZE CPlusMinus::FNGetSize(uint64_t n) {
	O_SIZE s;

  s=1;;
  if(n & 0xffffff00)
    s=2;
  if(n & 0xffff0000)
    s=4;
  if(n & 0xffffffff00000000)
    s=8;
	return s;
	}

