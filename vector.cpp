class complex {
public:
	int re,im;

	int operator+(int n) {
		im = im+n;
		return re+n;
		}
	int operator-(int n) {
		im -= n;
		return re-n;
		}
	};

int main() {
	complex c;
	int n;

	n=n+1;
	c=c+1;
	c=c*4;

	return c.re;
	}

#if 0

class Vector2D {
public:
    float x, y;

//    Vector2D(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}

    // 1. Operatore Binario (+) : Somma membro tra vettori
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    // 2. Operatore Unario (-) : Negazione delle coordinate
    Vector2D operator-() const {
        return Vector2D(-x, -y);
    }

    // 3. Operatore Assegnamento con Somma (+=)
    Vector2D& operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

int main() {
    Vector2D v1(1.0f, 2.0f);
    Vector2D v2(3.0f, 4.0f);

    Vector2D v3 = v1 + v2;  // Usa operator+
    Vector2D v4 = -v1;      // Usa operator-
    v1 += v2;               // Usa operator+=

    return 0;
}

#endif

