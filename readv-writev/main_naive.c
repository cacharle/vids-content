#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
#ifndef PORT
#define PORT 8080
#endif
        .sin_port = htons(PORT),
        .sin_addr = { .s_addr = INADDR_ANY }
    };
    bind(sock, (struct sockaddr*)&addr, sizeof addr);
    listen(sock, 128);

    for (;;)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof client_addr;
        int client_fd = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);

        char buf[4096];
        read(client_fd, buf, sizeof buf);

        char *body =
            "<html><head></head><body><p>hello there</p></body></html>";
        char content_length[32];
        sprintf(content_length, "Content-Length: %zu\r\n", strlen(body));

        char *status      = "HTTP/1.1 200 OK\r\n";
        char *type        = "Content-Type: text/html\r\n";
        char *connection  = "Connection: close\r\n";
        char *server      = "Server: write-demo\r\n";
        char *cache       = "Cache-Control: no-cache\r\n";
        char *vary        = "Vary: Accept-Encoding\r\n";
        char *header_end  = "\r\n";

        write(client_fd, status, strlen(status));
        write(client_fd, type, strlen(type));
        write(client_fd, connection, strlen(connection));
        write(client_fd, content_length, strlen(content_length));
        write(client_fd, server, strlen(server));
        write(client_fd, cache, strlen(cache));
        write(client_fd, vary, strlen(vary));
        write(client_fd, header_end, strlen(header_end));
        write(client_fd, body, strlen(body));
        close(client_fd);
    }

    close(sock);
    return 0;
}
