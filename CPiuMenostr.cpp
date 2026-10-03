#include "stdafx.h"
#include "CPiuMeno.h"
#include "CPiuMenoTrans.h"

#include <stdlib.h>

struct LINE *CPlusMinus::PROCInserLista(struct LINE *Root, struct LINE *Last, struct LINE *New) {
  struct LINE *A;      
 
  if(Root) {
    New->next=Last->next;
    Last->next=New;
    New->prev=Last;
    if(New->next) {
      A=New->next;
      A->prev=New;
      }      
    }
  else {
    Root=New;
    Root->next=NULL;
    Root->prev=NULL;
    }
  Last=New;   
   
  return Last;
  }
 
struct LINE *CPlusMinus::PROCDelLista(struct LINE *Root, struct LINE *Last, struct LINE *l) {
  struct LINE *A;
  
  if(l==Root) {
    A=Root->next;
    if(A)
      A->prev=0;
    }
  else {
    A=l->prev;
    A->next=l->next;
    if(l->next) {
      A=l->next;
      A->prev=l->prev;
      }
//    else
    A=l->prev;
    }
  GlobalFree(l);
  if(l==Last)
    return A;
  else
    return Last;
  }
 

void CPlusMinus::swap(struct LINE * *l1, struct LINE * *l2) {
  struct LINE *t;
  
  t=*l1;
  *l1=*l2;
  *l2=t;
  }                         
    
void CPlusMinus::subObj(COutputFile *FO,const char *s) {
  
  FO->printf("%s",s);
  }


void CPlusMinus::PROCOut(uint8_t type, const char *A, const char *B, const char *C, 
												 const char *R, enum LINE_FLAGS flags) {
  struct LINE *New;
  char myBuf[128];
	COutputFile *f;

//  myLog->print(0,"istr: %s: %s,%s (%x)\n",TEXT->opcode,TEXT->s1,TEXT->s2,TEXT->type);
    switch(type & ~LINE_TYPE_COMMENTO) {  
      case LINE_TYPE_NULLA:
		    f=FO5;
        break;
			case LINE_TYPE_OTTIMIZZATA:
      case LINE_TYPE_LABEL_CON_ISTRUZIONE:
			case LINE_TYPE_JUMP:
			case LINE_TYPE_JUMPC:
			case LINE_TYPE_CALL:
			case LINE_TYPE_ISTRUZIONE:
			case LINE_TYPE_ISTRUZIONE_CONT:
			case LINE_TYPE_JUMPGOTO:
			case LINE_TYPE_FUNCTION:
				f=FO4;
        break;
			case LINE_TYPE_FUNCTION_DECLARATION:
		    f=FO5;
        break;
			case LINE_TYPE_DICHIARAZIONE:
		    f=FO5;
        break;
      case LINE_TYPE_LABEL:
		    f=FO3;
        break;
      case LINE_TYPE_DATA_DEF:
      case LINE_TYPE_DATA_DEF_CONT:
        f=FO1;
        break;
      case LINE_TYPE_DATA:
        f=FO2;
        break;
      default:
				f=FO4;
        break;
			}

    if(type & LINE_TYPE_COMMENTO) 
	    f->printf("//\t");
    switch(type & ~LINE_TYPE_COMMENTO) {  
      case LINE_TYPE_NULLA:
		    f->printf("//\t");
				subObj(f,A);
        break;
			case LINE_TYPE_OTTIMIZZATA:
				subObj(f,A);
        break;
      case LINE_TYPE_LABEL:
		    f->printf("//\t");
		    subObj(f,A);
        break;
      case LINE_TYPE_DATA_DEF:
        f->printf("%s\t",A);
				if(B)
					subObj(f,B);
				if(C) {
					f->put('\t');
					subObj(f,C);
					}
				f->put(';');
        break;
      case LINE_TYPE_DATA_DEF_CONT:
        f->printf("%s\t",A);
				if(B)
					subObj(f,B);
				if(C) {
					f->put('\t');
					subObj(f,C);
					}
        break;
      case LINE_TYPE_DATA:
        f->printf("%s\t",A);
				if(B)
					subObj(f,B);
				f->put(';');
        break;
			case LINE_TYPE_DICHIARAZIONE:
        f->printf("%s\t",A);
				if(B)
					subObj(f,B);
				f->put(';');
        break;
      case LINE_TYPE_LABEL_CON_ISTRUZIONE:
		    subObj(f,A);
				if(B)
	        f->printf("\t%s",B);
        break;
			case LINE_TYPE_JUMP:
			case LINE_TYPE_JUMPC:
			case LINE_TYPE_CALL:
	      f->put('\t'); 				  // prima delle istruzioni TAB
        f->printf("%s(",A);
				if(B)
			    subObj(f,B);
        f->printf(");");
				f->put('\t');
        break;
			case LINE_TYPE_ISTRUZIONE:
      default:
	      f->put('\t'); 				  // prima delle istruzioni TAB
        f->printf("%s",A);
				if(B) {
					f->put('\t');
					subObj(f,B);
					}
				f->put(';');
        break;
			case LINE_TYPE_ISTRUZIONE_CONT:
        f->printf("%s",A);
				if(B) {
					f->put(' ');
					subObj(f,B);
					}
				if(C)
					subObj(f,C);

        break;
			case LINE_TYPE_FUNCTION:
        f->printf("%s ",A ? A : "void");
				subObj(f,B);
				if(C)
	        f->printf("(%s)",C);
				else
	        f->printf("()",C);
	      f->printf(" {");
				break;
			case LINE_TYPE_FUNCTION_DECLARATION:
        f->printf("%s ",A ? A : "void");
				subObj(f,B);
				if(C)
	        f->printf("(%s)",C);
				else
	        f->printf("()",C);
	      f->printf(";");
				break;
			case LINE_TYPE_JUMPGOTO:
		    f->printf("%s",A);
	      f->put('\t');
				if(B)
			    subObj(f,B);
        break;
			}
    if(flags & (LINE_IS_ASSEMBLER | LINE_IS_VOLATILE)) {
	    f->printf("\t// (n/o)");
			}
    if(R) {
      if(type) {
        f->put('\t');
        f->put('\t');
        }
	    f->printf("// %s",R);
      }
    if(type & LINE_TYPE_COMMENTO) {		// c'è già nella riga che arriva da OutSource... NO!
//			if(R && R[_tcslen(R)-1] != '\n')		// in certi casi...
			  f->putcr();
			}
		else {
			if(type != LINE_TYPE_ISTRUZIONE_CONT)
			  f->putcr();
			}
    
  }
 
