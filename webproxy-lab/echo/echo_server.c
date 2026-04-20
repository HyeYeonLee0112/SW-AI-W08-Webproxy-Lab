#include <stdio.h>

#include "../csapp.h"
#include "echo_common.h"

static void echo_client(int connfd)
{
    rio_t rio;
    char buf[MAXLINE];
    ssize_t n;

    rio_readinitb(&rio, connfd);

    while ((n = echo_handle_one_line(connfd, &rio, buf, MAXLINE)) > 0) {
        (void) n; // echo handled in common module
    }
}

int main(int argc, char **argv)
{
    echo_common_init();

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    int listenfd = Open_listenfd(argv[1]);
    if (listenfd < 0) {
        unix_error("Open_listenfd");
    }

    while (1) {
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        echo_client(connfd);
        Close(connfd);
    }

    return 0;
}
