#define _GNU_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Minimal TCP client for the proxy benchmark: connect to HOST:PORT and
 * sendfile() FILE to the socket. Zero user-space copies so the proxy,
 * not the client, is the bottleneck.
 */

static void die_if(const char *prefix, int predicate)
{
    if (predicate) { perror(prefix); exit(EXIT_FAILURE); }
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s HOST PORT FILE\n", argv[0]);
        return 1;
    }
    const char *host = argv[1];
    int port = atoi(argv[2]);
    const char *path = argv[3];

    int file_fd = open(path, O_RDONLY);
    die_if("open", file_fd == -1);
    struct stat st;
    die_if("stat", fstat(file_fd, &st) == -1);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    die_if("socket", fd == -1);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };
    die_if("inet_pton", inet_pton(AF_INET, host, &addr.sin_addr) != 1);
    die_if("connect", connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1);

    off_t remaining = st.st_size;
    while (remaining > 0) {
        ssize_t n = sendfile(fd, file_fd, NULL, remaining);
        die_if("sendfile", n == -1);
        remaining -= n;
    }

    close(fd); close(file_fd);
    return 0;
}
