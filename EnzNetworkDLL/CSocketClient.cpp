#include "pch.h"
#include "CSocketClient.h"
#include <mutex>
#include <iostream>

CSocketClient::CSocketClient()
{
	m_socket = INVALID_SOCKET;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw iResult;
	}
}
CSocketClient::CSocketClient(string ipServer)
{
	m_socket = INVALID_SOCKET;
	m_ipAddress = ipServer;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw iResult;
	}

}
CSocketClient::CSocketClient(string ipServer,string sPort)
{
	m_socket = INVALID_SOCKET;
	m_ipAddress = ipServer;
	m_sPortNum = sPort;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw iResult;
	}

}
CSocketClient::~CSocketClient()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}
	WSACleanup();
}
bool CSocketClient::ConnectToServer(int* pLastError)
{
	int iResult;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	*pLastError = 0;
	// Resolve the server address and port

	iResult = getaddrinfo(m_ipAddress.c_str(), m_sPortNum.c_str(), &hints, &result);
	if (iResult != 0) {
		*pLastError = WSAGetLastError();
		return false;
	}

	// Attempt to connect to the first address returned by
// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	m_socket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (m_socket == INVALID_SOCKET) {
		*pLastError = WSAGetLastError();
		freeaddrinfo(result);
		return false;
	}

	iResult = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		*pLastError = WSAGetLastError();

		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	freeaddrinfo(result);
	return true;
}
bool CSocketClient::ConnectToServer(string ipServer, string sPort, int *pLastError)
{
	int iResult;
	*pLastError = 0;

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port

	iResult = getaddrinfo(ipServer.c_str(), sPort.c_str(), &hints, &result);
	if (iResult != 0) {
		*pLastError = WSAGetLastError();
		return false;
	}

	// Attempt to connect to the first address returned by
// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	m_socket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (m_socket == INVALID_SOCKET) {
		*pLastError = WSAGetLastError();
		freeaddrinfo(result);
		return false;
	}

	iResult = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		*pLastError = WSAGetLastError();
		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	freeaddrinfo(result);

	return true;
}
bool CSocketClient::DisconnectFromServer()
{
	int iResult = shutdown(m_socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(m_socket);
		WSACleanup();
		return false;
	}
	closesocket(m_socket);
	WSACleanup();
	return true;
}