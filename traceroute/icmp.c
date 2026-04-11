#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEST_DOMAIN_NAME "google.com"

static uint16_t checksum(uint16_t *data, size_t size)
{
    uint32_t sum = 0;
    size_t   data_len = size / 2;
    for (size_t i = 0; i < data_len; i++)
        sum += data[i];
    return ~((sum << 16 >> 16) + (sum >> 16));
}

static struct addrinfo *destination_addrinfo;
static struct addrinfo *source_addrinfo;
static struct in_addr source_ip_addr;
static struct in_addr destination_ip_addr;

void
xgetaddrinfo(const char *node, struct addrinfo **addrinfo)
{
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    int err = getaddrinfo(node, NULL, &hints, addrinfo);
    assert(err == 0);
}

struct in_addr
addrinfo_to_ip(const struct addrinfo *addrinfo)
{
    // from: https://stackoverflow.com/questions/20115295
    struct sockaddr_in *addr = (struct sockaddr_in *)addrinfo->ai_addr;
    return (struct in_addr)addr->sin_addr;
}

int icmp_create_socket(const char *dest_domain_name)
{

    char localhost[1024];
    gethostname(localhost, 1024);
    xgetaddrinfo(localhost, &source_addrinfo);
    source_ip_addr = addrinfo_to_ip(source_addrinfo);

    xgetaddrinfo(dest_domain_name, &destination_addrinfo);
    destination_ip_addr = addrinfo_to_ip(destination_addrinfo);
    printf("Destination (%s) IP: %s\n",
            dest_domain_name, inet_ntoa(destination_ip_addr));

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    assert(sock != -1);
    int on = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return sock;
}

struct ping_packet
{
    struct iphdr   ip;
    struct icmphdr icmp;
};

void icmp_send(int sock, int sequence, int ttl)
{
    struct ping_packet packet = {
        .ip = {
            .ihl = 5,
            .version = 4,
            .tos = 0,
            .tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr)),
            .id = 0,
            .frag_off = 0,
            .ttl = ttl,
            .protocol = IPPROTO_ICMP,
            .check = 0,
            .saddr = source_ip_addr.s_addr,
            .daddr = destination_ip_addr.s_addr,
            .check = 0,
        },
        .icmp = {
            .type = ICMP_ECHO,
            .code = 0,
            .checksum = 0,
            .un = { .echo = { .id = 42, .sequence = sequence } }
        }
    };
    packet.ip.check = checksum((uint16_t *)&packet.ip, sizeof(packet.ip));
    packet.icmp.checksum = checksum((uint16_t *)&packet.icmp, sizeof(packet.icmp));

    setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    int err = sendto(sock,
               &packet,
               sizeof(packet),
               0,
               destination_addrinfo->ai_addr,
               destination_addrinfo->ai_addrlen);
    assert(err != -1);
}

int icmp_recv(int sock, struct sockaddr_in *src_addr)
{

    struct ping_packet packet;
    socklen_t addr_len = sizeof(*src_addr);
    int err = recvfrom(
        sock, &packet, sizeof(packet), 0,
        (struct sockaddr *)src_addr, &addr_len);
    if (err == -1 || packet.icmp.type == ICMP_DEST_UNREACH)
        return -1;
    if (packet.icmp.type == ICMP_ECHOREPLY)
        return -2;
    if (packet.icmp.type == ICMP_TIME_EXCEEDED)
        return 0;
    // printf("packet.icmp.type = %d\n", packet.icmp.type);
    assert(0);
}
