#define _GNU_SOURCE
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <systemd/sd-daemon.h>

void die_if(const char *prefix, int predicate)
{
    if (predicate) {
        puts(prefix);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

typedef union {
    int array[2];
    struct {
        int read_fd;
        int write_fd;
    };
} pipe_t;

/*
 * Per chunk:
 *
 *                 splice
 *  stdin (pipe) ---------> stdout
 *             |
 *         tee |
 *               ---------> p (pipe) -----------> fd (file)
 *                                    splice
 */

#define BUF_SIZE (1 << 20)

int main(int argc, char **argv)
{
    assert(argc > 1);
    int file_count = argc - 1;
    int *fds = malloc(sizeof(int) * file_count);
    for (int i = 0; i < file_count; i++) {
        fds[i] = open(argv[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
        die_if("fd1", fds[i] == -1);
    }
    pipe_t *ps = malloc(sizeof(pipe_t) * file_count);
    for (int i = 0; i < file_count; i++)
        pipe(ps[i].array);

    for (;;)
    {
        for (int i = 0; i < file_count; i++) {
            ssize_t copied_size = tee(
                STDIN_FILENO, ps[i].write_fd, BUF_SIZE, 0);
            die_if("tee", copied_size == -1);
            if (copied_size == 0)
                goto end;
        }

        for (int i = 0; i < file_count; i++) {
            ssize_t spliced_size2 = splice(
                ps[i].read_fd, NULL, fds[i], NULL, BUF_SIZE, 0);
            die_if("splice 2", spliced_size2 == -1);
        }

        ssize_t spliced_size = splice(
            STDIN_FILENO, NULL, STDOUT_FILENO, NULL,
            BUF_SIZE, 0);
        die_if("splice 1", spliced_size == -1);
    }
end:

    for (int i = 0; i < file_count; i++) {
        close(ps[i].read_fd);
        close(ps[i].write_fd);
        close(fds[i]);
    }
    return 0;
}
