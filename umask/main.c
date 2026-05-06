#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    umask(0066);
    // unlink("foo");
    // int fd = open("foo", O_TRUNC | O_WRONLY | O_CREAT, 0777);
    // write(fd, "hello", 5);
    // close(fd);

    rmdir("bar");
    mkdir("bar", 0777);

    return 0;
}

// umask shell builtin so it works with redirections
