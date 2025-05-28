#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int running = 1;

void *recv_thread(void *arg)
{
	int sockfd = *(int *)arg;
	char buf[1024];
	while (running)
	{
		memset(buf, 0, sizeof(buf));
		int n = read(sockfd, buf, sizeof(buf) - 1);
		if (n <= 0)
		{
			printf("Server closed connection or recv failed.\n");
			running = 0;
			break;
		}
		buf[n] = '\0';
		printf("\nrecv: %s", buf);
		printf("\nsend: ");
		fflush(stdout);
	}
	return NULL;
}

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

	pthread_t tid;
	pthread_create(&tid, NULL, recv_thread, &sockfd);

	char buf[1024];
	while (running)
	{
		printf("send: ");
		if (fgets(buf, sizeof(buf), stdin) == NULL)
		{
			running = 0;
			break;
		}
		ret = write(sockfd, buf, strlen(buf));
		if (ret < 0)
		{
			perror("write");
			running = 0;
			break;
		}
		if (strncmp(buf, "quit", 4) == 0)
		{
			running = 0;
			break;
		}
	}

	close(sockfd);
	running = 0;
	pthread_join(tid, NULL);
	return 0;
}