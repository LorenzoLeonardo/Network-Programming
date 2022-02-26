#include "pch.h"
#include "CPacketListener.h"
#include "DebugLog.h"

CPacketListener::CPacketListener(FNCallbackPacketListener fnPtr)
{
	m_bIsStopped = true;
	m_pObject = NULL;
	m_socket = INVALID_SOCKET;
	m_threadListening = NULL;
	m_fnCallbackDisplay = fnPtr;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;

}
CPacketListener::CPacketListener(FNCallbackPacketListenerEx fnPtr, void* pObject)
{
	m_bIsStopped = true;
	m_pObject = pObject;
	m_socket = INVALID_SOCKET;
	m_threadListening = NULL;
	m_fnCallbackDisplayEx = fnPtr;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;

}
CPacketListener::~CPacketListener()
{
	if (m_threadListening)
	{
		m_threadListening->detach();
		delete m_threadListening;
		m_threadListening = NULL;
	}
	WSACleanup();
}
void CPacketListener::PollingThread(void* args)
{
	DEBUG_LOG("CPacketListener: Thread Started.");
	CPacketListener* pListener = (CPacketListener*)args;
	int nBytes = 0;
	char* pBuffer = (char*)malloc(65536);
	unsigned char* upBuffer = NULL;
	
	if (pBuffer == NULL)
		return;
	do
	{
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, 65536, 0, NULL, 0);
		upBuffer = reinterpret_cast<unsigned char*> (pBuffer);
		pListener->m_fnCallbackDisplay(upBuffer, nBytes);
		memset(upBuffer, 0, 65536);
	} while ((nBytes > 0) && !pListener->IsStopped());

	free(pBuffer);
	pBuffer = NULL;
	closesocket(pListener->GetSocket());
	DEBUG_LOG("CPacketListener: Thread Ended.");
	return;
}

void CPacketListener::PollingThreadEx(void* args)
{
	DEBUG_LOG("CPacketListener: PollingThreadEx Thread Started.");
	CPacketListener* pListener = (CPacketListener*)args;
	int nBytes = 0;
	char* pBuffer = (char*)malloc(65536);
	unsigned char* upBuffer = NULL;

	if (pBuffer == NULL)
		return;
	do
	{
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, 65536, 0, NULL, 0);
		upBuffer = reinterpret_cast<unsigned char*> (pBuffer);
		pListener->m_fnCallbackDisplayEx(upBuffer, nBytes, pListener->GetCustomObject());
		memset(upBuffer, 0, 65536);
	} while ((nBytes > 0) && !pListener->IsStopped());

	free(pBuffer);
	pBuffer = NULL;
	DEBUG_LOG("CPacketListener: PollingThreadEx Thread Ended.");
	return;
}

bool CPacketListener::StartListening()
{
	int	iResult = 0;
	char szHostname[100];

	memset(szHostname, 0, sizeof(szHostname));

	m_bIsStopped = false;


	m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (gethostname(szHostname, sizeof(szHostname)) == SOCKET_ERROR)
	{
		closesocket(m_socket);
		return false;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_NUMERICSERV;

	iResult = getaddrinfo(szHostname, NULL, &hints, &result);

	if (iResult != NULL)
	{
		closesocket(m_socket);
		return false;
	}

	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}
	
	
	int nInput = 1;
	
	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}
	if (m_threadListening != NULL)
	{
		delete m_threadListening;
		m_threadListening = NULL;
	}
	m_threadListening = new thread(PollingThread, this);
	freeaddrinfo(result);
	m_threadListening->join();
	return true;
}

bool CPacketListener::StartListeningEx()
{
	int	iResult = 0;
	char szHostname[100];

	memset(szHostname, 0, sizeof(szHostname));

	m_bIsStopped = false;

	m_socket = NULL;
	m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (gethostname(szHostname, sizeof(szHostname)) == SOCKET_ERROR)
	{
		closesocket(m_socket);
		return false;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_NUMERICSERV;

	iResult = getaddrinfo(szHostname, NULL, &hints, &result);

	if (iResult != NULL)
	{
		closesocket(m_socket);
		return false;
	}

	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}


	int nInput = 1;

	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}
	if (m_threadListening != NULL)
	{
		m_threadListening->detach();
		delete m_threadListening;
		m_threadListening = NULL;
	}
	m_threadListening = new thread(PollingThreadEx, this);
	freeaddrinfo(result);
	return true;
}
void CPacketListener::StopListening()
{
	m_bIsStopped = true;
}
void CPacketListener::StopListeningEx()
{
	m_bIsStopped = true;
	if (m_socket)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}
}
