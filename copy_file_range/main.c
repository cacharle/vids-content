#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

#define BUF_SIZE (128 * 1024)

int main(int argc, char **argv)
{
    printf("pid: %d\n", getpid());
    assert(argc == 3);
    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    int read_size = 0;
    char buf[BUF_SIZE];
    do {
        read_size = read(fd_in, buf, BUF_SIZE);
        assert(read_size != -1);
        write(fd_out, buf, read_size);
    } while (read_size > 0);

    close(fd_in);
    close(fd_out);
    return 0;
}
