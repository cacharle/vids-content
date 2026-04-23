#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

void die_if(const char *prefix, int predicate)
{
    if (predicate) {
        perror(prefix);
        exit(EXIT_FAILURE);
    }
}

#define CHUNK (1 << 16)

static void write_all(int fd, const char *buf, size_t n, const char *prefix)
{
    while (n > 0) {
        ssize_t w = write(fd, buf, n);
        die_if(prefix, w == -1);
        buf += w;
        n -= w;
    }
}

static int listen_on(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    die_if("socket", fd == -1);
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    die_if("bind", bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1);
    die_if("listen", listen(fd, 1) == -1);
    return fd;
}

static int connect_sub(const char *host_port)
{
    char buf[256];
    strncpy(buf, host_port, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    char *colon = strchr(buf, ':');
    assert(colon);
    *colon = 0;
    const char *host = buf;
    const char *port = colon + 1;

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo(%s): %s\n", host_port, gai_strerror(rc));
        exit(EXIT_FAILURE);
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    die_if("socket sub", fd == -1);
    die_if("connect sub", connect(fd, res->ai_addr, res->ai_addrlen) == -1);
    freeaddrinfo(res);
    return fd;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s LISTEN_PORT sub1_host:port [sub2_host:port ...]\n",
                argv[0]);
        return 1;
    }
    int listen_port = atoi(argv[1]);
    int N = argc - 2;

    int lsock = listen_on(listen_port);
    int source = accept(lsock, NULL, NULL);
    die_if("accept", source == -1);
    close(lsock);

    int *subs = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++)
        subs[i] = connect_sub(argv[2 + i]);

    char *buf = malloc(CHUNK);

    for (;;) {
        ssize_t n = read(source, buf, CHUNK);
        die_if("read source", n == -1);
        if (n == 0)
            break;
        for (int i = 0; i < N; i++)
            write_all(subs[i], buf, n, "write sub");
    }

    for (int i = 0; i < N; i++)
        close(subs[i]);
    close(source);
    free(buf);
    free(subs);

    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    double u = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
    double s = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    fprintf(stderr, "cpu user=%.3fs sys=%.3fs total=%.3fs\n", u, s, u + s);
    return 0;
}
