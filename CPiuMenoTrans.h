//#define I8086 0
//#define Z80   1
//#define ARCHI 0
//#define I8051 0
//#define MICROCHIP 1
// MESSE IN PROGETTO!!

#define WM_ADDTEXT (WM_USER+1)		// v.openC
#define WM_CLSWINDOW (WM_USER+2)

#include <stdint.h>
       
#define ANSI  TRUE
#define ACORN FALSE
#define GD    FALSE   // REM CONVENZIONI SUI NOMIFILE

//#define TRUE 1
//#define FALSE 0

#define INT_SIZE 4
#define STACK_ITEM_SIZE 4
#define PTR_SIZE INT_SIZE

#define __VER__ MAKEWORD(0,1)

enum {
	PWM_COMMWRITE =  WM_USER+1,
	WM_UPDATE_PANE,
	};


#define MAX_NAME_LEN 31+8		// con 63 si schianta tutto!! sistemare 2026
#define MAX_DIM 4
#define MAX_TIPI 50
#define MAX_BLOCCHI 20

struct ERRORE {
  uint16_t t;
	uint8_t l;
  const char *s;
  };

union SUB_OP_DEF {
  char label[MAX_NAME_LEN+1];
  int n;
//  struct VARS *v;
  };
    
typedef uint32_t O_TYPE;
typedef uint16_t O_SIZE;
typedef uint32_t O_DIM[MAX_DIM];		// max 5 dim!

#define SIZE_NULL 0		// METTERE VALORE SPECIALE!
#define TYPE_NULL 0		// METTERE VALORE SPECIALE!?

  
enum LINE_TYPE {
	LINE_TYPE_NULLA=0,
	LINE_TYPE_LABEL,
	LINE_TYPE_DICHIARAZIONE,
  LINE_TYPE_CONST_DEF,
	LINE_TYPE_DATA,		// per BSS
	LINE_TYPE_DATA_DEF,		// altre
	LINE_TYPE_DATA_DEF_CONT,
	LINE_TYPE_LABEL_CON_ISTRUZIONE,
	LINE_TYPE_FUNCTION_DECLARATION,
	LINE_TYPE_FUNCTION,
	LINE_TYPE_JUMP=16,
	LINE_TYPE_JUMPGOTO,
	LINE_TYPE_JUMPC,
	LINE_TYPE_CALL,
	LINE_TYPE_ISTRUZIONE=24,
	LINE_TYPE_READ=32,			// usare, per differenziare...
	LINE_TYPE_WRITE=32,
	LINE_TYPE_PUSH=32,
	LINE_TYPE_POP=32,
	LINE_TYPE_ISTRUZIONE_CONT,
	LINE_TYPE_COMMENTO=64,	// qua va in OR!
	LINE_TYPE_OTTIMIZZATA=128,
	};

struct LINE {
  struct LINE *next;
  struct LINE *prev;
  enum LINE_TYPE type;             // 0 commento, 1 label, 2 data def, 3 label con istr., 8 jump, 9 call, 16 istr.
  char s1[128];		// usato anche in blocchi _asm per tutta la riga... attenzione
  char s2[128];		// usato anche in blocchi _asm per tutta la riga... attenzione
  char rem[128];
	uint8_t flag;		// se volatile, se asm (per ottimizzatore
  };
  
struct LINE_DEF {
	uint16_t used;
	struct VARS *vars;
  struct LINE_DEF *next;
  struct LINE_DEF *prev;
  char name[MAX_NAME_LEN+1];
  char *text;
  };

enum {
	OPTIMIZE_JUMP=1,
	OPTIMIZE_SUBEXPR=2,		// usato per propagazione registri ecc, andrebbero separati
	OPTIMIZE_INLINECALLS=4,
	OPTIMIZE_CONST=16,
	OPTIMIZE_SIZE=0x100,
	OPTIMIZE_SPEED=0x200,
	};
enum {
	TIPO_SPECIALE=1,		// output speciale, tipo Easy68K
	TIPO_EMBEDDED=2,		// copia Initialized in Const, affinché possano essere copiati in RAM alla partenza
	};

union STR_LONG {
  char s[128];
  long l;
	uint64_t l64;		// VA GESTITO IN MOLTI POSTI! e serve ulltoa(
  };

