#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "nullspace_finders.h"

#define DEFAULT_SEARCH_RANGE 64
#define DEFAULT_SHIFT_RANGE 4
#define DEFAULT_BEST_LIST "./seeds/final_seeds_best.csv"
#define DEFAULT_GOOD_LIST "./seeds/final_seeds_good.csv"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
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

#ifdef USE_PTHREAD
void *findMultiBasesThread(void *arg) {
#else
DWORD WINAPI findMultiBasesThread(LPVOID arg) {
#endif
    thread_info *info = (thread_info*)arg;
    int tid = info->tid;
    int range = info->range;
    int64_t *seeds = info->seeds;
    int64_t scnt = info->scnt;
    FILE *best_seeds = info->best_seeds;
    FILE *good_seeds = info->good_seeds;

    const int spos_dim = (2 * range) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

    Pos hut_1_7, hut_1_8;

    int mlist_size = 16;
    Pos *mlist = malloc(sizeof(Pos) * mlist_size);
    int mcnt = 0;

    for(int64_t s = 0; s < scnt; ++s) {
	int64_t seed = seeds[s];

	// find all structure chunk positions
	int spos_idx = 0;
	for(int x = -range; x <= range; ++x) {
	    for(int z = -range; z <= range; ++z, ++spos_idx) {
		spos_feat[spos_idx] = getFeaturePos(
		    FEATURE_CONFIG, seed, x, z
		);
		spos_mon[spos_idx] = getLargeStructurePos(
		    MONUMENT_CONFIG, seed, x, z
		);
	    }
	}

	// search for structure clusters
	spos_idx = 0;
	for(int x = -range; x < range; ++x, ++spos_idx) {
	    for(int z = -range; z < range; ++z, ++spos_idx) {
		int fcs_min = (x == 0 && z == 0) ? 3 : 4;
		int fcs = getClusterSize(
		    spos_feat[spos_idx],
		    spos_feat[spos_idx + spos_dim],
		    spos_feat[spos_idx + 1],
		    spos_feat[spos_idx + spos_dim + 1],
		    7+1, 7+43+1, 9+1, fcs_min
		);
		if(fcs >= fcs_min) {
		    ++fcnt;
		    if(fcnt > flist_size) {
			flist_size *= 2;
			flist = realloc(flist, sizeof(Pos) * flist_size);
		    }
		    flist[fcnt - 1] = (Pos){x, z};
		}

		int mcs = getClusterSize(
		    spos_mon[spos_idx],
		    spos_mon[spos_idx + spos_dim],
		    spos_mon[spos_idx + 1],
		    spos_mon[spos_idx + spos_dim + 1],
		    58+1, 0 /*replace with guardian farm size*/, 58+2, 2
		);
		if(mcs >= 2) {
		    ++mcnt;
		    if(mcnt > mlist_size) {
			mlist_size *= 2;
			mlist = realloc(mlist, sizeof(Pos) * mlist_size);
		    }
		    mlist[mcnt - 1] = (Pos){x, z};
		}
	    }
	}

	if(fcnt >= 2 && mcnt >= 1) {
	    //writeSeed(tid, seed, out, flist, mlist, fcnt, mcnt);
	}

	fcnt = 0;
	mcnt = 0;
    }

    free(spos_feat);
    free(spos_mon);
    free(mlist);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_multi_seeds [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --search_range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_SEARCH_RANGE);
    fprintf(stderr, "    --shift_range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_SHIFT_RANGE);
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
    int search_range = DEFAULT_SEARCH_RANGE;
    int shift_range = DEFAULT_SHIFT_RANGE;
    const char *base_list = NULL;
    const char *best_list = DEFAULT_BEST_LIST;
    const char *good_list = DEFAULT_GOOD_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--search_range=", 14)) {
	    search_range = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strncmp(argv[a], "--shift_range=", 13)) {
	    shift_range = (int)strtoll(argv[a] + 13, &endptr, 0);
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
	info[t].range = range;
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