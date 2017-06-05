/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrThread.h
 > Description  : POSIX/WIN32 thread wrapper class. 
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月05日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrThread.h"

#include <errno.h>
#include <list>

MvrMutex MvrThread::ourThreadsMutex;
MvrThread::MapType MvrThread::ourThreads;
#if defined(WIN32) && !defined(MINGW)
std::map<HANDLE, MvrThread *> MvrThread::ourThreadHandles;
#endif  // WIN32 
MVREXPORT MvrLog::LogLevel MvrThread::ourLogLevel = MvrLog::Verbose;
// TODO, instead of AREXPORT move accessors into .cpp
std::string MvrThread::ourUnknownThreadName = "unknown";

MVREXPORT void MvrThread::stopAll()
{
  MapType::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
  {
    (*iter).second->stopRunning();
  }
  ourThreadsMutex.unlock();
}

