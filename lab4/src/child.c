#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int divide_line(char *line, float *result) {
    char *end;
    *result = strtof(line, &end);
    if (end == line) {
        fprintf(stderr, "child: invalid number, exit\n");
        return -1;
    }
    while (1) {
        line = end;
        float next_num = strtof(line, &end);
        if (line == end) {
            break;
        }
        if (next_num == 0) {
            fprintf(stderr, "child: division by zero, exit\n");
            return -1;
        }
        *result /= next_num;
    }
    while (isspace((unsigned char) *end)) {
        end++;
    }
    if (*end != '\0') {
        fprintf(stderr, "child: invalid number, exit\n");
        return -1;
    }
    return 0;
}

int process_lines(char *data, size_t size) {
    char *line = NULL;
    size_t line_size = 0;
    size_t pos = 0;
    while (getline(&line, &line_size, stdin) != -1) {
        char *p = line;
        while (isspace((unsigned char) *p)) {
            p++;
        }
        if (*p == '\0') {
            continue;
        }

        float result;
        if (divide_line(line, &result) == -1) {
            free(line);
            return -1;
        }
        int count = snprintf(data + pos, size - pos, "%f\n", result);
        if (count < 0 || (size_t) count >= size - pos) {
            fprintf(stderr, "child: shared file is too small, exit\n");
            free(line);
            return -1;
        }
        pos += count;
    }
    free(line);
    return 0;
}

int main() {
    struct stat info;
    if (fstat(STDOUT_FILENO, &info) == -1) {
        perror("child: fstat");
        return -1;
    }
    size_t size = info.st_size;
    char *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, STDOUT_FILENO, 0);
    if (data == MAP_FAILED) {
        perror("child: mmap");
        return -1;
    }

    int result = process_lines(data, size);
    if (munmap(data, size) == -1) {
        perror("child: munmap");
        return -1;
    }
    return result;
}
