# Flow
EV_READ 이벤트가 터지면 acceptNewClientSocket

acceptNewClientSocket에서 ClientFD, ClientSocket을 만들고 해당하는 값들을 READ 이벤트 등록
Request를 Map에 등록

processHttpRequest 에서 Request vector를 반환받아 각각에 대한 response 생성

if method == "GET" 일 경우 Response::processGET
	CGI == true 일 경우 mbCGI = true; mCGIPath 설정

Response가 CGI 가 아닐 시 바로 WRITE 이벤트 바로 등록
CGI일 시 processCGI



# TODO
- HttpRequest=> HttpHandler로 명칭 변경 요망
- ConfigHandler::GetResponseOf 메서드와 중복 책임. => 하나로 병합 또는 한 쪽 삭제 요망
