#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PFN_MASK (((uint64_t)1 << 54) - 1)

typedef struct {
    unsigned long start;
    unsigned long end;
    char perms[5];
    off_t offset;
    char dev[6];
    int major, minor;
    long inode;
    char pathname[256];
} vma_t;

int parse_vma(FILE *fp, vma_t *vmas, int max_vmas) {
    int count = 0;
    char line[1024];

    while (fgets(line, sizeof(line), fp) && count < max_vmas) {
        vma_t *vma = &vmas[count++];
        sscanf(line, "%lx-%lx %4s %lx %s %d:%d %ld %255[^\n]",
               &vma->start, &vma->end, vma->perms, &vma->offset,
               vma->dev, &vma->major, &vma->minor, &vma->inode, vma->pathname);
    }
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return -1;
    }

    pid_t pid = atoi(argv[1]);

    // 打开 /proc/<pid>/maps
    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    FILE *fp_maps = fopen(maps_path, "r");
    if (!fp_maps) {
        perror("fopen maps");
        return -1;
    }

    // 读取 VMA 信息
    #define MAX_VMAS 1024
    vma_t vmas[MAX_VMAS];
    int vma_count = parse_vma(fp_maps, vmas, MAX_VMAS);
    fclose(fp_maps);

    // 打开 pagemap
    char pagemap_path[256];
    snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", pid);
    int fd_pagemap = open(pagemap_path, O_RDONLY);
    if (fd_pagemap < 0) {
        perror("open pagemap");
        return -1;
    }

    // 统计变量
    uint64_t present_count = 0;
    uint64_t absent_count = 0;
    uint64_t filepage_count = 0;
    uint64_t nonfilepage_count = 0;
    uint64_t file_shared_count = 0;
    uint64_t file_private_count = 0;
    uint64_t file_dirty_count = 0;
    uint64_t file_clean_count = 0;
    uint64_t file_private_clean_count = 0;
    uint64_t swapped_private_count = 0;
    uint64_t swapped_shared_count = 0;


    uint64_t private_file_dirty = 0, private_file_clean = 0;
    uint64_t shared_file_dirty = 0, shared_file_clean = 0;
    uint64_t anon_pages = 0;
    uint64_t total_private_file_plus_anon = 0;

    // 遍历每个 VMA
    for (int i = 0; i < vma_count; ++i) {
        vma_t *vma = &vmas[i];

        // 判断是否是文件映射
        int is_file = (vma->inode != 0 && 
                       strstr(vma->pathname, "[vvar]") == NULL &&
                       strstr(vma->pathname, "[vdso]") == NULL &&
                       strstr(vma->pathname, "[vsyscall]") == NULL);
        int is_stack_or_heap = strstr(vma->pathname, "[stack]") != NULL || 
                       strstr(vma->pathname, "[heap]") != NULL;

        int is_shared = (vma->perms[3] == 's');  // 's' 表示共享映射
        int is_writable = (vma->perms[1] == 'w');

        // 遍历这个 VMA 的所有虚拟页
        for (unsigned long vaddr = vma->start; vaddr < vma->end; vaddr += PAGE_SIZE) {
            uint64_t vpn = vaddr >> PAGE_SHIFT;
            off_t offset = vpn * sizeof(uint64_t);
            uint64_t entry;

            ssize_t bytes_read = pread(fd_pagemap, &entry, sizeof(entry), offset);
            if (bytes_read != sizeof(entry)) continue;

            // 判断是否在内存中
            int present = (entry >> 0) & 0x1;
            int swapped = (entry >> 1) & 0x1;
            if (present) {
                present_count++;
            } else {
                absent_count++;
            }
            if (!present) continue;

            int swapped_shared = (entry >> 57) & 0x1;
            int file_page = (entry >> 56) & 0x1;
            int soft_dirty = (entry >> 55) & 0x1;
            if (file_page) {
                filepage_count++;
                if (is_shared) {
                    file_shared_count++;
                } else {
                    file_private_count++;
                }

                if (soft_dirty) {
                    file_dirty_count++;
                } else {
                    file_clean_count++;
                }

                if ((!is_shared) && (!soft_dirty)) {
                    file_private_clean_count++;
                }
            } else {
                nonfilepage_count++;
            }

            if (swapped) {
                if (swapped_shared) {
                    swapped_shared_count++;
                } else {
                    swapped_private_count++;
                }
            }

            if (is_stack_or_heap && file_page) {
                perror("error: unexpected, stack or heap falls in file page\n");
            }

            if (!file_page) { // 匿名页
                anon_pages++;
                total_private_file_plus_anon++;
            } else if (is_file) {
                if (is_shared) {
                    if (soft_dirty)
                        shared_file_dirty++;
                    else
                        shared_file_clean++;
                } else {
                    if (is_writable) {
                        if (soft_dirty)
                            private_file_dirty++;
                        else
                            private_file_clean++;
                    } else {
                        private_file_clean++;  // 只读文件页
                    }
                    total_private_file_plus_anon++;
                }
            }
        }
    }

    close(fd_pagemap);

    // 输出结果
    printf("  Present: %lu pages (%.2f MB)\n", present_count, present_count * PAGE_SIZE / 1048576.0);
    printf("  Absent: %lu pages (%.2f MB)\n", absent_count, absent_count * PAGE_SIZE / 1048576.0);
    printf("\n");
    printf("  FilePage: %lu pages (%.2f MB)\n", filepage_count, filepage_count * PAGE_SIZE / 1048576.0);
    printf("    shared: %lu pages (%.2f MB)\n", file_shared_count, file_shared_count * PAGE_SIZE / 1048576.0);
    printf("    private: %lu pages (%.2f MB)\n", file_private_count, file_private_count * PAGE_SIZE / 1048576.0);
    printf("    dirty: %lu pages (%.2f MB)\n", file_dirty_count, file_dirty_count * PAGE_SIZE / 1048576.0);
    printf("    clean: %lu pages (%.2f MB)\n", file_clean_count, file_clean_count * PAGE_SIZE / 1048576.0);
    printf("    private & clean: %lu pages (%.2f MB)\n", file_private_clean_count, file_private_clean_count * PAGE_SIZE / 1048576.0);
    printf("  NonFilePage: %lu pages (%.2f MB)\n", nonfilepage_count, nonfilepage_count * PAGE_SIZE / 1048576.0);
    printf("  SwappedPrivate: %lu pages (%.2f MB)\n", swapped_private_count, swapped_private_count * PAGE_SIZE / 1048576.0);
    printf("  SwappedShared: %lu pages (%.2f MB)\n", swapped_shared_count, swapped_shared_count * PAGE_SIZE / 1048576.0);

    printf("Private File Pages:\n");
    printf("  Dirty: %lu pages (%.2f MB)\n", private_file_dirty, private_file_dirty * PAGE_SIZE / 1048576.0);
    printf("  Clean: %lu pages (%.2f MB)\n", private_file_clean, private_file_clean * PAGE_SIZE / 1048576.0);

    printf("Shared File Pages:\n");
    printf("  Dirty: %lu pages (%.2f MB)\n", shared_file_dirty, shared_file_dirty * PAGE_SIZE / 1048576.0);
    printf("  Clean: %lu pages (%.2f MB)\n", shared_file_clean, shared_file_clean * PAGE_SIZE / 1048576.0);

    printf("Anonymous Pages:\n");
    printf("  Total: %lu pages (%.2f MB)\n", anon_pages, anon_pages * PAGE_SIZE / 1048576.0);

    printf("Private File + Anonymous Total:\n");
    printf("  Total: %lu pages (%.2f MB)\n",
           total_private_file_plus_anon,
           total_private_file_plus_anon * PAGE_SIZE / 1048576.0);

    return 0;
}