// VARTYPE% BITS:
// 0-3 = PTR,
// 7=corpo FUNZ o VAR, 8=FUNZ., 9=FUNZ usata,
// 10=ARRAY, 11=STRUCT, 12=UNION, 13=FLOAT,
// 16=ENUM,
// 31=UNSIGNED 
// 30=ROM (su Microchip)
// VARSIZE% DIMENSIONI (PER LE FUNZIONI E' LA DIM. DI QUELLO CHE RITorNANO)
enum VAR_CLASSES {			// v. anche class CPlusMinus
	CLASSE_EXTERN=0,
	CLASSE_GLOBAL=1,
	CLASSE_STATIC=2,
	CLASSE_AUTO=3,
	CLASSE_REGISTER=4,
	CLASSE_MEMBER=5,		// per membri di una classe
  CLASSE_MEMBER_STATIC = 6, // static di classe (niente 'this')
  CLASSE_MEMBER_VIRTUAL = 7, // virtual di classe (usa vtable)

	CLASSE_INTERRUPT=0x10,			// nello stesso ordine, shiftati di 4 rispetto ai MODIFIERS
	CLASSE_PASCAL=0x20,
	// CLASSE_C
	CLASSE_INLINE=0x80,
	CLASSE_FASTCALL=0x100,
	CLASSE_VIRTUAL=0x200,
	};
enum VAR_MODIFIERS {		// v. anche class CPlusMinus
// 1 interrupt, 2 pascal, 4 C
	FUNC_MODIF_INTERRUPT=1,
	FUNC_MODIF_PASCAL=2,
	FUNC_MODIF_C=4,
	FUNC_MODIF_INLINE=8,
	FUNC_MODIF_FASTCALL=16,
	FUNC_MODIF_VIRTUAL=32,
	FUNC_MODIF_BUILTIN=64
	};
enum VAR_ATTRIBUTES {		// __attribute__ 
	FUNC_ATTRIB_NORETURN=1,
	FUNC_ATTRIB_NAKED=2,
	FUNC_ATTRIB_WEAK=4,			// in teoria anche variabili, ma raro
	VAR_ATTRIB_FIXED=0x100,
	VAR_ATTRIB_UNUSED=0x200,
	VAR_ATTRIB_PACKED=0x400,
	// poi volendo ci sono ripetuti i vari interrupt, const e altro
	};
enum VAR_TYPES {		// v. anche class CPlusMinus
	VARTYPE_NOTYPE=-1,
	VARTYPE_PLAIN_INT=0,
	VARTYPE_POINTER=1,			//0..15
	VARTYPE_2POINTER=2,			//**
	VARTYPE_IS_POINTER=0xf,			//0..15
	VARTYPE_IS_2POINTER=0xe,			//
	VARTYPE_NOT_A_POINTER=(uint32_t)(~(VARTYPE_IS_POINTER)),			// mask

/* --- FLAG E QUALIFICATORI (da 0x10 in poi) --- */
  VARTYPE_REFERENCE       = 0x10,           // & (L-value reference C++)
  VARTYPE_RVALUE_REF      = 0x20,           // && (R-value reference C++, opzionale)
		
	VARTYPE_FUNC_POINTER=0x40,
	VARTYPE_FUNC_BODY=0x80,
	VARTYPE_INITIALIZED=0x80,		// dovrebbe andare bene usare lo stesso :) usato anche per parametri default
	VARTYPE_FUNC=0x100,
	VARTYPE_FUNC_USED=0x200,

	VARTYPE_ARRAY=0x400,		// questo IMPLICA 1 ossia POINTER
	VARTYPE_STRUCT=0x800,
	VARTYPE_UNION=0x1000,
	VARTYPE_FLOAT=0x2000,		// per double metto size=8 opp. 4
	VARTYPE_BITFIELD=0x4000,
	VARTYPE_CLASS=0x8000,
	VARTYPE_ENUM=0x10000,

	VARTYPE_VOLATILE=0x8000000L,
	VARTYPE_NOIMMEDIATE=0x8000000L,		// usato come flag per builtin/inline! indica che non accetta immediato ma solo var/registro

	VARTYPE_FAR=0x10000000L,
	VARTYPE_SIGNED=0x00000000L,
	VARTYPE_UNSIGNED=0x80000000L,
	VARTYPE_CONST=0x40000000
	};
enum VAR_VISIBILITY {
	VIS_PUBLIC=0,
	VIS_PROTECTED,
	VIS_PRIVATE,
	};
