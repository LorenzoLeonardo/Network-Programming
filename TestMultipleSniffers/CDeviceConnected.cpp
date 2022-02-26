#include "pch.h"
#include "CDeviceConnected.h"
#include <mutex>

std::mutex mtxdown;
std::mutex mtxup;
CDeviceConnected::CDeviceConnected()
{
    m_hwndMainWnd = NULL;
    m_ulDataSizeDownload = 0;
    m_ulDataSizeUpload = 0;
    m_lfDownloadSpeed = 0;
    m_lfUploadSpeed = 0;
    m_lfMaxDownloadSpeed = 0;
    m_lfMaxUploadSpeed = 0;
    m_szHostName = _T("");
    m_szIPAddress = _T("");
    m_szMACAddress = _T("");

    m_StopThreadDownload = NULL;
    m_WaitThreadDownload = NULL;
    m_StopThreadUpload = NULL;
    m_WaitThreadUpload = NULL;
    m_hDownload = NULL;
    m_hUpload = NULL;


}

CDeviceConnected::CDeviceConnected(HWND hWnd)
{
    m_hwndMainWnd = hWnd;
    m_ulDataSizeDownload = 0;
    m_ulDataSizeUpload = 0;
    m_lfDownloadSpeed = 0;
    m_lfUploadSpeed = 0;
    m_lfMaxDownloadSpeed = 0;
    m_lfMaxUploadSpeed = 0;
    m_szHostName = _T("");
    m_szIPAddress = _T("");
    m_szMACAddress = _T("");

    m_StopThreadDownload = NULL;
    m_WaitThreadDownload = NULL;
    m_StopThreadUpload = NULL;
    m_WaitThreadUpload = NULL;
    m_hDownload = NULL;
    m_hUpload = NULL;

  
}

CDeviceConnected::~CDeviceConnected()
{
   
}

unsigned __stdcall CDeviceConnected::ThreadDownloadSpeedReader(LPVOID pvParam)
{
    CDeviceConnected* pParent = (CDeviceConnected *) pvParam;
    ULONG prev = 0, current = 0;
    ULONGLONG timePrev = GetTickCount64(), timeCurrent;
    double fUpSpeed = 0;

    pParent->m_ulDataSizeDownload = 0;
    prev = pParent->m_ulDataSizeDownload;

    while (::WaitForSingleObject(pParent->m_StopThreadDownload, 0) != WAIT_OBJECT_0)
    {
        timeCurrent = GetTickCount64();
        current = pParent->m_ulDataSizeDownload;
        if ((timeCurrent - timePrev) >= POLLING_TIME)
        {
           
            pParent->m_lfDownloadSpeed = ((double)(current - prev) / (double)(timeCurrent - timePrev)) * 8;
            if (pParent->m_lfMaxDownloadSpeed < pParent->m_lfDownloadSpeed)
                pParent->m_lfMaxDownloadSpeed = pParent->m_lfDownloadSpeed;

            
            pParent->m_ulDataSizeDownload = 0;
            prev = pParent->m_ulDataSizeDownload;
            
            ::SendMessage(pParent->m_hwndMainWnd, WM_UPDATE_DOWNSPEED, (WPARAM)pvParam, 0);

            timePrev = GetTickCount64();
           
        }
        Sleep(POLLING_TIME);
    }
    ::SetEvent(pParent->m_WaitThreadDownload);
    return 0;
}

unsigned __stdcall CDeviceConnected::ThreadUploadSpeedReader(LPVOID pvParam)
{
    CDeviceConnected* pParent = (CDeviceConnected*)pvParam;
    ULONG prev = 0, current = 0;
    ULONGLONG timePrev = GetTickCount64(), timeCurrent;
    double fUpSpeed = 0;

    pParent->m_ulDataSizeUpload = 0;
    prev = pParent->m_ulDataSizeUpload;

    while (::WaitForSingleObject(pParent->m_StopThreadUpload, 0) != WAIT_OBJECT_0)
    {
         timeCurrent = GetTickCount64();
        current = pParent->m_ulDataSizeUpload;
        if ((timeCurrent - timePrev) >= POLLING_TIME)
        {
           
            pParent->m_lfUploadSpeed = ((double)(current - prev) / (double)(timeCurrent - timePrev)) * 8;
            if (pParent->m_lfMaxUploadSpeed < pParent->m_lfUploadSpeed)
                pParent->m_lfMaxUploadSpeed = pParent->m_lfUploadSpeed;
          
            pParent->m_ulDataSizeUpload = 0;
            prev = pParent->m_ulDataSizeUpload;
           
            ::SendMessage(pParent->m_hwndMainWnd, WM_UPDATE_UPSPEED, (WPARAM)pvParam, 0);
            timePrev = GetTickCount64();
     
        }
        Sleep(POLLING_TIME);
    }
    ::SetEvent(pParent->m_WaitThreadUpload);

    return 0;
}

void CDeviceConnected::Suspend()
{
    SuspendThread(m_hDownload);
    SuspendThread(m_hUpload);
}

void CDeviceConnected::Resume()
{
    ResumeThread(m_hDownload);
    ResumeThread(m_hUpload);
}
void CDeviceConnected::Run()
{
    m_StopThreadDownload = CreateEvent(0, TRUE, FALSE, 0);
    m_WaitThreadDownload = CreateEvent(0, TRUE, FALSE, 0);


    m_hDownload = (HANDLE)_beginthreadex(NULL, 0, ThreadDownloadSpeedReader, this, 0, 0);
    m_hUpload = (HANDLE)_beginthreadex(NULL, 0, ThreadUploadSpeedReader, this, 0, 0);
}
void CDeviceConnected::Stop()
{

    // Trigger thread to stop

    if (m_StopThreadDownload)
        ::SetEvent(m_StopThreadDownload);
    if (m_StopThreadUpload)
        ::SetEvent(m_StopThreadUpload);

    TerminateThread(m_StopThreadDownload, 0);
    TerminateThread(m_StopThreadUpload, 0);

    if (m_WaitThreadDownload)
    {
        WaitForSingleObject(m_WaitThreadDownload, INFINITE);
        ::CloseHandle(m_WaitThreadDownload);
    }
    if (m_WaitThreadUpload)
    {
        WaitForSingleObject(m_WaitThreadUpload, INFINITE);
        ::CloseHandle(m_WaitThreadUpload);
    }
    if (m_StopThreadDownload)
        ::CloseHandle(m_StopThreadDownload);
    if (m_StopThreadUpload)
        ::CloseHandle(m_StopThreadUpload);

    if (m_hDownload)
        ::CloseHandle(m_hDownload);

    if (m_hUpload)
        ::CloseHandle(m_hUpload);

}