#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 19999
#define SCRATCH (1UL << 20)

void die_if(const char *prefix, int predicate)
{
    if (predicate) {
        puts(prefix);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    die_if("socket", listen_fd < 0);

    int one = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    };
    die_if("bind", bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0);
    die_if("listen", listen(listen_fd, 1) < 0);

    char *scratch = malloc(SCRATCH);
    die_if("malloc", scratch == NULL);

    for (;;) {
        int fd = accept(listen_fd, NULL, NULL);
        die_if("accept", fd < 0);
        for (;;) {
            ssize_t n = recv(fd, scratch, SCRATCH, 0);
            if (n <= 0) break;
        }
        close(fd);
    }
}
