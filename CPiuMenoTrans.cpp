/*  ***************************
	  *   C-TRANSPILER          *
		*        BY G.Dar         *
		*            26/9/26      *
		**************************/

// la dim di un oggetti aggregato dichiarato senza la parolina class struct ecc non è corretta
//ora dclvar controlla il mangling quindi forse possiam togliere alcune cercavar prima...
// n.b. const int MAX_SIZE = 100; ha linkage static in C++ mentre extern in C 
// si possono aver dtor virtuali - inserirli in vptr


#include "stdafx.h"
#include "CPiuMeno.h"
#include "CPiuMenoTrans.h"

#include <mmsystem.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <ctype.h>

struct complex {
//public:
	//int re,im;
;
	};

struct OPERANDO CPlusMinus::Op[]={
  "(",1,")",1,"[",1,"]",1,".",1,"->",1,"::",1,      // 7
  "*",2,"&",2,"++",2,"--",2,"~",2,"sizeof",2,"(",2,"!",2,"+",2,"-",2, // 10
  "*",3,"/",3,"%",3, // 3
  "+",4,"-",4,   // 2
  "<<",5,">>",5,   // 2
  "<",6,">",6,"<=",6,">=",6,   // 4
  "==",7,"!=",7,   //2
  "&",8,   // 1
  "^",9,   // 1
  "|",10,  // 1
  "&&",11,  // 1
  "||",12,  // 1
  "?",13,  // 1
  "=",14,"+=",14,"-=",14,"*=",14,"/=",14,"%=",14,">>=",14,"<<=",14,"&=",14,"^=",14,"|=",14,  // 11
	// lascio un buco =15 per poter fare alcune differenze in FNRev
  ",",16,  // 1
  };         
  
struct TIPI CPlusMinus::Types[50]={
  "void",0,0/* servirebbe un tipo diverso...*/,NULL,{0},
  "char",1,VARTYPE_PLAIN_INT,NULL,{0},
  "short",2,VARTYPE_PLAIN_INT,NULL,{0},
  "int",INT_SIZE,VARTYPE_PLAIN_INT,NULL,{0},
  "long",4,VARTYPE_PLAIN_INT,NULL,{0},
  "float",4,VARTYPE_FLOAT,NULL,{0},
  "double",8,VARTYPE_FLOAT,NULL,{0}
  };        

#define MAX_ERRORS 100


char *CPlusMinus::ctor="_ctor";
char *CPlusMinus::dtor="_dtor";
char *CPlusMinus::vptr="__vptr";
char *CPlusMinus::the_this="this";
char *CPlusMinus::to_mangle="__";
char *CPlusMinus::ptr_to_base="__base.";
char *CPlusMinus::ptr_to_base2="__base";
char *CPlusMinus::main_name="main";
char *CPlusMinus::struct_type="struct";
char *CPlusMinus::malloc_name="malloc",*CPlusMinus::free_name="free";

CPlusMinus::CPlusMinus(const CWnd *v) {

	myOutput=(CWnd *)v;
	myLog=new CLogFile("c:\\cpiumenoSpool.txt");
	}

CPlusMinus::~CPlusMinus() {

	delete myLog;
	}

int CPlusMinus::CompilaCpp(int argc, char **argv) {
  char fpr[256];                 // nome del file prep
  int i,ch;
  char ARGS[128],myBuf[256];
  char *p;
  struct VARS *V;
	uint32_t startTime;



#if _DEBUG
  Warning=3;
#else
  Warning=1;
#endif
// FLAGS:
  PreProcOnly=FALSE;
	PreProcCommenti=FALSE;
  CheckStack=FALSE;
	UsesMalloc=FALSE;
  OutSource=FALSE;
  OutList=FALSE;
  OutAsm=FALSE;
  SynCheckOnly=FALSE;
	StructPacking=4;		// per dword-align
	TipoOut=0;
  Optimize=0;           // bit 0=jump, bit 1=subexpr., 2=inlinecalls, b4=o1 (costanti), b8-9 size - speed
	OptimizeExpr=0;				// verificare...
  NoMacro=FALSE;

	InBlock=0;
	DefaultVisibility=VIS_PUBLIC;

  Brack=isRValue=isPtrUsed=inCast=0;
	GlblOut=NULL;

  __STDC__=0;
  debug=0;
	bExit=0;
	MaxTypes=7;		*Types[MaxTypes].s=0;		// e pulisco !
  LABEL=0;
	*NFS=*OUS=0;
	*__file__=0;
	__line__=0;

	Var=LVars=NULL;
	c_mode=0;
	CurrFunc=NULL;
	CurrFuncGotos=NULL;
	FuncReturnedValue=FALSE;
	Con=LCons=NULL;
  Enums=LEnums=NULL;
	StrTag=LTag=NULL;
	RootIn=NULL;
	FIn=NULL;
	FPre=NULL;
	FO1=FO2=FO3=FO4=FO5=NULL;
	FObj=FCod=NULL;
	FLst=FErr=NULL;

  Declaring=FuncCalled=SaveFP=ASM=AutoOff=0;

#ifdef _DEBUG
//	debug=3;
#endif

	/*char *zz=0;
	*zz=0;*/
	m_CPre=new CCPreProcessor(this,PreProcCommenti,myLog,0);		// passare parametro per attivare commenti in .i ecc!

//  try {

  for(i=1; i<argc; i++) {
//AfxMessageBox(argv[i]);
		if(*argv[i]=='-' || *argv[i]=='/') {
		  switch(*(argv[i]+1)) {
				case '?':
				  exit(99);                   // help
				  break;
				case 'D':
				  p=strchr(argv[i],'=');
				  if(p) {
					_tcsncpy(buffer,argv[i]+2,p-argv[i]-2);
					m_CPre->PROCDefine(buffer,p+1);
					}   
				  break;
				case 'd':
				  debug=1;		// 2 per FNLO
				  break;
				case 'E':
				  PreProcOnly=1;
				  break;
				case 'C':		// gemini dixit 14/9/26  SUPERMORTE A TUTTI
				  PreProcCommenti=1;
				  break;
				case 'F':
				  switch(*(argv[i]+2)) {
						case 'c':
							OutSource=TRUE;
							break;
						case 'a':
							OutAsm=TRUE;         // assembly listing
							break;
						case 'l':
							OutList=TRUE;
							break;
						case 'o':
							_tcscpy(OUS,argv[i]+4);         
							break;
						default:
							goto ukswitch;
							break;
						}
				  break;
				case 'G':
				   switch(*(argv[i]+2)) {
					  case 's':
							CheckStack=FALSE;
							break;
					  case 'e':
							CheckStack=TRUE;
							break;
					  default:
							goto ukswitch;
							break;
					  }     
				  break;
				case 'J':
      		m_CPre->PROCDefine("_CHAR_UNSIGNED","1");
				  break;
				case 'O':
				  switch(*(argv[i]+2)) {
						case 't':               // speed
						  Optimize|=OPTIMIZE_SIZE;
						  break;
						case 's':               // size
						  Optimize|=OPTIMIZE_SPEED;
						  break;
						case 'i':
						  Optimize|=OPTIMIZE_INLINECALLS;
						  break;
						case 'g':
						  Optimize|=OPTIMIZE_SUBEXPR;
						  break;
						case 'l':         // sarebbe ottimizza loop, noi lo usiamo per i salti
						  Optimize|=OPTIMIZE_JUMP;


//						  Optimize|=OPTIMIZE_SUBEXPR;
// prove 31/8/26


						  break;
						case 'x':
						  Optimize=0xffff;
						  break;
						case '0':
						  Optimize=0x00;
						  break;
						case '1':         // per le costanti...?? usare, gestire!
						  Optimize|=OPTIMIZE_CONST;
						  break;
						case '2':
						  break;
						case '3':
						  Optimize=0xffff;
						  break;
						}   
				  break;
				case 'P':
				  PreProcOnly=2;
				  break;
				case 'S':
				   switch(*(argv[i]+2)) {
					  case 't':
											  // imposta titolo
						break;
					  default:
							goto ukswitch;
						break;
					  }     
				  break;
				case 'u':
				  NoMacro=TRUE;
				  break;
				case 'w':
				  Warning=0;
				  break;
				case 'W':
				  switch(*(argv[i]+2)) {
						case 'X':
						  Warning=-1;
						  break;
						case '1':
						case '2':
						case '3':
						case '4':
						  Warning=*(argv[i]+2) - '0';
						  break;
						default:
						  goto ukswitch;
						  break;
						}     
				  
				  break;
				case 'Z':
				  switch(*(argv[i]+2)) {
					  default:
							goto ukswitch;
							break;
					  }     
				  break;
				default:
ukswitch:
				  PROCWarn(4002,argv[i]+1);
				  break;
				}
//		  *argv[i]=0;			// boh perché?? 2025
		  }
		else {
		  if(*argv[i]) { 
				if(!*NFS)
					_tcscpy(NFS,argv[i]);
				else
					PROCWarn(1017,argv[i]);		// diciamo :)  ma si schianta per lunghezza o boh
			  }
		  }
		}
  
	numErrors=numWarnings=0;
	panicMode=FALSE;

	m_CPre->setLasciaCommenti(PreProcCommenti);

	PROCInit();
  
  if(!*NFS) {
		char *p=(LPSTR)GlobalAlloc(GPTR,256);
		_tcscpy(p,"Sintassi: ccpp <nomefile> [switches]");
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		goto fine;
		}
  if(!*OUS)
		_tcscpy(OUS,NFS);
	CSourceFile::AddExt(OUS,"lst");

  _tcscpy(__file__,NFS);
  if(p=strrchr(__file__,'\\'))
    _tcscpy(__name__,p+1);
  else  
    _tcscpy(__name__,__file__);
  if(p=strchr(__name__,'.'))
    *p=0;
  myLog->print(0,"%s - %s\n",__date__,__file__);
  InBlock=0;
  CurrFunc=NULL;
	CurrFuncGotos=NULL;  
	FuncReturnedValue=FALSE;
	Declaring=TRUE;
  FuncCalled=FALSE;
	UseIRQ=UseFloat=FALSE;
  SaveFP=FALSE;
  AutoOff=0;
  m_CPre->PP=TRUE;            // PROCESSA O NO
	__line__=0;

	startTime=timeGetTime();

	if(myOutput) {
		p=(LPSTR)GlobalAlloc(GPTR,256);
		_tcscpy(p,NFS);
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		}

	if(myOutput) {
		p=(LPSTR)GlobalAlloc(GPTR,256);
		_tcscpy(p,"Preprocessore...");
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		}
  if(PreProcOnly==1) {
		m_CPre->FNLeggiFile(NFS,new COutputFile(stdout),0);
		return 0;
		}
  _tcscpy(fpr,NFS);
  CSourceFile::AddExt(fpr,"i");
  if(!(FPre=new COutputFile(fpr)))
		PROCError(1069,fpr);
	
	m_CPre->setDebugLevel(debug);
  i=m_CPre->FNLeggiFile(NFS,FPre,0);
  delete FPre;	FPre=NULL;
	if(!i)
		goto fine;

  if(PreProcOnly) {
		if(myOutput) {
			char *p=(LPSTR)GlobalAlloc(GPTR,256);
			wsprintf(p,"%s (preprocessato) - %d errori, %d warning (%u mSec)",fpr,numErrors,numWarnings,
				timeGetTime()-startTime);
			myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
			}
		return 0;
		}
	
	if(myOutput) {
		p=(LPSTR)GlobalAlloc(GPTR,256);
		_tcscpy(p,"Compilatore...");
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		}
  _tcscpy(__file__,NFS);
  if(!(FIn=new CSourceFile(fpr)))
		PROCError(1068,fpr);


	{char tempPath[256];
	GetTempPath(255,tempPath);
	GetTempFileNameA(tempPath,"CPlusMinus",0,myBuf);
  if(!(FO1=new COutputFile(myBuf)))				// DATA
		PROCError(1069);
	GetTempFileNameA(tempPath,"CPlusMinus",0,myBuf);
  if(!(FO2=new COutputFile(myBuf)))				// BSS
		PROCError(1069);
	GetTempFileNameA(tempPath,"CPlusMinus",0,myBuf);
  if(!(FO3=new COutputFile(myBuf)))				// CONST
		PROCError(1069);
	GetTempFileNameA(tempPath,"CPlusMinus",0,myBuf);
  if(!(FO4=new COutputFile(myBuf)))				// CODE
		PROCError(1069);
	GetTempFileNameA(tempPath,"CPlusMinus",0,myBuf);
  if(!(FO5=new COutputFile(myBuf)))				// EXTERN (per praticità di gestione
		PROCError(1069);
	}
/*¦ FO5 (EXTERN / INCLUDES / PROTOTIPI)                                    ¦
	¦   #include <stdio.h>, struct Shape, typedefs, prototipi mangled...     ¦
	+------------------------------------------------------------------------¦
	¦ FO3 (CONST / STRINGS)                                                  ¦
	¦   const char* str_1 = "Hello World";                                   ¦
	+------------------------------------------------------------------------¦
	¦ FO1 (DATA - Inizializzati)                                             ¦
	¦   int globalCount = 100;                                               ¦
	+------------------------------------------------------------------------¦
	¦ FO2 (BSS / Struct Instances)                                           ¦
	¦   Point ptGlobal;                                                      ¦
	+------------------------------------------------------------------------¦
	¦ FO4 (CODE - Metodi e Funzioni)                                         ¦
	¦   void Shape_print__(Shape* this) { ... }                              ¦
	¦   int main() { ... }

Concatenare su disco FO5 + FO3 + FO1 + FO2 + FO4
	*/

  if(OutList) {
		CSourceFile::AddExt(OUS,"map");
		FLst=new COutputFile(OUS);
    if(!FLst) 
	  	PROCError(1037,OUS);
//		PROCVarList(OUS);
    }
	CSourceFile::AddExt(OUS,"err");
	FErr=new COutputFile(OUS);
  if(!FErr) 
  	PROCError(1037,OUS);
	
  PROCOper(LINE_TYPE_COMMENTO,"");
//  StaticOut=LastOut->prev;
//  BSSOut=StaticOut;
	PROCOut1(FO3,";",NULL);

  FErr->printf("G.Dar transpiler v%u.%02u on %s %s\n\n",HIBYTE(__VER__),LOBYTE(__VER__),__date__,__time__);

  __line__=1;


//	try {		// NON va e non si pianta su eccezioni in PROCOper ecc... forse dipende dal chiamante, OpenC?
  do {
		MSG msg;

		FNLA(ARGS);
		
		if(debug) 
			myLog->print(0,ARGS);
		  
		if(panicMode) {			// METTERE ANCHE IN PROCBLOCK!! e togliere bexit di là
			FNLO(ARGS);
			if(*ARGS == '}' || *ARGS == ';')
				panicMode=FALSE;
			}
		else {
			if(*ARGS == '{') {
				FNLO(ARGS);
				PROCBlock();
				}
			else if(*ARGS==';') {		// cmq :)
				FNLO(ARGS);
	  		}
			else {
				if(*ARGS) {
					if(!FNIsStmt()) 
						PROCIsDecl();
	//				__line__++;
					}
				else {
					FNLO(ARGS);
					}
				} 
			}

		BOOL bMsgAvail=PeekMessage(&msg,NULL,0,0,PM_REMOVE /*| PM_NOYIELD*/);
		// serve per far comparire i messaggi nella finestra OpenC man mano che li posto!
		if(bMsgAvail) {
			if(msg.message == WM_QUIT)		// ovvero usarne uno speciale per fermare la compilazione...
		  	break;
			TranslateMessage(&msg); 	 /* Translates virtual key codes			 */
			DispatchMessage(&msg);		 /* Dispatches message to window			 */
			}

		if(numErrors >= MAX_ERRORS) {
      PROCError(1003,"error count exceeds 100; stopping compilation\n");		// MAX_ERRORS
      bExit=TRUE; // O interrompi il parsing dell'AST; v. PROCError cmq
			}
		} while(!FIn->Eof() && !bExit);
		/*}
	catch(CException e) {
		PROCError(1001,"exception");
		}*/
  if(InBlock>0)
		PROCError(1004);

//		PROCV("vartmp.map");

	delete FIn; FIn=NULL;
	CFile::Remove(fpr);
	delete m_CPre;

	CSourceFile::AddExt(OUS,"c");
  if(!(FObj=new COutputFile(OUS)))
		PROCError(1037,OUS);
  FObj->printf("// *** Generated by G.Dar C++ transpiler v%u.%02u on %s %s\n",HIBYTE(__VER__),LOBYTE(__VER__),__date__,__time__);
		FObj->printf("// [** DEBUG **]\n");
	if(debug)
		FObj->printf("// (debug level %u)\n\n",debug);
	else
		FObj->putcr();

	FObj->printf("// Command line: ");
  for(i=2; i<argc; i++) {
		FObj->printf("%s; ",argv[i]);
		}
	FObj->putcr(); FObj->putcr();

	FObj->printf("//\tTITLE\t%s\n\n",__file__);


  V=Var;
  while(V) {
		if(!V->isInTag) {
			if(V->classe==CLASSE_GLOBAL) {
				if(!(V->type & (VARTYPE_FUNC_BODY | VARTYPE_FUNC))) {
					FO5->printf("//PUBLIC\t_%s\n",V->name);
			  	}
			  }	
			}  
		V=V->next;  
		}
	// in effetti MSVC mette le extern in cima e le public nel code... ma vabbe'
  FO5->putcr();

  V=Var;
  while(V) {
		if(!V->isInTag) {
			if((V->classe == CLASSE_EXTERN && (!(V->type & VARTYPE_FUNC) || (V->type & VARTYPE_FUNC_USED))) || 
				(V->classe == CLASSE_EXTERN && !(V->type & VARTYPE_FUNC_POINTER)) || 
				(V->classe==CLASSE_GLOBAL && ((V->type & (VARTYPE_FUNC_BODY | VARTYPE_FUNC_USED | VARTYPE_FUNC)) == (VARTYPE_FUNC | VARTYPE_FUNC_USED)))) {
			  FO5->printf("//EXTRN\t%c%s\n",'_',V->name);
			  }
			if((V->classe == CLASSE_STATIC && V->type & VARTYPE_FUNC && !(V->type & VARTYPE_FUNC_BODY)))
				PROCError(2129,V->name);

			}			// tag

skippa_var:
		V=V->next;  
		}

  FO5->printf("//EXTRN\t__stktop\n");
	// ev. per 64bit

  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"");
	PROCOper(LINE_TYPE_FUNCTION,"void","__global_constructors",NULL, NULL,LINE_IS_NORMAL);
	// (inserire tutte istanze classi globali o statiche
	InBlock=1;		// per formattazione!
	struct VARS *v;
	v=Var;		// 
	while(v) {		// creo tutti gli oggetti globali
		if(v->type & VARTYPE_CLASS && v->hasTag && v->classe<CLASSE_AUTO) {	
			char MyBuf2[128],*p1;

			if(v->decor) {
				p1=_tcschr(v->decor,'(');		// v. sopra, è il marker per mangling
				if(p1)
					*p1++=0;
				}
			else
				p1=NULL;
			_tcscpy(myBuf,v->hasTag->label);		// 
			_tcscat(myBuf,ctor);
			_tcscat(myBuf,to_mangle);
			if(v->decor)
				_tcscat(myBuf,v->decor);
			_tcscpy(MyBuf2,"&");
			_tcscat(MyBuf2,v->name);
			if(p1) {
				_tcscat(MyBuf2,",");
				_tcscat(MyBuf2,p1);
				}
			if(v->decor)
				GlobalFree(v->decor);

			PROCOper(LINE_TYPE_CALL,myBuf,MyBuf2,NULL,v->name);

			}
		v=v->next;
		}
  PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"}\n");
	PROCOper(LINE_TYPE_FUNCTION,"void","__global_destructors",NULL, NULL,LINE_IS_NORMAL);
	v=Var;		// 
	while(v) {		// distruggo tutti gli oggetti globali
		if(v->type & VARTYPE_CLASS && v->hasTag && v->classe<CLASSE_AUTO) {	
			char MyBuf2[128];

			_tcscpy(myBuf,v->hasTag->label);		// 
			_tcscat(myBuf,dtor);
			_tcscat(myBuf,to_mangle);
			_tcscpy(MyBuf2,"&");
			_tcscat(MyBuf2,v->name);

			PROCOper(LINE_TYPE_CALL,myBuf,MyBuf2,NULL,v->name);

			}
		v=v->next;
		}
  PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"}\n");
	InBlock=0;		// per formattazione!


  FO1->Seek(0l,CFile::begin);  
  FO2->Seek(0l,CFile::begin);  
  FO3->Seek(0l,CFile::begin);  
  FO4->Seek(0l,CFile::begin);
  FO5->Seek(0l,CFile::begin);

	if(!(TipoOut & TIPO_SPECIALE)) {	// sarebbe per MC68000 ma in effetti può valere sempre, tipo se MemoryModel =COMPACT o simile
		FObj->printf("\n//****(EXTERN / INCLUDES / PROTOTIPI)\n",NULL);		
		if(UsesMalloc)
			FObj->printf("#include <stdlib.h>\t\t// per new/delete/malloc/free");		// FARE SOLO SE USATO
		while((ch=FO5->get()) != EOF)			// (EXTERN / INCLUDES / PROTOTIPI)
			FObj->put(ch);
		FObj->printf("\n//**(CONST / STRINGS)\n",NULL);		
		while((ch=FO3->get()) != EOF)			// FO3 (CONST / STRINGS)
			FObj->put(ch);
		FObj->printf("\n//**(DATA - Inizializzati)\n",NULL);		
		while((ch=FO1->get()) != EOF)			// (DATA - Inizializzati)
			FObj->put(ch);
		FObj->printf("\n//**(BSS / Struct Instances)\n",NULL);
		while((ch=FO2->get()) != EOF)			// (BSS / Struct Instances)
			FObj->put(ch);
		FObj->printf("\n//**(CODE - Metodi e Funzioni)\n");
		while((ch=FO4->get()) != EOF)			// (CODE - Metodi e Funzioni)
			FObj->put(ch);
		FObj->printf("\n//**** fine traduzione\n\n");
		}		// Tipo Speciale
	else {
		FObj->printf("//\n****(EXTERN / INCLUDES / PROTOTIPI)\n",NULL);		
		while((ch=FO5->get()) != EOF)			// (EXTERN / INCLUDES / PROTOTIPI)
			FObj->put(ch);
		FObj->printf("\n//**(CONST / STRINGS)\n",NULL);		
		while((ch=FO3->get()) != EOF)			// FO3 (CONST / STRINGS)
			FObj->put(ch);
		FObj->printf("\n//**(DATA - Inizializzati)\n",NULL);		
		while((ch=FO1->get()) != EOF)			// (DATA - Inizializzati)
			FObj->put(ch);
		FObj->printf("\n//**(BSS / Struct Instances)\n",NULL);
		while((ch=FO2->get()) != EOF)			// (BSS / Struct Instances)
			FObj->put(ch);
		FObj->printf("\n//**(CODE - Metodi e Funzioni)\n");
		while((ch=FO4->get()) != EOF)			// (CODE - Metodi e Funzioni)
			FObj->put(ch);

		FObj->printf("\n//**** fine traduzione\n\n");
		}
	FO5->Close(); CFile::Remove(FO5->GetFilePath()); delete FO5;
	FO4->Close(); CFile::Remove(FO4->GetFilePath()); delete FO4;
	FO3->Close(); CFile::Remove(FO3->GetFilePath()); delete FO3;
	FO2->Close(); CFile::Remove(FO2->GetFilePath()); delete FO2;
	FO1->Close(); CFile::Remove(FO1->GetFilePath()); delete FO1;

