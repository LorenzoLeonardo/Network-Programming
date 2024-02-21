#include "pch.h"
#include "EnzNetworkDLL.h"
#include "CTCPListener.h"
#include "COpenPortListener.h"
#include "CSocketClient.h"
#include "CLocalAreaListener.h"
#include "CSNMP.h"
#include "CPacketListener.h"
#include "DebugLog.h"
#include <memory>

static std::unique_ptr<COpenPortListener> g_pOpenPorts = nullptr;
static std::unique_ptr<CLocalAreaListener> g_pLocalAreaListener = nullptr;
static std::unique_ptr<CPacketListener> g_pPacketListener = nullptr;
static std::unique_ptr<CSNMP> g_SNMP = nullptr;
static std::unique_ptr<CICMP> g_pICMP = nullptr;
static char g_szAdapterName[MAX_ADAPTER_NAME_LENGTH + 4] = {};
static char g_szAdapterIP[32] = {};
static ULONG g_ulAdapterIP = 0;

extern "C" BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DEBUG_LOG("EnzTCP Library is Loaded.");
		g_ulAdapterIP = 0;
		break;
	case DLL_PROCESS_DETACH:
		_CrtDumpMemoryLeaks();
		DEBUG_LOG("EnzTCP Library is Unloaded.");
		break;
	}
	return TRUE;
}

HANDLE ENZTCPLIBRARY_API OpenServer(const char* sport, FNCallbackNewConnection pfnPtr) {
	std::shared_ptr<CTCPListener>
		Ptr = std::make_shared<CTCPListener>(sport, pfnPtr);

	return (HANDLE)Ptr.get();
}

void ENZTCPLIBRARY_API RunServer(HANDLE hHandle) {
	CTCPListener* listener = (CTCPListener*)hHandle;

	listener->Run();
}
void ENZTCPLIBRARY_API CloseClientConnection(HANDLE hHandle) {
	CSocket* clientSocket = (CSocket*)hHandle;

	if (clientSocket != nullptr)
		delete clientSocket;
}
void ENZTCPLIBRARY_API CloseServer(HANDLE hHandle) {
	CTCPListener* listener = (CTCPListener*)hHandle;

	if (listener != nullptr) {
		listener->Stop();
		delete listener;
	}
}


HANDLE ENZTCPLIBRARY_API ConnectToServer(const char* ipAddress, const char* portNum, int* pnlastError) {
	std::shared_ptr<CSocketClient> pSocket = nullptr;

	try {
		pSocket = std::make_shared<CSocketClient>(ipAddress, portNum);
		int nLastError = 0;
		if (pSocket == nullptr)
			throw (HANDLE)SOCKET_ERROR;

		if (pSocket->ConnectToServer(&nLastError))
			return (HANDLE)pSocket.get();
		else {
			pSocket->DisconnectFromServer();
			throw (HANDLE)SOCKET_ERROR;
		}
	}
	catch (HANDLE nError) {
		DEBUG_LOG("ConnectToServer(): Exception (" + to_string((ULONG_PTR)nError) + ")");
		return nError;
	}
}

void ENZTCPLIBRARY_API DisconnectFromServer(HANDLE hHandle) {
	if (hHandle != nullptr && hHandle != (HANDLE)SOCKET_ERROR) {
		CSocketClient* pSocket = (CSocketClient*)hHandle;
		delete pSocket;
		pSocket = nullptr;
	}
}

void ENZTCPLIBRARY_API EnumOpenPorts(char* ipAddress, int nNumPorts, FNCallbackFindOpenPort pfnPtr) {
	string sAddress(ipAddress);

	g_pOpenPorts = std::make_unique<COpenPortListener>(sAddress, nNumPorts, pfnPtr);
	if (g_pOpenPorts == nullptr)
		return;
	g_pOpenPorts->StartSearchingOpenPorts();
	return;
}

void ENZTCPLIBRARY_API StopSearchingOpenPorts() {
	if (g_pOpenPorts != nullptr)
		g_pOpenPorts->StopSearchingOpenPorts();
	return;
}

bool ENZTCPLIBRARY_API IsPortOpen(char* ipAddress, int nNumPorts, int* pnlastError) {
	string sAddress(ipAddress);

	try {
		CSocketClient port;
		return port.ConnectToServer(sAddress, to_string(nNumPorts), pnlastError);
	}
	catch (int nError) {
		DEBUG_LOG("IsPortOpen(): Exception (" + to_string(nError) + ")");
		return nError == 0;
	}
}

