#include "layers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SIMD_NOTIFY 0

#if defined USE_SIMD && __AVX2__
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#if SIMD_NOTIFY
#warning "Using AVX2 extensions."
#endif
#elif defined USE_SIMD && defined __SSE4_2__
#include <emmintrin.h>
#include <smmintrin.h>
#if SIMD_NOTIFY
#warning "Using SSE4.2 extensions."
#endif
#else
//#warning "Using no SIMD extensions."
#endif


static void oceanRndInit(OceanRnd *rnd, int64_t seed);


void initAddBiome(int id, int tempCat, int biometype, float temp, float height)
{
    if (id & (~0xff)) return;
    biomes[id].id = id;
    biomes[id].type = biometype;
    biomes[id].temp = temp;
    biomes[id].height = height;
    biomes[id].tempCat = tempCat;
}

void createMutation(int id)
{
    biomes[id].mutated = id + 128;
    biomes[id+128] = biomes[id];
    biomes[id+128].id = id+128;
}

/* initBiomes() has to be called before any of the generators can be used */
void initBiomes()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        biomes[i].id = none;
        biomes[i].type = Void;
        biomes[i].temp = 0.5;
        biomes[i].height = 0;
        biomes[i].tempCat = Void;
        biomes[i].mutated = -1;
    }

    const double hDefault = 0.1, hShallowWaters = -0.5, hOceans = -1.0, hDeepOceans = -1.8, hLowPlains = 0.125;
    const double hMidPlains = 0.2, hLowHills = 0.45, hHighPlateaus = 1.5, hMidHills = 1.0, hShores = 0.0;
    const double hRockyWaters = 0.1, hLowIslands = 0.2, hPartiallySubmerged = -0.2;

    initAddBiome(ocean, Oceanic, Ocean, 0.5, hOceans);
    initAddBiome(plains, Lush, Plains, 0.8, hDefault);
    initAddBiome(desert, Warm, Desert, 2.0, hLowPlains);
    initAddBiome(mountains, Lush, Hills, 0.2, hMidHills);
    initAddBiome(forest, Lush, Forest, 0.7, hDefault);
    initAddBiome(taiga, Lush, Taiga, 0.25, hMidPlains);
    initAddBiome(swamp, Lush, Swamp, 0.8, hPartiallySubmerged);
    initAddBiome(river, Lush, River, 0.5, hShallowWaters);
    initAddBiome(nether_wastes, Warm, Nether, 2.0, hDefault);
    initAddBiome(the_end, Lush, Sky, 0.5, hDefault);
    initAddBiome(frozen_ocean, Oceanic, Ocean, 0.0, hOceans);
    initAddBiome(frozen_river, Cold, River, 0.0, hShallowWaters);
    initAddBiome(snowy_tundra, Cold, Snow, 0.0, hLowPlains);
    initAddBiome(snowy_mountains, Cold, Snow, 0.0, hLowHills);
    initAddBiome(mushroom_fields, Lush, MushroomIsland, 0.9, hLowIslands);
    initAddBiome(mushroom_field_shore, Lush, MushroomIsland, 0.9, hShores);
    initAddBiome(beach, Lush, Beach, 0.8, hShores);
    initAddBiome(desert_hills, Warm, Desert, 2.0, hLowHills);
    initAddBiome(wooded_hills, Lush, Forest, 0.7, hLowHills);
    initAddBiome(taiga_hills, Lush, Taiga, 0.25, hLowHills);
    initAddBiome(mountain_edge, Lush, Hills, 0.2, hMidHills);
    initAddBiome(jungle, Lush, Jungle, 0.95, hDefault);
    initAddBiome(jungle_hills, Lush, Jungle, 0.95, hLowHills);
    initAddBiome(jungle_edge, Lush, Jungle, 0.95, hDefault);
    initAddBiome(deep_ocean, Oceanic, Ocean, 0.5, hDeepOceans);
    initAddBiome(stone_shore, Lush, StoneBeach, 0.2, hRockyWaters);
    initAddBiome(snowy_beach, Cold, Beach, 0.05, hShores);
    initAddBiome(birch_forest, Lush, Forest, 0.6, hDefault);
    initAddBiome(birch_forest_hills, Lush, Forest, 0.6, hLowHills);
    initAddBiome(dark_forest, Lush, Forest, 0.7, hDefault);
    initAddBiome(snowy_taiga, Cold, Taiga, -0.5, hMidPlains);
    initAddBiome(snowy_taiga_hills, Cold, Taiga, -0.5, hLowHills);
    initAddBiome(giant_tree_taiga, Lush, Taiga, 0.3, hMidPlains);
    initAddBiome(giant_tree_taiga_hills, Lush, Taiga, 0.3, hLowHills);
    initAddBiome(wooded_mountains, Lush, Hills, 0.2, hMidHills);
    initAddBiome(savanna, Warm, Savanna, 1.2, hLowPlains);
    initAddBiome(savanna_plateau, Warm, Savanna, 1.0, hHighPlateaus);
    initAddBiome(badlands, Warm, Mesa, 2.0, hDefault);
    initAddBiome(wooded_badlands_plateau, Warm, Mesa, 2.0, hHighPlateaus);
    initAddBiome(badlands_plateau, Warm, Mesa, 2.0, hHighPlateaus);

    initAddBiome(small_end_islands, Lush, Sky, 0.5, hDefault);
    initAddBiome(end_midlands, Lush, Sky, 0.5, hDefault);
    initAddBiome(end_highlands, Lush, Sky, 0.5, hDefault);
    initAddBiome(end_barrens, Lush, Sky, 0.5, hDefault);
    initAddBiome(warm_ocean, Oceanic, Ocean, 0.5, hOceans);
    initAddBiome(lukewarm_ocean, Oceanic, Ocean, 0.5, hOceans);
    initAddBiome(cold_ocean, Oceanic, Ocean, 0.5, hOceans);
    initAddBiome(deep_warm_ocean, Oceanic, Ocean, 0.5, hDeepOceans);
    initAddBiome(deep_lukewarm_ocean, Oceanic, Ocean, 0.5, hDeepOceans);
    initAddBiome(deep_cold_ocean, Oceanic, Ocean, 0.5, hDeepOceans);
    initAddBiome(deep_frozen_ocean, Oceanic, Ocean, 0.5, hDeepOceans);

    initAddBiome(the_void, Void, Void, 0.5, 0);

    createMutation(plains);
    createMutation(desert);
    createMutation(mountains);
    createMutation(forest);
    createMutation(taiga);
    createMutation(swamp);
    createMutation(snowy_tundra);
    createMutation(jungle);
    createMutation(jungle_edge);
    createMutation(birch_forest);
    createMutation(birch_forest_hills);
    createMutation(dark_forest);
    createMutation(snowy_taiga);
    createMutation(giant_tree_taiga);
    createMutation(giant_tree_taiga_hills);
    createMutation(wooded_mountains);
    createMutation(savanna);
    createMutation(savanna_plateau);
    createMutation(badlands);
    createMutation(wooded_badlands_plateau);
    createMutation(badlands_plateau);

    initAddBiome(bamboo_jungle, Lush, Jungle, 0.95, hDefault);
    initAddBiome(bamboo_jungle_hills, Lush, Jungle, 0.95, hLowHills);

    initAddBiome(soul_sand_valley, Warm, Nether, 2.0, hDefault);
    initAddBiome(crimson_forest, Warm, Nether, 2.0, hDefault);
    initAddBiome(warped_forest, Warm, Nether, 2.0, hDefault);
    initAddBiome(basalt_deltas, Warm, Nether, 2.0, hDefault);
}