//  PROCV("temp.map");
  if(FLst) {
    PROCVarList(FLst,(struct VARS*)NULL);
		delete FLst;		FLst=NULL;
    }

	LVars=Var;
	while(LVars) {
		Var=LVars;
		if(Var->definition) {
			struct LINE *t;
			while(Var->definition) {
				t=Var->definition->next;
				GlobalFree(Var->definition);
				Var->definition=t;
				}
			}
	  if(Var->type & VARTYPE_FUNC) 			// anche Pointer
			GlobalFree(Var->parm.ptr);
		if(Var->decor)
			GlobalFree(Var->decor);
		LVars=LVars->next;
		GlobalFree(Var);
		}

	LTag=StrTag;
	while(LTag) {
		StrTag=LTag;
		if(StrTag->friends) {
			struct TAGS *t;
			while(StrTag->friends) {
				t=StrTag->friends->next;
				GlobalFree(StrTag->friends);
				StrTag->friends=t;
				}
			}
		if(StrTag->member) {
			struct VARS *v;
			while(StrTag->member) {
				v=StrTag->member->next;
				GlobalFree(StrTag->member);
				StrTag->member=v;
				}
			}
		LTag=LTag->next;
		GlobalFree(StrTag);
		}

	if(FErr)
		delete FErr;

//		}
//	catch(...) {
//   AfxMessageBox("exce");
//    }
	if(myOutput) 	{
		p=(LPSTR)GlobalAlloc(GPTR,256);
		wsprintf(p,"%s - %d errori, %d warning (%u righe, %u mSec)",OUS,numErrors,numWarnings,FObj->getTotalLines(),
			timeGetTime()-startTime);
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		}

//	PROCT();
	delete FObj; FObj=NULL;
  return 1;

fine:
	return 0;
  } 
	  
int CPlusMinus::PROCBlock() {
  int I,i,j,m,M;
  char ARG[128];
  long OT;
	int ol;
  char MyBuf[128];
  struct VARS *v;
  char *p;
  
	if(panicMode) {			// v. anche sopra, aspettare ; o }
		return InBlock;
		}

	if(!InBlock)
		AutoOff = 0;

  InBlock++;
	if(InBlock >= MAX_BLOCCHI)
		PROCError(1002,"blocchi nidificati");
	OldTX[InBlock].AutoOff = min(OldTX[InBlock-1].AutoOff,OldTX[InBlock].AutoOff);
	OldTX[InBlock].flag=0;
  I=InBlock;    
  do {
		FIn->SavePosition();
		OT=FIn->GetPosition();
		ol=__line__;
		FNLA(ARG);
	
		if(debug) 
	  	myLog->print(0,">>>%s<<<",ARG);
	  
		switch(*ARG) {
		  case 0:                    // fine riga o fine FILE
				FNLO(ARG);
//    		PROCError(1004);
//				__line__++;
		    break;
		  case '{':
				FNLO(ARG);
				Declaring=TRUE;
				PROCBlock();
				break;
		  case ';':
				FNLO(ARG);
				break;
		  case '}':
				FNLO(ARG);
//				__line__++;
				if(c_mode && c_mode==InBlock) {
					c_mode=0;
					InBlock--;
					return 0;
					}
				else 
					goto end_block;
				break;
		  default:
				FIn->RestorePosition(OT);
				__line__=ol;
				if(!FNIsStmt()) 
				  PROCIsDecl();
//				__line__++;
				break;
		  }
		} while(*ARG != '}' && !FIn->Eof());

end_block:
  if(!I || FIn->Eof())			// bah sarebbe da gestire meglio la } finale
		PROCError(2054,"}");

	AutoOff = min(OldTX[I].AutoOff,AutoOff);
  if(I>1) {
//		OldTX[I-1].AutoOff = min(OldTX[I].AutoOff,OldTX[I-1].AutoOff);
		OldTX[I].AutoOff = 0;
		switch(*OldTX[I].T) {
		  case 0:

				break;
	  	case '&':			// se switch(
			  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"}","fine switch");
 
				*OldTX[I].T=0;
				*OldTX[I].B=0;
				*OldTX[I].C=0;
				break;
	  	case '#':		// se do while
				if(_tcscmp(FNLA(MyBuf),"while"))
		  		PROCError(2054,"while");
			  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"}","fine while");
				break;
	  	case '%':		// se if then else
				if(_tcscmp(FNLA(MyBuf),"else")) {
//          *OLDT[I]=0;
//          *OLDB[I]=0;
//          *OLDC[I]=0;
					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"}","fine else");
		  		}           
				break;
	  	default:
//					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"fine block");
	
				break;
	  	}                          
		}
  else {
		if(!CurrFunc) {
			PROCError(/*2062*/1001,"inBlock=0, function=NULL");
			return -1;
			}
		AutoOff &= -STACK_ITEM_SIZE;
		if(!_tcscmp(CurrFunc->name,main_name)) {
				}
//    myLog->print("eccomi %d",CurrFunc);
//  	PROCV("vartmp.map");
		if(CurrFunc->modif & FUNC_MODIF_INTERRUPT && !(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
			PROCOper(LINE_TYPE_ISTRUZIONE,"interrupt");


	  	}
		/*else beh no*/ if(SaveFP || AutoOff) {
			if(!(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
//		myLog->print("SaveFP: %d, AutoOff %d, FuncCalled %d, Checkstack %d",SaveFP,AutoOff,FuncCalled,CheckStack);
				}
			}

		if(AutoOff) {
			if(!(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
		  	if(abs(AutoOff) > 32767)		// VERIFICARE
					PROCError(1126);
	  	if(CheckStack) {
			  PROCOper(LINE_TYPE_ISTRUZIONE,"checkstack");
				v=FNCercaVar("_chkstk",FALSE);
				}		// checkstack
	  	else {
			  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"ENTER");
					if(OutSource) {
						wsprintf(MyBuf,"\t ossia %u bytes",abs(AutoOff));
						PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,MyBuf);
//						PROCOut1(FO2,MyBuf,NULL);
						}
				}
				}		// naked
	  	}			// AutoOff != 0
		else {
			if(!(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
			if(SaveFP)
			  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"ENTER");
	  	}		// naked
			}

//		if(Reg<Regs->MaxUser && !(CurrFunc->modif & (FUNC_MODIF_FASTCALL | FUNC_MODIF_INLINE))) {		
	//  	wsprintf(MyBuf,"{R%u-R%u}",Reg,Regs->MaxUser-1);
	 // 	PROCOper(LINE_TYPE_ISTRUZIONE,"STM.d"/*pushString*/,
	//			OPDEF_MODE_STACKPOINTER_INDIRETTO,-1,OPDEF_MODE_REGISTRI,(union SUB_OP_DEF*)MyBuf,0);
	 // 	}
//		if(*OldTX[1].T)
	//	  PROCOutLab(OldTX[1].T);
		if(SaveFP || AutoOff) {
			if(!(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
 	  		PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"LEAVE");
		  	}		// naked
	  	}

		if(CurrFunc->modif & FUNC_MODIF_INTERRUPT) {
			if(!(CurrFunc->attrib & FUNC_ATTRIB_NAKED)) {
			  PROCOper(LINE_TYPE_ISTRUZIONE,"LDM");
				PROCOper(LINE_TYPE_ISTRUZIONE,"LDST");
			  }

				PROCOper(LINE_TYPE_ISTRUZIONE,"RETI");

		  }

		if(!(CurrFunc->modif & FUNC_MODIF_INTERRUPT)) {
//		  PROCOper(LINE_TYPE_ISTRUZIONE,"\n\treturn");
			}

	  PROCOper(LINE_TYPE_LABEL_CON_ISTRUZIONE,"__return_end:\n;");

		/*
		struct VARS *v,*limit=NULL,*lastFound=NULL;		// OCCHIO METTERE PRIMA DI EVENTUALE return ESPLICITO!
		do {
			lastFound=NULL;
			v=Var;
			while(v && v != limit) {
				//(ANDREBBE in ordine inverso!
				if(v->type & VARTYPE_CLASS && v->hasTag && v->func.func==CurrFunc && v->classe==CLASSE_AUTO
					) {	// (per le static farlo insieme alle global alla fine
					lastFound=v;
					}
				v=v->next;
				}
			if(lastFound) {
				char MyBuf2[128];

				_tcscpy(MyBuf,lastFound->hasTag->label);		// 
				_tcscat(MyBuf,dtor);
				_tcscat(MyBuf,to_mangle);
				_tcscpy(MyBuf2,"&");
				_tcscat(MyBuf2,lastFound->name);

				PROCOper(LINE_TYPE_CALL,MyBuf,MyBuf2,NULL,lastFound->name);
				}
			limit = lastFound;
			} while(lastFound);
*/

			v=LVars;
			while(v) {
				//in ordine inverso!
				if(v->type & VARTYPE_CLASS && v->hasTag && v->func.func==CurrFunc && v->classe==CLASSE_AUTO
					) {	// (per le static farlo insieme alle global alla fine
					char MyBuf2[128];

					_tcscpy(MyBuf,v->hasTag->label);		// 
					_tcscat(MyBuf,dtor);
					_tcscat(MyBuf,to_mangle);
					_tcscpy(MyBuf2,"&");
					_tcscat(MyBuf2,v->name);

					PROCOper(LINE_TYPE_CALL,MyBuf,MyBuf2,NULL,v->name);
					}
				v=v->prev;
				}

		if(!_tcscmp(CurrFunc->name,main_name)) {		// idem
			PROCOper(LINE_TYPE_CALL,"__global_destructors",NULL,NULL,NULL,LINE_IS_NORMAL);
			PROCOper(LINE_TYPE_FUNCTION_DECLARATION,"void","__global_destructors",NULL,NULL,LINE_IS_NORMAL);
			}

		if(CurrFunc->size>0)
			PROCOper(LINE_TYPE_ISTRUZIONE,"return __retval");

//	  PROCOper(LINE_TYPE_DATA_DEF,CurrFunc->label);
	  PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"}\n");
//		PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"endfunction");
//		if(Reg<Regs->MaxUser && !(CurrFunc->modif & (FUNC_MODIF_FASTCALL | FUNC_MODIF_INLINE))) {
//	  	wsprintf(MyBuf,"{R%u-R%u}",Reg,Regs->MaxUser-1);
//	  	PROCOper(LINE_TYPE_ISTRUZIONE,"LDM.d"/*popString*/,OPDEF_MODE_STACKPOINTER_INDIRETTO,+1,
	//			OPDEF_MODE_REGISTRI,(union SUB_OP_DEF*)MyBuf,0);
	//  	}

		*OldTX[1].T=0;
		OldTX[1].AutoOff=0;
	  OldTX[1].id = __line__;		// v. altre, cmq ok qua
		AutoOff=0;
		Declaring=TRUE;		// v. C89 C99 :)
		SaveFP=FALSE;

//		Ottimizza(RootOut);
		// fare ricorsivamente fino a quando non ci sono più miglioramenti! 2026

		if(CurrFunc->modif & FUNC_MODIF_INLINE) {		// salvo il contenuto della inline! prima che venga flushata
			struct LINE *sl1=CurrFunc->definition,*sl2;
			i=0; j=0;
			CurrFunc->definition=sl2=(struct LINE*)GlobalAlloc(GPTR,sizeof(struct LINE));
			while(sl1 && (sl1->type != LINE_TYPE_COMMENTO || _tcsnicmp(sl1->rem,"----------",10))) {
#if 0
//				PROCOut(sl->type,sl->opcode,&sl->s1,&sl->s2/*,&sl->s3*/);
				sl2->type=sl1->type;
				_tcscpy(sl2->opcode,sl1->opcode);
				sl2->s1=sl1->s1;
				sl2->s2=sl1->s2;
				_tcscpy(sl2->rem,sl1->rem);
				sl2->next=(struct LINE*)GlobalAlloc(GPTR,sizeof(struct LINE));
				sl2=sl2->next;
				sl1=sl1->next;
				i++;
				if(i>25 && !j) {		// diciamo :)
					PROCWarn(4710,"function too large for inlining");
					j=1;
					}
#endif
				}
			}

		if(OutList) {
			PROCVarList(FLst,CurrFunc);
			}
		// cancella le variabili locali... POI USARE il members di Vars; andrebbe modificato VARS nel caso...
		/*{ struct VARS *V,*v;
		V=Var;
		while(V) {
			v=V->next;
			if(V->func.func==CurrFunc && V->classe>=CLASSE_STATIC && V->classe<=CLASSE_REGISTER) {
        if (V->prev)
          V->prev->next = V->next;
        else
          Var = V->next; // V era la testa! Ora la nuova testa è il successivo
        if(V->next)
          V->next->prev = V->prev;
	      GlobalFree(V);
				}
			V=v;
			}
		}*/
		{ struct VARS *V,*v;
		V=CurrFunc->members;
		while(V) {
			v=V->next;
			GlobalFree(V);
			V=v;
			}
		}
		while(CurrFuncGotos) {			    // cancello i goto
			struct VARS *g=CurrFuncGotos->next;
			GlobalFree(CurrFuncGotos);
			CurrFuncGotos=g;
			}
		CurrFuncGotos=NULL;

		if(FNGetMemSize(CurrFunc,1) && !FuncReturnedValue)
	  	PROCError(2561);
		CurrFunc=NULL;
		FuncReturnedValue=FALSE;
		}
  InBlock--;     
					  
  return InBlock;
  }
  
int CPlusMinus::PROCIsDecl() {
// Class= 0 SE EXTERN, 1 SE GLOBAL, 2 SE STATIC, 3 SE AUTO, 4 SE REGISTER e poi CLASS_MAMBER_xxx
  int v,t,i,f;
	O_DIM dim={0};
	O_SIZE size=INT_SIZE;
	enum VAR_CLASSES Class;
  O_TYPE type=VARTYPE_PLAIN_INT;
	uint8_t modif=0l;
	uint32_t attrib=0;
	struct VARS *newVar;
  struct TAGS *tag=NULL;
  char *p;
  long OldTextp,OldTextp2;
  char T[64],MyBuf[256 /*STR_LONG ...*/];
  int ol;
	bool is_unsigned=FALSE;
	char outbuf[256];
  
  Class=InBlock>0 ? CLASSE_AUTO : CLASSE_GLOBAL;     // CLASSE DI DEFAULT: GLOBAL O AUTO
  OldTextp=FIn->GetPosition();
	FIn->SavePosition();
	ol=__line__;
  FNLO(T);                    // LEGGO UN ITEM

  OldTextp2=FIn->GetPosition();

  v=-1;
  do {
		i=FNIsClass(T);             // per gestire interrupt, pascal ecc.
		if(i>=0) {
		  if(i>=16) {
				modif |= i >> 4;
				}
		  else {
				v=i;
				if(i==CLASSE_GLOBAL)			// gestisco const, v.
					type |= VARTYPE_CONST;
			  }
		  OldTextp2=FIn->GetPosition();
// v.sotto			ol=__line__;
		  FNLO(T);                 
		  }
		else {
			if(i==-1) {
				FIn->RestorePosition(OldTextp2);
				__line__=ol;
				}
			else
				return 0;
			}
		} while(i>=0);
	/*if(!_tcscmp(T,"const"))	{	// GESTIRE! usare ; v. anche di là
		type |= VARTYPE_CONST;
	  OldTextp2=FIn->GetPosition();
	  FNLO(T);                 
		}*/
  t=FNIsType(T);

  if(/*(m==0) && */(v<0) && (t==VARTYPE_NOTYPE) ) {
		tag=FNCercaAggr(T,FALSE);
		if(tag) {		// è una dichiarazione (senza "class" o "struct" ecc
			char MyBuf2[64],nome[MAX_NAME_LEN+1];
			struct VARS *V;
			struct TAGS *inBase;

			if(tag->type==2) {
				type=VARTYPE_CLASS;
				do {

					_tcscpy(MyBuf,tag->label);
					long tt=FIn->GetPosition();
					FNLO(nome);
rifo_tag_ptr:
					if(!_tcscmp(nome,".")) {
						PROCError(2275,nome);
						}
					else if(!_tcscmp(nome,"*")) {		// fa schifo... andrebbe usato GetType...
						type = (type & ~VARTYPE_IS_POINTER) | ((type & VARTYPE_IS_POINTER) + 1);
						FNLO(nome);
						goto rifo_tag_ptr;
						}
					else if(!_tcscmp(nome,"::")) {
						FIn->RestorePosition(OldTextp);
						*outbuf=0;
						FNEvalExpr(outbuf,16,MyBuf);
						PROCOper(LINE_TYPE_ISTRUZIONE,outbuf,NULL,NULL,"da ProcIsDecl",LINE_IS_NORMAL);
						}
					else {
						if(!(type & VARTYPE_IS_POINTER)) {
							_tcscat(MyBuf,ctor);
	// no qua						newVar=FNCercaCtor(tag,MyBuf,TRUE,&inBase);
							newVar=FNCercaVar(nome,TRUE,&inBase);
							if(newVar)
								PROCError(2086,MyBuf2);
							else {
								if(*FNLA(MyBuf2) == '(') {			// costruttore var senza parola "class" o struct
									PROCCheck('(');
									*MyBuf2=0;
									collectParmList(MyBuf2);
									}
								else
									*MyBuf2=0;
								FIn->RestorePosition(tt);
	inherit:
								_tcscat(MyBuf,to_mangle);
								_tcscat(MyBuf,MyBuf2);
								V=FNCercaVar(MyBuf,FALSE,&inBase);
								if(V) {
									newVar=PROCAllocVar(nome,VARTYPE_CLASS,Class,type,4,tag,NULL/*dim*/);	// cercare size della class
				//					newVar=PROCDclVar(Class,modif,VARTYPE_CLASS,size,tag,dim,attrib,FALSE,NULL);
									if(*MyBuf2) {
										newVar->decor=(char*)GlobalAlloc(GPTR,256);
										_tcscpy(newVar->decor,MyBuf2);		// salvo qua per chiamare costruttore adatto in global!
										}
									PROCOper(LINE_TYPE_ISTRUZIONE,"struct",tag->label,nome,";");
									FNLO(nome);
									if(!(type & VARTYPE_IS_POINTER)) {
										if(*FNLA(MyBuf2) == '(') {
											PROCCheck('(');
											if(CurrFunc) {		// se è locale...
												PROCUsaFun(outbuf,V,0x80,nome);
												PROCOper(LINE_TYPE_CALL,V,outbuf,NULL,LINE_IS_NORMAL);
												}
											else {		// ..altrimenti
												PROCUsaFun(outbuf,V,0x80,nome);
												if(newVar->decor) {
													_tcscat(newVar->decor,"(");		// metto separè
													p=_tcschr(outbuf,',');		// qua salto il "this" messo da usafun
													_tcscat(newVar->decor,p ? p+1 : outbuf);
													}
												}
											}
										else {
											if(1)		// SOLO SE subclass  __base
												wsprintf(MyBuf,"(struct %s*)&%s",V->isInTag->label,newVar->name);
											else
												wsprintf(MyBuf,"&%s",newVar->name);
											PROCOper(LINE_TYPE_CALL,V,MyBuf,NULL,LINE_IS_NORMAL);
											}
										}
									}
								else {
	/*NO!	si fa solo per quello di default, che quindi esiste di SICURO							if(tag->parent) {
										//if(visibility			// fare

										_tcscpy(MyBuf,tag->parent->label);
										_tcscat(MyBuf,ctor);
										goto inherit;
										}
									else*/
										PROCError(2512,nome);
										return 0;
									}
								}
							}		// not pointer
						else {
							if(*FNLA(MyBuf2) == '(') 			// 
								PROCError(2059,MyBuf2);
							V=FNCercaVar(nome,TRUE,&inBase);
							if(!V)
								newVar=PROCAllocVar(nome,type,Class,0,type & VARTYPE_IS_POINTER ? getPtrSize(0) : 4,tag,NULL/*dim*/);	// cercare size della class
							else
								PROCError(2011,nome);
							i=type & VARTYPE_IS_POINTER;
							*MyBuf=0;
							while(i--)
								_tcscat(MyBuf,"*");
							_tcscat(MyBuf,nome);
							PROCOper(LINE_TYPE_ISTRUZIONE,"struct",tag->label,MyBuf,";");
							}

						}
					FNLO(MyBuf);
					} while(*MyBuf==',');       // ev altri

				}
			else {
				do {
							if(c_mode)
								;
					newVar=PROCDclVar(outbuf,Class,modif,tag->type==1 ? VARTYPE_STRUCT : VARTYPE_UNION,size,tag,dim,attrib,FALSE,
						NULL,NULL);
					FNLO(MyBuf);
					} while(*MyBuf==',');       // ev altri

				}

			return 0;
			}
		}

  if(/*(m==0) && */(v<0) && (t==VARTYPE_NOTYPE) && (InBlock>0)) {
// SE NON SPECIFICA LA CLASSE, NE IL TIPO E SIAMO IN UN BLOCCO...



		if(Declaring) {
		  PROCOper(LINE_TYPE_COMMENTO,"+---------------------------------------");    // SEPARA LE DICHIARAZIONI DAL RESTO DELLA FUNZIONE
		  Declaring=FALSE;     
		  }
		if(OutSource) {
		  wsprintf(MyBuf,"<%5u>: ",__line__);
		  FNGetLine(OldTextp,MyBuf+9);		// 
//			MyBuf[_tcslen(MyBuf)-1]=0;		// tolgo CR se no diventa doppio
		  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_DATA_DEF,MyBuf);
		  }
		FIn->RestorePosition(OldTextp);
		__line__=ol;
		*outbuf=0;
		FNEvalExpr(outbuf,16,MyBuf);
		PROCOper(LINE_TYPE_ISTRUZIONE,outbuf);
// ...ALLORA E' UN'ESPRESSIONE
		PROCCheck(';');
//		__line__=ol;
		}
  else {
		if(OutSource) {
		  wsprintf(MyBuf,"<%5u>: ",__line__);
		  FNGetLine(OldTextp,MyBuf+9);		// 
//			MyBuf[_tcslen(MyBuf)-1]=0;		// tolgo CR se no diventa doppio
		  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_DATA_DEF,MyBuf);		// VA SPOSTATA SE è UN PROTOTIPO! in TYPE_DICHIARAZIONE
		  }
		if(1   || Declaring) {
// cpp   (SE SI PUO' ANCORA DICHIARARE... v. C89 C99
			*outbuf=0;
		  if(v>=0 || modif>0) {
				OldTextp=FIn->GetPosition();
				FNLO(T);
				if(v>=0)
				  Class=(enum VAR_CLASSES)v;
				}           // SE SI SPECIFICAVA UNA CLASSE LEGGI IL PROSSIMO ITEM
			if(FNIsType(T)==VARTYPE_NOTYPE) {
//			  SE NON SI SPECIFICAVA (O SI SPECIFICA) UN TIPO TORNA INDIETRO (errore, qua
				FIn->RestorePosition(OldTextp);
				__line__=ol;
				PROCError(2143/*4430*/,"type",T);		// errore in C++ (anche se VC98 lo prendeva come int, v. anche altrove
				}

			if(!_tcscmp(T,"unsigned") || !_tcscmp(T,"signed")) {
//				  OldTextp=FIn->GetPosition();
				}                  
			if(!_tcscmp(T,"unsigned")) {
				is_unsigned=TRUE;		// lo salvo... è meglio che spostare il puntatore!
				}                  
			if(type & (VARTYPE_UNSIGNED | VARTYPE_UNION | VARTYPE_STRUCT | VARTYPE_CLASS)) {
//			  Var[Vars+1].tag=Var[Vars].tag;     // non si capisce...
				type &= VARTYPE_NOT_A_POINTER;
				}                  
			else {
				size=INT_SIZE;
				type |= VARTYPE_PLAIN_INT;		// vabbe' :)
				}
			f=PROCGetType(outbuf,&type,&size,&tag,dim,&attrib,OldTextp);	// qua risolviamo cmq la cosa che eventualmente segue i ::

			FNLA(MyBuf);
			{struct TAGS *tag2;
			if(!InBlock && (tag2=FNCercaAggr(MyBuf,FALSE))) {		// questo è per la dichiarazioni di robe delle classi al livello esterno
				tag=tag2;
				FNLO(T);
				PROCCheck("::");		// obbligatorio dunque!
				do {
					if(type & VARTYPE_FUNC) {
						long OT;
						bool is_ctor=FALSE,is_dtor=FALSE;		// UNIRE con la dichiarazione di classe...
						char decor[128],T1S[128],mangle[64];
						struct TAGS *inBase;

						OT=FIn->GetPosition();
						FNLO(T1S);
						if(!_tcscmp(T1S,tag->label)) {		// costruttore
  						is_ctor=TRUE;
							}
						else if(!_tcscmp(T1S,"~")) {		// distruttore
  						is_dtor=TRUE;
							OT=FIn->GetPosition();
							FNLO(T1S);
							if(_tcscmp(T,tag->label))
								PROCError(2523,T1S);
							}
						FIn->RestorePosition(OT);

						if(is_ctor)
							_tcscpy(decor,ctor);
						else if(is_dtor)
							_tcscpy(decor,dtor);
						else 
							_tcscpy(decor,tag->label);

						if(is_ctor || is_dtor) {
		//					PROCError(2533); ctor
			//				PROCError(2524); dtor
							size=SIZE_NULL;
		// NO, c'è funzione!					t=TYPE_NULL;
							}

						_tcscpy(MyBuf,tag->label);
						FNLO(T1S);
						if(is_ctor || is_dtor)
							_tcscat(MyBuf,decor);
						else {
							_tcscat(MyBuf,"_");
							_tcscat(MyBuf,T1S);
							}
						_tcscat(MyBuf,to_mangle);
		//				FIn->RestorePosition(OT);
						PROCCheck('(');
						collectTypeList(mangle);
						_tcscat(MyBuf,mangle);
		//				if(!FNCercaVar(MyBuf,FALSE,&inBase)) {		// 
						if(!FNCercaVar(tag,MyBuf,&inBase)) {		// 
							PROCError(2039,T); 
							return 0;
							}
						FIn->RestorePosition(OT);

						Class=CLASSE_MEMBER;
//			      newVar=PROCAllocVar(MyBuf,type,Class,0,size,tag,dim);
//				FNLO(MyBuf);
			//			PROCOper(LINE_TYPE_DATA_DEF,"",outbuf,MyBuf,MyBuf,LINE_IS_NORMAL);
						newVar=PROCDclVar(outbuf,Class,modif,type,size,tag,dim,attrib,FALSE,T,mangle);

						FNLO(MyBuf);
		//				FNLO(MyBuf);
						if(*MyBuf == ':') {		// ma ci passa davvero di qua?? v. dclvar con fun
							_tcscpy(MyBuf,tag->parent->label);
							_tcscat(MyBuf,ctor);
							_tcscat(MyBuf,to_mangle);
							_tcscat(MyBuf,mangle);
							wsprintf(T1S,"(struct %s*)&this->%s",tag->parent->label,ptr_to_base2);
							InBlock++;		// per formattazione!
							PROCOper(LINE_TYPE_CALL,MyBuf,T1S,NULL,"chiamo altro",LINE_IS_NORMAL);

							if(FNHasVirtual(tag)) {
								wsprintf(T1S,"((struct %s*)this)->%s = (const struct %s_VTable*)&__vftable_%s",
									tag->parent->label,vptr,tag->parent->label,tag->label);				// Assegna vtable di Shape
								PROCOper(LINE_TYPE_ISTRUZIONE,T1S,NULL,NULL,"x x virtual",LINE_IS_NORMAL);
								}

							InBlock--;

							}
						else if(*MyBuf == '{') {
							Declaring=TRUE;

							PROCBlock();
							break;		// una sola funzione per volta!
							}
						else {
							PROCError(2588,T);		// NON può essere un prototipo
							break;		// 
							}
						}
					else {
						struct TAGS *inBase;
						struct VARS *v;
						char T1S[64];

						FNLA(MyBuf);
						if(!(v=FNCercaVar(tag,MyBuf /*T*/,&inBase))) {		// 
							PROCError(2039,MyBuf); 
							return 0;
							}
						if(v->type != type || v->size != size) {
							PROCError(2371,MyBuf); 
							return 0;
							}
						Class=CLASSE_MEMBER_STATIC;
// no!						newVar=PROCDclVar(outbuf,Class,modif,type,size,tag,dim,attrib,FALSE,NULL/*T1S*/,NULL);
						_tcscpy(T1S,T);
						_tcscat(T,"_");
						_tcscat(T,MyBuf);
						FNLO(MyBuf);
						FNLO(MyBuf);
						v->type |= VARTYPE_INITIALIZED;
						*T1S=0;
						if(*MyBuf=='=') {
							_tcscat(T1S,MyBuf);
							do {
								FNLO(MyBuf);
								if(*MyBuf != ';')
									_tcscat(T1S,MyBuf);
								} while(*MyBuf && *MyBuf != ';');
							FIn->unget(';');
							}
						PROCOper(LINE_TYPE_DATA,outbuf,T,T1S,NULL,LINE_IS_NORMAL);
						break;
						}
					if(iscsym(*MyBuf))
						PROCError(2059,MyBuf);		// se c'è subito un identificatore o cmq!
					} while(*MyBuf==',');       // oppure tante var statiche
				return 0;

			}
			}

			if(type & VARTYPE_FUNC && Class == CLASSE_GLOBAL)		// ev. poi cambiato sotto
				Class = CLASSE_EXTERN;
			if(!_tcscmp(T,"short")/* || !_tcscmp(T,"signed")*/) {
			  FNLA(T);
				if(FNIsType(T)==VARTYPE_PLAIN_INT && !type) {		// PATCH rapida per "short int" ecc... MIGLIORARE
				  FNLO(T);
					}
				}

			goto primogiro;		// perché ho già l'ev. ptr qua! v.sotto, migliorare

		  do {
				
		if(debug) {
			char *tmp=(char*)(tag ? tag->label : "");/*NON CI PIACE @#£$% si incasina la printf del log...*/
			myLog->print(0,"TIPO: t=%x, c=%x, s=%x, tag=%s, dim=%d",type,Class,size,tmp,dim); 
			}

				type &= VARTYPE_NOT_A_POINTER;
				i=type & VARTYPE_ARRAY ? 0 : 0;		// gli array sono sempre anche puntatori, minimo
				while(*FNLA(MyBuf)=='*') {		// sarebbe da gestire in GetType... qua il * non appartiene al tipo dichiarato a inizio riga ma per ciascuno...
					FNLO(MyBuf);
					i++;
					}
				type=i;
				OldTextp=FIn->GetPosition();

				subGetType(&type, &size, dim, OldTextp);

primogiro:
				if(is_unsigned)
					type |= VARTYPE_UNSIGNED;

	//			if(InBlock>0)
//					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"",outbuf,NULL,NULL,LINE_IS_NORMAL);
				if(type & VARTYPE_FUNC && !c_mode) {
					long l2=FIn->GetPosition();
					char T1S[64];
					struct TAGS *inBase;

					FNLO(MyBuf);
					PROCCheck('(');
					if(f)	{	// qua dovrebbe essere sicuro
						*MyBuf=0;
						collectTypeList(MyBuf);
				//				__line__=ol;
						FIn->RestorePosition(l2);
						if(tag) {
							FNLO(T1S);
							if(!FNCercaCtor(tag,MyBuf,TRUE,&inBase))
								PROCError(2512,T1S);
							FIn->RestorePosition(l2);
							}
						newVar=PROCDclVar(outbuf,Class,modif,type,size,tag,dim,attrib,FALSE,NULL,MyBuf);
						if(*MyBuf) {
							newVar->decor=(char*)GlobalAlloc(GPTR,256);
							_tcscpy(newVar->decor,MyBuf);		// salvo qua per chiamare costruttore adatto in global!
							}
						}
					}
				else {
					long l2=FIn->GetPosition();
					char T1S[64];
					struct TAGS *inBase;

					if(f) {
						if(tag && tag->type==2) {
							FNLO(T1S);
							if(*FNLA(MyBuf) != ';') {
								FNLO(MyBuf);
								PROCCheck('(');
								collectTypeList(MyBuf);
				//				__line__=ol;
								}
							if(!FNCercaCtor(tag,MyBuf,TRUE,&inBase))
								PROCError(2512,T1S);
							FIn->RestorePosition(l2);
							}
						else
							*MyBuf=0;
						newVar=PROCDclVar(outbuf,Class,modif,type,size,tag,dim,attrib,FALSE,NULL,NULL);
						if(*MyBuf && newVar) {
							newVar->decor=(char*)GlobalAlloc(GPTR,256);
							_tcscpy(newVar->decor,MyBuf);		// salvo qua per chiamare costruttore adatto in global!
							}
						}
					}
				FNLO(MyBuf);
				if(iscsym(*MyBuf))
					PROCError(2059,MyBuf);		// se c'è subito un identificatore o cmq!
				} while(*MyBuf==',');       // TUTTA UNA SERIE DI DICHIARAZIONI
		  }
		else {
		  PROCWarn(2143,"mixing declarations and code is incompatible with standards before C99");		// QUA FORSE NO!
//		  PROCError(2062,"type"); o anche 2143 "...before type"
		  }
		      // qui non è bello... tiene conto delle funz
		if(*MyBuf=='{') {
			if(newVar->type & VARTYPE_FUNC) {
//				if(newVar->classe < CLASSE_GLOBAL) messo sopra, v.
//					newVar->classe = CLASSE_GLOBAL;
				}
      FIn->unget('{');
			}
		else {
		  if(*MyBuf != ';')
		    PROCCheck(';');
			else
				__line__++;		// non va bene se ci sono più cose sulla stessa riga...
		  }  
		}      
  return 0;
  }
 
