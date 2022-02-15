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
#define ICMP_ECHOREQ 8   /* ICMP type: echo request */
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11


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
	bool Ping(HANDLE hIcmpFile, string sSrc, string sDest, IPAddr &, int);
public:
	CICMP();
	~CICMP();

	bool CheckDevice(string ipAddress, string& hostname, string& sMacAddress);
	string GetHostName(string ipAddress);
	int InitializeLocalIPAndHostname();
};

