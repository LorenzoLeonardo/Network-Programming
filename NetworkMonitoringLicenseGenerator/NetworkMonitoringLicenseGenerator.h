
// NetworkMonitoringLicenseGenerator.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNetworkMonitoringLicenseGeneratorApp:
// See NetworkMonitoringLicenseGenerator.cpp for the implementation of this class
//

class CNetworkMonitoringLicenseGeneratorApp : public CWinApp
{
public:
	CNetworkMonitoringLicenseGeneratorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNetworkMonitoringLicenseGeneratorApp theApp;
