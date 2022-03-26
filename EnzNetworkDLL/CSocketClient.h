#pragma once
#include "CSocket.h"
#include <string>
using namespace std;


class CSocketClient : CSocket
{
public:
	CSocketClient();
	CSocketClient(string ipServer);
	~CSocketClient();
	CSocketClient(string ipServer, string sPort);
	bool ConnectToServer(string ipServer, string, int *pLastError);
	bool ConnectToServer(int* pLastError);
	bool DisconnectFromServer();

private:
	string m_sPortNum;
};

