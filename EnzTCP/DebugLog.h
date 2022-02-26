#pragma once
#include <string>
#include <time.h>
#include <fstream>
#include <mutex>
using namespace std;
static mutex g_mutxLog;
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
inline void DEBUG_LOG(string logMsg) {

    
    DWORD value = 0;
    DWORD BufferSize = 4;

    RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Enzo Network Monitoring Tool", "DebugLog", /*RRF_RT_ANY*/RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
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
}