struct VARS *CPlusMinus::PROCDclVar(char *outbuf,enum VAR_CLASSES Class, uint8_t Modif, O_TYPE Type, O_SIZE Size, struct TAGS *tag, O_DIM dim, 
										uint32_t attrib, bool isParm, const char *decor, const char *mangle) {
  int v,t1,T2,i,f;
  char T[128],nome[256],S[128],MyBuf[256 /*sizeof(union STR_LONG)*/];
  long OldTextp,t,t2,BaseTextP=0;
  char *p;
  struct VARS *V,*newvar;
	int ol;
	int8_t ndim=0;
	bool isInitialized,dontAllocate=0;
	char varType[128];
  
  *S=0;
  OldTextp=FIn->GetPosition();
	ol=__line__;
  FNLO(nome);             // LEGGO UN ITEM
  // PRINT "Classe:"Class%" Tipo:"~type%" Size:"Size%" Dim:"dim%
	if(!iscsymf(*nome)) {		// ma OCCHiO che filtra anche new sizeof e delete!! rifinire

		if(FNGetOperatorFunc(nome,nome))
			;
		else if(!_tcscmp(nome,"sizeof")) {
			PROCError(2833,nome);
			return 0;
			}
		else if(*nome=='@')	{	// bah finezza... ampliare
			PROCError(2018,nome);
			return 0;
			}
		else {
			PROCError(2059,nome);
			return 0;
			}
		}

	if(_tcscmp(nome,main_name)) {	// eccezione (ehhh porcodio
		if(decor) {
			if(!_tcscmp(decor,ctor)) {
				_tcscat(nome,decor);
				if(mangle) {
					_tcscat(nome,to_mangle);
					_tcscat(nome,mangle);
					}
				_tcscpy(T,decor);
				}
			else if(!_tcscmp(decor,dtor)) {
				_tcscat(nome,decor);
				if(mangle) {
					_tcscat(nome,to_mangle);
					_tcscat(nome,mangle);
					}
				_tcscpy(T,decor);
				}
			else {
				_tcscpy(S,decor);
				_tcscat(S,"_");
				_tcscat(S,nome);
				if(mangle) {
					_tcscat(S,to_mangle);
					_tcscat(S,mangle);
					}
				_tcscpy(T,nome);
				_tcscpy(nome,S);
				}
	/*		if(mangle) {
				_tcscat(T,to_mangle);
				_tcscat(T,mangle);
				}*/
			}
		else if(mangle) {		// se funz globale e NON siamo in "C"
			_tcscpy(T,nome);
			_tcscat(nome,to_mangle);
			_tcscat(nome,mangle);
			}
		else
			_tcscpy(T,nome);
		}
	else
		_tcscpy(T,nome);
  V=FNCercaVar(nome,TRUE);
	// SE LA VARIABILE GIA' ESISTE... (nel blocco)
/*  if(V) {
//		if((Size != V->size) || ((V->classe>CLASSE_EXTERN) && (Class != V->classe))) 
		if(V->classe>CLASSE_EXTERN && Class >= V->classe)
	  	PROCError(2086,nome);
// SE LA DIMENSIONE E' DIVERSA O LA CLASSE, ERRORE
		} non è ancora ok per le static/global , ok gemini 2026*/
	if(V) {
		if(decor) {
			if(!_tcscmp(decor,ctor) || !_tcscmp(decor,dtor)) {
				if((V->type & ~(VARTYPE_FUNC | VARTYPE_FUNC_USED | VARTYPE_FUNC_BODY)) || V->size)		// non dovrebbe accettare manco void
					PROCError(2831);
				}
			}
		if(V->classe > CLASSE_EXTERN) {
			// 1. Se le classi non coincidono (es. STATIC vs GLOBAL) -> ERRORE
			if(Class != V->classe) {
				if(V->type & VARTYPE_FUNC) {
					// se prototipo DOPO la funzione, ossia cazzata :D  anche se VC98 accetta cmq, gemini dice di no
					if(((V->type & ~(VARTYPE_FUNC | VARTYPE_FUNC_USED | VARTYPE_FUNC_BODY)) 
						!= (Type & ~(VARTYPE_FUNC | VARTYPE_FUNC_USED | VARTYPE_FUNC_BODY)))	// e bisognerebbe controllare pure tutti i parametri...
						|| V->size != Size)
						PROCError(2084,nome);
					}
				else
					PROCError(2086,nome);
				}
			// 2. Se la classe è la stessa ma cambiano tipo o dimensione (es. long vs float) -> ERRORE
			else if((V->type & ~(VARTYPE_FUNC | VARTYPE_FUNC_USED) != Type) || V->size != Size)
				PROCError(V->type & VARTYPE_FUNC ? 2371 : 2086,nome);
			else if(!(V->type & VARTYPE_FUNC) && V->type & VARTYPE_INITIALIZED)
				PROCError(2086,nome);
			// 3. Se siamo dentro una funzione (AUTO/REGISTER), vieta QUALSIASI ridefinizione nello stesso blocco
			else if(Class >= CLASSE_AUTO && Class < CLASSE_MEMBER)		// var locali in diversi blocchi allo stesso livello ...
				PROCError(2086,nome);
			// Se arriviamo qui: sono due 'int x;' globali identiche (tentative definition valida)
			if(Class < CLASSE_AUTO)
				dontAllocate=TRUE;
			} 
		else {
			// Gestione EXTERN e Prototipi di funzioni
			if((V->type & ~VARTYPE_FUNC | VARTYPE_FUNC_USED) != (Type & ~VARTYPE_FUNC | VARTYPE_FUNC_USED))
				PROCError(2086,nome);
			if(V->classe != Class)
				PROCError(2371,nome);
			}
		}
  if(!InBlock && (Class>CLASSE_STATIC && Class<CLASSE_MEMBER) && !tag) {
		PROCWarn(2071,nome);
		Class=CLASSE_GLOBAL;        // AL LIVELLO PIU' ALTO NON CI POSSONO ESSERE Regs O AUTO
		}
  if(Class==CLASSE_REGISTER && FNGetMemSize(Type,Size,0/*dim*/,1) > INT_SIZE) {
		Class=CLASSE_AUTO;        // no reg + grandi di INT_SIZE
		}
  if(isParm && (Class<CLASSE_AUTO)) {
		PROCWarn(2071,nome);
		Class=CLASSE_AUTO;    // ...E COME PARAMETRI NIENTE STATICI O GLOBAL; anche se si "potrebbe" ... (v. microchip)
		}
  if(Class>CLASSE_STATIC && Class<CLASSE_MEMBER && (Type & VARTYPE_FUNC) && !tag) {
	  if(!(Type & VARTYPE_FUNC_POINTER)) 
			Class=CLASSE_GLOBAL;           //   SE E' UNA FUNZIONE globale ED E' AUTO O REGISTER DIVENTA GLOBAL
		}
  if(Class==CLASSE_REGISTER && Type & VARTYPE_VOLATILE) {
		PROCWarn(4042,"registro volatile");
		Class=CLASSE_AUTO;		// dice gemini :) 9/2026
		}

	if(Type & VARTYPE_FUNC && (Type & VARTYPE_CONST))			// solo C++?? ma...
//	  if(!(Type & VARTYPE_POINTER)) 
		PROCError(2071,nome);
	if(attrib & FUNC_ATTRIB_NAKED && Class == CLASSE_AUTO)
		PROCWarn(2215,nome);

	if(Type & VARTYPE_CONST) {
		char tempPath[256];
		if(Class>=CLASSE_AUTO)			// solo C++?? ma...
			PROCWarn(1002,"variabile const non statica");

		}
  if(!V) {
		V=PROCAllocVar(nome,Type,Class,Modif,Size,tag,dim);
		          //   SE LA VARIABILE NON ESISTEVA LA SI DICHIARA
		// NON QUA (se è un prototipo, DOPO la forzo a EXTERN
		}
	_tcscpy(V->label,T);
	V->attrib=attrib;
  if(Type & VARTYPE_FLOAT) {
		UseFloat=TRUE;
		}

  if(Type & VARTYPE_FUNC_POINTER) {			// patch... può esistere anche se var normale, pare (v. GetType
		while(*FNLA(MyBuf)==')') 
			FNLO(MyBuf);
	  }

	_tcscpy(varType,outbuf);
//	*outbuf=0;


  if(Type & VARTYPE_ARRAY) {			// SE E' UN ARRAY...

		if(Type & VARTYPE_IS_2POINTER)                  // se (almeno) doppio puntatore o arr. di ptr
		  /* MA NO!! perché? 2025  Size=PTR_SIZE*/;

    v=FNGetArraySize(V);             // dim totale oggetto
	  v=(v+4 /*STACK_ITEM_SIZE*/-1) & -4/*STACK_ITEM_SIZE*/;         //  dword

		while(*FNLA(MyBuf)=='[') {
		  while(*FNLO(MyBuf) != ']');
		  }
//		  myLog->print(0,"Array: %s, tipo: %x, size %x",nome,type,Size);
		isInitialized=*FNLA(MyBuf) == '=';
		if(isInitialized)
			V->type |= VARTYPE_INITIALIZED;

		switch(Class) {
		  case CLASSE_EXTERN:
//				PROCOut1(&StaticOut,Var[T1].label,NULL);
				break;

		  case CLASSE_GLOBAL:
		  case CLASSE_STATIC:                   // SE E' STATICO O GLOBAL
								// NOME DELL'ARRAY
				if(!isInitialized) {
					itoa(v,MyBuf,10);
					PROCOut1(Type & VARTYPE_CONST ? FO3 : FO2,V->label,"\tDB ",MyBuf," DUP (?)");     // ALLOCO v BYTES
					}
				else {
					PROCOut1(Type & VARTYPE_CONST ? FO3 : FO1,V->label,NULL);     // solo il nome
					}
				*S='\t';
				*(S+1)=0;
				PROCOper(LINE_TYPE_DATA_DEF,"",V->name,"[","]",LINE_IS_NORMAL);
				break;

		  case CLASSE_AUTO:
				if(isParm) {
//   SE E' UN PARAMETRO
				  AutoOff+=PTR_SIZE;            // OFFSET POSITIVI
				  }
				else {
				  OldTX[InBlock].AutoOff-=(v+STACK_ITEM_SIZE-1) & -STACK_ITEM_SIZE;         //  ALTRIMENTI NEGATIVI (e pari
				  }
//				sprintf(V->label,"%d",AutoOff);

				if(OutSource) {
//  				sprintf(MyBuf,"%s = %d",nome,MAKEPTROFS(V->label));
				  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_DATA_DEF,MyBuf);
				  }
				if(*FNLA(MyBuf) == '=') {
				  PROCError(1002,"init auto array");		// c'è anche altrove;  forse C99 ecc??
				  }
				PROCOper(LINE_TYPE_DATA_DEF,"",V->name,"[","]",LINE_IS_NORMAL);
				break;
		  default:
				break;
	  	}  

		}		// se array
  else {
		isInitialized= *FNLA(MyBuf) == '=';
		if(isInitialized)
			V->type |= VARTYPE_INITIALIZED;
    Size=FNGetMemSize(Type,Size,0/*dim*/,1);

		switch(Class) {
// ALLOCO VAR. NORMALE
		  case CLASSE_GLOBAL:
		  case CLASSE_STATIC:
			if(!(V->type & VARTYPE_FUNC))		// le funzioni vanno altrove
				PROCOper(LINE_TYPE_DATA,varType,V->name,NULL,V->label,LINE_IS_NORMAL);
			break;

	  case CLASSE_AUTO:
L5080:
			if(!isParm) {
				if(!(V->type & VARTYPE_FUNC))		// le funzioni vanno altrove
					PROCOper(LINE_TYPE_ISTRUZIONE,varType,V->name,NULL,V->label,LINE_IS_NORMAL);
				AutoOff -= Size;		// usarlo come flag per spaziare tra dichiarazioni e istruzioni!
		  	}			// non parm
			else {
	// GESTISCO IL PARAMETRO
			if(OutSource) {
  			itoa(AutoOff,MyBuf,10);
// 				wsprintf(MyBuf,"%s = %d",nome,MAKEPTROFS(V->label));
//			  PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,MyBuf);
			  }
			  }
			break;

	  case CLASSE_REGISTER:
			if(t1) {
// SE NON HO Regs DIVENTA AUTO
//		  	sprintf(V->label,"%d",t1);

		  	if(isParm) {
					if(!(V->func.func->modif & (FUNC_MODIF_FASTCALL | FUNC_MODIF_INLINE))) {
						OldTX[InBlock].AutoOff+=STACK_ITEM_SIZE; 
//					FNGetFPStr(MyBuf,AutoOff,NULL);
//						PROCOper(LINE_TYPE_ISTRUZIONE,movString,OPDEF_MODE_FRAMEPOINTER_INDIRETTO,(union SUB_OP_DEF*)0,OldTX[InBlock].AutoOff
	//						,OPDEF_MODE_REGISTRO32,MAKEPTRREG(V->label));
// PARAMETRO REGISTRO opp fastcall
						}
					else if(V->func.func->modif & FUNC_MODIF_INLINE) {
//		  			MAKEPTRREG(V->label)=Regs->MaxUser-1-t1;			// inizio a usare da D0/R0/AL
						}
					else {			// aggiusto #registro 
	//	  			MAKEPTRREG(V->label)=Regs->MaxUser-1-t1;			// inizio a usare da D0/R0/AL
						if(!(V->type & VARTYPE_FUNC))		// le funzioni vanno altrove
							PROCOper(LINE_TYPE_DATA,varType,V->name,NULL,V->label,LINE_IS_NORMAL);
						}
						}
		  	if(OutSource) {
//		  	  sprintf(MyBuf,"register %s = %s",(*Regs)[V],nome);
//					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"registro");
					}
				  }
				else {
					if(!(V->func.func->modif & (FUNC_MODIF_FASTCALL | FUNC_MODIF_INLINE)))
						;
					else
						PROCWarn(4042,"nessun registro disponibile");
				  V->classe=Class=CLASSE_AUTO;
				  goto L5080;
				  }
				break;
		  case CLASSE_MEMBER:
		  case CLASSE_MEMBER_VIRTUAL:
				if(!(V->type & VARTYPE_FUNC))		// le funzioni vanno altrove
					PROCOper(LINE_TYPE_DATA,V->name,NULL,NULL,V->label,LINE_IS_NORMAL);
				break;
		  case CLASSE_MEMBER_STATIC:
// no, solo se definita da fuori, v. di là				if(!(V->type & VARTYPE_FUNC))		// le funzioni vanno altrove
//					PROCOper(LINE_TYPE_DATA,V->name,NULL,NULL,V->label,LINE_IS_NORMAL);
				break;
		  default:
				break;
		  }

		_tcscat(outbuf," ");
		_tcscat(outbuf,V->name);
		}

  ol=__line__;
  FNLO(MyBuf);
  switch(*MyBuf) {
		case '(':		// è una funzione
			if(!(V->type & VARTYPE_FUNC_POINTER))
				*S=0;
		  T2=1;
		  t=FIn->GetPosition();
		  do {
				FNLO(T);
				switch(*T) {
				  case '(':
						T2++;
						break;
				  case ')':
						T2--;
						break;
				  case 0:
						PROCError(2059,T);
						break;
				  }
				} while(T2);
		  t2=FIn->GetPosition();
		  FNLA(T);
			if(*T == ',') {
			  FNLO(T);
				goto do_declare_ext;
				}

		  if(*T == ':') {		// inherit ctor ecc...
				BaseTextP=FIn->GetPosition();		// salvo per fare dopo
				FNLO(T);
				do {
					FNLO(T);
					} while(*T && *T != ';' && *T != '{');		// salto queste cose
				}

		  if(*T != ';') {		// ossia se segue '{'
				if(V->type & VARTYPE_FUNC_POINTER)
				  PROCError(2054,";");
				else if(V->type & VARTYPE_FUNC_BODY)
				  PROCError(2084,nome);
				else {
				  V->type |= VARTYPE_FUNC_BODY;
				  V->classe = Class;
/*				  if(Class==CLASSE_STATIC) {
//						sprintf(V->label,"$%s_0",FNGetLabel(MyBuf,0));      // mezza boiata...
						FNGetLabel(MyBuf,3);
						_tcscpy(V->label,MyBuf);
//						_tcscat(V->label,"_0");        // non capisco...
						}*/
				  }
				if(V->classe < CLASSE_GLOBAL)		// include CLASSE_MEMBER ecc
					V->classe = CLASSE_GLOBAL;
				if(V->modif & FUNC_MODIF_INTERRUPT) {
					if(V->size>0)
						PROCError(3001,nome);
					}
				if(!V->isInTag && InBlock>0)
				  PROCError(2601,nome);
				InBlock++;
//				if(InBlock)
					OldTX[InBlock].id = __line__;		// o rand()? v. altrove
				Declaring=TRUE;
				CurrFunc=V;
				FuncReturnedValue=FALSE;
				//gestire class


//				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,V->name,"(",NULL,NULL,LINE_IS_NORMAL);
// COMINCIO AD ALLOCARE LE VARIABILI locali parm
				PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"\n",NULL,NULL,NULL,LINE_IS_NORMAL);
				AutoOff=2*4;
				if(V->classe == CLASSE_GLOBAL) {
 					PROCOper(LINE_TYPE_DATA_DEF,"//PUBLIC\t");
					}
//				PROCOper(LINE_TYPE_DATA_DEF,V->label,"PROC");
				*OldTX[0].T=0;
				FIn->RestorePosition(t);
				__line__=ol;
				if(V->isInTag && V->classe != CLASSE_MEMBER_STATIC) {
					_tcscpy(outbuf,V->isInTag->label);
					_tcscat(outbuf," *this");
					}
				else
					*outbuf=0;
				FNLO(T);
				if(*T != ')') {

// CI SONO PARAMETRI
					if(V->isInTag) {
						if(strstr(V->name,dtor))		// eh beh!
							PROCError(2831);
//						_tcscat(V->name,to_mangle);		// mangling
//						_tcscat(V->label,to_mangle);
						_tcscat(outbuf,",");
						}
					else
						*V->parm.ptr32=0;		// no, per this
// PROTOTIPI

				  v=FNIsClass(T);
				  if((v>=0) || (FNIsType(T) != VARTYPE_NOTYPE)) {
						int totParm=V->isInTag ? 1 : 0;
						int *parmPtr=NULL;
						struct TAGS *tag2;

//						FIn->Seek(t,CFile::begin);
						if(*T==')' || *T==',' /*!iscsymf(*T)*/) {			// tipo parentesi subito chiusa, o virgola - csymf solo lettere! quindi mi incasina gli '*'
							PROCError(2055);
							return 0 /*break*/;
							}
//						subAcquisisciParm(V->parm.ptr32,V,V->isInTag != 0,TRUE);
					  do {
//							_tcscat(outbuf,T);
						  v=FNIsClass(T);
							if(!_tcscmp(FNLA(T),"const"))	{	// GESTIRE! usare v. anche di là
								FNLO(T);                 
								_tcscat(outbuf,T);
								}
						  if(v>=0) {
								v &= 0xf;
								Class=(enum VAR_CLASSES)v;
								t=FIn->GetPosition();
								FNLO(T);
								_tcscat(outbuf,T);
								}
							else {
								if(!(V->modif & (FUNC_MODIF_FASTCALL | FUNC_MODIF_INLINE)))
									Class=CLASSE_AUTO;
								else
									Class=CLASSE_REGISTER;
							  }

							Size=INT_SIZE;
							Type=VARTYPE_PLAIN_INT;
							tag2=NULL;		// non perdo quello in entrata!

							f=PROCGetType(outbuf,&Type,&Size,&tag2,dim,&attrib,t);

								if(debug)  {
									char *tmp=(char*)(tag ? tag->label : "");/*NON CI PIACE @#£$% si incasina la printf del log...*/
								myLog->print(0,"TIPO in FUNZ: t=%x, s=%x, tag=%s, dim=%d",Type,Size,tmp,dim);
								}

							if(!Type && !Size) {		// questo è void!
								goto void_parm;
								}

							if(c_mode)
								;
							newvar=PROCDclVar(outbuf,Class,0,Type,Size,tag,dim,attrib,TRUE,NULL,NULL);
	  					if(FNGetMemSize(Type,Size,NULL/*dim*/,0) != 0) {     // scavalco VOID, ma non void*
								if(!(V->modif & FUNC_MODIF_FASTCALL)) 		// e inline pure
									parmPtr=V->parm.ptr32;
								else
									parmPtr=V->parm.ptr32;
								i=totParm;
								if((V->isInTag && parmPtr[0]>1/*per this*/) || (!V->isInTag && parmPtr[0])) {		// se c'è già stato un prototipo per la funzione, ricontrollo i parm
									if(parmPtr[i*4+1] != Type || parmPtr[i*4+2] != Size)
									  PROCError(2082,nome,i);		// (mettere sia nome funzione che parm diverso...
									}
								if(parmPtr[i*4+1] & VARTYPE_INITIALIZED) {
									parmPtr[i*4+1]=parmPtr[i*4+1];//debug
									}
								else {
									parmPtr[i*4+1]=Type;
									parmPtr[i*4+2]=Size;
									parmPtr[i*4+3]=0;
									}

								totParm=i+1;
								if(totParm>20)
								  PROCError(1001,"func parm > 20");

								}
							if(V->isInTag) {
//								char ch[64];
	//							getDecor(ch,Type,Size,NULL/*dim*/,tag);
		//						_tcscat(V->name,ch);		// mangling
								}

void_parm:
skip_var:
							FNLO(T);
							if(*T == ',') {
								_tcscat(outbuf,T);
							  t=FIn->GetPosition();
								FNLO(T);
							  }
							else if(*T == '=') {
								int *parmPtr;
								parmPtr=V->parm.ptr32;
								t1=FNGetConst(MyBuf,1);
								if(parmPtr[i*4+1] & VARTYPE_INITIALIZED  && parmPtr[i*4+3] != t1)
									PROCError(2572); // se era già stato definito  
								parmPtr[i*4+3]=t1;		// defvalue, può essere costante o un membro statico o var globale o funzione/costruttore
								parmPtr[i*4+1] |= VARTYPE_INITIALIZED;
/*								int j=_tcslen(outbuf);
								while(outbuf[j] != ',' && j>0)
									outbuf[j--];*/
								_tcscat(outbuf,MyBuf);
							  goto skip_var;
								}
							else if(*T == ')')
								;
							else {
								if(iscsymf(*T)) {		// se c'è nome di variabile, lo salto
									_tcscat(outbuf," ");
									_tcscat(outbuf,T);
								  goto skip_var;
									}
								else 
									PROCError(2059);
								}
								{
								int *parmPtr;
								parmPtr=V->parm.ptr32;
//								i=parmPtr[0];
								if(i>0 && parmPtr[(i-1)*4+1] & VARTYPE_INITIALIZED && !(parmPtr[(i)*4+1] & VARTYPE_INITIALIZED))		// 
									PROCError(2548);
								}
							} while(*T != ')');
						if(parmPtr)
							parmPtr[0]=totParm;
						
					  }		// no class no type
//					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,") {\n",NULL, LINE_IS_NORMAL);
					PROCOper(LINE_TYPE_FUNCTION,varType,V->name,outbuf, NULL,LINE_IS_NORMAL);
				  }		// ci sono parametri
					else {		// lo faccio cmq perché se no mi esce l'errore perché sto facendo il mangling DOPO la dichiarazione
//						if(V->isInTag) {
	//						_tcscat(V->name,to_mangle);		// mangling
//							_tcscat(V->label,to_mangle);
//							*V->parm.ptr32=0;		// qua serve
							//}
						PROCOper(LINE_TYPE_FUNCTION,varType,V->name,outbuf, NULL,LINE_IS_NORMAL);
						}

					if(BaseTextP) {		// se c'era inherit ctor e/o init membri...
						BaseTextP=FIn->GetPosition();		
						struct VARS *v;
						struct TAGS *base;
						char outbuf1[128];
						char c[sizeof(union STR_LONG)];

						FNLO(T);

rifo_init_ctor:
						FNLO(T);
						if(!_tcscmp(T,tag->parent->label)) {		// cmq controllo anche da fuori; questo è un costruttore da ereditare
							PROCCheck('(');

							long l3=FIn->GetPosition();
							collectParmList(S);

							if(!FNCercaCtor(tag,S,TRUE,&base))
								PROCError(2512,T);		// 

							FIn->RestorePosition(l3);

							_tcscpy(MyBuf,tag->parent->label);
							_tcscat(MyBuf,ctor);
							_tcscat(MyBuf,to_mangle);
							_tcscat(MyBuf,S);
							wsprintf(T,"(struct %s*)&this->%s",tag->parent->label,ptr_to_base2);

							do {
								FNLO(outbuf1);
								if(*outbuf1 != ')' )
									_tcscat(T,outbuf1);
								} while(*outbuf1 && *outbuf1 != ')' && *outbuf1 != ';' && *outbuf1 != '{');		// in teoria cerco solo la (
							if(T[_tcslen(T)-1] == ',')
								T[_tcslen(T)-1] = 0;

							PROCOper(LINE_TYPE_CALL,MyBuf,T,NULL,"chiamo altro",LINE_IS_NORMAL);
							decor=NULL;		// marker di "trovato ctor"

							if(*T == ';') 		// 
								PROCWarn /*PROCError*/(2535,T);		// dice errore o warning
							if(*FNLA(T) == ',') {
								FNLO(T);
								goto rifo_init_ctor;
								}
							}
						else if(v=FNCercaVar(tag,T,&base)) {		// questo è un membro da inizializzare
							if(!base) {		// altrimenti vuol dire che c'è, ma nella base e quindi non accettabile

								if(v->classe == CLASSE_MEMBER_STATIC)
									PROCError(2438,T);
								// dare errore se 2 volte lo stesso  PROCError(2437,T);
								PROCCheck('(');
								*outbuf1=0;
								FNEvalExpr(outbuf1,15,c);
								_tcscpy(MyBuf,"this->");
								_tcscat(MyBuf,T);
								PROCOper(LINE_TYPE_ISTRUZIONE,MyBuf,"=",outbuf1);

								PROCCheck(')');
								if(*FNLA(T) == ',') {
									FNLO(T);
									goto rifo_init_ctor;
									}
								}
							else
								PROCError(2614,T,tag->label);
							}
						else
							PROCError(2614,T,tag->label);
						}

					if(decor && !_tcscmp(decor,ctor)) {		// se è costruttore
						if(tag->parent) {		// se mi serve chiamare la base...
							_tcscpy(MyBuf,tag->parent->label);
							_tcscat(MyBuf,ctor);
							_tcscat(MyBuf,to_mangle);
							_tcscat(MyBuf,S);
							wsprintf(T,"(struct %s*)&this->%s",tag->parent->label,ptr_to_base2);
							PROCOper(LINE_TYPE_CALL,MyBuf,T,NULL,"chiamo padre",LINE_IS_NORMAL);
							}

						if(FNHasVirtual(tag)) {		
							wsprintf(T,"((struct %s*)this)->%s = (const struct %s_VTable*)&__vftable_%s",
								tag/*->parent*/->label,vptr,tag/*->parent*/->label,tag->label);				// Assegna vtable 
							PROCOper(LINE_TYPE_ISTRUZIONE,T,NULL,NULL,"x xx virtual",LINE_IS_NORMAL);
							}

						}

						
					if(V->size>0)
						PROCOper(LINE_TYPE_ISTRUZIONE,varType,"__retval");

//				else
//     		  FIn->Seek(-1,SEEK_CUR);

				if(V->modif & FUNC_MODIF_INTERRUPT) {
					if(*V->parm.ptr32>0)
						PROCError(3002,nome);
					}

				InBlock=0;
				Declaring= TRUE;
    		PROCOper(LINE_TYPE_COMMENTO,"----------------------------------------");
	// serve quando la funzione è vuota
//				OldTX[0].TX=LastOut;
				if(!_tcscmp(nome,main_name)) {		// idem
					PROCOper(LINE_TYPE_CALL,"__global_constructors",NULL,NULL,NULL,LINE_IS_NORMAL);
					PROCOper(LINE_TYPE_FUNCTION_DECLARATION,"void","__global_constructors",NULL,NULL,LINE_IS_NORMAL);
					if(AutoOff>(2*4)) {		// idem
// main PUO' AVERE PARAMETRI DELLA COMMAND LINE
						V=FNCercaVar("_CLArgs",FALSE);
	  				if(!V)
		 					V=PROCAllocFunzProto("_CLArgs",VARTYPE_FUNC_USED | VARTYPE_PLAIN_INT,4);
      			PROCOper(LINE_TYPE_CALL,"_CLArgs");
						PROCOper(LINE_TYPE_ISTRUZIONE,"argc"); // prima argc
						PROCOper(LINE_TYPE_ISTRUZIONE,"argv"); // poi argv
						}

				  }
				if(V->modif & FUNC_MODIF_INTERRUPT) {
//mah 2025				if(V->classe & CLASSE_INTERRUPT) {         // interrupt
					UseIRQ=1;				
				  }
				AutoOff=0;
				}		// segue qualcos'altro invece di ;
			else {
				if(V->type & VARTYPE_FUNC_BODY && *V->parm.ptr32>=0) {		// in teoria controllare i parametri..
				  PROCError(2083,nome);		// errore qua!
//				  PROCWarn(2083,nome);		// oppure solo warning
					}
				FIn->RestorePosition(t);
				__line__=ol;
				if(V->isInTag && V->classe != CLASSE_MEMBER_STATIC) {
					_tcscpy(outbuf,V->isInTag->label);
					_tcscat(outbuf," *this");
					}
				else
					*outbuf=0;
				FNLO(T);
				if(*T != ')') {
					if((V->isInTag && *V->parm.ptr32>1/*per this*/) || (!V->isInTag && *V->parm.ptr32>0)) 
						PROCWarn(4028,V->name);		// c'è già fuori cmq, togliere o cambiare
					if(V->isInTag) {
						if(strstr(V->name,dtor))		// eh beh!
							PROCError(2831);
//						_tcscat(V->name,to_mangle);		// mangling
//						_tcscat(V->label,to_mangle);
						_tcscat(outbuf,",");
						}
	// in teoria sbagliato se compare di nuovo un prototipo... andrebbero controllati i parm (v. anche da fuori
					else
						*V->parm.ptr32=0;		// no, per this
				  if(*T=='.' || FNIsType(T) != VARTYPE_NOTYPE) {		// per ...
// PROTOTIPI e basta
					  do {
							int totParm=V->isInTag ? 1 : 0;		// qua non è usato, ev. fare come sopra
							struct TAGS *tag2;
							if(*T == '.') {            // gestisco ... variable parm
								int *parmPtr;
								parmPtr=V->parm.ptr32;
								i=parmPtr[0];
								parmPtr[i*4+1]=-1;
								parmPtr[i*4+2]=0;
								parmPtr[i*4+3]=0;// defvalue, può essere costante o un membro statico o var globale o funzione/costruttore
								parmPtr[0]=i+1;
//					    myLog->print("trovo ... e scrivo a %x",p+2+i*8);
								FIn->RestorePosition(t2);
								__line__=ol;
							  break;
							  }
							Size=INT_SIZE;
							Type=0;
							tag2=NULL;		// non perdo quello in entrata!
							f=PROCGetType(outbuf,&Type,&Size,&tag2,dim,&attrib,t);

							if(debug) {
								char *tmp=(char*)(tag ? tag->label : "");/*NON CI PIACE @#£$% si incasina la printf del log...*/
								myLog->print(0,"TIPO in prototyp: t=%x, s=%x, tag=%s, dim=%d",Type,Size,tmp,dim);
								}

//	  					if(Type & VARTYPE_POINTER && Size==0)
	//							Size=4;     // scavalco VOID, ma non void*
	  					if(FNGetMemSize(Type,Size,0/*dim*/,0) != 0) {     // scavalco VOID, ma non void*
								int *parmPtr;
								parmPtr=V->parm.ptr32;
								i=parmPtr[0];
								parmPtr[i*4+1]=Type;
								parmPtr[i*4+2]=Size;
								parmPtr[i*4+3]=0;// defvalue, può essere costante o un membro statico o var globale o funzione/costruttore
								parmPtr[0]=i+1;
								if(i>20)
								  PROCError(1001,"max func parm");
								}  
skip_var2:
							FNLO(T);
							if(*T == ',') {
								_tcscat(outbuf,T);
							  t=FIn->GetPosition();
								FNLO(T);
							  }
							else if(*T == '=') {
								int *parmPtr;
								parmPtr=V->parm.ptr32;
								t1=FNGetConst(MyBuf,1);
								if(parmPtr[i*4+1] & VARTYPE_INITIALIZED  && parmPtr[i*4+3] != t1)
									PROCError(2572); // se era già stato definito   TENDENZIALMENTE QUESTO NON DOVREBBE ACCADERE, un secondo prototipo è un errore
								parmPtr[i*4+3]=t1;		// defvalue, può essere costante o un membro statico o var globale o funzione/costruttore
								parmPtr[i*4+1] |= VARTYPE_INITIALIZED;
//								_tcscat(outbuf,MyBuf);
							  goto skip_var2;
								}
							else if(*T == ')')
								;
							else {
								if(iscsymf(*T)) {		// se c'è nome di variabile, lo salto
									_tcscat(outbuf," ");
									_tcscat(outbuf,T);
								  goto skip_var2;
									}
								else 
									PROCError(2059);
								}
								{
								int *parmPtr;
								parmPtr=V->parm.ptr32;
//								i=parmPtr[0];
								if(i>0 && parmPtr[(i-1)*4+1] & VARTYPE_INITIALIZED && !(parmPtr[(i)*4+1] & VARTYPE_INITIALIZED))		// 
									PROCError(2548);
								}
							} while(*T != ')');
						PROCOper(LINE_TYPE_FUNCTION_DECLARATION,varType,V->name,outbuf,"qua",LINE_IS_NORMAL);
					  }
					else {
						if(V->isInTag) {// HMM NON sembra più passare di qua... mai.. verificare e togliere
							struct VARS *v;
							char MyBuf2[64];
							struct TAGS *inBase;

							PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"COSTRUISCO",NULL,NULL,NULL,LINE_IS_NORMAL);
							FIn->RestorePosition(t);
							__line__=ol;
							_tcscpy(MyBuf,V->isInTag->label);
							_tcscat(MyBuf,ctor);
							_tcscat(MyBuf,to_mangle);
							collectParmList(MyBuf);
							v=FNCercaVar(MyBuf,FALSE,&inBase);
							FIn->RestorePosition(t);
							__line__=ol;
							if(!v)
								PROCError(2512,"ctor");
							else
								PROCUsaFun(outbuf,v,0x80);
//							PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,NULL,NULL,NULL,LINE_IS_NORMAL);
							PROCOper(LINE_TYPE_CALL,v,outbuf,NULL,LINE_IS_NORMAL);
							}
						}
  				}
			  }	
		  break;

		case '=':          // INIZIALIZZAZIONI
			if(isParm) // qua!! lascio passare tutto per i parametri di default, sia in dichiarazione che definizione cmq
				FIn->unget('=');
			else
		  switch(Class) {
				case CLASSE_EXTERN:
					PROCWarn(2205);		// non è chiaro, forse in C++ si può (solo se fuori blocco) ma in C boh
					break;
				case CLASSE_GLOBAL:
				case CLASSE_STATIC:
					{
					O_SIZE s2=FNGetMemSize(Type,Size,0/*dim*/,2);
					if(Type & VARTYPE_ARRAY) {
						char MyBuf1[128];
	//				myLog->print("Inizz array: %s, tipo %x, size %d",nome,type,Size);
						i=0;
	//					p=strstr(StaticOut->s,"\td");
	//					*p=0;
						PROCCheck('{');
						do {									// bisognerebbe gestire le dim array in init..
							if(/* *FNLA(MyBuf1) != ','*/  !*FNLA(MyBuf1) || *FNLA(MyBuf1) == '}')			// OCCHIO se c'è la virgola DOPO ultimo elemento lo prende come 0...
																																											// pare ok ora
								break;
							FNGetConst(MyBuf,1);
									// servirebbe messaggio se costante troppo grande per tipo var
									PROCOut1(Type & VARTYPE_CONST ? FO3 : FO1,"\tDB\t",MyBuf);
							i++;
							} while(*FNLO(MyBuf) == ',');
						if(dim[ndim]>0) {
							if(i > dim[ndim])
								PROCError(2078);
							}
						else
							dim[ndim]=i;
						ndim++;
						if(ndim>MAX_DIM-1)
							PROCError(1002,"dimensioni max=4");
						memcpy(V->dim,dim,sizeof(O_DIM));
  					*S=' ';	
						if(i && s2==1)
							PROCOut1(Type & VARTYPE_CONST ? FO3 : FO1,"\tALIGN 4",NULL);		// 
						}
					else {
						//p=strstr(S,"\tD");

						//*(p+4)=0;
						/*struct CONS myC=*/FNGetConst(MyBuf,1);
						_tcscat(S,MyBuf);
						}
					}
					break;
				case CLASSE_AUTO:
				case CLASSE_REGISTER:
	//				if(Type & VARTYPE_ARRAY) 
	// c'è già sopra					PROCError(1002,"inizializzazione array auto");		// forse C99 ecc??
					FIn->RestorePosition(OldTextp);
					__line__=ol;
	//				*MyBuf=0;
					*outbuf=0;
					FNEvalExpr(outbuf,14,MyBuf);	// NO = qua, cmq finire tutto
					PROCOper(LINE_TYPE_ISTRUZIONE,outbuf);
					break;

				case CLASSE_MEMBER:
				case CLASSE_MEMBER_VIRTUAL:
					break;
				case CLASSE_MEMBER_STATIC:
/*					_tcscat(outbuf,"=");		// in effetti non passo di qua! siccome è già stata definita PER FORZA, aggiungo solo
					do {
						FNLO(MyBuf);
						if(*MyBuf != ';')
							_tcscat(outbuf,MyBuf);
						} while(*MyBuf && *MyBuf != ';');
					FIn->unget(';');*/
					break;
				}
		  break;
		default:
