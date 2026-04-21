# GitHub Project Issue Drafts

기준:
- 참고 자료: `docs/Resource/CSAPP_2016_Ch11_Network_Programming_p940-992.pdf`
- 정리 방향: `학습`은 개념 정리, `구현`은 구현 포인트 정리, `테스티`는 검증/리뷰 포인트 정리
- 제외: `공통`

## 구현

### 설명 초안
- Tiny와 Proxy의 핵심 흐름을 구현 관점에서 정리한다.
- 서버는 `socket -> bind -> listen -> accept` 흐름으로 연결을 받고, `connfd` 기준으로 요청을 처리한다.
- Echo에서 Tiny로 넘어가면 HTTP 요청 해석, 정적 파일 처리, CGI 분기가 추가된다.
- Proxy에서는 브라우저 쪽 서버 역할과 원서버 쪽 클라이언트 역할을 동시에 다뤄야 한다.

### comment 초안
- 구현 관점에서 가장 먼저 잡아야 할 것은 `listenfd`와 `connfd`의 역할 분리다.
- `listenfd`는 연결 대기용이고, `connfd`는 실제 요청/응답을 처리하는 소켓이다.
- Tiny는 `request line`, `headers`, `URI 분기`, `static/dynamic 처리`를 순서대로 구현하는 것이 핵심이다.
- Proxy는 여기에 더해 upstream 서버로 다시 연결하는 `connect` 흐름과 캐시 처리가 추가된다.

### sub-issue 초안
- `socket`, `bind`, `listen`, `accept` 흐름 구현
- `echo`에서 `tiny`로 확장되는 요청 처리 구조 구현
- 정적 파일 응답 처리 구현
- CGI 기반 동적 응답 처리 구현
- Proxy의 브라우저 측/원서버 측 이중 연결 구조 구현

## 테스티

### 설명 초안
- 구현한 흐름이 실제로 기대한 대로 동작하는지 검증한다.
- 소켓 연결 생성, 요청 읽기, 응답 쓰기, 연결 종료가 끊기지 않는지 확인한다.
- 정적 요청과 동적 요청이 각각 올바른 경로로 분기되는지 확인한다.
- Proxy에서는 캐시 hit/miss와 원서버 재연결이 의도대로 움직이는지 확인한다.

### comment 초안
- 테스트는 단순히 성공 여부만 보는 것이 아니라, 어떤 단계에서 실패했는지 확인하는 용도로 작성한다.
- `accept` 이후 `connfd`가 정상적으로 요청을 처리하는지, `close`가 적절한 시점에 호출되는지 확인해야 한다.
- Tiny에서는 `parse_uri`, `serve_static`, `serve_dynamic`, `clienterror`가 각각 기대한 응답을 내는지 검증이 중요하다.
- Proxy에서는 여러 요청이 연속으로 들어올 때 연결과 캐시가 안정적으로 유지되는지 보는 것이 중요하다.

### sub-issue 초안
- `listenfd` / `connfd` 분리 검증
- Echo 서버 송수신 루프 검증
- Tiny 정적 콘텐츠 응답 검증
- Tiny CGI 동적 콘텐츠 응답 검증
- Proxy 캐시 hit/miss 검증

## 학습

### 설명 초안
- Chapter 11의 핵심 개념을 주제별로 정리한다.
- 소켓, `fd`, 커널 객체, `listenfd` / `connfd`의 차이, `fork` vs `thread pool`, Tiny와 Proxy의 역할 차이를 이해하는 것이 목표다.

### comment 초안
- 소켓은 프로세스가 직접 들고 있는 객체가 아니라, 커널이 관리하는 통신 자원을 `fd`로 참조하는 구조로 이해해야 한다.
- `listenfd`는 연결 요청을 받는 입구이고, `connfd`는 특정 클라이언트와 통신하는 실제 소켓이다.
- `fork` 기반 병렬 처리와 `thread pool` 기반 병렬 처리의 차이는 성능만이 아니라 격리, 공유 메모리, 동기화 난이도까지 포함해서 봐야 한다.
- Tiny는 HTTP 요청 해석과 응답 생성의 기본 구조를 익히기 위한 교육용 서버이고, Proxy는 그 흐름 위에 중계와 캐시를 추가한 구조다.

### sub-issue 초안
- 소켓과 `fd`의 관계 정리
- `listenfd` / `connfd` 구조 정리
- Echo에서 Tiny로 넘어가는 이유 정리
- Tiny의 정적/동적 처리 정리
- `fork` 방식과 `thread pool` 방식 비교 정리
- Proxy가 필요한 이유 정리

