#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <poll.h>
#include <sys/signalfd.h>
#include <signal.h>

int main()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    sigprocmask(SIG_BLOCK, &mask, NULL);
    int sigint_fd = signalfd(-1, &mask, 0);
    assert(sigint_fd != -1);
    while (1)
    {
        struct pollfd fds[2];
        fds[0].events = POLLIN;
        fds[0].fd = STDIN_FILENO;
        fds[1].events = POLLIN;
        fds[1].fd = sigint_fd;
        int ret = poll(fds, 2, 1000);
        assert(ret != -1);
        if (ret == 0)
            continue;
        if (fds[0].revents == POLLIN)
        {
            char buf[32 + 1];
            int read_size = read(STDIN_FILENO, buf, 32);
            buf[read_size] = '\0';
            printf("Stdin received: %s", buf);
        }
        else if (fds[1].revents == POLLIN)
        {
            struct signalfd_siginfo info;
            read(sigint_fd, &info, sizeof(info));
            if (info.ssi_signo == SIGINT)
                printf("Received SIGINT\n");
            else if (info.ssi_signo == SIGQUIT) {
                printf("Received SIGQUIT\n");
                break;
            }
        }
    }
}
