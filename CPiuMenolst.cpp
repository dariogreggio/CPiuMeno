#include "stdafx.h"
#include "CPiuMeno.h"
#include "CPiuMenoTrans.h"

#include <stdlib.h>
#include <conio.h>


struct ERRORE CPlusMinus::Errs[]={
  1000,1,"unknown internal error - contact Cyberdyne",
  1001,1,"internal error:",
  1002,1,"unsupported:",
  1003,1,"error count exceeds 100; stopping compilation",
  1004,1,"unexpected EOF",
  1016,1,"#if[n]def expected an identifier",
  1017,1,"unexpected chars",
  1018,1,"unexpected #elif",
  1019,1,"unexpected #else",
  1020,1,"unexpected #endif",
  1021,1,"bad preprocessor command",
  1022,1,"expected #endif",
  1023,1,"cannot open source file",
  1024,1,"cannot open include file",
  1035,1,"expression too complex, please simplify",
  1037,1,"cannot open object file",
  1065,1,"out of tags space",
  1068,1,"cannot open file",
  1069,1,"write error on file",
#if MC68000 || ARCHI
  1126,1,"automatic allocation exceeds size (32768)" ,
#elif GD24032
  1126,1,"automatic allocation exceeds size (32768)" ,		// qua?
#else
  1126,1,"automatic allocation exceeds size (128)" /*anche 2127*/,
#endif
  2000,1,"partially unimplemented:",
  2001,1,"newline in constant",
  2007,1,"#define syntax",
  2010,1,"invalid formal list",
  2011,1,"redefinition:",
  2012,1,"bad char following include",
  2015,1,"too many chars in costant",
  2017,1,"illegal escape sequence",
  2018,1,"unknown character",
  2021,1,"invalid character",
  2025,1,"enum/struct/union type redefinition:",
  2026,1,"type redefinition:",
  2027,1,"use of undefined type",
  2030,1,"struct/union member redefinition:",
  2037,1,"left operand specifies undefined struct/union",
  2038,1,"not struct/union member of namespace:",   /* anche 2021*/
  2039,1,"not struct/union member:",
  2040,1,"different levels of indirection", /*anche 4047*/
  2041,1,"illegal digit for base",
  2043,1,"illegal break",
  2044,1,"illegal continue",
  2045,1,"label redefined",
  2046,1,"illegal case",
  2047,1,"illegal default",
  2048,1,"more than one default",
  2049,1,"case value already used:",
  2050,1,"non-integral switch expression",
  2051,1,"case expression not constant",
  2052,1,"case expression not integral",
  2053,1,"case expression too large for switch variable",
  2054,1," expected",
	2055,1, "expected formal parameter name list",
  2057,1,"expected constant expression",
  2058,1,"divide by zero",
  2059,1,"syntax error",
  2062,1,"unexpected",/*anche 2132*/
  2064,1,"not a function:",/*2063 anche ok*/
  2065,1,"undefined",			//undeclared identifier, v. anche 3861
  2068,1,"illegal cast",
  2070,1,"illegal sizeof operand",
  2071,1,"bad storage class",
  2078,1,"too many initializers",
  2079,1,"uses undefined struct/union",
  2082,1,"redefinition of formal parameter:",
  2083,1,"redundant declaration of function:",
  2084,1,"funtion already has a body:",
  2086,1,"redefinition:",/*anche 2011?*/
  2087,1,"missing subscript",
  2093,1,"can't use address of automatic variable as static init",
  2094,1,"label undefined",
  2097,1,"illegal initialization",
	2099,1,"initializer is not a constant",
  2100,1,"illegal indirection",
  2101,1,"'&' on constant",
  2103,1,"'&' on register variable",
  2104,1,"'&' on bitfield variable",
  2105,1,"needs lvalue",
  2106,1,"left operand must be lvalue",
  2109,1,"subscript on non-array",
//   2110,1,"variable used as a pointer",
  2110,1,"pointer + pointer",
  2111,1,"pointer + non-integral value",
  2112,1,"illegal use of pointer",
  2115,1,"incompatible types",
  2116,1,"function parameter list differed",
  2121,1,"bad right operand",
#if MC68000 || ARCHI
  2127,1,"stack allocation exceeds size (32768)", /*anche 1126*/
#elif GD24032
  2127,1,"stack allocation exceeds size (32768)",
#else
  2127,1,"stack allocation exceeds size (128)",
#endif
	2129,1,"static function '' declared but not defined",
  2137,1,"empty character constant",
  2141,3,"value out of range for enum"/*anche 4341*/,
  2143,1,"syntax error : missing ';' before 'type'",
  2146,1,"syntax error : missing ';' before identifier 'type'",
  2149,1,"named bitfield cannot have zero width",
  2153,1,"hex constant must have at least one digit",
  2156,1,"pragma must be outside function",
  2166,1,"l-value specifies const object",
  2187,1,"syntax error : 'void' was unexpected",
  2200,1,"warning treated as error",
  2205,1,"can't initialize extern variable",
	2215,1,"local variable '%s' in naked function '%s' allocated without stack frame",
  2221,1,"'.' left operand points to struct/union, use ->",/*anche 2231*/
  2222,1,"'->' left operand has struct/union type, use .",/*anche 2232*/
  2223,1,"left operand must point to struct/union type",/*anche 2227*/
  2224,1,"left operand must have struct/union type",/*anche 2228*/
  2275,1,"illegal use of this type as an expression",
  2297,1,"operand is illegal (not integer)",
// anche ,gemini	2301,1,"local variable '%s' in naked function '%s' allocated without stack frame"
  2352,1,"illegal call of non-static member function",
  2371,1,"redefinition (different basic types):",/*anche altri*/		// questa per funzioni
  2438,1,"cannot initialize static data member in constructor initializer list",		// anche 2649
  2440,1,"cannot convert from 'void' to ",		// e mettere il tipo :)
	2504,1,"base class undefined",
  2511,1,"overloaded member function not found in ",		// anche 2632
  2512,1,"costruttore appropriato non disponibile",
	2523,1,"destructor tag mismatch",
  2524,1,"a destructor cannot have a return type",
  2528,1,"pointer to reference is illegal",
  2533,1,"constructors cannot have a return type",
  2548,1,"missing default parameter for parameter N",
	2556,1,"overloaded function differs only by return type from",
	2561,1,"function must return a value",
	2562,1,"void function returning a value",
	2572,1,"redefinition of default argument",
	2588,1,"qualificatore di classe non valido per una dichiarazione globale",		// anche 2253 dice..
  2599,1,"local records are not supported",
  2601,1,"local functions are not supported",
  2649,1,"cannot initialize static data member in constructor initializer list",		// anche 2438
	2651,1,"a union cannot be used as a base class",
	2652,1,"a union cannot inherit from a base class",		// anche 2653 dice
  2660,1,"function does not take N arguments",
  2665,1,"none of the overloads could convert all the argument types",
  2671,1,"static member functions cannot be virtual",
  2831,1,"a destructor cannot have parameters",
   3001,1,"interrupt function returning a value",
   3002,1,"interrupt function with parms",
	3861,1,"identifier not found",		// v. 2065
  4002,1,"ignoring unknown flag",/*Microsoft D4002*/
  4005,1,"macro redefinition",
  4013,3,"function undefined; assuming extern returning int",
  4018,3,"signed/unsigned mismatch",
	4028,3,"redundant declaration of function:",		// anche 2083
	4033,1,"must return a value",
  4035,1,"function with no return value",
  4042,1,"bad storage class",
  4047,1,"different levels of indirection",
  4049,1,"indirection to different types",
  4068,1,"#pragma o attributo sconosciuto",
  4069,1,"ignorato: ",
  4091,1,"'struct ': ignored on left of 'type' when no variable is declared",
  4098,1,"void function returning a value",		// anche 2562 qua
  4099,1,"void type invalid",
  4101,3,"unreferenced local variable",
  4102,3,"unreferenced label",
  4127,4,"conditional expression is constant",/*anche 4727*/
  4131,4,"old-style declaration",
  4133,2,"incompatible types",
	4172,3,"returning address of local variable or temporary",
	4244,3,"conversion, truncation, possible loss of data",
  4305,3,"truncation from ",
  4309,3,"truncation of constant value",
	4430,4,"missing type specifier - int assumed",
  4701,3,"local variable used without initialization",
  4705,4,"statement has no effect",
  4710,4,"function not inlined",
	4715,3,"not all control paths return a value",
  4761,3,"integral size mismatch in argument; conversion supplied",
  0,0,NULL
  };

  
