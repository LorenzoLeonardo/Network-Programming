#pragma once
#include "EnzTCP.h"
#include <thread>
#include <map>
#include <string>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <mutex>
using namespace std;



typedef void (*CallbackLocalAreaListener)(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsConnected);

class CLocalAreaListener
{
public:
	CLocalAreaListener(const char *szStartingIPAddress, CallbackLocalAreaListener pFncPtr, int nPollingTimesMS);
	~CLocalAreaListener();
	CallbackLocalAreaListener m_fnptrCallbackLocalAreaListener;

	void Start();
	void Stop();
	string GetStartingIPAddress();
	bool CheckIPDeviceConnected(string ipAddress, string& hostName, string& macAddress);
	map<thread*, int>* GetThreads();
	bool HasNotStopped()
	{
		return m_bHasStarted;
	}
	int GetPollingTime()
	{
		return m_nPollingTimeMS;
	}
	int IsMainThreadStarted()
	{
		return m_bMainThreadStarted;
	}
	void SetMainThreadHasStarted(bool b)
	{
		m_bMainThreadStarted = b;
	}
private:

	map<thread*, int> m_mapThreads;
	thread* m_threadMain;
	string  m_szStartingIP;
	bool	m_bHasStarted;
	int		m_nPollingTimeMS;
	bool    m_bMainThreadStarted;


};
