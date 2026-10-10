class Shape {
public:

private:
  int x;

public:
  Shape(int n=88) {		// non usa il default
		x=4;
		}
  Shape(int n=1,char r=8/*,char a=3,char b=3*/) {
		x=5;
		}
  Shape() {
		}
  ~Shape() {
		}
	int operator+(int n) {
		return n;
		}
	virtual int buco(int a, int b) {
		return a-b;		// manca b
		}
	virtual int buco3(char *s) {
		return 1;
		}
	virtual int buco2(int a);

	};

/*class*/ Shape glShape();

class Point : Shape {
public:
/*static*/	signed short t;

public:
	Point(int x,int y);
	Point(int x);
	Point(float l);
	char culo1() {
		return '\x2';
		}
	void aaa();
	virtual int buco2(int a) {
		return a-b;		// non dà errore
		}
	virtual int buco4(int a) {
		return a;
		}
//	char n;		// (dà errore se non c'è...

	};

//Point::Point(float g) : Shape(4),t(5)/*,t(0)*/ {	g=1.0; }
Point::Point(float g) {	g=1.0; }
//Point::Point(float l);

int main(short int a) {
	short int y;

	Point pt3(8,8);
	Point pt(1.2);
	Point pt2(3.4);
/*	class */ Shape sh,*sh2;
	sh.buco(3,7);
	sh2->buco(3,7);
//	Shape::buco(3,7);
	pt.buco(3,7);
	pt.buco4(37);

	return 98;

	}

