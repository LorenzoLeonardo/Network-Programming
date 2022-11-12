
// CheckOpenPorst.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h" // main symbols

// CCheckOpenPortsApp:
// See CheckOpenPorst.cpp for the implementation of this class
//

class CCheckOpenPortsApp : public CWinApp {
public:
  CCheckOpenPortsApp();

  // Overrides
public:
  virtual BOOL InitInstance();

  // Implementation

  DECLARE_MESSAGE_MAP()
private:
  void ElevateProcess();
  BOOL IsAdministrator();
  int ProcessAppToFirewall(LPCTSTR szAppName);
};

extern CCheckOpenPortsApp theApp;
