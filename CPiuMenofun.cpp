#include "stdafx.h"
#include "Cpiumeno.h"
#include "CpiumenoTrans.h"

#include <mmsystem.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <ctype.h>


int CPlusMinus::PROCUsaFun(struct VARS *V,bool isMember,const char *n) {    //
  int I,T=0;
	int16_t i,j;
  int totParm,prParm=0;
  int *parmPtr;
	O_TYPE parmType;
	O_SIZE parmSize;
  char Clabel[32],MyBuf[128];
  struct LINE *t,*t1;
  struct VARS RPtr;
  struct OPERAND R;
  union STR_LONG RCost;
	bool parmProto;
	char outbuf[256];
			  
  if(debug)
    myLog->print(0,"USAFUN %x\n",V);           
		   
  I=0;
  ZeroMemory(&R,sizeof(struct OPERAND));
  ZeroMemory(&RPtr,sizeof(struct VARS));
  ZeroMemory(&RCost,sizeof(union STR_LONG));
	*Clabel=0;
  
  FuncCalled=TRUE;

	if(!_tcsncmp(V->name,"_builtin_",9)) {		// in effetti andrebbe beccato prima, e non inserita nelle VAR...

// e poi sotto, naturalmente

		}

	if(!(V->type & VARTYPE_FUNC_POINTER)) {
		struct VARS *v;
		V->type |= VARTYPE_FUNC_USED;         // funzione usata almeno una volta  
		// siccome ora questa V è una copia, vado a settare il flag di quella vera SE C'ERA PROTOTIPO! (credo valga per inline ? 2026
		v=FNCercaVar(V->name,0);
		if(v)
			v->type |= VARTYPE_FUNC_USED;
		}

	if(V->modif & FUNC_MODIF_INLINE) {
		if(!_tcscmp(CurrFunc->name,V->name))
			PROCError(4710,"function has recursion");
		// ovvero si potrebbe convertire a una call normale
		}

	parmPtr=V->parm.ptr32;
	if(parmPtr) {
		totParm=parmPtr[0];		// il primo int è il #parm da prototipo
		parmPtr++;
		if(debug)
		  myLog->print(0,"La fun %s ha %d parm\n",V->name,totParm);
		}
	else {
		totParm=-1;
	  }	

	if(isMember) {
		if(1)		// SOLO SE subclass
			wsprintf(outbuf,"(%s*)&%s,",V->isInTag->label,n);
		else
			wsprintf(outbuf,"&%s,",n);
		}
	else
		*outbuf=0;
  if(*FNLA(MyBuf) != ')') {
		do {
		  R.Q=0;
		  R.size=0;
		  R.type=0l;
			R.var=&RPtr;
			R.cost=&RCost;
//		  isRValue=isPtrUsed=0;

			if(isMember) {		// inietto this
				parmType=parmPtr[0];
				parmSize=parmPtr[1];
				parmProto=TRUE;
			  i=FNGetMemSize(parmType,parmSize,0/*dim*/,1);
				goto dcl_parm;
				}

		  i=0;
		  FNRev(outbuf,14,&i,Clabel,&R);
			_tcscat(outbuf,",");

      if(totParm != -1 && prParm<totParm) {
				parmType=parmPtr[0];
				parmSize=parmPtr[1];
				// defvalue...
				parmProto=TRUE;
				}
			else {
				parmType=R.type;
				parmSize=R.size;
				parmProto=FALSE;
				}

		  i=FNGetMemSize(parmType,parmSize,0/*dim*/,1);
		  j=FNGetMemSize(R.type,R.size,0/*dim*/,1);

			if(parmType & VARTYPE_IS_POINTER && !(R.type & VARTYPE_IS_POINTER)) {	// 
				PROCError(2116,R.var ? R.var->name : "");
				}
			else if(parmSize /*void* assorbe tutto!*/ && (parmType ^ R.type) & (VARTYPE_STRUCT | VARTYPE_UNION)
				|| ((parmType ^ R.type) & VARTYPE_IS_POINTER)) {	// e poi tag, dim array...
				PROCWarn(4133,R.var ? R.var->name : "");
				}

dcl_parm:



L19440:
  	  prParm++;
			if(parmPtr)
		    parmPtr+=4;		// mi sposto al tipo e size e defvalue del prossimo parm

			if(OutSource) {
				char myBuf[32];
				wsprintf(myBuf,"parm %u",prParm);
//				_tcscat(LastOut->rem,myBuf);
				}


			if(isMember) {
				isMember=FALSE;		// e salto, come se avessi trovato il this davvero!
				}
			else {
				FNLO(MyBuf);          
				if((*MyBuf != ',') && (*MyBuf != ')')) 
					PROCError(2059,MyBuf);
				}

			if(parmPtr) {                              // se finisce con ... (var args)
				if(parmPtr[0] == -1)
					totParm=-1;		// ...da qui in poi do tutto buono!
				}

		  } while(*MyBuf != ')');


		}
  else {
		if(isMember) {		// inietto this
			parmType=parmPtr[0];
			parmSize=parmPtr[1];
			parmProto=TRUE;
			i=FNGetMemSize(parmType,parmSize,0/*dim*/,1);
		  FNLO(MyBuf);
		  *MyBuf = ')';		// forzo uscita!
			goto dcl_parm;
			}

	  FNLO(MyBuf);
	  }

//	myLog->print("totparm %d, prparm %d\n",totParm,prParm);  
  if(totParm != -1) {
rifo_defparm:
		if(prParm>totParm) {
			wsprintf(MyBuf,"%s, #%u",V->name,prParm);
			PROCError(2116,MyBuf);
			}
		else if(prParm<totParm) {

			parmType=parmPtr[0];
			parmSize=parmPtr[1];

			if(parmType & VARTYPE_INITIALIZED) {
			// (andare a vedere ev. parametri di default!
				sprintf(MyBuf,"%d",parmPtr[2]);		// finire con altre cose!
				_tcscat(outbuf,MyBuf);
				parmPtr+=4;
				prParm++;
				if(prParm<totParm)
					_tcscat(outbuf,",");
			// e poi ev.    PROCError(2664,MyBuf); se il prox non ce l'ha e quello prima sì!
				goto rifo_defparm;
				}
			else {

				wsprintf(MyBuf,"%s, #%u",V->name,prParm);
				PROCError(2116,MyBuf);
				}
			}

    }

	if(!_tcsncmp(V->name,"_builtin_",9)) {		// in effetti andrebbe beccato prima, e non inserita nelle VAR...
// v sopra


		}
	if(V->type & VARTYPE_FUNC_POINTER) {
		switch(V->classe) {
			case CLASSE_EXTERN:
			case CLASSE_GLOBAL:
			case CLASSE_STATIC:
				  PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
				break;
			case CLASSE_AUTO:
				PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
				break;
			case CLASSE_REGISTER:
				PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d");
				break;
			}
		if(V->modif & FUNC_MODIF_INLINE)
			;
		else
		  PROCOper(LINE_TYPE_CALL,"call ptr");
		}
	else {
		if(V->modif & FUNC_MODIF_INLINE)
			;
		else {
			i=_tcslen(outbuf)-1;
			if(outbuf[i] == ',') {
				outbuf[i]=0;
				}
			PROCOper(LINE_TYPE_CALL,V,outbuf,NULL,LINE_IS_NORMAL);
			}
		}

  if(I && !(V->attrib & FUNC_ATTRIB_NORETURN)) {
			PROCOper(LINE_TYPE_ISTRUZIONE,"ADD.d");
		}

	if(V->modif & FUNC_MODIF_INLINE) {
		struct LINE *sl=V->definition,*sl2;
		int32_t myAutoOff=AutoOff;
		if(V->attrib & FUNC_ATTRIB_NORETURN)
			PROCError(4710,"function cannot be no_return");
		CurrFunc->inlineCnt++;
		while(sl && (sl->type != LINE_TYPE_COMMENTO || _tcsnicmp(sl->rem,"----------",10))) {
			if(sl->type != LINE_TYPE_NULLA) {
				switch(sl->type) {
					case LINE_TYPE_ISTRUZIONE:
							wsprintf(MyBuf,"ret_%s_%s_%u",V->label,CurrFunc->label,CurrFunc->inlineCnt);
							PROCOutLab(MyBuf);
						break;
					case LINE_TYPE_JUMP:
					case LINE_TYPE_JUMPC:
//						if(!_tcscmp(sl->opcode,"JMP") || !_tcscmp(sl->opcode,"JR")) {		// gestire altre CPU!
//							wsprintf(MyBuf,"%s_%s_%s_%u",sl->s1.s.label,V->label,CurrFunc->label,CurrFunc->inlineCnt);
//							PROCOut(sl->type,sl->opcode,(struct OP_DEF*)MyBuf,NULL);
//							}
						break;
					case LINE_TYPE_JUMPGOTO:
							PROCOper(LINE_TYPE_JUMPGOTO,"inlinegoto");
//							}
						break;
					case LINE_TYPE_CALL:
//						if(!_tcscmp(sl->opcode,"CALL")) {		// gestire altre CPU!
						PROCOper(LINE_TYPE_CALL,"inlinecall");
//							}
						break;
					case LINE_TYPE_DATA_DEF:
//							PROCOut(sl->type,sl->s1,sl->s2/*,&sl->s3*/);
						break;
					case LINE_TYPE_LABEL:
					case LINE_TYPE_LABEL_CON_ISTRUZIONE:
							wsprintf(MyBuf,"%s_%u",sl->s1,CurrFunc->inlineCnt);
							PROCAllocGoto(MyBuf);
							PROCOutLab(MyBuf,CurrFunc->label);
						break;
					default:
						break;
					}
				}
			sl2=sl;
			sl=sl->next;
			}

		}

  
  return 0;
  }

