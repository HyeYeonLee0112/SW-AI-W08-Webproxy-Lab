# echo 서버 학습 노트

## 목차
- [1. fd](#1-fd)
- [2. echo_client 코드 분석](#2-echo_client-코드-분석)
- [3. echo_server 코드 분석](#3-echo_server-코드-분석)
- [4. open_clientfd 코드 흐름](#4-open_clientfd-코드-흐름)

---

# 1. fd

## 1.1 fd란 무엇인가

- `fd`는 `file descriptor`의 줄임말이다.
- 프로세스가 여는 I/O 자원에 붙는 정수 번호다.
- 코드에서는 그냥 `int`처럼 보이지만, 실제로는 커널 안의 자원을 가리키는 손잡이 역할을 한다.

## 1.2 fd가 붙는 대상

- 파일: `open()`
- 파이프: `pipe()`
- 표준 입출력: `0(stdin)`, `1(stdout)`, `2(stderr)`
- 소켓: `socket()`, `accept()`, `open_clientfd()`

## 1.3 왜 fd가 필요한가

- 프로세스는 커널의 자원을 직접 만지지 못한다.
- 대신 `fd`를 통해 커널이 관리하는 대상을 간접적으로 사용한다.
- 같은 `fd` 번호가 다른 프로세스에서는 전혀 다른 자원을 가리킬 수 있다.

## 1.4 listen socket과 accepted socket

```mermaid
flowchart TB
  L["listen socket 1개"]
  A1["accepted socket 1"]
  A2["accepted socket 2"]
  A3["accepted socket 3"]

  C1["클라이언트 1"]
  C2["클라이언트 2"]
  C3["클라이언트 3"]

  L --> A1
  L --> A2
  L --> A3
  C1 --> L
  C2 --> L
  C3 --> L
```

- 서버는 `listen socket` 1개로 연결 요청을 기다린다.
- 클라이언트가 연결하면 `accept()`가 새 소켓을 하나 만든다.
- 이 새 소켓이 `accepted socket`이다.
- 따라서 서버는 하나의 `listen socket`과 여러 개의 `accepted socket`을 동시에 가진다.

## 1.5 fd와 커널 자원

```mermaid
flowchart LR
  P["프로세스 공간"] --> F["fd"]
  F --> T["fd table"]
  T --> S["커널 소켓 객체"]
  S --> A["주소 정보"]
  S --> C["연결 정보"]
  S --> B["송수신 버퍼"]
  S --> R["프로토콜 상태"]
```

- 프로세스는 `fd`를 가진다.
- 커널은 `fd table`을 통해 실제 소켓 객체를 찾는다.
- 소켓 객체 안에는 주소, 연결 상태, 버퍼 정보가 들어 있다.

## 1.6 대기큐(backlog queue)는 에코 서버에 있다

- 대기큐는 에코 서버에서 `listen()`이 담당하는 커널 내부의 연결 대기 공간이다.
- 코드에서 직접 `queue` 객체를 만들지 않아도, `Open_listenfd()` 내부의 `listen(listenfd, backlog)`가 이 동작을 켠다.
- 클라이언트가 `connect()`를 시도하면, 아직 `accept()`되지 않은 연결이 이 대기 공간에 쌓인다.
- 서버가 `Accept(listenfd, ...)`를 호출하면 그중 하나가 꺼내져 `connfd`가 된다.

```mermaid
flowchart TD
  C1["클라이언트 1"] -->|connect| L["listenfd"]
  C2["클라이언트 2"] -->|connect| L
  C3["클라이언트 3"] -->|connect| L
  L -->|backlog| Q["커널 연결 대기 큐"]
  Q -->|accept| S["서버의 connfd"]
```

- 이 큐는 "서버 코드의 변수"가 아니라 "커널이 관리하는 상태"다.
- 그래서 우리가 직접 큐를 push/pop 하는 코드는 없고, `listen()`과 `accept()` 호출로 간접 제어한다.

---

# 흐름

```mermaid
sequenceDiagram
  participant U as 사용자
  participant C as echo_client
  participant S as echo_server

  U->>C: 문자열 입력
  C->>C: fgets()로 buf 저장
  C->>S: Rio_writen()으로 전송
  S->>S: 받은 데이터 확인
  S->>C: 같은 데이터 응답
  C->>C: Rio_readlineb()로 응답 읽기
  C->>U: Fputs()로 화면 출력
```

# 2. echo_client 코드 분석

## 2.1 이 프로그램이 하는 일

- 사용자가 입력한 문자열을 서버로 보낸다.
- 서버가 같은 문자열을 다시 돌려주면 화면에 출력한다.
- 그래서 이름이 `echo_client`이다.

## 2.2 `argc`와 `argv`

```c
if (argc != 3) {
    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    return 1;
}
```

- `argc`는 명령행 인자 개수다.
- `argv[0]`는 실행 파일 이름이다.
- `argv[1]`은 `host`다.
- `argv[2]`는 `port`다.

예시:

```bash
./echo_client example.com 8080
```

- `argv[0] = "./echo_client"`
- `argv[1] = "example.com"`
- `argv[2] = "8080"`

## 2.3 `host`와 `port`

- `host`는 서버 주소 문자열이다.
- `port`는 서버가 기다리는 포트 번호 문자열이다.
- `open_clientfd(host, port)`는 이 둘을 이용해서 실제 연결을 만든다.

## 2.4 `open_clientfd` 호출

```c
clientfd = Open_clientfd(host, port);
```

- 성공하면 `clientfd`는 연결된 소켓의 `fd`가 된다.
- 실패하면 음수 값을 돌려준다.

## 2.5 데이터 루프

```c
while (fgets(buf, MAXLINE, stdin) != NULL) {
    Rio_writen(clientfd, buf, strlen(buf));
    if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        Fputs(buf, stdout);
    } else {
        break;
    }
}
```

- `fgets()`가 사용자 입력을 읽는다.
- `Rio_writen()`이 서버로 보낸다.
- `Rio_readlineb()`가 서버 응답을 읽는다.
- `Fputs()`가 화면에 출력한다.

```mermaid
flowchart TD
  A["사용자 입력"] --> B["fgets()"]
  B --> C["buf에 문자열 저장"]
  C --> D["Rio_writen(clientfd, buf, strlen(buf))"]
  D --> E["서버로 전송"]
  E --> F["서버가 같은 데이터 수신"]
  F --> G["Rio_readlineb(&rio, buf, MAXLINE)"]
  G --> H["응답 데이터를 buf에 저장"]
  H --> I["Fputs(buf, stdout)"]
  I --> J["화면 출력"]
```

---

# 3. echo_server 코드 분석

## 3.1 이 프로그램이 하는 일

- 클라이언트가 보낸 데이터를 읽는다.
- 읽은 데이터를 그대로 다시 보낸다.
- 그래서 이름이 `echo server`이다.

## 3.2 실제 코드

```c
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
```

## 3.3 코드리뷰 형태로 보는 포인트

- `Open_listenfd(argv[1])`
  - 포트만 받아 서버를 열고 있다.
  - 즉, 에코 서버는 "특정 호스트에 붙는 서버"가 아니라 "해당 포트를 듣는 서버"다.
- `while (1)`
  - 순차 처리의 핵심이다.
  - 한 번에 하나의 `connfd`만 받아서 처리하고 다시 `accept()`로 돌아간다.
- `Accept(listenfd, ...)`
  - 대기큐에서 연결 하나를 꺼내는 단계다.
  - 이 지점에서 비로소 클라이언트 전용 `connfd`가 생긴다.
- `Getnameinfo(...)`
  - 연결 상대를 사람이 읽을 수 있는 형태로 바꾸는 로그 출력용 코드다.
  - 핵심 로직은 아니지만 연결 확인에 유용하다.
- `echo(connfd)`
  - 실제 데이터 왕복은 여기서 일어난다.
  - `connfd`가 없으면 클라이언트와 주고받을 수 없으므로, 이 fd가 연결의 실체다.
- `Close(connfd)`
  - 한 클라이언트 처리가 끝나면 정리한다.
  - 이 서버는 연결을 재사용하지 않고, 한 연결을 다 처리한 뒤 닫는다.

## 3.4 echo 함수 내부 리뷰

- `rio_readinitb(&rio, connfd)`
  - `connfd`에 대해 buffered I/O를 준비한다.
  - 이후 읽기 함수는 커널 fd를 직접 만지는 대신 RIO 버퍼를 거친다.
- `while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)`
  - 한 줄 단위로 읽으며, 더 이상 읽을 것이 없으면 종료한다.
  - 에코 서버는 입력 경계보다 "받은 그대로 돌려주는 것"이 중요하다.
- `Rio_writen(connfd, buf, n)`
  - 읽은 바이트 수만큼 정확히 다시 쓴다.
  - 에코 서버의 핵심은 변형 없이 반환하는 것이다.

### `echo server` 흐름

```mermaid
flowchart TD
  A["main() 시작"] --> B["Open_listenfd"]
  B --> C["listenfd 준비"]
  C --> D["Accept(listenfd, ...)"]
  D --> E["connfd 생성"]
  E --> F["echo(connfd)"]
  F --> G["Rio_readinitb"]
  G --> H["Rio_readlineb"]
  H --> I["Rio_writen"]
  I --> H
  H --> J["클라이언트가 닫히면 종료"]
  J --> K["Close(connfd)"]
```

- 서버는 `listenfd`로 연결 요청을 기다린다.
- `accept()`로 클라이언트별 `connfd`가 생긴다.
- `echo()`는 `connfd`를 이용해서 읽고 다시 쓰는 루프를 돈다.
- `Rio_readlineb()`로 한 줄 읽고 `Rio_writen()`으로 같은 내용을 다시 보낸다.
- 에코 서버는 "받은 걸 그대로 돌려주는 서버"를 이해하기 좋은 예제다.

# 4. open_clientfd 코드 흐름

## 4.1 전체 흐름

```mermaid
flowchart TD
  A["getaddrinfo()"] --> B["주소 목록 받기"]
  B --> C["socket()"]
  C --> D["connect()"]
  D --> E["성공하면 clientfd 반환"]
  D --> F["실패하면 다음 주소 시도"]
  F --> B
```

## 4.2 왜 주소 목록이 필요한가

- `host`는 하나의 IP가 아닐 수 있다.
- IPv4와 IPv6가 함께 있을 수 있다.
- DNS 결과가 여러 개면 그중 하나를 차례대로 시도해야 한다.

## 4.3 실패 처리

- `socket()`이 실패하면 다음 시도로 넘어간다.
- `connect()`가 실패하면 `close()`하고 다음 주소를 시도한다.
- 모든 시도가 실패하면 에러를 돌려준다.

## 4.4 핵심 정리

- `fd`는 프로세스가 커널 자원을 쓰기 위한 번호다.
- `listen socket`은 연결 요청을 받는 창구다.
- `accepted socket`은 실제로 한 클라이언트와 통신하는 소켓이다.
- `echo_client`는 입력한 문자열을 서버에 보내고 다시 받는 구조를 공부하기 좋은 예제다.
