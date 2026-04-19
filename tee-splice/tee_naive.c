#define _GNU_SOURCE
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void die_if(const char *prefix, int predicate)
{
    if (predicate) {
        puts(prefix);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

static void write_all(int fd, const char *buf, size_t count, const char *prefix)
{
    while (count > 0) {
        ssize_t written = write(fd, buf, count);
        die_if(prefix, written == -1);
        buf += written;
        count -= written;
    }
}

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

    char *buf = malloc(BUF_SIZE);

    for (;;)
    {
        ssize_t read_size = read(STDIN_FILENO, buf, BUF_SIZE);
        die_if("read", read_size == -1);
        if (read_size == 0)
            break;

        write_all(STDOUT_FILENO, buf, read_size, "write stdout");

        for (int i = 0; i < file_count; i++)
            write_all(fds[i], buf, read_size, "write file");
    }

    for (int i = 0; i < file_count; i++)
        close(fds[i]);
    free(buf);
    free(fds);
    return 0;
}
