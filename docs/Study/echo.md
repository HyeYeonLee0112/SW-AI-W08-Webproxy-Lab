# FD(File Descriptor)와 echo 서버에서의 소켓 이해

질문한 포인트를 기준으로 정리한 메모:
- `fd`는 “정수형 식별자”(id)이다.
- `fd` 자체가 소켓/파일 자체가 아니라, 커널이 관리하는 자원에 접근하기 위한 **키**다.
- 소켓 관련 API에서 쓰이는 경우, 이 `fd`는 커널의 소켓 객체를 참조한다.

## 1) FD는 무엇인가?
- `fd`(File Descriptor)는 운영체제가 열린 I/O 자원에 붙이는 정수형 번호.
- 종류와 무관하게 동일한 개념을 쓴다.
  - 일반 파일: `open("a.txt", O_RDONLY)` → 파일 fd
  - 파이프: `pipe()`
  - 터미널/표준 입출력: `0`, `1`, `2`
  - 소켓: `socket()`, `accept()`, `Open_clientfd()` 반환 값
- 즉, `fd`는 **“무엇인가를 직접 담는 값”이 아니라 그 대상에 대한 참조값**이다.

## 2) “정말로 fd가 소켓을 가리키는 정수인가?”
- 네. 소켓 객체를 다루는 경우에는 맞다.
- 다만 소켓만이 아니므로, 용어를 정확히 하자면  
  - “소켓을 식별하는 정수”가 아니라  
  - “소켓 객체를 커널에서 찾아낼 수 있게 해주는 정수형 식별자”다.

## 3) 소켓일 때 fd의 동작
`open_clientfd()`에서:
- `clientfd = Open_clientfd(host, port)`를 통해 정수 fd 하나를 받는다.
- 이후 `Rio_readlineb(&rio, clientfd, ...)`, `Rio_writen(clientfd, ...)` 처럼  
  **같은 fd를 넘겨서 소켓의 상태/버퍼/연결을 커널이 찾아서 처리**한다.
- 정리할 때는 `Close(clientfd)`로 fd를 반환해 커널 자원을 해제한다.

즉, 실제로 커널이 들고 있는 것은 “소켓 객체(연결 상태, 버퍼, 상대 주소 등)”이고,  
사용자가 들고 있는 건 그 객체로 가는 **문 손잡이(hadle)**인 정수형 fd다.

## 4) 질문 흐름 정리(핵심 정합성)
- `fd`가 소켓인지 궁금했지?
  - 예, 소켓 함수 맥락에서 만든 fd는 소켓을 가리킵니다.
- 그러면 fd는 항상 소켓이냐고?
  - 아니요, 파일/파이프/터미널도 fd가 될 수 있습니다.
- 결국 소켓인 경우만 본다면?
  - 네, 소켓 객체에 접근할 수 있게 해 주는 정수형 id(핸들)라고 보면 됩니다.

## 5) 코드에서 자주 나오는 이름들
- `clientfd`: 클라이언트가 서버에 연결할 때 사용한 소켓 fd
- `listenfd`: 서버에서 `bind`/`listen`한 수신용 소켓 fd
- `connfd`: `accept()`로 수락한 개별 클라이언트 연결 소켓 fd
- 이름은 다르지만 본질은 동일한 `int fd` 타입의 파일 디스크립터.

## 6) 한 줄 요약
`fd`는 커널 자원(파일·소켓·파이프 등)을 직접 가리키는 값이 아니라,  
커널 내부 객체를 찾아오게 해 주는 정수형 핸들이고, 소켓 맥락에서는 소켓 객체를 지칭한다.

---

## 7) `open_clientfd` 코드 분석 (CS:APP 스타일)

### 7.1 코드(원본)

```c
/* $begin open_clientfd */
int open_clientfd(char *hostname, char *port) {
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break; /* Success */
        if (close(clientfd) < 0) { /* Connect failed, try another */  //line:netp:openclientfd:closefd
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}
/* $end open_clientfd */
```

### 7.2 한눈에 보는 흐름(핵심)
1. `hostname`, `port`를 받아서 `getaddrinfo`로 연결 가능한 주소 후보 목록을 받는다.  
2. 후보 목록을 순회하면서:
   - `socket()`으로 소켓 fd를 만든다.
   - `connect()`로 연결 시도한다.
   - 실패하면 그 fd를 `close()`하고 다음 후보로 간다.
3. 하나라도 성공하면 그 소켓 fd를 반환한다.
4. 모두 실패하면 `-1`을 반환한다.

### 7.3 줄별 포인트 정리
- `struct addrinfo hints` 초기화 (`memset`)  
  - `hints`를 0으로 비워야 `getaddrinfo`가 의미 없는 값으로 오작동하지 않는다.
- `hints.ai_socktype = SOCK_STREAM`  
  - TCP 연결용 소켓을 요구한다(스트림).
- `AI_NUMERICSERV`  
  - `port`를 이름이 아닌 숫자 포트 문자열로 처리.
- `AI_ADDRCONFIG`  
  - 로컬 네트워크 스택 환경에 맞는 주소만 받아오도록 도움.
- `getaddrinfo(hostname, port, &hints, &listp)`  
  - 성공하면 `listp`가 연결 리스트로 주소 후보를 반환.
- `for` 루프  
  - IPv4/IPv6 같은 여러 후보 주소를 모두 시도하는 **복원력(robustness)** 패턴.
- `socket(...)` 실패 시 `continue`  
  - 현재 후보를 건너뛰고 다음 후보로.
- `connect(...)` 실패 시 `close(clientfd)`  
  - 실패한 소켓 자원 회수(중요). 안 하면 fd 누수 발생.
- `close()` 실패 처리  
  - 종료 시도조차 실패하면 `-1` 반환(예외 방어).
- `freeaddrinfo(listp)`  
 - `getaddrinfo`가 할당한 메모리를 반드시 해제.
- 마지막 반환 분기  
  - `!p`이면 후보 전부 실패.
  - 성공한 후보가 있으면 그 `clientfd` 반환.

### 7.4 `fd` 관점에서 본 핵심 포인트
- 이 함수는 성공 시 **연결된 소켓에 대한 fd**를 돌려준다.
- 그래서 반환된 `clientfd`는 이후 `Rio_readlineb(clientfd, ...)`, `Rio_writen(clientfd, ...)`처럼  
  소켓 I/O를 위한 정수형 핸들로 사용된다.
- 즉, “이 값이 소켓 자체”가 아니라, “커널의 소켓 객체를 가리키는 핸들”.

### 7.5 에러 코드 해석
- `-2`: `getaddrinfo` 단계 실패  
  - 주소 해석/네트워크 환경에서 문제.
- `-1`: 소켓 생성/연결 단계에서 실패(또는 실패 소켓 닫기 실패 포함).
- 반환 값 체크가 중요  
  - 보통 `if (clientfd < 0)`로 호출부에서 즉시 처리한다.

### 7.6 이 코드가 좋은 이유
- 하나의 IP/주소 고정 가정이 아니라 **가능한 후보를 순회**해 성공 확률을 높인다.
- 실패 경로마다 자원 정리(`close`, `freeaddrinfo`)를 수행해 누수 방지.
- 최소한의 분기만으로 성공/실패를 명확히 나누어 `클라이언트 소켓 생성 → 연결 → 사용` 파이프라인과 딱 맞는다.
