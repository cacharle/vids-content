#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

/*
 * TCP broadcaster: one source connection fanned out to two subscribers.
 *
 *                    splice
 *   source (tcp) -----------> main_p --[tee]----> sub_p --[splice]--> sub0
 *                                \
 *                                 \-[splice]--------------------------> sub1
 *
 * main_p holds the bytes just once. tee() duplicates the refcounted
 * pipe pages into sub_p without copying payload. sub1 consumes main_p
 * directly, which drains it.
 */

#define LISTEN_PORT 19000
#define SUB0_PORT   19001
#define SUB1_PORT   19002
#define PIPE_SIZE   (1 << 20)
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

    int main_p[2], sub_p[2];
    pipe(main_p);
    pipe(sub_p);

    for (;;) {
        ssize_t n = splice(source, NULL, main_p[1], NULL, CHUNK, SPLICE_F_MOVE);
        if (n == 0)
            break;

        ssize_t t = tee(main_p[0], sub_p[1], n, 0);
        assert(t == n);

        ssize_t s = splice(sub_p[0], NULL, sub0, NULL, n, 0);
        assert(s == n);
        s = splice(main_p[0], NULL, sub1, NULL, n, 0);
        assert(s == n);
    }

    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    double u = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
    double s = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    fprintf(stderr, "cpu user=%.3fs sys=%.3fs total=%.3fs\n", u, s, u + s);
    return 0;
}
