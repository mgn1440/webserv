# 웹서버가 매개변수의 경로를 이용하여 configHandler 클래스를 생성한다.

인자로 nginx config 파일이 들어온다. 만약 인자가 존재하지 않는다면 default.conf 파일의 경로를 사용하고 만약 해당하는 config 파일도 존재하지 않는다면 [Error: file does not exist] 에러를 뱉는다. config 파일에 대한 파싱을 진행한다. 상단의 scope에는 types와 server가 들어올 수 있다. types는 한 개만, server 블록은 여러 개로 구성되어 있을 수 있다. types는 mime.types를 관리하는데 types 블록 내에는 [Content-type] [format(extansion)]의 구성으로 이루어져 있다. default 확장자는 text일 시 text/plain이 될 것이고 text가 아닌 이외의 것(실행 파일들)들은 application/octet-stream이 될 것이다. 이 때 Content-type은 [type]/[subtype]으로 구성되어 있어야 하며 형식을 만족시키지 않을 시 fail이다. 마찬가지로 format의 경우는 alphabet + digit의 형식으로 진행되어야 할 것 같다.(추론) std::map<std::string, std::string>의 형식으로 저장되며 format이 key값으로 들어간다. 찾지 못할 시 defalt값을 반환한다.

config에서 발생하는 모든 에러는 [Error: invalid config {reason} setting] 형식을 따른다. 만약 server 블록이 시작된다면 Server클래스에게 std::ifstrem을 넘겨 server 블록 파싱을 시작한다. 마찬가지로 server 블록을 파싱하던 중 location 블록이 시작된다면 Location클래스가 파싱을 맡는다. 이 때 scope가 닫히면 생성자를 끝마친다. 만일 scope가 닫히지 않음에도 불구하고 ifs.eof()를 만나게 된다면 config일 구성이 잘못되었단 뜻이므로 {scope} 에러를 뱉는다. server블록의 생성이 끝나고 난 후 다시 server/type 블록이 시작되지 않는다면 {scope} 에러를 뱉는다.

Location 블록과 Server 블록의 경우 공통되는 특성을 가질 수 있다. Root, Index, CGI, AutoIndex, LimitExcept이며 이들은 Location이 훨씬 우위를 가진다.
Root, Index의 경우 URI(경로) 규칙을 지켜야 한다. 허가되지 않은 문자는 사용할 수 없다. RFC 3986 참조
CGI는 cgi [format] [cgi dir] (Ex: cgi .php /usr/bin/php;)로 구성되어 있으며 마찬가지로 format과 directory에 대한 규칙을 지켜야 한다.
LimitExcept의 경우 구현하고 싶은 메소드를 "대문자"로 작성한다. 대문자가 아닐 때 생각해 봐야 함.

Server 블록의 경우 listen, server_name, error_page, client_max_size를 가질 수 있다. 이 때 블록별로 데이터를 저장하는 방식은 std::map<std::pair<int, std::string>, Server> 이다. 해당 포트에 대하여 default 가 없다면 default를 추가해주도록 한다.
std::map<std::pair<int, std::string>, int>로 std::vector<Server>의 index를 받아오는 방법도 좋다.

Location 블록의 경우 공통의 값들로도 충분한 구성을 할 수 있다.

그 외에도 ConfigHandler가 가져야 하는 인자들은 다음과 같다.
std::map<int, std::string> statCode: statCode에 대한 status string을 저장하는 STL

# 웹서버가 TCP를 이용하여 ServerSocket을 생성하고 이에 해당하는 HttpRequest 클래스를 생성한다.
Config 객체로부터 portList를 받음
WebServer의 SetTCP를 통해 port 개수만큼 serveSocket 만듬
serveSocket을 FILT_READ 이벤트 등록
clientSocket을 생성

# socket으로부터 Read 이벤트가 발생하면 버퍼에 값을 담아서 socket에 해당하는 HttpRequest 클래스에 전달한다.
Http Request를 담당하는 Http 객체 생성 HttpRequest::HttpRequest(int client, ConfigHandler&);
clientSocket FILT_READ 이벤트 등록
clientSocket을 통해 Http Request 요청 발생
-> Http.ReceiveRequestMessage()을 통해 데이터를 보낸다.
-> FILT_TIMER 이벤트를 등록

