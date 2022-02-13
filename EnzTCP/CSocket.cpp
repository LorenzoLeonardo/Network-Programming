#include "pch.h"
#include "CSocket.h"

using namespace std;

CSocket::CSocket()
{
    m_socket = NULL;
    memset(&m_addr, 0, sizeof(m_addr));
    m_hostname = "";
    m_ipAddress = "";


}

CSocket::CSocket(SOCKET s)
{
    m_socket = s;
    memset(&m_addr, 0, sizeof(m_addr));
    m_hostname = "";
    m_ipAddress = "";
}
CSocket::~CSocket()
{

}
SOCKET  CSocket::GetSocket()
{
    return m_socket;
}

void  CSocket::SetClientAddr(struct sockaddr addr)
{
    m_addr = addr;
    SetIP();
    SetHostname();
}
const char* CSocket::GetIP()
{
    return m_ipAddress.c_str();
}
const char* CSocket::GetHostName()
{
    return m_hostname.c_str();
}
void CSocket::SetHostname()
{
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    ULONG PhysAddrLen = 6;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult = 0;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_flags = AI_ALL;

    iResult = getaddrinfo(m_ipAddress.c_str(), NULL, &hints, &result);
    if (iResult == 0)
    {
        memset(hostname, 0, sizeof(hostname));
        int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
        freeaddrinfo(result);
        iResult = getaddrinfo(hostname, NULL, &hints, &result);
        m_hostname = hostname;
    }
}
bool CSocket::GetDefaultGateway(char* szDefaultIPAddress)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    IPAddr* pDefaultGateway = 0;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    ULONG PhysAddrLen = 6;
    addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult = 0;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_flags = AI_ALL;

    iResult = getaddrinfo("localhost", NULL, &hints, &result);
    if (iResult == 0)
    {
        memset(hostname, 0, sizeof(hostname));
        int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
        freeaddrinfo(result);
        iResult = getaddrinfo(hostname, NULL, &hints, &result);
        pDefaultGateway = (IPAddr*)(result->ai_addr->sa_data + 2);
        char szIPAddress[32];
        memset(szIPAddress, 0, sizeof(szIPAddress));
        inet_ntop(AF_INET, (const void*)pDefaultGateway, szIPAddress, sizeof(szIPAddress));
        string sTemp = szIPAddress;

        sTemp = sTemp.substr(0, sTemp.rfind('.', sTemp.length()) + 1);
        sTemp += "1";

        memcpy_s(szDefaultIPAddress, sizeof(char) * sTemp.length(), sTemp.c_str(), sizeof(char) * sTemp.length());
        WSACleanup();
        return true;
    }
    else
    {
        WSACleanup();
        return false;
    }
}

void CSocket::SetIP()
{
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&m_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    char str[INET_ADDRSTRLEN];
    memset(str, 0, sizeof(str));
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

    m_ipAddress = str;
}


const char* CSocket::Receive()
{
    int iResult = 0;
    int nError = 0;

    char recvbuf[MAX_BUFFER_SIZE];

    memset(recvbuf, 0, sizeof(recvbuf));

    iResult = recv(m_socket, recvbuf, sizeof(recvbuf), 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        throw nError;
    }
    dataRecv = recvbuf;
    return dataRecv.c_str();
}
void CSocket::Send(char* sendbuf)
{
    int iResult = 0;
    int nError = 0;

    iResult = send(m_socket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        throw nError;
    }
}