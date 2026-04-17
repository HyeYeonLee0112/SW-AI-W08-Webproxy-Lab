# CSAPP 11장 + Proxy Lab 학습 패키지

이 문서 묶음은 정답집이 아니다. 목적은 `tiny`와 `proxy`를 대신 구현해 주는 것이 아니라, 구현 전에 이해해야 할 개념과 흐름을 잡아 주는 것이다.

이 패키지는 다음 순서로 읽는 것을 권장한다.

1. [01-roadmap.md](01-roadmap.md)
2. [02-core-concepts.md](02-core-concepts.md)
3. [03-c-for-networking.md](03-c-for-networking.md)
4. [04-socket-flow.md](04-socket-flow.md)
5. [05-echo-to-tiny.md](05-echo-to-tiny.md)
6. [06-tiny-webserver-guide.md](06-tiny-webserver-guide.md)
7. [07-self-study-framework.md](07-self-study-framework.md)
8. [08-proxy-lab-guide.md](08-proxy-lab-guide.md)
9. [09-proxy-implementation-roadmap.md](09-proxy-implementation-roadmap.md)
10. [10-practice-checklist.md](10-practice-checklist.md)
11. [11-quiz-and-review.md](11-quiz-and-review.md)
12. [12-glossary.md](12-glossary.md)

## 최종 목표
이번 학습의 최종 목표는 아래 5가지를 하나의 흐름으로 이해하는 것이다.

1. 소켓 이해하기
2. 에코 서버 이해하기
3. CSAPP 코드 기반으로 `tiny/tiny.c`, `tiny/cgi-bin/adder.c` 완성하기
4. 11.6c, 11.7, 11.9, 11.10, 11.11을 스스로 공부하고 설명하기
5. `proxy.c`를 완성해 순차 처리, 병렬 처리, 캐시를 구현하기

## 큰 그림
- 소켓은 네트워크 프로그래밍의 입출력 창구다.
- 에코 서버는 "요청을 받고 응답을 보낸다"는 최소 구조를 보여 준다.
- tiny 웹서버는 여기에 HTTP 해석과 정적/동적 콘텐츠 처리가 추가된 형태다.
- proxy 서버는 tiny보다 한 단계 더 나아가, 서버이면서 동시에 클라이언트 역할도 수행한다.
- Proxy Lab의 핵심은 단순 전달이 아니라 `동시성`과 `캐시`까지 포함한 설계다.

## 문서 사용 원칙
- 정답 코드, 완성 코드, 문제 정답은 직접 제공하지 않는다.
- 대신 개념, 책임, 흐름, 체크 질문, 실습 관찰 포인트를 제공한다.
- 각 문서를 읽고 나면 "무엇을 구현해야 하는지"보다 먼저 "왜 그렇게 구현하는지"를 설명할 수 있어야 한다.

## 팀원별 11.1~11.4 정리 문서
11.1~11.4 범위는 각자 책을 읽고 자기 언어로 정리할 수 있도록 팀원별 문서를 따로 두었다.

- [CYB 정리](CYB/11.1-11.4-study.md)
- [IHY 정리](IHY/11.1-11.4-study.md)
- [IJC 정리](IJC/11.1-11.4-study.md)
- [JHS 정리](JHS/11.1-11.4-study.md)
