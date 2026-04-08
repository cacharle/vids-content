#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "icmp.h"

int main()
{
    int sock = icmp_create_socket("google.com");

    for (int ttl = 1; ttl < 32; ttl++)
    {
        icmp_send(sock, ttl, ttl);
        struct sockaddr_in src_addr = {0};
        int ret = icmp_recv(sock, &src_addr);
        if (ret == -1) {
            printf("%d: timeout\n", ttl);
            continue;
        }

        char src_hostname[1048] = "";
        getnameinfo((struct sockaddr *)&src_addr, sizeof(src_addr),
                    src_hostname, sizeof(src_hostname), NULL, 0, 0);
        printf("%d: received from %s (%s)\n", ttl, src_hostname, inet_ntoa(src_addr.sin_addr));

        if (ret == -2)
            break;
    }

    close(sock);
    return 0;
}

