#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <locale.h>

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    int fd = open(argv[1], O_RDONLY);
    off64_t offset = 0;
    off64_t new_offset = 0;
    while (1)
    {
        offset = lseek64(fd, offset, SEEK_HOLE);
        if (offset == -1)
            break;
        printf("Hole found at offset %'lu\n", offset);

        offset = lseek64(fd, offset, SEEK_DATA);
        if (offset == -1)
            break;
        char buf[32];
        read(fd, buf, 32);
        printf("Data found at offset %'lu: %s\n", offset, buf);
        // offset += 32;
        offset = lseek64(fd, -32, SEEK_CUR);
    }
    close(fd);
    return 0;
}