struct VARS {
  char label[128];		// qua mettiamo il nome "puro"
  char name[MAX_NAME_LEN+1];		//il nome mangled
  enum VAR_CLASSES classe;
	enum VAR_VISIBILITY visibility;
// 0 extern, 1 global, 2 static, 3 auto, 4 register; 
  uint8_t modif;
// 1 interrupt, 2 pascal, 4 C ecc
  O_TYPE type;
  O_SIZE size;
  uint8_t block;
	short int blockId;
  union {
	  struct VARS *func;
		int32_t value;
		} func;
  union {
		char *ptr;
		int *ptr32;
		struct {
			int16_t ofs;
			int16_t flag;
			};
		} parm;
	char *decor;
  struct TAGS *isInTag;         // se <>0, la var. è un membro della struct tag
  struct TAGS *hasTag;      // questo indica il tag di questa struct
  struct TAGS *hasBase;      // se è presente in una class base (viene assegnato da CercaVar ogni volta
  O_DIM dim;							// dim TOTALE dell'array o aggr
  uint8_t attrib;
	uint8_t inlineCnt;
	struct LINE *definition;		// dove è definita (usato da funzioni inlined)
  struct VARS *members;		// var locali di una funzione
  struct VARS *prev;
  struct VARS *next;
  };

struct VARS2 {		// usata da enum
  uint32_t value;
  O_TYPE type;
  O_SIZE size;
  struct VARS *func;
  uint8_t block;
  struct VARS2 *next;
  };

struct CONS {
  char label[MAX_NAME_LEN+1];
  char name[128];
  struct CONS *next;
  };  
//#pragma message USARE vars2 ANCHE PER STRUCT *********************
struct TAGS {
  char label[MAX_NAME_LEN+1];
	uint8_t type;		// 2=class, 1=struct, 0=union
  struct VARS *member;		
  struct VARS *func;
  uint8_t block;
	short int blockId;
	struct TAGS *parent;		// per ereditarietà
	struct TAGS *friends;		// in effetti dice che possono esserci anche funzioni singole o altro
  struct TAGS *next;
  };  
   
struct ENUMS {
  char name[MAX_NAME_LEN+1];
  struct VARS2 var;
  struct ENUMS *next;
  };  
   
struct OPERAND {
//	enum VQ_ELEM V;
	int8_t Q;
	O_SIZE size;
// verificare...	enum ARITM_ELEM T;
	O_TYPE type;
	struct VARS *var;
	union STR_LONG *cost;
	struct TAGS *tag; 
//	uint8_t tipo_D0;			// flag usato per le operazioni indirette su puntatori		BOH 2026 :)
	O_DIM dim;
	uint8_t flag;					// usato ad es. per calcolare pos. array
	};
  
struct OPERANDO {
  char *s;
  /*enum OPERANDI*/ int8_t p;
  };

struct TIPI {
  char s[MAX_NAME_LEN+1];
  O_SIZE size;
  O_TYPE type;
  struct TAGS *tag;
  O_DIM dim;
  };

struct BLOCK_PTR {
//  struct LINE *TX;   // CONTIENE I PUNTATORI PENDENTI LastOut AL FILE OUT
  char T[32];    // CONTIENE I NOMI DEI FINE-BLOCCHI, # SE DO, & SE SWITCH,% SE if
  char C[32];   // CONTIENE LE LABEL PER continue
  char B[32];   // CONTIENE LE LABEL PER break
	int id;				// usato per variabili locali in blocchi paralleli
	int AutoOff;	// per variabili locali dentro blocco
  char *parm;
	int8_t flag;		// usato per segnalare cose, tipo "default" già uscito in switch(
  };
   


class CPiuMenoDoc;
class CLogFile;

/////////////////////////////////////////////////////////////////////////////



class CSourceFile : public CFile {
public:
	CSourceFile(LPCTSTR);
  static char *FNTrasfNome(char *);
	static char *AddExt(char *n, const char *x);
	int get();
	char *FNLO(char *);
	char *FNLA(char *);
	bool Eof() { return GetPosition() >= GetLength(); }
	void unget(char);
// fare?? v. hex ecc	int scanf(const TCHAR *s,...);
	int getInt();
	int getHex();
	int getOct();
	void SavePosition();
	void RestorePosition();
	void RestorePosition(uint32_t);
	uint32_t getLineFromPosition(long pos=-1);

public:
	uint32_t savedPosition[10];
	uint16_t lineno;			// 65535 :)

private:
	uint16_t savedLineno;
	uint8_t savedPositionIdx;

	};

class COutputFile : public CStdioFile {
public:
	COutputFile(LPCTSTR s);
	COutputFile(FILE *f);
//	COutputFile();
	int printf(const TCHAR *s,...);
	void print(const TCHAR *s);		// usata per NON espandere i %d ecc, tipo stringhe literal
//	void println(const TCHAR *s);
	void println(const TCHAR *s,...);
	void put(char ch);
	void putcr() { put('\n'); }
	void write(const TCHAR *);
	int get();
	bool eof() { return GetPosition() >= GetLength(); }
	int getTotalLines() { return totLines; }

private:
	uint32_t totLines;
//	CMemFile *mF;

	};