# HttpRequest 클래스가 request를 파싱하여 해당 데이터를 담은 request 구조체를 반환한다.
    - void HttpRequest::ReceiveRequestMessage(const std::string& data);
    - HttpRequest::HttpRequest(int port, ConfigHandler& configHandler);
    - void HttpRequest::ReceiveRequestMessage(const std::string& data);
    - int* ConfigHandler::GetMaxSize(int port, std::string host);
HttpRequest 클래스에 해당하는 포트번호와 ConfigHandler reference도 같이 전달한다. 클라이언트랑 연결되는 welcome socket마다 HTTPRequest를 하나씩 매핑하여 읽은 내용을 전달한다. 해당 내용은 한 줄씩 파싱되어 struct request에 담겨 return 된다. 파싱하다 Host의 정보를 알게 되었을 때 client max size를 ConfigHandler.GetMaxSize();로부터 전달받는다. 해당 값은 request의 최대 크기를 정의해놓아, 해당 범위를 넘는다면 Error를 발생시킨다.
뒤에서 자세히 후술하겠지만 HEAD를 뺀 나머지 구현 method(GET, POST, PUT, DELETE)에는 body가 존재해야 한다. 이 말은 Content-length 헤더 또는 Transfer-Encoding: chunked 헤더가 존재해야 함을 의미한다. body가 존재하지 않는다면 무슨 에러를 뱉어야 할까?
이 외에 형식에 문제가 없다면 특빌히 파싱해야 하는 start line의 method, direction, version, header line의 host에서 port를 제외한 값을 저장한다. 이는 server_name으로 사용될 것. std::map<std::string, std::string> 꼴로 나머지 헤더를 들고 있는다.


string으로 된 request string을 받는다. buffer에 받은 스트링을 이어붙이고
mParsedRequest.parsedStatus 를 보고 상황에 알맞은 파싱 단계로 들어간다.
start-line 부터일때 - 한줄을 읽고 method uri http-version 으로 나눠서 저장
공백을 기준으로 split을 한다
uri는 origin-form, absolute-form, authority-form, asterisk-form으로 들어 올 수 있는데
origin-form일때는 query문 파싱을 해서 저장을 하고 absolute-form일때는 scheme을 제거한다.
header - 한줄씩 getline으로 읽으면서 map에 field, value로 저장을 한다. maxSize를 비교하기 위해서
mSavedHeaderSize에 읽은 줄의 크기를 저장을 하고 buffer에서 지워주기 위해서 mConsumeBufferSize에도 동일한 값을 더해준다
host가 파싱이 되어있으면 MaxSize 를 비교하고 현재 사이즈가 maxSize를 초과했으면 431
/r/n만 들어온 라인인지 검사해서 맞으면 body parsing으로
/r/n으로 끝나지 않았는데 buffer에 문자가 남아있으면 400 error 아니면 잘려서 들어온 것이므로 위에서 더한 mConsumeBufferSize를 다시 빼준다.
host 헤더의 경우파싱을 한 후 파싱한 데이터를 가지고 config-handler에서 maxSize들을 받아오고 startline의 size를 검사한다.
host 헤더가 두개 들어오면 400 error 를 반환한다.
filed-name: field-value 형식으로 되어있어야 하고 field-name과 콜론 사이에 공백이 있으면 에러(400), 콜론이 없으면 에러(400)
field-value를 trim해서 저장한다.
body - 1. Transefer-Encoding 이고 값이 chunked 면 받는다.
		2. Transefer-Encoding 이고 값이 chunked 가 아니면 400 error 반환하고 tcp연결을 끊는다.
		3. Content-Length랑 Transfer-Encoding이 둘다 존재하면 Content-Lengh를 삭제하고 Transfer-Encoding을 취한다.
		4. Transfer-Encoding이 없고 Content-Length가 이상하면 400 error
		5. Transfer-Encoding이 없고 Content-Length가 존재하면 Content-Length로 파싱
		6. 위의 것들이 다 아니면 body size가 0 or 411(length required)
바디가 필요하지 않은 method에 바디가 있으면 무시한다. 바디가 필요없는 메소드라고 하더라도 바디는 파싱을 해야한다.
바디까지 파싱이 완료되면 request vector에 저장을 하고 버퍼가 남아있으면 다음 순회를 돈다. 반환은 request vector를 반환한다.


# request 구조체를 configHandler에 전달하여 response 구조체를 반환한다.
Http Request가 파싱 후 std::vector<struct request> WebServ 객체에 반환
WebServ 객체에서 std::vector<struct request> -> Config 에 넘겨준다.

