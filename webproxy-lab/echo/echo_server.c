#include <stdio.h>

#include "../csapp.h"

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    /* connfd에 대해 RIO 버퍼를 초기화한다.
     * 이후부터는 한 줄씩 읽어서 그대로 다시 써 주는 에코 루프가 된다. */
    rio_readinitb(&rio, connfd);

    /* 클라이언트가 보낸 한 줄을 읽고, 같은 내용을 다시 돌려준다. */
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        /* 읽은 바이트 수 n만큼을 그대로 다시 전송한다. */
        Rio_writen(connfd, buf, n);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    /* 서버가 기다릴 포트를 열고, 클라이언트 연결을 받아들일 준비를 한다. */
    int listenfd = Open_listenfd(argv[1]);    

    /* 무한 루프로 연결 요청을 계속 받는다.
     * accept()는 새 클라이언트마다 별도의 connfd를 돌려준다. */
    int connfd;
    socklen_t clientlen;
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        /* 연결한 클라이언트의 주소와 포트를 문자열로 얻어 출력한다. */
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        /* 실제 에코 처리는 connfd를 통해 이루어진다. */
        echo(connfd);

        /* 한 클라이언트 처리가 끝나면 해당 연결 소켓을 닫는다. */
        Close(connfd);
    }

    exit(0);
}
