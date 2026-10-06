#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"

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

int main() {
    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, stdin) != -1) {
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
        printf("%f\n", result);
        fflush(stdout);
    }
    free(line);
    return 0;
}
