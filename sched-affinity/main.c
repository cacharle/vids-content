#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#define ITERATIONS 4000000000

int main(int argc, char **argv)
{
    int cpu = atoi(argv[1]);

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    sched_setaffinity(0, sizeof cpuset, &cpuset);

    size_t iterations = ITERATIONS;
    __asm__ volatile("" : "+r"(iterations));

    for (int r = 0; r < 10; r++) {
        long sum = 0;
        for (size_t i = 0; i < iterations; i++) {
            sum += i;
        }
        printf("sum: %ld\n", sum);
    }
    return 0;
}