void setWorldSeed(Layer *layer, int64_t worldSeed)
{
    if (layer->p2 != NULL && layer->getMap != mapHills)
        setWorldSeed(layer->p2, worldSeed);

    if (layer->p != NULL)
        setWorldSeed(layer->p, worldSeed);

    if (layer->oceanRnd != NULL)
        oceanRndInit(layer->oceanRnd, worldSeed);

    int64_t st = worldSeed;
    st = mcStepSeed(st, layer->layerSeed);
    st = mcStepSeed(st, layer->layerSeed);
    st = mcStepSeed(st, layer->layerSeed);

    layer->startSalt = st;
    layer->startSeed = mcStepSeed(st, 0);
}


void mapNull(const Layer * l, int * out, int x, int z, int w, int h)
{
}

void mapSkip(const Layer * l, int * out, int x, int z, int w, int h)
{
    if (l->p == NULL)
    {
        printf("mapSkip() requires a non-null parent layer.\n");
        exit(1);
    }
    l->p->getMap(l->p, out, x, z, w, h);
}


void mapIsland(const Layer * l, int * out, int x, int z, int w, int h)
{
    register int i, j;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            cs = getChunkSeed(ss, i + x, j + z);
            out[i + j*w] = mcFirstIsZero(cs, 10);
        }
    }

    if (x > -w && x <= 0 && z > -h && z <= 0)
    {
        out[-x + -z * w] = 1;
    }
}

// FIXME: currently SIMD only works properly for certain sizes
#if defined USE_SIMD && defined __AVX2__

