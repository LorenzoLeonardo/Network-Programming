#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"
#include "CLocalAreaListener.h"
#include "CSNMP.h"
#include "CPacketListener.h"

CCheckOpenPorts* g_pOpenPorts = NULL;
CLocalAreaListener* g_pLocalAreaListener = NULL;
CPacketListener* g_pPacketListener = NULL;

CSNMP*   g_SNMP = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
        if(g_pOpenPorts != NULL)
        {
            delete g_pOpenPorts;
            g_pOpenPorts = NULL;
        }
        if (g_pLocalAreaListener != NULL)
        {
            delete g_pLocalAreaListener;
            g_pLocalAreaListener = NULL;
        }
        if (g_pPacketListener != NULL)
        {
            delete g_pPacketListener;
            g_pPacketListener = NULL;
        }
        if (g_SNMP != NULL)
        {
            delete g_SNMP;
            g_SNMP = NULL;
        }
        break;
    }
    return TRUE;
}

HANDLE ENZTCPLIBRARY_API OpenServer(const char* sport, FuncNewConnection pfnPtr)
{
    CTCPListener* Ptr = new CTCPListener(sport, pfnPtr);
    return (HANDLE)Ptr;
}

void ENZTCPLIBRARY_API RunServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Run();
}
void ENZTCPLIBRARY_API CloseClientConnection(HANDLE hHandle)
{
    CSocket* clientSocket = (CSocket*)hHandle;

    if (clientSocket != NULL)
        delete clientSocket;
}
void ENZTCPLIBRARY_API CloseServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    if (listener != NULL)
    {
        listener->Stop();
        delete listener;
    }
}


HANDLE ENZTCPLIBRARY_API ConnectToServer(const char* ipAddress, const char* portNum, int* pnlastError)
{
    try
    {
        CSocketClient* pSocket = new CSocketClient(ipAddress, portNum);
        int nLastError = 0;

        if (pSocket->ConnectToServer(&nLastError))
            return (HANDLE)pSocket;
        else
        {
            pSocket->DisconnectFromServer();
            delete pSocket;
            pSocket = NULL;
            return (HANDLE)SOCKET_ERROR;
        }
    }
    catch (int nError)
    {
         return (HANDLE)SOCKET_ERROR;
    }
}

void ENZTCPLIBRARY_API DisconnectFromServer(HANDLE hHandle)
{
    if (hHandle != NULL && hHandle != (HANDLE)SOCKET_ERROR)
    {
        CSocketClient* pSocket = (CSocketClient*)hHandle;
        delete pSocket;
        pSocket = NULL;
    }
}

void ENZTCPLIBRARY_API EnumOpenPorts(char* ipAddress, int nNumPorts, FuncFindOpenPort pfnPtr)
{
    if(g_pOpenPorts != NULL)
    {
        delete g_pOpenPorts;
        g_pOpenPorts = NULL;
    }
    string sAddress(ipAddress);

    g_pOpenPorts = new CCheckOpenPorts(sAddress, nNumPorts, pfnPtr);
    g_pOpenPorts->StartSearchingOpenPorts();
    return;
}

void ENZTCPLIBRARY_API StopSearchingOpenPorts()
{
    if (g_pOpenPorts != NULL)
        g_pOpenPorts->StopSearchingOpenPorts();
    return;
}

bool ENZTCPLIBRARY_API IsPortOpen(char* ipAddress, int nNumPorts, int *pnlastError)
{
   string sAddress(ipAddress);

   try
   {
       CSocketClient port;
       return port.ConnectToServer(sAddress, to_string(nNumPorts), pnlastError);
   }
   catch (int nError)
   {
       return nError == 0;
   }
}

void ENZTCPLIBRARY_API StartLocalAreaListening(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int nPollingTimeMS)
{
    if (g_pLocalAreaListener != NULL)
    {
        delete g_pLocalAreaListener;
        g_pLocalAreaListener = NULL;
    }
    g_pLocalAreaListener = new CLocalAreaListener(ipAddress, fnpPtr, nPollingTimeMS);
    g_pLocalAreaListener->Start();
}
void ENZTCPLIBRARY_API StopLocalAreaListening()
{
    if (g_pLocalAreaListener != NULL)
    {
        g_pLocalAreaListener->Stop();
    }
}

bool ENZTCPLIBRARY_API StartSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD& dwLastError)
{
    if (g_SNMP != NULL)
    {
        delete g_SNMP;
        g_SNMP = NULL;
    }
    g_SNMP = new CSNMP();

    return g_SNMP->InitSNMP(szAgentIPAddress, szCommunity, nVersion, dwLastError);
}
smiVALUE ENZTCPLIBRARY_API SNMPGet(const char* szOID, DWORD& dwLastError)
{
    smiVALUE value;
    memset(&value, 0, sizeof(value));

    if (g_SNMP == NULL)
    {
        return value;
    }
    return g_SNMP->Get(szOID, dwLastError);
}
void ENZTCPLIBRARY_API EndSNMP()
{
    g_SNMP->EndSNMP();
    if (g_SNMP != NULL)
    {
        delete g_SNMP;
        g_SNMP = NULL;
    }
}

bool ENZTCPLIBRARY_API GetDefaultGateway(char* szDefaultIPAddress)
{
    WSADATA wsaData;
    IPAddr* pDefaultGateway = 0;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    ULONG PhysAddrLen = 6;
    addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult = 0;
    
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
        return false;
 
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

        sTemp= sTemp.substr(0,sTemp.rfind('.', sTemp.length())+1);
        sTemp += "1";
        
        memcpy_s(szDefaultIPAddress, sizeof(char)*sTemp.length(), sTemp.c_str(), sizeof(char) * sTemp.length());
        WSACleanup();
        return true;
    }
    else
    {
        WSACleanup();
        return false;
    }
}

bool ENZTCPLIBRARY_API StartPacketListener(FNCallbackPacketListener fnpPtr)
{
    if (g_pPacketListener == NULL)
    {
        g_pPacketListener = new CPacketListener(fnpPtr);
        if (g_pPacketListener == NULL)
            return false;
    }
    if (g_pPacketListener->IsStopped())
        return g_pPacketListener->StartListening();
    else
        return true;
}
void ENZTCPLIBRARY_API StopPacketListener()
{
    if (g_pPacketListener != NULL)
        g_pPacketListener->StopListening();
}