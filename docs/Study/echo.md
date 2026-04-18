# echo 서버 학습 노트

## 목차
- [1. fd](#1-fd)
- [2. echo_client 소스코드 분석](#2-echo_client-%EC%86%8C%EC%8A%A4%EC%BD%94%EB%93%9C-%EB%B6%84%EC%84%9D)

---

# 1. fd
> 대분류 요약: 파일 디스크립터(`fd`)는 정수처럼 보이지만, 커널이 관리하는 I/O 자원 접근 키다.
> 학습 포인트: 정수 값과 실제 소켓 객체의 관계를 분리해서 보기.

<div style="height: 2px; background: linear-gradient(90deg, #666, #ddd); margin: 10px 0 20px 0;"></div>

## 1.1 fd는 무엇인가
- `fd`(File Descriptor)는 운영체제가 열려 있는 I/O 자원(파일, 소켓, 파이프, 터미널 등)에 부여하는 **정수형 식별자**다.
- fd 자체는 자원 자체가 아니라, 커널 내부 객체를 가리키는 **핸들(식별 키)**이다.
- 즉, “`fd` 값”은 번호이고, 실제 동작은 커널이 그 번호를 통해 자원 객체를 찾아 수행한다.

## 1.2 fd는 소켓만이 아니다
- 파일: `open(...)` 결과
- 파이프: `pipe()` 결과
- 표준 입출력: `0(stdin)`, `1(stdout)`, `2(stderr)`
- 소켓: `socket()`, `accept()`, `Open_clientfd()` 결과
- 따라서 `fd`는 범용 I/O 자원 식별자다.

## 1.3 소켓 맥락에서 fd를 이해하기
- 소켓을 위한 `connect`, `read`, `write`, `close`는 모두 fd를 받아 동작한다.
- `open_clientfd`가 성공하면 반환하는 값 `clientfd`는 “연결된 소켓 객체를 가리키는 정수형 핸들”이다.
- 사용 완료 후 `Close(clientfd)`로 해제해야 자원 누수를 막는다.

## 1.4 `fd`가 “가리키는 값”처럼 보이는 이유
- `fd`는 `int`이므로 보통 “정수 번호”로 보이지만, 의미상 커널 객체의 참조 포인트다.
- 같은 `fd` 번호를 통해 이후 I/O 호출들이 동일한 자원을 다룬다.
- `fd`는 정수 연산 대상이 아니라, 커널이 해석해야 하는 **자원 식별 토큰**이다.

## 1.5 `host`, `port`, `fd` 연결 관점으로 한 줄 정리
- `host`/`port`는 문자열 포인터이고, `clientfd`는 정수형 식별자이다.
- 문자열 인자를 해석해 네트워크 주소를 찾고, 그 결과로 연결된 소켓 fd를 얻는 흐름이다.

---

# 2. echo_client 소스코드 분석
> 대분류 요약: `argv` 문자열 포인터(`host`, `port`)가 어떻게 소켓 연결(`fd`)로 이어지는지 순서대로 본다.
> 학습 포인트: `argc/argv` 구조 → `open_clientfd` 연결 생성 → 입출력 루프.

<div style="height: 2px; background: linear-gradient(90deg, #666, #ddd); margin: 10px 0 20px 0;"></div>

## 2.1 핵심 질문

`echo_client`는 `./echo_client <host> <port>`로 실행되며, `argv`에서 문자열 인자를 받아 소켓을 열고 서버와 통신한다.

## 2.2 `argc/argv` 검증 코드

```c
if (argc != 3) {
    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    return 1;
}
```

- `argc`는 “실행 파일 이름 포함 전체 인자 개수”
  - `argv[0]`: 실행 파일명
  - `argv[1]`: host
  - `argv[2]`: port
- 사용자 입력은 `host`, `port` 2개지만, `argc`는 3이어야 통과한다.
- 즉, “사용자 입력 2개” + “프로그램명 1개” = 총 3개.

## 2.3 `host`/`port` 포인터로 받는 이유

```c
const char *host = argv[1]; // 명령행에서 전달된 host 문자열 시작 주소
const char *port = argv[2]; // 명령행에서 전달된 port 문자열 시작 주소(숫자 문자열)
```

- `argv`는 `char **`라서, 각 `argv[i]`는 문자열의 시작 주소다.
- 따라서 `host`, `port`는 문자열 자체를 복사하는 게 아니라 **주소를 받아 참조**한다.
- 즉 `host`는 `"example.com"`의 첫 글자 `'e'` 위치 주소, `port`는 `"8080"`의 첫 글자 `'8'` 위치 주소.

## 2.4 예시: 실행 시 `argv` 구성

실행:
```bash
./echo_client example.com 8080
```

내부 값:
- `argc = 3`
- `argv[0] = "./echo_client"`
- `argv[1] = "example.com"`
- `argv[2] = "8080"`
- `argv[3] = NULL`

## 2.5 문자열 값 접근 방식 (주소와 문자)

- `host`는 문자열 전체가 아니라 시작 주소를 담는다.
- `argv[1][0] == 'e'`, `argv[2][0] == '8'`처럼 문자 단위 인덱싱이 가능하다.
- `host[1]`은 `"example.com"`의 두 번째 문자 `'x'`를 가리킨다.

## 2.6 `argv` 값과 주소를 동시에 보는 예시

실행:
```bash
./echo_client example.com 8080
```

개념 배열(`argv`)의 값:
```c
argv[0] = "./echo_client"
argv[1] = "example.com"
argv[2] = "8080"
argv[3] = NULL
```

메모리 느낌(가상 주소 포함):
```text
argv (char **) = 0x7ffdfc20

0x7ffdfc20 -> 0x7ffdfc40   // argv[0] 주소(포인터 값)
0x7ffdfc28 -> 0x7ffdfc60   // argv[1] 주소
0x7ffdfc30 -> 0x7ffdfc70   // argv[2] 주소
0x7ffdfc38 -> 0x00000000   // argv[3] = NULL

[0x7ffdfc40] = "./echo_client\0"
[0x7ffdfc60] = "example.com\0"
[0x7ffdfc70] = "8080\0"
```

핵심 포인트:
- `argv`는 `char**` → 포인터들의 배열(배열 요소는 문자열 시작 주소)
- `argv[1]`은 `"example.com"` 문자열의 시작 주소
- `host = argv[1];`는 문자열을 복사한 게 아니라 같은 시작 주소를 공유
- `*argv[1]` 또는 `argv[1][0]`은 `'e'`, `argv[2][0]`은 `'8'`을 읽음

```c
char *host = argv[1];
char *port = argv[2];
// host 는 "example.com"의 e 주소, port는 "8080"의 8 주소를 가리킴
```

## 2.6 `open_clientfd` 호출 및 반환

```c
clientfd = Open_clientfd(host, port);
```

- `clientfd`가 0보다 크면 연결된 소켓 fd 획득 성공.
- `<0`이면 실패.

## 2.7 `open_clientfd` 핵심 동작 정리 (CS:APP 패턴)

```c
/* $begin open_clientfd */
int open_clientfd(char *hostname, char *port) {
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;

    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }

    for (p = listp; p; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        if (close(clientfd) < 0) {
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;
    else
        return clientfd;
}
/* $end open_clientfd */
```

### 2.7.1 핵심 흐름
1. `getaddrinfo`로 서버 주소 후보 리스트를 얻는다.
2. 후보를 하나씩 반복한다.
   - `socket()` 생성
   - `connect()` 시도
   - 성공하면 그 `fd`를 바로 반환한다.
   - 실패하면 `close()`하고 다음 후보로 간다.
3. 끝까지 실패하면 `-1`.
4. `getaddrinfo` 자체 실패는 `-2`.
5. 호스트 이름 하나가 DNS에서 여러 IP 주소로 해석될 수 있어서, 그 후보들을 순회할 수 있다.

예시:
```text
host = "example.com"
port = "8080"

DNS / getaddrinfo 결과 후보
  후보 1: IPv6 2606:2800:220:1:248:1893:25c8:1946
  후보 2: IPv6 2606:2800:220:1:248:1893:25c8:1947
  후보 3: IPv4 93.184.216.34
  후보 4: IPv4 93.184.216.35
```

이렇게 후보가 여러 개 나오면 `open_clientfd()`가 하나씩 `socket()`과 `connect()`를 시험해 본다.

```mermaid
flowchart TD
  A[getaddrinfo] -->|실패| R1[-2 반환]
  A -->|성공| B[후보 목록 순회]
  B --> C[socket 생성]
  C -->|실패| B
  C --> D[connect 시도]
  D -->|성공| R2[clientfd 반환]
  D -->|실패| E[close]
  E --> B
  B -->|모두 실패| R3[-1 반환]
```

### 2.7.2 왜 후보 순회가 중요한가
- IPv4/IPv6 등 여러 주소가 있을 수 있으므로, 하나만 고정하지 않고 순회해 연결 안정성을 높인다.
- 실패 자원(`socket` fd)은 즉시 `close`로 회수한다.

## 2.8 echo_client 데이터 루프

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

- 사용자가 입력하면 서버로 전송(`Rio_writen`)
- 서버에서 한 줄 응답 수신(`Rio_readlineb`) 후 화면 출력(`Fputs`)
- EOF 또는 종료 응답이 오면 루프 종료

```mermaid
flowchart TD
  A[readline] --> B{result}
  B -->|positive| C[save line]
  C --> D[print]
  D --> E[next]
  B -->|zero| F[EOF]
  F --> G[end]
  B -->|negative| H[error]
  H --> G
```

해석:
- 반환값이 `0보다 크면` 한 줄을 정상적으로 읽은 것이다.
- 반환값이 `0이면` EOF이거나 상대방이 연결을 종료한 상황으로 본다.
- 반환값이 `0보다 작으면` 읽기 실패로 보고 루프를 끝낸다.

### `buf`가 반복마다 어떻게 바뀌는가

`buf`는 한 번만 쓰고 끝나는 공간이 아니라, `while`이 돌 때마다 **입력값을 담았다가, 서버 응답으로 다시 덮어쓰는 임시 공간**이다.

흐름은 이렇게 볼 수 있다.

```text
사용자 입력
   ↓
fgets(buf, MAXLINE, stdin)
   ↓
buf = "입력한 문자열"
   ↓
Rio_writen(clientfd, buf, strlen(buf))
   ↓
서버가 같은 내용을 돌려줌
   ↓
Rio_readlineb(&rio, buf, MAXLINE)
   ↓
buf = "서버 응답 문자열"
   ↓
Fputs(buf, stdout)
```

예시: 사용자가 순서대로 `hello`, `bye`, 그리고 EOF(입력 종료)를 넣는 경우

```mermaid
flowchart TD
  S[루프 시작]
  S --> L1

  subgraph LOOP1[1회전]
    L1[fgets 결과: hello] --> L2[buf = hello]
    L2 --> L3[Rio_writen 전송]
    L3 --> L4[Rio_readlineb 수신]
    L4 --> L5[buf = hello]
    L5 --> L6[Fputs 출력]
  end

  L6 --> C1{다음 while 검사}
  C1 -->|계속| L7

  subgraph LOOP2[2회전]
    L7[fgets 결과: bye] --> L8[buf = bye]
    L8 --> L9[Rio_writen 전송]
    L9 --> L10[Rio_readlineb 수신]
    L10 --> L11[buf = bye]
    L11 --> L12[Fputs 출력]
  end

  L12 --> C2{다음 while 검사}
  C2 -->|EOF| N[while 종료]
```

핵심 포인트:
- `while`은 `fgets` 결과가 `NULL`이 아닌 동안 계속 돈다.
- 한 바퀴마다 `buf`는 먼저 사용자 입력을 담고, 그 다음 서버 응답으로 덮어써진다.
- `Rio_writen`은 `buf`를 읽어서 보내기만 하고, `Rio_readlineb`가 `buf` 내용을 새로 바꾼다.
- 그래서 `buf`는 루프 한 바퀴 안에서 “송신용 내용”과 “수신용 내용”을 번갈아 담는다.
 - `subgraph` 하나가 `while`의 한 바퀴를 뜻한다고 보면 된다.

## 2.9 `rio_t`는 무엇인가

- `rio_t`는 CS:APP의 Robust I/O에서 쓰는 **읽기 상태 저장용 구조체 타입**이다.
- `buf`가 데이터를 잠깐 담는 공간이라면, `rio_t`는 그 읽기 과정을 관리하는 상태 상자다.
- 소켓/파일에서 데이터를 안정적으로 이어 읽기 위해 `fd`, 남은 바이트 수, 현재 위치, 내부 버퍼를 함께 기억한다.

```c
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* 어느 소켓에서 읽는지 기억한다. */
    int rio_cnt;               /* 아직 소비하지 않은 데이터가 몇 바이트 남았는지 */
    char *rio_bufptr;          /* 내부 버퍼 안에서 현재 읽을 위치에 대한 포인터 */
    char rio_buf[RIO_BUFSIZE]; /* 실제 데이터를 잠깐 저장하는 버퍼 */
} rio_t;
```
### `rio_t` 내부 필드 예시

| 필드 | 의미 | 초기화 직후 예시 | 읽기 중 예시 |
|---|---|---|---|
| `rio_fd` | 연결된 fd | `clientfd` | `clientfd` |
| `rio_cnt` | 내부 버퍼에 남은 바이트 수 | `0` | `12` |
| `rio_bufptr` | 다음에 읽을 위치 | `rio_buf` 시작 주소 | `rio_buf + 5` |
| `rio_buf` | 내부 버퍼 저장 공간 | 빈 버퍼 | `"GET / HTTP/1.0\r\n..."` 일부 저장 |

### `Rio_readinitb`가 하는 일

- `Rio_readinitb(&rio, clientfd)`는 `rio`의 읽기 상태를 `clientfd` 기준으로 초기화한다.
- 즉, `rio`의 필드들을 읽기 시작할 수 있는 상태로 맞춰 주는 함수다.
- `clientfd` 자체를 바꾸는 것이 아니라, `rio`가 `clientfd`를 읽도록 준비시킨다.

초기화 전후 예시:

```c
// 초기화 전
rio.rio_fd = ?;
rio.rio_cnt = ?;
rio.rio_bufptr = ?;

//초기화
Rio_readinitb(&rio, clientfd);

// 초기화 직후
rio.rio_fd = clientfd; //`rio_fd`는 `clientfd`를 기억한다.
rio.rio_cnt = 0; //아직 읽은 데이터가 없으므로 `0`
rio.rio_bufptr = rio.rio_buf; //내부 버퍼의 시작 위치를 가리킨다.
```
- 이후 `Rio_readlineb`가 이 상태를 바탕으로 데이터를 읽어 간다.



## 2.10 핵심 정리
- `argv`는 실행명까지 포함한 문자열 포인터 배열.
- `host`, `port`는 그 배열의 문자열 시작 주소를 담는 `char*` 포인터.
- `open_clientfd`는 이 문자열을 바탕으로 주소 해석 후 소켓을 만들어 정수형 `fd`를 돌려준다.
- `fd`는 커널 소켓 객체를 가리키는 참조로 사용된다.
