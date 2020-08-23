#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"

#define DEFAULT_MAX_DISTANCE 64
#define DEFAULT_OUT_LIST "./seeds/dqh_dmon.txt"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int range;
    int64_t *slist;
    int64_t scnt;
    FILE *out;
} threadinfo;

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

/* writes a string containing a seed and its structure positions to stdout
 * and out file
 */
void writeSeed(int tid, char *str, FILE *out) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif

    printf("Thread %d: %s\n", tid, str);
    fprintf(out, "%s\n", str);
    fflush(out);

#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif
}

/* searches a list of seeds for quad structures and double monuments */
#ifdef USE_PTHREAD
void *findDoubleQuadStructuresThread(void *arg) {
#else
DWORD WINAPI findDoubleQuadStructuresThread(LPVOID arg) {
#endif
    threadinfo *info = (threadinfo*)arg;
    int tid = info->tid;
    int range = info->range;
    int64_t *slist = info->slist;
    int64_t scnt = info->scnt;
    FILE *out = info->out;

    for(int s = 0; s < scnt; ++s) {
	;
    }

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_multi_bases [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --range=<integer>\n");
    fprintf(stderr, "        (Defaults to %d regions)\n", DEFAULT_MAX_DISTANCE);
    fprintf(stderr, "    --seed_list=<file path>\n");
    fprintf(stderr, "        (Required)\n");
    fprintf(stderr, "    --out_list=<file path>\n");
    fprintf(stderr, "        (Defaults to %s)\n", DEFAULT_OUT_LIST);
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to %d)\n", DEFAULT_NUM_THREADS);
}

int main(int argc, char *argv[]) {
    int range = DEFAULT_MAX_DISTANCE;
    char *seed_list_filename = NULL;
    char *out_list_filename = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--range=", 15)) {
	    range = (int)strtoll(argv[a] + 15, &endptr, 0);
	} else if(!strncmp(argv[a], "--seed_list=", 12)) {
	    primary_list_filename = argv[a] + 12;
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

    // set up input seed list
    if(seed_list_filename == NULL) {
	fprintf(stderr, "Primary input list not specified\n");
	usage();
	exit(1);
    }

    int64_t scnt, *seeds;
    seeds = loadSavedSeeds(seed_list_filename, &pcnt);
    if(seeds == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", seed_list_filename);
	exit(1);
    }

    // set up output seed list
    FILE *out = fopen(out_list_filename, "a");
    if(out == NULL) {
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
	int64_t start = t * scnt / num_threads;
	int64_t end = (t + 1) * scnt / num_threads;

	info[t].tid = t;
	info[t].range = range;
	info[t].slist = &seeds[start];
	info[t].scnt = end - start;
	info[t].out = out;
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
