#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"

#define DEFAULT_MAX_DISTANCE 64
#define DEFAULT_OUT_LIST "./seeds/double_quadhut_list.txt"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
    int64_t *plist;
    int64_t pcnt;
    int64_t *slist;
    int64_t scnt;
    FILE *out;
} threadinfo;

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

/* writes a seed and the position of its second quad structure to stdout and
 * out file
 */
void writeSeed(int tid, int64_t seed, int dx, int dz, FILE *out_list) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif

    printf("Thread %d: %ld: (%d,%d)\n", tid, seed, dx, dz);
    fprintf(out_list, "%ld: (%d,%d)\n", seed, dx, dz);
    fflush(out_list);

#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif
}

/* searches a list of quad structure seeds for double quad structures */
#ifdef USE_PTHREAD
void *findDoubleQuadStructuresThread(void *arg) {
#else
DWORD WINAPI findDoubleQuadStructuresThread(LPVOID arg) {
#endif
    threadinfo *info = (threadinfo*)arg;
    int tid = info->tid;
    int range = info->range;
    int64_t *plist = info->plist;
    int64_t pcnt = info->pcnt;
    int64_t *slist = info->slist;
    int64_t scnt = info->scnt;
    FILE *out = info->out;

    // cubitect's method for calculating the offset between two seeds:
    //
    // seed2 = (seed1 - 341873128712*dx - 132897987541*dz) mod 2^48
    // diff = (seed2 - seed1) mod 2^48
    // => diff + 341873128712*dx + 132897987541*dz = n*(2^48)
    // => diff + n*(2^48) + 132897987541*dz = -341873128712*dx
    // => (diff + n*(2^48) + 132897987541*dz) mod 341873128712 = 0

    for(int64_t i = 0; i < pcnt; ++i) {
	for(int64_t j = 0; j < scnt; ++j) {
	    int64_t diff = (slist[j] - plist[i]) & 0xffffffffffff;
	    if(diff == 0) continue;

	    const int64_t start = diff - range * 132897987541;
	    const int64_t end = diff + range * 132897987541;

	    for(int64_t lhs = start; lhs <= end; lhs += 132897987541) {
		// lhs mod 8 = 0 is a requirement for lhs mod 341873128712 = 0
		if(lhs & 0x7) continue;

		int64_t mod = lhs % 341873128712;
		
		if(mod == 0) {
		    int64_t dx = (lhs) / 341873128712;
		    int64_t dz = -(lhs - diff) / 132897987541;

		    //if(dx >= -range && dx <= range) {
			writeSeed(tid, plist[i], dx, dz, out);
		    //}
		}
		if((lhs + (1L<<48)) % 341873128712 == 0) {
		    int64_t dx = (lhs + (1L<<48)) / 341873128712;
		    int64_t dz = -(lhs - diff) / 132897987541;

		    //if(dx >= -range && dx <= range) {
			writeSeed(tid, plist[i], dx, dz, out);
		    //}
		}
		if((lhs - (1L<<48)) % 341873128712 == 0) {
		    int64_t dx = (lhs - (1L<<48)) / 341873128712;
		    int64_t dz = -(lhs - diff) / 132897987541;

		    //if(dx >= -range && dx <= range) {
			writeSeed(tid, plist[i], dx, dz, out);
		    //}
		}
	    }
	}
    }

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_double_bases [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_MAX_DISTANCE);
    fprintf(stderr, "    --primary_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --secondary_list=<file path>\n");
    fprintf(stderr, "        (Optional, reuses primary list if left blank)\n");
    fprintf(stderr, "    --out_list=<file path>\n");
    fprintf(stderr, "        (Defaults to %s)\n", DEFAULT_OUT_LIST);
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

int main(int argc, char *argv[]) {
    int range = DEFAULT_MAX_DISTANCE;
    char *primary_list_filename = NULL;
    char *secondary_list_filename = NULL;
    char *out_list_filename = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--range=", 15)) {
	    range = (int)strtoll(argv[a] + 15, &endptr, 0);
	} else if(!strncmp(argv[a], "--primary_list=", 15)) {
	    primary_list_filename = argv[a] + 15;
	} else if(!strncmp(argv[a], "--secondary_list=", 17)) {
	    secondary_list_filename = argv[a] + 17;
	} else if(!strncmp(argv[a], "--out_list=", 11)) {
	    out_list_filename = argv[a] + 11;
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

    // set up input seed lists
    if(primary_list_filename == NULL) {
	fprintf(stderr, "Primary input list not specified\n");
	usage();
	exit(1);
    }

    int64_t pcnt, *primary_seeds;
    primary_seeds = loadSavedSeeds(primary_list_filename, &pcnt);
    if(primary_seeds == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", primary_list_filename);
	exit(1);
    }

    int64_t scnt, *secondary_seeds;
    if(secondary_list_filename != NULL) {
	secondary_seeds = loadSavedSeeds(secondary_list_filename, &scnt);
    } else {
	secondary_seeds = primary_seeds;
	scnt = pcnt;
    }
    if(secondary_seeds == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", secondary_list_filename);
	exit(1);
    }
    
    // set up output seed list
    FILE *out_list = fopen(out_list_filename, "a");
    if(out_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", out_list_filename);
	exit(1);
    }

    // set up for multithreading
#ifdef USE_PTHREAD
    pthread_mutex_init(&ioMutex, NULL);
#else
    ioMutex = CreateMutex(NULL, FALSE, NULL);
#endif

    thread_id_t *threads = malloc(sizeof(thread_id_t) * num_threads);
    threadinfo *info = malloc(sizeof(threadinfo) * num_threads);
    for(int t = 0; t < num_threads; ++t) {
	int64_t primary_start = t * pcnt / num_threads;
	int64_t primary_end = (t + 1) * pcnt / num_threads;

	info[t].tid = t;
	info[t].range = range;
	info[t].plist = &primary_seeds[primary_start];
	info[t].pcnt = primary_end - primary_start;
	info[t].slist = secondary_seeds;
	info[t].scnt = scnt;
	info[t].out = out_list;
    }

    // create search threads
#ifdef USE_PTHREAD
    for(int t = 0; t < num_threads; ++t) {
	pthread_create(&threads[t], NULL,
			findDoubleQuadStructuresThread, (void*)&info[t]);
    }

    for(int t = 0; t < num_threads; ++t) {
	pthread_join(threads[t], NULL);
    }
#else
    for(int t = 0; t < num_threads; ++t) {
	threads[t] = CreateThread(NULL, 0,
			findDoubleQuadStructuresThread, (LPVOID)&info[t],
			0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
#endif
    
    free(threads);
    free(info);
    fclose(out_list);

    return 0;
}
