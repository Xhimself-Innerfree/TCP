#include <stdio.h>
#include <string.h>	
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
//Server by JL
DWORD WINAPI thread_function(LPVOID lpThreadParameter)
{
	//printf("Thread created\n");
	SOCKET Client_socket = *(SOCKET*)lpThreadParameter;
	free(lpThreadParameter);
	while (1) {
		//Receive data
		char buffer[1024] = { 0 };//buffer to store the received data
		int bytes_received = recv(Client_socket, buffer, sizeof(buffer), 0);
		if (bytes_received <= 0) break;
		printf("FNL_Server: Copy that %llu: %s\n", Client_socket, buffer);
		send(Client_socket, buffer, bytes_received, 0);
		//closesocket(Client_socket);
	}

	printf("FNL_Server: Connection closed %llu\n", Client_socket);
	closesocket(Client_socket);

	return 0;
}

//Get IPv4 address of Server


int main() {
	//Get IPv4
	ULONG bufferSize = 15000;
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);

	// Adapter
	DWORD result = GetAdaptersAddresses(
		AF_INET,                 //  IPv4 
		GAA_FLAG_INCLUDE_PREFIX, // 
		NULL,
		pAddresses,
		&bufferSize
	);

	if (result == ERROR_BUFFER_OVERFLOW) {
		free(pAddresses);
		pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
		result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &bufferSize);
	}

	if (result != NO_ERROR) {
		std::cerr << "GetAdaptersAddresses failed: " << result << std::endl;
		free(pAddresses);
		return 1;
	}

	// get every adapter
	for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != NULL; pCurr = pCurr->Next) {
		// skip 127.0.0.1
		if (pCurr->OperStatus != IfOperStatusUp ||
			pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) { 
			continue;
		}

		//  IPv4 
		PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress;
		while (pUnicast) {
			if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
				sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
				char ipStr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(sa_in->sin_addr), ipStr, INET_ADDRSTRLEN);
				std::cout << "Adapter: " << pCurr->FriendlyName << "\nIPv4: " << ipStr << std::endl;
			}
			pUnicast = pUnicast->Next;
		}
	}

	free(pAddresses);

	//Entrust in Windows Sockets
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Create a socket
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM is for TCP connection SOCK_DGRAM is for UDP connection
	if (listen_socket == INVALID_SOCKET) {
		printf("Error creating socket: %d\n", WSAGetLastError());
		return -1;
	}

	//Set up the server address
	struct sockaddr_in local = { 0 };
	local.sin_family = AF_INET; // AF_INET IPv4    AF_INET6 IPv6
	local.sin_port = htons(8080); //port
	local.sin_addr.s_addr = inet_addr("0.0.0.0");//htonl(INADDR_ANY); //IP "0.0.0.0" means that the server will listen on all network interfaces

	//Bind the socket
	if (bind(listen_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		printf("Error binding socket: %d\n", WSAGetLastError());
		return -1;
	}

	//Listen on the socket
	if (listen(listen_socket, 10) == SOCKET_ERROR) { //10 is the maximum number of connections that can be queued
		printf("Error listening on socket: %d\n", WSAGetLastError());
		return -1;
	}

	//Wait & Accept connections
	while (1) {
		SOCKET Client_socket = accept(listen_socket, NULL, NULL);
		if (Client_socket == INVALID_SOCKET) {
			continue;
		}

		printf("FNL_Server: Connection established %llu\n", Client_socket);

		SOCKET* sockfd = (SOCKET*)malloc(sizeof(SOCKET));
		*sockfd = Client_socket;  //protect your socket

		//MultiThread a Server can connect to multiple clients
		CreateThread(NULL, 0, thread_function, sockfd, 0, NULL);
	}


	return 0;
}