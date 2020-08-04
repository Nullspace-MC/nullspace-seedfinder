#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "finders.h"

#define DEFAULT_START_SEED 0
#define DEFAULT_END_SEED 281474976710656
#define DEFAULT_QUADHUT_LIST "./seeds/quadhut_bases.txt"
#define DEFAULT_NUM_THREADS 1

typedef struct {
    int tid;
    int64_t start;
    int64_t end;
    FILE *list;
} threadinfo;

pthread_mutex_t ioLock;

int distance(Pos p1, Pos p2) {
    const int dx = p2.x - p1.x;
    const int dz = p2.z - p1.z;
    return (dx * dx) + (dz * dz);
}

void* writeSeed(int tid, int64_t seed, FILE *quadhut_list) {
    pthread_mutex_lock(&ioLock);

    printf("Thread %d: Found %ld\n", tid, seed);
    fprintf(quadhut_list, "%ld\n", seed);
    fflush(quadhut_list);

    pthread_mutex_unlock(&ioLock);

    return NULL;
}

void *findOriginStructuresThread(void *arg) {
    threadinfo info = *(threadinfo*)arg;

    const int tid = info.tid;
    const int64_t start_seed = info.start;
    const int64_t end_seed = info.end;
    FILE *quadhut_list = info.list;

    pthread_mutex_lock(&ioLock);
    printf("Thread %d: Searching from %ld to %ld\n",
	    tid, start_seed, end_seed);
    pthread_mutex_unlock(&ioLock);

    for(int64_t seed = start_seed; seed < end_seed; ++seed) {
	Pos huts[4];
	huts[0] = getStructurePos(FEATURE_CONFIG, seed, 0, 0);
	huts[1] = getStructurePos(FEATURE_CONFIG, seed, 0, 1);
	huts[2] = getStructurePos(FEATURE_CONFIG, seed, 1, 0);
	huts[3] = getStructurePos(FEATURE_CONFIG, seed, 1, 1);

	int totalX = huts[0].x + huts[1].x + huts[2].x + huts[3].x;
	int totalZ = huts[0].z + huts[1].z + huts[2].z + huts[3].z;
	
	// quadhut check
	Pos center = {totalX/4, totalZ/4};
	int isQuadhut = 1;
	for(int r = 0; r < 4; ++r) {
	    if(distance(huts[r], center) > 16384) {
		isQuadhut = 0;
		break;
	    }
	}

	if(isQuadhut) {
	    writeSeed(tid, seed, quadhut_list);
	}
    }

    pthread_exit(NULL);

    return 0;
}

int main(int argc, char *argv[]) {
    int64_t start_seed = DEFAULT_START_SEED;
    int64_t end_seed = DEFAULT_END_SEED;
    char *quadhut_list_filename = DEFAULT_QUADHUT_LIST;
    char *file_access_mode = "w";
    int num_threads = DEFAULT_NUM_THREADS;
    
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--start_seed=", 13)) {
	    start_seed = strtoll(argv[a] + 13, &endptr, 0);
	} else if(!strncmp(argv[a], "--end_seed=", 11)) {
	    end_seed = strtoll(argv[a] + 11, &endptr, 0);
	} else if(!strncmp(argv[a], "--quadhut_list=", 15)) {
	    quadhut_list_filename = argv[a] + 15;
	} else if(!strncmp(argv[a], "--num_threads=", 14)) {
	    num_threads = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strcmp(argv[a], "--append") || !strcmp(argv[a], "-a")) {
	    file_access_mode = "a";
	}
    }

    FILE *quadhut_list = fopen(quadhut_list_filename, file_access_mode);

    if(quadhut_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", quadhut_list_filename);
	exit(-1);
    }
    
    pthread_mutex_init(&ioLock, NULL);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    threadinfo *info = malloc(sizeof(threadinfo) * num_threads);
    int64_t seed_range = end_seed - start_seed;
    for(int t = 0; t < num_threads; ++t) {
	info[t].tid = t;
	info[t].start = start_seed + (t * seed_range / num_threads);
	info[t].end = start_seed + ((t + 1) * seed_range / num_threads);
	info[t].list = quadhut_list;

	pthread_create(&threads[t], NULL,
			findOriginStructuresThread,
			(void*)&info[t]);
    }

    for(int t = 0; t < num_threads; ++t) {
	pthread_join(threads[t], NULL);
    }
    
    free(threads);
    free(info);
    fclose(quadhut_list);

    return 0;
}
