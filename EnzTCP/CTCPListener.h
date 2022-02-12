#pragma once

#include"CSocket.h"
#include "CSocketServer.h"
#include <thread>
#include <stdio.h>
#include <string>
#include <iostream>


using namespace std;

typedef void (*FuncNewConnection)(void*);

class CTCPListener
{
private:
	string m_ipAddress;
	string m_port;
	
	CSocketServer*  m_socketServer;
	bool m_bIsRunning;
	FuncNewConnection m_pfnMessageReceived;

public:
	
	CTCPListener(string ipAddress, string port, FuncNewConnection handler);
	CTCPListener(string port, FuncNewConnection handler);
	~CTCPListener();

	
	void Run();
	void Stop();
};

