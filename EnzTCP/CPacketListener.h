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
public :

	CPacketListener(FNCallbackPacketListener fnPtr);
	~CPacketListener();
	
	FNCallbackPacketListener m_fnCallbackDisplay;

	static void PollingThread(void* args);
	bool StartListening();
	void StopListening();
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
};

