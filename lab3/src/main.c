#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_N 16

int n;
double *a;
int next_task = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int read_matrix() {
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_N) {
        fprintf(stderr, "matrix size from 1 to %d expected\n", MAX_N);
        return -1;
    }
    a = malloc(n * n * sizeof(double));
    if (a == NULL) {
        perror("malloc");
        return -1;
    }
    for (int i = 0; i < n * n; i++) {
        if (scanf("%lf", &a[i]) != 1) {
            fprintf(stderr, "invalid matrix element\n");
            free(a);
            return -1;
        }
    }
    return 0;
}

double sum_permutations(int row, int used, double product) {
    if (row == n) {
        return product;
    }
    double sum = 0;
    for (int col = 0; col < n; col++) {
        if (used & (1 << col)) {
            continue;
        }
        double term = sum_permutations(row + 1, used | (1 << col), product * a[row * n + col]);
        int inversions = __builtin_popcount(used >> (col + 1));
        sum += inversions % 2 == 0 ? term : -term;
    }
    return sum;
}

void *worker(void *arg) {
    double *result = arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        int task = next_task++;
        pthread_mutex_unlock(&mutex);
        if (task >= n * n) {
            break;
        }

        int col0 = task / n;
        int col1 = task % n;
        if (col0 == col1) {
            continue;
        }
        double term = sum_permutations(2, (1 << col0) | (1 << col1), a[col0] * a[n + col1]);
        *result += col1 > col0 ? term : -term;
    }
    return NULL;
}

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main(int argc, char **argv) {
    if (argc != 2 || atoi(argv[1]) <= 0) {
        fprintf(stderr, "usage: %s max_threads < input\n", argv[0]);
        return -1;
    }
    int threads_count = atoi(argv[1]);

    if (read_matrix() == -1) {
        return -1;
    }
    if (n == 1) {
        printf("%.10g\n", a[0]);
        free(a);
        return 0;
    }

    if (threads_count > n * (n - 1)) {
        threads_count = n * (n - 1);
    }
    pthread_t threads[threads_count];
    double results[threads_count];

    double start = get_time_ms();
    for (int i = 0; i < threads_count; i++) {
        results[i] = 0;
        if (pthread_create(&threads[i], NULL, worker, &results[i]) != 0) {
            perror("pthread_create");
            exit(-1);
        }
    }
    double det = 0;
    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
        det += results[i];
    }
    double finish = get_time_ms();

    printf("%.10g\n", det);
    fprintf(stderr, "time: %.3f ms\n", finish - start);

    free(a);
    return 0;
}
