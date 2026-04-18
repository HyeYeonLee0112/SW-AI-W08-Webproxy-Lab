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
    
    /* rio를 clientfd에서 읽을 수 있도록 초기화하는 함수
    rio 필드(rio_fd)dp 어떤 fd에서 읽을지 저장하고
    내부 버퍼 상태를 0으로 초기화하고
    버퍼 시작 위치를 맞춰서
    */
    rio_readinitb(&rio, clientfd);
    
    /* 루프 종료조건
    fgets(buf, MAXLINE, stdin)==NULL
    의미: eof표준 입력을 만났다.
    사용자가 ctrl+d를 누릈거나 파일로 텍스트 줄을 모두 소진
    */
    while (fgets(buf, MAXLINE, stdin) != NULL) {

       
        //송신(클라 소켓에 buf를 보냄)
        /* Rio_writen 함수: 클라이언트 소켓에 버퍼의 데이터를 안정적으로 전송
        소켓(clientfd)에 
        보낼 문자열이 들어있는 버퍼(buf)를 
        strlen(buf) 길이만큼
        소켓에 안정적으로 전송한다

        "안정적": Rio_writen은 한 번에 다 못 보내면 여러 번 나눠서라도 끝까지 보내주는 함수
        일반 write()보다 “끝까지 보낸다”는 점에서 더 안전
        */
        Rio_writen(clientfd, buf, strlen(buf));

        /* Rio_readlineb: (에코) 수신
        *큰 그림: 서버 -> 네트워크 -> clientfd 소켓 -> Rio_readlineb -> buf
        서버로부터 클라 소켓소켓으로 들어온 데이터를 buf에 복사
        */
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
            Fputs(buf, stdout); //문자열(buf))을 표준 출력(stdout)에 출력
        } 
        //함수 반환값이 음수면 예외처리
        else 
            break;
        
    }

    //식별자를 닫는다 => 클라의 소켓을 닫음
    Close(clientfd);
    return 0;
}