static inline __m256i set8ChunkSeeds(int ws, __m256i xs, __m256i zs)
{
    __m256i out = _mm256_set1_epi32(ws);
    __m256i mul = _mm256_set1_epi32(1284865837);
    __m256i add = _mm256_set1_epi32(4150755663);
    out = _mm256_add_epi32(xs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    out = _mm256_add_epi32(zs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    out = _mm256_add_epi32(xs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    return _mm256_add_epi32(zs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
}

static inline __m256i mc8NextInt(__m256i* cs, int ws, int mask)
{
    __m256i andm = _mm256_set1_epi32(mask);
    __m256i ret = _mm256_and_si256(andm, _mm256_srli_epi32(*cs, 24));
    *cs = _mm256_add_epi32(_mm256_set1_epi32(ws), _mm256_mullo_epi32(*cs, _mm256_add_epi32(_mm256_set1_epi32(4150755663), _mm256_mullo_epi32(*cs, _mm256_set1_epi32(1284865837)))));
    return _mm256_add_epi32(ret, _mm256_and_si256(andm, _mm256_cmpgt_epi32(_mm256_set1_epi32(0), ret)));
}

static inline __m256i select8Random2(__m256i* cs, int ws, __m256i a1, __m256i a2)
{
    __m256i cmp = _mm256_cmpeq_epi32(_mm256_set1_epi32(0), mc8NextInt(cs, ws, 0x1));
    return _mm256_or_si256(_mm256_and_si256(cmp, a1), _mm256_andnot_si256(cmp, a2));
}

static inline __m256i select8Random4(__m256i* cs, int ws, __m256i a1, __m256i a2, __m256i a3, __m256i a4)
{
    __m256i val = mc8NextInt(cs, ws, 0x3);
    __m256i v2 = _mm256_set1_epi32(2);
    __m256i cmp1 = _mm256_cmpeq_epi32(val, _mm256_set1_epi32(0));
    __m256i cmp2 = _mm256_cmpeq_epi32(v2, val);
    __m256i cmp3 = _mm256_cmpgt_epi32(v2, val);
    return _mm256_or_si256(
        _mm256_and_si256(cmp3, _mm256_or_si256(_mm256_and_si256(cmp1, a1), _mm256_andnot_si256(cmp1, a2))),
        _mm256_andnot_si256(cmp3, _mm256_or_si256(_mm256_and_si256(cmp2, a3), _mm256_andnot_si256(cmp2, a4)))
    );
}

static inline __m256i select8ModeOrRandom(__m256i* cs, int ws, __m256i a1, __m256i a2, __m256i a3, __m256i a4)
{
    __m256i cmp1 = _mm256_cmpeq_epi32(a1, a2);
    __m256i cmp2 = _mm256_cmpeq_epi32(a1, a3);
    __m256i cmp3 = _mm256_cmpeq_epi32(a1, a4);
    __m256i cmp4 = _mm256_cmpeq_epi32(a2, a3);
    __m256i cmp5 = _mm256_cmpeq_epi32(a2, a4);
    __m256i cmp6 = _mm256_cmpeq_epi32(a3, a4);
    __m256i isa1 = _mm256_or_si256(
                       _mm256_andnot_si256(cmp6, cmp1),
                       _mm256_or_si256 (
                           _mm256_andnot_si256(cmp5, cmp2),
                           _mm256_andnot_si256(cmp4, cmp3)
                       )
                   );
    __m256i isa2 = _mm256_or_si256(
                       _mm256_andnot_si256(cmp3, cmp4),
                       _mm256_andnot_si256(cmp2, cmp5)
                   );
    __m256i isa3 = _mm256_andnot_si256(cmp1, cmp6);

    return _mm256_or_si256(
        _mm256_andnot_si256(
            _mm256_or_si256(
                isa1,
                _mm256_or_si256(isa2, isa3)
            ),
            select8Random4(cs, ws, a1, a2, a3, a4)
        ),
        _mm256_or_si256(
            _mm256_and_si256(isa1, a1),
            _mm256_or_si256(
                _mm256_and_si256(isa2, a2),
                _mm256_and_si256(isa3, a3)
            )
        )
    );
}

#elif defined USE_SIMD && defined __SSE4_2__

static inline __m128i set4ChunkSeeds(int ws, __m128i xs, __m128i zs)
{
    __m128i out = _mm_set1_epi32(ws);
    __m128i mul = _mm_set1_epi32(1284865837);
    __m128i add = _mm_set1_epi32(4150755663);
    out = _mm_add_epi32(xs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    out = _mm_add_epi32(zs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    out = _mm_add_epi32(xs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    return _mm_add_epi32(zs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
}

static inline __m128i mc4NextInt(__m128i* cs, int ws, int mask)
{
    __m128i andm = _mm_set1_epi32(mask);
    __m128i ret = _mm_and_si128(andm, _mm_srli_epi32(*cs, 24));
    *cs = _mm_add_epi32( _mm_set1_epi32(ws), _mm_mullo_epi32(*cs, _mm_add_epi32(_mm_set1_epi32(4150755663), _mm_mullo_epi32(*cs, _mm_set1_epi32(1284865837)))));
    return _mm_add_epi32(ret, _mm_and_si128(andm, _mm_cmplt_epi32(ret, _mm_set1_epi32(0))));
}

static inline __m128i select4Random2(__m128i* cs, int ws, __m128i a1, __m128i a2)
{
    __m128i cmp = _mm_cmpeq_epi32(_mm_set1_epi32(0), mc4NextInt(cs, ws, 0x1));
    return _mm_or_si128(_mm_and_si128(cmp, a1), _mm_andnot_si128(cmp, a2));
}

static inline __m128i select4Random4(__m128i* cs, int ws, __m128i a1, __m128i a2, __m128i a3, __m128i a4)
{
    __m128i val = mc4NextInt(cs, ws, 0x3);
    __m128i v2 = _mm_set1_epi32(2);
    __m128i cmp1 = _mm_cmpeq_epi32(val, _mm_set1_epi32(0));
    __m128i cmp2 = _mm_cmpeq_epi32(val, v2);
    __m128i cmp3 = _mm_cmplt_epi32(val, v2);
    return _mm_or_si128(
        _mm_and_si128(cmp3, _mm_or_si128(_mm_and_si128(cmp1, a1), _mm_andnot_si128(cmp1, a2))),
        _mm_andnot_si128(cmp3, _mm_or_si128(_mm_and_si128(cmp2, a3), _mm_andnot_si128(cmp2, a4)))
    );
}

static inline __m128i select4ModeOrRandom(__m128i* cs, int ws, __m128i a1, __m128i a2, __m128i a3, __m128i a4)
{
    //((a == b)&(c != d) | (a == c)&(b != d) | (a == d)&(b != c))&a | ((b == c)&(a != d) | (b == d)&(a != c))&b | ((c == d)&(a != b))&c
    __m128i cmp1 = _mm_cmpeq_epi32(a1, a2);
    __m128i cmp2 = _mm_cmpeq_epi32(a1, a3);
    __m128i cmp3 = _mm_cmpeq_epi32(a1, a4);
    __m128i cmp4 = _mm_cmpeq_epi32(a2, a3);
    __m128i cmp5 = _mm_cmpeq_epi32(a2, a4);
    __m128i cmp6 = _mm_cmpeq_epi32(a3, a4);
    __m128i isa1 = _mm_or_si128(
                       _mm_andnot_si128(cmp6, cmp1),
                       _mm_or_si128 (
                           _mm_andnot_si128(cmp5, cmp2),
                           _mm_andnot_si128(cmp4, cmp3)
                       )
                   );
    __m128i isa2 = _mm_or_si128(
                       _mm_andnot_si128(cmp3, cmp4),
                       _mm_andnot_si128(cmp2, cmp5)
                   );
    __m128i isa3 = _mm_andnot_si128(cmp1, cmp6);
    return _mm_or_si128(
        _mm_andnot_si128(
            _mm_or_si128(
                isa1,
                _mm_or_si128(isa2, isa3)
            ),
            select4Random4(cs, ws, a1, a2, a3, a4)
        ),
        _mm_or_si128(
            _mm_and_si128(isa1, a1),
            _mm_or_si128(
                _mm_and_si128(isa2, a2),
                _mm_and_si128(isa3, a3)
            )
        )
    );
}

#endif

#if defined USE_SIMD && __AVX2__

void mapZoom(Layer *l, int* out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pWidth = (areaWidth>>1)+2, pHeight = (areaHeight>>1)+1;
    
    l->p->getMap(l->p, out, areaX>>1, areaZ>>1, pWidth, pHeight+1);
    
    __m256i (*selectRand)(__m256i* cs, int st, __m256i a1, __m256i a2, __m256i a3, __m256i a4) = (l->p->getMap == mapIsland) ? select8Random4 : select8ModeOrRandom;
    int newWidth = (areaWidth+10)&0xFFFFFFFE;//modified to ignore ends
    int x, z;
    __m256i cs, a, b, a1, b1, toBuf1, toBuf2, aSuf;
    __m256i mask1 = _mm256_setr_epi32(0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0), mask2 = _mm256_setr_epi32(0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF);
    __m256i shuffle = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
    int pX = areaX&0xFFFFFFFE;
    __m256i xs = _mm256_set_epi32(pX+14, pX+12, pX+10, pX+8, pX+6, pX+4, pX+2, pX), zs;
    __m256i v2 = _mm256_set1_epi32(2), v16 = _mm256_set1_epi32(16);
    int* buf = (int*) malloc((newWidth+1)*((areaHeight+2)|1)*sizeof(*buf));
    int* idx = buf;
    int* outIdx = out;    
    //z first!
    for (x = 0; x < pWidth-1; x += 8)
    {
        a = _mm256_loadu_si256((__m256i*)(outIdx));//0, 0
        b = _mm256_loadu_si256((__m256i*)(outIdx+1));//1, 0
        zs = _mm256_set1_epi32(areaZ&0xFFFFFFFE);
        for (z = 0; z < pHeight; z++)
        {
            cs = set8ChunkSeeds(l->startSalt, xs, zs);
            outIdx += pWidth;
            a1 = _mm256_loadu_si256((__m256i*)(outIdx));//0, 1
            b1 = _mm256_loadu_si256((__m256i*)(outIdx+1));//1, 1
            toBuf1 = _mm256_permutevar8x32_epi32(select8Random2(&cs, l->startSalt, a, a1), shuffle);
            toBuf2 = _mm256_permutevar8x32_epi32(select8Random2(&cs, l->startSalt, a, b), shuffle);
            aSuf = _mm256_permutevar8x32_epi32(a, shuffle);
            _mm256_maskstore_epi32(idx, mask1, aSuf);
            _mm256_maskstore_epi32(idx+1, mask1, toBuf2);
            _mm256_maskstore_epi32(idx+7, mask2, aSuf);
            _mm256_maskstore_epi32(idx+8, mask2, toBuf2);
            idx += newWidth;
            toBuf2 = _mm256_permutevar8x32_epi32(selectRand(&cs, l->startSalt, a, b, a1, b1), shuffle);
            _mm256_maskstore_epi32(idx, mask1, toBuf1);
            _mm256_maskstore_epi32(idx+1, mask1, toBuf2);
            _mm256_maskstore_epi32(idx+7, mask2, toBuf1);
            _mm256_maskstore_epi32(idx+8, mask2, toBuf2);
            idx += newWidth;
            a = a1;
            b = b1;
            zs = _mm256_add_epi32(zs, v2);
        }
        outIdx += 8-pHeight*pWidth;        
        idx += 16-pHeight*2*newWidth;    
        xs = _mm256_add_epi32(xs, v16);
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z*areaWidth], &buf[(z + (areaZ & 1))*newWidth + (areaX & 1)], areaWidth*sizeof(int));
    }

    free(buf);
}

#elif defined USE_SIMD && defined __SSE4_2__

void mapZoom(Layer *l, int* out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pWidth = (areaWidth>>1)+2, pHeight = (areaHeight>>1)+1;
    
    l->p->getMap(l->p, out, areaX>>1, areaZ>>1, pWidth, pHeight+1);
    
    __m128i (*selectRand)(__m128i* cs, int st, __m128i a1, __m128i a2, __m128i a3, __m128i a4) = (l->p->getMap == mapIsland) ? select4Random4 : select4ModeOrRandom;
    int newWidth = areaWidth+6&0xFFFFFFFE;//modified to ignore ends
    int x, z;
    __m128i cs, a, b, a1, b1, toBuf1, toBuf2, aSuf;
    __m128i mask1 = _mm_setr_epi32(0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0), mask2 = _mm_setr_epi32(0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF);
    int pX = areaX&0xFFFFFFFE;
    __m128i xs = _mm_set_epi32(pX+6, pX+4, pX+2, pX), zs;
    __m128i v2 = _mm_set1_epi32(2), v8 = _mm_set1_epi32(8);
    int* buf = (int*) malloc((newWidth+1)*(areaHeight+2|1)*sizeof(*buf));
    int* idx = buf;
    int* outIdx = out;
    //z first!
    for (x = 0; x < pWidth-1; x += 4)
    {
        a = _mm_loadu_si128((__m128i_u*)(outIdx));//0, 0
        b = _mm_loadu_si128((__m128i_u*)(outIdx+1));//1, 0
        zs = _mm_set1_epi32(areaZ&0xFFFFFFFE);
        for (z = 0; z < pHeight; z++)
        {
            cs = set4ChunkSeeds(l->startSalt, xs, zs);
            outIdx += pWidth;
            a1 = _mm_loadu_si128((__m128i_u*)(outIdx));//0, 1
            b1 = _mm_loadu_si128((__m128i_u*)(outIdx+1));//1, 1
            toBuf1 = _mm_shuffle_epi32(select4Random2(&cs, l->startSalt, a, a1), 0xD8);//11011000->3120->1324
            toBuf2 = _mm_shuffle_epi32(select4Random2(&cs, l->startSalt, a, b), 0xD8);
            aSuf = _mm_shuffle_epi32(a, 0xD8);
            _mm_maskmoveu_si128(aSuf, mask1, (char*)(idx));
            _mm_maskmoveu_si128(toBuf2, mask1, (char*)(idx+1));
            _mm_maskmoveu_si128(aSuf, mask2, (char*)(idx+3));
            _mm_maskmoveu_si128(toBuf2, mask2, (char*)(idx+4));
            idx += newWidth;
            toBuf2 = _mm_shuffle_epi32(selectRand(&cs, l->startSalt, a, b, a1, b1), 0xD8);
            _mm_maskmoveu_si128(toBuf1, mask1, (char*)(idx));
            _mm_maskmoveu_si128(toBuf2, mask1, (char*)(idx+1));
            _mm_maskmoveu_si128(toBuf1, mask2, (char*)(idx+3));
            _mm_maskmoveu_si128(toBuf2, mask2, (char*)(idx+4));
            idx += newWidth;            
            a = a1;
            b = b1;
            zs = _mm_add_epi32(zs, v2);
        }
        outIdx += 4-pHeight*pWidth;
        idx += 8-pHeight*2*newWidth;
        xs = _mm_add_epi32(xs, v8);
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z*areaWidth], &buf[(z + (areaZ & 1))*newWidth + (areaX & 1)], areaWidth*sizeof(int));
    }

    free(buf);
}

#else

void mapZoom(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x >> 1;
    int pZ = z >> 1;
    int pW = ((x + w) >> 1) - pX + 1;
    int pH = ((z + h) >> 1) - pZ + 1;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int newW = (pW) << 1;
    int newH = (pH) << 1;
    int idx, v00, v01;
    int *buf = (int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const int st = (int)l->startSalt;
    const int ss = (int)l->startSeed;

    int isHighestZoom = l->p->getMap == mapIsland;

    for (j = 0; j < pH; j++)
    {
        idx = (j << 1) * newW;
        v00 = out[(j+0)*pW];
        v01 = out[(j+1)*pW];

        for (i = 0; i < pW; i++)
        {
            int v10 = out[i+1 + (j+0)*pW];
            int v11 = out[i+1 + (j+1)*pW];

            int chunkX = (i + pX) << 1;
            int chunkZ = (j + pZ) << 1;

            int cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = v00;
            buf[idx + newW] = (cs >> 24) & 1 ? v01 : v00;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            buf[idx] = (cs >> 24) & 1 ? v10 : v00;


            if (isHighestZoom)
            {
                //selectRandom4
                cs *= cs * 1284865837 + 4150755663;
                cs += st;
                int r = (cs >> 24) & 3;
                buf[idx + newW] = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
            }
            else
            {
                //selectModeOrRandom
                int v;
                if      (v10 == v01 && v01 == v11) v = v10;
                else if (v00 == v10 && v00 == v01) v = v00;
                else if (v00 == v10 && v00 == v11) v = v00;
                else if (v00 == v01 && v00 == v11) v = v00;
                else if (v00 == v10 && v01 != v11) v = v00;
                else if (v00 == v01 && v10 != v11) v = v00;
                else if (v00 == v11 && v10 != v01) v = v00;
                else if (v10 == v01 && v00 != v11) v = v10;
                else if (v10 == v11 && v00 != v01) v = v10;
                else if (v01 == v11 && v00 != v10) v = v01;
                else
                {
                    cs *= cs * 1284865837 + 4150755663;
                    cs += st;
                    int r = (cs >> 24) & 3;
                    v = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
                }
                buf[idx + newW] = v;
            }

            idx++;
            v00 = v10;
            v01 = v11;
        }
    }

    for (j = 0; j < h; j++)
    {
        memcpy(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }

    free(buf);
}
#endif


void mapAddIsland(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v00 = out[i+0 + (j+0)*pW];
            int v20 = out[i+2 + (j+0)*pW];
            int v02 = out[i+0 + (j+2)*pW];
            int v22 = out[i+2 + (j+2)*pW];
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == 0 && (v00 != 0 || v20 != 0 || v02 != 0 || v22 != 0))
            {
                cs = getChunkSeed(ss, i+x, j+z);

                int v = 1;
                int inc = 0;

                if (v00 != 0)
                {
                    ++inc; v = v00;
                    cs = mcStepSeed(cs, st);
                }
                if (v20 != 0)
                {
                    if (++inc == 1 || (cs & (1LL << 24)) == 0) v = v20;
                    cs = mcStepSeed(cs, st);
                }
                if (v02 != 0)
                {
                    switch(++inc)
                    {
                    case 1: v = v02; break;
                    case 2: if ((cs & (1LL << 24)) == 0) v = v02; break;
                    default: if (((cs >> 24) % 3) == 0) v = v02;
                    }
                    cs = mcStepSeed(cs, st);
                }
                if (v22 != 0)
                {
                    switch(++inc)
                    {
                    case 1: v = v22; break;
                    case 2: if ((cs & (1LL << 24)) == 0) v = v22; break;
                    case 3: if (((cs >> 24) % 3) == 0) v = v22; break;
                    default: if ((cs & (3LL << 24)) == 0) v = v22;
                    }
                    cs = mcStepSeed(cs, st);
                }

                if ((cs >> 24) % 3 == 0)
                    out[i + j*w] = v;
                else if (v == 4)
                    out[i + j*w] = 4;
                else
                    out[i + j*w] = 0;
            }
            else if (v11 > 0 && (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0))
            {
                cs = getChunkSeed(ss, i+x, j+z);

                if ((cs >> 24) % 5 == 0)
                    out[i + j*w] = (v11 == 4) ? 4 : 0;
                else
                    out[i + j*w] = v11;
            }
            else
            {
                out[i + j*w] = v11;
            }
        }
    }
}


void mapRemoveTooMuchOcean(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            out[i + j*w] = v11;

            if (out[i+1 + (j+0)*pW] != 0) continue;
            if (out[i+2 + (j+1)*pW] != 0) continue;
            if (out[i+0 + (j+1)*pW] != 0) continue;
            if (out[i+1 + (j+2)*pW] != 0) continue;

            if (v11 == 0)
            {
                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 2))
                {
                    out[i + j*w] = 1;
                }
            }
        }
    }
}


