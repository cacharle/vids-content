#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stddef.h>

#define CHILDREN 10
#define MEM_SIZE (1ULL * 1024 * 1024 * 1024)  /* 4 GiB */

int main(int argc, char **argv)
{
    char *p = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    for (size_t i = 0; i < MEM_SIZE; i += 4096)
        p[i] = 1;

    for (int i = 0; i < CHILDREN; i++)
    {
        int pid = vfork();
        if (pid == 0) {
            p[1] = 0;
            execve(argv[0], argv + 1, (char *const []){NULL});
        }
    }
    for (int i = 0; i < CHILDREN; i++)
        wait(NULL);
    munmap(p, MEM_SIZE);
}