do_declare_ext:
			FIn->unget(*MyBuf);		//FIn->Seek(-1,CFile::current);     
			__line__=ol;
		  break;
		}

	if(!dontAllocate) {
		if(*S) {
			if(Type & VARTYPE_CONST) {
//				PROCOut1(FO3,S," //",V->label,NULL);
//				PROCOper(LINE_TYPE_LABEL,outbuf,NULL,NULL,V->label,LINE_IS_NORMAL);
				}
			else {
				if(!isInitialized)
//					PROCOut1(FO2,S," //",V->label,NULL);
//					PROCOper(LINE_TYPE_DATA,outbuf,NULL,NULL,V->label,LINE_IS_NORMAL);
;
				else
//					PROCOut1(FO1,S," //",V->label,NULL);
//					PROCOper(LINE_TYPE_DATA_DEF,outbuf,NULL,NULL,V->label,LINE_IS_NORMAL);
		//				PROCOut1((*S==' ' /*|| *S=='\t' pare ok così 2025*/ ) ? FO2 : FO1,S," ;",nome,NULL);
		;
				}

			}
		}
	
  return V;
  }
	 
int CPlusMinus::subAsm(char *s) {
  char MyBuf[64],buf[3][64],ch;
  struct VARS *V;
  int i,ol;
  long ot;
	int8_t state=0;
	bool go=FALSE;
  
  *MyBuf=0;
	*buf[0]=*buf[1]=*buf[2]=0;

  FNLO(s);
	ot=FIn->GetPosition();
	ol=__line__;
  do {
//    l=__line__;
		  myLog->print(0,"SUBASM: %s,  %d, ",s,state);
		switch(state) {
			case 0:		// istruzione
				_tcscpy(buf[0],s);

				// ev altre varianti...

				state++;
				break;
			case 1:		// 1st op
			case 2:		// 2nd op
			case 3:		// 3nd op		(Archimedes, GD24032
#if 0
			  if(*s==',') {
					}
				else {
					if(FNIsOp(s,0)) {
//  					_tcscat(buf[state],s);
						// virgola, implicita; oppure parentesi, ++ ecc
						state--;
						}
					else if(i=Regs->FNIsReg(s)) {		// 
					  u[state-1].mode=OPDEF_MODE_REGISTRO32;
					  u[state-1].s.n=i-1;
						}
					else if(V=FNCercaVar(s,FALSE)) {
						switch(V->classe) {
							case CLASSE_EXTERN:
							case CLASSE_GLOBAL:
							case CLASSE_STATIC:
	//						  u[state-1].mode=OPDEF_MODE_VARIABILE_INDIRETTO;
//								_tcscpy(u[state-1].s.label,V->label);
								break;
							case CLASSE_AUTO:
		//						i=MAKEPTROFS(V->label);
			//	        sprintf(MyBuf,"%s%+d",Regs->FpS,i);
				//			  u[state-1].mode=OPDEF_MODE_FRAMEPOINTER_INDIRETTO;
					//			u[state-1].ofs=MAKEPTROFS(V->label);
								break;
							case CLASSE_REGISTER:
						//	  u[state-1].mode=OPDEF_MODE_REGISTRO32;
							//	u[state-1].s.n=MAKEPTRREG(V->label);
								break;
							}
						}
					else {

			      if(isdigit(*s))
							u[state-1].mode=0  ;  //OPDEF_MODE_IMMEDIATO32;

					  u[state-1].s.n=atoi(s);
						}
					state++;
					}
#endif
				break;
			default:
				go=TRUE;
				break;
			}
	  if(*s=='}') {
			FIn->RestorePosition(ot);
			__line__=ol;
			return 0;
	    }
		else {
			ot=FIn->GetPosition();
			ch=FIn->get();
			if(ch=='\n') {
	//			FIn->RestorePosition(ot);
				go=TRUE;
				}
			else {
				FIn->unget(ch);
				FNLO(s); 
				}
			}

		} while(!go);

//  PROCOper(LINE_TYPE_ISTRUZIONE,MyBuf,OPDEF_MODE_NULLA,0);		// ATTENZIONE lim max= sizeof(opcode), SPEZZARE operandi

  if(buf[2][0])
	  PROCOper(LINE_TYPE_ISTRUZIONE,buf[0], NULL, NULL, NULL,LINE_IS_ASSEMBLER);
	else
	  PROCOper(LINE_TYPE_ISTRUZIONE,buf[0], NULL, NULL, NULL, LINE_IS_ASSEMBLER);
	
  
  return 1;
  }

