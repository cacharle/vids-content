#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

void die_if(const char *prefix, int predicate)
{
    if (predicate) {
        puts(prefix);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

#define PORT  19999
#define CHUNK (1UL << 20) /* 1 MiB */
#define TOTAL (4UL << 30) /* 4 GiB */

int main(void)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    die_if("socket", sock_fd < 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    };
    die_if("connect", connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0);

    char *data = mmap(
        NULL,
        CHUNK,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    die_if("mmap", data == MAP_FAILED);
    memset(data, 'a', CHUNK);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (size_t sent = 0; sent < TOTAL;) {
        size_t remaining = TOTAL - sent;
        size_t want = remaining < CHUNK ? remaining : CHUNK;
        ssize_t n = write(sock_fd, data, want);
        die_if("write", n <= 0);
        sent += n;
    }

    close(sock_fd);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double secs = (end.tv_sec - start.tv_sec)
                + (end.tv_nsec - start.tv_nsec) / 1e9;
    double gib = (double)TOTAL / (double)(1UL << 30);
    printf("write:    %.2f GiB in %.3f s -> %.2f GiB/s\n", gib, secs, gib / secs);
    return 0;
}