void mapAddSnow(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);
    
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (isShallowOcean(v11))
            {
                out[i + j*w] = v11;
            }
            else
            {
                cs = getChunkSeed(ss, i+x, j+z);
                int r = mcFirstInt(cs, 6);
                int v;

                if      (r == 0) v = 4;
                else if (r <= 1) v = 3;
                else             v = 1;

                out[i + j*w] = v;
            }
        }
    }
}


void mapCoolWarm(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == 1)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (v10 == 3 || v10 == 4 || v21 == 3 || v21 == 4 || v01 == 3 || v01 == 4 || v12 == 3 || v12 == 4)
                {
                    v11 = 2;
                }
            }

            out[i + j*w] = v11;
        }
    }
}


void mapHeatIce(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == 4)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (v10 == 1 || v10 == 2 || v21 == 1 || v21 == 2 || v01 == 1 || v01 == 2 || v12 == 1 || v12 == 2)
                {
                    v11 = 3;
                }
            }

            out[i + j*w] = v11;
        }
    }
}


void mapSpecial(const Layer * l, int * out, int x, int z, int w, int h)
{
    l->p->getMap(l->p, out, x, z, w, h);

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j*w];
            if (v == 0) continue;

            cs = getChunkSeed(ss, i+x, j+z);

            if (mcFirstIsZero(cs, 13))
            {
                cs = mcStepSeed(cs, st);
                v |= (1 + mcFirstInt(cs, 15)) << 8 & 0xf00;
                // 1 to 1 mapping so 'out' can be overwritten immediately
                out[i + j*w] = v;
            }
        }
    }
}


