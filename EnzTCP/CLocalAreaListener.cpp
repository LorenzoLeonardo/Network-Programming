#include "pch.h"
#include "CLocalAreaListener.h"
#include "DebugLog.h"

CLocalAreaListener* g_pCLocalAreaListener = NULL;
CLocalAreaListener::CLocalAreaListener()
{
	m_fnptrCallbackLocalAreaListener = NULL;
	m_szStartingIP = "";
	m_szSubnetMask = "";
	m_threadMain = NULL;
	m_bHasStarted = false;
	m_nPollingTimeMS = 0;
	m_bMainThreadStarted = FALSE;
	m_objICMP = NULL;
	m_hStopThread = NULL;
	m_hWaitThread = NULL;
	m_hMainThread = NULL;
	m_hMainStopThread = NULL;
	try
	{
		m_objICMP = new CICMP();
	}
	catch (int nError)
	{
		throw nError;
	}
}
CLocalAreaListener::CLocalAreaListener(const char* szStartingIPAddress, const char* subNetMask, CallbackLocalAreaListener pFncPtr, int nPollingTimeMS)
{
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_szSubnetMask = subNetMask;
	m_threadMain = NULL;
	m_bHasStarted = false;
	m_nPollingTimeMS = nPollingTimeMS;
	m_bMainThreadStarted = FALSE;
	m_objICMP = NULL;
	m_hStopThread = NULL;
	m_hWaitThread = NULL;
	m_hMainThread = NULL;
	try
	{
		m_objICMP = new CICMP();
	}
	catch (int nError)
	{
		throw nError;
	}
}

CLocalAreaListener::~CLocalAreaListener()
{
	//WaitToEndThreads();
	if (m_threadMain != NULL)
	{
		delete m_threadMain;
		m_threadMain = NULL;
	}
	if (m_objICMP != NULL)
	{
		delete m_objICMP;
		m_objICMP = NULL;
	}

	if (m_hMainThread)
	{
		WaitForSingleObject(m_hStopThread, INFINITE);
		WaitForSingleObject(m_hWaitThread, INFINITE);
		WaitForSingleObject(m_hMainThread, INFINITE);
		WaitForSingleObject(m_hMainStopThread, INFINITE);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hMainThread);
		CloseHandle(m_hMainStopThread);
		m_hStopThread = NULL;
		m_hWaitThread = NULL;
		m_hMainThread = NULL;
		m_hMainStopThread = NULL;
	}
}
map<thread*, int>* CLocalAreaListener::GetThreads()
{
	return &m_mapThreads;
}

void CLocalAreaListener::MultiQueryingThread(void* args)
{
	string* p = (string*)args;
	string hostName;
	string macAddress = "";

	if (g_pCLocalAreaListener->CheckIPDeviceConnected(*p, hostName, macAddress))
	{
		if(!(hostName.empty() || macAddress.empty()))
			g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*)(*p).c_str(), (const char*)hostName.c_str(), (const char*)macAddress.c_str(), true);
	}
	delete p;
	p = NULL;
}

void CLocalAreaListener::MainThread(void* args)
{
	DEBUG_LOG("CLocalAreaListener: Thread Started.");
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	string subnetMask = pCLocalAreaListener->GetSubnetMask();
	int nPollTime = pCLocalAreaListener->GetPollingTime();
	ULONG ulLimit = 0;

	ULONG ulIP;
	ULONG ulMask;
	ULONG ulStartingIP;
	ULONG ulTemp;
	inet_pton(AF_INET, ipAddressStart.c_str(), &ulIP);
	inet_pton(AF_INET, subnetMask.c_str(), &ulMask);

	ulIP = htonl(ulIP);
	ulMask = htonl(ulMask);
	ulStartingIP = ulIP & ulMask;//get the starting address
	ulLimit = 0xFFFFFFFF - ulMask;//get the max limit of IPAddress that can be search within the subnet.

	pCLocalAreaListener->SetMainThreadHasStarted(TRUE);

	char szIP[32];
	memset(szIP, 0,sizeof(szIP));

	ulIP = ulStartingIP;

	do
	{
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", NULL,NULL, false);
		for (ULONG i = 1; i <= (ulLimit-1); i++)
		{
			ulTemp = ulStartingIP + i;
			ulTemp = htonl(ulTemp);
			inet_ntop(AF_INET, &ulTemp, szIP, sizeof(szIP));
			string* str = new string;
			*str = szIP;//ipAddressStart + to_string(i);
			(*pCLocalAreaListener->GetThreads())[new thread(MultiQueryingThread, str)] = i;
		}
		map<thread*, int>::iterator it = pCLocalAreaListener->GetThreads()->begin();

		while (it != pCLocalAreaListener->GetThreads()->end())
		{
			if(it->first->joinable())
				it->first->join();
			it++;
		}
		it = pCLocalAreaListener->GetThreads()->begin();
		while (it != pCLocalAreaListener->GetThreads()->end())
		{
			delete it->first;
			it++;
		}
		pCLocalAreaListener->GetThreads()->clear();
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("end", NULL,NULL, false);
		Sleep(nPollTime);
	} while (pCLocalAreaListener->HasNotStopped());

	
	pCLocalAreaListener->SetMainThreadHasStarted(FALSE);
	g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("stop", NULL, NULL, false);
	DEBUG_LOG("CLocalAreaListener: Thread Ended.");
	return;
}
string CLocalAreaListener::GetStartingIPAddress()
{
	return m_szStartingIP;
}
void CLocalAreaListener::Start()
{
	if (!IsMainThreadStarted())
	{
		m_bHasStarted = true;
		g_pCLocalAreaListener = this;
		if (m_threadMain != NULL)
		{
			delete m_threadMain;
			m_threadMain = NULL;
		}
		m_threadMain = new thread(MainThread, this);
		m_threadMain->join();
	}
}

