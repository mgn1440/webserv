# Flow

## Parse Config file

- Max StartLine Size: 10K, Max HeaderSzie: 1M
- client_max_size only set body size
- what about redirection? 

## Create Server Socket

- createServerSocket
	- listenFd(serverFd) value를 WebServ::mServSockList에 저장

- setKqueue
	- listenFd를 담은 kevent를 mChangeList에 pushback 후 mKq에 등록
	- mChangeList clear

## Run Kqueue Loop

### EVFIILT_READ 

- acceptNewClientSocket
	- client Socket 생성 후 EVFILT_READ이벤트 등록
	- client Socket당 Handler와 Response 배열 등록

acceptNewClientSocket에서 ClientFD, ClientSocket을 만들고 해당하는 값들을 READ 이벤트 등록
Request를 Map에 등록

processHttpRequest 에서 Request vector를 반환받아 각각에 대한 response 생성

if method == "GET" 일 경우 Response::processGET
	CGI == true 일 경우 mbCGI = true; mCGIPath 설정

Response가 CGI 가 아닐 시 바로 WRITE 이벤트 바로 등록
CGI일 시 processCGI



# TODO
- client_max_size Body size만 받도록 바꾸기
- redirection 구현하기
- file 전송할 때 중간에 NULL이 들어올 수 있으므로 mBody는 따로 보내기. size도 같이 저장해야 함.

# 제언

- 디렉터가 필요하다.
- 집중하는 시간 필요.


POST /directory/youpi.bla HTTP/1.1
Host: localhost:8000
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
X-Secret-Header-For-Test: 0
Accept-Encoding: gzip


# 평가표 점검
- socket fd를 통한 read/write 오류시 클라이언트 제거
- 모든 fd의 read/write eturn 값이 0이랑 -1 모두 체크하기
- 사제 error page 설정하기
- delete 실패시 403 forbidden 처리하기
- 쿼리문 실제 테스트 해보기
- wordpres/loop.php 돌리다가 중간에 창 닫았을 때 이하 에러 발생(클라이언트에서 먼저 끊었을 때)
	addEventError: : Bad file descriptor
	- 같은 port로 접근시 0x10f5445e3 in WebServ::writeHttpResponse WebServ.cpp:358 에서 seg falut
- autoindex referer의 경로를 앞에 붙일 때 에러 발생(referer 처리유무에 대한 토론 필요)
- autoindex 위에 절대경로 root로부터의 상대경로로 바꾸기


# Seg Fault Fix
- Response에 clientFD, pipeRead, pipeWrite, pid 값을 가지도록 한다.
	## 구성
		std::map<int, int> mServSockPortMap; // key: servSocket, val: port
		std::map<int, HttpHandler> mRequestMap; // key: clientFD, value: HttpHandler
		std::map<int, std::deque<Response> > mResponseMap; // key: clientFD, value: Response

		std::map<int, Response*> mCGIPipeMap; // key: readPipeFD, value: Response pointer
		std::map<int, Response*> mCGIClientMap;  // key: clientFD, value: Response pointer  
		std::map<int, Response*> mCGIPidMap; // key: pid, value: Response pointer
		std::map<int, Response*> mCGIPostPipeMap; // key: writePipeFD, value: Response pointer

		mClientFd;
		mPipeFd[2];
		mPid;
		mWritten;
	## 제언
		- deque를 없애는 구조로 가는 것