int CPlusMinus::FNIsStmt() {
  int OldDcl,T1,I,i,c;
  char TS[128],T1S[64],C[sizeof(union STR_LONG)],MyBuf[128],MyBuf1[64];
	uint32_t attrib=0;
  char *p;
  long OldTextp,l,l1;
	int ol;
	char outbuf[256];
  
  OldDcl=Declaring;
  Declaring=FALSE;
rifoStmt:  
  OldTextp=FIn->GetPosition();
	ol=__line__;
  FNLO(TS);
  switch(*(WORD*)TS) {
		case '#':
			FNLO(MyBuf);
			FNLO(MyBuf);
			__line__=atoi(MyBuf);
			FNLO(MyBuf);
			_tcsncpy(__file__,MyBuf+1,_tcslen(MyBuf)-2);
			__file__[_tcslen(MyBuf)-2]=0;
			FIn->get();		// mangio il CR per non alterare contatore line!
			Declaring=OldDcl;
			return TRUE;
			break;
		case 'a_':
			if(!_tcscmp(TS,"_asm")) {                 // CODICE Assembly
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"asm ");
				if(*FNLA(MyBuf) == '{') {
					FNLO(MyBuf);
				  FNLA(MyBuf);
					if(*MyBuf!='}') {		// safety se vuoto!
						for(;;) {
							subAsm(MyBuf);
							FNLA(MyBuf);
							if(!*MyBuf || *MyBuf == '}') {
								FNLO(MyBuf);
								break;
								}
							}
						}
					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\");
					}
				else {
					subAsm(MyBuf);
					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\");
					}
				}
			else
				goto noStmt;
		  break;
		case 'rb':
			if(!_tcscmp(TS,"break")) {
				PROCOper(LINE_TYPE_ISTRUZIONE,"\tbreak");
				T1=InBlock;
				while(T1) {
					if(*OldTX[T1].B) {
//						PROCOper(LINE_TYPE_JUMP,"jr",OldTX[T1].B);
						T1=1;
						}
					T1--;
					if(T1==1)
						PROCError(2043);
					}
				}
			else 
				goto noStmt;
	    break;
		case 'ac':
			if(!_tcscmp(TS,"case")) {
				_tcscpy(T1S,OldTX[InBlock].T);
				if(*T1S != '&') {
					PROCError(2046);
					}
				else {
					p=OldTX[InBlock].parm;
					i=*((int *)p);
					
					c=FNGetConst(C,0);
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"case",C,":");
					if((OldTX[InBlock].C[1]==1 && abs(c) & 0xffffff00) || (OldTX[InBlock].C[1]==2 && abs(c) & 0xffff0000))
						PROCWarn(2053);
					if(OldTX[InBlock].C[1]==1)
						c &= 0xff;
					else if(OldTX[InBlock].C[1]==2)
						c &= 0xffff;
					*((int *)(p+sizeof(int)+i*sizeof(int)))=c;
					*((int *)p)=i+1;
					while(i--) {
						p+=sizeof(int);
						if(*(int *)p == c)
							PROCError(2049,C);
						}
					PROCCheck(':');
					sprintf(MyBuf,"%s_%x",T1S+1,c);
//					PROCOutLab(MyBuf);
					}
				}
			else
				goto noStmt;
			break;
		case 'oc':
			if(!_tcscmp(TS,"continue")) {
				PROCOper(LINE_TYPE_ISTRUZIONE,"continue");
				T1=InBlock;
				while(T1) {
					if(*OldTX[T1].C) {
						PROCOper(LINE_TYPE_JUMP,"jmp",OldTX[T1].C);
						T1=1;
						}
					T1--;
					if(T1==1)
						PROCError(2044);
					}
				}
			else
				goto noStmt;
			break;
		case 'ed':
			if(!_tcscmp(TS,"default")) {
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"\tdefault:");
				_tcscpy(T1S,OldTX[InBlock].T);
				if(*T1S != '&') {
					PROCError(2047);
					}
				else {
					char T2S[128];

					if(OldTX[InBlock].flag)
						PROCError(2048);

					OldTX[InBlock].flag=1;
					_tcscpy(MyBuf,T1S+1);
					_tcscpy(T2S,MyBuf);
					_tcscat(T2S,"_");
	//				_tcscat(OldTX[InBlock].TX->next->s,"_");



	//				_tcscat(OldTX[InBlock].T,"_");



	//      $((OLDTX%(InBlock%)!0)+8)+="_";
					PROCCheck(':');
//					PROCOutLab(T2S);
	//			  swap(&LastOut,&OLDTX[InBlock]);
	//			  swap(&LastOut,&OLDTX[InBlock]);
	/*		  if(*FNLA(MyBuf)) {
				if(FNIsStmt()) {
					}
				else 
					PROCIsDecl();
				}
				*/
					}
				}
			else
				goto noStmt;
		  break;
		case 'od':
			if(!*(TS+2)) {
				if(*FNLA(MyBuf) == '{')
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"do {");
				else
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"do");
				*OldTX[InBlock+1].T='#';
				_tcscpy(OldTX[InBlock+1].T+1,TS);
				_tcscat(TS,"_");
				_tcscpy(MyBuf,TS);
				_tcscat(MyBuf,"1");
				PROCLoops(NULL,TS,MyBuf);
				}
			else
				goto noStmt;
			break;
		case 'le':
			if(!_tcscmp(TS,"else")) {
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"else");
				I=InBlock+1;
	//    myLog->print("leggo da %d",I);
				if(*OldTX[I].T != '%')
					PROCError(2062,"else");
				_tcscpy(TS,OldTX[I].T+1);
				*TS='E'; TS[6]=0;	// e tolgo _T/_F
	//		_tcscat(TS,"_else");
				i=*FNLA(MyBuf) == '{';
				if(i)
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT," {");
//				PROCOper(LINE_TYPE_JUMP,"jr",TS);
				*MyBuf=' ';
				_tcscpy(MyBuf+1,TS);
				PROCLoops(MyBuf,"","");
				if(i)
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"\t}",NULL,NULL,MyBuf);
				}
			else
				goto noStmt;
		  break;
		case 'ne':  
			if(!_tcscmp(TS,"enum")) {
				uint32_t enum_cnt=0;

				PROCWarn(2000,"enum tag non gestiti ");
				PROCOper(LINE_TYPE_DICHIARAZIONE,"enum {");

				FNLA(TS);
				if(*TS != '{') {		// qua c'è il TAG dell'enum.. per ora me ne frego
					FNLO(TS);
					FNCercaEnum(TS,NULL,TRUE);
	//				i=FNAllocEnum(MyBuf);

					}
				PROCCheck('{');
				for(;;) {
					int j;
					int8_t OP=0,Co=0;
					char AS[64];
					struct OPERAND V;
					struct VARS v;
					union STR_LONG VCost;
					ZeroMemory(&V,sizeof(struct OPERAND));
					ZeroMemory(&VCost,sizeof(union STR_LONG));
					ZeroMemory(&v,sizeof(struct VARS));
					V.cost=&VCost;
					V.var=&v;
					FNLO(AS);
					FNLA(MyBuf);		// buttare fuori in ASM queste linee?
					*outbuf=0;
					if(*MyBuf == '=') {
						PROCCheck('=');
						j=FNGetAritElem(outbuf,&OP,MyBuf,&V,Co);
						if(j != ARITM_IS_COSTANTE)
							PROCError(2141);		// migliorare messaggio
						enum_cnt=V.cost->l;
						}
					else
						enum_cnt;
					if(FNCercaEnum(TS,AS,TRUE))
						PROCError(2011,AS);		// 
					PROCOper(LINE_TYPE_DICHIARAZIONE,TS,AS);
					FNAllocEnum(TS,AS,enum_cnt,FNGetSize((uint32_t)V.cost->l));
					if(OutSource) {
						wsprintf(MyBuf,"|%5u| : %s=%u",__line__,AS,enum_cnt);
	//					FNGetLine(OldTextp,MyBuf+10);		// complicato... lascio solo nomi
	//					MyBuf[_tcslen(MyBuf)-2]=0;		// tolgo CR se no diventa doppio
						PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_DICHIARAZIONE,MyBuf);
						}
					FNLA(MyBuf);
					if(*MyBuf == ',')
						PROCCheck(',');
					else if(*MyBuf == '}') {
						PROCOper(LINE_TYPE_DICHIARAZIONE,"\t}");
						PROCCheck('}');
						break;
						}
					else
						PROCError(2054,"}");
					enum_cnt++;
					}
				PROCCheck(';');
				Declaring=TRUE;
				}	
			else
				goto noStmt;
			break;
		case 'of':
			if(!_tcscmp(TS,"for")) {
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"for(");
				PROCCheck('(');
	//		*MyBuf=0;
				*outbuf=0;
				FNEvalExpr(outbuf,16,MyBuf);              // 1° expr
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,";");
				PROCCheck(';');     //    FNLO(TS);
				l=FIn->GetPosition();
				I=1;
				do {
					FNLO(TS);
					switch(*TS) {
						case '(':
							I++;
							break;
						case ')':
							I--;
							break;
						case 0:
							PROCError(2059);
							break;
						}
					} while(I);
		//    t2=LastOut;
				l1=FIn->GetPosition();
				FIn->RestorePosition(l);
				__line__=ol;
				if(*FNLA(MyBuf) != ';') {
		//      _tcscpy(MyBuf,"!");
					_tcscpy(MyBuf,T1S);
					FNEvalCond(outbuf,MyBuf,T1S,TRUE);
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,";");
					}
				PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"endfor");
				PROCCheck(';');
	//		*MyBuf=0;
				*outbuf=0;
				FNEvalExpr(outbuf,16,MyBuf);                 // 2° expr
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,")");
				_tcscpy(MyBuf,T1S);
				_tcscat(MyBuf,"_");
				FIn->RestorePosition(l1);
				__line__=ol;
				_tcscpy(OldTX[InBlock+1].T,T1S);
	//			_tcscat(T1S,"_");  
				i=*FNLA(MyBuf) == '{';
				if(i)
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT," {");
				_tcscpy(MyBuf,T1S);
				_tcscat(MyBuf,"_");  
				PROCLoops(NULL,T1S,MyBuf);
				if(i)
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"\t}",NULL,NULL,MyBuf);
				}
			else
				goto noStmt;
			break;
		case 'og':
			if(!_tcscmp(TS,"goto")) {
				if(*FNLA(MyBuf) != ';') {
	  			FNLO(MyBuf);
	  			PROCOper(LINE_TYPE_JUMPGOTO,"goto",MyBuf);
		  		}
				else
					PROCError(2059,TS);
				}  
			else
				goto noStmt;
		  break;
		case 'fi':
			if(!*(TS+2)) {
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"if(");
				PROCCheck('(');
				FNGetLabel(TS,2,-1);
			//    *MyBuf='!';
				_tcscpy(MyBuf,TS);
				*outbuf=0;
				FNEvalCond(outbuf,MyBuf,TS,TRUE);
				PROCCheck(')');
				i=*FNLA(MyBuf) == '{';
				if(i)
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,") {\n");
				else
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,")");
				*MyBuf='%';
				_tcscpy(MyBuf+1,TS);
				PROCLoops(MyBuf,"","");
				if(i)
					PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"\t}",NULL,NULL,MyBuf);
				}
			else
				goto noStmt;
			break;
		case 'rp':		
			if(!_tcscmp(TS,"pragma")) {
				if(!*FNLO(MyBuf))
					PROCError(2059,TS);
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"pragma",MyBuf);
				if(*FNLA(MyBuf1) == '(') {
					PROCCheck('(');
					if(*FNLA(MyBuf1) != ')')
						FNLO(MyBuf1);			// ev. finire...
					PROCCheck(')');
					}
				else
					FNLO(MyBuf1);			// ev. finire...
				PROCWarn(2000,MyBuf);
				if(!_tcscmp(MyBuf,"code_seg")) {
	//	      if(InBlock>0)                       // sembra di no... ma mi fa strano! (2010)
	//	        PROCError(2156);
					FNLO(MyBuf);
					PROCOper(LINE_TYPE_ISTRUZIONE,MyBuf);
					}
				Declaring=OldDcl;
				}
			else
				goto noStmt;
			break;
//		case 'il':			// #line! ma arriva sopra col cancelletto, per forza
//			FNLO(MyBuf);
//			FNLO(MyBuf);
//			break;
		case 'er':		
			if(!_tcscmp(TS,"return")) {
				if(*FNLA(MyBuf) != ';') {
					if(!FNGetMemSize(CurrFunc,1)) 
	  				PROCError(2562);
					// sarebbe da dare anche il Warning se non-void senza return...
					l=CurrFunc->type & ~(VARTYPE_FUNC | VARTYPE_FUNC_BODY | VARTYPE_FUNC_USED);
					i=CurrFunc->size;
					FNEvalECast(outbuf,MyBuf,(O_TYPE*)&l,(O_SIZE*)&i);
					FuncReturnedValue=TRUE;
//					PROCOper(LINE_TYPE_ISTRUZIONE,"\n\treturn",outbuf,NULL,NULL,LINE_IS_NORMAL);
					PROCOper(LINE_TYPE_ISTRUZIONE,"__retval=",outbuf,NULL,NULL,LINE_IS_NORMAL);
					PROCOper(LINE_TYPE_ISTRUZIONE,"goto __return_end",NULL,NULL,NULL,LINE_IS_NORMAL);
					}
				else {
					if(FNGetMemSize(CurrFunc,1)) 
	  				PROCError(2561);
					// anche 4715 ev.
//					PROCOper(LINE_TYPE_ISTRUZIONE,"\n\treturn",NULL,NULL,NULL,LINE_IS_NORMAL);
					PROCOper(LINE_TYPE_ISTRUZIONE,"goto __return_end",NULL,NULL,NULL,LINE_IS_NORMAL);
					}
				PROCReturn();
				}
			else
				goto noStmt;
			break;
		case 'ws':
			if(!_tcscmp(TS,"switch")) {
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"switch(");
				PROCCheck('(');
		//		*MyBuf=0;
				l=0;
				i=0;
				FNEvalECast(outbuf,MyBuf,(O_TYPE*)&l,(O_SIZE*)&i);
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,outbuf,") {\n");
				if(
					i>2 ||
					(l & (VARTYPE_STRUCT | VARTYPE_UNION | VARTYPE_CLASS | VARTYPE_ARRAY | VARTYPE_IS_POINTER | VARTYPE_FUNC | VARTYPE_FLOAT /*0x1d0f*/)))
					PROCError(2050);
				PROCCheck(')');
				FNGetLabel(TS,1);
				I=InBlock+1;
				*OldTX[I].T='&';
				_tcscpy(OldTX[I].T+1,TS);
				OldTX[I].flag=0;
				OldTX[I].C[0]=0;            // salvo qui (tanto non si usa) SIZE
				OldTX[I].C[1]=i;
				p=OldTX[I].parm=(char*)GlobalAlloc(GPTR,1024);
				*((int*)p)=0;
				PROCLoops(NULL,TS,"");
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"}\n",NULL,NULL,"endswitch",LINE_IS_NORMAL);
				}
			else
				goto noStmt;
			break;
		case 'yt':	
			if(!_tcscmp(TS,"typedef")) {
				l=FIn->GetPosition();
				FNLO(TS);
				PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"typedef",TS);
				T1=MaxTypes;
				*outbuf=0;
				i=PROCGetType(outbuf,&Types[T1].type,&Types[T1].size,&Types[T1].tag,Types[T1].dim,&attrib,l);
				FNLO(MyBuf);
				if(FNIsType(MyBuf) != VARTYPE_NOTYPE)
					PROCError(2026,MyBuf);
				MaxTypes++;
				if(MaxTypes>=MAX_TIPI)
					PROCError(1002,"massimo numero di typedef");
				_tcscpy(Types[T1].s,MyBuf);
				while(*FNLO(MyBuf) != ';');
				Declaring=OldDcl;
				PROCOper(LINE_TYPE_NULLA,NULL);
				}
			else
				goto noStmt;
			break;
		case 'hw':	
			if(!_tcscmp(TS,"while")) {
				PROCCheck('(');
				*outbuf=0;
				if(*OldTX[InBlock+1].T=='#') {         // se chiude do...
					I=InBlock+1;
		//		  *MyBuf='!';
					_tcscpy(MyBuf,OldTX[I].B);
					i=FNEvalCond(outbuf,MyBuf,OldTX[I].B,TRUE);
					PROCCheck(')');
	//			  if(i)
//						PROCOper(LINE_TYPE_JUMP,"jmp",OldTX[I].T+1);
					PROCOper(LINE_TYPE_JUMP,"} while(",outbuf,")");
					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"enddo");
					*OldTX[I].T=0;
					*OldTX[I].B=0;
					*OldTX[I].C=0;
					PROCCheck(';');
					}
				else {                                  // ...altrimenti
					FNGetLabel(TS,1);
			//      *MyBuf='!';
					_tcscpy(MyBuf,TS);
					FNEvalCond(outbuf,MyBuf,TS,TRUE);
					if(*FNLA(MyBuf) == '{')
						PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"while(",outbuf,") {");
					else
						PROCOper(LINE_TYPE_ISTRUZIONE_CONT,"while(",outbuf,")");
					PROCCheck(')');
					PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"beginwhile");
					_tcscpy(MyBuf,TS);
					_tcscat(MyBuf,"_");
//					PROCOper(LINE_TYPE_JUMP,"while(",MyBuf);
					*MyBuf=' ';
					_tcscpy(MyBuf+1,TS);
					_tcscpy(MyBuf1,TS);
					_tcscat(MyBuf1,"_");
					PROCLoops(MyBuf,TS,MyBuf1);
					}
				}
			else
				goto noStmt;
			break;
		default:
noStmt:		
			FNLA(MyBuf);
			if(*MyBuf==':' && MyBuf[1] != ':') {		// per ::
				if(!FNCercaGoto(TS)) {
					PROCAllocGoto(TS);
					PROCOutLab(TS,CurrFunc->label);
					}
				else
					PROCError(2045,TS);
				FNLO(TS);
				goto rifoStmt;       // la label da sola non fa stmt...
				}
/*			else if(*MyBuf=='{') {
				*OldTX[InBlock+1].T=0;
				PROCLoops(NULL,TS,MyBuf,LastOut);
				}*/
			else if(*MyBuf==':' && MyBuf[1] == ':') {		// per ::
				struct TAGS *tag;
				struct VARS *v,*newVar;
				O_DIM dim;
				char mangle[64];
				struct TAGS *inBase;

				if(tag=FNCercaAggr(TS,FALSE)) {		// qua solo ctor e dtor, v in Decl per altri casi
					*outbuf=0;
					FNLO(MyBuf);
					l=FIn->GetPosition();
					FNLA(MyBuf);
					*T1S=0;
					if(*MyBuf == '~') {
						FNLO(T1S);
						l=FIn->GetPosition();
						FNLA(MyBuf);
						}
					if(_tcscmp(MyBuf,TS)) {
						if(!InBlock)		// può solo essere ctor o dtor, oppure var. 
							PROCError(2143,MyBuf);		// senza tipo, errore in C++ (anche se VC98 lo prendeva come int, v. anche altrove
						else
							goto no_class_decl;
						}
					 {
						if(*T1S != '~')
							_tcscpy(MyBuf,ctor);
						else 
							_tcscpy(MyBuf,dtor);
						FNLO(MyBuf1);
						if(*FNLA(MyBuf1) == '(') {
							PROCCheck('(');
							_tcscat(MyBuf,to_mangle);
							collectTypeList(mangle);
							_tcscat(MyBuf,mangle);
							_tcscat(TS,MyBuf);
							v=FNCercaVar(TS,FALSE,NULL);
							FIn->RestorePosition(l);
							__line__=ol;
							if(v && v->type & VARTYPE_FUNC_USED)
								PROCError(2086,"ctor");
							else {
	//							PROCCheck('(');
								if(*T1S != '~')
									_tcscpy(MyBuf,ctor);
								else 
									_tcscpy(MyBuf,dtor);
								newVar=PROCDclVar(outbuf,CLASSE_MEMBER,0,VARTYPE_FUNC/*TYPE_NULL*/,SIZE_NULL,tag,dim,0,FALSE,
									MyBuf,mangle);
								if(!v)
									PROCOper(LINE_TYPE_FUNCTION_DECLARATION,NULL,newVar->name,outbuf,NULL,LINE_IS_NORMAL);
		//						*decor=0;
								if(*FNLA(MyBuf1) == ':') {		// non dovrebbe mai accadere
									while(*FNLO(MyBuf1) != ';' && *MyBuf1 != '{')
										;
									FIn->unget(*MyBuf1);

									}
								if(*FNLA(MyBuf) == '{') {
									PROCCheck('{');
									Declaring=TRUE;

									PROCBlock();
									if(*FNLA(TS)=='}') {
										PROCCheck('}');
										wsprintf(MyBuf,"\t};\n");
										PROCOper(LINE_TYPE_DATA_DEF,MyBuf);
										}
									}
								else {
									if(v)
										PROCWarn(4028,newVar->name);
									PROCCheck(';');
									// era un metodo! basta così?
						//			goto no_class_decl;
									}
								}
							}
						else {
							O_TYPE type=0;
							O_SIZE size=4;
							FIn->RestorePosition(l);
							__line__=ol;
							FNLO(MyBuf);
							v=FNCercaVar(MyBuf,FALSE,NULL);
							if(!v)
								PROCError(2039,MyBuf);
							FIn->RestorePosition(l);
							__line__=ol;
								newVar=PROCDclVar(outbuf,CLASSE_MEMBER_STATIC,0,type,size,tag,dim,0,FALSE,
									NULL,NULL);
//								PROCOper(LINE_TYPE_DATA,newVar->name,outbuf,NULL,NULL,LINE_IS_NORMAL);
								// e mettere extern all'altra, o togliere
								PROCCheck(';');
							}
						}

					}
				}
			else {
no_class_decl:
				FIn->RestorePosition(OldTextp);
				__line__=ol;
				Declaring=OldDcl;
				return FALSE;
				}
			break;  
		}

	if(!CurrFunc) {
//		PROCError(2062,TS);		// oppure provare a dichiarare... ma NON eseguire! anche se funzione
		}

  if(OutSource) {    
		wsprintf(MyBuf,"|%5d| : ",__line__);
		FNGetLine(OldTextp,MyBuf+10);
//		MyBuf[_tcslen(MyBuf)-2]=0;		// tolgo CR se no diventa doppio
		PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,MyBuf);
		}

	if(c_mode==-1) {
		c_mode=0;
		}
  return TRUE;
  }
 
