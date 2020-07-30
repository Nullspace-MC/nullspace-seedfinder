#include <stdio.h>
#include <stdint.h>

typedef struct {
    int64_t old;
    int64_t cur;
} pair;

/* Uses the Extended Euclidean Algorithm to calculate Bézout coefficients of
 * a pair of numbers.
 */
int main(int argc, char *argv[]) {
    const int64_t a = 341873128712;
    const int64_t b = 132897987541;

    pair r = {a, b};
    pair s = {1, 0};
    pair t = {0, 1};

    int64_t quotient;
    while(r.cur != 0) {
	quotient = r.old / r.cur;
	r = (pair){r.cur, r.old - quotient * r.cur};
	s = (pair){s.cur, s.old - quotient * s.cur};
	t = (pair){t.cur, t.old - quotient * t.cur};
    }

    printf("Bézout coefficients of %ld and %ld are %ld and %ld.\n",
	    a, b, s.old, t.old);

    return 0;
}
