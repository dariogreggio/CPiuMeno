//test_base.cpp - File di test per il Transpiler C++ -> C

int pippo(int a,char c) { return 1; }
int ciao(int a,char b=7,int c=9);
float ff;
class Shape;		// 

// Struct/Classe semplice con incapsulamento e costruttore
class Shape {
public:
	int id;
    void printId() { 		/* ... */ 		}

private:
    int x;
    int y;

public:
	static  void muori() {
		return ;
		}
	int crepa() {
		return 666;
		}


    // Costruttore con valori di default
    Shape(int startX, int startY) {
        x = startX;
        y = startY;
    }
    Shape() {
		x=y=0;
		}
    ~Shape() {
//	free();
		}


    // Overloading dei metodi (test per il Name Mangling)
    void print() {
        // In C diventerà un'uscita formattata es. printf
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
	Point(int z) {
		}
	Point(float l);
	~Point();
	int scala(int n) {		// (non va senza tipo di ritorno e se solo prototipo e/o senza var
	return 1;
		}
	void scala(char *s) ;
	void /*Point::*/Move(int h);		// deve accettare classe e :: ev errore se diversa


    // Metodo semplice (richiede il passaggio esplicito di 'this' in C)
    void move(int dx, int dy) {
        x += dx;
        y += dy;
		}
	};

void Point::Move(int h) {
	}
//void Point::Movez(int h);
//int Point::Point(float g) {	return 5; }
		

#if 0
// Funzione che accetta un Riferimento C++ (&)
void scalePoint(Point& p, int factor) {
    // Il transpiler dovrà trasformare p.move in Point_move(&p, ...)
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

//SHAPE glShape;	// (NON d� errore alloca SHAPE!
class Shape glShape;

int ciao(int a,int b,int c) {
	while(a--)
		b++;
	return b;
	}

// Main di test
int main(int a) {
	short int y;
	struct CULO v;

w.a=3;
y=a;

    // Inizializzazione oggetto (deve invocare il costruttore)
    Point pt(10, 20);
Point pt2;
Shape sh;
y=2;
y=pt.x;
pt.print();
pt.print(1);
Shape::muori();
pt.muori();
    // Chiamata a metodo su oggetto
   pt2.move(5, -2);
   int finalX = pt.getX();


y=Point::muori();
//y=Point::crepa();
//Point::print();
pt.crepa();


//class	Point pt2(10,20);
#if 0


    // Passaggio per riferimento
  //  scalePoint(pt, 2);

#endif

	ciao(3); ciao(15,17); ciao(1,2);
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

