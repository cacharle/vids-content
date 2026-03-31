#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#define CHUNK_SIZE (1024 * 1024 * 1024)  /* 1 GiB — no userspace buffer, so go big */

int main(int argc, char **argv)
{
    assert(argc == 3);
    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    ssize_t copied_bytes;
    do {
        copied_bytes = copy_file_range(fd_in, NULL, fd_out, NULL, CHUNK_SIZE, 0);
    } while (copied_bytes > 0);

    close(fd_in);
    close(fd_out);
    return 0;
}
