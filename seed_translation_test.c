#include <stdio.h>
#include <stdint.h>

#define SEED_1 114797533878687
#define D_REG_X 5
#define D_REG_Z 8

int64_t translateSeed(int64_t seed, int dRegX, int dRegZ) {
    return (seed - 341873128712 * dRegX - 132897987541 * dRegZ) & 0xffffffffffff;
}

int main() {
    int64_t seed1 = SEED_1;
    int64_t seed2 = translateSeed(seed1, D_REG_X, D_REG_Z);
    int64_t diff = seed1 - seed2;

    printf("Seed 1: %ld\n", seed1);
    printf("Seed 2: %ld (Translated by %d, %d)\n", seed2, D_REG_X, D_REG_Z);
    printf("Difference: %ld (Seed 1 - Seed 2)\n", diff);
    
    int64_t dRegX = (49284120807 * diff) & 0x1fffffffffff;
    int64_t dRegZ = (-126780825563 * diff) & 0xffffffffffff;

    int64_t seed3 = translateSeed(seed1, dRegX, dRegZ);

    printf("Seed 3: %ld (Translated by %ld, %ld)\n", seed3, dRegX, dRegZ);
}
