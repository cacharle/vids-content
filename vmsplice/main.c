#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

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

#define SIZE 16

int main()
{
    int fd = open("foo", O_WRONLY | O_TRUNC | O_CREAT, 0644);
    pipe_t p;
    pipe(p.array);

    char *data = mmap(
        NULL,
        SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    assert(data != MAP_FAILED);
    memset(data, 'a', SIZE);

    struct iovec iov[] = {
        { .iov_base = data, .iov_len = SIZE }
    };
    vmsplice(p.write_fd, iov, 1, SPLICE_F_GIFT);
    splice(p.read_fd, NULL, fd, NULL, SIZE, 0);

    memset(data, '#', SIZE);
    vmsplice(p.write_fd, iov, 1, SPLICE_F_GIFT);
    splice(p.read_fd, NULL, fd, NULL, SIZE, 0);

    return 0;
}
