#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    posix_fadvise(fd, 0, size, POSIX_FADV_DONTNEED);

    printf("Evicted %s from page cache\n", filename);
    close(fd);
    return 0;
}