#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "generator.h"
#include "layers.h"
#include "nullspace_finders.h"

#define DEFAULT_SEARCH 64
#define DEFAULT_SHIFT 4
#define DEFAULT_OUT_LIST "./seeds/final_seeds.csv"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int search;
    int shift;
    int64_t *seeds;
    int64_t scnt;
    FILE *out;
} thread_info;

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

void writeSeed(FILE *out, int tid, int64_t seed,
	int dx, int dz, Pos hut_1_7, Pos hut_1_8, Pos mon) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif
    
    printf("Thread %d: %ld 1.7 Hut: (%d,%d) 1.8 Hut: (%d,%d) "
	"Monument: (%d,%d)\n", tid, seed, hut_1_7.x + dx, hut_1_7.z + dz,
	hut_1_8.x + dx, hut_1_8.z + dz, mon.x + dx, mon.z + dz);

    fprintf(out, "%ld,%d,%d,%d,%d,%d,%d\n", seed,
	hut_1_7.x + dx, hut_1_7.z + dz, hut_1_8.x + dx, hut_1_8.z + dz,
	mon.x + dx, mon.z + dz);

#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif
}

#define addIfMonCluster(A,B) do {				\
    if(getMinRadius2(						\
	spos_mon[A],						\
	spos_mon[B],						\
	58+1, 39, 58+1						\
    ) <= 128) {							\
	++(*moncnt);						\
	if(*moncnt > *mons_size) {				\
	    *mons_size *= 2;					\
	    *mon_0 = realloc(*mon_0, sizeof(Pos) * *mons_size);	\
	    *mon_1 = realloc(*mon_1, sizeof(Pos) * *mons_size);	\
	}							\
	(*mon_0)[*moncnt - 1] = spos_mon[A];			\
	(*mon_1)[*moncnt - 1] = spos_mon[B];			\
	ret &= 0x3;						\
    }								\
} while(0)

/* Sets the positions for the 1.7 triple huts, 1.8 quad huts and double
 * ocean monuments to those of the given seed. If the seed does not
 * contain any one of those, the appropiate bit of the return value is
 * set. (1s place for 1.7 triple huts, 2s place for 1.8 quad huts and 4s
 * place for double ocean monuments) Will return 0 if all
 * necessary clusters are located.
 */
int setStructurePositions(int64_t seed, int search,
	int *hut_1_7_exclude, Pos *hut_1_7, Pos *hut_1_8,
	int *mons_size, Pos **mon_0, Pos **mon_1, int *moncnt) {
    const int spos_dim = (2 * search) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);
    int ret = 0x7;

    int spos_idx = 0;
    for(int x = -search; x <= search; ++x) {
	for(int z = -search; z <= search; ++z, ++spos_idx) {
	    spos_feat[spos_idx] = getFeaturePos(
		FEATURE_CONFIG, seed, x, z
	    );
	    spos_mon[spos_idx] = getLargeStructurePos(
		MONUMENT_CONFIG, seed, x, z
	    );
	}
    }

    spos_idx = 0;
    for(int x = -search; x < search; ++x, ++spos_idx) {
	for(int z = -search; z < search; ++z, ++spos_idx) {
	    // get hut positions
	    int cmin = (x == 0 && z == 0) ? 3 : 4;
	    int ay = (x == 0 && z == 0) ? 5+43+1 : 7+43+1;
	    int csize = getClusterSize(
		spos_feat[spos_idx],
		spos_feat[spos_idx + spos_dim],
		spos_feat[spos_idx + 1],
		spos_feat[spos_idx + spos_dim + 1],
		7+1, ay, 9+1, cmin
	    );
	    if(x == 0 && z == 0 && csize >= 3) {
		*hut_1_7_exclude = 0xf;
		hut_1_7[0] = spos_feat[spos_idx];
		hut_1_7[1] = spos_feat[spos_idx + spos_dim];
		hut_1_7[2] = spos_feat[spos_idx + 1];
		hut_1_7[3] = spos_feat[spos_idx + spos_dim + 1];
		if(csize == 3) {
		    // determine which triple huts to check
		    *hut_1_7_exclude = 0;
		    for(int o = 0; o < 4; ++o) {
			float r = getMinRadius3(
			    hut_1_7[(1 + o) % 4],
			    hut_1_7[(2 + o) % 4],
			    hut_1_7[(3 + o) % 4],
			    7+1, ay, 9+1, 128
			);
			if(r <= 128) {
			    *hut_1_7_exclude |= 1<<o;
			}
		    }
		}

		ret &= 0x6;
	    } else if(csize == 4) {
		hut_1_8[0] = spos_feat[spos_idx];
		hut_1_8[1] = spos_feat[spos_idx + spos_dim];
		hut_1_8[2] = spos_feat[spos_idx + 1];
		hut_1_8[3] = spos_feat[spos_idx + spos_dim + 1];

		ret &= 0x5;
	    }
	    
	    // get monument positions
	    addIfMonCluster(spos_idx, spos_idx + spos_dim + 1);
	    addIfMonCluster(spos_idx + 1, spos_idx + spos_dim);
	    addIfMonCluster(spos_idx, spos_idx + 1);
	    addIfMonCluster(spos_idx, spos_idx + spos_dim);
	}
    }

    free(spos_feat);
    free(spos_mon);

    return ret;
}