void mapAddMushroomIsland(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            // surrounded by ocean?
            if (v11 == 0 && !out[i+0 + (j+0)*pW] && !out[i+2 + (j+0)*pW] && !out[i+0 + (j+2)*pW] && !out[i+2 + (j+2)*pW])
            {
                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 100))
                {
                    out[i + j*w] = mushroom_fields;
                    continue;
                }
            }

            out[i + j*w] = v11;
        }
    }
}


void mapDeepOcean(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[(i+1) + (j+1)*pW];

            if (isShallowOcean(v11))
            {
                // count adjacent oceans
                int oceans = 0;
                if (isShallowOcean(out[(i+1) + (j+0)*pW])) oceans++;
                if (isShallowOcean(out[(i+2) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+0) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+1) + (j+2)*pW])) oceans++;

                if (oceans > 3)
                {
                    switch (v11)
                    {
                    case warm_ocean:
                        v11 = deep_warm_ocean;
                        break;
                    case lukewarm_ocean:
                        v11 = deep_lukewarm_ocean;
                        break;
                    case ocean:
                        v11 = deep_ocean;
                        break;
                    case cold_ocean:
                        v11 = deep_cold_ocean;
                        break;
                    case frozen_ocean:
                        v11 = deep_frozen_ocean;
                        break;
                    default:
                        v11 = deep_ocean;
                    }
                }
            }

            out[i + j*w] = v11;
        }
    }
}


const int warmBiomes[] = {desert, desert, desert, savanna, savanna, plains};
const int lushBiomes[] = {forest, dark_forest, mountains, plains, birch_forest, swamp};
const int coldBiomes[] = {forest, mountains, taiga, plains};
const int snowBiomes[] = {snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga};

void mapBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    l->p->getMap(l->p, out, x, z, w, h);

    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int idx = i + j*w;
            int id = out[idx];
            int hasHighBit = (id & 0xf00) >> 8;
            id &= -0xf01;

            if (getBiomeType(id) == Ocean || id == mushroom_fields)
            {
                out[idx] = id;
                continue;
            }

            cs = getChunkSeed(ss, i + x, j + z);

            switch(id){
            case Warm:
                if (hasHighBit) out[idx] = mcFirstIsZero(cs, 3) ? badlands_plateau : wooded_badlands_plateau;
                else out[idx] = warmBiomes[mcFirstInt(cs, 6)];
                break;
            case Lush:
                if (hasHighBit) out[idx] = jungle;
                else out[idx] = lushBiomes[mcFirstInt(cs, 6)];
                break;
            case Cold:
                if (hasHighBit) out[idx] = giant_tree_taiga;
                else out[idx] = coldBiomes[mcFirstInt(cs, 4)];
                break;
            case Freezing:
                out[idx] = snowBiomes[mcFirstInt(cs, 4)];
                break;
            default:
                out[idx] = mushroom_fields;
            }
        }
    }
}


const int lushBiomesBE[] = {forest, dark_forest, mountains, plains, plains, plains, birch_forest, swamp};

void mapBiomeBE(const Layer * l, int * out, int x, int z, int w, int h)
{
    l->p->getMap(l->p, out, x, z, w, h);

    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int idx = i + j*w;
            int id = out[idx];
            int hasHighBit = (id & 0xf00) >> 8;
            id &= -0xf01;

            if (getBiomeType(id) == Ocean || id == mushroom_fields)
            {
                out[idx] = id;
                continue;
            }

            cs = getChunkSeed(ss, i + x, j + z);

            switch(id){
                case Warm:
                    if (hasHighBit) out[idx] = mcFirstIsZero(cs, 3) ? badlands_plateau : wooded_badlands_plateau;
                    else out[idx] = warmBiomes[mcFirstInt(cs, 6)];
                    break;
                case Lush:
                    if (hasHighBit) out[idx] = jungle;
                    else out[idx] = lushBiomesBE[mcFirstInt(cs, 6)];
                    break;
                case Cold:
                    if (hasHighBit) out[idx] = giant_tree_taiga;
                    else out[idx] = coldBiomes[mcFirstInt(cs, 4)];
                    break;
                case Freezing:
                    out[idx] = snowBiomes[mcFirstInt(cs, 4)];
                    break;
                default:
                    out[idx] = mushroom_fields;
            }
        }
    }
}


void mapRiverInit(const Layer * l, int * out, int x, int z, int w, int h)
{
    l->p->getMap(l->p, out, x, z, w, h);

    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            if (out[i + j*w] > 0)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                out[i + j*w] = mcFirstInt(cs, 299999)+2;
            }
            else
            {
                out[i + j*w] = 0;
            }
        }
    }
}


void mapAddBamboo(const Layer * l, int * out, int x, int z, int w, int h)
{
    l->p->getMap(l->p, out, x, z, w, h);

    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int idx = i + j*w;
            if (out[idx] != jungle) continue;

            cs = getChunkSeed(ss, i + x, j + z);
            if (mcFirstIsZero(cs, 10))
            {
                out[idx] = bamboo_jungle;
            }
        }
    }
}


