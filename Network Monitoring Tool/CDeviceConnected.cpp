#include "pch.h"
#include "CDeviceConnected.h"
#include <mutex>

std::mutex mtxdown;
std::mutex mtxup;
CDeviceConnected::CDeviceConnected()
{
    m_ulDataSizeDownload = 0;
    m_ulDataSizeUpload = 0;
    m_lfDownloadSpeed = 0;
    m_lfUploadSpeed = 0;
    m_lfMaxDownloadSpeed = 0;
    m_lfMaxUploadSpeed = 0;
    m_szHostName = _T("");
    m_szIPAddress = _T("");
    m_szMACAddress = _T("");
    m_ullDownloadStartTime = 0;
    m_ullUploadStartTime = 0;
    m_hPacketListener = NULL;



}

CDeviceConnected::~CDeviceConnected()
{
   
}
CDeviceConnected CDeviceConnected::operator=(const CDeviceConnected& b)
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
    this->m_ullDownloadStartTime = b.m_ullDownloadStartTime;
    this->m_ullUploadStartTime = b.m_ullUploadStartTime;
    this->m_hPacketListener = b.m_hPacketListener;
    return *this;
}