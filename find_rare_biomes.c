#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"
#include "layers.h"

#define DEFAULT_RANGE 2048
#define DEFAULT_OUT_LIST "./seeds/best_seeds.txt"
#define DEFAULT_NUM_THREADS 1

const int biome_list[] = {
    mushroom_fields, jungle, badlands, ice_spikes
};

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

void writeSeed(FILE *out, int tid, int64_t seed) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif

    printf("Thread %d: %ld\n", tid, seed);
    fprintf(out, "%ld\n", seed);

#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif
}

#ifdef USE_PTHREAD
void *findRareBiomesThread(void *arg) {
#else
DWORD WINAPI findFinalSeedsThread(LPVOID arg) {
#endif
    thread_info *info = (thread_info*)arg;
    int tid = info->tid;
    int range = info->range;
    int64_t *seeds = info->seeds;
    int64_t scnt = info->scnt;
    FILE *out = info->out;

    range = range >> 4;
    int ax = -range, az = -range;
    int w = 2 * range, h = 2 * range;
    int a = w * h;

    LayerStack g;
    setupGenerator(&g, MC_1_7);
    int *cache = allocCache(&g.layers[L_SHORE_16], w, h);
    BiomeFilter filter = setupBiomeFilter(biome_list, 4);

    for(int64_t s = 0; s < scnt; ++s) {
	if(!(checkForBiomes(&g, L_SHORE_16, cache, seeds[s],
	    ax, az, w, h, filter, 1) > 0)) continue;

	applySeed(&g, seeds[s]);
	genArea(&g.layers[L_SHORE_16], cache, ax, az, w, h);

	int mushroom_count = 0;
	int jungle_count = 0;
	int badlands_count = 0;
	int ice_spikes_count = 0;
	for(int i = 0; i < a; ++i) {
	    if(cache[i] == mushroom_fields) ++mushroom_count;
	    if(cache[i] == jungle) ++jungle_count;
	    if(cache[i] == badlands) ++badlands_count;
	    if(cache[i] == ice_spikes) ++ice_spikes_count;

	    if(mushroom_count > 200 && jungle_count !=0 &&
		badlands_count > 1000 && ice_spikes_count !=0) {
		writeSeed(out, tid, seeds[s]);
		break;
	    }
	}
    }

    free(cache);
    
#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_rare_biomes [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_RANGE);
    fprintf(stderr, "    --in_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --out_list=<file path>\n");
    fprintf(stderr, "        (Defaults to \"%s\")\n", DEFAULT_OUT_LIST);
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

/* Searches a list of seeds provided in a csv formatted like the output
 * of find_final_seeds for seeds with good biomes within the given range
 * of (0,0).
 */
int main(int argc, char *argv[]) {
    int range = DEFAULT_RANGE;
    const char *in_list = NULL;
    const char *out_list = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--range=", 8)) {
	    range = (int)strtoll(argv[a] + 8, &endptr, 0);
	} else if(!strncmp(argv[a], "--in_list=", 10)) {
	    in_list = argv[a] + 10;
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

    // setup input list
    if(in_list == NULL) {
	fprintf(stderr, "Input list not specified\n");
	usage();
	exit(1);
    }

    int64_t scnt, *seeds;
    seeds = loadSavedSeeds(in_list, &scnt);
    if(seeds == NULL) {
	fprintf(stderr, "Could not parse seeds from \"%s\"\n", in_list);
	usage();
	exit(1);
    }

    FILE *out = fopen(out_list, "w");
    if(out == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", out_list);
	exit(1);
    }
    
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
			findRareBiomesThread, (void*)&info[t]);
    }

    for(int t = 0; t < num_threads; ++t) {
	pthread_join(threads[t], NULL);
    }
#else
    for(int t = 0; t < num_threads; ++t) {
	threads[t] = CreateThread(NULL, 0,
			findRareBiomesThread, (LPVOID)&info[t],
			0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
#endif

    free(threads);
    free(info);
    free(seeds);
    fclose(out);

    return 0;
}