char *CPlusMinus::getDecor(char *decor,O_TYPE Type, O_SIZE Size, O_DIM dim, struct TAGS *tag) {
	char *chp=decor;
	int8_t lp;

	lp = Type & VARTYPE_IS_POINTER;
	while(lp--)
		*chp++='P';
	if(Type & VARTYPE_FLOAT)
		*chp++='F';
	else if(Type & (VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_CLASS)) {
		_tcscat(chp,tag->label);
		}
	else {
		switch(Size) {
			case 0:
				*chp++='v';
				break;
			case 1:
				*chp++='c';
				break;
			case 2:
				*chp++='s';
				break;
			case 4:
				*chp++='i';
				break;
/*long	l	
float	f	
double	d	
bool	b	
Riferimento (&): prefisso R
Costante (const): prefisso K*/
			}
		}
	*chp=0;
	return decor;
	}

int CPlusMinus::collectParmList(char *decor) {// questa raccoglie i tipi (mangling) da una chiamata a funzione
	// entriamo DOPO la parentesi e usciamo PRIMA della parentesi
	int totParm;
  struct OPERAND R;
  union STR_LONG RCost;
  struct VARS RPtr;
	short int i;
  char Clabel[32],MyBuf[128];
	char ch[64];
	char outbuf[256];
			  
  ZeroMemory(&R,sizeof(struct OPERAND));
  ZeroMemory(&RPtr,sizeof(struct VARS));
  ZeroMemory(&RCost,sizeof(union STR_LONG));

	totParm=0;

	//*decor=0;
	*outbuf=0;

  if(*FNLA(MyBuf) != ')') {
		do {
		  R.Q=0;
		  R.size=0;
		  R.type=0l;
			R.var=&RPtr;
			R.cost=&RCost;

		  i=0;
		  FNRev(outbuf,14,&i,Clabel,&R);

		  FNGetMemSize(R.type,R.size,NULL/*dim*/,1);

			_tcscat(decor,getDecor(ch,R.type,R.size,NULL/*dim*/,R.tag));

			FNLO(MyBuf);          
			if((*MyBuf != ',') && (*MyBuf != ')')) 
				PROCError(2059,MyBuf);
			if(*MyBuf == ')') 
				FIn->unget(')');

			totParm++;

		  } while(*MyBuf != ')');

		}

	return totParm;
	}

