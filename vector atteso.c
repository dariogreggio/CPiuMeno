/* --- Struttura dati --- */
struct Vector2D {
    float x;
    float y;
};

/* --- Prototipi e Mangling degli Operatori --- */
void Vector2D_ctor__FF(struct Vector2D *this, float _x, float _y);
void Vector2D_op_add__RVector2D(struct Vector2D *__retval, const struct Vector2D *this, const struct Vector2D *other);
void Vector2D_op_neg__(struct Vector2D *__retval, const struct Vector2D *this);
struct Vector2D* Vector2D_op_add_assign__RVector2D(struct Vector2D *this, const struct Vector2D *other);

/* --- Costruttore --- */
void Vector2D_ctor__FF(struct Vector2D *this, float _x, float _y) {
    this->x = _x;
    this->y = _y;
}

/* 1. Traduzione di operator+ */
void Vector2D_op_add__RVector2D(struct Vector2D *__retval, const struct Vector2D *this, const struct Vector2D *other) {
    /* Costruisce direttamente l'oggetto di ritorno temporaneo */
    Vector2D_ctor__FF(__retval, this->x + other->x, this->y + other->y);
}

/* 2. Traduzione di operator- (unario) */
void Vector2D_op_neg__(struct Vector2D *__retval, const struct Vector2D *this) {
    Vector2D_ctor__FF(__retval, -this->x, -this->y);
}

/* 3. Traduzione di operator+= */
struct Vector2D* Vector2D_op_add_assign__RVector2D(struct Vector2D *this, const struct Vector2D *other) {
    this->x += other->x;
    this->y += other->y;
    return this; /* Restituisce *this per consentire il chaining */
}

/* --- Main --- */
int main(short int a) {
    int __retval;
    
    struct Vector2D v1;
    struct Vector2D v2;
    struct Vector2D v3;
    struct Vector2D v4;
    
    /* Variabili temporanee per contenere i rvalue restituiti dagli operatori */
    struct Vector2D __tmp1;
    struct Vector2D __tmp2;

    /* Inizializzazioni con default/valori espressi */
    Vector2D_ctor__FF(&v1, 1.0f, 2.0f);
    Vector2D_ctor__FF(&v2, 3.0f, 4.0f);

    /* Vector2D v3 = v1 + v2; */
    Vector2D_op_add__RVector2D(&__tmp1, &v1, &v2);
    v3 = __tmp1; /* Copia della struct C89 */

    /* Vector2D v4 = -v1; */
    Vector2D_op_neg__(&__tmp2, &v1);
    v4 = __tmp2;

    /* v1 += v2; */
    Vector2D_op_add_assign__RVector2D(&v1, &v2);

    __retval = 0;
    return __retval;
}

