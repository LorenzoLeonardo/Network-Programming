#pragma once

#define WM_UPDATE_DOWNSPEED WM_USER+100
#define WM_UPDATE_UPSPEED WM_USER+101

#define POLLING_TIME 1000

class CDeviceConnected
{
public:
    ULONG m_ulDataSizeDownload;
    ULONG m_ulDataSizeUpload;
    double m_lfDownloadSpeed;
    double m_lfUploadSpeed;
    double m_lfMaxDownloadSpeed;
    double m_lfMaxUploadSpeed;
    CString m_szHostName;
    CString m_szIPAddress;
    CString m_szMACAddress;


    CDeviceConnected();
    CDeviceConnected(HWND hWnd);
    ~CDeviceConnected();
    CDeviceConnected operator=(const CDeviceConnected& b)
    {
        this->m_ulDataSizeDownload = b.m_ulDataSizeDownload;
        this->m_ulDataSizeUpload = b.m_ulDataSizeUpload;
        this->m_lfDownloadSpeed = b.m_lfDownloadSpeed;
        this->m_lfUploadSpeed = b.m_lfUploadSpeed;
        this->m_lfMaxDownloadSpeed = b.m_lfMaxDownloadSpeed;
        this->m_lfMaxUploadSpeed = b.m_lfMaxUploadSpeed;
        this->m_szHostName = b.m_szHostName;
        this->m_szIPAddress = b.m_szIPAddress;
        this->m_szMACAddress = b.m_szMACAddress;
#if 0
        this->m_StopThreadDownload = b.m_StopThreadDownload;
        this->m_WaitThreadDownload = b.m_WaitThreadDownload;
        this->m_StopThreadUpload = b.m_StopThreadUpload;
        this->m_WaitThreadUpload = b.m_WaitThreadUpload;
        this->m_hDownload = b.m_hDownload;
        this->m_hUpload = b.m_hUpload;
        this->m_hwndMainWnd = b.m_hwndMainWnd;
#endif // 0

        return *this;
    }
    void Suspend();
    void Resume();
    void Run();
    void Stop();

 
private:

    HANDLE m_StopThreadDownload;//For event
    HANDLE m_WaitThreadDownload;//For event
    HANDLE m_StopThreadUpload;//For event
    HANDLE m_WaitThreadUpload;//For event
    HANDLE m_hDownload;
    HANDLE m_hUpload;
    HWND   m_hwndMainWnd;
    static unsigned __stdcall ThreadDownloadSpeedReader(LPVOID pvParam);
    static unsigned __stdcall ThreadUploadSpeedReader(LPVOID pvParam);
};


