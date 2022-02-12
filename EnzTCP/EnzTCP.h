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

#define MAX_BUFFER_SIZE 1024

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "Wsnmp32.lib")
#pragma comment(lib, "Snmpapi.lib")
#pragma comment(lib, "Mgmtapi.lib")

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