std::vector<struct request>를 순회하면서
정의하지 않는 method일 시 501(Not Implemented)을 return 한다.
direction => server, location 순회 -> 맞는 친구
direction을 보고 해당하는 server block과 location block을 탐색한다. 이때 request method의 값이 중요하다. method의 값에 따라서 response의 구성이 달라진다. 이제 method와 request target을 보고 response를 작성한다.

1. static
리소스를 받아온 뒤  위 vector size만큼 Response객체를 만들어서 std::queue(WebServ)에 저장한다.

2. CGI HEADER
std::queue에 저장하고, FILT_WRITE event를 등록한다.
2. CGI BODY
2-1. pipe 만들고
2-2. pipe[0]에 FILT_READ 이벤트 등록
2-3. fork() => 자식은 execve, 부모는 자식의 pid를 FILT_PROC 이벤트 등록
2-4. FILT_READ 이벤트가 발생하면 알맞는 Response 객체 string 멤버 변수에 저장하고, FILT_WRITE 등록



ConfigHandler에서 struct request를 읽고 struct response를 생성한다. 이 때  이 때 Invalid한 request start line일 시 400(Bad Request) or 301(Move Permanently)을 return 한다.
URI의 경우 8000octet 이상 지원을 권장한다.

start line 이 정상적이라고 판단이 되었을 때
response가 body(본문)을 가지지 않는 경우는 다음과 같다.
모든 HEAD, 2xx 응답이 필요한 CONNECT(터널 모드로 전환), 1xx(informational), 204(no content), 304(not modified)응답
비록 size가 0일지라도 모든 나머지 상황은 body를 가진다. body는 \r\n으로 끝나지 않는다.
Content-length: 0\r\n
\r\n

Content-Language(몰?루), Last-Modified(안해도 됨) 사용해야 할까?


- 기본값
    Start line\r\n
    Date: \r\n
    Server: Webserv\r\n
    Content-length:
    Content-Type:

- GET: 대상 리소스의 현재 표현 전송
- HEAD: GET과 동일하나 status line과 header 부문만 전송
    Content-Type과 Content-length, body 불필요
- POST: 요청 페이로드(메시지에서 가장 중요한 부분, body)에서 리소스의 구체적 처리 수행
    - 206(Partial Content), 304(Not Modified), 416(Range Not Satisfiable)으로 전송. in RFC...
    우리가 구현할 것
    200(OK): 리소스 생성 안함.
    201(Created): 하나 이상 리소스가 원 서버 내 생성시. Location
    205(Reset Content): body가 필요 없을 때??
    204 vs 205 What's the difference? 205는 document view를 초기화 할 것을 요구.
- PUT: 대상 리소스의 모든 현재 표현을 요청 페이로드로 대체. 식별자를 보내야 하며 실제로 존재하지 않아도 괜찮다.
    200(OK): 리소스 생성 안함.
    201(Created): 하나 이상 리소스가 원 서버 내 생성시. Location
    204(No Content): body가 필요 없을 때
- DELETE: 대상 리소스의 현재 표현을 모두 제거
    200(OK), 204(No Content)
    body: delete message?

기본적인 response
HTTP/[version] [stat code] [stat]\r\n
Server: Webserv
Date: [Day], [date] [Month] [year] [time] GMT //IMF-fixdate
Content-Type: [mime.types]
Transfer-Encoding: chunked(or Content-Length)

struct response
{
    // preRes
    bool cgi;
    int pipeFd;
    int port;


    // response
	std::string httpVer;
	int statCode;
	std::string stat;
	std::string date;
	std::string contentType;
	int connection;
	std::string expire = "Wed, 11 Jan 1984 05:00:00 GMT";
	std::string body;
};

# 만약 cgi가 사용되었다면 WebServer에서 cgi의 실행 결과를 E_Read로 받아서  response 구조체에 추가한다.
# HttpResponse 클래스에서 response 구조체를 받아서 해당하는 값들로 response문을 작성 후 반환한다.
# Webserv에서 response 문에 해당하는 소켓을 찾아서 해당 값으로 반환한다.

# Question
1. Response 클래스는 하나만 있어도 되는가?
2. CGI를 처리중일 때 같은 client socket에서 요청을 계속 받아 처리해야하는가? 아니면 CGI 또는 timeout을 기다리는 동안 block 해야 하는가?
3. 응답 status code에서 204(No Content)와 205(Reset Content)를 어떻게 구분하는가?
4. char*, string 중 어떤 것을 써야 하는가?