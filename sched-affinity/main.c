#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#define ITERATIONS 40000000000

int main(int argc, char **argv)
{
    int cpu = atoi(argv[1]);

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    sched_setaffinity(0, sizeof cpuset, &cpuset);

    long sum = 0;
    #pragma GCC ivdep
    for (size_t i = 0; i < ITERATIONS; i++) {
        sum += i;
    }
    printf("sum: %ld\n", sum);
    return 0;
}
