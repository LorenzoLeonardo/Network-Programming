#pragma once
#include "EnzNetworkDLL.h"
#include <string>
#include <map>
#include <thread>

using namespace std;

typedef struct
{
	string sPort;
} THREADMON_t;
class COpenPortListener {
public:
	COpenPortListener();
	COpenPortListener(string ipTargetIPAddress, int nNumberOfPorts, FNCallbackFindOpenPort pfnPtr);
	COpenPortListener(string ipTargetIPAddress, int nPort);

	~COpenPortListener();
	FNCallbackFindOpenPort m_pfnFindOpenPort;
	void StartSearchingOpenPorts();
	void StopSearchingOpenPorts();
	string GetIPAddress();
	map<std::shared_ptr<thread>, int>* GetThreads();
	std::shared_ptr<thread> GetThreadMonitoring();
	bool IsPortOpen(string ipAddress, string port, int* pLastError);
	int GetNumPorts();
	bool IsStopped() {
		return m_bStopSearchingOpenPorts;
	}

private:
	string m_ipAddressTarget;
	int m_nNumPorts;
	map<std::shared_ptr<thread>, int> m_mapThreads;
	std::shared_ptr<thread> m_tMonitor;
	int m_nPort;
	bool m_bStopSearchingOpenPorts;
};