char *CPlusMinus::FNGetLabel(char *A,uint8_t m,int8_t m2) {

  switch(m) {
    case 0:
      m='L';
      break;
    case 1:
      m='J';
      break;  
    case 2:
      m='L';
      break;  
    case 3:
      m='$';
      break;  
    default:
      m='X';
      break;  
    }
  wsprintf(A,"%c%05d",m,++LABEL);
  switch(m2) {
    case -1:
			_tcscat(A,"_F");
			break;
    case 1:
			_tcscat(A,"_T");
			break;
    case 0:
			break;
		}
  return A;
  }

 
int CPlusMinus::PROCGenCondBranch(const char *Alabel, int T, int8_t *VQ, O_SIZE Size) {
  int i,B;
 
//  if(V & 0xf)
//    S=PTR_SIZE;
  if(*VQ & VALUE_IS_CONDITION) {		// qua se condizione con operatore ossia > < == ecc...
		B=FNGetCondString(*VQ & 0x2f,T);
		}
  else {				//...altrimenti è semplice 0 o !0 su variabile
    i=*VQ & 0xf;
		/* QUA????   usare.f ?  bisognerebbe "segnarselo" da istruzione precedente, oppure fare in ottimizzazione
		*/
		if(i==VALUE_IS_EXPR_FUNC || (*VQ & VALUE_IS_CONDITION_VALUE)) {
					PROCOper(LINE_TYPE_ISTRUZIONE,"MOV.d.f");
			}
    if(*VQ & VALUE_IS_CONDITION_VALUE)
      T=!T;          
		if(T)
		  B=CONDIZ_UGUALE & 0xf;
		else
		  B=CONDIZ_DIVERSO & 0xf;
    *VQ=CONDIZ_DIVERSO & 0xf;                // converto una var in cond != 0
		}
//  PROCOper(LINE_TYPE_JUMPC,"jr",Alabel);

  return 0;
  }
 
int CPlusMinus::FNGetCondString(uint8_t A, uint8_t B) {   // (was: bit 5 di A vale 0 se signed, 1 unsigned
  
  if(B)
		A ^= 1;
//  if((A & 0x20 && A < 0x24))       // <, <=, >, >= possono essere signed o unsigned
//    A = (A & 0x1f) + 6;
	// NON SERVE PIU' 2025, v. condizioni _UNSIGNED e subCmp
//  if(A & 0x20 && (A < 0x24 || A>=0x26))       // questo serve cmq...
    A &= 0xf;

  return A;
  }
 
int CPlusMinus::PROCAssignCond(int8_t *VQ, O_TYPE *T, O_SIZE *S, char *Clabel) {
  char MyBuf[32],MyBuf1[32];
  int i;
				 
  i=*VQ & (VALUE_IS_CONDITION | 0xf);
	_tcscpy(MyBuf1,OpCond[FNGetCondString(i & VALUE_IS_CONDITION ? i : CONDIZ_UGUALE | ((i ^ 1) & 1),FALSE) & 0xf]);
	_tcslwr(MyBuf1);		// finezza :)
	_tcscpy(MyBuf,"SECL");
	switch(*S) {
		case 1:
			_tcscat(MyBuf,".b");
			break;
		case 2:
			_tcscat(MyBuf,".w");
			break;
		case 4:
			_tcscat(MyBuf,".d");
			break;
		}
  PROCOper(LINE_TYPE_ISTRUZIONE,MyBuf,MyBuf1);
  *T=VARTYPE_PLAIN_INT;
  *S=INT_SIZE;
  *VQ=VALUE_IS_EXPR;
  return 0;
  }
 
int CPlusMinus::PROCReturn() {

  if(!*OldTX[1].T) {
//		sprintf(OLDT[1],"_ret",Var[CurrFunc].name);     // era oltre 20 char...
		FNGetLabel(OldTX[1].T,1);
		*OldTX[1].T='R';
		}
//			PROCOper(LINE_TYPE_JUMP,"jr",OldTX[1].T);
	
  return 0;
  }
 
long CPlusMinus::FNGetConst(char *s,bool m) {
  int16_t i;
  struct OPERAND V;
  char Clabel[32];
  long T;
	char outbuf[256];
 
  ZeroMemory(&V,sizeof(struct OPERAND));
	V.Q=-5;
	V.cost=(union STR_LONG *)s;
  ZeroMemory(s,sizeof(union STR_LONG));
	*Clabel=0;
  i=0;
  isRValue=isPtrUsed=0;
	*outbuf=0;
  FNRev(outbuf,14,&i,Clabel,&V);		 //14, evitare virgole
  T=V.cost->l;
  
  if(debug)
		myLog->print(0,"Costante: %s",s);
	
  if(!m) {                          // accetto solo costanti
isError:
	  if(!(V.Q & VALUE_IS_COSTANTE))
			PROCError(2057);
	
isCost:
		if(V.Q==VALUE_IS_COSTANTE)	
		  ltoa(T,s,10);			// usare ulltoa
		else
			_tcscpy(s,V.cost->s);
    return T;
    }
  else {                            // accetto costanti e var statiche
    if(V.Q & VALUE_IS_COSTANTE)
      goto isCost;
    if(V.Q==VALUE_IS_VARIABILE) {
      if(V.var->classe<=CLASSE_STATIC)
        _tcscpy(s,V.var->label);
      else 
        goto isError;  
      }
    else   
      goto isError;  
    }
  return 0;  
  }
 
int CPlusMinus::PROCLoops(const char *T, const char *T1, const char *T2) {
  int I,i;
  char MyBuf[128];
	char outbuf[256];
 
  I=InBlock+1;
  if(T && *T) {
		_tcscpy(OldTX[I].T,T);
//          myLog->print("scrivo in %d, %s",I,T);
		}
  _tcscpy(OldTX[I].B,T1);
  _tcscpy(OldTX[I].C,T2);
	OldTX[I].flag=0;
  if(*FNLA(MyBuf) == '{') {
		Declaring=TRUE;
		if(I)
			OldTX[I].id = __line__;		// o rand()?
		FNLO(MyBuf);
		PROCBlock();
		}
  else {   
    if(!*MyBuf)                  // per ignorare i commenti... (PROCBLOCK non ne ha bisogno)
      FNLO(MyBuf);
		InBlock=I;
//          myLog->print("inblock: %d",InBlock);
rifoIsStmt:
		if(FNIsStmt()) {
	  	if(*FNLA(MyBuf) == ';')
				PROCCheck(';');
	  	if(!_tcscmp(FNLA(MyBuf),"else")) {      // è l'unico caso in cui lego uno stmt al successivo
		  	goto rifoIsStmt;
		  	}
	  	}
		else {
		  if(OutSource) {
				wsprintf(MyBuf,"|%5d| : ",__line__);
				FNGetLine(FIn->GetPosition(),MyBuf+10);
//				MyBuf[_tcslen(MyBuf)-2]=0;		// tolgo CR se no diventa doppio
				PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,MyBuf);
				}
			*outbuf=0;
	  	FNEvalExpr(outbuf,15,MyBuf);
			PROCOper(LINE_TYPE_ISTRUZIONE,outbuf);
	  	}
		InBlock--;
		}
	
  return 0;
  }
 
int8_t CPlusMinus::CmpDecorated(const char *s1,const char *s2) {

	// finire gestendo il "reference" ossia R e magari restituendo un valore di "somiglianza"
	return _tcscmp(s1,s2);

	// gemini 10/26
  const char *p1 = s1;
  const char *p2 = s2;

  // 1. Confronta il nome base prima del mangling (es. "foo__")
  while(*p1 && *p2 && (*p1 == *p2)) {
    if(*p1 == '_' && *(p1+1) == '_') {
      p1 += 2;
      p2 += 2;
      break; // Trovato il separatore "__", passiamo ai tipi
      }
    p1++;
    p2++;
    }

  // Se i nomi base differiscono, non c'è match
  if(*p1 == '\0' || *p2 == '\0' || (p1 - s1 != p2 - s2 && *(p1-1) != '_')) {
      // Gestisci eventuale disallineamento prima di '__'
	  }

  // 2. Analisi dei decoratori/tipi carattere per carattere
  bool isRef = false;

    // Se la firma nel simbolo dichiara un Reference ('R'), lo riconosciamo
  if(*p2 == 'R') {
    isRef = true;
    p2++; // Avanzi solo il puntatore della firma dichiarata
    }

    // 3. Confronta il resto dei tipi (es. "i" con "i", "Pi" con "Pi")
  while(*p1 && *p2) {
    if(*p1 != *p2)
      return -1; // Tipi incompatibili
    p1++;
    p2++;
    }

  // Entrambe le stringhe devono finire insieme
  if(*p1 == '\0' && *p2 == '\0')
    return isRef ? 1 : 0; // 0 = Match esatto, 1 = Match via Reference

  return -1;

	}

struct VARS *CPlusMinus::FNCercaVar(const char *N, bool M, struct TAGS **base) {
// M% TRUE=RICERCA NEL BLOCCO, FALSE RICERCA GLOBALE
  register int Bl;
  struct VARS *F, *V;

//  PROCV();
  
  Bl=InBlock;
  F=CurrFunc;
	if(base)
		*base=NULL;
  do {       
		if(!Bl) {
		  F=0;
		  }
		V=Var;
		while(V) {
//        myLog->print(0,"Cercavar\a: %s <> %s, livello %u",N,V->name,Bl);
			if(!Bl) {
				struct VARS *v;
				if(v=FNCercaVar(V->isInTag,N,base)) {	// forse serve anche sotto, se classe definita localmente
					return v;
					}
				else {
					if(!V->block) {
						if(!_tcscmp(N,V->name)) { 
							return V;
							}
						}
					}
				}
			else {
				if(!V->isInTag) {
					if(V->func.func==F) {

						//if(V->func.func->isInTag)
	//						v=FNCercaVar(CurrFunc->isInTag,TS);
						//else

						if(V->block==Bl && (/*!Bl inutile || */ V->blockId==OldTX[Bl].id)) {
							if(!CmpDecorated(N,V->name)) { 
								return V;
								}
							}
						}
					}
				}
		  V=V->next;
		  }
	//    if(Bl)
		  Bl--;
		} while(!M && (Bl>=0));

  return NULL;
  }
 
struct VARS *CPlusMinus::FNCercaVar(struct TAGS *tag,const char *N,struct TAGS **base) {
  struct VARS *F, *V;

	if(base)
		*base=NULL;

	do {
 		V=Var;
		while(V) {
			if(V->isInTag==tag) {

				// controllare blocco e funzione!
				if(!CmpDecorated(N,V->name)) { 
					if(base) {
						if(*base)
							V->hasBase=*base;
						else
							V->hasBase=NULL;
						}
					return V;
					}
				}
			V=V->next;
			}
		if(base && tag)
			*base=tag->parent;
		} while(tag && (tag=tag->parent));

  return NULL;
  }
 
struct VARS *CPlusMinus::FNCercaFunz(struct TAGS *tag,const char *N,const char *F,struct TAGS **base) {
  struct VARS *V;
  char T[256];

	if(base)
		*base=NULL;

	do {
		wsprintf(T,"%s_%s__%s",tag->label,N,F);
 		V=Var;
		while(V) {
			if(V->isInTag==tag) {

				// controllare blocco e funzione!
				if(!CmpDecorated(T,V->name)) { 
					if(base) {
						if(*base)
							V->hasBase=*base;
						else
							V->hasBase=NULL;
						}
					return V;
					}
				}
			V=V->next;
			}
		if(base && tag)
			*base=tag->parent;
		} while(tag && (tag=tag->parent));

  return NULL;
  }
 
struct VARS *CPlusMinus::FNCercaCtor(struct TAGS *tag,const char *M,bool is_ctor,struct TAGS **base) {
  //char T[256];

//	wsprintf(T,"%s%s__%s",tag->label,is_ctor ? ctor : dtor,M);

	return FNCercaFunz(tag,is_ctor ? ctor+1 : dtor+1,M,base);		// salto "_" !

  }
 
struct TAGS *CPlusMinus::FNCercaAggr(const char *N,bool M) {
	struct TAGS *C;

  C=StrTag;
  while(C) {
			// controllare blocco e funzione!
    if(!_tcscmp(N,C->label))
      break;
    C=C->next;
    } 

	return C;
	}

bool CPlusMinus::FNHasVar(struct TAGS *tag) {		// ossia "se è definita"
  struct VARS *V;

 	V=Var;
	while(V) {
		if(V->isInTag==tag) {
			return TRUE;
			}
		V=V->next;
		}

  return FALSE;
  }
 
bool CPlusMinus::FNHasVirtual(struct TAGS *tag) {
  struct VARS *V;

	if(tag->type != 2)
		return FALSE;

 	V=Var;
	while(V) {
		if(V->isInTag==tag && V->classe==CLASSE_MEMBER_VIRTUAL) {
			return TRUE;
			}
		V=V->next;
		}



  return TRUE;		// FINIRE usare
  }

struct VARS *CPlusMinus::PROCAllocVar(const char *N, O_TYPE Type, enum VAR_CLASSES Class, uint8_t Modif, O_SIZE Size, 
															 struct TAGS *Tag, O_DIM Dim) {
  struct VARS *V;
  char MyBuf[64],MyBuf1[64];
  
  V=(struct VARS*)GlobalAlloc(GPTR,sizeof(struct VARS)); 
  if(!V) {
fineMem:
    PROCError(1001,"Fine memoria VARS");
    }
  if(Var) {
    LVars->next=V;
		V->prev=LVars;
    LVars=V;
    }
  else {
    LVars=Var=V;
		V->prev=(struct VARS*)NULL;
    }
  V->next=(struct VARS*)NULL;
 
  if(debug)  
    myLog->print(0,"Alloco: %s, size %d",N,Size);

  V->type=Type;
	if(!(V->type & VARTYPE_FUNC)) {
		V->hasTag=Tag;
		V->isInTag=NULL;                   // attenzione: vedi IsDecl
		}
	else {
		V->hasTag=NULL;
		V->isInTag=Tag;                   // è una patch per ora...
		}
	if(Dim)
		memcpy(V->dim,Dim,sizeof(V->dim));
  _tcsncpy(V->name,N,MAX_NAME_LEN);
	V->name[MAX_NAME_LEN]=0;
  V->size=Size;
  V->block=InBlock;
  if(InBlock)
		V->blockId=OldTX[InBlock].id;
  V->func.func=CurrFunc;
  V->classe=Class;
	V->hasBase=NULL;
  V->modif=Modif;
	V->definition=NULL;
	V->members=NULL;
  if(Type & VARTYPE_FUNC) {			// anche Pointer
  	if(!(V->parm.ptr32=(int*)GlobalAlloc(GPTR,256)))
  	  goto fineMem;
		if(V->classe==CLASSE_MEMBER || V->classe==CLASSE_MEMBER_VIRTUAL /*Tag*/ /*Type & VARTYPE_CLASS*/) { // non static
			V->parm.ptr32[0+1] = VARTYPE_POINTER	| VARTYPE_CLASS;
			V->parm.ptr32[0+2] = getPtrSize(VARTYPE_POINTER);
		  *V->parm.ptr32=1;                // this
			struct VARS *v;
			v=PROCAllocVar("this",VARTYPE_POINTER,CLASSE_AUTO,0,getPtrSize(VARTYPE_POINTER),NULL /*V->isInTag*/,NULL);
			v->parm.ofs=8;		// primo parm ?
		  v->func.func=V;
			}
		else if(V->classe==CLASSE_MEMBER_STATIC /*Tag*/ /*Type & VARTYPE_CLASS*/) { // 
		  *V->parm.ptr32=0;                // no lista parm
			}
		else {
		  *V->parm.ptr32=0 /*-1*/;                // (no lista parm  DICE IN C++ Significa tassativamente f(void) (nessun parametro)
			}
		if(Modif & FUNC_MODIF_INLINE) {
			if(!(Type & VARTYPE_FUNC_POINTER))
//				V->definition=LastOut;		// ma poi viene cancellata... occorre copiare (v.)
;
			}
	  }
	else
		V->parm.ptr=NULL;
	V->decor=NULL;
	V->visibility=DefaultVisibility;
	
	if(CurrFunc && Class>=CLASSE_AUTO && Class<=CLASSE_REGISTER) {		// per ora li lascio entrambi!
	  struct VARS *v=CurrFunc->members,*v1=NULL;
		while(v) {
			v1=v;
			v=v->next;
			}
		v=(struct VARS *)GlobalAlloc(GPTR,sizeof(struct VARS)); 
		*v=*V;
		if(!v)
			PROCError(1001,"Fine memoria VARS");
		if(v1)
			v1->next=v;
		else
			CurrFunc->members=v;
		v->next=(struct VARS*)NULL;
		v->prev=v1;
		}
 
  return V;
  }

struct VARS *CPlusMinus::PROCAllocFunzProto(const char *name, O_TYPE type, O_SIZE size) {
	struct VARS *v;

	v=PROCAllocVar(name,type | VARTYPE_FUNC,CLASSE_EXTERN,0,size,NULL,NULL);
	if(v) {
		v->func.func=NULL;		// forzatura ovvia! 
		v->block=0;		// forzatura ovvia! faccio qua
		v->blockId=0;
		}
  return v;
	}

struct VARS *CPlusMinus::PROCAllocGoto(const char *label) {
	struct VARS *g,*g2,*g3;

	g=(struct VARS*)GlobalAlloc(GPTR,sizeof(struct VARS)); 
	if(!g) {
		PROCError(1001,"Fine memoria GOTO");
		}
	if(CurrFuncGotos) {
		g2=g3=CurrFuncGotos;
		while(g2) {
			g3=g2;
			g2=g2->next;
			}
		g3->next=g;
		}
	else {
		CurrFuncGotos=g;
		}
	_tcsncpy(g->label,label,sizeof(g->label));		// uso questo e non Name...
	g->next=(struct VARS *)NULL;
	return g;
	}

struct VARS *CPlusMinus::FNCercaGoto(const char *N) {
  register int Bl;
  struct VARS *V;
  
	V=CurrFuncGotos;
	while(V) {
		if(!_tcscmp(N,V->label)) { 
			return V;
			}
		V=V->next;
		}

  return NULL;
  }
 
struct ENUMS *CPlusMinus::FNCercaEnum(const char *tag, const char *N, bool M) {
// M% TRUE=RICERCA NEL BLOCCO, FALSE RICERCA GLOBALE
  register int Bl;
  struct ENUMS *E;
	struct VARS *V;

// usare tag??
  Bl=InBlock;
  V=CurrFunc;
  do {       
		if(!Bl) {
		  V=NULL;
		  }
		E=Enums;
		while(E) {
			if(E->var.func==V) {
			  if(E->var.block==Bl) {
					if(!_tcscmp(N,E->name)) { 
					  return E;
						}
				  }
				}
		  E=E->next;
		  }
	//    if(Bl)
		  Bl--;
		} while(!M && (Bl>=0));

  return NULL;
  }
 
 
int CPlusMinus::PROCCast(O_TYPE T1, O_SIZE S1, O_TYPE *T2, O_SIZE *S2, int8_t reg) {
// T2 e S2 si tramutano in T1 e S1
  char myBuf[64];
	int8_t reg2;
	O_SIZE s2;
	O_TYPE t3;
  
	reg2=reg+1;		// per Z80 e/o 8086, finire!

  if(*S2 != S1 || T1 != *T2)	// gestire array vs ptr, aggregati ecc
							PROCOper(LINE_TYPE_ISTRUZIONE | LINE_TYPE_COMMENTO,"cast");

  S1=FNGetMemSize(T1,S1,NULL/*dim*/,1);
  s2=FNGetMemSize(*T2,*S2,NULL/*dim*/,1);
  if((T1 & VARTYPE_UNSIGNED) ^ (*T2 & VARTYPE_UNSIGNED))
		PROCWarn(4018);


  if(s2 != S1 /*|| (T1 & VARTYPE_POINTER != T2 & VARTYPE_POINTER) sottinteso quindi */) {
		switch(S1) {
		  case 1:
				break;
		  case 2:
				switch(s2) {
				  case 1:
						break;
				  case 4:
						break;
				  default:
						break;
				  }
				break;
		  case 4:
				switch(s2) {
				  case 1:
						break;
				  case 2:
						break;
				  default:
						break;
				  }
				break;
		  default:
				break;
		  }
		}      
	
//	t3=*T2 & 0x1FF00000L;		// preservo alcune cose (verificare)
//	*T2=T1 & ~0x1FF0400FL;
	t3=*T2 & ~VARTYPE_UNSIGNED;		// preservo alcune cose (verificare)
	*T2=T1 & VARTYPE_UNSIGNED;
	*T2 |= t3;
	*S2=S1;
  return 1; 
  }



struct CONS *CPlusMinus::FNAllocCost(const char *A, uint8_t mode, O_TYPE Type) {
  int i;
  char MyBuf[256],MyBuf2[256];
  struct CONS *C;
  
  C=(struct CONS*)GlobalAlloc(GPTR,sizeof(struct CONS)); 
  if(!C) {
    PROCError(1001,"Fine memoria CONS");
    }
 
  if(Con) {
    LCons->next=C;
    LCons=C;
    }
  else {
    LCons=Con=C;
    }
  C->next=(struct CONS *)NULL;


  _tcsncpy(C->name,A,MAX_NAME_LEN);
	C->name[MAX_NAME_LEN]=0;
  _tcscpy(C->label,FNGetLabel(MyBuf,0));
  switch(mode) {
		case 1:
		  PROCOut1(FO3,MyBuf," DB ",A);
			break;
	  case 2:
				PROCOut1(FO3,MyBuf," DW ",A);
			break;
	  case 3:
				PROCOut1(FO3,MyBuf," DD ",A);
			break;
	  case 4:		// 8 byte, come per double
			PROCOut1(FO3,MyBuf," DQ 0x",A);
			break;
		}
  return C;
  }

struct ENUMS *CPlusMinus::FNAllocEnum(const char *tag, const char *A, uint32_t value, O_SIZE Size) {
  int i;
  char MyBuf[256],MyBuf2[256];
  struct ENUMS *C;
  
  C=(struct ENUMS*)GlobalAlloc(GPTR,sizeof(struct ENUMS)); 
  if(!C) {
    PROCError(1001,"Fine memoria ENUMS");
    }

	//FINIRE con tag!
 
  if(Enums) {
    LEnums->next=C;
    LEnums=C;
    }
  else {
    LEnums=Enums=C;
    }
  C->next=(struct ENUMS *)NULL;
 
  _tcsncpy(C->name,A,MAX_NAME_LEN);
	C->name[MAX_NAME_LEN]=0;
	C->var.value=value;
  return C;
  }


