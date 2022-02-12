#pragma once
#include "EnzTCP.h"
#include <stdlib.h>
#include <thread>

using namespace std;

class CPacketListener
{
private:
	bool m_bIsStopped;
	thread* m_thread;
	SOCKET m_socket;
public :

	CPacketListener();
	~CPacketListener();
	
	FNCallbackPacketListener m_fnCallback;

	static void PollingThread(void* args);
	bool StartListening(FNCallbackPacketListener fnPtr);
	void StopListening();
	bool IsStopped()
	{
		return m_bIsStopped;
	}
	SOCKET GetSocket()
	{
		return m_socket;
	}
};

