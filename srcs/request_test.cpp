#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "HttpRequest.hpp"

#define PORT 4242

std::string getIndexListOf(const std::string& path);
int main() {
    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[30000] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

	int reuse = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while (1) {
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: html\n";
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 30000);

		HttpRequest http(8080);
		std::vector<struct Request> req = http.ReciveRequestMessage(buffer);
        printf("%s\n", buffer);
		std::string path;
		path += ".";
		path += req[0].URI;
		try{
			std::string dir_info = getIndexListOf(path);
			hello += "Content-Length";
			hello += dir_info.size();
			hello += "\n\n";
			hello += dir_info;
		}
		catch (std::exception & e){
			std::cout << "is not directory!" << std::endl;
		}
        write(new_socket, hello.c_str(), hello.size());
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}
