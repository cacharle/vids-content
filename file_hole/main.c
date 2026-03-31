#define _LARGEFILE64_SOURCE
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int main()
{
    int fd = open("foo", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    for (int i = 0; i < 10; i++) {
        lseek64(fd, 1000ull * 1000ull * 1000000ull, SEEK_CUR);
        char buf[32];
        memset(buf, 0, 32);
        sprintf(buf, "i=%d", i);
        write(fd, buf, 32);
        lseek64(fd, -32, SEEK_CUR);
    }
    close(fd);
    return 0;
}
