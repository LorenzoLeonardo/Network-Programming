#include "pch.h"
#include "CSocketServer.h"



CSocketServer::CSocketServer(string serverPort) {
	m_serverPort = serverPort;
	m_ListenSocket = INVALID_SOCKET;
}
CSocketServer::~CSocketServer() {
}

bool CSocketServer::Initialize(string port) {
	WSADATA wsaData = {};
	int iResult = 0;
	int nError = 0;

	m_serverPort = port;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		nError = WSAGetLastError();
		throw nError;
	}
	// cout << "WSAStartup() = SUCCESS.\r\n";
	struct addrinfo *result = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(nullptr, m_serverPort.c_str(), &hints, &result);
	if (iResult != 0) {
		nError = WSAGetLastError();
		WSACleanup();
		throw nError;
	}
	// cout << "getaddrinfo() = SUCCESS.\r\n";
	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ListenSocket == INVALID_SOCKET) {
		nError = WSAGetLastError();
		// printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		throw nError;
	}
	// cout << "socket() = SUCCESS.\r\n";
	//  Setup the TCP listening socket
	iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		nError = WSAGetLastError();
		freeaddrinfo(result);
		closesocket(m_ListenSocket);
		WSACleanup();
		throw nError;
	}
	// cout << "bind() = SUCCESS.\r\n";
	if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		nError = WSAGetLastError();
		closesocket(m_ListenSocket);
		WSACleanup();
		throw nError;
	}

	return true;
}
CSocket* CSocketServer::Accept() {
	struct sockaddr client_addr;
	int nSize = sizeof(client_addr);
	CSocket* pSocket = nullptr;

	memset(&client_addr, 0, sizeof(client_addr));
	SOCKET ClientSocket = accept(m_ListenSocket, (struct sockaddr*)&client_addr, &nSize);
	if (ClientSocket == INVALID_SOCKET) {
		WSAGetLastError();
		return nullptr;
	}
	else {
		pSocket = new CSocket(ClientSocket);
		pSocket->SetClientAddr(client_addr);
		string ip = pSocket->GetIP();
		string hostname = pSocket->GetHostName();
		return pSocket;
	}
}
bool CSocketServer::Cleanup() {
	closesocket(m_ListenSocket);
	WSACleanup();
	return true;
}
