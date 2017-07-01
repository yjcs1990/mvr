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
  vsnprintf(arg, sizeof(arg), logName, ptr);
  arg[sizeof(arg) - 1] = '\0';
  va_end(ptr);
  return setLogName(arg);
}

void MvrMutex::initLockTiming()
{
  myFirstLock = true;
  myLockTime = new MvrTime;
  myLockStarted = new MvrTime;
}

void MvrMutex::uninitLockTiming()
{
  delete myLockTime;
  myLockTime = 0;
  delete myLockStarted;
  myLockStarted = 0;
}


void MvrMutex::startLockTimer() 
{
  if(ourLockWarningMS > 0)
    myLockStarted->setToNow();
}

void MvrMutex::checkLockTime() 
{
	//printf("ourLockWarningMS=%d, myLockStarted->mSecSince=%d\n", ourLockWarningMS, myLockStarted->mSecSince());
  if (ourLockWarningMS > 0 && myLockStarted &&
        myLockStarted->mSecSince() >= ourLockWarningMS)
    MvrLog::logNoLock(
   	  MvrLog::Normal, 
   	  "LockWarning: locking '%s' from thread '%s' %d pid %d took %.3f sec", 
      myLogName.c_str(), MvrThread::getThisThreadName(),
   	  MvrThread::getThisThread(), 
#ifdef WIN32
	  0,
#else
	  getpid(), 
#endif
   	  myLockStarted->mSecSince() / 1000.0);

}

void MvrMutex::startUnlockTimer() 
{
  if (ourUnlockWarningMS > 0)
  {
    myLockTime->setToNow();
    myFirstLock = false;
  }
}

void MvrMutex::checkUnlockTime() {
	//printf("checking unlock time: warningms=%d, myFirstLock=%d, msecSince=%d\n", ourUnlockWarningMS, myFirstLock, myLockTime->mSecSince());
  if (ourUnlockWarningMS > 0 && !myFirstLock &&  myLockTime &&
        myLockTime->mSecSince() >= ourUnlockWarningMS)
    MvrLog::logNoLock(MvrLog::Normal, 
		    "LockWarning: unlocking '%s' from thread ('%s' %d pid %d) was locked for %.3f sec", 
		    myLogName.c_str(), 
		    MvrThread::getThisThreadName(), 
		    MvrThread::getThisThread(), 
#ifdef WIN32
			0,
#else
			getpid(),
#endif
		    myLockTime->mSecSince() / 1000.0);
}
