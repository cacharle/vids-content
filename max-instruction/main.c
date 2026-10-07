#include <stdio.h>
#include <limits.h>
#include <immintrin.h>
#include <time.h>

// gcc -O2 -fno-if-conversion -fno-if-conversion2 main.c && ./a.out
// gcc -O3 main.c && ./a.out
// https://godbolt.org/z/vso61hbec

void max_naive(double *xs, double *ys, double *zs, size_t n)
{
    for (size_t i = 0; i < n; i++)
        zs[i] = xs[i] < ys[i] ? xs[i] : ys[i];
}

void max_faster(double *xs, double *ys, double *zs, size_t n)
{
    for (size_t i = 1; i < n; i++) {
        __m128d maximum = _mm_max_sd(_mm_set_sd(xs[i]), _mm_set_sd(ys[i]));
        _mm_store_sd(&zs[i], maximum);
    }
}

int main()
{
    srand(1);
    size_t n = 1000024;
    double *xs = malloc(n * sizeof(double));
    double *ys = malloc(n * sizeof(double));
    double *zs = malloc(n * sizeof(double));
    for (size_t i = 0; i < n; i++) {
        xs[i] = (double)(rand() % 100000) / 1000.0;
        ys[i] = (double)(rand() % 100000) / 1000.0;
    }

    struct timespec start1, start2, end1, end2;
    clock_gettime(CLOCK_MONOTONIC, &start1);
    max_naive(xs, ys, zs, n);
    clock_gettime(CLOCK_MONOTONIC, &end1);
    clock_gettime(CLOCK_MONOTONIC, &start2);
    max_faster(xs, ys, zs, n);
    clock_gettime(CLOCK_MONOTONIC, &end2);

    double secs1 = (end1.tv_sec - start1.tv_sec)
                + (end1.tv_nsec - start1.tv_nsec) / 1e9;
    double secs2 = (end2.tv_sec - start2.tv_sec)
                + (end2.tv_nsec - start2.tv_nsec) / 1e9;

    printf("max1: %f\n", zs[0]);
    printf("max2: %f\n", zs[0]);

    printf("duration1: %f\n", secs1);
    printf("duration2: %f\n", secs2);
}
