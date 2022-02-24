#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"
#include "CLocalAreaListener.h"
#include "CSNMP.h"
#include "CPacketListener.h"
#include "DebugLog.h"

CCheckOpenPorts* g_pOpenPorts = NULL;
CLocalAreaListener* g_pLocalAreaListener = NULL;
CPacketListener* g_pPacketListener = NULL;
CSNMP*   g_SNMP = NULL;
CICMP* g_pICMP = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DEBUG_LOG("Application Started");
        break;
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
        if (g_pICMP != NULL)
        {
            delete g_pICMP;
            g_pICMP = NULL;
        }
        DEBUG_LOG("Application Ended");
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
        if (pSocket == NULL)
            throw (HANDLE)SOCKET_ERROR;

        if (pSocket->ConnectToServer(&nLastError))
            return (HANDLE)pSocket;
        else
        {
            pSocket->DisconnectFromServer();
            delete pSocket;
            pSocket = NULL;
            throw (HANDLE)SOCKET_ERROR;
        }
    }
    catch (HANDLE nError)
    {
         return nError;
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
    if (g_pOpenPorts == NULL)
        return;
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

bool ENZTCPLIBRARY_API StartLocalAreaListening(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int nPollingTimeMS)
{
    try
    {
        if (g_pLocalAreaListener != NULL)
        {
            delete g_pLocalAreaListener;
            g_pLocalAreaListener = NULL;
        }
        g_pLocalAreaListener = new CLocalAreaListener(ipAddress, fnpPtr, nPollingTimeMS);
        if (g_pLocalAreaListener == NULL)
            return false;
        g_pLocalAreaListener->Start();
    }
    catch (int nError)
    {
        return nError == 0;
    }
    return true;
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
    if (g_SNMP == NULL)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
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
    if (g_SNMP != NULL)
    {
        g_SNMP->EndSNMP();
        delete g_SNMP;
        g_SNMP = NULL;
    }
}

bool ENZTCPLIBRARY_API GetDefaultGateway(char* szDefaultIPAddress)
{
    CSocket sock;
    return sock.GetDefaultGateway(szDefaultIPAddress);
}

bool ENZTCPLIBRARY_API StartPacketListener(FNCallbackPacketListener fnpPtr)
{
    if (g_pPacketListener == NULL)
    {
        try
        {
            g_pPacketListener = new CPacketListener(fnpPtr);
            if (g_pPacketListener == NULL)
                return false;
        }
        catch (int nError)
        {
            return !(nError==INVALID_SOCKET);
        }
    }
    if (g_pPacketListener->IsStopped())
        return g_pPacketListener->StartListening();
    else
        return true;
}
void ENZTCPLIBRARY_API StopPacketListener()
{
    if (g_pPacketListener != NULL)
    {
        g_pPacketListener->StopListening();
    }
}

bool ENZTCPLIBRARY_API GetNetworkDeviceStatus(const char* ipAddress, char* hostname, int nSizeHostName, char* macAddress, int nSizeMacAddress, DWORD* pError)
{
    string shostName, smacAddress;
    bool bRet = false;

    if (g_pICMP == NULL)
    {
        try
        {
            g_pICMP = new CICMP();
            if (g_pICMP == NULL)
            {
                *pError = ERROR_NOT_ENOUGH_MEMORY;
                return false;
            }

            bRet = g_pICMP->CheckDevice(ipAddress, shostName, smacAddress, pError);
            if (nSizeHostName < shostName.length())
            {
                *pError = ERROR_INSUFFICIENT_BUFFER;
                bRet = false;
            }
            else if (nSizeMacAddress < smacAddress.length())
            {
                *pError = ERROR_INSUFFICIENT_BUFFER;
                bRet = false;
            }
            memcpy(macAddress, smacAddress.c_str(),smacAddress.length());
            memcpy(hostname, shostName.c_str(), shostName.length());
            return bRet;
        }
        catch (int nError)
        {
            *pError = nError;
            return bRet;
        }
    }
    else
    {
        bRet = g_pICMP->CheckDevice(ipAddress, shostName, smacAddress, pError);
        if (nSizeHostName < shostName.length())
        {
            *pError = ERROR_INSUFFICIENT_BUFFER;
            bRet = false;
        }
        else if (nSizeMacAddress < smacAddress.length())
        {
            *pError = ERROR_INSUFFICIENT_BUFFER;
            bRet = false;
        }
        memcpy(macAddress, smacAddress.c_str(), smacAddress.length());
        memcpy(hostname, shostName.c_str(), shostName.length());
        return bRet;
    }
}