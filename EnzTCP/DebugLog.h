#pragma once
#include <string>
#include <time.h>
#include <fstream>
#include <mutex>
using namespace std;
static mutex g_mutxLog;

#ifndef UNICODE
inline string getCurrentDateTime(string s) 
{
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];

    memset(buf, 0, sizeof(buf));
    localtime_s(&tstruct ,&now);
    if (s == "now")
        strftime(buf, sizeof(buf)-1, "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf)-1, "%Y-%m-%d", &tstruct);
    return string(buf);
};
/*inline void DEBUG_LOG(string logMsg) {

    
    DWORD value = 0;
    DWORD BufferSize = 4;

    RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Enzo Network Monitoring Tool", "DebugLog", RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
    if (value)
    {
        g_mutxLog.lock();
        string filePath = getCurrentDateTime("date") + ".txt";
        string now = getCurrentDateTime("now");
        ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << '\t' << logMsg << '\n';
        ofs.close();
        g_mutxLog.unlock();
    }
}*/

inline void DEBUG_LOG(string logMsg, ...)
{
    DWORD value = 0;
    DWORD BufferSize = 4;

    RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Enzo Network Monitoring Tool", "DebugLog", /*RRF_RT_ANY*/RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
    if (value)
    {
        g_mutxLog.lock();
        static char loc_buf[64];
        char* temp = loc_buf;
        int len;
        va_list arg;
        va_list copy;
        va_start(arg, logMsg);
        va_copy(copy, arg);
        len = vsnprintf(NULL, 0, logMsg.c_str(), arg);
        va_end(copy);
        if (len >= sizeof(loc_buf)) {
            temp = (char*)malloc(len + 1);
            if (temp == NULL) {
                g_mutxLog.unlock();
                return;
            }
        }
        vsnprintf(temp, len + 1, logMsg.c_str(), arg);
        //printf(temp); // replace with any print function you want
        logMsg = temp;
        string filePath = getCurrentDateTime("date") + ".txt";
        string now = getCurrentDateTime("now");
        ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << '\t' << logMsg << '\n';
        ofs.close();
        va_end(arg);
        if (len >= sizeof(loc_buf)) {
            free(temp);
        }
        g_mutxLog.unlock();
    }
}
#else
inline wstring getCurrentDateTime(wstring s)
{
    time_t now = time(0);
    struct tm  tstruct;
    wchar_t  buf[80];

    memset(buf, 0, sizeof(buf));
    localtime_s(&tstruct, &now);
    if (s == L"now")
        wcsftime(buf, sizeof(buf) - 1, L"%Y-%m-%d %X", &tstruct);
    else if (s == L"date")
        wcsftime(buf, sizeof(buf) - 1, L"%Y-%m-%d", &tstruct);
    return wstring(buf);
};
/*inline void DEBUG_LOG(wstring logMsg) {


    DWORD value = 0;
    DWORD BufferSize = 4;

    RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Enzo Network Monitoring Tool", L"DebugLog", RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
    if (value)
    {
        g_mutxLog.lock();
        wstring filePath = getCurrentDateTime(L"date") + L".txt";
        wstring now = getCurrentDateTime(L"now");
        wofstream  ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << L'\t' << logMsg << L'\n';
        ofs.close();
        g_mutxLog.unlock();
    }
}*/

inline void DEBUG_LOG(wstring logMsg, ...)
{
    DWORD value = 0;
    DWORD BufferSize = 4;

    RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Enzo Network Monitoring Tool", L"DebugLog", /*RRF_RT_ANY*/RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
    if (value)
    {
        g_mutxLog.lock();
        static wchar_t loc_buf[64];
        wchar_t* temp = loc_buf;
        int len;
        va_list arg;
        va_list copy;
        va_start(arg, logMsg);
        va_copy(copy, arg);
        len = _vsnwprintf(NULL, 0, logMsg.c_str(), arg);
        va_end(copy);
        if (len >= (sizeof(loc_buf)/sizeof(wchar_t))) {
            temp = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
            if (temp == NULL) {
                g_mutxLog.unlock();
                return;
            }
            memset(temp, 0, sizeof(wchar_t) * (len + 1));
        }
        _vsnwprintf(temp, sizeof(wchar_t) * (len), logMsg.c_str(), arg);
        logMsg = temp;
        wstring filePath = getCurrentDateTime(L"date") + L".txt";
        wstring now = getCurrentDateTime(L"now");
     // string filepathA = CW2A(filePath.c_str());
        wofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << L'\t' << logMsg << L'\n';
        ofs.close();
        va_end(arg);
        if (len >= sizeof(loc_buf)) {
            free(temp);
        }
        g_mutxLog.unlock();
    }
}
#endif