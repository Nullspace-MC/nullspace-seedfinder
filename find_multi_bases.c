#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"

#define DEFAULT_RANGE 64
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
    int64_t *seeds;
    int64_t *scnt;
} threadinfo;

#ifdef USE_PTHREAD
void *findMultiBasesThread(void *arg);
#else
DWORD WINAPI findMultiBasesThread(LPVOID arg);
#endif

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

/* searches a list of 48 bit base seeds for additional triple huts, quad
 * huts and double monuments in the given range around (0,0)
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
	fprintf(stderr, "Could not parse base seeds from \"%s\"\n", base_list);
	usage();
	exit(1);
    }

    return 0;
}

int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11);

#ifdef USE_PTHREAD
void *findMultiBasesThread(void *arg) {
#else
DWORD WINAPI findMultiBasesThread(LPVOID arg) {
#endif
    threadinfo *info = (threadinfo*)arg;
    int tid = info->tid;
    int range = info->range;
    int64_t *seeds = info->seeds;
    int64_t scnt = info->scnt;


    int spos_dim = (2 * range) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

    for(int64_t s = 0; s < scnt; ++s) {
	int64_t seed = seeds[s];
	int spos_idx;
	
	// find all structure positions
	spos_idx = 0;
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

	// check for structure clusters
	spos_idx = 0;
	for(int x = -range; x < range; ++x, ++spos_idx) {
	    for(int z = -range; z < range; ++z, ++spos_idx) {
		int cluster_size = clusterSize(
		    &spos_feat[spos_idx],
		    &spos_feat[spos_idx + spos_dim],
		    &spos_feat[spos_idx + 1],
		    &spos_feat[spos_idx + spos_dim + 1]
		);
	    }
	}
    }

    free(spos_feat);
    free(spos_mon);
}

/* returns the size of the feature cluster within 4 regions, given the
 * chunk position of the feature in each region
 *
 * returns 0 for all cluster sizes less than 3, because they are not of
 * interest in this case
 */
int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11) {
    // check if diagonal regions are in range
    int x1, z1, x2, z2;
    int diag = 0;
    float sqrad;

    x1 = (feat11->x + 32) - feat00->x;
    z1 = (feat11->z + 32) - feat00->z;
    if(x*x + z*z <= 255) diag += 1;
    x2 = (feat01->x + 32) - feat10->x;
    z2 = (feat10->z + 32) - feat01->z;
    if(x*x + z*z <= 255) diag += 2;

    // check for quadhut
    if(diag == 3) {
	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);

	if(sqrad < 128) {
	    return 4;
	}
    }

    // check for triple hut including first diagonal
    if(diag & 1) {
	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    31, 0,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}

	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    0, 31,
	    feat10->x, feat10->z,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}
    }

    //check for triple hut including second diagonal
    if(diag & 2) {
	sqrad = getEnclosingRadius(
	    31, 31,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}

	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    0, 0,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    7+1, 7+43+1, 9+1,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}
    }

    return 0;
}
