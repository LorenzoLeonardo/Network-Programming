#include "pch.h"
#include "CLocalAreaListener.h"
#include "DebugLog.h"

CLocalAreaListener* g_pCLocalAreaListener = NULL;

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
}
map<thread*, int>* CLocalAreaListener::GetThreads()
{
	return &m_mapThreads;
}

void MultiQueryingThread(void* args)
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

void MainThread(void* args)
{
	DEBUG_LOG("CLocalAreaListener: Thread Started.");
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	string subnetMask = pCLocalAreaListener->GetSubnetMask();
	int nStart = atoi(ipAddressStart.substr(ipAddressStart.rfind('.', ipAddressStart.size())+1, ipAddressStart.size()).c_str());
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

	ipAddressStart = ipAddressStart.substr(0, ipAddressStart.rfind('.', ipAddressStart.size()) + 1);
	pCLocalAreaListener->SetMainThreadHasStarted(TRUE);

	char szIP[32];
	memset(szIP, 0,sizeof(szIP));

	ulIP = ulStartingIP;

	do
	{
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", NULL,NULL, false);
		for (int i = 1; i <= (ulLimit-1); i++)
		{
			ulTemp = ulIP + i;
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
