#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
//Client by JL

int main() {
	char IPv4_Server [100];
	std::cout << "Client" << std::endl;
	std::cout << "please input the server IP address: ";
	std::cin >> IPv4_Server;

	//Entrust in Windows Sockets
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Create a socket
	SOCKET Client_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == Client_Socket) {
		printf("socket() failed\n");
		return -1;
	}

	//Connect to the server
	struct sockaddr_in target;
	target.sin_family = AF_INET;  // AF_INET IPv4    AF_INET6 IPv6
	target.sin_port = htons(8080); //the same as the server
	target.sin_addr.s_addr = inet_addr(IPv4_Server); //IP address of the server

	if (-1 == connect(Client_Socket, (struct sockaddr*)&target, sizeof(target))) {
		printf("connect() failed\n");
		closesocket(Client_Socket);
		return -1;
	}

	//Start Communication
	while (1) {
		char sendbuf[1024] = { 0 };
		printf("Please input the message to send: ");
		scanf("%s", sendbuf);
		send(Client_Socket, sendbuf, strlen(sendbuf), 0);

		char recvbuf[1024] = { 0 };
		int rec = recv(Client_Socket, recvbuf, sizeof(recvbuf), 0);
		if (rec <= 0) {
			printf("recv() failed\n");
			break;
		}
		printf("Received message: %s\n", recvbuf);
	}

	//Close the socket
	closesocket(Client_Socket);

	return 0;
}