class CCPreProcessor;

class CPlusMinus {
public:
/*	enum VAR_CLASSES {
		CLASSE_EXTERN=0,
		CLASSE_GLOBAL=1,
		CLASSE_STATIC=2,
		CLASSE_AUTO=3,
		CLASSE_REGISTER=4,
		CLASSE_INTERRUPT=0x10,
		CLASSE_PASCAL=0x20,
		CLASSE_INLINE=0x40
		};*/
	enum {
		FUNC_MODIF_INTERRUPT=1,
		FUNC_MODIF_PASCAL=2,
		FUNC_MODIF_C=4,
		FUNC_MODIF_INLINE=8,
		FUNC_MODIF_FASTCALL=16,
		FUNC_MODIF_VIRTUAL=32
		};

	enum VALUES {
		VALUE_IS_EXPR=0,			// boh... non ho ancora capito bene! forse proprio "altro" ossia espressione! tipo op. ? :
		VALUE_IS_EXPR_FUNC=1,
		VALUE_IS_PTR=2,			// vale SOLO per puntatori, ossia 32bit su GD24032 e v. 68000, 16 su altri 
		VALUE_IS_D0=3,
		VALUE_IS_VARIABILE=4,
		VALUE_IS_COSTANTE=8,
		VALUE_IS_COSTANTEPLUS=8 + 1,

		// -5 è usato in FNGetConst come marker!

//                V->Q |= 0x20;            // segnala ! condizionale  VERIFICARE! era anche usato per indicare condizione di tipo unsigned...
//ma è usato anche come "condizione"...		VALUE_CONDITION_UNSIGNED=0x20,				// potrebbe non servire più, 2025 con le nuove condizioni per unsigned, ma è ancora usata in giro!
//		VALUE_CONDITION_UNSIGNED=0x10,		// non più usata 2025 cmq (creati valori espliciti per unsigned
		VALUE_IS_CONDITION=0x20,					// è una condizione < > == ecc
		VALUE_HAS_CONDITION=0x40,
		VALUE_IS_CONDITION_VALUE=0x80,		// significa che usiamo un'expr come vero o falso
		VALUE_CONDITION_UP=0x0100,			// usata come mask... non so bene perché
		VALUE_CONDITION_MASK=0x00ff			// e viceversa
		};
	enum OPERANDO_CONDIZIONALE {
		CONDIZ_MINORE=VALUE_IS_CONDITION | 0,
		CONDIZ_MAGGIORE_UGUALE=VALUE_IS_CONDITION | 1,
		CONDIZ_MINORE_UGUALE=VALUE_IS_CONDITION | 2,
		CONDIZ_MAGGIORE=VALUE_IS_CONDITION | 3,
		CONDIZ_UGUALE=VALUE_IS_CONDITION | 4,
		CONDIZ_DIVERSO=VALUE_IS_CONDITION | 5,
		CONDIZ_MINORE_UNSIGNED=VALUE_IS_CONDITION | 6,
		CONDIZ_MAGGIORE_UGUALE_UNSIGNED=VALUE_IS_CONDITION | 7,
		CONDIZ_MINORE_UGUALE_UNSIGNED=VALUE_IS_CONDITION | 8,
		CONDIZ_MAGGIORE_UNSIGNED=VALUE_IS_CONDITION | 9,
		};
	enum OPERAND_MODES {
		// c'era MODE_IS_OTHER: pre 2026 :)
		MODE_IS_VARIABLE=0,
		MODE_IS_CONSTANT1=1,
		MODE_IS_CONSTANT2=2,
		};
	enum ARITM_OP {
		ARITM_IS_EOL=0,
		ARITM_IS_COSTANTE=1,
		ARITM_IS_VARIABILE=2,
		ARITM_IS_OPERANDO=3,
		ARITM_IS_UNKNOWN=-1,
		};
	enum LINE_FLAGS {
		LINE_IS_NORMAL=0,
		LINE_IS_ASSEMBLER=1,
		LINE_IS_VOLATILE=2,
		};

