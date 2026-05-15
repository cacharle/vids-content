#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ITERATIONS 10000000000

int main(int argc, char **argv)
{
    int niceness = atoi(argv[1]);
    int ret = nice(niceness);
    if (ret == -1)
        perror("nice");

    int sum = 0;
    for (size_t i = 0; i < ITERATIONS; i++) {
        sum += i;
    }
    printf("sum: %d\n", sum);

    return 0;
}
