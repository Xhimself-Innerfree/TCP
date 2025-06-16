#include <stdio.h>
#include <string.h>
#include <Ws2tcpip.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define PORT  6060
#define IP "ff02::2"
#define BUF_LEN 256

int main(int argc, char* argv[])
{
    WSADATA     wsaData;
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD(1, 1);
    WSAStartup(wVersionRequested, &wsaData);

    struct sockaddr_in6 addr = { 0 };
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, IP, &addr.sin6_addr);

    int sock;
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    char msgbuf[BUF_LEN];
    printf("This is FNLab Sender, Please Enter message to send (type 'exit' to quit):\n");
    while (1)
    {
        // Read user input from stdin
        if (fgets(msgbuf, BUF_LEN, stdin) == NULL)
            break;
        // Remove trailing newline
        size_t len = strlen(msgbuf);
        if (len > 0 && msgbuf[len - 1] == '\n')
            msgbuf[len - 1] = '\0';
        if (strcmp(msgbuf, "exit") == 0)
            break;

        int sent = sendto(sock, msgbuf, (int)strlen(msgbuf), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (sent < 0)
        {
            perror("Send failed");
            break;
        }
        printf("Sent: %s\n", msgbuf);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
