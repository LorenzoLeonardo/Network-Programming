#include "pch.h"
#include "CICMP.h"

CICMP::CICMP()
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (iResult != 0)
        throw iResult;

    iResult = InitializeLocalIPAndHostname();
    if (iResult !=0)
        throw iResult;
}
CICMP::~CICMP()
{
    WSACleanup();
}
int CICMP::InitializeLocalIPAndHostname()
{
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult = 0;
    char hostname[NI_MAXHOST];
    char ipAddress[INET_ADDRSTRLEN];

    memset(hostname, 0, sizeof(hostname));
    memset(ipAddress, 0, sizeof(ipAddress));
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_flags = AI_ALL;

    iResult = getaddrinfo("localhost", NULL, &hints, &result);
    if (iResult != 0)
        return iResult;
    
    iResult = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
    if (iResult != 0)
        return iResult;
    m_HostName = hostname;
    iResult = getaddrinfo(hostname, NULL, &hints, &result);
    if (iResult != 0)
        return iResult;

    inet_ntop(AF_INET, (const void*)(result->ai_addr->sa_data+2), ipAddress, sizeof(ipAddress));
    m_HostIP = ipAddress;
    
    return iResult;
}
string CICMP::GetHostName(string ipAddress)
{
    int iResult = 0;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_flags = AI_ALL;

    iResult = getaddrinfo(ipAddress.c_str(), NULL, &hints, &result);
    if (iResult != 0)
       return "";

    memset(hostname, 0, sizeof(hostname));
    iResult = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NAMEREQD);
    if (iResult != 0)
        return "";

    string sRet = hostname;
    return sRet;
}
bool CICMP::Ping(HANDLE hIcmpFile,string sSrc, string sDest, IPAddr &dest, int TTL)
{
    IP_OPTION_INFORMATION icmpOptions=
    {
         TTL,         // Time To Live
         0,           // Type Of Service
         IP_FLAG_DF,  // IP header flags
         0            // Size of options data
    };
    
    IPAddr src;
    bool bRet = false;

    DWORD dwReplySize = sizeof(ICMP_ECHO_REPLY) + SIZEOF_ICMP_ERROR + SIZEOF_IO_STATUS_BLOCK;
   
    ICMP_ECHO_REPLY *icmpReply;
    char sData[32] ="Data Send";

    PVOID pDataReply = NULL;

    pDataReply = malloc (dwReplySize);
    
    memset(pDataReply, 0, dwReplySize);
    inet_pton(AF_INET, sSrc.c_str(), &src);
    inet_pton(AF_INET, sDest.c_str(), &dest);

    int iReplies = IcmpSendEcho2Ex(hIcmpFile, NULL, NULL,NULL, src, dest, (VOID*)sData, (short)strlen(sData), &icmpOptions, pDataReply, dwReplySize, TTL);
    
    icmpReply = (ICMP_ECHO_REPLY*)pDataReply;
    if (iReplies != 0)
    {
        switch (icmpReply->Status)
        {
            case IP_SUCCESS:
                bRet = true;
                break;
            case IP_DEST_HOST_UNREACHABLE:
                bRet = false;
                break;
            case IP_DEST_NET_UNREACHABLE:
                bRet = false;
                break;
            case IP_REQ_TIMED_OUT:
                bRet = false;
                break;
            default:
                bRet = false;
                break;
        }
    }
    free(pDataReply);
    return bRet;
}
bool CICMP::CheckDevice(string ipAddress, string& hostname, string& sMacAddress)
{
    HANDLE hIcmpFile;
    //struct hostent* remoteHost;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    constexpr int nSendSize = 2;
    char SendData[nSendSize];
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;
    bool bRet = false;
    struct in_addr ReplyAddr;
    int iResult = 0;
    sMacAddress = "";
    // Validate the parameters
    hostname = GetHostName(ipAddress);

    memset(SendData, 0, sizeof(SendData));
    SendData[0] = 1;

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE)
          return false;

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + nSendSize + 8;
    ReplyBuffer = (VOID*)malloc(ReplySize);
    if (ReplyBuffer == NULL)
    {
        IcmpCloseHandle(hIcmpFile);
        return false;
    }
    
    if(inet_pton(AF_INET, ipAddress.c_str(), &ipaddr)!=1)
    {
        free(ReplyBuffer);
        IcmpCloseHandle(hIcmpFile);
        return false;
    }
    IPAddr dest;
    //dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, (LPVOID)SendData, (WORD)strlen(SendData), NULL, ReplyBuffer, ReplySize, 500);
    if (Ping(hIcmpFile, m_HostIP, ipAddress, dest, TIME_TO_LIVE))
    {
        ULONG MacAddr[2];
        ULONG PhysAddrLen = 6;
        IPAddr ipSource;
        LPBYTE bPhysAddr;
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
  
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        inet_pton(AF_INET, m_HostIP.c_str(), &ipSource);
        
        dwRetVal = SendARP(dest, ipSource, MacAddr, &PhysAddrLen);
        if (dwRetVal == NO_ERROR)
        {
            bPhysAddr = (BYTE*)&MacAddr;
            if (PhysAddrLen)
            {
                char szMac[32];
                memset(szMac, 0, sizeof(szMac));
                for (int i = 0; i < 6; i++)
                {
                    if (i < 5)
                    {
                        memset(szMac, 0, sizeof(szMac));
                        sprintf_s(szMac, sizeof(szMac), "%02X-", bPhysAddr[i]);
                        sMacAddress += szMac;
                    }
                    else
                    {
                        memset(szMac, 0, sizeof(szMac));
                        sprintf_s(szMac, sizeof(szMac), "%02X", bPhysAddr[i]);
                        sMacAddress += szMac;
                    }
                }
            }
            bRet = true;
        }
        else
        {
            bRet = false;
        }
    }
    else
        bRet = false;
    
    free(ReplyBuffer);
    IcmpCloseHandle(hIcmpFile);
    return bRet;
}