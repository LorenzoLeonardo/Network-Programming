#include "pch.h"
#include "CLocalAreaListener.h"
#include "DebugLog.h"

CLocalAreaListener* g_pCLocalAreaListener = nullptr;
CLocalAreaListener::CLocalAreaListener() {
	m_fnptrCallbackLocalAreaListener = nullptr;
	m_szStartingIP = "";
	m_szSubnetMask = "";
	m_threadMain = nullptr;
	m_bHasStarted = false;
	m_nPollingTimeMS = 0;
	m_bMainThreadStarted = FALSE;
	m_objICMP = nullptr;
	m_hStopThread = nullptr;
	m_hWaitThread = nullptr;
	m_hMainThread = nullptr;
	m_hMainStopThread = nullptr;
	try {
		m_objICMP = std::make_unique<CICMP>();
	}
	catch (int nError) {
		throw nError;
	}
}
CLocalAreaListener::CLocalAreaListener(const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimeMS) {
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_szSubnetMask = subNetMask;
	m_threadMain = nullptr;
	m_bHasStarted = false;
	m_nPollingTimeMS = nPollingTimeMS;
	m_bMainThreadStarted = FALSE;
	m_objICMP = nullptr;
	m_hStopThread = nullptr;
	m_hWaitThread = nullptr;
	m_hMainThread = nullptr;
	try {
		m_objICMP = std::make_unique<CICMP>();
	}
	catch (int nError) {
		throw nError;
	}
}

CLocalAreaListener::~CLocalAreaListener() {
	// WaitToEndThreads();
	if (m_hMainThread) {
		WaitForSingleObject(m_hStopThread, INFINITE);
		WaitForSingleObject(m_hWaitThread, INFINITE);
		WaitForSingleObject(m_hMainThread, INFINITE);
		WaitForSingleObject(m_hMainStopThread, INFINITE);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hMainThread);
		CloseHandle(m_hMainStopThread);
		m_hStopThread = nullptr;
		m_hWaitThread = nullptr;
		m_hMainThread = nullptr;
		m_hMainStopThread = nullptr;
	}
}
map<std::unique_ptr<thread>, int>* CLocalAreaListener::GetThreads() {
	return &m_mapThreads;
}

void CLocalAreaListener::MultiQueryingThread(void* args) {
	string* p = (string*)args;
	string hostName;
	string macAddress = "";

	if (g_pCLocalAreaListener->CheckIPDeviceConnected(*p, hostName, macAddress)) {
		if (!(hostName.empty() || macAddress.empty()))
			g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*)(*p).c_str(), (const char*)hostName.c_str(), (const char*)macAddress.c_str(), true);
	}
	delete p;
	p = nullptr;
}

void CLocalAreaListener::MainThread(void* args) {
	DEBUG_LOG("CLocalAreaListener: Thread Started.");
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	string subnetMask = pCLocalAreaListener->GetSubnetMask();
	int nPollTime = pCLocalAreaListener->GetPollingTime();
	ULONG ulLimit = 0;

	ULONG ulIP = 0;
	ULONG ulMask = 0;
	ULONG ulStartingIP = 0;
	ULONG ulTemp = 0;
	inet_pton(AF_INET, ipAddressStart.c_str(), &ulIP);
	inet_pton(AF_INET, subnetMask.c_str(), &ulMask);

	ulIP = htonl(ulIP);
	ulMask = htonl(ulMask);
	ulStartingIP = ulIP & ulMask;  // get the starting address
	ulLimit = 0xFFFFFFFF - ulMask; // get the max limit of IPAddress that can be search within the subnet.

	pCLocalAreaListener->SetMainThreadHasStarted(TRUE);

	char szIP[32] = {};

	ulIP = ulStartingIP;

	do {
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", nullptr, nullptr, false);
		for (ULONG i = 1; i <= (ulLimit - 1); i++) {
			ulTemp = ulStartingIP + i;
			ulTemp = htonl(ulTemp);
			inet_ntop(AF_INET, &ulTemp, szIP, sizeof(szIP));
			string* str = new string;
			*str = szIP;
			(*pCLocalAreaListener->GetThreads())[std::make_unique<thread>(MultiQueryingThread, str)] = i;
		}
		map<std::unique_ptr<thread>, int>::iterator it = pCLocalAreaListener->GetThreads()->begin();

		while (it != pCLocalAreaListener->GetThreads()->end()) {
			if (it->first->joinable())
				it->first->join();
			it++;
		}
		it = pCLocalAreaListener->GetThreads()->begin();

		pCLocalAreaListener->GetThreads()->clear();
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("end", nullptr, nullptr, false);
		Sleep(nPollTime);
	} while (pCLocalAreaListener->HasNotStopped());


	pCLocalAreaListener->SetMainThreadHasStarted(FALSE);
	g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("stop", nullptr, nullptr, false);
	DEBUG_LOG("CLocalAreaListener: Thread Ended.");
	return;
}
string CLocalAreaListener::GetStartingIPAddress() {
	return m_szStartingIP;
}
void CLocalAreaListener::Start() {
	if (!IsMainThreadStarted()) {
		m_bHasStarted = true;
		g_pCLocalAreaListener = this;

		m_threadMain = std::make_unique<thread>(MainThread, this);
		m_threadMain->join();
	}
}

void CLocalAreaListener::Stop() {
	m_bHasStarted = false;
}
bool CLocalAreaListener::CheckIPDeviceConnected(string ipAddress, string& hostName, string& macAddress) {
	if (m_objICMP) {
		return m_objICMP->CheckDevice(ipAddress, hostName, macAddress);
	}
	else
		return false;
}

