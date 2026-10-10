//test_base.cpp - File di test per il Transpiler C++ -> C

int pippo(int a,char c) { return 1; }

extern "C" {
int minchia(int a);
}
int ciao(int a,int b=7,int c=9);

float ff;
class Shape;		// 

// Struct/Classe semplice con incapsulamento e costruttore
class Shape {
public:
	int id;
	static long id2;		// esce int?! ora Ë ok, int e long sono uguali :) o no??
//	static char ch=0;		// (DARE errore anche se func
    void printId() { 		/* ... */ 		}

private:
    int x;
    int y;

public:
	static  void muori();
	static  void muori() {
		return ;
		}
	int crepa() {
		return 666;
		}


    // Costruttore con valori di default
    Shape(int startX, int startY) {
				int z;
        x = startX;
        y = startY;
				z=x;
    }
    Shape() {
		x=y=0;
		}
    ~Shape() {
//	free();
		}


    // Overloading dei metodi (test per il Name Mangling)
    void print() {
        // In C diventer√† un'uscita formattata es. printf
    }

    void print(int prefixCode) {
        // Stesso nome, parametri diversi -> deve generare un nome C unico
    }


    // Metodo getter
    int getX() {
        return x;
    }

//	char c;

	};


//int Shape::id2;
int Shape::id2=3456;

class Dario {
	int aa;
	};

class Point : Shape {
public:
	signed short t;
//    int x;
  //  int y;
public:
	Point(char *s) {
		}
	Point(char **s) {
		}
//	Point(int z) : Shape(&z) {
	//	}
	Point(float l);
	Point() {}
	~Point();
	int scala(int n) {		// (non va senza tipo di ritorno e se solo prototipo e/o senza var
	return 1;
		}
	void scala(char *s) ;
	void Point::Move(int h);		// deve accettare classe e :: ev errore se diversa


    // Metodo semplice (richiede il passaggio esplicito di 'this' in C)
    void move(int dx, int dy) {
			int z;
        x += dx;
        y += dy;
			z=y;
			z=dx;
//z=id2;
		}
	};

Point::Point(float g) : Shape(/*l*/)  {	g=1.0; /*return 5;*/ }
void Point::Move(int h) {
	}
//void Point::Movez(int h);
///*int */Point::Point(float g) {	/*return 5;*/ }
/*int */Point::~Point() {	/*return 5;*/ }
		

#if 0
// Funzione che accetta un Riferimento C++ (&)
void scalePoint(Point& p, int factor) {
    // Il transpiler dovr√† trasformare p.move in Point_move(&p, ...)
    p.move(factor, factor);
	}
#endif

void refTest(int& p, int factor) {
  p*=factor;
	}

struct CULO {
int a,b,c;
};
CULO w;

//SHAPE glShape;	// (NON d‡ errore alloca SHAPE!
//class Shape glShape(1,1);

int ciao(int a,int b,int c=9) {
	char *p=new char[32];
	p=new Shape;
	p=new char;
	p=new class Shape;	// d‡ warning per il :
	p=new class Shape(10,7);
	p=new struct CULO;		// d‡ warning per il :
	while(a--)
		b++;
	delete p;
	return b;
	}

// Main di test
int main(int a) {
	short int y;
	struct CULO v;

w.a=3;
y=a;
y=Shape::id2;

    // Inizializzazione oggetto (deve invocare il costruttore)
//    Point pt2(10, 20);
Point pt;
Shape sh;
Shape sh2(10,20);
y=2;
y=pt.x;
pt.print();
pt.print(1);
Shape::muori();
//Point::print();
//sh::muori();
pt.muori();
    // Chiamata a metodo su oggetto
   pt.move(5, -2);
   int finalX = pt.getX();
//   finalX = pt.getX();


//y=Point::muori();			// (DEVE dare errore!!
//y=Point::crepa();
//Point::print();
pt.crepa();
sh2.muori();


//class	Point pt2(10,20);

    // Passaggio per riferimento
//    scalePoint(pt, 2);


//	ciao(3); ciao(15,17); ciao(1,2);		// finire mangling con parm default!
ciao(1,2,3);

	if(y==2)
		y=8;

	if(y==2) {
		y=8;
		}
	else {
		y=225;
		}

	for(y=1; y<4; y++) {
		pt.crepa();
		}

	switch(y) {
		case 3:
			break;
		case 31:
			break;
		default:
			break;
		}

	return 0;
	}


//int ciao(int a,char b=1);