int CPlusMinus::PROCInit() {
  int i,t;
	char myBuf[128];
						   
//	myOutput->PostMessage(WM_CLSWINDOW,0,(LPARAM)myBuf);

  _strdate(__date__);
  t=__date__[3];
  __date__[3]=__date__[0];
  __date__[0]=t;
  t=__date__[4];
  __date__[4]=__date__[1];
  __date__[1]=t;
  _strtime(__time__);
  if(!NoMacro) {                      
		m_CPre->PROCDefine("DARIO","1");
		}
  wsprintf(myBuf,"Traduttore da C++ a C di G.Dar, (C) 2023-2026 - Versione %d.%02d",HIBYTE(__VER__),LOBYTE(__VER__));
	if(myOutput) {
		char *p=(LPSTR)GlobalAlloc(GPTR,256);
		_tcscpy(p,myBuf);
		myOutput->PostMessage(WM_ADDTEXT,0,(LPARAM)p);
		}
	
	for(i=0; i<MAX_BLOCCHI; i++)
		ZeroMemory(&OldTX[i],sizeof(BLOCK_PTR));
  return 0;                        
  }



CString CTimeEx::getNow(int ex) {
	int i;
	CString S;

	switch(ex) {
		case 0:
			S=CTime::GetCurrentTime().Format("%d/%m/%Y %H:%M:%S");
			break;
		case 1:
		case 2:
			S.Format(ex == 1 ? _T("sono le ore %d e %d del %d %s %d") : _T("sono le ore %02d e %02d del %d %s %d"),
				GetCurrentTime().GetHour(),GetCurrentTime().GetMinute(),
				GetCurrentTime().GetDay(),(LPTSTR)(LPCTSTR)Num2Mese(GetCurrentTime().GetMonth()),
				GetCurrentTime().GetYear());
			break;
		}

	return S;
  }


CString CTimeEx::getNowGMT(bool bAddCR) {
	time_t aclock;
	struct tm *newtime;
	int i;
	CString S;

	S.Format(_T("%s %s %02u %02u:%02u:%02u %04u"),
		Num2Day3(CTime::GetCurrentTime().GetDayOfWeek()),
		Num2Month3(CTime::GetCurrentTime().GetMonth()),
		CTime::GetCurrentTime().GetDay(),
		CTime::GetCurrentTime().GetHour(),
		CTime::GetCurrentTime().GetMinute(),
		CTime::GetCurrentTime().GetSecond(),
		CTime::GetCurrentTime().GetYear());

#ifndef _WIN32_WCE
	i=-(_timezone/3600)+(_daylight ? 1 : 0);
#else
	i=0;
#endif
	if(!i)
		S+=_T(" UTC");				// anche "GMT"
	else {
		CString S2;
		S2.Format(_T(" %c%02d00"),i>=0 ? '+' : '-',abs(i));
		S+=S2;
		}
	if(bAddCR)
		S+="\r\n";

	return S;
	}

CString CTimeEx::getNowGoogle(bool bAddCR) {
	time_t aclock;
	struct tm *newtime;
	int i;
	CString S;

	S.Format(_T("%04u-%02u-%02uT%02u:%02u:%02uZ"),
		CTime::GetCurrentTime().GetYear(),
		CTime::GetCurrentTime().GetMonth(),
		CTime::GetCurrentTime().GetDay(),
		CTime::GetCurrentTime().GetHour(),
		CTime::GetCurrentTime().GetMinute(),
		CTime::GetCurrentTime().GetSecond());

#ifndef _WIN32_WCE
	i=-(_timezone/3600)+(_daylight ? 1 : 0);
#else
	i=0;
#endif
	if(!i)
		S+=_T(" UTC");				// anche "GMT"
	else {
		CString S2;
		S2.Format(_T(" %c%02d00"),i>=0 ? '+' : '-',abs(i));
		S+=S2;
		}
	if(bAddCR)
		S+="\r\n";

	return S;
	}

CString CTimeEx::Num2Mese(int i) {

  switch(i) {
		case 1:
      return _T("Gennaio");
			break;
		case 2:
      return _T("Febbraio");
			break;
		case 3:
      return _T("Marzo");
			break;
		case 4:
      return _T("Aprile");
			break;
		case 5:
      return _T("Maggio");
			break;
		case 6:
      return _T("Giugno");
			break;
		case 7:
      return _T("Luglio");
			break;
		case 8:
      return _T("Agosto");
			break;
		case 9:
	    return _T("Settembre");
			break;
		case 10:
      return _T("Ottobre");
			break;
		case 11:
      return _T("Novembre");
			break;
		case 12:
      return _T("Dicembre");
			break;
	  }
  
  }

CString CTimeEx::Num2Giorno(int i) {

  switch(i) {
		case 1:
      return _T("Domenica");
			break;
		case 2:
      return _T("Luned");
			break;
		case 3:
      return _T("Marted");
			break;
		case 4:
      return _T("Mercoled");
			break;
		case 5:
      return _T("Gioved");
			break;
		case 6:
      return _T("Venerd");
			break;
		case 7:
      return _T("Sabato");
			break;
	  }
  }

CString CTimeEx::Num2Month3(int i) {

  switch(i) {
		case 1:
      return _T("Jan");
			break;
		case 2:
      return _T("Feb");
			break;
		case 3:
      return _T("Mar");
			break;
		case 4:
      return _T("Apr");
			break;
		case 5:
      return _T("May");
			break;
		case 6:
      return _T("Jun");
			break;
		case 7:
      return _T("Jul");
			break;
		case 8:
      return _T("Aug");
			break;
		case 9:
	    return _T("Sep");
			break;
		case 10:
      return _T("Oct");
			break;
		case 11:
      return _T("Nov");
			break;
		case 12:
      return _T("Dec");
			break;
	  }
  
  }

CString CTimeEx::Num2Day3(int i) {

  switch(i) {
		case 1:
      return _T("Sun");
			break;
		case 2:
      return _T("Mon");
			break;
		case 3:
      return _T("Tue");
			break;
		case 4:
      return _T("Wed");
			break;
		case 5:
      return _T("Thu");
			break;
		case 6:
      return _T("Fri");
			break;
		case 7:
      return _T("Sat");
			break;
	  }
  }

int CTimeEx::getMonthFromString(const CString S) {

	if(!S.CompareNoCase(_T("JAN")))
		return 1;
	else if(!S.CompareNoCase(_T("FEB")))
		return 2;
	else if(!S.CompareNoCase(_T("MAR")))
		return 3;
	else if(!S.CompareNoCase(_T("APR")))
		return 4;
	else if(!S.CompareNoCase(_T("MAY")))
		return 5;
	else if(!S.CompareNoCase(_T("JUN")))
		return 6;
	else if(!S.CompareNoCase(_T("JUL")))
		return 7;
	else if(!S.CompareNoCase(_T("AUG")))
		return 8;
	else if(!S.CompareNoCase(_T("SEP")))
		return 9;
	else if(!S.CompareNoCase(_T("OCT")))
		return 10;
	else if(!S.CompareNoCase(_T("NOV")))
		return 11;
	else if(!S.CompareNoCase(_T("DEC")))
		return 12;

	return 0;
	}

int CTimeEx::getMonthFromGMTString(const CString S) {
	CString S2=S.Left(3);

	return getMonthFromString(S2);

	}


CString CTimeEx::getFasciaDellaGiornata() {

	if(GetCurrentTime().GetHour() >=7 && GetCurrentTime().GetHour()<13)
		return _T("stamattina");
	else if(GetCurrentTime().GetHour() >=13 && GetCurrentTime().GetHour()<20)
		return _T("oggi");
	else if(GetCurrentTime().GetHour() >=20 && GetCurrentTime().GetHour()<24)
		return _T("stasera");
	else if(GetCurrentTime().GetHour() >=0 && GetCurrentTime().GetHour()<7)
		return _T("stanotte");

	}

CString CTimeEx::getSaluto() {

	if(GetCurrentTime().GetHour() >=7 && GetCurrentTime().GetHour()<13)
		return _T("Buongiorno");
	else if(GetCurrentTime().GetHour() >=13 && GetCurrentTime().GetHour()<20)
		return _T("Buon pomeriggio");
	else if(GetCurrentTime().GetHour() >=20 && GetCurrentTime().GetHour()<24)
		return _T("Buonasera");
	else if(GetCurrentTime().GetHour() >=0 && GetCurrentTime().GetHour()<7)
		return _T("Buonanotte");

	}

