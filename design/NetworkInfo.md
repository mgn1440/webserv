GET /wp-admin/edit.php?post_type=post HTTP/1.1
Host: localhost:4242
Connection: keep-alive
sec-ch-ua: "Chromium";v="116", "Not)A;Brand";v="24", "Google Chrome";v="116"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: none
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Accept-Encoding: gzip, deflate, br
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

HTTP/1.1 200 OK
Server: nginx
Date: Thu, 28 Mar 2024 04:33:09 GMT
Content-Type: application/json; charset=UTF-8
Transfer-Encoding: chunked
Connection: keep-alive
X-Powered-By: PHP/8.1.27
X-Robots-Tag: noindex
X-Content-Type-Options: nosniff
Referrer-Policy: strict-origin-when-cross-origin
X-Frame-Options: SAMEORIGIN
Expires: Wed, 11 Jan 1984 05:00:00 GMT
Cache-Control: no-cache, must-revalidate, max-age=0, no-store, private

62
{"wp-refresh-post-lock":{"new_lock":"1711600389:1"},"wp-auth-check":true,"server_time":1711600389}
0



GET /wp-admin/edit.php?post_type=post HTTP/1.1
Host: localhost:4242
Connection: keep-alive
sec-ch-ua: "Chromium";v="116", "Not)A;Brand";v="24", "Google Chrome";v="116"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: none
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Accept-Encoding: gzip, deflate, br
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7


구현 관련 팁

- 객체의 상태를 변경하는 명령은 반환값을 가질 수 없다.
- 객체의 정보를 반환하는 쿼리는 상태를 변경할 수 없다.

# RFC 3986
- URL : scheme://[userinfo@][host]:[port]/[path]?[query]#[fragment]
- "_" 지양 "-" 지향. 마지막에 "/" 포함 불가능.
- 파일 확장자 URI에 비포함. Content-Type을 통해서 body 처리 방법 결정 (Accept 요청 헤더 활용 권장)
- xchar = unreserved | reserved | extra
- unreserved = alphabet | digits | safe | extra
- safe = " $-_.+ ", extra = " !*'(); "
- reserved = " ; /계층적 구조 ?쿼리의 시작 : @ &=쿼리 스트링 파라미터 구분과 키 값 연결 "
- "~" == %7E == %7e

# RFC 7230
- HTTP의 connection less, state less 특성을 극복하기 위해 RESTful API 사용
- GET, POST, PUT, PATCH, DELETE
- 프록시(Proxy) : 중간에 요청을 받아서 서버를 매개해주는 장치
- 리버스 프록시(Reverse Proxy) : 내부 망에서 보안, 속도, 방화벽등을 이유로 서버를 매개해주는 장치
- ABNF(Augmented BNF) : 구문 정의 형식
- 헤더를 읽고 헤더 데이터를 이용해 메시지 본문이 필요한지 여부 파악
- Request와 Response의 차이는 start line과 body 길이 결정 알고리즘이다.
- Date, Host등 제어 데이터가 포함된 헤더 필드를 먼저 보내라.
- 메시지 본문이 있을 시 Content-Length 또는 Transfer-Encoding 헤더 필드에 표시된다.
- 응답 메시지의 본문 규칙은 다음과 같다.
    - HEAD 요청의 응답은 메시지 본문을 포함하지 않는다.
    - CONNECT 요청에 2xx 응답은 메시지 본문 대신 터널 모드로 전환한다.
    - 1xx(informational), 204(no content), 304(not modified)응답에는 본문이 포함되지 않는다.
    - 나머지는 모두 본문을 가지나 길이는 0일 수 있다.
- Content length에 대한 오버플로 방지를 해야한다.
- Transfer-Encoding을 통해서 body를 chunk 단위로 보낼 수 있다. 마지막 chunk는 항상 0으로 끝나야 한다.(CGI 사용에 유리할 듯)
- Request target form
    - origin form: GET /where?q=now HTTP/1.1
    - absolute form: GET http://ex1.org/pub/proj.html HTTP/1.1
        - 프록시 요청에 필요.
    - authority-form, asterisk-form...
- HOST 헤더는 Start line 이후에 바로 오는 것이 좋다.
- HTTP/1.1은 영속적 커넥션(Persistent Connectio)을 사용한다.
- 커넥션을 끊기 위해서 close 를 사용한다. 해당 요청을 받은 서버는 최종 응답 발송 후 커넥션 종료를 시작한다.



##  Request Message Format
[method] [target] [HTTP ver] \r\n => request line

// if data is chunked
Transfer-Encoding: Chunked \r\n
\r\n
[HEXA-length]\r\n
This is HTTP Chunk transfer encoding\r\n
Last length of Chunk is 0


## Request Rule
- request line에서 Invalid시 400(Bad Req) or 301(Move Permanently), 8000octet 이상 지원 권장
- 정의되지 않은 method시 501(Not Implemented)
- request target이 길면 414(URI too long)
- 헤더 필드 형식 : "[field name]:[OWS][field values][OWS]"
- field name 뒤에 OWS가 올 시 400
- 정해진 헤더 필드 크기를 초과하면 4xx(Client Errror)를 응답해야 한다.
- Accept: 허용 가능한 응답 미디어 타입 지정


# RFC 7231
- MIME.types = type/subtype. type에는 discrete와 multipart 두 가지 클래스로 나뉜다.
- discrete: application(/octet-stream), audio, example, font, image, model, text(/plain), video
- multipart: message, multipart
- important mime.types: application/octget-stream, text/plain, text/css, text/html, text/javascript 

- Content-Type를 통해서 브라우저에 값을 전송하는 것을 upload.(subject file 참조)
- text type은 HTTP에서 \r\n, \r, \n 중 하나로 통일해 전송해야 한다.

- GET: 대상 리소스의 현재 표현 전송
- HEAD: GET과 동일하나 status line과 header 부문만 전송
- POST: 요청 페이로드(메시지에서 가장 중요한 부분, body)에서 리소스의 구체적 처리 수행
    - 206(Partial Content), 304(Not Modified), 416(Range Not Satisfiable)으로 전송. 하나 이상의 리소스가 원 서버에 생성되었을 경우에는 201(Created) 전송. 생성시에는 Location header 사용?
- PUT: 대상 리소스의 모든 현재 표현을 요청 페이로드로 대체. 식별자를 보내야 하며 실제로 존재하지 않아도 괜찮다.
    - 생성: 201(Created), 수정: 200(OK), 204(No Content)
- DELETE: 대상 리소스의 현재 표현을 모두 제거
- CONNECT: 대상 리소스로 식별된 서버에 대한 터널 설립
- OPTIONS: 옵션 설명
- TRACE: 루프백 테스트 수행

- 응답 status code(Ref 03)

# RFC 7233
- 206(Partial Content): 단일 부분 전송일 경우 Content-Range 헤더와 페이로드 필수
- 206(Partial Content): 여러 부분 전송일 경우 multipart/byteranges 와multipart/byteranges Content-type 헤더 필드를 생성
- 서버가 범위 요청을 지원하지 않는 경우 서버는 200 OK 상태를 돌려보낸다.

## Response Message Format
- Date: Tue, 15 Nov 1994 08:12:31 GMT
- Location: 응답과 관련된 특정 리소스를 가리킴

# Reference
- 01:RFC: https://hochan049.gitbook.io/cs-interview/web/
- 02:Mime types: https://developer.mozilla.org/ko/docs/Web/HTTP/Basics_of_HTTP/MIME_types
- 03:응답 Status Code: https://hochan049.gitbook.io/cs-interview/web/rfc-7321-http-1.1/6.-response-status-codes

