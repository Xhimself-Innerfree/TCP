#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(void)
{
	//1.create
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("socket");
		return -1;
	}
	printf("socket create success..................\n");

	//2.bind
	struct sockaddr_in myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port	  = htons(6666);//port
	myaddr.sin_addr.s_addr = inet_addr("0.0.0.0");//IP
	int ret = bind(sockfd,(struct sockaddr*)&myaddr,sizeof(myaddr));
	if(ret < 0){
		perror("bind:");
		goto err;
	}

	//3.setup a listener
	ret = listen(sockfd,8);
	if(ret < 0){
		perror("listen:");
		goto err;
	}
	printf("listen...............\n");
	//4.connect
	int connfd = accept(sockfd, NULL, NULL);
	if(connfd < 0){
		perror("accept:");
		goto err;
	}

	//5.read msg
	char buf[1024];
	while(1){
		memset(buf,0,sizeof(buf));
		ret = read(connfd,buf,sizeof(buf));
		if(ret < 0){
			perror("read:");
			break;
		}else if(ret == 0){
			printf("write close!\n");
			break;
		}else{
			printf("recv:%s\n",buf);
		}
	}
	//6.close socket
	close(sockfd);
	close(connfd);
	
err:
	close(sockfd);
	return -1;


}
