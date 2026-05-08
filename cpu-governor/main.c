#include <stdio.h>
#include <stdint.h>
#include <time.h>

int main(void) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < 2'000'000'000ULL; i++) {
        x += i * i;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double s = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    printf("%.3f s  (x=%lu)\n", s, (unsigned long)x);
    return 0;
}
