/*
*	Created By Lorenzo Leonardo
*	Enzo Tech Computer Solutions
*
*	EnzTCP.dll Interface, Library for network related applications 
*
*/
#pragma once
#ifdef ENZTCPLIBRARY_EXPORTS
#define ENZTCPLIBRARY_API __declspec(dllexport)
#else
#define ENZTCPLIBRARY_API __declspec(dllimport)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include  <winsock2.h>
#include  <winsnmp.h>
#include  <snmp.h>
#include  <mgmtapi.h>
#include  <ws2tcpip.h>
#include  <iphlpapi.h>

#define MAX_BUFFER_SIZE 1024
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "Wsnmp32.lib")
#pragma comment(lib, "Snmpapi.lib")
#pragma comment(lib, "Mgmtapi.lib")

typedef struct _tIPV4HDR
{
	UCHAR ucIPHeaderLen : 4; // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
	UCHAR ucIPVersion : 4; // 4-bit IPv4 version
	UCHAR ucIPTos; // IP type of service
	USHORT usIPTotalLength; // Total length
	USHORT usIPID; // Unique identifier
	UCHAR ucIPFragOffset : 5; // Fragment offset field
	UCHAR ucIPMoreFragment : 1;
	UCHAR ucIPDontFragment : 1;
	UCHAR ucIPReservedZero : 1;
	UCHAR ucIPFragOffset1; //fragment offset
	UCHAR ucIPTTL; // Time to live
	UCHAR ucIPProtocol; // Protocol(TCP,UDP etc)
	USHORT usIPChecksum; // IP checksum
	UINT unSrcaddress; // Source address
	UINT unDestaddress; // Source address
} IPV4_HDR;

typedef struct _tUDPHDR
{
	USHORT usSourcePort; // Source port no.
	USHORT usDestPort; // Dest. port no.
	USHORT usUDPLength; // Udp packet length
	USHORT usUDPChecksum; // Udp checksum (optional)
} UDP_HDR;

// TCP header
typedef struct _tTCPHDR
{
	USHORT usSourcePort; // source port
	USHORT usDestPort; // destination port
	UINT unSequence; // sequence number - 32 bits
	UINT unAcknowledge; // acknowledgement number - 32 bits
	UCHAR ucNS : 1; //Nonce Sum Flag Added in RFC 3540.
	UCHAR ucReservedPart1 : 3; //according to rfc
	UCHAR ucDataOffset : 4; /*The number of 32-bit words in the TCP header.	This indicates where the data begins.The length of the TCP header is always a multiple	of 32 bits.*/
	UCHAR ucFIN : 1; //Finish Flag
	UCHAR ucSYN : 1; //Synchronise Flag
	UCHAR ucRST : 1; //Reset Flag
	UCHAR ucPSH : 1; //Push Flag
	UCHAR ucACK : 1; //Acknowledgement Flag
	UCHAR ucURG : 1; //Urgent Flag
	UCHAR ucECN : 1; //ECN-Echo Flag
	UCHAR ucCWR : 1; //Congestion Window Reduced Flag
	USHORT usWindow; // window
	USHORT usChecksum; // checksum
	USHORT usUrgentPointer; // urgent pointer
} TCP_HDR;

typedef struct _tICMPHDR
{
	BYTE byType; // ICMP Error type
	BYTE byCode; // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
} ICMP_HDR;

class  ISocket
{
public:
	virtual SOCKET GetSocket() = 0;
	virtual void   SetClientAddr(struct sockaddr addr) = 0;
	virtual const char* GetIP() = 0;
	virtual const char* GetHostName() = 0;
	virtual const char* Receive() = 0;
	virtual void   Send(char*) = 0;
};

typedef void (*CallbackLocalAreaListener)(const char* , const char*, const char*, bool );
typedef void (*FuncNewConnection)(void*);
typedef void (*FuncFindOpenPort)(char* , int, bool, int);

extern "C" ENZTCPLIBRARY_API	HANDLE		OpenServer(const char * sport, FuncNewConnection);
extern "C" ENZTCPLIBRARY_API	void		RunServer(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		CloseServer(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		CloseClientConnection(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		EnumOpenPorts(char* ipAddress, int nNumPorts, FuncFindOpenPort);
extern "C" ENZTCPLIBRARY_API	void		StopSearchingOpenPorts();
extern "C" ENZTCPLIBRARY_API	bool		IsPortOpen(char* ipAddress, int nNumPorts, int* pnLastError);
extern "C" ENZTCPLIBRARY_API	void        StartLocalAreaListening(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int nPollingTimeMS);
extern "C" ENZTCPLIBRARY_API	void		StopLocalAreaListening();
extern "C" ENZTCPLIBRARY_API	bool		StartSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD & dwLastError);
extern "C" ENZTCPLIBRARY_API	smiVALUE	SNMPGet(const char* szOID, DWORD & dwLastError);
extern "C" ENZTCPLIBRARY_API	void		EndSNMP();
extern "C" ENZTCPLIBRARY_API	bool        GetDefaultGateway(char* szDefaultIPAddress);
extern "C" ENZTCPLIBRARY_API	HANDLE		ConnectToServer(const char* ipAddress, const char* portNum, int* pnlastError);
extern "C" ENZTCPLIBRARY_API	void		DisconnectFromServer(HANDLE hHandle);