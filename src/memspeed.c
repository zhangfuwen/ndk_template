#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    int size = 200*1024*1024;
    char * p = (char*)malloc(size);

    for(int i = 0;i<size;i++) {
        p[i] = 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int total = 0;
    for(int i = 0; i< size;i+=4096*8) { // cache manager can prefetch cache lines, DRAM's row buffer size could be 2K
        char val = p[i];
        total += val;
    }
    printf("total is %d\n", total);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds and nanoseconds
    int elapsed = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
    printf("Time taken: %d ns, latency: %d ns\n", elapsed, elapsed/total);
    return 0;
}
