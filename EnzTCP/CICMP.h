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


#pragma comment(lib, "iphlpapi.lib")
using namespace std;

class CICMP
{
private:
	WSADATA m_wsaData;
public:
	CICMP();
	~CICMP();

	bool CheckDevice(string ipAddress, string& hostname, string& sMacAddress);
	string GetHostName(string ipAddress);
};

