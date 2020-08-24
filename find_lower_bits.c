#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"

/* finds lower 20 bits of seeds that can potentially yield quad bases
 * 
 * IMPORTANT: these base seeds need the salt of the structure in question
 * subtracted from them in order to be usable
 *
 * uses property of nextInt(24) that its value mod 8 depends only on the
 * bottommost 20 bits of the seed
 */
int main(int argc, char *argv[]) {
    int range = (argc > 1) ? atoi(argv[1]) : 3;
    int lower = 8 - range;
    int upper = range - 1;

    for(int64_t seed = 0; seed < 0x100000; ++seed) {
	int64_t s00 = seed;
	int64_t s01 = 341873128712 + seed;
	int64_t s10 = 132897987541 + seed;
	int64_t s11 = 341873128712 + 132897987541 + seed;
	const int64_t K = 0x5deece66dLL;

	int x00, z00, x01, z01, x10, z10, x11, z11;
	int x, z;

	s00 ^= K;
	JAVA_NEXT_INT24(s00, x00); if((x00 & 0x7) < lower) continue;
	JAVA_NEXT_INT24(s00, z00); if((z00 & 0x7) < lower) continue;

	s11 ^= K;
	JAVA_NEXT_INT24(s11, x11); if((x11 & 0x7) > upper) continue;
	JAVA_NEXT_INT24(s11, z11); if((z11 & 0x7) > upper) continue;

	x00 = (x00 & 0x7) + 16;
	z00 = (z00 & 0x7) + 16;
	x11 &= 0x7;
	z11 &= 0x7;
	x = (x11 + 32) - x00;
	z = (z11 + 32) - z00;
	if(x*x + z*z > 255) continue;

	s01 ^= K;
	JAVA_NEXT_INT24(s01, x01); if((x01 & 0x7) > upper) continue;
	JAVA_NEXT_INT24(s01, z01); if((z01 & 0x7) < lower) continue;

	s10 ^= K;
	JAVA_NEXT_INT24(s10, x10); if((x10 & 0x7) < lower) continue;
	JAVA_NEXT_INT24(s10, z10); if((z10 & 0x7) > upper) continue;

	x01 &= 0x7;
	z01 = (z01 & 0x7) + 16;
	x10 = (x10 & 0x7) + 16;
	z10 &= 0x7;
	x = (x01 + 32) - x10;
	z = (z10 + 32) - z01;
	if(x*x + z*z > 255) continue;

	float sqrad = getEnclosingRadius(
	    x00, z00, x11, z11, x01, z01, x10, z10,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);

	if(sqrad < 128) {
	    printf("%#07x\n", (unsigned int)seed);
	}
    }

    return 0;
}