void CLocalAreaListener::Stop()
{
	m_bHasStarted = false;
}
bool CLocalAreaListener::CheckIPDeviceConnected(string ipAddress,string &hostName, string &macAddress)
{
	if (m_objICMP)
		return 	m_objICMP->CheckDevice(ipAddress, hostName, macAddress);
	else
		return false;
}

unsigned _stdcall CLocalAreaListener::MultiQueryingThreadEx(void* args)
{
	CLANObject* obj = (CLANObject*)args;
	string hostName;
	string macAddress = "";

	if (obj->m_pCLocalAreaListener->CheckIPDeviceConnected(obj->ipAddress, hostName, macAddress))
	{
		if (!(hostName.empty() || macAddress.empty()))
			obj->m_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*)(obj->ipAddress).c_str(), (const char*)hostName.c_str(), (const char*)macAddress.c_str(), true);
	}
	delete obj;
	obj = NULL;
	_endthreadex(0);
	return 0;
}

unsigned _stdcall CLocalAreaListener::MainThreadEx(void* args)
{
	DEBUG_LOG("CLocalAreaListener:MainThreadEx() Thread Started.");
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	string subnetMask = pCLocalAreaListener->GetSubnetMask();
	int nPollTime = pCLocalAreaListener->GetPollingTime();
	ULONG ulLimit = 0;
	ULONG ulIP;
	ULONG ulMask;
	ULONG ulStartingIP;
	ULONG ulTemp;
	inet_pton(AF_INET, ipAddressStart.c_str(), &ulIP);
	inet_pton(AF_INET, subnetMask.c_str(), &ulMask);
	ulIP = htonl(ulIP);
	ulMask = htonl(ulMask);
	ulStartingIP = ulIP & ulMask;//get the starting address
	ulLimit = 0xFFFFFFFF - ulMask;//get the max limit of IPAddress that can be search within the subnet.
	char szIP[32];
	memset(szIP, 0, sizeof(szIP));

	ulIP = ulStartingIP;
	DWORD dwRetSingleObject = 0;
	do
	{
		pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", NULL, NULL, false);
		dwRetSingleObject = WaitForSingleObject(pCLocalAreaListener->m_hStopThread, 0);
		for (ULONG i = 1; (i <= (ulLimit - 1)) && (dwRetSingleObject != WAIT_OBJECT_0); i++)
		{
			ulTemp = ulStartingIP + i;
			ulTemp = htonl(ulTemp);
			inet_ntop(AF_INET, &ulTemp, szIP, sizeof(szIP));
			CLANObject* obj = new CLANObject();
			obj->m_pCLocalAreaListener = pCLocalAreaListener;
			obj->ipAddress = szIP;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, MultiQueryingThreadEx, obj, 0, NULL);
			pCLocalAreaListener->m_mapThreadsEx[hThread] = i;
		}
		map<HANDLE, int>::iterator it = pCLocalAreaListener->m_mapThreadsEx.begin();

		while (it != pCLocalAreaListener->m_mapThreadsEx.end())
		{
			WaitForSingleObject(it->first,INFINITE);
			it++;
		}
		it = pCLocalAreaListener->m_mapThreadsEx.begin();
		while (it != pCLocalAreaListener->m_mapThreadsEx.end())
		{
			CloseHandle(it->first);
			it++;
		}
		pCLocalAreaListener->m_mapThreadsEx.clear();
		pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("end", NULL, NULL, false);
		Sleep(nPollTime);
	} while (dwRetSingleObject != WAIT_OBJECT_0);
	SetEvent(pCLocalAreaListener->m_hWaitThread);
	
	DEBUG_LOG("CLocalAreaListener:MainThreadEx() Thread Ended.");
	pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("stop", NULL, NULL, false);
	_endthreadex(0);
	return 0;
}

unsigned _stdcall CLocalAreaListener::StopThread(void* args)
{
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	SetEvent(pCLocalAreaListener->m_hStopThread);
	WaitForSingleObject(pCLocalAreaListener->m_hWaitThread, INFINITE);
	_endthreadex(0);
	return 0;
}
bool CLocalAreaListener::StartEx(const char* szStartingIPAddress, const char* subNetMask, CallbackLocalAreaListener pFncPtr, int nPollingTimeMS)
{
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_szSubnetMask = subNetMask;
	m_nPollingTimeMS = nPollingTimeMS;

	if (m_hMainThread)
	{
		WaitForSingleObject(m_hStopThread, INFINITE);
		WaitForSingleObject(m_hWaitThread, INFINITE);
		WaitForSingleObject(m_hMainThread, INFINITE);
		WaitForSingleObject(m_hMainStopThread, INFINITE);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hMainThread);
		CloseHandle(m_hMainStopThread);
		m_hStopThread = NULL;
		m_hWaitThread = NULL;
		m_hMainThread = NULL;
		m_hMainStopThread = NULL;
	}
	m_hStopThread = CreateEvent(NULL,TRUE, FALSE,NULL);
	if (!m_hStopThread)
		return false;
	m_hWaitThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!m_hWaitThread)
	{
		CloseHandle(m_hStopThread);
		return false;
	}
	m_hMainThread = (HANDLE)_beginthreadex(NULL, 0, MainThreadEx, this, 0, NULL);
	if (!m_hMainThread)
	{
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		return false;
	}
	return true;
}
void CLocalAreaListener::StopEx()
{
	if (m_hMainThread)
		m_hMainStopThread = (HANDLE)_beginthreadex(NULL, 0, StopThread, this, 0, NULL);
}
