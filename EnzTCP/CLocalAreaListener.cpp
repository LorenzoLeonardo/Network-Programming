#include "pch.h"
#include "CLocalAreaListener.h"
#include "CICMP.h"


CLocalAreaListener* g_pCLocalAreaListener = NULL;

CLocalAreaListener::CLocalAreaListener(const char* szStartingIPAddress, CallbackLocalAreaListener pFncPtr, int nPollingTimeMS)
{
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_threadMain = NULL;
	m_bHasStarted = false;
	m_nPollingTimeMS = nPollingTimeMS;
	m_bMainThreadStarted = FALSE;
}

CLocalAreaListener::~CLocalAreaListener()
{
	
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
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	int nStart = atoi(ipAddressStart.substr(ipAddressStart.rfind('.', ipAddressStart.size())+1, ipAddressStart.size()).c_str());
	int nPollTime = pCLocalAreaListener->GetPollingTime();

	ipAddressStart = ipAddressStart.substr(0, ipAddressStart.rfind('.', ipAddressStart.size()) + 1);
	pCLocalAreaListener->SetMainThreadHasStarted(TRUE);
	do
	{
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener("start", NULL,NULL, false);
		for (int i = 1; i <= 254; i++)
		{
			string* str = new string;
			*str = ipAddressStart + to_string(i);
			(*pCLocalAreaListener->GetThreads())[new thread(MultiQueryingThread, str)] = i;
		}
		map<thread*, int>::iterator it = pCLocalAreaListener->GetThreads()->begin();

		while (it != pCLocalAreaListener->GetThreads()->end())
		{
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
		m_threadMain->detach();
	}
}

void CLocalAreaListener::Stop()
{
	m_bHasStarted = false;
}
bool CLocalAreaListener::CheckIPDeviceConnected(string ipAddress,string &hostName, string &macAddress)
{
	CICMP* objICMP = new CICMP();
	bool bRet = objICMP->CheckDevice(ipAddress, hostName, macAddress);
	delete objICMP;

	return bRet;
}
