#include "pch.h"
#include "COpenPortListener.h"
#include "CSocketClient.h"
#include "DebugLog.h"
COpenPortListener* g_objPtrCCheckOpenPorts = nullptr;
COpenPortListener::COpenPortListener() {
	m_pfnFindOpenPort = nullptr;
	m_nNumPorts = 0;
	m_ipAddressTarget = "";
	g_objPtrCCheckOpenPorts = this;
	m_tMonitor = nullptr;
}
COpenPortListener::COpenPortListener(string ipTargetIPAddress, int nPort) {
	m_pfnFindOpenPort = nullptr;
	m_nNumPorts = 0;
	m_nPort = nPort;
	m_ipAddressTarget = ipTargetIPAddress;
	g_objPtrCCheckOpenPorts = this;
	m_tMonitor = nullptr;
}
COpenPortListener::COpenPortListener(string ipTargetIPAddress, int nNumberOfPorts, FNCallbackFindOpenPort pfnPtr) {
	m_pfnFindOpenPort = pfnPtr;
	m_nNumPorts = nNumberOfPorts;
	m_ipAddressTarget = ipTargetIPAddress;
	g_objPtrCCheckOpenPorts = this;
	m_tMonitor = nullptr;
}
COpenPortListener::~COpenPortListener() {
}
int COpenPortListener::GetNumPorts() {
	return m_nNumPorts;
}
string COpenPortListener::GetIPAddress() {
	return m_ipAddressTarget;
}
map<std::shared_ptr<thread>, int>* COpenPortListener::GetThreads() {
	return &m_mapThreads;
}
std::shared_ptr<thread> COpenPortListener::GetThreadMonitoring() {
	return m_tMonitor;
}
void ThreadMultiFunc(LPVOID pParam) {
	string cs;
	THREADMON_t* pTmon = (THREADMON_t*)pParam;
	int nLastError = 0;

	cs = g_objPtrCCheckOpenPorts->GetIPAddress();

	if (g_objPtrCCheckOpenPorts->IsPortOpen(cs, pTmon->sPort, &nLastError))
		g_objPtrCCheckOpenPorts->m_pfnFindOpenPort((char*)cs.c_str(), stoi(pTmon->sPort), true, nLastError);
	else
		g_objPtrCCheckOpenPorts->m_pfnFindOpenPort((char*)cs.c_str(), stoi(pTmon->sPort), false, nLastError);
	return;
}
void ThreadMonitorThreads(LPVOID pParam) {
	DEBUG_LOG("COpenPortListener: Thread Started.");
	COpenPortListener* pCCheckOpenPorts = (COpenPortListener*)pParam;
	int nOuterLoopLimit = pCCheckOpenPorts->GetNumPorts() / 1000;
	int i = 1;

	for (int j = 0; j <= nOuterLoopLimit && !pCCheckOpenPorts->IsStopped(); j++) {
		for (; (i <= pCCheckOpenPorts->GetNumPorts()) && (i % (1000 * (j + 1))) && !pCCheckOpenPorts->IsStopped(); i++) {
			THREADMON_t* ptmon = new THREADMON_t;
			ptmon->sPort = to_string(i);
			(*pCCheckOpenPorts->GetThreads())[std::make_unique<thread>(ThreadMultiFunc, ptmon)] = i;
		}
		// map<thread*, int>* PDlg = (map<thread*, int>*)pParam;
		map<std::shared_ptr<thread>, int>::iterator it = pCCheckOpenPorts->GetThreads()->begin();

		while (it != pCCheckOpenPorts->GetThreads()->end()) {
			it->first->join();
			it++;
		}
		it = pCCheckOpenPorts->GetThreads()->begin();

		pCCheckOpenPorts->GetThreads()->clear();
	}
	g_objPtrCCheckOpenPorts->m_pfnFindOpenPort((char*)"DONE", 0, true, 0);
	DEBUG_LOG("COpenPortListener: Thread Ended.");
	return;
}
bool COpenPortListener::IsPortOpen(string ipAddress, string port, int* pLastError) {
	try {
		CSocketClient clientSock(ipAddress);

		return clientSock.ConnectToServer(ipAddress, port, pLastError);
	}
	catch (int nError) {
		return nError == 0;
	}
}

void COpenPortListener::StartSearchingOpenPorts() {
	if (m_tMonitor == nullptr) {
		m_bStopSearchingOpenPorts = false;
		m_tMonitor = std::make_unique<thread>(ThreadMonitorThreads, this);
		m_tMonitor->join();
	}
}

void COpenPortListener::StopSearchingOpenPorts() {
	m_bStopSearchingOpenPorts = true;
}