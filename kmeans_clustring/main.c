#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

void kmeansImplementation(char* dataFileName, int numClusters);

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Error! Usage: ./kmeans <data-file-name> <number-of-clusters>\n");
        return 1;
    }

    errno = 0;
    char *end = NULL;
    long numClusters =  strtol(argv[2], &end, 10);            

    if (end == argv[2] || *end != '\0' || errno == ERANGE ||
        numClusters < 0 || numClusters > INT_MAX) {
        fprintf(stderr, "Error: <number-of-clusters> must be a non-negative integer.\n");
        return 1;
    }

    kmeansImplementation(argv[1], (int)numClusters);  
    return 0;
}
