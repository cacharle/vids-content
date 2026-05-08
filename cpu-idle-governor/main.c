// Fake latency-sensitive "application" used to demonstrate that deep
// CPU C-states cost wake-up latency.
//
// The program imitates a server that handles sparse requests: between
// requests the core sits idle long enough to drop into a deep idle
// state (C6 or similar). Each iteration we:
//   1. compute an absolute target wake-up time,
//   2. clock_nanosleep until that time,
//   3. record how late we actually woke (target - now),
//   4. do a small fixed amount of "work" to imitate handling.
//
// Step 3 is the C-state exit cost. With C-states enabled it shows up
// as a tail in the latency distribution; disable them with
//   sudo cpupower -c 1-3 idle-set -D 0
// and the tail collapses.
//
// Build:  gcc -O2 -Wall -Wextra -o app main.c
// Run:    sudo taskset -c 1 chrt -f 99 ./app
//         sudo taskset -c 1 chrt -f 99 ./app 500 100   # 500 iters, 100 ms

#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define NS_PER_SEC 1000000000L

static int64_t ts_diff_ns(const struct timespec *a, const struct timespec *b) {
    return (int64_t)(a->tv_sec  - b->tv_sec)  * NS_PER_SEC
         + (int64_t)(a->tv_nsec - b->tv_nsec);
}

static void ts_add_ns(struct timespec *t, int64_t ns) {
    t->tv_sec  += ns / NS_PER_SEC;
    t->tv_nsec += ns % NS_PER_SEC;
    if (t->tv_nsec >= NS_PER_SEC) {
        t->tv_sec  += 1;
        t->tv_nsec -= NS_PER_SEC;
    }
}

// Pretend to handle a request — small, predictable CPU burst. Volatile
// sink keeps the optimizer from deleting the loop.
static volatile uint64_t g_sink;
static void handle_request(void) {
    uint64_t acc = 0;
    for (int i = 0; i < 5000; i++) acc = acc * 1315423911u + i;
    g_sink = acc;
}

static int cmp_i64(const void *a, const void *b) {
    int64_t x = *(const int64_t *)a, y = *(const int64_t *)b;
    return (x > y) - (x < y);
}

int main(int argc, char **argv) {
    long iterations  = 500;
    long interval_ms = 100;  // long enough for the core to enter deep idle

    if (argc > 1) iterations  = strtol(argv[1], NULL, 10);
    if (argc > 2) interval_ms = strtol(argv[2], NULL, 10);
    if (iterations <= 0 || interval_ms <= 0) {
        fprintf(stderr, "usage: %s [iterations] [interval_ms]\n", argv[0]);
        return 1;
    }

    // Avoid page-fault jitter contaminating the measurement.
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
        perror("mlockall (continuing anyway)");

    int64_t *samples = calloc(iterations, sizeof(*samples));
    if (!samples) { perror("calloc"); return 1; }

    fprintf(stderr, "handling %ld fake requests, %ld ms idle between each "
                    "(~%ld s total)\n",
            iterations, interval_ms, (iterations * interval_ms) / 1000);

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    for (long i = 0; i < iterations; i++) {
        ts_add_ns(&next, (int64_t)interval_ms * 1000000L);

        int rc;
        do {
            rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        } while (rc == EINTR);
        if (rc) { errno = rc; perror("clock_nanosleep"); return 1; }

        struct timespec woke;
        clock_gettime(CLOCK_MONOTONIC, &woke);
        samples[i] = ts_diff_ns(&woke, &next);  // wake-up overshoot

        handle_request();
    }

    qsort(samples, iterations, sizeof(*samples), cmp_i64);
    int64_t min = samples[0];
    int64_t med = samples[iterations / 2];
    int64_t p99 = samples[(iterations * 99) / 100];
    int64_t max = samples[iterations - 1];
    int64_t sum = 0;
    for (long i = 0; i < iterations; i++) sum += samples[i];
    double avg = (double)sum / iterations;

    printf("\nwake-up latency over %ld requests (us):\n", iterations);
    printf("  min %7.1f   avg %7.1f   med %7.1f   p99 %7.1f   max %7.1f\n",
           min / 1000.0, avg / 1000.0, med / 1000.0, p99 / 1000.0,
           max / 1000.0);

    free(samples);
    return 0;
}