void CPlusMinus::PROCOut1(COutputFile *f,const char *A, const char *A1, const char *A2, const char *A3) {
  char myBuf[256];
  
  f->printf("%s",A);  
  if(A1)
    f->printf("%s",A1);  
  if(A2)
    f->printf("%s",A2);  
  if(A3)
    f->printf("%s",A3);  
  f->putcr();
  
  *myBuf=0;
  if(debug>2)
    myLog->print(0,"-------+> %s\n",myBuf);  
    
  }
                         
void CPlusMinus::PROCOper(uint8_t n, const char *A, const char *B, const char *C, const char *R, enum LINE_FLAGS flag) {
  
  PROCOut(n,A,B,C,R,flag);
  }
      
void CPlusMinus::PROCOper(uint8_t n, struct OPERANDO *op, const char *R, enum LINE_FLAGS flag) {
	char A[64];

	sprintf(A,"%s",op->s);

  PROCOut(n,A,NULL,NULL,R,flag);
  }
      
void CPlusMinus::PROCOper(uint8_t n, struct VARS *V, const char *parm, const char *R, enum LINE_FLAGS flag) {

  PROCOut(n,V->name,parm,NULL,R,flag);
  }
      
void CPlusMinus::PROCOper(uint8_t n, int v, const char *R, enum LINE_FLAGS flag) {
	char A[64];

	sprintf(A,"%d",v);

  PROCOut(n,A,NULL,NULL,R,flag);
  }
      
      
int CPlusMinus::PROCOutLab(const char *A,const char *A1,const char *A2) {
	char B[64];

	*B=0;
    
  if(A)
	  ;
	else		// potrebbe essere comodo, ma in genere se piazzo una LABEL poi la usero' di nuovo (dovrei restituire A...)
		FNGetLabel(B,2);
  if(A1 && *A1) {
    _tcscat(B,A1);
    }
  if(A2 && *A2) {
    _tcscat(B,A2);
    }
  PROCOut(LINE_TYPE_LABEL,A,B,NULL,NULL,LINE_IS_NORMAL);
  
  return 0;
  }


