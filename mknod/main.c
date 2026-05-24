#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <stdio.h>

int main()
{
    umask(0);
    mknod("my_null", 0755 | S_IFCHR, makedev(1, 3));
    mknod("my_zero", 0644 | S_IFCHR, makedev(1, 5));
    mknod("my_urandom", 0755 | S_IFCHR, makedev(1, 9));
    mknod("my_nvme", 0755 | S_IFBLK, makedev(259, 3));

    // vvv used in mkfifo
    mknod("my_fifo", 0755 | S_IFIFO, 0);
    mknod("my_reg", 0755 | S_IFREG, 0);

    perror("NULL");
    return 0;
}
