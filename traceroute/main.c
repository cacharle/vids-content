#include <unistd.h>
#include <stdio.h>
#include <netdb.h>

#include "icmp.h"

int main()
{
    int sock = icmp_create_socket("google.com");

    for (int ttl = 1; ttl < 32; ttl++)
    {
        icmp_send(sock, ttl, ttl);
        struct addrinfo src_addrinfo = {0};
        int ret = icmp_recv(sock, &src_addrinfo);
        if (ret == -2) {
            printf("end reached\n");
            break;
        }
        if (ret == -1) {
            printf("%d: timeout\n", ttl);
            continue;
        }
        printf("%d: received\n", ttl);
    }

    close(sock);
    return 0;
}

