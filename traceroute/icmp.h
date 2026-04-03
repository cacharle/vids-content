#ifndef VIDS_ICMP_H
#define VIDS_ICMP_H

int icmp_create_socket(const char *dest_domain_name);
void icmp_send(int sock, int sequence, int ttl);
int icmp_recv(int sock, struct addrinfo *src_addrinfo);

#endif // VIDS_ICMP_H
