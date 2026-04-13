#include <sys/timerfd.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
    int fd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec spec = {
        .it_interval = { .tv_sec = 1, .tv_nsec = 0 },
        .it_value    = { .tv_sec = 1, .tv_nsec = 0 },
    };
    timerfd_settime(fd, 0, &spec, NULL);

    for (;;)
    {
        uint64_t expir;
        read(fd, &expir, sizeof expir);
        printf("Timer\n");
    }

    close(fd);
    return 0;
}
