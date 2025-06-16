#include <stdio.h>
#include <Ws2tcpip.h>
#include <winsock2.h>  
#pragma comment(lib,"ws2_32.lib")

#define PORT  6060
#define IP "ff02::2"    
#define BUF_LEN 256  

int main(int argc, char* argv[])
{
    WSADATA     wsaData;
    WORD wVersionRequested;                   // Winsock version
    wVersionRequested = MAKEWORD(1, 1);       // Set version to 1.1
    // Initialize Windows Sockets API (Winsock)
    WSAStartup(wVersionRequested, &wsaData);

    // Set up the local address structure for IPv6 UDP
    struct sockaddr_in6 addr = { AF_INET6, htons(PORT) };

    // Create a UDP socket using IPv6
    int l_nServer;
    if ((l_nServer = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        // Socket creation failed
        perror("Socket creation failed");
        return -1;
    }
    // Bind the socket to the local address and port
    bind(l_nServer, (struct sockaddr*)&addr, sizeof(addr));

    // Set up the multicast group structure for IPv6
    struct ipv6_mreq group;
    // Use the default multicast interface (interface index 0)
    group.ipv6mr_interface = 0;
    // Set the multicast group address
    inet_pton(AF_INET6, IP, &group.ipv6mr_multiaddr);
    // Join the multicast group on the specified interface
    // This allows the socket to receive multicast packets sent to the group
    setsockopt(l_nServer, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&group, sizeof(group));

    int l_naddLen = sizeof(addr);
    int l_nReadLen = 0;
    char msgbuf[BUF_LEN];
    printf("FNLab RCVer Waiting to receive messages...\n");
    while (1)
    {
        // Receive a message from any sender
        l_nReadLen = recvfrom(l_nServer, msgbuf, BUF_LEN, 0, (struct sockaddr*)&addr, &l_naddLen);
        if (l_nReadLen < 0)
        {
            // Receiving failed
            perror("Receive failed");
            exit(1);
        }
        msgbuf[l_nReadLen] = '\0'; // Null-terminate the received string
        printf("%s\n", msgbuf);

        // --- Removed response sending code ---
        // strcpy_s(msgbuf, "world");
        // int l_nLen = sendto(l_nServer, msgbuf, strlen(msgbuf), 0, (struct sockaddr*)&addr, sizeof(addr));
        // if (l_nLen < 0)
        // {
        //     perror("Send failed");
        //     exit(1);
        // }
        // printf("Send %s\n", msgbuf);
    }

    return 0;
}
