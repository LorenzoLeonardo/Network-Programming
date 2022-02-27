#pragma once

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
    ULONGLONG m_ullDownloadStartTime;
    ULONGLONG m_ullUploadStartTime;
    HANDLE m_hPacketListenerDownload;
    HANDLE m_hPacketListenerUpload;

    CDeviceConnected();
    ~CDeviceConnected();
    CDeviceConnected operator=(const CDeviceConnected& b);
};


