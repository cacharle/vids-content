#define _GNU_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Minimal TCP sink for the proxy benchmark: accept one connection,
 * splice everything into a regular output file until EOF. Zero
 * user-space copies so the proxy, not the sink, is the bottleneck.
 */

static void die_if(const char *prefix, int predicate)
{
    if (predicate) { perror(prefix); exit(EXIT_FAILURE); }
}

#define CHUNK (1 << 16)

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s PORT OUTFILE\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    const char *out_path = argv[2];

    int lsock = socket(AF_INET, SOCK_STREAM, 0);
    die_if("socket", lsock == -1);
    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    };
    die_if("bind", bind(lsock, (struct sockaddr *)&addr, sizeof(addr)) == -1);
    die_if("listen", listen(lsock, 1) == -1);

    int c = accept(lsock, NULL, NULL);
    die_if("accept", c == -1);
    close(lsock);

    int pipefd[2];
    die_if("pipe", pipe(pipefd) == -1);
    int out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    die_if("open outfile", out == -1);

    for (;;) {
        ssize_t n = splice(c, NULL, pipefd[1], NULL, CHUNK, SPLICE_F_MOVE);
        die_if("splice recv", n == -1);
        if (n == 0) break;
        while (n > 0) {
            ssize_t m = splice(pipefd[0], NULL, out, NULL, n, SPLICE_F_MOVE);
            die_if("splice write", m == -1);
            n -= m;
        }
    }

    close(c); close(pipefd[0]); close(pipefd[1]); close(out);
    return 0;
}
