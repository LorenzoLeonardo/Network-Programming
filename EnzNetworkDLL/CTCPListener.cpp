#include "pch.h"
#include "CTCPListener.h"


CTCPListener::CTCPListener(string ipAddress, string port, FNCallbackNewConnection handler) : m_ipAddress(ipAddress), m_port(port), m_pfnMessageReceived(handler) {
	m_bIsRunning = true;
	m_socketServer = std::make_unique<CSocketServer>(m_port);
}

CTCPListener::CTCPListener(string port, FNCallbackNewConnection handler) : m_port(port), m_pfnMessageReceived(handler) {
	m_bIsRunning = true;
	m_socketServer = std::make_unique<CSocketServer>(m_port);
}

CTCPListener::~CTCPListener() {
}


void CTCPListener::Stop() {
	m_bIsRunning = false;
	m_socketServer->Cleanup();
}
void CTCPListener::Run() {
	if (!m_socketServer->Initialize(m_port))
		return;

	while (m_bIsRunning) {
		CSocket* socket = m_socketServer->Accept();
		m_pfnMessageReceived((void*)socket);
	}
}