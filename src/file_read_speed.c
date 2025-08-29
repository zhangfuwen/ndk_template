#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#define FILE_PATH "/sdcard/testfile.bin"
#define FILE_SIZE (1024 * 1024 * 1024) // 1GB
#define BLOCK_SIZE (4096) // 4KB (must be aligned for O_DIRECT)

void *aligned_alloc_block(size_t size, size_t alignment) {
    void *ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

double get_time_sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char *argv[]) {
    int fd;
    void *buf = aligned_alloc_block(BLOCK_SIZE, BLOCK_SIZE);
    if (!buf) {
        perror("aligned_alloc_block");
        return 1;
    }
    memset(buf, 0xAB, BLOCK_SIZE);

    int write_passes = 1;
    int read_passes = 1;

    // Parse flag-style parameters
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            write_passes = atoi(argv[++i]);
            if (write_passes < 1) write_passes = 1;
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            read_passes = atoi(argv[++i]);
            if (read_passes < 1) read_passes = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [-w WRITE_PASSES] [-r READ_PASSES]\n", argv[0]);
            free(buf);
            return 0;
        }
    }

    // --- Write Phase ---
    double total_write_time = 0.0;
    for (int wpass = 1; wpass <= write_passes; wpass++) {
        fd = open(FILE_PATH, O_CREAT | O_WRONLY | O_DIRECT, 0644);
        if (fd < 0) {
            perror("open for write");
            free(buf);
            return 1;
        }

        size_t written = 0;
        double start = get_time_sec();
        while (written < FILE_SIZE) {
            ssize_t rc = write(fd, buf, BLOCK_SIZE);
            if (rc < 0) {
                perror("write");
                close(fd);
                free(buf);
                return 1;
            }
            written += rc;
        }
        fsync(fd);
        double end = get_time_sec();
        double duration = end - start;
        double speed = (double)FILE_SIZE / (1024 * 1024) / duration;
        total_write_time += duration;
        printf("Write pass %d: %.2f seconds, %.2f MB/s\n", wpass, duration, speed);
        close(fd);
    }
    double total_write_speed = ((double)FILE_SIZE * write_passes) / (1024 * 1024) / total_write_time;
    printf("Total write: %.2f seconds, average speed: %.2f MB/s\n", total_write_time, total_write_speed);

    // --- Read Phase ---
    double total_read_time = 0.0;
    for (int rpass = 1; rpass <= read_passes; rpass++) {
        fd = open(FILE_PATH, O_RDONLY | O_DIRECT);
        if (fd < 0) {
            perror("open for read");
            free(buf);
            return 1;
        }

        size_t read_bytes = 0;
        double start = get_time_sec();
        while (read_bytes < FILE_SIZE) {
            ssize_t rc = read(fd, buf, BLOCK_SIZE);
            if (rc < 0) {
                perror("read");
                close(fd);
                free(buf);
                return 1;
            }
            if (rc == 0) break; // EOF
            read_bytes += rc;
        }
        double end = get_time_sec();
        double duration = end - start;
        double speed = (double)FILE_SIZE / (1024 * 1024) / duration;
        total_read_time += duration;
        printf("Read pass %d: %.2f seconds, %.2f MB/s\n", rpass, duration, speed);
        close(fd);
    }
    double total_read_speed = ((double)FILE_SIZE * read_passes) / (1024 * 1024) / total_read_time;
    printf("Total read: %.2f seconds, average speed: %.2f MB/s\n", total_read_time, total_read_speed);

    free(buf);
    return 0;
}