static inline int replaceEdge(int *out, int idx, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID)
{
    if (id != baseID) return 0;

    // areSimilar() has not changed behaviour for ids < 128, so use the faster variant
    if (areSimilar113(v10, baseID) && areSimilar113(v21, baseID) &&
        areSimilar113(v01, baseID) && areSimilar113(v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}

void mapBiomeEdge(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            if (!replaceEdge(out, i + j*w, v10, v21, v01, v12, v11, wooded_badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, v10, v21, v01, v12, v11, badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, v10, v21, v01, v12, v11, giant_tree_taiga, taiga))
            {
                if (v11 == desert)
                {
                    if (v10 != snowy_tundra && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
                    {
                        out[i + j*w] = v11;
                    }
                    else
                    {
                        out[i + j*w] = wooded_mountains;
                    }
                }
                else if (v11 == swamp)
                {
                    if (v10 != desert && v21 != desert && v01 != desert && v12 != desert &&
                        v10 != snowy_taiga && v21 != snowy_taiga && v01 != snowy_taiga && v12 != snowy_taiga &&
                        v10 != snowy_tundra && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
                    {
                        if (v10 != jungle && v12 != jungle && v21 != jungle && v01 != jungle &&
                            v10 != bamboo_jungle && v12 != bamboo_jungle &&
                            v21 != bamboo_jungle && v01 != bamboo_jungle)
                            out[i + j*w] = v11;
                        else
                            out[i + j*w] = jungleEdge;
                    }
                    else
                    {
                        out[i + j*w] = plains;
                    }
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
        }
    }
}


void mapHills(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;
    int *buf = NULL;

    if (l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    buf = (int *) malloc(pW*pH*sizeof(int));

    l->p->getMap(l->p, out, pX, pZ, pW, pH);
    memcpy(buf, out, pW*pH*sizeof(int));

    l->p2->getMap(l->p2, out, pX, pZ, pW, pH);

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int a11 = buf[i+1 + (j+1)*pW]; // biome branch
            int b11 = out[i+1 + (j+1)*pW]; // river branch
            int idx = i + j*w;

            int var12 = (b11 - 2) % 29 == 0;

            if (a11 != 0 && b11 >= 2 && (b11 - 2) % 29 == 1 && a11 < 128)
            {
                out[idx] = (biomeExists(a11 + 128)) ? a11 + 128 : a11;
            }
            else
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (mcFirstIsZero(cs, 3) && !var12)
                {
                    out[idx] = a11;
                }
                else
                {
                    int hillID = a11;

                    switch(a11)
                    {
                    case desert:
                        hillID = desert_hills; break;
                    case forest:
                        hillID = wooded_hills; break;
                    case birch_forest:
                        hillID = birch_forest_hills; break;
                    case dark_forest:
                        hillID = plains; break;
                    case taiga:
                        hillID = taiga_hills; break;
                    case giant_tree_taiga:
                        hillID = giant_tree_taiga_hills; break;
                    case snowy_taiga:
                        hillID = snowy_taiga_hills; break;
                    case plains:
                        cs = mcStepSeed(cs, st);
                        hillID = mcFirstIsZero(cs, 3) ? wooded_hills : forest; break;
                    case snowy_tundra:
                        hillID = snowy_mountains; break;
                    case jungle:
                        hillID = jungle_hills; break;
                    case ocean:
                        hillID = deep_ocean; break;
                    case mountains:
                        hillID = wooded_mountains; break;
                    case savanna:
                        hillID = savanna_plateau; break;
                    default:
                        if (areSimilar(a11, wooded_badlands_plateau))
                            hillID = badlands;
                        else if (a11 == deep_ocean && mcFirstIsZero(cs = mcStepSeed(cs, st), 3))
                            hillID = mcFirstIsZero(mcStepSeed(cs, st), 2) ? plains : forest;
                        break;
                    }

                    if (var12 && hillID != a11)
                    {
                        if (biomeExists(hillID + 128))
                            hillID += 128;
                        else
                            hillID = a11;
                    }

                    if (hillID == a11)
                    {
                        out[idx] = a11;
                    }
                    else
                    {
                        int a10 = buf[i+1 + (j+0)*pW];
                        int a21 = buf[i+2 + (j+1)*pW];
                        int a01 = buf[i+0 + (j+1)*pW];
                        int a12 = buf[i+1 + (j+2)*pW];
                        int equals = 0;

                        if (areSimilar(a10, a11)) equals++;
                        if (areSimilar(a21, a11)) equals++;
                        if (areSimilar(a01, a11)) equals++;
                        if (areSimilar(a12, a11)) equals++;

                        if (equals >= 3)
                            out[idx] = hillID;
                        else
                            out[idx] = a11;
                    }
                }
            }
        }
    }

    free(buf);
}


void mapHills113(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;
    int *buf = NULL;

    if (l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    buf = (int *) malloc(pW*pH*sizeof(int));

    l->p->getMap(l->p, out, pX, pZ, pW, pH);
    memcpy(buf, out, pW*pH*sizeof(int));

    l->p2->getMap(l->p2, out, pX, pZ, pW, pH);

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int a11 = buf[i+1 + (j+1)*pW]; // biome branch
            int b11 = out[i+1 + (j+1)*pW]; // river branch
            int idx = i + j*w;

            int bn = (b11 - 2) % 29;

            if (!isShallowOcean(a11) && b11 >= 2 && bn == 1)
            {
                int m = biomes[a11].mutated;
                if (m > 0)
                    out[idx] = m;
                else
                    out[idx] = a11;
            }
            else
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (bn == 0 || mcFirstIsZero(cs, 3))
                {
                    int hillID = a11;

                    switch(a11)
                    {
                    case desert:
                        hillID = desert_hills; break;
                    case forest:
                        hillID = wooded_hills; break;
                    case birch_forest:
                        hillID = birch_forest_hills; break;
                    case dark_forest:
                        hillID = plains; break;
                    case taiga:
                        hillID = taiga_hills; break;
                    case giant_tree_taiga:
                        hillID = giant_tree_taiga_hills; break;
                    case snowy_taiga:
                        hillID = snowy_taiga_hills; break;
                    case plains:
                        cs = mcStepSeed(cs, st);
                        hillID = mcFirstIsZero(cs, 3) ? wooded_hills : forest; break;
                    case snowy_tundra:
                        hillID = snowy_mountains; break;
                    case jungle:
                        hillID = jungle_hills; break;
                    case bamboo_jungle:
                        hillID = bamboo_jungle_hills; break;
                    case ocean:
                        hillID = deep_ocean; break;
                    case mountains:
                        hillID = wooded_mountains; break;
                    case savanna:
                        hillID = savanna_plateau; break;
                    default:
                        if (areSimilar113(a11, wooded_badlands_plateau))
                            hillID = badlands;
                        else if (isDeepOcean(a11))
                        {
                            cs = mcStepSeed(cs, st);
                            if (mcFirstIsZero(cs, 3))
                            {
                                cs = mcStepSeed(cs, st);
                                hillID = mcFirstIsZero(cs, 2) ? plains : forest;
                            }
                        }
                        break;
                    }

                    if (bn == 0 && hillID != a11)
                    {
                        hillID = biomes[hillID].mutated;
                        if (hillID < 0)
                            hillID = a11;
                    }

                    if (hillID != a11)
                    {
                        int a10 = buf[i+1 + (j+0)*pW];
                        int a21 = buf[i+2 + (j+1)*pW];
                        int a01 = buf[i+0 + (j+1)*pW];
                        int a12 = buf[i+1 + (j+2)*pW];
                        int equals = 0;

                        if (areSimilar113(a10, a11)) equals++;
                        if (areSimilar113(a21, a11)) equals++;
                        if (areSimilar113(a01, a11)) equals++;
                        if (areSimilar113(a12, a11)) equals++;

                        if (equals >= 3)
                            out[idx] = hillID;
                        else
                            out[idx] = a11;
                    }
                    else
                    {
                        out[idx] = a11;
                    }
                }
                else
                {
                    out[idx] = a11;
                }
            }
        }
    }

    free(buf);
}



