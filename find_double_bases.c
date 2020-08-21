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
    int64_t *primary_list;
    int64_t *secondary_list;
    int primary_start;
    int primary_end;
    FILE *out_list;
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
    pthread_mutex_lock(&ioMutetex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif

    printf("%ld: (%d,%d)\n", seed, dx, dz);
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
    threadinfo info = *(threadinfo*)arg;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_double_bases [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --max_distance=<integer>\n");
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
    int max_distance = DEFAULT_MAX_DISTANCE;
    char *primary_list_filename = NULL;
    char *secondary_list_filename = NULL;
    char *out_list_filename = DEFAULT_OUT_LIST;
    int num_threads = DEFAULT_NUM_THREADS;

    // parse arguments
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--max_distance=", 15)) {
	    max_distance = (int)strtoll(argv[a] + 15, &endptr, 0);
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
	info[t].tid = t;
	info[t].primary_list = primary_seeds;
	info[t].secondary_list = secondary_seeds;
	info[t].primary_start = t * pcnt / num_threads;
	info[t].primary_end = (t + 1) * pcnt / num_threads;
	info[t].out_list = out_list;
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
