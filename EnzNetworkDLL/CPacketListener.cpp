#include "pch.h"
#include "CPacketListener.h"
#include "DebugLog.h"

CPacketListener::CPacketListener(FNCallbackPacketListener fnPtr) {
	m_bIsStopped = true;
	m_pObject = nullptr;
	m_socket = INVALID_SOCKET;
	m_threadListening = nullptr;
	m_fnCallbackDisplay = fnPtr;
	m_fnCallbackDisplayEx = nullptr;
	m_hThread = nullptr;
	m_hStopThread = nullptr;
	m_hWaitThread = nullptr;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;
}
CPacketListener::CPacketListener(FNCallbackPacketListenerEx fnPtr, void* pObject) {
	m_bIsStopped = true;
	m_pObject = pObject;
	m_socket = INVALID_SOCKET;
	m_threadListening = nullptr;
	m_fnCallbackDisplayEx = fnPtr;
	m_fnCallbackDisplay = nullptr;
	m_hThread = nullptr;
	m_hStopThread = nullptr;
	m_hWaitThread = nullptr;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;
}
CPacketListener::~CPacketListener() {
	CleanupHandles();
	WSACleanup();
}
void CPacketListener::CleanupHandles() {
	if (m_hThread != nullptr) {
		SetEvent(m_hStopThread);
		WaitListeningEx(m_hWaitThread);
		CloseHandle(m_hStopThread);
		CloseHandle(m_hWaitThread);
		CloseHandle(m_hThread);
		m_hStopThread = nullptr;
		m_hWaitThread = nullptr;
		m_hThread = nullptr;
	}
}
void CPacketListener::PollingThread(void* args) {
	DEBUG_LOG("CPacketListener: Thread Started.");
	CPacketListener* pListener = (CPacketListener*)args;
	int nBytes = 0;
	char* pBuffer = (char*)malloc(MAX_PACKET_SIZE);
	unsigned char* upBuffer = nullptr;

	if (pBuffer == nullptr) {
		DEBUG_LOG("CPacketListener: Out of Memory.");
		DEBUG_LOG("CPacketListener: Thread Ended.");
		return;
	}
	do {
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, MAX_PACKET_SIZE, 0, nullptr, 0);
		upBuffer = reinterpret_cast<unsigned char*>(pBuffer);
		pListener->m_fnCallbackDisplay(upBuffer, nBytes);
		memset(upBuffer, 0, MAX_PACKET_SIZE);
	} while ((nBytes > 0) && !pListener->IsStopped());

	free(pBuffer);
	pBuffer = nullptr;
	closesocket(pListener->GetSocket());
	DEBUG_LOG("CPacketListener: Thread Ended.");
	return;
}

unsigned _stdcall CPacketListener::PollingThreadEx(void* args) {
	CPacketListener* pListener = (CPacketListener*)args;
	DEBUG_LOG("CPacketListener: PollingThreadEx (" + to_string(GetCurrentThreadId()) + ") Thread Started.");

	int nBytes = 0;
	unsigned char* pBuffer = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PACKET_SIZE);


	if (pBuffer == nullptr) {
		SetEvent(pListener->m_hWaitThread);
		DEBUG_LOG("CPacketListener: Out of Memory." + to_string(GetCurrentThreadId()));
		DEBUG_LOG("CPacketListener: PollingThreadEx (" + to_string(GetCurrentThreadId()) + ") Thread Ended.");
		return 0;
	}
	do {
		nBytes = recvfrom(pListener->GetSocket(), (char*)pBuffer, MAX_PACKET_SIZE, 0, nullptr, 0);

		pListener->m_fnCallbackDisplayEx(pBuffer, nBytes, pListener->m_pObject);
	} while ((nBytes > 0) && (WaitForSingleObject(pListener->m_hStopThread, 0) != WAIT_OBJECT_0));

	HeapFree(GetProcessHeap(), 0, pBuffer);
	pBuffer = nullptr;
	DEBUG_LOG("CPacketListener: PollingThreadEx (" + to_string(GetCurrentThreadId()) + ") Thread Ended.");
	SetEvent(pListener->m_hWaitThread);

	return 0;
}

bool CPacketListener::StartListening() {
	int iResult = 0;
	char szHostname[100] = {};

	m_bIsStopped = false;
	m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (gethostname(szHostname, sizeof(szHostname)) == SOCKET_ERROR) {
		closesocket(m_socket);
		return false;
	}

	struct addrinfo *result = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_NUMERICSERV;

	iResult = getaddrinfo(szHostname, nullptr, &hints, &result);

	if (iResult != ERROR_SUCCESS) {
		closesocket(m_socket);
		return false;
	}

	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}

	int nInput = 1;

	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(m_socket);
		return false;
	}

	m_threadListening = std::make_shared<thread>(PollingThread, this);
	freeaddrinfo(result);
	m_threadListening->join();
	return true;
}

bool CPacketListener::StartListeningEx(ULONG ulNICIP) {
	int iResult = 0;
	char szHostname[100] = {};
	int nInput = 1;
	ULONG ulIP = 0;

	m_socket = INVALID_SOCKET;
	m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (gethostname(szHostname, sizeof(szHostname)) == SOCKET_ERROR) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_NUMERICSERV;

	iResult = getaddrinfo(szHostname, nullptr, &hints, &result);

	if (iResult != ERROR_SUCCESS) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	ptr = result;
	while (ptr != nullptr) {
		memcpy(&ulIP, (ptr->ai_addr->sa_data + 2), sizeof(ulIP));
		if (ulIP == ulNICIP) {
			result = ptr;
			break;
		}
		ptr = ptr->ai_next;
	}

	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	CleanupHandles();
	m_hStopThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_hWaitThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, PollingThreadEx, this, 0, nullptr);
	freeaddrinfo(result);
	return true;
}
void CPacketListener::StopListening() {
	m_bIsStopped = true;
}
void CPacketListener::StopListeningEx() {
	if (m_hStopThread) {
		SetEvent(m_hStopThread);
	}
	if (m_socket != INVALID_SOCKET) {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void CPacketListener::WaitListeningEx(HANDLE hHandle) {
	while (::MsgWaitForMultipleObjects(1, &hHandle, FALSE, INFINITE,
			   QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1) {
		MSG message;
		::PeekMessage(&message, 0, 0, 0, PM_NOREMOVE);
	}
}
