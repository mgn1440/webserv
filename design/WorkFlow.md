# 워크 플로우 개요
- 설정 파일 파싱
- kqueue를 이용한 서버의 기본 뼈대 제작
- RFC를 준수하는 GET, POST... 순으로 메소드 구현
- 하나의 메소드 구현마다 테스터를 이용한 테스트
- CGI 구현

# 주의 사항
- 설정파일은 nginx 를 참고하되, 완전히 똑같을 필요는 없음.
- select() 는 read set, write set 을 한번에 체크하고 select() 이후 클라이언트 소켓별로 단 한번의 read 또는 write 를 써야만 한다. 이는 subject에도 명시되어있으며, 이를 어길시 0점. 해당 부분이 문제가 되면 코드를 처음부터 갈아엎어야하는 불상사가 생길수도...
- read/recv/write/send 의 리턴값을 반드시 체크. -1, 0, 그 이상인지 모두 체크해야함. errno 체크시 0점.3
- curl, telnet 명령어를 이용한 테스트는 필수. 특히 telnet이 동작하는 방식을 잘 보면 단일 연결에 여러개의 http request 를 보낼 수 있는 것을 알 수 있는데 tester 를 통과하려면 단일 연결에 여러개의 요청을 처리할 수 있어야 한다. (nginx 를 이용, telnet으로 테스트해보자)
- siege -b 명령어로 서버 부하테스트에서 99.5% 를 넘겨야함. 해당 명령어를 수행하면 무한루프가 되는데 ctrl-c 를 할때까지 멈추지 않고 계속 무한히 돌아가야함.
- 영어를 잘하는 동료와 같이하면 좋음.
- rfc 의 모든 부분을 읽을 필요는 없다. 꼭 알아야 하는 중요한 부분만 읽으면 되는데 문제는 중요한 부분이 뭔지 모른다는 것. 중요한게 무엇인지 알기 위해서는 읽고 또 읽어야한다....
- 오랫동안 테스터를 통과하지 못하면 테스터가 문제가 있다는 현실부정 & 뇌내망상이 생기는데 테스터에는 절대로 문제가 없다. 문제가 있는건 내 코드.
- 테스터를 통과하면 엄청난 것을 볼 수 있다.

# 워크 플로우
- 웹서버가 매개변수의 경로를 이용하여 configHandler 클래스를 생성한다.
    - ConfigHandler
    - std::map<std::pair<int, std::string>, Server> mServerMap;
    - std::vector<int> mPortVec;
    - 포트는 중복될 수 있다. 다만 ServerName(Host?)이 달라야 한다. ServerName이 없는 경우 같은 포트의 첫 Server를 선택한다.

    - httpRequest() -> port

    Question
    - Host와 ServerName은 어떤 차이가 있는가?
- 웹서버가 TCP를 이용하여 socket을 생성하고 이에 해당하는 HttpRequest 클래스를 생성한다.
    - std::vector<int> configHandler.getPort();
    - 각각의 포트번호를 돌면서 TCP Server socket 생성.
    - 해당 번호에 대해서 클라이언트의 요청이 오면 welcome socket을 만들고 이에 해당하는 HttpRequest 클래스를 생성하여 std::map<int, HttpRequest*> 에 담아 저장한다.

    Question
    - 각 request method의 특징은 어떻게 되는가?
    - 각 request method에 대응하여 response는 어떻게 변화하는가?
- socket으로부터 Read 이벤트가 발생하면 버퍼에 값을 담아서 socket에 해당하는 HttpRequest 클래스에 전달한다.
    - void HttpRequest::ReceiveRequestMessage(const std::string& data);
- HttpRequest 클래스가 request를 파싱하여 해당 데이터를 담은 request 구조체를 반환한다.
    - HttpRequest 클래스에 해당하는 포트번호도 같이 전달한다.
- request 구조체를 configHandler에 전달하여 response 구조체를 반환한다.
- 만약 cgi가 사용되었다면 WebServer에서 cgi의 실행 결과를 E_Read로 받아서  response 구조체에 추가한다.
    
    Question
    - pipeFd를 kqueue에 등록하는 과정은 어디에서 일어나는가?
    - fork를 해서 cgi를 돌리는 것은 configHandler에서 일어날텐데...
- HttpResponse 클래스에서 response 구조체를 받아서 해당하는 값들로 response문을 작성 후 반환한다.
- Webserv에서 response 문에 해당하는 소켓을 찾아서 해당 값으로 반환한다.


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