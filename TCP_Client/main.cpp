#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <thread>
#include <atomic>

int main()
{
	char IPv4_Server[100];
	std::cout << "Client" << std::endl;
	std::cout << "please input the server IP address: ";
	std::cin >> IPv4_Server;

	// Entrust in Windows Sockets
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Create a socket
	SOCKET Client_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == Client_Socket)
	{
		printf("socket() failed\n");
		return -1;
	}

	// Connect to the server
	struct sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_port = htons(6666);
	target.sin_addr.s_addr = inet_addr(IPv4_Server);

	if (-1 == connect(Client_Socket, (struct sockaddr *)&target, sizeof(target)))
	{
		printf("connect() failed\n");
		closesocket(Client_Socket);
		return -1;
	}

	std::atomic<bool> running(true);

	// Thread for receiving messages
	std::thread recv_thread([&]()
							{
        char recvbuf[1024] = { 0 };
        while (running) {
            int rec = recv(Client_Socket, recvbuf, sizeof(recvbuf) - 1, 0);
            if (rec <= 0) {
                printf("Server closed connection or recv() failed\n");
                running = false;
                break;
            }
            recvbuf[rec] = '\0';
            printf("\nReceived message: %s\n", recvbuf);
            printf("Please input the message to send: ");
            fflush(stdout);
        } });

	// Main thread for sending messages
	while (running)
	{
		char sendbuf[1024] = {0};
		printf("Please input the message to send: ");
		if (!fgets(sendbuf, sizeof(sendbuf), stdin))
		{
			running = false;
			break;
		}
		// Remove trailing newline
		size_t len = strlen(sendbuf);
		if (len > 0 && sendbuf[len - 1] == '\n')
			sendbuf[len - 1] = '\0';
		if (strcmp(sendbuf, "quit") == 0)
		{
			running = false;
			break;
		}
		send(Client_Socket, sendbuf, strlen(sendbuf), 0);
	}

	closesocket(Client_Socket);
	running = false;
	if (recv_thread.joinable())
		recv_thread.join();
	WSACleanup();
	return 0;
}