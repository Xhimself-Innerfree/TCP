#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main() {
    ULONG bufferSize = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);

    // ��ȡ��������Ϣ
    DWORD result = GetAdaptersAddresses(
        AF_INET,                 // ֻ��ȡ IPv4 ��ַ
        GAA_FLAG_INCLUDE_PREFIX, // ����ǰ׺��Ϣ
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

    // ��������������
    for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != NULL; pCurr = pCurr->Next) {
        // ����δ���õ��������ͻػ����������� 127.0.0.1��
        if (pCurr->OperStatus != IfOperStatusUp ||
            pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) { // �ؼ��޸ģ����˻ػ��ӿ�
            continue;
        }

        // ���� IPv4 ��ַ
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
#pragma comment(lib,"Iphlpapi.lib") //��Ҫ���Iphlpapi.lib��
//#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    //PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    //�õ��ṹ���С,����GetAdaptersInfo����
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    //��¼��������
    DWORD netCardNum = 0;
    GetNumberOfInterfaces(&netCardNum);
    cout << "����������" << netCardNum << endl; netCardNum = 0;
    //��¼ÿ�������ϵ�IP��ַ����
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //����������ص���ERROR_BUFFER_OVERFLOW
        //��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
        //��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
        //�ͷ�ԭ�����ڴ�ռ�
        delete pIpAdapterInfo;
        //���������ڴ�ռ������洢����������Ϣ
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        //�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        //���������Ϣ
        //�����ж�����,���ͨ��ѭ��ȥ�ж�
        while (pIpAdapterInfo)
        {
            cout << "������ţ�" << ++netCardNum << "\t" << pIpAdapterInfo->Index << endl;
            cout << "�������ƣ�" << pIpAdapterInfo->AdapterName << endl;
            cout << "����������" << pIpAdapterInfo->Description << endl;
            cout << "�������ͣ�";
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
            cout << "����MAC��ַ��";
            for (DWORD i = 0; i < pIpAdapterInfo->AddressLength; i++)
                if (i < pIpAdapterInfo->AddressLength - 1)
                {
                    printf("%02X-", pIpAdapterInfo->Address[i]);
                }
                else
                {
                    printf("%02X\n", pIpAdapterInfo->Address[i]);
                }
            cout << "����IP��ַ���£�" << endl;
            IPnumPerNetCard = 0;
            //���������ж�IP,���ͨ��ѭ��ȥ�ж�
            IP_ADDR_STRING* pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                cout << "�������ϵ�IP������" << ++IPnumPerNetCard << endl;
                cout << "IP ��ַ��" << pIpAddrString->IpAddress.String << endl;
                cout << "�������룺" << pIpAddrString->IpMask.String << endl;
                cout << "���ص�ַ��" << pIpAdapterInfo->GatewayList.IpAddress.String << endl;
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo = pIpAdapterInfo->Next;
            cout << "--------------------------------------------------------------------" << endl;
        }

    }
    //�ͷ��ڴ�ռ�
    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }
    return 0;
}
*/