	friend class CCPreProcessor;

protected:
// Attributes
	CWnd *myOutput;
	CLogFile *myLog;
	CCPreProcessor *m_CPre;

public:
	CPlusMinus(const CWnd *p);
	~CPlusMinus();


protected:
	int8_t bExit;
	int8_t c_mode;		// se siamo in un blocco extern "C", a che livello InBlock
	char buffer[128];
  COutputFile *FPre;
	CSourceFile *FIn;
	COutputFile *FO1,*FO2,*FO3,*FO4,*FO5;
	COutputFile *FObj,*FCod;
	COutputFile *FLst,*FErr;
	static const struct ERRORE Errs[];
	static char *dtor,*ctor,*vptr,*the_this;
	static char *to_mangle;
	static char *ptr_to_base,*ptr_to_base2;
	static char *struct_type;
	static char *main_name;
	static char *malloc_name,*free_name;
	int8_t Warning;
//  struct LINE *RootOut,*LastOut;
	struct VARS *Var;
	struct VARS *LVars;   // root, used, last
	struct VARS *CurrFunc,*CurrFuncGotos;
	struct CONS *Con;
	struct CONS *LCons;
  struct ENUMS *LEnums;
	struct ENUMS *Enums;
	struct TAGS *StrTag;
	struct TAGS *LTag;
	//struct VARS *TAG;           
	//int SX;
	uint8_t InBlock; 			// LIVELLO DI BLOCCHI
	enum VAR_VISIBILITY DefaultVisibility;
	uint16_t MaxTypes;
	int8_t UseIRQ,UseFloat;
	char *RootIn;
	uint8_t Brack;				// usati da FNRev
	int8_t isRValue,isPtrUsed,inCast;	// tutti questi potrebbero andare in  struct OPERAND
	bool FuncReturnedValue;
	struct LINE *GlblOut;	//

	char NFS[256],OUS[256];

	int __STDC__;
	int LABEL;
	char __file__[256],__name__[256];
	int __line__;
	char __date__[11];
	char __time__[11];
	int Declaring,FuncCalled,SaveFP,ASM,AutoOff;
	uint8_t debug,verbose;
	uint8_t PreProcOnly;          // PREPROCESSA SOLO SU stdout  -E
	uint8_t PreProcCommenti;			// inserisce commenti nel .i e quindi in output
	bool CheckStack;            // INSERISCE LO STACK PROBE    -Gs
	uint8_t OutSource;             // INSERISCE LE RIGHE C NELL'OUTPUT  -Fc
	uint8_t OutAsm;             // crea un file asm senza righe C (non usato) -Fa
	uint8_t OutList;               // CREA FILE LISTING          -Fl
	uint8_t TipoOut;						// varia tipo ASM file generato (v.68000->Easy68K
	uint8_t OptimizeExpr;         // RICORDA le SUB-Expr        -Og
	uint8_t SynCheckOnly;         // SOLO SYNTAX CHECKING        -Zs
	uint8_t NoMacro;              // DISABILITA MACRO PREDEF.    -u
	uint8_t StorageDefault;				// storage class di default (finire)
	uint8_t StructPacking;				// packing delle struct
	uint16_t Optimize;
	bool UsesMalloc;
	static struct TIPI Types[MAX_TIPI];
	static struct OPERANDO Op[];
	uint16_t numErrors,numWarnings;
	bool panicMode;		// per gestire valanga di errori

// Operations
public:
	int CompilaCpp(int,char **);

// Compiler
	char *AddExt(char *, char *);
	int PROCBlock();
	int PROCIsDecl();
	struct VARS *PROCDclVar(char *,enum VAR_CLASSES, uint8_t, O_TYPE type, O_SIZE size, 
		struct TAGS *, O_DIM dim, uint32_t attrib, bool isparm, const char *decor, const char *mangle);
	int subAsm(char *);
	int FNIsStmt();
	char *FNGetLabel(char *,uint8_t,int8_t m=0);
	int PROCGenCondBranch(const char *, int T, int8_t *VQ, O_SIZE );
	int FNGetCondString(uint8_t , uint8_t);
	int PROCAssignCond(int8_t *VQ, O_TYPE *type, O_SIZE *size, char *);
	int PROCReturn();
	long FNGetConst(char *,bool);
	int PROCLoops(const char *, const char *, const char *);
	int FNRegFree();
	int8_t CmpDecorated(const char *,const char *);
	struct VARS *FNCercaVar(const char *, bool,struct TAGS **base=NULL);
	struct VARS *FNCercaVar(struct TAGS *,const char *,struct TAGS **base=NULL);
	struct TAGS *FNCercaAggr(const char *, bool);
	struct VARS *FNCercaFunz(struct TAGS *,const char *,const char *,struct TAGS **base=NULL);
	struct VARS *FNCercaCtor(struct TAGS *,const char *,bool is_ctor,struct TAGS **base=NULL);
	bool FNHasVar(struct TAGS *);			// ossia "se è definita"
	bool FNHasVirtual(struct TAGS *);			// ossia "se ha almeno una funzione virtual"
	struct VARS *PROCAllocVar(const char *name, O_TYPE type, enum VAR_CLASSES, uint8_t modif, O_SIZE size, struct TAGS *, O_DIM dim);
	struct VARS *PROCAllocFunzProto(const char *name, O_TYPE type, O_SIZE size);
	struct VARS *PROCAllocGoto(const char *label);
	struct VARS *FNCercaGoto(const char *);
  struct ENUMS *FNCercaEnum(const char *,const char *,bool);
	int PROCCast(O_TYPE, O_SIZE, O_TYPE*, O_SIZE*, int8_t);
	int PROCReadD0(char *outbuf,struct VARS *, O_TYPE type, O_SIZE size, int16_t cond, int ofs, bool asPtr);
	int PROCStoreD0(char *outbuf,const char *op,struct VARS *, int8_t VQ, struct VARS *, union STR_LONG *, uint16_t ofs);
	int PROCGetAdd(int8_t VQ, struct VARS *, int ofs, bool asPtr);
	int PROCUsaFun(char *outbuf,struct VARS *,uint8_t isMember=0,const char *n=NULL);
	struct CONS *FNAllocCost(const char *, uint8_t, O_TYPE type=0);
	struct ENUMS *FNAllocEnum(const char *tag, const char *name, uint32_t value, O_SIZE Size);
	int PROCInit();
	int subOfsD0(struct VARS *, int, int, int);
	
