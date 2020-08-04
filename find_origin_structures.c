#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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

#ifdef USE_PTHREAD
pthread_mutex_t ioMutex;
#else
HANDLE ioMutex;
#endif

int distance(Pos p1, Pos p2) {
    const int dx = p2.x - p1.x;
    const int dz = p2.z - p1.z;
    return (dx * dx) + (dz * dz);
}

void writeSeed(int tid, int64_t seed, FILE *quadhut_list) {
#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif

    printf("Thread %d: Found %ld\n", tid, seed);
    fprintf(quadhut_list, "%ld\n", seed);
    fflush(quadhut_list);

#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif
}

#ifdef USE_PTHREAD
void *findOriginStructuresThread(void *arg) {
#else
DWORD WINAPI findOriginStructuresThread(LPVOID arg) {
#endif
    threadinfo info = *(threadinfo*)arg;

    const int tid = info.tid;
    const int64_t start_seed = info.start;
    const int64_t end_seed = info.end;
    FILE *quadhut_list = info.list;

#ifdef USE_PTHREAD
    pthread_mutex_lock(&ioMutex);
#else
    WaitForSingleObject(ioMutex, INFINITE);
#endif
    printf("Thread %d: Searching from %ld to %ld\n",
	    tid, start_seed, end_seed);
#ifdef USE_PTHREAD
    pthread_mutex_unlock(&ioMutex);
#else
    ReleaseMutex(ioMutex);
#endif

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

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}

void usage() {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "  find_origin_structures [OPTION]...\n");
    fprintf(stderr, "    --help    (-h)\n");
    fprintf(stderr, "    --start_seed=<integer>\n");
    fprintf(stderr, "        (Defaults to 0)\n");
    fprintf(stderr, "    --end_seed=<integer>\n");
    fprintf(stderr, "        (Defaults to 281474976710656)\n");
    fprintf(stderr, "    --work_unit=<integer>\n");
    fprintf(stderr, "        (Optional, specifies a work unit 0-255)\n");
    fprintf(stderr, "    --num_units=<integer>\n");
    fprintf(stderr, "        (Defaults to 1)\n");
    fprintf(stderr, "    --quadhut_list=<file path>\n");
    fprintf(stderr, "        (Defaults to ./seeds/quadhut_list.txt)\n");
    fprintf(stderr, "    --num_threads=<integer>\n");
    fprintf(stderr, "        (Defaults to 1)\n");
}

int main(int argc, char *argv[]) {
    int64_t start_seed = DEFAULT_START_SEED;
    int64_t end_seed = DEFAULT_END_SEED;
    char *quadhut_list_filename = DEFAULT_QUADHUT_LIST;
    char *file_access_mode = "a";
    int num_threads = DEFAULT_NUM_THREADS;

    int work_unit = -1;
    int num_units = 1;
    
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--start_seed=", 13)) {
	    start_seed = strtoll(argv[a] + 13, &endptr, 0);
	} else if(!strncmp(argv[a], "--end_seed=", 11)) {
	    end_seed = strtoll(argv[a] + 11, &endptr, 0);
	} else if(!strncmp(argv[a], "--work_unit=", 12)) {
	    work_unit = (int)strtoll(argv[a] + 12, &endptr, 0);
	} else if(!strncmp(argv[a], "--num_units=", 12)) {
	    num_units = (int)strtoll(argv[a] + 12, &endptr, 0);
	} else if(!strncmp(argv[a], "--quadhut_list=", 15)) {
	    quadhut_list_filename = argv[a] + 15;
	} else if(!strncmp(argv[a], "--num_threads=", 14)) {
	    num_threads = (int)strtoll(argv[a] + 14, &endptr, 0);
	} else if(!strcmp(argv[a], "--help") || !strcmp(argv[a], "-h")) {
	    usage();
	    exit(0);
	} else {
	    fprintf(stderr, "Unrecognized argument: %s\n", argv[a]);
	    usage();
	    exit(-1);
	}
    }

    if(work_unit >= 0 && work_unit < 256) {
	start_seed = 1099511627776*((int64_t)work_unit);
	end_seed = start_seed + 1099511627776*((int64_t)num_units);
    }

    FILE *quadhut_list = fopen(quadhut_list_filename, file_access_mode);

    if(quadhut_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", quadhut_list_filename);
	exit(-1);
    }
    
#ifdef USE_PTHREAD
    pthread_mutex_init(&ioMutex, NULL);
#else
    ioMutex = CreateMutex(NULL, FALSE, NULL);
#endif

    thread_id_t *threads = malloc(sizeof(thread_id_t) * num_threads);
    threadinfo *info = malloc(sizeof(threadinfo) * num_threads);
    int64_t seed_range = end_seed - start_seed;
    for(int t = 0; t < num_threads; ++t) {
	info[t].tid = t;
	info[t].start = start_seed + (t * seed_range / num_threads);
	info[t].end = start_seed + ((t + 1) * seed_range / num_threads);
	info[t].list = quadhut_list;
    }

#ifdef USE_PTHREAD
    for(int t = 0; t < num_threads; ++t) {
	pthread_create(&threads[t], NULL,
			findOriginStructuresThread, (void*)&info[t]);
    }

    for(int t = 0; t < num_threads; ++t) {
	pthread_join(threads[t], NULL);
    }
#else
    for(int t = 0; t < num_threads; ++t) {
	threads[t] = CreateThread(NULL, 0,
			findOriginStructuresThread, (LPVOID)&info[t],
			0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
#endif
    
    free(threads);
    free(info);
    fclose(quadhut_list);

    return 0;
}
