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
	m_hThread = NULL;
	m_hStopThread = NULL;
	m_hWaitThread = NULL;
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
	m_hThread = NULL;
	m_hStopThread = NULL;
	m_hWaitThread = NULL;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;

}
CPacketListener::~CPacketListener()
{
	CleanupHandles();
	WSACleanup();
}
void CPacketListener::CleanupHandles()
{
	if (m_hThread != NULL)
	{
		SetEvent(m_hStopThread);
		WaitForSingleObject(m_hWaitThread, INFINITE);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hThread);
		m_hStopThread = NULL;
		m_hWaitThread = NULL;
		m_hThread = NULL;
	}
}
void CPacketListener::PollingThread(void* args)
{
	DEBUG_LOG("CPacketListener: Thread Started.");
	CPacketListener* pListener = (CPacketListener*)args;
	int nBytes = 0;
	char* pBuffer = (char*)malloc(MAX_PACKET_SIZE);
	unsigned char* upBuffer = NULL;
	
	if (pBuffer == NULL)
	{
		DEBUG_LOG("CPacketListener: Out of Memory.");
		DEBUG_LOG("CPacketListener: Thread Ended.");
		return;
	}
	do
	{
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, MAX_PACKET_SIZE, 0, NULL, 0);
		upBuffer = reinterpret_cast<unsigned char*> (pBuffer);
		pListener->m_fnCallbackDisplay(upBuffer, nBytes);
		memset(upBuffer, 0, MAX_PACKET_SIZE);
	} while ((nBytes > 0) && !pListener->IsStopped());

	free(pBuffer);
	pBuffer = NULL;
	closesocket(pListener->GetSocket());
	DEBUG_LOG("CPacketListener: Thread Ended.");
	return;
}

unsigned _stdcall CPacketListener::PollingThreadEx(void* args)
{
	CPacketListener* pListener = (CPacketListener*)args;
	DEBUG_LOG("CPacketListener: PollingThreadEx (" + to_string(GetCurrentThreadId()) + ") Thread Started.");
	
	int nBytes = 0;
	char* pBuffer = (char*)malloc(MAX_PACKET_SIZE);
	unsigned char* upBuffer = NULL;

	if (pBuffer == NULL)
	{
		SetEvent(pListener->m_hWaitThread);
		DEBUG_LOG("CPacketListener: Out of Memory." + to_string(GetCurrentThreadId()));
		DEBUG_LOG("CPacketListener: PollingThreadEx (" + to_string(GetCurrentThreadId()) + ") Thread Ended.");
		return 0;
	}
	do
	{
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, MAX_PACKET_SIZE, 0, NULL, 0);
		upBuffer = reinterpret_cast<unsigned char*> (pBuffer);
		pListener->m_fnCallbackDisplayEx(upBuffer, nBytes, pListener->m_pObject);
		memset(upBuffer, 0, MAX_PACKET_SIZE);
	} while ((nBytes > 0) && (WaitForSingleObject(pListener->m_hStopThread,0)!= WAIT_OBJECT_0));

	free(pBuffer);
	pBuffer = NULL;
	DEBUG_LOG("CPacketListener: PollingThreadEx ("+ to_string(GetCurrentThreadId()) + ") Thread Ended.");
	SetEvent(pListener->m_hWaitThread);

	return 0;
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

bool CPacketListener::StartListeningEx(ULONG ulNICIP)
{
	int	iResult = 0;
	char szHostname[100];
	int nInput = 1;
	ULONG ulIP;
	memset(szHostname, 0, sizeof(szHostname));

	//m_bIsStopped = false;
	m_socket = INVALID_SOCKET;
	m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (gethostname(szHostname, sizeof(szHostname)) == SOCKET_ERROR)
	{
		closesocket(m_socket);
		m_socket = NULL;
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
		m_socket = INVALID_SOCKET;
		return false;
	}

	ptr = result;
	while (ptr != NULL)
	{
		memcpy(&ulIP, (ptr->ai_addr->sa_data + 2), sizeof(ulIP));
		if (ulIP == ulNICIP)
		{
			result = ptr;
			break;
		}
		ptr = ptr->ai_next;
	}
	
	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	CleanupHandles();
	m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWaitThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL,0, PollingThreadEx, this,0,NULL);
	freeaddrinfo(result);
	return true;
}
void CPacketListener::StopListening()
{
	m_bIsStopped = true;
}
void CPacketListener::StopListeningEx()
{
	if (m_hStopThread)
	{
		SetEvent(m_hStopThread);

	}
	if (m_socket!=INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void CPacketListener::WaitListeningEx()
{
	WaitForSingleObject(m_hWaitThread, INFINITE);
}