	void subEvEx(char *,uint8_t, int16_t *cond, char *, struct OPERAND *);
	uint16_t FNEvalExpr(char *,uint8_t, char *);
 	int FNEvalECast(char *,char *, O_TYPE *type, O_SIZE *size);
	int FNEvalCond(char *,char *, const char *, uint16_t cond);
	void skipExpr(uint8_t Pty,char delim);

	int collectParmList(char *);
	int collectTypeList(char *);
	char *getDecor(char *,O_TYPE, O_SIZE, O_DIM, struct TAGS *);
	int subAcquisisciParm(int *ptr32,struct VARS *V,bool is_member,bool doDeclare);


  int8_t FNRev(char *,int8_t Pty,int16_t *cond,char *,struct OPERAND *);
  char *ConRecEval(char *, uint8_t pty, long *);
  long EVAL(char *);

	int subShift(uint8_t, int16_t cond, int mode, int8_t VQ, struct VARS *, O_TYPE type, O_SIZE size, O_TYPE, union STR_LONG *, 
		union STR_LONG *, struct OP_DEF *, bool bAutoAssign);
	int subAdd(bool, int16_t cond, int mode, int8_t VQ, struct VARS *, O_TYPE * type1, O_SIZE * size1, int8_t RQ, O_TYPE type2, O_SIZE size2,
		union STR_LONG *, union STR_LONG *, struct OP_DEF *,bool bAutoAssign);
	int subMul(char, int16_t cond, int mode, int8_t VQ, struct VARS *, O_TYPE type1, O_SIZE size1, int8_t RQ, O_TYPE type2, O_SIZE size2, 
		union STR_LONG *, union STR_LONG *, struct OP_DEF *, bool bAutoAssign);
	uint8_t FNIs1Bit(uint32_t);
	uint8_t FNIsPower2(uint32_t);
	O_SIZE getPtrSize(O_TYPE t);
	int subAOX(char, int16_t cond, int mode, int8_t VQ, struct VARS *, O_TYPE type1, O_SIZE size1, int8_t RQ, 
		O_TYPE type2, O_SIZE size2, union STR_LONG *, union STR_LONG *, struct OP_DEF *, bool bAutoAssign);
	enum OPERANDO_CONDIZIONALE subCMP(const char *, int16_t code, int mode, int8_t VQ, struct VARS *, O_TYPE type1, O_SIZE size1, 
		int RQ, O_TYPE type2, O_SIZE size2, 
		union STR_LONG *, union STR_LONG *, struct OP_DEF *);
	int subInc(bool, int16_t cond, uint8_t T, int8_t VQ, struct VARS *, uint8_t qty, O_TYPE type, O_SIZE size, struct OP_DEF *, 
		struct OP_DEF *,uint8_t isPtr);

	void subObj(COutputFile *,const char*);
	int PROCError(int, ...);
  int PROCWarn(int, ...);
  int PROCV(const char *);
	int PROCT();
	int PROCD();
	int PROCVarList(COutputFile *, struct VARS *, struct VARS *v=NULL);

	char *FNLO(char *);
	char *FNLA(char *);
	int FNGetEscape();
	int FNGetOct(const char *);
	bool PROCCheck(const char *);
	bool PROCCheck(char);
	long FNGetLine(long,char *);
	int FNGoToEOL();
	unsigned int xtoi(const char *);
  BYTE xtob(char );
  unsigned int btoi(const char *);
	char *itox(char *,unsigned int);
	char *itob(char *,unsigned int);
	int lltoa(uint64_t num, char *str, /*int len, */uint8_t base);

