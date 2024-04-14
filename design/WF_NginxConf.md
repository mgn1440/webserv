
- 웹서버가 매개변수의 경로를 이용하여 configHandler 클래스를 생성한다.
    - mime.types를 파싱한다.
    - std::map<std::pair<int, std::string>, Server> mServerMap;
    - std::vector<int> mPortVec;
    - 포트는 중복될 수 있다. 다만 ServerName(Host?)이 달라야 한다. ServerName이 없는 경우 같은 포트의 첫 Server를 선택한다.

    - httpRequest() -> port

    Question
    - Host와 ServerName은 어떤 차이가 있는가?