#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/random.h>

int main()
{
    // int fd = open("/dev/urandom", O_RDONLY);
    int seed;
    // read(fd, &seed, sizeof(seed));
    getrandom(&seed, sizeof(seed), 0);
    srand(seed);
    for (int i = 0; i < 10; i++)
        printf("> %d\n", rand());
    // close(fd);
    return 0;
}