	int OPComp(struct OP_DEF *, struct OP_DEF *);
	struct LINE *GetNextNoRem(struct LINE *);
	int Ottimizza(struct LINE *);

  struct LINE *PROCInserLista(struct LINE *, struct LINE *, struct LINE *);
  struct LINE *PROCDelLista(struct LINE *, struct LINE *, struct LINE *);
	void PROCDelLastLine(struct LINE *);
	void swap(struct LINE * *, struct LINE * *);
  void PROCOut(uint8_t, const char *, const char *, const char *s3, const char *R/*=NULL*/, enum LINE_FLAGS flags=LINE_IS_NORMAL);
  void PROCOut1(COutputFile *,const char *, const char *, const char *s3=NULL, const char *s4=NULL);
  void PROCOper(uint8_t, const char *, const char *s2=NULL, const char *s3=NULL, const char *R=NULL, enum LINE_FLAGS=LINE_IS_NORMAL);
  void PROCOper(uint8_t, struct OPERANDO *, const char *R=NULL, enum LINE_FLAGS=LINE_IS_NORMAL);
  void PROCOper(uint8_t, int, const char *R=NULL, enum LINE_FLAGS=LINE_IS_NORMAL);
  void PROCOper(uint8_t, struct VARS *, const char *parm=NULL, const char *R=NULL, enum LINE_FLAGS=LINE_IS_NORMAL);
  int PROCOutLab(const char *,const char *s1=NULL,const char *s2=NULL);

	enum ARITM_OP FNGetAritElem(char *outbuf,int8_t *OP, char *, struct OPERAND *, int8_t Co);
	int subGetType(O_TYPE *type, O_SIZE *size, O_DIM dim, long textpointer);
	int PROCGetType(char *outbuf,O_TYPE *type, O_SIZE *size, struct TAGS **, O_DIM dim, uint32_t *attrib,long textpointer);
	long FNIsType(char *);
	struct VARS *FNGetAggr(struct TAGS *, const char *, int8_t, int *);
	struct VARS *FNGetAggr(struct TAGS *, int8_t);
	uint32_t FNGetAggr2(struct VARS *, struct VARS *, int *, int *ofs=NULL);
	struct TAGS *subAllocTag(const char *,int8_t);
	struct TAGS *FNAllocAggr(int8_t);
	int StoreVar(char *outbuf,const char *op,struct VARS *Vvar,int8_t VQ, struct VARS *RVar, union STR_LONG *, uint16_t ofs);
	int ReadVar(char *outbuf,struct VARS *,O_TYPE type,O_SIZE size,uint8_t/*bool*/ isCond,bool asPtr);

	int PROCUseCost(char *outbuf,int8_t Q, O_TYPE type, O_SIZE size, union STR_LONG *,bool asPtr);
	int FNIsOp(const char *, int);
	int FNIsClass(const char *);
	char *FNGetOperatorFunc(const char *n,char *s);
	O_SIZE FNGetSize(uint32_t);
	O_SIZE FNGetSize(uint64_t);
	O_SIZE FNGetMemSize(O_TYPE type, O_SIZE size, O_DIM dim, uint8_t m);
	O_SIZE FNGetMemSize(struct VARS *, uint8_t);
	O_TYPE FNGetPureType(struct VARS *);
	O_SIZE FNGetArraySize(struct VARS *);
	O_SIZE FNGetArraySize(O_DIM);
	uint8_t FNGetArrayDims(struct VARS *);

	static char *OpCond[16];
	static char *StrOp[20];
	struct BLOCK_PTR OldTX[MAX_BLOCCHI];

// Implementation
	};



/////////////////////////////////////////////////////////////////////////////

struct LINE_NUMBER_IN_FILES {
	uint16_t lineno;			// il #riga (0=inizio
	uint32_t lineEndPos;	// la posizione in cui cambia ossia inizio nuova riga
	};
struct PROCESSED_FILES {
	char nomeFile[116];
	uint32_t begin,end;			// inizio e fine del file letto e processato, nel file uscente .i
	LINE_NUMBER_IN_FILES lines[8192];			// fare dinamico...
	};