unsigned _stdcall CLocalAreaListener::MultiQueryingThreadEx(void* args) {
	auto dumb_ptr = static_cast<std::shared_ptr<CLANObject>*>(args);
	auto obj = std::move(*dumb_ptr);
	delete dumb_ptr;

	string hostName;
	string macAddress = "";

	if (obj->m_pCLocalAreaListener->CheckIPDeviceConnected(obj->ipAddress, hostName, macAddress)) {
		if (!(hostName.empty() || macAddress.empty()))
			obj->m_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*)(obj->ipAddress).c_str(), (const char*)hostName.c_str(), (const char*)macAddress.c_str(), true);
	}

	return 0;
}

unsigned _stdcall CLocalAreaListener::MainThreadEx(void* args) {
	DEBUG_LOG("CLocalAreaListener:MainThreadEx() Thread Started.");
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	string subnetMask = pCLocalAreaListener->GetSubnetMask();
	int nPollTime = pCLocalAreaListener->GetPollingTime();
	ULONG ulLimit = 0;
	ULONG ulIP = 0;
	ULONG ulMask = 0;
	ULONG ulStartingIP = 0;
	ULONG ulTemp = 0;
	inet_pton(AF_INET, ipAddressStart.c_str(), &ulIP);
	inet_pton(AF_INET, subnetMask.c_str(), &ulMask);
	ulIP = htonl(ulIP);
	ulMask = htonl(ulMask);
	ulStartingIP = ulIP & ulMask;  // get the starting address
	ulLimit = 0xFFFFFFFF - ulMask; // get the max limit of IPAddress that can be search within the subnet.
	char szIP[32] = {};

	ulIP = ulStartingIP;
	DWORD dwRetSingleObject = 0;

	do {
		pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", nullptr, nullptr, false);
		dwRetSingleObject = WaitForSingleObject(pCLocalAreaListener->m_hStopThread, 0);
		for (ULONG i = 1; (i <= (ulLimit - 1)) && (dwRetSingleObject != WAIT_OBJECT_0); i++) {
			ulTemp = ulStartingIP + i;
			ulTemp = htonl(ulTemp);
			inet_ntop(AF_INET, &ulTemp, szIP, sizeof(szIP));
			std::shared_ptr<CLANObject> obj = std::make_shared<CLANObject>();
			if (!obj)
				break;
			obj->m_pCLocalAreaListener = pCLocalAreaListener;
			obj->ipAddress = szIP;
			auto dumb_ptr = new std::shared_ptr<CLANObject>(obj); // or add `std::move` here if the original `shared_ptr` instance isn't required anymore

			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, MultiQueryingThreadEx, static_cast<void*>(dumb_ptr), 0, nullptr);
			if (!hThread)
				break;
			pCLocalAreaListener->m_mapThreadsEx[hThread] = i;
		}
		map<HANDLE, int>::iterator it = pCLocalAreaListener->m_mapThreadsEx.begin();

		while (it != pCLocalAreaListener->m_mapThreadsEx.end()) {
			WaitForSingleObject(it->first, INFINITE);
			it++;
		}
		it = pCLocalAreaListener->m_mapThreadsEx.begin();
		while (it != pCLocalAreaListener->m_mapThreadsEx.end()) {
			CloseHandle(it->first);
			it++;
		}
		pCLocalAreaListener->m_mapThreadsEx.clear();
		pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("end", nullptr, nullptr, false);
		Sleep(nPollTime);
	} while (dwRetSingleObject != WAIT_OBJECT_0);
	SetEvent(pCLocalAreaListener->m_hWaitThread);

	DEBUG_LOG("CLocalAreaListener:MainThreadEx() Thread Ended.");
	pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("stop", nullptr, nullptr, false);

	return 0;
}

unsigned _stdcall CLocalAreaListener::StopThread(void* args) {
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	SetEvent(pCLocalAreaListener->m_hStopThread);
	pCLocalAreaListener->WaitListeningEx(pCLocalAreaListener->m_hWaitThread);

	return 0;
}
bool CLocalAreaListener::StartEx(const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimeMS) {
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_szSubnetMask = subNetMask;
	m_nPollingTimeMS = nPollingTimeMS;
	m_objICMP->InitializeLocalIPAndHostname(m_szStartingIP.c_str());
	if (m_hMainThread) {
		WaitForSingleObject(m_hStopThread, INFINITE);
		WaitForSingleObject(m_hWaitThread, INFINITE);
		WaitListeningEx(m_hMainThread);
		WaitListeningEx(m_hMainStopThread);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hMainThread);
		CloseHandle(m_hMainStopThread);
		m_hStopThread = nullptr;
		m_hWaitThread = nullptr;
		m_hMainThread = nullptr;
		m_hMainStopThread = nullptr;
	}
	m_hStopThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!m_hStopThread)
		return false;
	m_hWaitThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!m_hWaitThread) {
		CloseHandle(m_hStopThread);
		return false;
	}
	m_hMainThread = (HANDLE)_beginthreadex(nullptr, 0, MainThreadEx, this, 0, nullptr);
	if (!m_hMainThread) {
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		return false;
	}
	return true;
}
void CLocalAreaListener::StopEx() {
	if (m_hMainThread)
		m_hMainStopThread = (HANDLE)_beginthreadex(nullptr, 0, StopThread, this, 0, nullptr);
}

void CLocalAreaListener::WaitListeningEx(HANDLE hHandle) {
	while (::MsgWaitForMultipleObjects(1, &hHandle, FALSE, INFINITE,
			   QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1) {
		MSG message;
		::PeekMessage(&message, 0, 0, 0, PM_NOREMOVE);
	}
}
