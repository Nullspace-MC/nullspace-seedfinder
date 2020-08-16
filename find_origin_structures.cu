#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_START_SEED 0
#define DEFAULT_END_SEED 281474976710656
#define DEFAULT_QUADHUT_LIST "./seeds/quadhut_bases.txt"

#define cudaCheckErrors(msg) \
    do { \
	cudaError_t __err = cudaGetLastError(); \
	if(__err != cudaSuccess) { \
	    fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
		msg, cudaGetErrorString(__err), \
		__FILE__, __LINE__); \
	    exit(1);\
	} \
    } while(0)

/* queues the given seed in a buffer for a host thread to write the seed to
 * the output file
 */
__device__ void queueForOutput(int64_t seed, volatile int64_t *buf,
		    int thread_id) {
    while(buf[thread_id] != -1);
    buf[thread_id] = seed;
}

/* returns the square of the distance between two points */
__device__ int distanceSquared(int2 *p1, int2 *p2) {
    const int dx = p2->x - p1->x;
    const int dy = p2->y - p1->y;
    return (dx * dx) + (dy * dy);
}

/* returns the coordinates of the structure of a given region */
__device__ int2 getStructurePos(int64_t seed, int regX, int regZ) {
    // set seed
    seed = regX * 341873128712 + regZ * 132897987541 + seed + 14357617;
    seed = (seed ^ 0x5deece66dLL);

    // get chunk x within region
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    int x = (int)(seed >> 17) % 24;

    // get chunk z within region
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    int z = (int)(seed >> 17) % 24;

    // get block position from chunk position
    x = ((regX * 32 + x) << 4) + 9;
    z = ((regZ * 32 + z) << 4) + 9;
    return make_int2(x, z);
}

/* searches the base seed space for quad structures */
__global__ void findOriginStructures(int64_t start, int64_t range,
		    volatile int64_t *buf) {
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int num_threads = gridDim.x * blockDim.x;
    
    const int64_t thread_start = start + (thread_id * range / num_threads);
    const int64_t thread_end = start + ((thread_id + 1) * range / num_threads);

    for(int64_t seed = thread_start; seed < thread_end; ++seed) {
	// get hut chunks
	int2 huts[4];
	huts[0] = getStructurePos(seed, 0, 0);
	huts[1] = getStructurePos(seed, 0, 1);
	huts[2] = getStructurePos(seed, 1, 0);
	huts[3] = getStructurePos(seed, 1, 1);

	// quadhut check
	int2 center = make_int2(
	    (huts[0].x + huts[1].x + huts[2].x + huts[3].x) / 4,
	    (huts[0].y + huts[1].y + huts[2].y + huts[3].y) / 4
	);
	int isQuadhut = 1;
	for(int i = 0; i < 4; ++i) {
	    if(distanceSquared(&huts[i], &center) > 16384) {
		isQuadhut = 0;
		break;
	    }
	}

	if(isQuadhut) {
	    queueForOutput(seed, buf, thread_id);
	}
    }
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
}

int main(int argc, char *argv[]) {
    int64_t start_seed = DEFAULT_START_SEED;
    int64_t end_seed = DEFAULT_END_SEED;
    char *quadhut_list_filename = DEFAULT_QUADHUT_LIST;
    char *file_access_mode = "a";
    
    int work_unit = -1;
    int num_units = 1;

    // parse args
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
    int64_t seed_range = end_seed - start_seed;

    FILE *quadhut_list = fopen(quadhut_list_filename, file_access_mode);

    if(quadhut_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", quadhut_list_filename);
	exit(-1);
    }

    int grid_size, block_size, thread_cnt;
    cudaDeviceGetAttribute(&grid_size, cudaDevAttrMultiProcessorCount, 0);
    block_size = 1024;
    cudaCheckErrors("cudaDeviceGetAttribute fail");
    thread_cnt = grid_size * block_size;

    // buffer setup
    cudaSetDeviceFlags(cudaDeviceMapHost);
    volatile int64_t *h_buf, *d_buf;
    cudaHostAlloc((void**)&h_buf, sizeof(int64_t) * thread_cnt,
	cudaHostAllocMapped);
    cudaHostGetDevicePointer((void**)&d_buf, (void*)h_buf, 0);
    cudaCheckErrors("cudaHostAlloc fail");
    for(int t = 0; t < thread_cnt; ++t) {
	h_buf[t] = -1;
    }

    // stream setup
    cudaStream_t streamk;
    cudaStreamCreate(&streamk);
    cudaCheckErrors("cudaStreamCreate fail");

    // kernel call
    cudaEvent_t eventk;
    cudaEventCreateWithFlags(&eventk, cudaEventDisableTiming);
    findOriginStructures<<<grid_size, block_size, 0, streamk>>>(start_seed,
	seed_range, d_buf);
    cudaCheckErrors("kernel launch fail");
    cudaEventRecord(eventk, streamk);

    // writing buffer contents to output
    while(cudaEventQuery(eventk) == cudaErrorNotReady) {
	for(int t = 0; t < thread_cnt; ++t) {
	    if(h_buf[t] != -1) {
		int64_t seed = h_buf[t];
		printf("%lld\n", seed);
		fprintf(quadhut_list, "%lld\n", seed);
		fflush(quadhut_list);

		h_buf[t] = -1;
	    }
	}
    }
    // clean out buffer after kernel finishes
    for(int t = 0; t < thread_cnt; ++t) {
	if(h_buf[t] != -1) {
	    int64_t seed = h_buf[t];
	    printf("%lld\n", seed);
	    fprintf(quadhut_list, "%lld\n", seed);
	    fflush(quadhut_list);

	    h_buf[t] = -1;
	}
    }

    cudaFree((void*)h_buf);

    return 0;
}