int CPlusMinus::PROCError(int Er, const char *a) {
  int i;
  char myBuf[256],errBuf[256];

  wsprintf(errBuf,"%s: (%d): errore %d",__file__,__line__,Er);
  i=0;
  while (Er != Errs[i].t && Errs[i].t)
    i++;
  if(Errs[i].t) {
  	wsprintf(myBuf,": %s",Errs[i].s);
		_tcscat(errBuf,myBuf);
 	  if(a)
 	    wsprintf(myBuf,": %s",a);
 	  else  
 	    wsprintf(myBuf,".");
		_tcscat(errBuf,myBuf);
		if(FErr) {
			FErr->println(errBuf);
			if(FIn)	// se errore DOPO la compilazione
				FNGetLine(FIn->GetPosition(),myBuf);			// SISTEMARE posizione...
			FErr->println(myBuf);
			}
	  if(debug) {
	    PROCV("vartmp.map");
//	    PROCT();
	    }
    }
	else	{
	  wsprintf(myBuf,"(%d)",Er);
    PROCError(1000,myBuf);
    }
/*  if(FPre)
    fclose(FPre);
  if(FObj)
    fclose(FObj);
  if(FO1)
    fclose(FO1);
  if(FO2)
    fclose(FO2);
  if(FO3)
    fclose(FO3);
  if(FLst) {
    FLst->Close();
		delete FLst;
		FLst=NULL;
		}*/
	// NON dovremmo uscire al primo errore... forse
	//
	bExit=1;
	// per gestire la valanga di errori, fare poi così:
	//1. Panic-Mode Recovery (La tecnica dei "Punti di Ancoraggio", cercare ; (fine istruzione) o } (fine blocco scope) o ) (fine lista parametri)
	//2. Flag di Suppressione (panic_mode / Suppressed Errors) dopo il primo errore e fino a resync
	//3. Il "Soffitto" degli Errori (Error Limit)
	//4. Bilanciamento delle Parentesi (Brace Matching Tracking)

	if(!panicMode) {
		numErrors++;
		if(myOutput) {
			char *p=(LPSTR)GlobalAlloc(GPTR,256);
			_tcscpy(p,errBuf);
			myOutput->PostMessage(WM_ADDTEXT,1,(LPARAM)p);
			}
		}
	else	
		panicMode=TRUE;
//	throw; 
  return 0;
  }

