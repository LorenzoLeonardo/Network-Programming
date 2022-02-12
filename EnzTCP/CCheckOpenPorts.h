#pragma once
#include "EnzTCP.h"
#include <string>
#include <map>
#include <thread>

using namespace std;

typedef struct
{
	string sPort;
}THREADMON_t;
class CCheckOpenPorts
{
public :
	CCheckOpenPorts();
	CCheckOpenPorts(string ipTargetIPAddress, int nNumberOfPorts, FuncFindOpenPort pfnPtr);
	CCheckOpenPorts(string ipTargetIPAddress, int nPort);

	~CCheckOpenPorts();
	FuncFindOpenPort m_pfnFindOpenPort;
	void StartSearchingOpenPorts();
	void StopSearchingOpenPorts();
	string GetIPAddress();
	map<thread*, int> *GetThreads();
	thread* GetThreadMonitoring();
	bool IsPortOpen(string ipAddress, string port, int* pLastError);
	int GetNumPorts();
	bool IsStopped()
	{
		return m_bStopSearchingOpenPorts;
	}
private:
	string m_ipAddressTarget;
	int m_nNumPorts;
	map<thread*, int> m_mapThreads;
	thread* m_tMonitor;
	int m_nPort;
	bool m_bStopSearchingOpenPorts;
};

