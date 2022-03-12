#pragma once

#include "EnzTCP.h"
#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <string>
#include <format>

/* ICMP types */
#define ICMP_ECHOREPLY 0 /* ICMP type: echo reply */
#define ICMP_ECHO 8   /* ICMP type: echo request */
#define SIZEOF_ICMP_ERROR 8
#define SIZEOF_IO_STATUS_BLOCK 8
#define PING_TIMEOUT 1000
#define ICMP_MIN 8 // Minimum 8-byte ICMP packet (header)
#define MAX_PACKET       65536
typedef struct _tICMP_OPTIONS
{
    BYTE Ttl;
    BYTE Tos;
    BYTE Flags;
    BYTE OptionsSize;
    HANDLE OptionsData;
}ICMP_OPTIONS;


#pragma comment(lib, "iphlpapi.lib")
using namespace std;

class CICMP
{
private:
	WSADATA m_wsaData;
	string m_HostName;
	string m_HostIP;

	bool Ping(HANDLE hIcmpFile, string sSrc, string sDest, IPAddr &, UCHAR);
	void FillICMPData(char* icmp_data, int datasize);
	USHORT CheckSum(USHORT* buffer, int size);
	bool DecodeICMPHeader(USHORT usSeq, char* buf, int bytes, struct sockaddr_in* from);
public:
	CICMP();
	~CICMP();

	bool CheckDevice(string ipAddress, string& hostname, string& sMacAddress);
	bool CheckDevice(string ipAddress, string& hostname, string& sMacAddress, DWORD* pError);
	bool CheckDeviceEx(string ipAddress, string& hostname, string& sMacAddress);
	string GetHostName(string ipAddress);
	int InitializeLocalIPAndHostname(const char*);

};

