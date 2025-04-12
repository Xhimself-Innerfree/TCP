#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main() {
    ULONG bufferSize = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);

    // 获取适配器信息
    DWORD result = GetAdaptersAddresses(
        AF_INET,                 // 只获取 IPv4 地址
        GAA_FLAG_INCLUDE_PREFIX, // 包含前缀信息
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

    // 遍历所有适配器
    for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != NULL; pCurr = pCurr->Next) {
        // 跳过未启用的适配器和回环适配器（如 127.0.0.1）
        if (pCurr->OperStatus != IfOperStatusUp ||
            pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) { // 关键修改：过滤回环接口
            continue;
        }

        // 遍历 IPv4 地址
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
    return 0;
}
/*
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <iostream>
using namespace std;
#pragma comment(lib,"Iphlpapi.lib") //需要添加Iphlpapi.lib库
//#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    //记录网卡数量
    DWORD netCardNum = 0;
    GetNumberOfInterfaces(&netCardNum);
    cout << "网卡数量：" << netCardNum << endl; netCardNum = 0;
    //记录每张网卡上的IP地址数量
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量
        //释放原来的内存空间
        delete pIpAdapterInfo;
        //重新申请内存空间用来存储所有网卡信息
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        //输出网卡信息
        //可能有多网卡,因此通过循环去判断
        while (pIpAdapterInfo)
        {
            cout << "网卡序号：" << ++netCardNum << "\t" << pIpAdapterInfo->Index << endl;
            cout << "网卡名称：" << pIpAdapterInfo->AdapterName << endl;
            cout << "网卡描述：" << pIpAdapterInfo->Description << endl;
            cout << "网卡类型：";
            switch (pIpAdapterInfo->Type)
            {
            case MIB_IF_TYPE_OTHER:        cout << "OTHER" << endl; break;
            case MIB_IF_TYPE_ETHERNET:    cout << "ETHERNET" << endl; break;
            case MIB_IF_TYPE_TOKENRING:    cout << "TOKENRING" << endl; break;
            case MIB_IF_TYPE_FDDI:        cout << "FDDI" << endl; break;
            case MIB_IF_TYPE_PPP:        cout << "PPP" << endl; break;
            case MIB_IF_TYPE_LOOPBACK:    cout << "LOOPBACK" << endl; break;
            case MIB_IF_TYPE_SLIP:        cout << "SLIP" << endl; break;
            default:                    cout << "" << endl; break;
            }
            cout << "网卡MAC地址：";
            for (DWORD i = 0; i < pIpAdapterInfo->AddressLength; i++)
                if (i < pIpAdapterInfo->AddressLength - 1)
                {
                    printf("%02X-", pIpAdapterInfo->Address[i]);
                }
                else
                {
                    printf("%02X\n", pIpAdapterInfo->Address[i]);
                }
            cout << "网卡IP地址如下：" << endl;
            IPnumPerNetCard = 0;
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING* pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                cout << "该网卡上的IP数量：" << ++IPnumPerNetCard << endl;
                cout << "IP 地址：" << pIpAddrString->IpAddress.String << endl;
                cout << "子网掩码：" << pIpAddrString->IpMask.String << endl;
                cout << "网关地址：" << pIpAdapterInfo->GatewayList.IpAddress.String << endl;
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo = pIpAdapterInfo->Next;
            cout << "--------------------------------------------------------------------" << endl;
        }

    }
    //释放内存空间
    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }
    return 0;
}
*/