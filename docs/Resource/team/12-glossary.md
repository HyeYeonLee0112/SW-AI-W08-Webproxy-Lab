# 12. 용어집

## BSD Socket
운영체제가 제공하는 네트워크 통신 인터페이스. C 네트워크 프로그래밍은 대부분 이 API를 통해 이뤄진다.

## File Descriptor
열린 입출력 대상을 가리키는 정수 핸들. 파일뿐 아니라 소켓도 file descriptor다.

## Hostname
사람이 읽기 쉬운 서버 이름. 실제 연결 직전에는 보통 DNS를 통해 IP 주소로 해석된다.

## IP Address
네트워크에서 호스트를 식별하는 주소. 포트와 함께 있어야 특정 서비스에 연결할 수 있다.

## Port
한 호스트 안에서 어떤 서비스와 통신할지 구분하는 번호.

## TCP
신뢰성 있고 순서를 보장하는 바이트 스트림 전송 프로토콜. HTTP 통신의 기반이 된다.

## UDP
연결 없이 데이터를 보내는 전송 프로토콜. 빠르지만 전달 보장이 없다.

## HTTP
웹에서 요청과 응답을 주고받는 애플리케이션 계층 프로토콜.

## DNS
호스트 이름을 IP 주소로 바꾸는 시스템.

## Iterative Server
한 번에 하나의 연결을 처리하는 서버 구조. 단순하지만 느린 요청에 약하다.

## Concurrent Server
여러 연결을 동시에 처리할 수 있는 서버 구조. 스레드나 프로세스 같은 동시성 도구가 필요하다.

## CGI
서버가 외부 프로그램을 실행해 그 결과를 동적 콘텐츠로 응답하는 방식.

## URI
서버에서 요청 대상 자원을 가리키는 식별자. tiny는 URI를 보고 정적/동적 요청을 구분한다.

## Origin Server
실제 콘텐츠를 가지고 있는 원래 서버. Proxy Lab에서는 tiny가 이 역할을 한다.

## Proxy
클라이언트와 원서버 사이에서 요청과 응답을 중계하는 서버.

## Cache Hit
요청한 객체가 이미 캐시에 있어 원서버에 다시 가지 않아도 되는 상태.

## Cache Miss
요청한 객체가 캐시에 없어 원서버에서 다시 가져와야 하는 상태.

## User-Agent
클라이언트 종류를 나타내는 HTTP 헤더. Proxy Lab starter code에도 관련 문자열이 등장한다.

## Header
요청 또는 응답의 메타데이터 줄. HTTP 메시지에서 본문 앞에 위치한다.

## Request Line
HTTP 요청의 첫 줄. method, URI, version으로 구성된다.

## Response Line
HTTP 응답의 첫 줄. version, status code, reason phrase로 구성된다.
