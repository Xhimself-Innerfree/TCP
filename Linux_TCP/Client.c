#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
	// 1.create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket:");
		return -1;
	}
	printf("client socket create success........\n");
	// 2.connect
	struct sockaddr_in srvaddr;
	memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(6666);
	srvaddr.sin_addr.s_addr = inet_addr("47.253.235.149");
	int ret = connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
	if (ret < 0)
	{
		perror("connect");
		close(sockfd);
		return -1;
	}
	// 3.send msg
	char buf[1024];
	while (1)
	{
		printf("send: ");
		fgets(buf, sizeof(buf), stdin);
		ret = write(sockfd, buf, sizeof(buf));
		if (ret < 0)
		{
			perror("write");
			break;
		}
		if (strncmp(buf, "quit", 4) == 0)
		{
			break;
		}
	}

	// 4.关闭套接字
	close(sockfd);
	return 0;
}
