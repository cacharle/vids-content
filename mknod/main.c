#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <stdio.h>

int main()
{
    // mknod("my_null", 0755 | S_IFCHR, makedev(1, 3));
    // mknod("my_zero", 0755 | S_IFCHR, makedev(1, 5));
    // mknod("my_urandom", 0755 | S_IFCHR, makedev(1, 9));
    // // mknod("my_ram", 0755 | S_IFBLK, makedev(1, 0));
    // mknod("my_zram", 0755 | S_IFBLK, makedev(252, 0));

    // // vvv used in mkfifo
    // mknod("my_fifo", 0755 | S_IFIFO, 0);
    // perror(NULL);

    mknod("my_reg", 0755 | S_IFREG, 0);
    return 0;
}
