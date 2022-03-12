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

#define MAX_PACKET_SIZE 65536
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
#define ICMP_PROTOCOL 1 
#define IGMP_PROTOCOL 2
#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define MAX_PORT 65535
#define POLLING_TIME 500

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "Wsnmp32.lib")
#pragma comment(lib, "Snmpapi.lib")
#pragma comment(lib, "Mgmtapi.lib")

#pragma pack(push, 1)
typedef struct _tIPV4HDR
{
	UCHAR ucIPHeaderLen : 4;
	UCHAR ucIPVersion : 4;
	UCHAR ucIPTos;
	USHORT usIPTotalLength; 
	USHORT usIPID; 
	USHORT usFragAndFlags;
	UCHAR ucIPTTL; 
	UCHAR ucIPProtocol; 
	USHORT usIPChecksum;
	union
	{
		UINT unSrcaddress;
		UCHAR ucSrcaddress[4];
	};
	union
	{
		UINT unDestaddress;
		UCHAR ucDestaddress[4];
	};

} IPV4_HDR;

// TCP header
typedef struct _tTCPHDR
{
	USHORT usSourcePort;
	USHORT usDestPort;
	UINT unSequence;
	UINT unAcknowledge;
	UCHAR ucNS : 1;
	UCHAR ucReservedPart1 : 3;
	UCHAR ucDataOffset : 4;
	UCHAR ucFIN : 1;
	UCHAR ucSYN : 1;
	UCHAR ucRST : 1;
	UCHAR ucPSH : 1;
	UCHAR ucACK : 1;
	UCHAR ucURG : 1;
	UCHAR ucECN : 1;
	UCHAR ucCWR : 1;
	USHORT usWindow;
	USHORT usChecksum;
	USHORT usUrgentPointer;
} TCP_HDR;

//UDP Header
typedef struct _tUDPHDR
{
	USHORT usSourcePort; 
	USHORT usDestPort; 
	USHORT usUDPLength; 
	USHORT usUDPChecksum;
} UDP_HDR;

//ICMP Header
typedef struct _tICMPHDR
{
	BYTE byType; 
	BYTE byCode; 
	USHORT usChecksum;
	USHORT usID;
	USHORT usSeq;
	ULONG  ulTimeStamp;
} ICMP_HDR;

#pragma pack(pop)

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

typedef void (*FNCallbackLocalAreaListener)(const char* , const char*, const char*, bool );
typedef bool (*FNCallbackPacketListener)(unsigned char* buffer, int nSize);
typedef bool (*FNCallbackPacketListenerEx)(unsigned char* buffer, int nSize, void* pObject);
typedef void (*FNCallbackNewConnection)(void*);
typedef void (*FNCallbackFindOpenPort)(char* , int, bool, int);
typedef void (*FNCallbackAdapterList)(void*);

extern "C" ENZTCPLIBRARY_API	HANDLE		OpenServer(const char * sport, FNCallbackNewConnection);
extern "C" ENZTCPLIBRARY_API	void		RunServer(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		CloseServer(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		CloseClientConnection(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		EnumOpenPorts(char* ipAddress, int nNumPorts, FNCallbackFindOpenPort);
extern "C" ENZTCPLIBRARY_API	void		StopSearchingOpenPorts();
extern "C" ENZTCPLIBRARY_API	bool		IsPortOpen(char* ipAddress, int nNumPorts, int* pnLastError);
extern "C" ENZTCPLIBRARY_API	bool        StartLocalAreaListening(const char* ipAddress,const char* subNetMask, FNCallbackLocalAreaListener fnpPtr, int nPollingTimeMS);
extern "C" ENZTCPLIBRARY_API	void		StopLocalAreaListening();
extern "C" ENZTCPLIBRARY_API	bool		StartSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD & dwLastError);
extern "C" ENZTCPLIBRARY_API	smiVALUE	SNMPGet(const char* szOID, DWORD & dwLastError);
extern "C" ENZTCPLIBRARY_API	void		EndSNMP();
extern "C" ENZTCPLIBRARY_API	bool        GetDefaultGateway(char* szDefaultIPAddress);
extern "C" ENZTCPLIBRARY_API	bool        GetDefaultGatewayEx(const char* szAdapterName, char* szDefaultGateway, int nSize);
extern "C" ENZTCPLIBRARY_API	HANDLE		ConnectToServer(const char* ipAddress, const char* portNum, int* pnlastError);
extern "C" ENZTCPLIBRARY_API	void		DisconnectFromServer(HANDLE hHandle);
extern "C" ENZTCPLIBRARY_API	bool		StartPacketListener(FNCallbackPacketListener fnpPtr);
extern "C" ENZTCPLIBRARY_API	void		StopPacketListener();
extern "C" ENZTCPLIBRARY_API	bool		GetNetworkDeviceStatus(const char* ipAddress, char* hostname, int nSizeHostName, char* macAddress, int nSizeMacAddress, DWORD * pError);
extern "C" ENZTCPLIBRARY_API	bool		EnumNetworkAdapters(FNCallbackAdapterList);
extern "C" ENZTCPLIBRARY_API    HANDLE      CreatePacketListenerEx(FNCallbackPacketListenerEx fnpPtr, void* pObject);
extern "C" ENZTCPLIBRARY_API	bool		StartPacketListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		StopPacketListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API	void		WaitPacketListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API    void        DeletePacketListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API    HANDLE      CreateLocalAreaListenerEx();
extern "C" ENZTCPLIBRARY_API	bool		StartLocalAreaListenerEx(HANDLE, const char* szStartingIPAddress, const char* subNetMask, FNCallbackLocalAreaListener pFncPtr, int nPollingTimeMS);
extern "C" ENZTCPLIBRARY_API	void		StopLocalAreaListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API    void        DeleteLocalAreaListenerEx(HANDLE);
extern "C" ENZTCPLIBRARY_API    void        SetNICAdapterToUse(const char* szAdapterName, ULONG ipAddress);