#ifdef USE_PTHREAD
void *findMultiBasesThread(void *arg) {
#else
DWORD WINAPI findMultiBasesThread(LPVOID arg) {
#endif
    thread_info *info = (thread_info*)arg;
    int tid = info->tid;
    int search = info->search;
    int shift = info->shift;
    int64_t *seeds = info->seeds;
    int64_t scnt = info->scnt;
    FILE *out = info->out;

    LayerStack g;
    setupGenerator(&g, MC_1_7);
    Layer *lFilterBiome = &g.layers[L_BIOME_256];
    int *biomeCache = allocCache(lFilterBiome, 3, 3);
    int64_t lsBiome = g.layers[L_BIOME_256].layerSeed;

    // The lowest 4 bits of hut_1_7_exclude indicate which hut triplets
    // to check, as denoted by the hut excluded from each triplet.
    int hut_1_7_exclude;
    Pos *hut_1_7 = malloc(sizeof(Pos) * 4);
    Pos *hut_1_8 = malloc(sizeof(Pos) * 4);
    int mons_size = 16;
    Pos *mon_0 = malloc(sizeof(Pos) * mons_size);
    Pos *mon_1 = malloc(sizeof(Pos) * mons_size);
    int moncnt;

    for(int64_t s = 0; s < scnt; ++s) {
	int64_t base = seeds[s];

	// search for structure clusters
	moncnt = 0;
	if(setStructurePositions(base, search,
	    &hut_1_7_exclude, hut_1_7, hut_1_8,
	    &mons_size, &mon_0, &mon_1, &moncnt
	)) continue;

	for(int x = -shift; x <= shift; ++x) {
	    for(int z = -shift; z <= shift; ++z) {
		int64_t seed;
		int64_t tbase = moveStructure(base, x, z);
		int dx = x << 9;
		int dz = z << 9;
		
		int areaX_1_8 = ((hut_1_8[0].x + dx) >> 8) | 1;
		int areaZ_1_8 = ((hut_1_8[0].z + dz) >> 8) | 1;
		int areaX_1_7 = ((hut_1_7[0].x + dx) >> 8) | 1;
		int areaZ_1_7 = ((hut_1_7[0].z + dz) >> 8) | 1;

		// check seed base for lush -> swamp conversion
		int64_t ss, cs, i, j;
		for(i = 0; i < 5; ++i) {
		    seed = tbase + ((i+0x53) << 48);
		    ss = getStartSeed(seed, lsBiome);
		    cs = getChunkSeed(ss, areaX_1_8+1, areaZ_1_8+1);
		    if(mcFirstInt(cs, 6) == 5) break;
		}
		if(i >= 5) continue;
		for(j = 0; j < 5; ++j) {
		    seed = tbase + ((j+0x53) << 48);
		    ss = getStartSeed(seed, lsBiome);
		    cs = getChunkSeed(ss, areaX_1_7+1, areaZ_1_7+1);
		    if(mcFirstInt(cs, 6) == 5) break;
		}
		if(j >= 5) continue;

		// check upper 16 bits for correct biomes
		for(int64_t u = 0; u < 0x10000; ++u) {
		    seed = tbase + (u << 48);

		    // 1.8 rough swamp check
		    ss = getStartSeed(seed, lsBiome);
		    cs = getChunkSeed(ss, areaX_1_8+1, areaZ_1_8+1);
		    if(mcFirstInt(cs, 6) != 5) continue;
		    setWorldSeed(lFilterBiome, seed);
		    genArea(lFilterBiome, biomeCache,
			areaX_1_8+1, areaZ_1_8+1, 1, 1);
		    if(biomeCache[0] != swamp) continue;

		    // 1.7 rough swamp check
		    cs = getChunkSeed(ss, areaX_1_7+1, areaZ_1_7+1);
		    if(mcFirstInt(cs, 6) != 5) continue;
		    genArea(lFilterBiome, biomeCache,
			areaX_1_7+1, areaZ_1_7+1, 1, 1);
		    if(biomeCache[0] != swamp) continue;

		    // verify 1.8 hut biomes
		    if(!isViableStructurePos(Swamp_Hut, MC_1_7, &g, seed,
			hut_1_8[0].x + dx, hut_1_8[0].z + dz)) continue;
		    if(!isViableStructurePos(Swamp_Hut, MC_1_7, &g, seed,
			hut_1_8[1].x + dx, hut_1_8[1].z + dz)) continue;
		    if(!isViableStructurePos(Swamp_Hut, MC_1_7, &g, seed,
			hut_1_8[2].x + dx, hut_1_8[2].z + dz)) continue;
		    if(!isViableStructurePos(Swamp_Hut, MC_1_7, &g, seed,
			hut_1_8[3].x + dx, hut_1_8[3].z + dz)) continue;

		    // verify 1.7 hut biomes
		    int hut_1_7_success = 0;
		    for(int o = 0; o < 4; ++o) {
			if(!(hut_1_7_exclude & 1<<o)) continue;

			if(o != 0 && !isViableStructurePos(Swamp_Hut,
			    MC_1_7, &g, seed, hut_1_7[0].x + dx,
			    hut_1_7[0].z +dz)) continue;
			if(o != 1 && !isViableStructurePos(Swamp_Hut,
			    MC_1_7, &g, seed, hut_1_7[1].x + dx,
			    hut_1_7[1].z +dz)) continue;
			if(o != 2 && !isViableStructurePos(Swamp_Hut,
			    MC_1_7, &g, seed, hut_1_7[2].x + dx,
			    hut_1_7[2].z +dz)) continue;
			if(o != 3 && !isViableStructurePos(Swamp_Hut,
			    MC_1_7, &g, seed, hut_1_7[3].x + dx,
			    hut_1_7[3].z +dz)) continue;

			hut_1_7_success = 1;
			break;
		    }
		    if(!hut_1_7_success) continue;

		    int mon_success = 0;
		    int mon = -1;
		    for(int m = 0; m < moncnt; ++m) {
			if(!isViableStructurePos(Monument, MC_1_7, &g,
			    seed, mon_0[m].x + dx,
			    mon_0[m].z + dz)) continue;
			if(!isViableStructurePos(Monument, MC_1_7, &g,
			    seed, mon_1[m].x + dx,
			    mon_1[m].z + dz)) continue;

			mon_success = 1;
			mon = m;
			break;
		    }
		    if(!mon_success) continue;

		    writeSeed(out, tid, seed, dx, dz,
			hut_1_7[0], hut_1_8[0], mon_0[mon]);
		}
	    }
	}
    }

    free(hut_1_7);
    free(hut_1_8);
    free(mon_0);
    free(mon_1);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_multi_seeds [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --search=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_SEARCH);
    fprintf(stderr, "    --shift=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_SHIFT);
    fprintf(stderr, "    --base_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --out_list=<file path>\n");
    fprintf(stderr, "        (Defaults to \"%s\")\n", DEFAULT_OUT_LIST);
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

/* Searches a list of 48 bit base seeds for full seeds with quad
 * features, triple features and double monuments in the given range
 * around (0,0) and outputs the seeds along with information on the
 * locations of the structures.
 */
int main(int argc, char *argv[]) {
    int search = DEFAULT_SEARCH;
    int shift = DEFAULT_SHIFT;
    const char *base_list = NULL;
    const char *out_list = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--search=", 9)) {
	    search = (int)strtoll(argv[a] + 9, &endptr, 0);
	} else if(!strncmp(argv[a], "--shift=", 8)) {
	    shift = (int)strtoll(argv[a] + 8, &endptr, 0);
	} else if(!strncmp(argv[a], "--base_list=", 12)) {
	    base_list = argv[a] + 12;
	} else if(!strncmp(argv[a], "--out_list=", 11)) {
	    out_list = argv[a] + 11;
	} else if(!strncmp(argv[a], "--num_threads=", 14)) {
	    num_threads = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strcmp(argv[a], "--help") || !strcmp(argv[a], "-h")) {
	    usage();
	    exit(0);
	} else {
	    fprintf(stderr, "Unrecognized argument: \"%s\"\n", argv[a]);
	    usage();
	    exit(1);
	}
    }

    // setup base seed list
    if(base_list == NULL) {
	fprintf(stderr, "Base seed list not specified\n");
	usage();
	exit(1);
    }

    int64_t scnt, *seeds;
    seeds = loadSavedSeeds(base_list, &scnt);
    if(seeds == NULL) {
	fprintf(stderr, "Could not parse seeds from \"%s\"\n", base_list);
	usage();
	exit(1);
    }

    FILE *out = fopen(out_list, "w");
    if(out == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", out_list);
	exit(1);
    }
    fprintf(out, "world seed,1.7 hut x,1.7 hut z,"
	    "1.8 hut x,1.8 hut z,monument x,monument z\n");
    
    initBiomes();

#ifdef USE_PTHREAD
    pthread_mutex_init(&ioMutex, NULL);
#else
    ioMutex = CreateMutex(NULL, FALSE, NULL);
#endif

    thread_id_t *threads = malloc(sizeof(thread_id_t) * num_threads);
    thread_info *info = malloc(sizeof(thread_info) * num_threads);
    for(int t = 0; t < num_threads; ++t) {
	info[t].tid = t;
	info[t].search = search;
	info[t].shift = shift;
	int64_t start = (t * scnt) / num_threads;
	int64_t end = ((t + 1) * scnt) / num_threads;
	info[t].seeds = &seeds[start];
	info[t].scnt = end - start;
	info[t].out = out;
    }

#ifdef USE_PTHREAD
    for(int t = 0; t < num_threads; ++t) {
	pthread_create(&threads[t], NULL,
			findMultiBasesThread, (void*)&info[t]);
    }

    for(int t = 0; t < num_threads; ++t) {
	pthread_join(threads[t], NULL);
    }
#else
    for(int t = 0; t < num_threads; ++t) {
	threads[t] = CreateThread(NULL, 0,
			findMultiBasesThread, (LPVOID)&info[t],
			0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
#endif

    free(threads);
    free(info);
    fclose(out);

    return 0;
}
