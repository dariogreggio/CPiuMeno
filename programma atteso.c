/* test_base.c - Codice C generato dal Transpiler */

typedef struct {
    int x;
    int y;
} Point;

/* Costruttore -> Funzione di inizializzazione */
void Point_ctor__ii(Point* const this, int startX, int startY) {
    this->x = startX;
    this->y = startY;
}

/* Metodo -> Funzione globale con 'this' */
void Point_move__ii(Point* const this, int dx, int dy) {
    this->x += dx;
    this->y += dy;
}

/* Overloading 1 */
void Point_print(Point* const this) {
}

/* Overloading 2 (Name Mangling per distinguere i parametri) */
void Point_print__i(Point* const this, int prefixCode) {
}

int Point_getX(Point* const this) {
    return this->x;
}

/* Riferimento Point& convertito in puntatore Point* */
void scalePoint(Point* const p, int factor) {
    Point_move(p, factor, factor);
}

int main() {
    Point pt;
    Point_ctor(&pt, 10, 20); /* Invocazione esplicita costruttore */

    Point_move(&pt, 5, -2);

    scalePoint(&pt, 2);     /* Passaggio dell'indirizzo al posto del riferimento */

    int finalX = Point_getX(&pt);

    return 0;
}