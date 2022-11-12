#pragma once
#include "EnzNetworkDLL.h"
#include <string>
#include <windows.h>

using namespace std;

class CSocket : ISocket {
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
	virtual ~CSocket();
	SOCKET GetSocket();
	void SetClientAddr(struct sockaddr addr);
	const char* GetIP();
	const char* GetHostName();
	const char* Receive();
	void Send(char* sendbuf);
	bool GetDefaultGateway(char* szDefaultIPAddress);
	bool GetDefaultGateway(const char* szAdapterName, char* szDefaultGateway, int nSize);
};
