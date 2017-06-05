/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMutex.cpp
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月05日
***************************************************************************************************/
#include "MvrExport.h"
#include "MvrMutex.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrThread.h"

#include <stdio.h>
#include <stdarg.h>

unsigned int MvrMutex::ourLockWarningMS = 0;
unsigned int MvrMutex::ourUnlockWarningMS = 0;
MvrFunctor *MvrMutex::ourNonRecursiveDeadlockFunctor = NULL;

MVREXPORT void MvrMutex::setLogNameVar(const char *logName, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, logName);
  vsnprintf(arg, sizeof(argt), logName, ptr);
  arg[sizeof(arg)-1] = '\0';
  va_end(ptr);
  return setLogName(arg);
}

MVREXPORT MvrMutex::initLockTiming(void)
{
  myFirstLock = true;
  myLockTime  = new MvrTime;
  myLockStarted = new MvrTime;
}

void MvrMutex::uninitLockTiming(void)
{
  delete myLockTime;
  myLockTime = 0;
  delete myLockStarted;
  myLockStarted = 0;
}

void MvrMutex::startLockTimer()
{
  if (ourLockWarningMS > 0)
    myLockStarted->setToNow();
}
void MvrMutex::checkLockTime(void)
{
  if (ourLockWarningMS > 0 && myLockStarted &&
      myLockStarted->mSecSince() >= ourLockWarningMS){
    MvrLog::logNoLock(MvrLog::Normal,
                      "LockWarning: locking '%s' from thread '%s' %d pid %d took %.3f sec",
                      myLogName.c_str(), MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
#ifdef WNI32
                      0,
#else
                      getpid(),
#endif
                      myLockStarted->mSecSince() / 1000.0);
  }
}

void MvrMutex::startUnlockTimer()
{
  if (ourUnlockWarningMS > 0)
  {
    myLockTime->setToNow();
    myFirstLock = false;
  }
}

void MvrMutex::checkUnlockTime()
{
  if (ourLockWarningMS > 0 && myLockStarted && myLockTime &&
      myLockTime->mSecSince() >= ourUnlockWarningMS){
    MvrLog::logNoLock(MvrLog::Normal,
                    	"LockWarning: unlocking '%s' from thread ('%s' %d pid %d) was locked for %.3f sec", 
                      myLogName.c_str(), 
                      MvrThread::getThisThreadName(), 
                      MvrThread::getThisThread(), 
#ifdef WNI32
                      0,
#else
                      getpid(),
#endif
                      myLockTime->mSecSince() / 1000.0);
  }
}