CTime CTimeEx::parseGMTTime(const CString S) {
	char *p;
	int i,j,tzFound=0,reverseUTC=0;
	struct tm t;
	CString s=S;
	
//	_tzset();			// questo imposterebbe la timezone, che altrimenti potrebbe defaultare a -8h
	// v. Joshua.cpp::InitInstance

	while(_istspace(s.GetAt(0)))
		s=s.Mid(1);
	if(_istalpha(s.GetAt(0))) {
		s.MakeUpper();
		if(!s.Left(2).CompareNoCase(_T("SU")))
			i=0;
		else if(!s.Left(2).CompareNoCase(_T("MO")))
			i=1;
		else if(!s.Left(2).CompareNoCase(_T("TU")))
			i=2;
		else if(!s.Left(2).CompareNoCase(_T("WE")))
			i=3;
		else if(!s.Left(2).CompareNoCase(_T("TH")))
			i=4;
		else if(!s.Left(2).CompareNoCase(_T("FR")))
			i=5;
		else if(!s.Left(2).CompareNoCase(_T("SA")))
			i=6;
		else				// NON deve capitare... patch per evitare il peggio!
			i=0;
		t.tm_wday=i;
		s=s.Mid(3);
		while(!isspace(s.GetAt(0)))
			s=s.Mid(1);
		if(s.GetAt(0) ==',')
			s=s.Mid(1);
		s=s.Mid(1);
no_day:
		while(_istspace(s.GetAt(0)))
			s=s.Mid(1);
		if(_istdigit(s.GetAt(0))) {
			t.tm_mday=_ttoi((LPTSTR)(LPCTSTR)s);
			while(iswdigit(s.GetAt(0)))
				s=s.Mid(1);
			s=s.Mid(1);
			t.tm_mon=getMonthFromGMTString(s);
			s=s.Mid(4);
			t.tm_year=_ttoi((LPTSTR)(LPCTSTR)s);
			if(t.tm_year<80)
				t.tm_year+=100;
			if(t.tm_year>=200)
				t.tm_year-=1900;
			s=s.Mid(5);
			t.tm_hour=_ttoi(s);
			s=s.Mid(3);
			t.tm_min=_ttoi(s);
			s=s.Mid(3);
			t.tm_sec=_ttoi(s);
			if(s.GetAt(1) == '-')
				reverseUTC=1;
			s=s.Mid(2,2);
			i=_ttoi(s);
			if(reverseUTC)
				i=-i;
			}
		else {
			t.tm_mon=getMonthFromGMTString(s);
			s=s.Mid(4);
			t.tm_mday=_ttoi(s);
			s=s.Mid(3);
			t.tm_hour=_ttoi(s);
			s=s.Mid(3);
			t.tm_min=_ttoi(s);
			s=s.Mid(3);
			t.tm_sec=_ttoi(s);
			s=s.Mid(3);
			if(s.GetAt(0) == 'U') {		// variante con "UTC 1998
				t.tm_year=_ttoi(s.Mid(4))-1900;
				i=0;								// greenwich
				tzFound=1;
				}
			else {							  // variante con 1998 +0100
				t.tm_year=_ttoi(s)-1900;
				s=s.Mid(4);
				while(s.GetLength()>0 && _istspace(s.GetAt(0)))
					s=s.Mid(1);
				if(s.GetLength()>0) {		// se la timezone segue l'anno...
					if(_istdigit(s.GetAt(0)) || s.GetAt(0)=='+' || s.GetAt(0)=='-') {
						tzFound=1;
						if(s.GetAt(0) == '-')
							reverseUTC=1;
						if(s.GetAt(0)=='+' || s.GetAt(0)=='-')
							s=s.Mid(1);
						s=s.Mid(0,2);		// stronco i minuti!
						i=_ttoi(s);									// ...la leggo
						if(reverseUTC)
							i=-i;
						}
					else if(S.GetAt(0)=='U' || S.GetAt(0)=='G')	{		// per ora solo UTC o GMT (sono lo stesso?)!
						i=0;								// greenwich
						tzFound=1;
						}
					}
				}
			}
		}
	else {
		if(_istalpha(s.GetAt(4))) {		// caso in cui manca il giorno della settimana, poi idem come sopra...
			s.MakeUpper();
			t.tm_wday=0;
			goto no_day;
			}
		else {
			s=s.Mid(2);		// salto \xd\xa
			s=s.Mid(6);
			t.tm_year=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			t.tm_mon=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			t.tm_mday=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			t.tm_hour=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			t.tm_min=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			t.tm_sec=_ttoi((LPTSTR)(LPCTSTR)s);
			s=s.Mid(3);
			i=_ttoi((LPTSTR)(LPCTSTR)s);
			}
		}
	if(tzFound)
#ifndef _WIN32_WCE
		t.tm_hour=t.tm_hour-i-(_timezone/3600)+(_daylight ? 1 : 0);
#else
		t.tm_hour=t.tm_hour-i-(0/3600)+0;
#endif

	return CTime::CTime(t.tm_year+1900,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
	}

CTime CTimeEx::parseTime(const CString S) {		// semplice, tipo "05/10/10 12:00"
	char *p;
	int i,j,tzFound=0;
	WORD nDay,nMonth,nYear,nHour,nMinute,nSecond;
	CString s=S;
	
//	_tzset();			// questo imposterebbe la timezone, che altrimenti potrebbe defaultare a -8h
	// v. Joshua.cpp::InitInstance

	p=(char *)(LPCTSTR)S;
	while(isspace(*p) && *p)
		p++;
	nDay=atoi(p);		// 2 digit
	while(isdigit(*p))
		p++;
	p++;
	while(isspace(*p) && *p)
		p++;
	nMonth=atoi(p);			// 2 digit
	while(isdigit(*p))
		p++;
	p++;
	while(isspace(*p) && *p)
		p++;
	nYear=atoi(p);		// 2-4 digit (v.sotto)
	while(isdigit(*p))
		p++;

	nHour=0;
	nMinute=0;
	nSecond=0;
	if(*p) {
		p++;
		while(isspace(*p) && *p)
			p++;
		nHour=atoi(p);		// 2 digit
		while(isdigit(*p))
			p++;
		p++;
		while(isspace(*p) && *p)
			p++;
		nMinute=atoi(p);	// 2 digit

		while(isdigit(*p))
			p++;
		if(*p) {
			p++;
			while(isspace(*p) && *p)
				p++;
			nSecond=atoi(p); // 2 digit
			}
		}

	if(nYear < 100) {
		if(nYear < 80)
			nYear+=2000;
		else
			nYear+=1900;
		}


	i=0;
	if(tzFound)
#ifndef _WIN32_WCE
		nHour=nHour-i-(_timezone/3600)+(_daylight ? 1 : 0);
#else
		nHour=nHour-i-(0/3600)+0;
#endif

	return CTime::CTime(nYear,nMonth,nDay,nHour,nMinute,nSecond);
	}

bool CTimeEx::isWeekend() {

	return GetCurrentTime().GetDayOfWeek() == 1 || GetCurrentTime().GetDayOfWeek() == 7;
	}

bool CTimeEx::isWeekend(CTime t) {

	return t.GetDayOfWeek() == 1 || t.GetDayOfWeek() == 7;
	}

WORD CTimeEx::GetDayOfYear() {
	struct tm *myTm;

	myTm=CTime::GetCurrentTime().GetLocalTm();
	return myTm->tm_yday;
	}

void CTimeEx::AddMonths(int n) {
	int nYear  = GetYear();
	int nMonth = GetMonth();

	while(n--) {
		nMonth++;
		if(nMonth > 12) {
			nMonth = 1;
			++nYear;
			}
		}
	// construct first day of next month  
	CTimeEx tNext(nYear, nMonth, 1, 0, 0, 0); 
	// get the number of days in the next month
	int nDays = tNext.GetDaysOfMonth();
	// construct the date for next month
	int nNewDay = min(nDays, GetDay());
	CTimeEx tNew(nYear, nMonth, 
						nNewDay, 
						GetHour(), GetMinute(), GetSecond());
   // assign the new date
  *this = tNew;
	}

int CTimeEx::GetDaysOfMonth() { 
   CTimeEx tNext(GetYear(), GetMonth(), 1, 0, 0, 0); 

   tNext += CTimeSpan(31, 1, 0, 0); 

   return 32 - tNext.GetDay();
	}



//-------------------------------------------------------
/*
CinziaG    5.8.2017
*/

//--------------------------------------------------------------
CLogFile::CLogFile(const CString s, const CWnd *myWnd, DWORD m) :
	nomeFile(s),textWnd(myWnd),mode(m) { 
	
	hIndexFile=NULL;
	if((mode & 0xff) >= dateTimeMillisec) {
		timeBeginPeriod(1);
		}
	if(mode & keepOpen) {
	try {
		if(Open())
			SeekToEnd(); 
		}
	catch(CFileException e) {
		;
		}
	
		}

	if(mode & useIndex)
		hIndexFile=new CFile;

	InitializeCriticalSection(&m_cs);
	}

CLogFile::CLogFile(CFile *f2,const CWnd *myWnd,DWORD m) :
	textWnd(myWnd),mode(m) { 

	m_hFile=f2->m_hFile;
	hIndexFile=NULL;
	mode &= ~keepOpen;
	if((mode & 0xff) >= dateTimeMillisec) {
		timeBeginPeriod(1);
		}
	try {
		SeekToEnd(); 
		}
	catch(CFileException e) {
		;
		}
	
	if(mode & useIndex)
		hIndexFile=new CFile;

	}

CLogFile::~CLogFile() { 

	if((mode & 0xff) >= dateTimeMillisec) {
		timeEndPeriod(1);
		}

	if(mode & keepOpen)
		Close();

	if(hIndexFile)
		delete hIndexFile;			hIndexFile=NULL;

	}


int CLogFile::Open() { 
	int i;
	
	i=CStdioFile::Open(nomeFile,CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::typeText /*2023 per multithread.. | CFile::shareDenyWrite*/);

	if(mode & useIndex) {
		getIndexFileName();
		if(hIndexFile)
			hIndexFile->Open(nomeFileNdx,CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite /*| CFile::shareDenyWrite*/);
		}
	
	return i;
	}

void CLogFile::Close() { 
	
	CStdioFile::Close();

	if(mode & useIndex) {
		if(hIndexFile)
			hIndexFile->Close();
		}
	}


// RICORDARSI DI CASTARE A int gli eventuali valori 32bit che sforano (-2miliardi o + 2 miliardi) o vengono mal-messi dalla chiamata...
int CLogFile::print(int m,const TCHAR *s,...) {		 // m=0 info, 1= letture skynet, 2=errore
	TCHAR myBuf[2048],myBuf1[2048],myFmt[16];
	register int i,j,ch,k;
	int n,pad;
	double d;
	CTime myT;
	TCHAR *p,*p_myBuf;
//	static bool inUse;
  va_list vl;
	CString S;
	char padch=' ';

//	if(mode & keepOpen) {
//		while(inUse)
//			Sleep(20);
//		}
//	if(!inUse) {

	EnterCriticalSection(&m_cs);

//		inUse=1;
	va_start(vl,s);
	p_myBuf=myBuf;
	if(m & 0x100) {			// se un flag almeno...
		myBuf[0]=(m & 0xff)+'$';				// flag tipo riga
		myBuf[1]=' ';			// marker di 'gi letto'
		p_myBuf=myBuf+2;
		}
	if(mode & 0xff) {			// no dontUseDate...
		S=getNow();
		_tcscpy(p_myBuf,(LPTSTR)(LPCTSTR)S);
		j=_tcslen(myBuf);
		myBuf[j++]=' ';
		p_myBuf=&myBuf[j];
		}
	else
		j=p_myBuf-myBuf;
	myBuf[j]=0;
	i=0;
	while(ch=s[i++]) {
		pad=0;
		if(ch == '%') {
			if(s[i] == '%') {
				i++;
				goto no_var;
				}
			myFmt[0]=ch;
			k=1;
			while((ch=s[i++]) && !isalpha(ch))			// salvo dettagli...
				myFmt[k++]=ch;
			myFmt[k++]=ch;			// ...copio effettivo format-type
			myFmt[k]=0;
			if(myFmt[1]=='-') {
				if(isdigit(myFmt[2])) {
					pad=-atoi(&myFmt[2]);
					}
				// altrimenti...
				}
			else if(myFmt[1]=='+') {
				// segno ...
				}
			if(isdigit(myFmt[1])) {
				pad=atoi(&myFmt[1]);
				}
			switch(myFmt[k-1]) {
				case 'l':
				case 'L':
//					k++;
					// bah me ne fotto e proseguo!
					myFmt[k-1]=s[i++];
				case 'd':
				case 'D':
					n=va_arg(vl,int);
					goto subCopia0;
					break;
				case 'p':
				case 'P':
					myBuf[j++]='0';
					myBuf[j++]='x';
				case 'u':
				case 'U':
				case 'x':
				case 'X':
				case 'o':		//ottale
				case 'O':
					n=va_arg(vl,unsigned int);
subCopia0:
					sprintf(myBuf1,myFmt,n);
subCopia:
					if(pad) {
						pad-=_tcslen(myBuf1);
						if(pad<0)
							pad=0;
						while(pad--)
							myBuf[j++]=padch;		//put(' ');
						}
					_tcscpy(myBuf+j,myBuf1);
subCopia2:
					j+=_tcslen(myBuf1);
					break;
				case 'f':
				case 'g':
					d=va_arg(vl,double);
					sprintf(myBuf1,myFmt,d);
					goto subCopia;
					break;
				case 's':
					p=va_arg(vl,TCHAR *);
/*					if(p<(char*)100)
						p="culo";*/
					if(pad) {
						pad-=_tcslen(p);
						if(pad<0)
							pad=0;
						while(pad--)
							myBuf[j++]=padch;		//put(' ');
						}
					_tcsncpy(myBuf+j,p,2000-j);
					j+=min(_tcslen(p),2000-j);
					myBuf[j]=0;
					break;
				case 'c':
					n=va_arg(vl,char);
					myBuf[j++]=n;
					break;
				case 't':
					myT=va_arg(vl,CTime);
					S.Format(_T("%02u/%02u/%02u %02u:%02u:%02u"),
						myT.GetDay(),
						myT.GetMonth(),
						myT.GetYear(),
						myT.GetHour(),
						myT.GetMinute(),
						myT.GetSecond());
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
				case 'T':
					myT=va_arg(vl,CTime);
					S=myT.Format(_T("%02u %b %04u %02u:%02u:%02u"));
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
/*					case '%':
					myBuf[j++]='%';
					break;*/
				default:
					break;
				}
			}
		else {
//			if(ch == '\n')		perch?? 2026
//				myBuf[j++]=13;
no_var:
			myBuf[j++]=ch;
			}
		if(j>2000)
			break;
		}

	myBuf[j]=0;
//	((CMainFrame *)theApp.m_pMainWnd)->m_wndStatusBar.SetWindowText(myBuf+2);
	if(textWnd && *textWnd) {
		TCHAR *p=_tcsdup(myBuf+2);
		if(p) {
			if(::IsWindow(textWnd->m_hWnd))	{// serve in chiusura...
//					if(*p=='\'')
//						((CWnd *)textWnd)->PostMessage(WM_UPDATE_PANE2,0,(DWORD)p);	//a che serviva?? la 2 label la uso per server web...
//					else
					((CWnd *)textWnd)->PostMessage(WM_UPDATE_PANE,0,(DWORD)p);
				}
			else
				free(p);
			}
		else
			free(p);
		}
//	if(textWnd && *textWnd && ((CMainFrame *)(*textWnd))->m_wndStatusBar)
	//	((CMainFrame *)*textWnd)->m_wndStatusBar.SetPaneText(1,myBuf+2,TRUE);

//		myBuf[j++]=13;
	myBuf[j++]=10;
	myBuf[j]=0;

	i=0;
try {
	if(mode & keepOpen)
		goto already_open;
	if(Open()) {
		n=SeekToEnd(); 
already_open:
		WriteString(myBuf);

		if(mode & useIndex) {
			hIndexFile->SeekToEnd();
			hIndexFile->Write(&n,sizeof(DWORD));
			}

		if(mode & flushImmediate)
			Flush();	 // rimesso... anche se rallenta!
	//FlushFileBuffers
	if(!(mode & keepOpen))
		Close();
		}
	}
catch(CFileException e) {
	AfxMessageBox(e.m_cause);
	i=-1;
	}


	va_end(vl);
//		inUse=0;
	LeaveCriticalSection(&m_cs);
//		}
	return i;
  }

void CLogFile::operator<<(const TCHAR *s) {
	char *myBuf;
	int i,j;

	myBuf=new char[_tcslen(s)+1+64];

	myBuf[0]=(flagInfo & 0xff) + '$';				// flag tipo riga, sempre 0!
	myBuf[1]=' ';				// marker di 'gi letto'
	_tcscpy(myBuf+2,(LPTSTR)(LPCTSTR)getNow());
	j=_tcslen(myBuf);
	myBuf[j++]=' ';
//	i=_tcslen(s);
//	_tcsncpy(myBuf+j,s,min(i+1,1000));
	_tcscpy(myBuf+j,s);
	j=_tcslen(myBuf);
//	myBuf[j++]=13;
	myBuf[j++]=10;
	myBuf[j]=0;

	try {
		if(mode & keepOpen)
			goto already_open;
		if(Open()) {
			SeekToEnd(); 
already_open:
			WriteString(myBuf);

			if(mode & useIndex) {
				int n=SeekToEnd(); 
				hIndexFile->SeekToEnd();
				hIndexFile->Write(&n,sizeof(DWORD));
				}

			if(mode & flushImmediate)
				Flush();	 // rimesso... anche se rallenta!
		//FlushFileBuffers
		if(!(mode & keepOpen))
			Close();
			}
		}
	catch(CFileException e) {
		i=-1;
		}

	delete myBuf;
	}

CString CLogFile::getNow() const {
	int m=mode & 0xffff;
	CString S;

	S.Format(_T("%02u/%02u/%02u"),
		CTime::GetCurrentTime().GetDay(),
		CTime::GetCurrentTime().GetMonth(),
		CTime::GetCurrentTime().GetYear());
	if((mode & 0xff) >= dateTime) {
		CString S2;
		S2.Format(_T("%02u:%02u:%02u"),
			CTime::GetCurrentTime().GetHour(),
			CTime::GetCurrentTime().GetMinute(),
			CTime::GetCurrentTime().GetSecond());
		S+=_T(" ");
		S+=S2;
	if((mode & 0xff) >= dateTimeMillisec) {
			CString S3;
			S3.Format(_T("%u"),/*GetTickCount*/ timeGetTime());
			S+=_T(" ");
			S+=S3;
			}
		}

	return S;
	}

CString CLogFile::getNowApache() {
	CString S;

	S.Format(_T("%02u/%s/%02u:%02u:%02u:%02u %02d00"),
		CTime::GetCurrentTime().GetDay(),
		CTimeEx::Num2Month3(CTime::GetCurrentTime().GetMonth()),
		CTime::GetCurrentTime().GetYear(),
		CTime::GetCurrentTime().GetHour(),
		CTime::GetCurrentTime().GetMinute(),
		CTime::GetCurrentTime().GetSecond(),
		-(_timezone/3600)+(_daylight ? 1 : 0)
		);

	return S;
	}

char *CLogFile::getLine(int n,char *s,UINT nMax) {
	char myBuf[1024];
	CStdioFile mF;

	if(!n)
		goto errore;

	if(mode & useIndex) {
		int myPos;

		if(mF.Open(nomeFile,CFile::modeRead | CFile::typeText | CFile::shareDenyNone)) {
			CFile mF2;

			if(mF2.Open(nomeFileNdx,CFile::modeRead | CFile::shareDenyNone)) {
				mF2.Seek(n*sizeof(DWORD),CFile::begin);
				mF2.Read(&myPos,sizeof(DWORD));
				mF2.Close();

				mF.Seek(myPos,CFile::begin);
				mF.ReadString(myBuf,nMax);
				mF.Close();
				_tcscpy(s,myBuf);
				}
			}
		}
	else {

		nMax=min(nMax,1000);
		if(mF.Open(nomeFile,CFile::modeRead | CFile::typeText | CFile::shareDenyNone)) {
			while(n) {
				if(!mF.ReadString(myBuf,nMax))
					break;
				n--;
				}
			mF.Close();
			if(!n)
				_tcscpy(s,myBuf);
			else
				goto errore;
			}
		else {
errore:
			s=NULL;
			}
		}

fine:
	return s;
	}

DWORD CLogFile::getTotLines() const {
	DWORD n=0;
	char myBuf[1024];
	CStdioFile mF;

	if(mode & useIndex) {
		if(mF.Open(nomeFileNdx,CFile::modeRead | CFile::shareDenyNone)) {
			n=mF.SeekToEnd();
			mF.Close();

			n/=sizeof(DWORD);
			}

		}

	else {
		if(mF.Open(nomeFile,CFile::modeRead | CFile::typeText | CFile::shareDenyNone)) {
			while(mF.ReadString(myBuf,1000))
				n++;
			mF.Close();
			}
		}

	// fare un confronto tra i due e forzare ReIndex?? oppure... usare l'uno Oppure l'altro?


	return n;
	}

int CLogFile::clearAll() {
	
	if(mode & keepOpen)
		Close();
	CStdioFile::Open(nomeFile,CFile::modeCreate);
	CStdioFile::Close();
	if(mode & useIndex)
		ReIndex();
	if(mode & keepOpen)
		Open();
	return 1;
	}

bool CLogFile::GetStatus(CFileStatus &fs) {

	if(mode & keepOpen)
		return CStdioFile::GetStatus(fs) ? TRUE : FALSE;
	else {
		if(Open()) {
			int i=CStdioFile::GetStatus(fs);
			Close();
			return i ? TRUE : FALSE;
			}
		else
			return FALSE;
		}
	}


char *CLogFile::getAsHex(const byte *s,char *d,UINT nMax) {

	while(nMax--) {
		wsprintf(d,"%02X ",*s);
		d+=3;
		s++;
		}

	return d;
	}

#ifdef _WIN32_WCE

void CStdioFileEx::WriteString(CString S) {
	
	Write((LPTSTR)(LPCTSTR)S,S.GetLength());

	}

CString CStdioFileEx::ReadString() {
	CString S;	
	char myBuf[64];
	int i;

	do {
		i=Read(myBuf,1);
		if(i<1)
			break;
		S+=*myBuf;
		if(*myBuf=='\n')
			break;
		} while(1);

	return S;
	}

#endif

int CLogFile::ReIndex() {
	int i=0,n;
	CFile hTmpIndexFile;
	CStdioFile mF;

	if(!(mode & useIndex))
		return -1;

	if(mode & keepOpen) {
		Close();
		if(hIndexFile) 
			hIndexFile->Close();
		}

	getIndexFileName();

	hTmpIndexFile.Open(nomeFileNdx,CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive);

	n=0;
	hTmpIndexFile.Write(&n,sizeof(DWORD));

	if(mF.Open(nomeFile,CFile::modeRead | CFile::typeText | CFile::shareDenyNone)) {
		char myBuf[1024];

		i=1;

		while(mF.ReadString(myBuf,1000) > 0) {
			n=mF.GetPosition();
			hTmpIndexFile.Write(&n,sizeof(DWORD));
			}
		mF.Close();
		}

	hTmpIndexFile.Close();

	if(mode & keepOpen)
		Open();

	return i;
	}

CString CLogFile::getIndexFileName() {
	int i;

	i=nomeFile.Find('.');
	if(i>=0) {
		nomeFileNdx=nomeFile.Left(i+1)+"ndx";
		}
	else {
		nomeFileNdx=nomeFile+".ndx";
		}

	return nomeFileNdx;
	}

int CLogFile::RenameAndStore(int how) {
	int i;
	CString S,S1;

	if(mode & keepOpen) {
		Close();
		if(hIndexFile) 
			hIndexFile->Close();
		}

	S1=nomeFile;
	S1=S1.Left(S1.Find('.'));
	S1=S1.Mid(S1.ReverseFind('\\'));
	S=S1+CTime::GetCurrentTime().Format("_%Y_%m_%d.txt");
//	S=nomeFile+'\\'+S+".txt";

	i=1;

	TRY
	{
		Rename(nomeFile,S);
	}
	CATCH( CFileException, e )
	{
		i=0;

    #ifdef _DEBUG
        afxDump << "Impossibile rinominare File " << nomeFile << " , cause = "
            << e->m_cause << "\n";
    #endif
	}
	END_CATCH

	clearAll();
	

	if(mode & keepOpen)
		Open();

	return i;
	}





// ------------------------------------------------------------------------------------------------
CSourceFile::CSourceFile(LPCTSTR s) : CFile(s,CFile::modeRead /*| NON VA! e dà pure eccezione CFile::typeText */
																						| CFile::shareDenyWrite 
																						| FILE_FLAG_RANDOM_ACCESS /*CFile::osRandomAccess=0x10000000*/) {

	lineno=savedLineno=savedPositionIdx=0;
	}

char *CSourceFile::FNTrasfNome(char *A) {
  char *T,B[256];
  
  if(ANSI)
    return A;
  _tcscpy(B,A);
  if(ACORN) {
    T=strchr(B,'.');
    if(T) { 
      _tcscpy(A,T+1);
      _tcscat(A,".");
      strncat(A,B,T-B-1);
      return A;
      }
    else 
      return A;
    }
  if(GD) {
    T=strchr(A,'.');
    if(T) {   
      *T='_';
      return A;
      }
    else 
      return A;
    }      
                    
  return A;
  }

char *CSourceFile::AddExt(char *n, const char *x) {
  char *p;
  
  if(p=strchr(n,'.')) {
		_tcscpy(p+1,x);
		}
  else {
		_tcscat(n,".");
		_tcscat(n,x);
		}      
		
	return n;	
  }
  
int CSourceFile::get() {
	char ch;

rifo:
	if(Read(&ch,1) < 1)
		return EOF;
	if(ch=='\r')
		goto rifo;
	if(ch=='\n')
		lineno++;

	return ch;
	}

void CSourceFile::unget(char ch) {

	Seek(-1,CFile::current);
	if(ch=='\n') {
		lineno--;
		Seek(-1,CFile::current);		// riavvolgo anche CR...
		}
	}

void CSourceFile::SavePosition() {
	if(savedPositionIdx<10-1)
		savedPositionIdx++;
	else	// errore...
		;
	savedPosition[savedPositionIdx]=GetPosition();
	savedLineno=lineno;
	}
void CSourceFile::RestorePosition() {
	// non va... serve forse una specie di stack delle posizioni salvate, più d'una...
	if(savedPositionIdx>0)
		Seek(savedPosition[--savedPositionIdx],begin);
	else
		;		// errore...
	lineno=savedLineno;
	}
void CSourceFile::RestorePosition(uint32_t pos) {
	Seek(pos,begin);
	lineno=savedLineno;
	}

uint32_t CSourceFile::getLineFromPosition(long pos) {
	uint32_t oldpos=GetPosition();
	uint16_t line=1;
	char ch;
	
	if(pos==-1)
		pos=GetPosition();
	Seek(0,CFile::begin);
	do {
		if(Read(&ch,1) < 1)
			break;
		if(ch=='\n')
			line++;
		} while(GetPosition()<(uint32_t)pos);

	Seek(oldpos,CFile::begin);
	return line;
	}

int CSourceFile::getHex() {
	int ch;
	int n=0;

	while((ch=get()) != EOF) {
		if(isdigit(ch)) {
			n*=16;
			n+=ch-'0';
			}
		else {
			ch = toupper(ch);
			if(ch>='A' && ch<='F') {
				n*=16;
				n+=ch+10-'A';
				}
			else {
				unget(ch);
				break;
				}
			}
		}

	return n;
	}

int CSourceFile::getOct() {
  int t;
	int n=0;
	int ch;
  
	while((ch=get()) != EOF) {
    t=ch-48;
    if((t>=0) && (t<8)) {
      n=n*8+t;
      }
    else {
			unget(ch);
			break;
      }
    }
  return n;
  }
      
int CSourceFile::getInt() {
	int ch;
	int n=0;

	while((ch=get()) != EOF) {
		n*=10;
		if(isdigit(ch)) {
			n+=ch-'0';
			}
		else
			break;
		}

	return n;
	}

/*int CSourceFile::scanf(const TCHAR *s,...) {
	TCHAR myBuf[2048],myFmt[16];
	register int i,j,ch,k;
	int *n;
	double *d;
	CTime *myT;
	TCHAR *p,*p_myBuf;
  va_list vl;
	CString S;

	ReadString(myBuf);

	va_start(vl,s);
	p_myBuf=myBuf;
	myBuf[j]=0;
	i=0;
	while(ch=s[i++]) {
		if(ch == '%') {
			if(s[i] == '%') {
				i++;
				goto no_var;
				}
			myFmt[0]=ch;
			k=1;
			while((ch=s[i++]) && !isalpha(ch))			// salvo dettagli...
				myFmt[k++]=ch;
			myFmt[k++]=ch;			// ...copio effettivo format-type
			myFmt[k]=0;
			switch(myFmt[k-1]) {
				case 'd':
				case 'D':
					n=va_arg(vl,int*);
					goto subLeggi0;
					break;
				case 'p':
				case 'P':
					myBuf[j++]='0';
					myBuf[j++]='x';
				case 'u':
				case 'U':
				case 'x':
				case 'X':
				case 'o':		//ottale
				case 'O':
					n=va_arg(vl,int*);
subLeggi0:
					sscanf(myBuf1,myFmt,n);
subLeggi:
					_tcscpy(myBuf+j,myBuf1);
subLeggi2:
					j+=_tcslen(myBuf1);
					break;
				case 'f':
				case 'g':
					d=va_arg(vl,double*);
					sprintf(myBuf1,myFmt,d);
					goto subLeggi;
					break;
				case 's':
					p=va_arg(vl,TCHAR *);
					_tcsncpy(myBuf+j,p,2000-j);
					j+=min(_tcslen(p),2000-j);
					myBuf[j]=0;
					break;
				case 'c':
					n=va_arg(vl,char*);
					myBuf[j++]=n;
					break;
				case 't':
					myT=va_arg(vl,CTime*);
					S.Format(_T("%02u/%02u/%02u %02u:%02u:%02u"),
						myT.GetDay(),
						myT.GetMonth(),
						myT.GetYear(),
						myT.GetHour(),
						myT.GetMinute(),
						myT.GetSecond());
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
				case 'T':
					myT=va_arg(vl,CTime*);
					S=myT.Format(_T("%02u %b %04u %02u:%02u:%02u"));
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
				default:
					break;
				}
			}
		else {
			if(ch == '\n')
				myBuf[j++]=13;

no_var:
			myBuf[j++]=ch;
			}
		if(j>2000)
			break;
		}

	myBuf[j]=0;

	i=0;

	va_end(vl);
	return i;
	}*/



// ------------------------------------------------------------------------------------------------
COutputFile::COutputFile(LPCTSTR s) : CStdioFile(s,CFile::modeCreate | CFile::modeReadWrite /*CFile::modeWrite*/),
	totLines(0) { 

	 }
COutputFile::COutputFile(FILE *f) : CStdioFile(f),totLines(0) { 
	}

/*COutputFile::COutputFile() : totLines(0) {		// così non serve... vedere come fare, magari derivando da CFile
	mF=new CMemFile();
	}*/
/* non è base class COutputFile::COutputFile() : CMemFile(),totLines(0) {
	}
	*/

int COutputFile::get() {
	char ch;

rifo:
	if(Read(&ch,1) < 1)
		return EOF;
	if(ch=='\r')
		goto rifo;

	return ch;
	}

void COutputFile::put(char ch) {

	if(!totLines)			// :) vabbe' finezza
		totLines=1;
	
	Write(&ch,1);
	if(ch=='\n')
		totLines++;
	}

int COutputFile::printf(const TCHAR *s,...) {
	TCHAR myBuf[2048],myBuf1[2048],myFmt[16];
	register int i,j,ch,k;
	int n,pad;
	double d;
	CTime myT;
	TCHAR *p,*p_myBuf;
  va_list vl;
	CString S;
	char padch=' ';

	va_start(vl,s);
	p_myBuf=myBuf;
	j=0;
	myBuf[j]=0;
	i=0;
	while(ch=s[i++]) {
		pad=0;
		if(ch == '%') {
			if(s[i] == '%') {
				i++;
				goto no_var;
				}
			myFmt[0]=ch;
			k=1;
			while((ch=s[i++]) && !isalpha(ch))			// salvo dettagli...
				myFmt[k++]=ch;
			myFmt[k++]=ch;			// ...copio effettivo format-type
			myFmt[k]=0;
			if(myFmt[1]=='-') {
				if(isdigit(myFmt[2])) {
					pad=-atoi(&myFmt[2]);
					}
				// altrimenti...
				}
			else if(myFmt[1]=='+') {
				// segno ...
				}
			if(isdigit(myFmt[1])) {
				pad=atoi(&myFmt[1]);
				}
			switch(myFmt[k-1]) {
				case 'l':
				case 'L':
//					k++;
					// bah me ne fotto e proseguo!
					myFmt[k-1]=s[i++];
				case 'd':
				case 'D':
					n=va_arg(vl,int);
					goto subCopia0;
					break;
				case 'p':
				case 'P':
					myBuf[j++]='0';
					myBuf[j++]='x';
				case 'u':
				case 'U':
				case 'x':
				case 'X':
				case 'o':		//ottale
				case 'O':
					n=va_arg(vl,unsigned int);
subCopia0:
					sprintf(myBuf1,myFmt,n);
subCopia:
					if(pad) {
						pad-=_tcslen(myBuf1);
						if(pad<0)
							pad=0;
						while(pad--)
							myBuf[j++]=padch;		//put(' ');
						}
					_tcscpy(myBuf+j,myBuf1);
subCopia2:
					j+=_tcslen(myBuf1);
					break;
				case 'f':
				case 'g':
					d=va_arg(vl,double);
					sprintf(myBuf1,myFmt,d);
					goto subCopia;
					break;
				case 's':
					p=va_arg(vl,TCHAR *);
					if(pad) {
						pad-=_tcslen(p);
						if(pad<0)
							pad=0;
						while(pad--)
							myBuf[j++]=padch;		//put(' ');
						}
					_tcsncpy(myBuf+j,p,2000-j);
					j+=min(_tcslen(p),2000-j);
					myBuf[j]=0;
					break;
				case 'c':
					n=va_arg(vl,char);
					myBuf[j++]=n;
					break;
				case 't':
					myT=va_arg(vl,CTime);
					S.Format(_T("%02u/%02u/%02u %02u:%02u:%02u"),
						myT.GetDay(),
						myT.GetMonth(),
						myT.GetYear(),
						myT.GetHour(),
						myT.GetMinute(),
						myT.GetSecond());
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
				case 'T':
					myT=va_arg(vl,CTime);
					S=myT.Format(_T("%02u %b %04u %02u:%02u:%02u"));
					_tcscpy(myBuf+j,(LPCTSTR)S);
					j+=S.GetLength();
					break;
				default:
					break;
				}
			}
		else {

no_var:
			myBuf[j++]=ch;
			}
		if(j>2000)
			break;
		}

	myBuf[j]=0;

	i=0;
	write(myBuf);

	va_end(vl);
	return i;
	}

void COutputFile::print(const TCHAR *s) {

	write(s);
	}

void COutputFile::println(const TCHAR *fmt,...) {
  va_list argptr;

//  va_start(argptr, fmt);
//  print(fmt, argptr);		// non va... boh
//  va_end(argptr);
	write(fmt);
	putcr();
	totLines++;
	}

void COutputFile::write(const TCHAR *s) {
	
	while(*s) {
		put(*s++);
		}
	}




int CPlusMinus::Ottimizza(struct LINE *r) {
	return 0;
	}
uint8_t CPlusMinus::FNIs1Bit(uint32_t t) {
#if 1
  register uint32_t i,j=0;
	uint8_t k,i1;
  
  for(i=0x00000001,i1=0; i; i <<= 1,i1++) {
    if(t & i) {
      j++;
      k=i1;
      }
    }
//    printf("1 bit: %d %d\n\a",j,k);
  if(j==1)
    return k+1;
  else
    return 0;   
#else

	// gemini 3/9/26
	// 1. Verifica se t ha ESATTAMENTE un bit a 1 (e t non è zero)
  if (t == 0 || (t & (t - 1)) != 0)
    return 0; // Più di un bit (o zero bit)

    // 2. Calcola l'indice del bit + 1
/*#if defined(__GNUC__) || defined(__clang__)
  return __builtin_ctz(t) + 1; // Count Trailing Zeros hardware
#elif defined(_MSC_VER)
  unsigned long index;
  _BitScanForward(&index, t);
  return (uint8_t)(index + 1);
#else*/
  // Fallback portabile in C puro con algoritmo De Bruijn (senza cicli)
  static const uint8_t MultiplyDeBruijnBitPosition[32] = {
    1, 2, 29, 3, 30, 15, 25, 4, 31, 23, 12, 16, 26, 18, 5, 9,
    32, 28, 14, 24, 22, 11, 17, 27, 27, 13, 21, 10, 20, 19, 8, 7
	  };
  return MultiplyDeBruijnBitPosition[(uint32_t)(t * 0x077CB531U) >> 27];

	// OVVERO: 2. Scansione bit via hardware x86 nativa (386+)
/*  uint32_t index;
  __asm {
    bsf eax, t       ; Cerca l'indice del primo bit a 1 da destra
    mov index, eax   ; Salva l'indice (0..31)
		}
  return (uint8_t)(index + 1);*/
//#endif
#endif

  }

uint8_t CPlusMinus::FNIsPower2(uint32_t t) {
  register uint32_t i,k,j=0;
	uint8_t i1;
  
  for(i=0x80000000,i1=31,k=0x7fffffff; i; i >>= 1,i1--) {
    if(t & i) {
	    if(!(t & k))
				return i1;
			else
				return 0;
      }
		k &= k >> 1;
    }
   return 0;   
  }

/* gemini propone 9/26
uint8_t CPlusMinus::FNIsPower2(uint32_t t) {
  // Verifica se ha esattamente un solo bit a 1
  if (t == 0 || (t & (t - 1)) != 0) 
      return 0;

  // Calcola l'esponente (posizione del bit)
  // Su GCC/Clang: __builtin_ctz(t)
  // Su MSVC: _BitScanForward(&index, t)
  // O algoritmo classico a conteggio:
  uint8_t exp = 0;
  while (t >>= 1) 
		exp++;
  return exp; 
}*/

O_SIZE CPlusMinus::getPtrSize(O_TYPE t) {
	O_SIZE s;

	s=PTR_SIZE;

	return s;
  }
int CPlusMinus::PROCReadD0(char *outbuf,struct VARS *V, O_TYPE T, O_SIZE S, int16_t cond, int ofs, bool asPtr) {   // m=0 se norm, 1 se condiz.


	//PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"ReadDO");
	if(V)
		_tcscat(outbuf,V->name);
//	PROCOper(LINE_TYPE_ISTRUZIONE,V);
	return 1;
	}
int CPlusMinus::PROCStoreD0(char *outbuf,const char *op,struct VARS *V, int8_t RQ, struct VARS *RVar, union STR_LONG *RCost, uint16_t ofs2) {

	//PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"StoreDO");
	if(V)
		_tcscat(outbuf,V->name);
	if(op)
		_tcscat(outbuf,op);
//	PROCOper(LINE_TYPE_ISTRUZIONE,V);
	return 1;
	}
int CPlusMinus::PROCGetAdd(int8_t VQ, struct VARS *V, int ofs, bool asPtr) {

//	PROCOper(LINE_TYPE_COMMENTO | LINE_TYPE_ISTRUZIONE,"GetAdd");
	//if(V)
		//_tcscat(outbuf,V->name);
//	PROCOper(LINE_TYPE_ISTRUZIONE,V);
	return 1;
	}
