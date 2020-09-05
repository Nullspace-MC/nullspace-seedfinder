#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "nullspace_finders.h"

#define DEFAULT_RANGE 64
#define DEFAULT_OUT_LIST "./seeds/multi_bases.txt"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
    int64_t *seeds;
    int64_t scnt;
    FILE *out;
} thread_info;

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

void writeSeed(int tid, int64_t seed, FILE *out,
	Pos *flist, Pos *mlist, int fcnt, int mcnt) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif
    
    printf("Thread %d: %ld", tid, seed);
    fprintf(out, "%ld", seed);
    for(int i = 0; i < fcnt; ++i) {
	printf(" f(%d,%d)", flist[i].x, flist[i].z);
	fprintf(out, " f(%d,%d)", flist[i].x, flist[i].z);
    }
    for(int i = 0; i < mcnt; ++i) {
	printf(" m(%d,%d)", mlist[i].x, mlist[i].z);
	fprintf(out, " m(%d,%d)", mlist[i].x, mlist[i].z);
    }
    printf("\n");
    fprintf(out, "\n");

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
    FILE *out = info->out;

    const int spos_dim = (2 * range) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

    int flist_size = 16;
    int mlist_size = 16;
    Pos *flist = malloc(sizeof(Pos) * flist_size);
    Pos *mlist = malloc(sizeof(Pos) * mlist_size);
    int fcnt = 0;
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
	    writeSeed(tid, seed, out, flist, mlist, fcnt, mcnt);
	}

	fcnt = 0;
	mcnt = 0;
    }

    free(spos_feat);
    free(spos_mon);
    free(flist);
    free(mlist);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_origin_triplets [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_RANGE);
    fprintf(stderr, "    --base_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --out_list=<file path>\n");
    fprintf(stderr, "        (Defaults to \"%s\"\n", DEFAULT_OUT_LIST);
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

int main(int argc, char *argv[]) {
    int range = DEFAULT_RANGE;
    const char *base_list = NULL;
    const char *out_list = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--range=", 7)) {
	    range = (int)strtoll(argv[a] + 7, &endptr, 0);
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