bool ENZTCPLIBRARY_API StartLocalAreaListening(const char* ipAddress, const char* subNetMask, FNCallbackLocalAreaListener fnpPtr, int nPollingTimeMS) {
	try {
		g_pLocalAreaListener = std::make_unique<CLocalAreaListener>(ipAddress, subNetMask, fnpPtr, nPollingTimeMS);
		if (g_pLocalAreaListener == nullptr)
			return false;
		g_pLocalAreaListener->Start();
	}
	catch (int nError) {
		DEBUG_LOG("StartLocalAreaListening(): Exception (" + to_string(nError) + ")");
		if (g_pLocalAreaListener != nullptr) {
			g_pLocalAreaListener = nullptr;
		}
		return nError == 0;
	}
	return true;
}
void ENZTCPLIBRARY_API StopLocalAreaListening() {
	if (g_pLocalAreaListener != nullptr) {
		g_pLocalAreaListener->Stop();
	}
}

bool ENZTCPLIBRARY_API StartSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD& dwLastError) {
	g_SNMP = std::make_unique<CSNMP>();
	if (g_SNMP == nullptr) {
		dwLastError = ERROR_NOT_ENOUGH_MEMORY;
		DEBUG_LOG("StartSNMP(): Exception (" + to_string(dwLastError) + ")");
		return false;
	}
	return g_SNMP->InitSNMP(szAgentIPAddress, szCommunity, nVersion, dwLastError);
}
smiVALUE ENZTCPLIBRARY_API SNMPGet(const char* szOID, DWORD& dwLastError) {
	smiVALUE value;
	memset(&value, 0, sizeof(value));

	if (g_SNMP == nullptr) {
		return value;
	}
	return g_SNMP->Get(szOID, dwLastError);
}
void ENZTCPLIBRARY_API EndSNMP() {
	if (g_SNMP != nullptr) {
		g_SNMP->EndSNMP();
		g_SNMP = nullptr;
	}
}

bool ENZTCPLIBRARY_API GetDefaultGateway(char* szDefaultIPAddress) {
	CSocket sock;
	return sock.GetDefaultGateway(szDefaultIPAddress);
}

bool ENZTCPLIBRARY_API GetDefaultGatewayEx(const char* szAdapterName, char* szDefaultGateway, int nSize) {
	CSocket sock;
	return sock.GetDefaultGateway(szAdapterName, szDefaultGateway, nSize);
}

bool ENZTCPLIBRARY_API StartPacketListener(FNCallbackPacketListener fnpPtr) {
	if (g_pPacketListener == nullptr) {
		try {
			g_pPacketListener = std::make_unique<CPacketListener>(fnpPtr);
			if (g_pPacketListener == nullptr)
				return false;
		}
		catch (int nError) {
			DEBUG_LOG("StartPacketListener(): Exception (" + to_string(nError) + ")");
			if (g_pPacketListener != nullptr) {
				g_pPacketListener = nullptr;
			}
			return !(nError == INVALID_SOCKET);
		}
	}
	if (g_pPacketListener->IsStopped())
		return g_pPacketListener->StartListening();
	else
		return true;
}
void ENZTCPLIBRARY_API StopPacketListener() {
	if (g_pPacketListener != nullptr) {
		g_pPacketListener->StopListening();
	}
}

bool ENZTCPLIBRARY_API GetNetworkDeviceStatus(const char* ipAddress, char* hostname, int nSizeHostName, char* macAddress, int nSizeMacAddress, DWORD* pError) {
	string shostName, smacAddress;
	bool bRet = false;

	if (g_pICMP == nullptr) {
		try {
			g_pICMP = std::make_unique<CICMP>();
			if (g_pICMP == nullptr) {
				*pError = ERROR_NOT_ENOUGH_MEMORY;
				throw *pError;
			}
			g_pICMP->InitializeLocalIPAndHostname(g_szAdapterIP);
			bRet = g_pICMP->CheckDevice(ipAddress, shostName, smacAddress, pError);
			if (nSizeHostName < shostName.length()) {
				*pError = ERROR_INSUFFICIENT_BUFFER;
				throw *pError;
			}
			else if (nSizeMacAddress < smacAddress.length()) {
				*pError = ERROR_INSUFFICIENT_BUFFER;
				throw *pError;
			}
			memcpy(macAddress, smacAddress.c_str(), smacAddress.length());
			memcpy(hostname, shostName.c_str(), shostName.length());
			return bRet;
		}
		catch (DWORD nError) {
			*pError = nError;
			DEBUG_LOG("GetNetworkDeviceStatus(): Exception (" + to_string(nError) + ")");
			if (g_pICMP) {
				g_pICMP = nullptr;
			}
			return bRet;
		}
	}
	else {
		g_pICMP->InitializeLocalIPAndHostname(g_szAdapterIP);
		bRet = g_pICMP->CheckDevice(ipAddress, shostName, smacAddress, pError);
		if (nSizeHostName < shostName.length()) {
			*pError = ERROR_INSUFFICIENT_BUFFER;
			bRet = false;
		}
		else if (nSizeMacAddress < smacAddress.length()) {
			*pError = ERROR_INSUFFICIENT_BUFFER;
			bRet = false;
		}
		memcpy(macAddress, smacAddress.c_str(), smacAddress.length());
		memcpy(hostname, shostName.c_str(), shostName.length());
		return bRet;
	}
}

