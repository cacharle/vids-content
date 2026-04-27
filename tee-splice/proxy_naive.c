#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define LISTEN_PORT 19000
#define SUB0_PORT   19001
#define SUB1_PORT   19002
#define CHUNK       (1 << 16)

static int connect_to(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    };
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    return fd;
}

int main(void)
{
    int lsock = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in laddr = {
        .sin_family = AF_INET,
        .sin_port = htons(LISTEN_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    bind(lsock, (struct sockaddr *)&laddr, sizeof(laddr));
    listen(lsock, 1);
    int source = accept(lsock, NULL, NULL);

    int sub0 = connect_to(SUB0_PORT);
    int sub1 = connect_to(SUB1_PORT);

    char buf[CHUNK];
    for (;;) {
        ssize_t n = read(source, buf, CHUNK);
        if (n == 0)
            break;
        ssize_t w = write(sub0, buf, n);
        assert(w == n);
        w = write(sub1, buf, n);
        assert(w == n);
    }

    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    double u = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
    double s = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    fprintf(stderr, "cpu user=%.3fs sys=%.3fs total=%.3fs\n", u, s, u + s);
    return 0;
}
