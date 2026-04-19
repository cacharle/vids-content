#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int out = open("copy.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t n;

    while ((n = tee(0, 1, 65536, 0)) > 0) {
        // tee copied stdin -> stdout pipe buffer (no consumption)
        // now actually consume stdin by splicing it to the file
        while (n > 0) {
            ssize_t m = splice(0, NULL, out, NULL, n, SPLICE_F_MOVE);
            if (m <= 0) break;
            n -= m;
        }
    }
    close(out);
}
