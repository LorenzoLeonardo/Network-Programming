#include "pch.h"
#include "CICMP.h"
#include "DebugLog.h"

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
bool CICMP::Ping(HANDLE hIcmpFile,string sSrc, string sDest, IPAddr &dest, int nTTL)
{
    IP_OPTION_INFORMATION icmpOptions=
    {
         nTTL,         // Time To Live
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
    if (pDataReply != NULL)
    {
        memset(pDataReply, 0, dwReplySize);
        inet_pton(AF_INET, sSrc.c_str(), &src);
        inet_pton(AF_INET, sDest.c_str(), &dest);

        int iReplies = IcmpSendEcho2Ex(hIcmpFile, NULL, NULL, NULL, src, dest, (VOID*)sData, (short)strlen(sData), &icmpOptions, pDataReply, dwReplySize, nTTL);

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
    }
    return bRet;
}
bool CICMP::CheckDeviceEx(string ipAddress, string& hostname, string& sMacAddress)
{
    SOCKET sockRaw;
    const char* lpdest = ipAddress.c_str();
    char* icmp_data = NULL, * recvbuf = NULL;
    struct sockaddr_in dest, from;
    int iResult = 0, timeoutsend = PING_TIMEOUT, timeoutrecv = PING_TIMEOUT, fromlen = sizeof(from), datasize = 0;
    struct hostent* hp = NULL;
    USHORT usSequenceNumber = atoi(ipAddress.substr(ipAddress.rfind('.', ipAddress.size()) + 1, ipAddress.size()).c_str());
    bool bRet = false;

    hostname = GetHostName(ipAddress);

    icmp_data = (char*)malloc(MAX_PACKET);
    if (!icmp_data)
    {
        DEBUG_LOG("CICMP::CheckDeviceEx() icmp_data -> Out of memory.");
        return bRet;
    }
    memset((void*)icmp_data, 0, MAX_PACKET);

    recvbuf = (char*)malloc(MAX_PACKET);
    if (!recvbuf)
    {
        DEBUG_LOG("CICMP::CheckDeviceEx() recvbuf -> Out of memory.");
        return bRet;
    }
    memset((void*)recvbuf, 0, MAX_PACKET);

    sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockRaw == INVALID_SOCKET)
    {
        DEBUG_LOG("CICMP::CheckDeviceEx(): socket -> Invalid Socket.");
        goto CLEANPUP;
    }

   // iResult = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeoutsend, sizeof(timeoutsend));
   // if (iResult == SOCKET_ERROR)
   //     goto CLEANPUP;

   // iResult = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeoutrecv, sizeof(timeoutrecv));
   // if (iResult == SOCKET_ERROR)
    //    goto CLEANPUP;

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    if ((dest.sin_addr.s_addr = inet_addr(lpdest)) == INADDR_NONE)
    {
        if ((hp = gethostbyname(lpdest)) != NULL)
        {
            memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
            dest.sin_family = hp->h_addrtype;
        }
    }
    datasize += sizeof(ICMP_HDR);

    FillICMPData(icmp_data, datasize);

    ((ICMP_HDR*)icmp_data)->byType = ICMP_ECHO;
    ((ICMP_HDR*)icmp_data)->ulTimeStamp = GetTickCount();
    ((ICMP_HDR*)icmp_data)->usSeq = usSequenceNumber;
    ((ICMP_HDR*)icmp_data)->usChecksum = CheckSum((USHORT*)icmp_data, datasize);

    iResult = sendto(sockRaw, icmp_data, datasize, 0,
        (struct sockaddr*)&dest, sizeof(dest));
    if (iResult == SOCKET_ERROR)
    {
        DEBUG_LOG("CICMP::CheckDeviceEx(): sendto -> Socket Error." + to_string(WSAGetLastError()));
        goto CLEANPUP;
    }

    iResult = recvfrom(sockRaw, recvbuf, MAX_PACKET, 0, (struct sockaddr*)&from, &fromlen);
    if (iResult == SOCKET_ERROR)
    {
        DEBUG_LOG("CICMP::CheckDeviceEx(): recvfrom -> Socket Error. " + to_string(WSAGetLastError()));
        goto CLEANPUP;
    }
    else
    {
        if (DecodeICMPHeader(usSequenceNumber, recvbuf, iResult, &from))
        {
            ULONG MacAddr[2];
            ULONG PhysAddrLen = 6;
            IPAddr ipSource;
            LPBYTE bPhysAddr;
            DWORD dwRetVal;

            inet_pton(AF_INET, m_HostIP.c_str(), &ipSource);

            dwRetVal = SendARP(from.sin_addr.S_un.S_addr, ipSource, MacAddr, &PhysAddrLen);
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
                char szIP[32];
                inet_ntop(AF_INET, &from.sin_addr.S_un.S_addr, szIP, sizeof(szIP));
                string sIP(szIP);
                DEBUG_LOG("CICMP::CheckDeviceEx(): SendARP -> Error. " + sIP);
                bRet = false;
            }
        }
        else
        {
            char szIP[32];
            inet_ntop(AF_INET, &from.sin_addr.S_un.S_addr, szIP, sizeof(szIP));
            string sIP(szIP);
            DEBUG_LOG("CICMP::CheckDeviceEx(): DecodeICMPHeader() - false " + sIP);
            bRet = false;
        }
    }