bool ENZTCPLIBRARY_API EnumNetworkAdapters(FNCallbackAdapterList pFunc) {
	PIP_ADAPTER_INFO pAdapterInfo = nullptr, pAdapter = nullptr;
	ULONG ulOutBufLen = 0;
	bool bRet = false;

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		pAdapterInfo = (PIP_ADAPTER_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulOutBufLen);
		if (pAdapterInfo) {
			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
				pAdapter = pAdapterInfo;
				while (pAdapter) {
					pFunc((void*)pAdapter);
					pAdapter = pAdapter->Next;
				}
				bRet = true;
			}
			else
				DEBUG_LOG("EnumNetworkAdapters(): GetAdaptersInfo() Exception (" + to_string(GetLastError()) + ")");
		}
		else
			DEBUG_LOG("EnumNetworkAdapters(): GetAdaptersInfo() Exception (" + to_string(ERROR_NOT_ENOUGH_MEMORY) + ")");
	}
	else
		DEBUG_LOG("EnumNetworkAdapters(): GetAdaptersInfo() Exception (" + to_string(GetLastError()) + ")");

	if (pAdapterInfo) {
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = nullptr;
	}
	return bRet;
}

HANDLE ENZTCPLIBRARY_API CreatePacketListenerEx(FNCallbackPacketListenerEx fnpPtr, void* pObject) {
	CPacketListener* pPacketListener = nullptr;
	try {
		pPacketListener = new CPacketListener(fnpPtr, pObject);
		return pPacketListener;
	}
	catch (int nError) {
		DEBUG_LOG("CreatePacketListenerEx(): Exception (" + to_string(nError) + ")");
		if (pPacketListener) {
			delete pPacketListener;
			pPacketListener = nullptr;
		}
		return pPacketListener;
	}
}

bool ENZTCPLIBRARY_API StartPacketListenerEx(HANDLE hHandle) {
	CPacketListener* pPacketListener = (CPacketListener*)hHandle;
	if (pPacketListener)
		return pPacketListener->StartListeningEx(g_ulAdapterIP);
	else
		return false;
}

void ENZTCPLIBRARY_API StopPacketListenerEx(HANDLE hHandle) {
	CPacketListener* pPacketListener = (CPacketListener*)hHandle;
	if (pPacketListener) {
		pPacketListener->StopListeningEx();
	}
}

void ENZTCPLIBRARY_API DeletePacketListenerEx(HANDLE& hHandle) {
	CPacketListener* pPacketListener = (CPacketListener*)hHandle;
	if (pPacketListener) {
		delete pPacketListener;
		pPacketListener = nullptr;
	}
}

HANDLE ENZTCPLIBRARY_API CreateLocalAreaListenerEx() {
	CLocalAreaListener* pLanListener = nullptr;
	try {
		pLanListener = new CLocalAreaListener();
		return pLanListener;
	}
	catch (int nError) {
		DEBUG_LOG("CreateLocalAreaListenerEx(): Exception (" + to_string(nError) + ")");
		if (pLanListener) {
			delete pLanListener;
			pLanListener = nullptr;
		}
		return pLanListener;
	}
}
bool ENZTCPLIBRARY_API StartLocalAreaListenerEx(HANDLE hHandle, const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimeMS) {
	CLocalAreaListener* pLanListener = (CLocalAreaListener*)hHandle;
	if (pLanListener)
		return pLanListener->StartEx(szStartingIPAddress, subNetMask, pFncPtr, nPollingTimeMS);
	else
		return false;
}
void ENZTCPLIBRARY_API StopLocalAreaListenerEx(HANDLE hHandle) {
	CLocalAreaListener* pLanListener = (CLocalAreaListener*)hHandle;
	if (pLanListener)
		return pLanListener->StopEx();
}
void ENZTCPLIBRARY_API DeleteLocalAreaListenerEx(HANDLE& hHandle) {
	CLocalAreaListener* pLanListener = (CLocalAreaListener*)hHandle;
	if (pLanListener) {
		delete pLanListener;
		pLanListener = nullptr;
	}
}

void ENZTCPLIBRARY_API SetNICAdapterToUse(const char* szAdapterName, ULONG ulIPAddress) {
	strcpy_s(g_szAdapterName, sizeof(g_szAdapterName), szAdapterName);
	g_ulAdapterIP = ulIPAddress;
	inet_ntop(AF_INET, &g_ulAdapterIP, g_szAdapterIP, sizeof(g_szAdapterIP));
}
