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