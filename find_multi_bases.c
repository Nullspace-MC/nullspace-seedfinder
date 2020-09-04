#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "nullspace_finders.h"

#define DEFAULT_RANGE 64
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
    int64_t *seeds;
    int64_t scnt;
} thread_info;

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

    const int spos_dim = (2 * range) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

    for(int64_t s = 0; s < scnt; ++s) {
	int64_t seed = seeds[s];

	// find all structure chunk positions
	int spos_idx = 0;
	for(int x = -range; x <= range; ++x) {
	    for(int z = -range; z <= range; ++z, ++spos_idx) {
		spos_feat[spos_idx] = getFeatureChunkInRegion(
		    FEATURE_CONFIG, seed, x, z
		);
		spos_mon[spos_idx] = getLargeStructureChunkInRegion(
		    MONUMENT_CONFIG, seed, x, z
		);
	    }
	}

	// search for structure clusters
	int fcnt, mcnt;
	spos_idx = 0;
	for(int x = -range; x < range; ++x, ++spos_idx) {
	    for(int x = -range; z < range; ++z, ++spos_idx) {
		int feat_cluster_size = clusterSize(
		    &spos_feat[spos_idx],
		    &spos_feat[spos_idx + spos_dim],
		    &spos_feat[spos_idx + 1],
		    &spos_feat[spos_idx + spos_dim + 1],
		    7+1, 7+43+1, 9+1, 3
		);
		if(feat_cluster_size == 4) {
		    ++fcnt;
		} else if(feat_cluster_size == 3) {
		    int64_t tseed = moveStructure(seed, -x, -z);
		}

		int mon_cluster_size = clusterSize(
		    &spos_mon[spos_idx],
		    &spos_mon[spos_idx + spos_dim],
		    &spos_mon[spos_idx + 1],
		    &spos_mon[spos_idx + spos_dim + 1],
		    58+1, /*TODO: replace with gaurdian farm height*/ 0, 58+1, 2
		);
		if(mon_cluster_size >= 2) {
		    ++mcnt;
		}
	    }
	}
    }

    free(spos_feat);
    free(spos_mon);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_other_structures [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_RANGE);
    fprintf(stderr, "    --base_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

/* Searches a list of 48 bit base seeds for additional triple huts, quad
 * huts and double monuments in the given range around (0,0).
 */
int main(int argc, char *argv[]) {
    int range = DEFAULT_RANGE;
    const char *base_list = NULL;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--range=", 7)) {
	    range = (int)strtoll(argv[a] + 7, &endptr, 0);
	} else if(!strncmp(argv[a], "--base_list=", 12)) {
	    base_list = argv[a] + 12;
	} else if(!strncmp(argv[a], "--num_threads=", 14)) {
	    num_threads = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strcmp(argv[a] "--help") || !strcmp(argv[a], "-h")) {
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

    thread_id_t *threads = malloc(sizeof(thread_id_t) * num_threads);
    thread_info *info = malloc(sizeof(thread_info) * num_threads);
    for(int t = 0; t < num_threads; ++t) {
	info[t].tid = t;
	info[t].range = range;
	int64_t start = (t * scnt) / num_threads;
	int64_t end = ((t + 1) * scnt) / num_threads;
	info[t].seeds = &seeds[start];
	info[t].scnt = end - start;

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

    return 0;
}