int CPlusMinus::PROCWarn(int Er, const char *a) {
  int i;
  char myBuf[256],errBuf[256];

  if(Warning) {
		numWarnings++;
	  i=0;
    while (Er != Errs[i].t && Errs[i].t)
      i++;
    if(Errs[i].t) {  
		  if(Warning<0 || Errs[i].l <= Warning) {
			  wsprintf(errBuf,"%s: (%d): warning %d",__file__,__line__,Er);
			  wsprintf(myBuf,": %s %s",Errs[i].s,a ? a : ".");
				_tcscat(errBuf,myBuf);
				{
				char *p=(LPSTR)GlobalAlloc(GPTR,256);
				_tcscpy(p,errBuf);
				myOutput->PostMessage(WM_ADDTEXT,2,(LPARAM)p);
				}
				if(FErr)
					FErr->println(errBuf);
		    if(Warning<0)
		      PROCError(2200,NULL);
			  }
			}
		else	
      PROCError(1000,NULL);
    
    }
  return 0;
  }

int CPlusMinus::PROCV(const char *n) {
  int i;
  char *B;
	char myBuf[512];
  COutputFile *FO;
  struct VARS *V;
  struct CONS *C;
  
  B=CSourceFile::FNTrasfNome((char *)n);
  FO=new COutputFile(B);
  if(!FO) 
    PROCError(1069,OUS);
//  FO=stderr;  
  V=Var;
  while(V) {
		FO->printf("Ogg. %s\t\tTipo %x\tLabel %s\tSize %d\tdi %s\n",
			V->name,V->type,V->label,V->size,V->func.func ? V->func.func->name : "");
    FO->printf("Blocco attuale: %d, blocco var: %d\n",InBlock,V->block);
		if(V->isInTag) {
      FO->printf("\tFa parte del TAG: %s\n",V->isInTag->label);
			}
		if(V->isInTag) {
      FO->printf("\tIl suo TAG e': %s\n",V->isInTag->label);
			}
    V=V->next;
    }
  FO->putcr();
  C=Con;
  while(C) {
    FO->printf("Cost. %s\t\tLabel %s\n",C->name,C->label);
    C=C->next;
    }
  delete FO;
  return 0;
  }

