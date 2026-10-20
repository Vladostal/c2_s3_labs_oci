#include "stdio.h"
#include "stdlib.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return -1;
    }
    int n = atoi(argv[1]);

    printf("%d\n", n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%s%d", j == 0 ? "" : " ", rand() % 19 - 9);
        }
        printf("\n");
    }
    return 0;
}
