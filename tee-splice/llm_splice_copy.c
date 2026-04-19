#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int in = open(argv[1], O_RDONLY);
    int out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int pipefd[2];
    pipe(pipefd);

    ssize_t n;
    while ((n = splice(in, NULL, pipefd[1], NULL, 65536, 0)) > 0) {
        splice(pipefd[0], NULL, out, NULL, n, 0);
    }

    close(in); close(out);
    close(pipefd[0]); close(pipefd[1]);
}
