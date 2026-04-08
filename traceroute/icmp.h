#ifndef VIDS_ICMP_H
#define VIDS_ICMP_H

struct in_addr
addrinfo_to_ip(const struct addrinfo *addrinfo);
int icmp_create_socket(const char *dest_domain_name);
void icmp_send(int sock, int sequence, int ttl);
int icmp_recv(int sock, struct sockaddr_in *src_addr);

#endif // VIDS_ICMP_H
