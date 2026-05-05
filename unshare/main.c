#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/capability.h>

int main()
{
    // unshare(CLONE_NEWUSER);
    int uid = getuid();
    printf("uid = %d\n", uid);

    cap_t caps = cap_get_proc();
    cap_flag_value_t v;
    cap_get_flag(caps, CAP_SYS_ADMIN, CAP_EFFECTIVE, &v);
    printf("CAP_SYS_ADMIN: %s\n", v == CAP_SET ? "yes" : "no");
    cap_free(caps);

    return 0;
}