int CPlusMinus::PROCT() {
  int t;
	char myBuf[256];
  
  for(t=0; t<MaxTypes; t++) {
    myLog->print(1,"Tipo %s:\t\t%lx\t\tSize: %x\t\tTag: %s\n",
			Types[t].s,Types[t].type,Types[t].size,Types[t].tag ? Types[t].tag->label : "");
    }
    
  return 0;
  }

int CPlusMinus::PROCD() {
  struct LINE_DEF *t;
  
  t=m_CPre->RootDef;
  while(t) {
    printf("%s \t\t\tè %s",t->name,t->text);
    t=t->next;
    }
    
  return 0;
  }
  
int CPlusMinus::PROCVarList(COutputFile *FO, struct VARS *func, struct VARS *Vroot) {
  /*static ??*/ int T=0;
  int I,i;
	int bs1,bs2;
  char *p;
  char myBuf[256];
  struct VARS *V;
	struct TAGS *C;
  
  if(func) {
    FO->printf("%s: variabili locali\n",func->name);
		}
  else {
    FO->println("\fVariabili globali");
		}
  V=Var;
  while(V) {
    if(V->func.func==func) {
			if(!(T % 60)) {
				FO->printf("\n%32s%10s%16s%10s%12s%8s%12s\n\n","Nome","Classe","Tipo","Dim.","Offset/Registro","Vis.","Tag");
				}
			FO->printf("%32s",V->name);

	//    i=26-strlen(Var[T].name)/2;
	//    while(i--)
	//      FO->put('.',FO);
	//    FO->printf("\t");
			switch(V->classe) {
				case CLASSE_EXTERN:
					p="extern";
					break;
				case CLASSE_GLOBAL:
					p="global";
					break;
				case CLASSE_STATIC:
					p="static";
					break;
				case CLASSE_AUTO:
					i=0;
					if(i<0)
						p="auto";
					else 
						p="param";
					break;
				case CLASSE_REGISTER:
					p="register";
					break;
				case CLASSE_MEMBER:
					p="member";
					break;
				case CLASSE_MEMBER_STATIC:
					p="st. member";
					break;
				case CLASSE_MEMBER_VIRTUAL:
					p="virt.member";
					break;
				}
			FO->printf("%12s",p);
	//	  FO->printf("\t");

			if(V->type & VARTYPE_FUNC) {
				if(V->type & VARTYPE_FUNC_POINTER)
					p="funct/ptr";
				else
					p="function";
				}
			// VA TUTTO RIVISTO, i tipi possono miscelarsi...

			else if(V->type & VARTYPE_ARRAY) {
				p=myBuf;
				int j=0;
				while(V->dim[j] && j<MAX_DIM)
					j++;
				sprintf(myBuf,"array[%u]",j);
				}
			else if(V->type & VARTYPE_IS_POINTER) 
				p="pointer";
			else if(V->type & VARTYPE_IS_REFERENCE) 
				p="reference";		// in teoria può essere anche reference a pointer...
			else {
				if(V->type & (VARTYPE_CLASS))
					p="class";
				else if(V->type & (VARTYPE_UNION | VARTYPE_STRUCT))
					p="struct/union";
				else if(V->type & VARTYPE_FLOAT)
					p="float";
				else if(V->type & VARTYPE_BITFIELD)
					p="bitfield";
				else {             
					p=myBuf;
					if(V->type & VARTYPE_UNSIGNED)
						_tcscpy(myBuf,"unsigned ");
					else
						*myBuf=0;
					if(V->size == INT_SIZE) {
						_tcscat(myBuf,"int ");
						}
					else {  
						switch(V->size) {
							case 1:
								_tcscat(myBuf,"char ");
								break;
							case 2:
								_tcscat(myBuf,"short ");
								break;
							case 4:
								_tcscat(myBuf,"long ");
								break;
							} 
						}
					if(V->type & VARTYPE_FAR)
						_tcscat(myBuf,"far ");
					}

				}
			FO->printf("%16s",p);

	//	  FO->printf("\t");
			if(V->type & VARTYPE_FUNC) {
				p=myBuf;
//				p="***";
				sprintf(myBuf,"%u %c",V->size,V->type & VARTYPE_POINTER ? '*' : ' ');
				}
			else {
				p=myBuf;
				if(V->type & VARTYPE_ARRAY)
					sprintf(myBuf,"%u",FNGetArraySize(V));
				else {
					if(V->type & (VARTYPE_STRUCT | VARTYPE_UNION | VARTYPE_CLASS))
						sprintf(myBuf,"%u",V->size);
					else if(V->type & VARTYPE_BITFIELD) {
						int j;
						j=FNGetAggr2(NULL,V,&bs1,&bs2);
						sprintf(myBuf,"%u",bs2);
						}
					else {
						if(V->type & VARTYPE_IS_POINTER)
							sprintf(myBuf,"%u",getPtrSize(V->type));
						else
							sprintf(myBuf,"%u",V->size);
						}
					}
				}
			FO->printf("%9s",p);

			if(V->type & VARTYPE_BITFIELD) {
				sprintf(myBuf,"%u",bs1);
				}
			else {
				if(V->classe==CLASSE_AUTO) {
					p=myBuf;
					I=0;
					sprintf(myBuf,"%d",I);
					}
				else {
					p="***";
  				}
				}
			FO->printf("%7s",p);

			if(V->classe==CLASSE_REGISTER) {

				}
			else
				*myBuf=0;

			_tcscat(myBuf,"            ");

			if(V->classe >= CLASSE_MEMBER) {
				switch(V->visibility) {
					case VIS_PUBLIC:
						_tcscat(myBuf," public    ");
						break;
					case VIS_PROTECTED:
						_tcscat(myBuf," protected ");
						break;
					case VIS_PRIVATE:
						_tcscat(myBuf," private   ");
						break;
					}
				}
			else
				_tcscat(myBuf,"     -       ");

			if(V->isInTag) {
				_tcscat(myBuf,"\t\t(di:");
				_tcscat(myBuf,V->isInTag->label);
				_tcscat(myBuf,")");
				}
			if(V->hasTag) {
				_tcscat(myBuf,"\t\t");
				_tcscat(myBuf,V->hasTag->label);
				}
		

			FO->println(myBuf);
			T++;
			if(!(T % 60)) {
				FO->put('\f');
				}
			}
    V=V->next;
    }
	FO->putcr();

	if(!func && !Vroot) {
		T=0;
    FO->println("\fOggetti aggregati");
    C=StrTag;
    while(C) {
			FO->printf("%32s %s\n",C->label,C->member ? C->member->label : "");
      C=C->next;
      } 
		}

	FO->putcr();
  
  return 0;
  }
  
char *CPlusMinus::OpCond[16]={		// v. OPERANDO_CONDIZIONALE , ne servono solo 6 (logicamente!  NO! direi 10, per unsigned/signed
  "LT","GE","LE","GT","EQ","NE", "C","NC","LS","HI"		// 
  };
