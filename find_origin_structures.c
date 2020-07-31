#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "finders.h"

#define DEFAULT_START_SEED 0
#define DEFAULT_END_SEED 281474976710656
#define DEFAULT_QUADHUT_LIST "./seeds/quadhut_bases.txt"
#define DEFAULT_TRIHUT_LIST "./seeds/trihut_bases.txt"
#define DEFAULT_MONUMENT_LIST "./seeds/monument_bases.txt"

int main(int argc, char *argv[]) {
    int64_t start_seed = DEFAULT_START_SEED;
    int64_t end_seed = DEFAULT_END_SEED;
    char *quadhut_list_filename = DEFAULT_QUADHUT_LIST;
    char *trihut_list_filename = DEFAULT_TRIHUT_LIST;
    char *monument_list_filename = DEFAULT_MONUMENT_LIST;
    char *file_access_mode = "w";
    
    char *endptr;
    for(int a = 1; a < argc; ++a) {
	if(!strncmp(argv[a], "--start_seed=", 13)) {
	    start_seed = strtoll(argv[a] + 13, &endptr, 0);
	} else if(!strncmp(argv[a], "--end_seed=", 11)) {
	    end_seed = strtoll(argv[a] + 11, &endptr, 0);
	} else if(!strncmp(argv[a], "--quadhut_list=", 15)) {
	    quadhut_list_filename = argv[a] + 15;
	} else if(!strncmp(argv[a], "--trihut_list=", 14)) {
	    trihut_list_filename = argv[a] + 14;
	} else if(!strncmp(argv[a], "--monument_list=", 16)) {
	    monument_list_filename = argv[a] + 16;
	} else if(!strcmp(argv[a], "--append") || !strcmp(argv[a], "-a")) {
	    file_access_mode = "a";
	}
    }

    FILE *quadhut_list = fopen(quadhut_list_filename, file_access_mode);
    FILE *trihut_list = fopen(trihut_list_filename, file_access_mode);
    FILE *monument_list = fopen(monument_list_filename, file_access_mode);

    int open = 1;
    if(quadhut_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", quadhut_list_filename);
	open = 0;
    }
    if(trihut_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", trihut_list_filename);
	open = 0;
    }
    if(monument_list == NULL) {
	fprintf(stderr, "Could not open \"%s\"\n", monument_list_filename);
	open = 0;
    }
    if(!open) {
	exit(-1);
    }

    for(int64_t seed = start_seed; seed < end_seed; ++seed) {
	//go add seed checking here
    }

    fclose(quadhut_list);
    fclose(trihut_list);
    fclose(monument_list);
}
