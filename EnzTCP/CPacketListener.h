#pragma once
#include "EnzTCP.h"
#include <stdlib.h>
#include <thread>

using namespace std;

class CPacketListener
{
private:
	bool m_bIsStopped;
	thread* m_threadListening;
	SOCKET m_socket;
	void* m_pObject;
	HANDLE m_hThread;
	HANDLE m_hStopThread;
	HANDLE m_hWaitThread;
protected:
	void CleanupHandles();
public :

	CPacketListener(FNCallbackPacketListener fnPtr);
	CPacketListener(FNCallbackPacketListenerEx fnPtr, void* pObject);
	~CPacketListener();
	
	FNCallbackPacketListener m_fnCallbackDisplay;
	FNCallbackPacketListenerEx m_fnCallbackDisplayEx;

	static void PollingThread(void* args);
	static unsigned _stdcall PollingThreadEx(void* args);
	bool StartListening();
	bool StartListeningEx();
	void StopListening();
	void StopListeningEx();
	bool IsStopped()
	{
		return m_bIsStopped;
	}
	SOCKET GetSocket()
	{
		return m_socket;
	}
	thread* GetThread()
	{
		return m_threadListening;
	}
	void* GetCustomObject()
	{
		return m_pObject;
	}
	HANDLE GetStopEventHandle()
	{
		return m_hStopThread;
	}
	HANDLE GetWaitEventHandle()
	{
		return m_hWaitThread;
	}
};

