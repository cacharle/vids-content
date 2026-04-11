#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, "./foo", IN_ALL_EVENTS);

    while (1)
    {
        struct inotify_event event;
        read(fd, &event, sizeof event);
        if (event.mask & IN_CREATE)
            printf("File created\n");
        if (event.mask & IN_ACCESS)
            printf("File accessed\n");
        if (event.mask & IN_MODIFY)
            printf("File modified\n");
        if (event.mask & IN_DELETE_SELF) {
            printf("File deleted\n");
            inotify_rm_watch(fd, wd);
            wd = inotify_add_watch(fd, "./foo", IN_ALL_EVENTS);
        }
    }

    return 0;
}
