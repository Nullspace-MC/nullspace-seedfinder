#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "nullspace_finders.h"

#define DEFAULT_SEARCH 64
#define DEFAULT_SHIFT 4
#define DEFAULT_BEST_LIST "./seeds/final_seeds_best.csv"
#define DEFAULT_GOOD_LIST "./seeds/final_seeds_good.csv"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int search;
    int shift;
    int64_t *seeds;
    int64_t scnt;
    FILE *best_seeds;
    FILE *good_seeds;
} thread_info;

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

void writeSeed(FILE *out, int tid, int64_t seed,
	Pos hut_1_7, Pos hut_1_8, Pos mon, int qual) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif
    
    printf("Thread %d: %ld 1.7 Hut: (%d,%d)(%d) 1.8 Hut: (%d,%d) "
	"Monument: (%d,%d)\n", tid, seed, hut_1_7.x, hut_1_7.z, qual,
	hut_1_8.x, hut_1_8.z, mon.x, mon.z);

    fprintf(out, "%ld,%d,%d,%d,%d,%d,%d\n", seed, hut_1_7.x, hut_1_7.z,
	hut_1_8.x, hut_1_8.z, mon.x, mon.z);

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
	58+1, 0 /*replace with guardian farm size*/, 58+2	\
    ) <= 128) {							\
	++(*moncnt);						\
	if(*moncnt > *mons_size) {				\
	    *mons_size *= 2;					\
	    *mon_0 = realloc(*mon_0, sizeof(Pos) * *mons_size);	\
	    *mon_1 = realloc(*mon_1, sizeof(Pos) * *mons_size);	\
	}							\
	(*mon_0)[*moncnt - 1] = spos_mon[A];			\
	(*mon_1)[*moncnt - 1] = spos_mon[B];			\
    }								\
} while(0)

void setStructurePositions(int64_t seed, int search,
	int *hut_1_7_exclude, Pos *hut_1_7, Pos *hut_1_8,
	int *mons_size, Pos **mon_0, Pos **mon_1, int *moncnt) {
    const int spos_dim = (2 * search) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

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
	    int csize = getClusterSize(
		spos_feat[spos_idx],
		spos_feat[spos_idx + spos_dim],
		spos_feat[spos_idx + 1],
		spos_feat[spos_idx + spos_dim + 1],
		7+1, 7+43+1, 9+1, cmin
	    );
	    if(x == 0 && z == 0) {
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
			    hut_1_7[(0 + o) % 4],
			    hut_1_7[(1 + o) % 4],
			    hut_1_7[(2 + o) % 4],
			    7+1, 7+43+1, 9+1, 128
			);
			if(r <= 128) {
			    *hut_1_7_exclude |= 1<<o;
			}
		    }
		}
	    } else if(csize == 4) {
		hut_1_8[0] = spos_feat[spos_idx];
		hut_1_8[1] = spos_feat[spos_idx + spos_dim];
		hut_1_8[2] = spos_feat[spos_idx + 1];
		hut_1_8[3] = spos_feat[spos_idx + spos_dim + 1];
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
    FILE *best_seeds = info->best_seeds;
    FILE *good_seeds = info->good_seeds;

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
	int64_t seed = seeds[s];

	// search for structure clusters
	moncnt = 0;
	setStructurePositions(seed, search,
	    &hut_1_7_exclude, hut_1_7, hut_1_8,
	    &mons_size, &mon_0, &mon_1, &moncnt
	);
	if(moncnt > 0) {
	    if(hut_1_7_exclude == 0xf) {
		writeSeed(best_seeds, tid, seed,
		    hut_1_7[0], hut_1_8[0], mon_0[0], 4
		);
	    } else {
		writeSeed(good_seeds, tid, seed,
		    hut_1_7[0], hut_1_8[0], mon_0[0], 3
		);
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
    fprintf(stderr, "    --best_list=<file path>\n");
    fprintf(stderr, "        (Defaults to \"%s\")\n", DEFAULT_BEST_LIST);
    fprintf(stderr, "    --good_list=<file path>\n");
    fprintf(stderr, "        (Defaults to \"%s\")\n", DEFAULT_GOOD_LIST);
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
    const char *best_list = DEFAULT_BEST_LIST;
    const char *good_list = DEFAULT_GOOD_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--search=", 9)) {
	    search = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strncmp(argv[a], "--shift=", 8)) {
	    shift = (int)strtoll(argv[a] + 13, &endptr, 0);
	} else if(!strncmp(argv[a], "--base_list=", 12)) {
	    base_list = argv[a] + 12;
	} else if(!strncmp(argv[a], "--best_list=", 12)) {
	    best_list = argv[a] + 12;
	} else if(!strncmp(argv[a], "--good_list=", 12)) {
	    good_list = argv[a] + 12;
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

    FILE *best_seeds = fopen(best_list, "w");
    if(best_seeds == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", best_list);
	exit(1);
    }
    fprintf(best_seeds, "world seed,1.7 hut x,1.7 hut z,"
	    "1.8 hut x,1.8 hut z,monument x,monument z\n");

    FILE *good_seeds = fopen(good_list, "w");
    if(good_seeds == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", good_list);
	exit(1);
    }
    fprintf(good_seeds, "world seed,1.7 hut x,1.7 hut z,"
	    "1.8 hut x,1.8 hut z,monument x,monument z\n");

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
	info[t].best_seeds = best_seeds;
	info[t].good_seeds = good_seeds;
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
    fclose(best_seeds);
    fclose(good_seeds);

    return 0;
}
