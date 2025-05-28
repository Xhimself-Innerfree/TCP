#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CLIENTS FD_SETSIZE
#define BUF_SIZE 1024

int main(void)
{
	int sockfd, connfd, ret, maxfd, i;
	int client_fds[MAX_CLIENTS];
	struct sockaddr_in myaddr;
	char buf[BUF_SIZE];

	// 1. create
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket");
		return -1;
	}
	printf("socket create success..................\n");

	// 2. bind
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(6666);
	myaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	ret = bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if (ret < 0)
	{
		perror("bind:");
		close(sockfd);
		return -1;
	}

	// 3. listen
	ret = listen(sockfd, 8);
	if (ret < 0)
	{
		perror("listen:");
		close(sockfd);
		return -1;
	}
	printf("listen...............\n");

	// 4. initialize client fd array
	for (i = 0; i < MAX_CLIENTS; i++)
		client_fds[i] = -1;

	fd_set allset, rset;
	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);
	maxfd = sockfd;

	while (1)
	{
		rset = allset;
		int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready < 0)
		{
			perror("select:");
			break;
		}

		// New client connection
		if (FD_ISSET(sockfd, &rset))
		{
			connfd = accept(sockfd, NULL, NULL);
			if (connfd < 0)
			{
				perror("accept:");
				continue;
			}
			printf("New client connected: fd=%d\n", connfd);
			for (i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_fds[i] < 0)
				{
					client_fds[i] = connfd;
					break;
				}
			}
			if (i == MAX_CLIENTS)
			{
				printf("Too many clients\n");
				close(connfd);
			}
			else
			{
				FD_SET(connfd, &allset);
				if (connfd > maxfd)
					maxfd = connfd;
			}
			if (--nready <= 0)
				continue;
		}

		// Check all clients for data
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			int fd = client_fds[i];
			if (fd < 0)
				continue;
			if (FD_ISSET(fd, &rset))
			{
				memset(buf, 0, sizeof(buf));
				ret = read(fd, buf, sizeof(buf));
				if (ret <= 0)
				{
					if (ret == 0)
						printf("Client fd=%d closed connection\n", fd);
					else
						perror("read:");
					close(fd);
					FD_CLR(fd, &allset);
					client_fds[i] = -1;
				}
				else
				{
					printf("recv from fd=%d: %s\n", fd, buf);
					// Broadcast to other clients
					for (int j = 0; j < MAX_CLIENTS; j++)
					{
						if (client_fds[j] >= 0 && client_fds[j] != fd)
						{
							write(client_fds[j], buf, ret);
						}
					}
				}
				if (--nready <= 0)
					break;
			}
		}
	}

	// Cleanup
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_fds[i] >= 0)
			close(client_fds[i]);
	}
	close(sockfd);
	return 0;
}