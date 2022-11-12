#pragma once
#include "EnzNetworkDLL.h"
#include <thread>
#include <map>
#include <string>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <mutex>
#include "CICMP.h"

using namespace std;

typedef void (*FNCallbackLocalAreaListener)(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsConnected);

class CLocalAreaListener {
public:
	CLocalAreaListener();

	CLocalAreaListener(const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimesMS);
	~CLocalAreaListener();
	FNCallbackLocalAreaListener m_fnptrCallbackLocalAreaListener;

	static void MainThread(void* args);
	static void MultiQueryingThread(void* args);

	void Start();
	void Stop();
	string GetStartingIPAddress();
	bool CheckIPDeviceConnected(string ipAddress, string& hostName, string& macAddress);
	map<unique_ptr<thread>, int>* GetThreads();
	bool HasNotStopped() {
		return m_bHasStarted;
	}
	int GetPollingTime() {
		return m_nPollingTimeMS;
	}
	int IsMainThreadStarted() {
		return m_bMainThreadStarted;
	}
	void SetMainThreadHasStarted(bool b) {
		m_bMainThreadStarted = b;
	}
	string GetSubnetMask() {
		return m_szSubnetMask;
	}
	HANDLE m_hStopThread;
	HANDLE m_hWaitThread;
	HANDLE m_hMainThread;
	HANDLE m_hMainStopThread;
	static unsigned _stdcall MainThreadEx(void* args);
	static unsigned _stdcall MultiQueryingThreadEx(void* args);
	static unsigned _stdcall StopThread(void* args);
	bool StartEx(const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimeMS);
	void StopEx();
	void WaitListeningEx(HANDLE hHandle);

private:
	map<std::unique_ptr<thread>, int> m_mapThreads;
	map<HANDLE, int> m_mapThreadsEx;
	thread* m_threadMain;
	string m_szStartingIP;
	string m_szSubnetMask;
	bool m_bHasStarted;
	int m_nPollingTimeMS;
	bool m_bMainThreadStarted;
	std::unique_ptr<CICMP> m_objICMP;
};

class CLANObject {
public:
	CLocalAreaListener* m_pCLocalAreaListener;
	string ipAddress;

	CLANObject() {
		m_pCLocalAreaListener = nullptr;
	}
};