CLEANPUP:
    if (sockRaw != INVALID_SOCKET)
        closesocket(sockRaw);
    free(recvbuf);
    free(icmp_data);
    recvbuf = NULL;
    icmp_data = NULL;

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
    if (Ping(hIcmpFile, m_HostIP, ipAddress, dest, PING_TIMEOUT))
    {
        ULONG MacAddr[2];
        ULONG PhysAddrLen = 6;
        IPAddr ipSource;
        LPBYTE bPhysAddr;
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
  
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        inet_pton(AF_INET, m_HostIP.c_str(), &ipSource);
        
        dwRetVal = SendARP(ipaddr, ipSource, MacAddr, &PhysAddrLen);
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
            char szIP[32];
            inet_ntop(AF_INET, &ipaddr, szIP, sizeof(szIP));
            string sIP(szIP);

            DEBUG_LOG("CICMP::CheckDevice(): SendARP("+ sIP +") Error:"+to_string(dwRetVal));
            bRet = false;
        }
    }
    else
        bRet = false;
    
    free(ReplyBuffer);
    IcmpCloseHandle(hIcmpFile);
    return bRet;
}
void CICMP::FillICMPData(char* icmp_data, int datasize)
{
    ICMP_HDR* icmp_hdr = NULL;
    char* datapart = NULL;

    icmp_hdr = (ICMP_HDR*)icmp_data;
    icmp_hdr->byType = ICMP_ECHO;
    icmp_hdr->byCode = 0;
    icmp_hdr->usID = (USHORT)GetCurrentProcessId();
    icmp_hdr->usChecksum = 0;
    icmp_hdr->usSeq = 0;

    datapart = icmp_data + sizeof(ICMP_HDR);

    memset(datapart, 'E', datasize - sizeof(ICMP_HDR));
}

USHORT CICMP::CheckSum(USHORT* buffer, int size)
{
    unsigned long cksum = 0;

    while (size > 1)
    {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }
    if (size)
    {
        cksum += *(UCHAR*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (USHORT)(~cksum);
}

bool CICMP::DecodeICMPHeader(USHORT usSeq, char* buf, int bytes, struct sockaddr_in* from)
{
    IPV4_HDR* iphdr = NULL;
    ICMP_HDR* icmphdr = NULL;
    unsigned short  iphdrlen;

    iphdr = (IPV4_HDR*)buf;
    iphdrlen = iphdr->ucIPHeaderLen * 4;


    if (bytes < iphdrlen + ICMP_MIN)
    {
        DEBUG_LOG("CICMP::DecodeICMPHeader(): bytes < iphdrlen + ICMP_MIN.");
        return true;
    }

    icmphdr = (ICMP_HDR*)(buf + iphdrlen);

    if (icmphdr->byType != ICMP_ECHOREPLY)
    {
        return false;
    }


    if (icmphdr->usID != (USHORT)GetCurrentProcessId())
    {
        DEBUG_LOG("CICMP::DecodeICMPHeader(): icmphdr->usID != (USHORT)GetCurrentProcessId().");
        return false;
    }


    if (icmphdr->usSeq == usSeq)
    {
        return true;
    }

    return false;
}
