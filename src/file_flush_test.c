#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("File opened. Reading file to load into page cache...\n");

    // 使用 read() 读取整个文件，强制加载进 page cache
    char buffer[4096];
    ssize_t bytes_read;
    off_t total_read = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        total_read += bytes_read;
        printf("first word: %x\n", *(uint32_t*)buffer);
        // 可选：模拟处理数据
        // 这里只是简单累加字节数即可
    }

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Read %ld bytes from the file.\n", (long)total_read);

    // 尝试 fflush（注意：对只读 fd 没有效果）
    FILE *fp = fdopen(fd, "r");  // 将 fd 转换为 FILE*，以便使用 fflush
    if (!fp) {
        perror("fdopen");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Calling fflush on read-only stream (this should do nothing).\n");
    fflush(fp);  // 对只读流无效

    printf("Closing file...\n");
    fclose(fp);  // fclose 会自动调用 fflush，也无效

    printf("Done.\n");

    return 0;
}