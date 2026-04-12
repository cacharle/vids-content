#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, "./foo", IN_ALL_EVENTS);

    while (1)
    {
        char buf[4096];
        int len = read(fd, buf, sizeof buf);

        struct inotify_event *event;
        int i = 0;
        for (char *ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len, i++) {
            event = (struct inotify_event *)ptr;
            if (event->mask & IN_ACCESS)
                printf("File accessed");
            if (event->mask & IN_MODIFY)
                printf("File modified");
            if (event->mask & IN_ATTRIB)
                printf("File attrib changed");
            if (event->mask & IN_CLOSE_WRITE)
                printf("File closed (write)");
            if (event->mask & IN_CLOSE_NOWRITE)
                printf("File closed (no write)");
            if (event->mask & IN_OPEN)
                printf("File opened");
            if (event->mask & IN_MOVED_FROM)
                printf("File moved from");
            if (event->mask & IN_MOVED_TO)
                printf("File moved to");
            if (event->mask & IN_CREATE)
                printf("File created");
            if (event->mask & IN_DELETE)
                printf("File deleted");
            if (event->mask & IN_DELETE_SELF) {
                printf("Watch target deleted");
                inotify_rm_watch(fd, wd);
                wd = inotify_add_watch(fd, "./foo", IN_ALL_EVENTS);
            }
            if (event->mask & IN_MOVE_SELF)
                printf("Watch target moved");
            if (event->len > 0)
                printf(" with name %s", event->name);
            putchar('\n');
        }
    }

    return 0;
}
