#include "pch.h"
#include "CPacketListener.h"


CPacketListener::CPacketListener(FNCallbackPacketListener fnPtr)
{
	m_bIsStopped = true;
	m_socket = INVALID_SOCKET;
	m_threadListening = NULL;
	m_fnCallback = fnPtr;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw INVALID_SOCKET;

}
CPacketListener::~CPacketListener()
{
	WSACleanup();
}
void CPacketListener::PollingThread(void* args)
{
	CPacketListener* pListener = (CPacketListener*)args;
	int nBytes = 0;
	char* pBuffer = (char*)malloc(65536);
	unsigned char* upBuffer = NULL;
	
	if (pBuffer == NULL)
		return;
	do
	{
		nBytes = recvfrom(pListener->GetSocket(), pBuffer, 65536, 0, NULL, 0); //Eat as much as u can
		upBuffer = reinterpret_cast<unsigned char*> (pBuffer);
		pListener->m_fnCallback(upBuffer, nBytes);
		memset(upBuffer, 0, 65536);
	} while ((nBytes > 0) && !pListener->IsStopped());

	free(pBuffer);
	pBuffer = NULL;
	closesocket(pListener->GetSocket());
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
		return false;

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_NUMERICSERV;

	iResult = getaddrinfo(szHostname, NULL, &hints, &result);

	if (iResult != NULL)
		return false;

	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		return false;
	}
	
	
	int nInput = 1;
	if (WSAIoctl(m_socket, SIO_RCVALL, &nInput, sizeof(nInput), 0, 0, (LPDWORD)&iResult, 0, 0) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		return false;
	}
	
	m_threadListening = new thread(PollingThread, this);
	freeaddrinfo(result);
	return true;
}
void CPacketListener::StopListening()
{
	m_bIsStopped = true;
}