class CCPreProcessor {
public:
	enum {
		MAX_DEFS=10
		};
	bool PP;
	struct LINE_DEF *RootDef,*LastDef;
	CPlusMinus *m_Cc;
	CLogFile *m_Log;

private:
	bool UNDEFD[MAX_DEFS];
	bool already_done[MAX_DEFS];		// per elif/else
	bool already_done2[MAX_DEFS];		// per else multipli
	uint8_t IfDefs;
	uint8_t debug,lasciaCommenti;
	struct PROCESSED_FILES filesInfo[MAX_DEFS];

public:
	CCPreProcessor(CPlusMinus *p,uint8_t commenti,CLogFile *logfile,uint8_t d);
	~CCPreProcessor();
  struct LINE_DEF *PROCInserLista(struct LINE_DEF *, struct LINE_DEF *, struct LINE_DEF *);
  struct LINE_DEF *PROCDelLista(struct LINE_DEF *, struct LINE_DEF *, struct LINE_DEF *);
	void swap(struct LINE_DEF * *l1, struct LINE_DEF * *l2);
	char *FNGetParm(CSourceFile *, char *, bool UNDEFD[]);
	char *FNParse(char *, int *, char *);
	char *FNGrab(CSourceFile *, char *, bool UNDEFD[]);
	char *FNGrab1(char *, int *, int *, char *);
	char *FNGetLine(CSourceFile *, char *);
	struct LINE_DEF *FNDefined(const char *);
	int PROCDefine(const char *, const char *);
	char *FNGetNextPre(CSourceFile *, bool, char *, bool UNDEFD[]);
	char *FNPreProcess(CSourceFile *, char *, bool UNDEFD[]);
	void bumpIfs(bool UNDEFD[],int8_t direction,bool state,int8_t *IfDefs);
	int FNLeggiFile(char *, COutputFile *, uint8_t);
	void setDebugLevel(uint8_t d) { debug=d; }
	void setLasciaCommenti(uint8_t t) { lasciaCommenti=t; }
	};



class CTimeSpanEx : public CTimeSpan {
public:
	static CTimeSpan GetCurrentTime();
	};


class CTimeEx : public CTime {
public:
	CTimeEx(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec, int nDST = -1) { CTime t(nYear, nMonth, nDay,
      nHour, nMin, nSec, nDST); *this=*((CTimeEx*)&t);}
	CTimeEx(CTime t) { *this=*((CTimeEx*)&t); }
	static CString Num2Mese(int);
	static CString Num2Giorno(int);
	static CString Num2Month3(int);
	static CString Num2Day3(int);
	static CString getNow(int ex=0);
	static CString getNowGMT(bool bAddCR=TRUE);
	static CString getNowGoogle(bool bAddCR=TRUE);
	static int getMonthFromString(const CString);
	static int getMonthFromGMTString(const CString);
	static CString getMese() { return Num2Giorno(GetCurrentTime().GetDay()); };
	static CString getGiorno() { return Num2Mese(GetCurrentTime().GetMonth()); };
	static CString getFasciaDellaGiornata();
	static CString getSaluto();
	static WORD GetDayOfYear();
	static CTime parseGMTTime(const CString);
	static CTime parseTime(const CString);
	static bool isWeekend();
	bool isWeekend(CTime);
	void AddMonths(int n=1);
	int GetDaysOfMonth();
	};


class CLogFile : public CStdioFile {
	// in VC42 non riuscivo a ereditare da CStdioFile!!! d errore del c. con GetFileTitle
public:
	enum timeStampTypes {
		dontUseDate=0,
		date=1,
		dateTime=2,
		dateTimeMillisec=3,
		flagInfo=0x100,
		flagInfo2=0x100,
		flagValue=0x101,
		flagError=0x102,
		flagError2=0x103,
		flagWarning=0x104,
		flagAlert=0x105,
		flushImmediate=0x80000000,
		keepOpen=0x40000000,
		useIndex=0x20000000
		};
private:
	CString nomeFile,nomeFileNdx;
	const CWnd *textWnd;		// se c'e', indica dove visualizzare la riga di log
	DWORD mode;
	CFile *hIndexFile;
	CRITICAL_SECTION m_cs;

public:
	CLogFile(const CString,const CWnd *myWnd=NULL,DWORD m=dateTime | flushImmediate);
	CLogFile(CFile *f2,const CWnd *myWnd=NULL,DWORD m=dateTime);
	~CLogFile();
	int print(int m,const TCHAR *s,...);		 // m=0 info, 1= letture skynet, 2=errore
	int Open();
	void Close();
	void operator<<(const TCHAR *);
	int ReIndex();
	CString getNow() const;
	int RenameAndStore(int how=0 /* default: appende GG/MM/AAAA*/);
	static CString getNowApache();
	char *getLine(int ,char *,UINT nMax=255);
	DWORD getTotLines() const;
	CString getIndexFileName();
	bool GetStatus(CFileStatus &);
	int clearAll();
	static char *getAsHex(const BYTE *,char *,UINT );

private:
//	CString GetFileTitle() { CString a; return a; };
	};


