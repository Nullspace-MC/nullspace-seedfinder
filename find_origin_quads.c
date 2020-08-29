#include <stdio.h>
#include <stdint.h>
#include "finders.h"

const StructureConfig sconf = FEATURE_CONFIG;

const int lowerBitsCount = 10;
const int64_t lowerBits[] = {
    0x43f18, 0x65118, 0x75618, 0x79a0a, 0x89718,
    0x9371a, 0xa5a08, 0xb5e18, 0xc751a, 0xf520a
};

const char *baseSeedsPath = "./seeds/qh_bases.txt";

/* finds lower 48 bits of seeds that can potentially yield quad bases
 *
 * saves results to "./seeds/qh_bases.txt"
 */
int main(int argc, char *argv[]) {
    FILE *baseSeeds = fopen(baseSeedsPath, "w");
    if(baseSeeds == NULL) {
	fprintf(stderr, "Could not open file \"%s\"\n", baseSeedsPath);
	exit(1);
    }

    for(int64_t u = 0; u < 281474976710656; u += 0x100000) {
	for(int l = 0; l < lowerBitsCount; ++l) {
	    int64_t seed = u + ((lowerBits[l] - sconf.salt) & 0xfffff);
	    int64_t s00 = seed + sconf.salt;
	    int64_t s01 = 341873128712 + seed + sconf.salt;
	    int64_t s10 = 132897987541 + seed + sconf.salt;
	    int64_t s11 = 341873128712 + 132897987541 + seed + sconf.salt;
	    const int64_t K = 0x5deece66dLL;

	    int x, z;

	    // The lower 20 bits already guarantee the structures will be
	    // in range if they are in the correct 8*8 section of each
	    // region, so we just need to verify that each structure is in
	    // the correct 8*8 section.
	    s00 ^= K;
	    JAVA_NEXT_INT24(s00, x); if(x < 16) continue;
	    JAVA_NEXT_INT24(s00, z); if(z < 16) continue;

	    s01 ^= K;
	    JAVA_NEXT_INT24(s01, x); if(x >= 8) continue;
	    JAVA_NEXT_INT24(s01, z); if(z < 16) continue;

	    s10 ^= K;
	    JAVA_NEXT_INT24(s10, x); if(x < 16) continue;
	    JAVA_NEXT_INT24(s10, z); if(z >= 8) continue;

	    s11 ^= K;
	    JAVA_NEXT_INT24(s11, x); if(x >= 8) continue;
	    JAVA_NEXT_INT24(s11, z); if(z >= 8) continue;

	    printf("%ld\n", seed);
	    fprintf(baseSeeds, "%ld\n", seed);
	}
    }

    fclose(baseSeeds);
    return 0;
}
