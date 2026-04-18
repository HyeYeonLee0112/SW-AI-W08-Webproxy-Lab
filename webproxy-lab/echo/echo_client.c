#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../csapp.h"

//인자 3개: argv[0](실행 파일 이름) + argv[1](host) + argv[2](port)
int main(int argc, char **argv)
{

    //1. 인자 개수(argc) 3개가 아니면 예외처리 
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    //2. 변수 set
    
    // hostname 문자열의 시작 주소 (예: "localhost", "127.0.0.1")
    char *host = argv[1]; 
    //port 문자열의 시작 주소 (예: "8080"); 정수 아님(문자열)
    char *port = argv[2]; 

    //열려있는?또는 소켓을 열어서 클라의 소켓 식별자를 반환
    int clientfd = Open_clientfd(host, port);

    //clientfd가 음수면 예외처리
    if (clientfd < 0) {
        errno = -clientfd;
        unix_error("Open_clientfd");
    }

    /*
    buf: “데이터를 담는 임시 상자”
    rio는 “그 상자를 관리하는 읽기 상태 관리자”
    */
    //(입력받은 문자열 & 서버 응답)을 임시 저장하는 메모리 공간
    char buf[MAXLINE]; 
    rio_t rio; //buf 관리하는 읽기 상태 관리자
    
    /*
    clientfd에서 읽을 준비를 하고
    rio 안에 현재 읽기 상태를 저장한다는 뜻이에요
    */
    rio_readinitb(&rio, clientfd);
    
    /* 루프 종료조건
    fgets(buf, MAXLINE, stdin)==NULL
    의미: eof표준 입력을 만났다.
    사용자가 ctrl+d를 누릈거나 파일로 텍스트 줄을 모두 소진
    */
    while (fgets(buf, MAXLINE, stdin) != NULL) {

        //통신 버퍼를 전체 돌면서 뭔가를 함
        //표준 입력에서 텍스트 줄을 반복해서 읽는 루프에 진입
        //서버에 텍스트 줄을 전송하고 서버에서 에코 줄을 읽어서 그 결과를 표준 출력으로 인쇄한다.
        Rio_writen(clientfd, buf, strlen(buf));
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
            Fputs(buf, stdout);
        } else {
            break;
        }
    }

    //식별자를 닫는다 => 클라의 소켓을 닫음
    Close(clientfd);
    return 0;
}
