#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/uio.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr = { .s_addr = INADDR_ANY }
    };
    bind(sock, (struct sockaddr*)&addr, sizeof addr);
    listen(sock, 1);

    for (;;)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof client_addr;
        int client_fd = accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);

        char buf[4096];
        read(client_fd, buf, sizeof buf);

        char *header_start =
            "HTTP/1.1 200 xx\r\n"
            "Content-Type: html\r\n"
            "Content-Length: ";
        char *content =
            "<html><head></head><body><p>hello there</p></body></html>";
        char length_buf[32];
        sprintf(length_buf, "%zu", strlen(content));
        char *header_content_separator = "\r\n\r\n";

        struct iovec iov[4] = {
            { .iov_base = header_start, .iov_len = strlen(header_start) },
            { .iov_base = length_buf, .iov_len = strlen(length_buf) },
            { .iov_base = header_content_separator, .iov_len = strlen(header_content_separator) },
            { .iov_base = content, .iov_len = strlen(content) },
        };
        writev(client_fd, iov, 4);
        close(client_fd);
    }

    close(sock);
    return 0;
}
