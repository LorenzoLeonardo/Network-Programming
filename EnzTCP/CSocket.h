#pragma once
#include "EnzTCP.h"
#include <string>
#include <windows.h>

using namespace std;

class CSocket : ISocket
{
protected:
	SOCKET m_socket;
	struct sockaddr m_addr;
	string m_hostname;
	string m_ipAddress;
	string dataRecv;
	void SetHostname();
	void SetIP();

public:
	CSocket();
	CSocket(SOCKET s);
	~CSocket();
	SOCKET  GetSocket();
	void  SetClientAddr(struct sockaddr addr);
	const char* GetIP();
	const char* GetHostName();
	const char* Receive();
	void Send(char* sendbuf);
};

