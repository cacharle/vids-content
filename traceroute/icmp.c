#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEST_DOMAIN_NAME "google.com"

int icmp_create_socket()
{
}

void icmp_send(int id, int ttl)
{
}

uint16_t
checksum(uint16_t *data, size_t size)
{
    uint32_t sum = 0;
    size_t   data_len = size / 2;
    for (size_t i = 0; i < data_len; i++)
        sum += data[i];
    return ~((sum << 16 >> 16) + (sum >> 16));
}

int main()
{
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    struct addrinfo   *destination_addrinfo;
    int err = getaddrinfo(
        DEST_DOMAIN_NAME,
        NULL,
        &hints,
        &destination_addrinfo);
    assert(err == 0);
    struct sockaddr_in *addr =
        (struct sockaddr_in *)destination_addrinfo->ai_addr;
    printf("Destination IP: %s\n",
            inet_ntoa((struct in_addr)addr->sin_addr));

    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

    struct icmphdr packet = {
        .type = ICMP_ECHO,
        .code = 0,
        .checksum = 0,
        .un = { .echo = { .id = 0, .sequence = 0 } }
    };
    packet.checksum = checksum((uint16_t *)&packet, sizeof(packet));

    err = sendto(sock,
               &packet,
               sizeof(packet),
               0,
               destination_addrinfo->ai_addr,
               destination_addrinfo->ai_addrlen);
    assert(err != -1);

    err = recvfrom(sock,
                   &packet,
                   sizeof(packet),
                   0,
                   destination_addrinfo->ai_addr,
                   &destination_addrinfo->ai_addrlen);
    assert(err != -1);
    printf("Received pong\n");

    close(sock);
    return 0;
}
