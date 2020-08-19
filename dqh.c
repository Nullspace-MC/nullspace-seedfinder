#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "finders.h"
#include "xmapview.h"



int main()
{
    long scnt, *seeds;
    //seeds = loadSavedSeeds("quadbases_Q1b.txt", &scnt);
    seeds = loadSavedSeeds("./seeds/origin_quadhut_bases.txt", &scnt);

    // (baseSeed - regionX*341873128712 - regionZ*132897987541) & 0xffffffffffff;
    // 132897987541 = 0x1ef1565bd5
    // 341873128712 = 0x4f9939f508

    // (A + 341873128712*x + 132897987541*z) & 0xffffffffffff == B & 0xffffffffffff
    // D = (A-B) & 0xffffffffffff
    // => D + 341873128712*x + 132897987541*z == n*(1<<48)
    // => D + n*(1<<48) + 132897987541*z == -341873128712*x
    // => required: (D + n*(1<<48) + 132897987541*z) % 341873128712 == 0


    //const long range = 530;
    const long range = 64;
    int i;

    printf("i_start = ");
    if(scanf("%d", &i) != 1) i = 0;

    for(; i < scnt; i++)
    {
        for(int j = 0; j < scnt; j++)
        {
            if(i == j) continue;
            long D = (seeds[j] - seeds[i]) & 0xffffffffffff;

            const long start = D - range * 132897987541;
            const long end   = D + range * 132897987541;

            for(long lhs = start; lhs <= end; lhs += 132897987541)
            {
                // (x % 8 == 0) is a requirement for (x % 0x4f9939f508 == 0)
                if(lhs & 0x7) continue;

                // long integer divisions take about 60 cycles
                long mod = lhs % 341873128712;

                if(mod == 0)
                {
                    long x = (lhs) / 341873128712;
                    long z = -(lhs - D) / 132897987541;

                    printf("%ld & %ld @ (%ld %ld)\n", seeds[i], seeds[j], x, z);
                }

                //mod = mod + 0x1a66ad4347; // (lhs+(1L<<48)) % 0x4f9939f508

                if((lhs+(1L<<48)) % 341873128712 == 0)
                {
                    long x = (lhs+(1L<<48)) / 341873128712;
                    long z = -(lhs - D) / 132897987541;

                    printf("%ld & %ld @ (%ld %ld)\n", seeds[i], seeds[j], x, z);
                }

                if((lhs-(1L<<48)) % 341873128712 == 0)
                {
                    long x = (lhs-(1L<<48)) / 341873128712;
                    long z = -(lhs - D) / 132897987541;

                    printf("%ld & %ld @ (%ld %ld)\n", seeds[i], seeds[j], x, z);
                }
            }
        }
        if(i % 1000 == 0)
            printf("i=%d\n", i);
    }

    return 0;
}