static inline int reduceID(int id)
{
    return id >= 2 ? 2 + (id & 1) : id;
}

void mapRiver(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = reduceID(vz1[i+1]);

            if (v11 == reduceID(vz1[i+0]) && v11 == reduceID(vz1[i+2]) &&
                v11 == reduceID(vz0[i+1]) && v11 == reduceID(vz2[i+1]))
            {
                out[i + j * w] = -1;
            }
            else
            {
                out[i + j * w] = river;
            }
        }
    }
}


void mapSmooth(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v01 = vz1[i+0];
            int v10 = vz0[i+1];

            if (v11 != v01 || v11 != v10)
            {
                int v21 = vz1[i+2];
                int v12 = vz2[i+1];
                if (v01 == v21 && v10 == v12)
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (cs & ((int64_t)1 << 24))
                        v11 = v10;
                    else
                        v11 = v01;
                }
                else
                {
                    if (v01 == v21) v11 = v01;
                    if (v10 == v12) v11 = v10;
                }
            }

            out[i + j * w] = v11;
        }
    }
}


void mapRareBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    int i, j;

    l->p->getMap(l->p, out, x, z, w, h);

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j * w];

            if (v == plains)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (mcFirstIsZero(cs, 57))
                {
                    // Sunflower Plains
                    out[i + j*w] = plains + 128;
                }
            }
        }
    }
}


inline static int replaceOcean(int *out, int idx, int v10, int v21, int v01, int v12, int id, int replaceID)
{
    if (isOceanic(id)) return 0;

    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
        out[idx] = id;
    else
        out[idx] = replaceID;

    return 1;
}

inline static int isBiomeJFTO(int id)
{
    return biomeExists(id) && (getBiomeType(id) == Jungle || id == forest || id == taiga || isOceanic(id));
}

void mapShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            int biome = biomeExists(v11) ? v11 : 0;

            if (v11 == mushroom_fields)
            {
                if (v10 != ocean && v21 != ocean && v01 != ocean && v12 != ocean)
                    out[i + j*w] = v11;
                else
                    out[i + j*w] = mushroom_field_shore;
            }
            else if (/*biome < 128 &&*/ getBiomeType(biome) == Jungle)
            {
                if (isBiomeJFTO(v10) && isBiomeJFTO(v21) && isBiomeJFTO(v01) && isBiomeJFTO(v12))
                {
                    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                        out[i + j*w] = v11;
                    else
                        out[i + j*w] = beach;
                }
                else
                {
                    out[i + j*w] = jungleEdge;
                }
            }
            else if (v11 != mountains && v11 != wooded_mountains && v11 != mountain_edge)
            {
                if (isBiomeSnowy(biome))
                {
                    replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, snowy_beach);
                }
                else if (v11 != badlands && v11 != wooded_badlands_plateau)
                {
                    if (v11 != ocean && v11 != deep_ocean && v11 != river && v11 != swamp)
                    {
                        if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                            out[i + j*w] = v11;
                        else
                            out[i + j*w] = beach;
                    }
                    else
                    {
                        out[i + j*w] = v11;
                    }
                }
                else
                {
                    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                    {
                        if (getBiomeType(v10) == Mesa && getBiomeType(v21) == Mesa && getBiomeType(v01) == Mesa && getBiomeType(v12) == Mesa)
                            out[i + j*w] = v11;
                        else
                            out[i + j*w] = desert;
                    }
                    else
                    {
                        out[i + j*w] = v11;
                    }
                }
            }
            else
            {
                replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, stone_shore);
            }
        }
    }
}


void mapRiverMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    int idx;
    int len;
    int *buf;

    if (l->p2 == NULL)
    {
        printf("mapRiverMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    len = w*h;
    buf = (int *) malloc(len*sizeof(int));

    l->p->getMap(l->p, out, x, z, w, h); // biome chain
    memcpy(buf, out, len*sizeof(int));

    l->p2->getMap(l->p2, out, x, z, w, h); // rivers

    for (idx = 0; idx < len; idx++)
    {
        if (isOceanic(buf[idx]))
        {
            out[idx] = buf[idx];
        }
        else
        {
            if (out[idx] == river)
            {
                if (buf[idx] == snowy_tundra)
                    out[idx] = frozen_river;
                else if (buf[idx] == mushroom_fields || buf[idx] == mushroom_field_shore)
                    out[idx] = mushroom_field_shore;
                else
                    out[idx] = out[idx] & 255;
            }
            else
            {
                out[idx] = buf[idx];
            }
        }
    }

    free(buf);
}



/* Initialises data for the ocean temperature types using the world seed.
 * This function is called when the world seed is applied in setWorldSeed().
 */
static void oceanRndInit(OceanRnd *rnd, int64_t seed)
{
    int i = 0;
    memset(rnd, 0, sizeof(*rnd));
    setSeed(&seed);
    rnd->a = nextDouble(&seed) * 256.0;
    rnd->b = nextDouble(&seed) * 256.0;
    rnd->c = nextDouble(&seed) * 256.0;

    for (i = 0; i < 256; i++)
    {
        rnd->d[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        int n3 = nextInt(&seed, 256 - i) + i;
        int n4 = rnd->d[i];
        rnd->d[i] = rnd->d[n3];
        rnd->d[n3] = n4;
        rnd->d[i + 256] = rnd->d[i];
    }
}

static double lerp(const double part, const double from, const double to)
{
    return from + part * (to - from);
}

/* Table of vectors to cube edge centres (12 + 4 extra), used for ocean PRNG */
const double cEdgeX[] = {1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 0.0, 0.0, 0.0, 0.0,  1.0, 0.0,-1.0, 0.0};
const double cEdgeY[] = {1.0, 1.0,-1.0,-1.0, 0.0, 0.0, 0.0, 0.0, 1.0,-1.0, 1.0,-1.0,  1.0,-1.0, 1.0,-1.0};
const double cEdgeZ[] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0,-1.0,-1.0,  0.0, 1.0, 0.0,-1.0};

static double indexedLerp(int idx, const double d1, const double d2, const double d3)
{
    idx &= 0xf;
    return cEdgeX[idx] * d1 + cEdgeY[idx] * d2 + cEdgeZ[idx] * d3;
}


static double getOceanTemp(const OceanRnd *rnd, double d1, double d2, double d3)
{
    d1 += rnd->a;
    d2 += rnd->b;
    d3 += rnd->c;
    int i1 = (int)d1 - (int)(d1 < 0);
    int i2 = (int)d2 - (int)(d2 < 0);
    int i3 = (int)d3 - (int)(d3 < 0);
    d1 -= i1;
    d2 -= i2;
    d3 -= i3;
    double t1 = d1*d1*d1 * (d1 * (d1*6.0-15.0) + 10.0);
    double t2 = d2*d2*d2 * (d2 * (d2*6.0-15.0) + 10.0);
    double t3 = d3*d3*d3 * (d3 * (d3*6.0-15.0) + 10.0);

    i1 &= 0xff;
    i2 &= 0xff;
    i3 &= 0xff;

    int a1 = rnd->d[i1]   + i2;
    int a2 = rnd->d[a1]   + i3;
    int a3 = rnd->d[a1+1] + i3;
    int b1 = rnd->d[i1+1] + i2;
    int b2 = rnd->d[b1]   + i3;
    int b3 = rnd->d[b1+1] + i3;

    double l1 = indexedLerp(rnd->d[a2],   d1,   d2,   d3);
    double l2 = indexedLerp(rnd->d[b2],   d1-1, d2,   d3);
    double l3 = indexedLerp(rnd->d[a3],   d1,   d2-1, d3);
    double l4 = indexedLerp(rnd->d[b3],   d1-1, d2-1, d3);
    double l5 = indexedLerp(rnd->d[a2+1], d1,   d2,   d3-1);
    double l6 = indexedLerp(rnd->d[b2+1], d1-1, d2,   d3-1);
    double l7 = indexedLerp(rnd->d[a3+1], d1,   d2-1, d3-1);
    double l8 = indexedLerp(rnd->d[b3+1], d1-1, d2-1, d3-1);

    l1 = lerp(t1, l1, l2);
    l3 = lerp(t1, l3, l4);
    l5 = lerp(t1, l5, l6);
    l7 = lerp(t1, l7, l8);

    l1 = lerp(t2, l1, l3);
    l5 = lerp(t2, l5, l7);

    return lerp(t3, l1, l5);
}

void mapOceanTemp(const Layer * l, int * out, int x, int z, int w, int h)
{
    int i, j;
    OceanRnd *rnd = l->oceanRnd;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            double tmp = getOceanTemp(rnd, (i + x) / 8.0, (j + z) / 8.0, 0);

            if (tmp > 0.4)
                out[i + j*w] = warm_ocean;
            else if (tmp > 0.2)
                out[i + j*w] = lukewarm_ocean;
            else if (tmp < -0.4)
                out[i + j*w] = frozen_ocean;
            else if (tmp < -0.2)
                out[i + j*w] = cold_ocean;
            else
                out[i + j*w] = ocean;
        }
    }
}