int CPlusMinus::collectTypeList(char *decor) {  // questa raccoglie i tipi (mangling) da un prototipo o definizione
	// entriamo DOPO la parentesi e usciamo PRIMA della parentesi
	int totParm;
	short int i;
  char MyBuf[128];
	char ch[64];
	char outbuf[256];
	O_DIM dim;
	uint32_t attrib;
	O_SIZE Size;
	O_TYPE Type;
	struct TAGS *tag;
	long t;
			  
	totParm=0;

//	*decor=0;
	*outbuf=0;

  if(*FNLA(MyBuf) != ')') {

		do {

			t=FIn->GetPosition();
			FNLO(MyBuf);
//			t=FIn->GetPosition();
	//		FIn->RestorePosition(t2);

			Size=INT_SIZE;
			Type=VARTYPE_PLAIN_INT;
			tag=NULL;

			PROCGetType(outbuf,&Type,&Size,&tag,dim,&attrib,t);

			_tcscat(decor,getDecor(ch,Type,Size,dim,tag));

			do {
				FNLO(MyBuf);
				} while(iscsym(*MyBuf));
			if((*MyBuf != ',') && (*MyBuf != ')')) 
				PROCError(2059,MyBuf);
			if(*MyBuf == ')') 
				FIn->unget(')');

			totParm++;

		  } while(*MyBuf == ',');

		}

	return totParm;
	}
