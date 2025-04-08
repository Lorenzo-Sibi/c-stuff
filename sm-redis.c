#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#define PING_MSG "PING\n"

static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd)
{
    char rbuf[64] = {};
    char confbuf[4] = {0};
    size_t n, msg_config;
    do
    {
        msg_config = recv(connfd, confbuf, sizeof(4), MSG_WAITALL);
        if (msg_config < 0)
        {
            msg("recv() error - msg_config");
            return;
        }

        int n_ping = atoi(&confbuf[1]);

        n = recv(connfd, rbuf, n_ping * (sizeof(PING_MSG) - 1), MSG_WAITALL);
        if (n < 0)
        {
            msg("recv() error");
            return;
        }
        fprintf(stderr, "client says: %s\n", rbuf);

        char wbuf[] = "+PONG\r\n";
        for (int i = 0; i < n_ping; i++)
        {
            write(connfd, wbuf, strlen(wbuf));
        }
    } while (n != 0);
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    // this is needed for most server applications
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("listen()");
    }
    while (1)
    {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0)
        {
            continue; // error
            printf("Failed connection. Skipped.\n");
        }

        do_something(connfd);
        close(connfd);
    }

CLOSE:
    close(fd);
    return 0;
}