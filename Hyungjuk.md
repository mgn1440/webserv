Directs : listen server_name error_page client_max_body_size rewrite root autoindex index
Scope : server location if limit_except

- RFC 7230 ~ 7235 : HTTP
- RFC 3875 : [CGI](https://qaos.com/sections.php?op=viewarticle&artid=194)

- Form, Isindex tag, 

- 어떤 종류의 파일인지 분석(SSI, CGI, Perl)
- cgi-bin 디렉토리에서 가동
- 같은 포트를 가질 시에는 server_name을 확인하며 만약 일치하는 server가 존재하지 않을 시 port 일치의 맨 처음, 또는 default server 사용.
- Host header field가 존재하지 않으면 Error
- 