/* Warning: this function is horribly slow compared to other layers! */
void mapOceanMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    int landX = x -  8, landZ = z -  8;
    int landW = w + 17, landH = h + 17;
    int *map1, *map2;

    if (l->p2 == NULL)
    {
        printf("mapOceanMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    l->p->getMap(l->p, out, landX, landZ, landW, landH);
    map1 = (int *) malloc(landW*landH*sizeof(int));
    memcpy(map1, out, landW*landH*sizeof(int));

    l->p2->getMap(l->p2, out, x, z, w, h);
    map2 = (int *) malloc(w*h*sizeof(int));
    memcpy(map2, out, w*h*sizeof(int));


    int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int landID = map1[(i+8) + (j+8)*landW];
            int oceanID = map2[i + j*w];

            if (!isOceanic(landID))
            {
                out[i + j*w] = landID;
                continue;
            }

            int ii, jj;
            for (ii = -8; ii <= 8; ii += 4)
            {
                for (jj = -8; jj <= 8; jj += 4)
                {
                    int nearbyID = map1[(i+ii+8) + (j+jj+8)*landW];

                    if (isOceanic(nearbyID)) continue;

                    if (oceanID == warm_ocean)
                    {
                        out[i + j*w] = lukewarm_ocean;
                        goto loop_x;
                    }

                    if (oceanID == frozen_ocean)
                    {
                        out[i + j*w] = cold_ocean;
                        goto loop_x;
                    }
                }
            }

            if (landID == deep_ocean)
            {
                switch (oceanID)
                {
                case lukewarm_ocean:
                    oceanID = deep_lukewarm_ocean;
                    break;
                case ocean:
                    oceanID = deep_ocean;
                    break;
                case cold_ocean:
                    oceanID = deep_cold_ocean;
                    break;
                case frozen_ocean:
                    oceanID = deep_frozen_ocean;
                    break;
                }
            }

            out[i + j*w] = oceanID;

            loop_x:;
        }
    }

    free(map1);
    free(map2);
}


void mapVoronoiZoom(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int pX = x >> 2;
    int pZ = z >> 2;
    int pW = (w >> 2) + 2;
    int pH = (h >> 2) + 2;
    int newW = (pW-1) << 2;
    int newH = (pH-1) << 2;
    int i, j;
    int *buf = (int *)malloc((newW+1)*(newH+1)*sizeof(*buf));

    l->p->getMap(l->p, out, pX, pZ, pW, pH);

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < pH - 1; j++)
    {
        int v00 = out[(j+0)*pW];
        int v01 = out[(j+1)*pW];

        for (i = 0; i < pW - 1; i++)
        {
            cs = getChunkSeed(ss, (i+pX) << 2, (j+pZ) << 2);
            int64_t da1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            int64_t da2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (i+pX+1) << 2, (j+pZ) << 2);
            int64_t db1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            int64_t db2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (i+pX) << 2, (j+pZ+1) << 2);
            int64_t dc1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            int64_t dc2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            cs = getChunkSeed(ss, (i+pX+1) << 2, (j+pZ+1) << 2);
            int64_t dd1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            int64_t dd2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            int v10 = out[i+1 + (j+0)*pW] & 255;
            int v11 = out[i+1 + (j+1)*pW] & 255;

            int ii, jj;

            for (jj = 0; jj < 4; jj++)
            {
                int idx = ((j << 2) + jj) * newW + (i << 2);

                for (ii = 0; ii < 4; ii++)
                {
                    int mi = 10240*ii;
                    int mj = 10240*jj;

                    int64_t da = (mj-da2)*(mj-da2) + (mi-da1)*(mi-da1);
                    int64_t db = (mj-db2)*(mj-db2) + (mi-db1)*(mi-db1);
                    int64_t dc = (mj-dc2)*(mj-dc2) + (mi-dc1)*(mi-dc1);
                    int64_t dd = (mj-dd2)*(mj-dd2) + (mi-dd1)*(mi-dd1);

                    if (da < db && da < dc && da < dd)
                    {
                        buf[idx++] = v00;
                    }
                    else if (db < da && db < dc && db < dd)
                    {
                        buf[idx++] = v10;
                    }
                    else if (dc < da && dc < db && dc < dd)
                    {
                        buf[idx++] = v01;
                    }
                    else
                    {
                        buf[idx++] = v11;
                    }
                }
            }

            v00 = v10;
            v01 = v11;
        }
    }

    for (j = 0; j < h; j++)
    {
        memcpy(&out[j * w], &buf[(j + (z & 3))*newW + (x & 3)], w*sizeof(int));
    }

    free(buf);
}

