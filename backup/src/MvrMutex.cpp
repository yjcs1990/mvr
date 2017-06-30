/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMutex.cpp
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月13日
***************************************************************************************************/
#define _XOPEN_SOURCE 500
#include "MvrExport.h"
#include "MvrMutex.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrThread.h"
#include "MvrLog.h"
#include "mvriaInternal.h"
#include "MvrFunctor.h"

#include <sys/types.h>
#include <unistd.h>   // for getpid();
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

unsigned int MvrMutex::ourLockWarningMS = 0;
unsigned int MvrMutex::ourUnlockWarningMS = 0;
MvrFunctor *MvrMutex::ourNonRecursiveDeadlockFunctor = NULL;


MvrMutex::MvrMutex(bool recursive) :
          myFailedInit(false),
          myMutex(),
          myLog(false),
          myLogName("(unnamed)")
{
  initLockTiming();

  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  if (pthread_mutex_init(&myMutex, &attr) != 0)
  {
    myFailedInit = true;
    MvrLog::logNoLock(MvrLog::Terse,
                      "MvrMutex::MvrMutex: Failed to initialize mutex.");
  }

  pthread_mutexattr_destroy(&attr);

  myNonRecursive = !recursive;
  myWasAlreadyLocked = false;

  myStrMap[STATUS_FAILED_INIT]    ="Failed to initialize";
  myStrMap[STATUS_FAILED]         ="General failure";
  myStrMap[STATUS_ALREADY_LOCKED] ="Mutex already locked";
}          

MvrMutex::MvrMutex(const MvrMutex &mutex)
{
  myLog = mutex.myLog;
  if (pthread_mutex_init(&myMutex, 0) != 0)
  {
    myFailedInit = true;
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::MvrMutex: Failed to initialize mutex");
  }
  else
    unlock();
  myLogName = mutex.myLogName;
  initLockTiming();

  myStrMap[STATUS_FAILED_INIT]    ="Failed to initialize";
  myStrMap[STATUS_FAILED]         ="General failure";
  myStrMap[STATUS_ALREADY_LOCKED] ="Mutex already locked";
}

MvrMutex::~MvrMutex()
{
  int ret;
  if (!myFailedInit && (ret = pthread_mutex_destroy(&myMutex)) != 0)
  {
    if (ret == EBUSY)
    {
      MvrLog::logNoLock(MvrLog::Verbose,
                        "MvrMutex::~MvrMutex: Failed to destroy mutex %s. A thread is currently blocked waiting for this mutex.",
                        myLogName.c_str());
    }
    else
    {
      MvrLog::logNoLock(MvrLog::Verbose,
                        "MvrMutex::~MvrMutex: Failed to destroy mutex %s. Unknown error.",
                        myLogName.c_str());      
    }
  }
  uninitLockTiming();
}

/*
 * Lock the mutex. This function will block until no other thread has this
 * mutex locked. If it returns 0, then it obtained the lock and the thread
 * is free to use the critical section that this mutex protects. Else it
 * returns an error code. See getError().
 */
int MvrMutex::lock()
{
  if (ourLockWarningMS > 0)
    startLockTimer();
  if (myLog)
    MvrLog::logNoLock(MvrLog::Terse, 
                      "Locking '%s' from thread '%s' %d pid %d",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::lock: Initialization of mutex '%s' form thread ('%s' %d pid %d) failed, failed lock",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::lock: Initialization of mutex failed , faild lock");
    return STATUS_FAILED_INIT;
  }                      

  int ret;
  if ((ret = pthread_mutex_lock(&myMutex)) != 0)
  {
    if (ret == EDEADLK)
    {
      MvrLog::logNoLock(MvrLog::Terse, 
                        "MvrMutex::lock: Trying to lock mutex '%s' form thread ('%s' %d pid %d) which is already locked by this thread",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid());    
      return STATUS_ALREADY_LOCKED;       
    }
    else
    {
      MvrLog::logNoLock(MvrLog::Terse, 
                        "MvrMutex::lock: Failed to lock mutex '%s' form thread ('%s' %d pid %d) due to an unknown error",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid());    
      return STATUS_FAILED;      
    }
  }
  
  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
      MvrLog::logNoLock(MvrLog::Terse, 
                        "MvrMutex: '%s' tried to lock recursively even through it is nonrecursive, from thread '%s' %d pid %d, invoking functor '%s'",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid(),
                        ourNonRecursiveDeadlockFunctor->getName());
      MvrLog::logBacktrace(MvrLog::Normal);
      ourNonRecursiveDeadlockFunctor->invoke();
      exit(255);                                                  
      }
      else
      {
        MvrLog::logNoLock(MvrLog::Terse,
                          "MvrMutex: '%s' tried to lock recursively even through it is nonrecursive, from thread '%s' %d pid %d, calling Mvria::shutdown",
                          myLogName.c_str(),
                          MvrThread::getThisThreadName(),
                          MvrThread::getThisOSThread(),
                          getpid());
        MvrLog::logBacktrace(MvrLog::Normal);
        Mvria::shutdown();
        exit(255);
      }
    }
    myWasAlreadyLocked = true;
  }
  if (ourLockWarningMS > 0)
    checkLockTime();
  if (ourUnlockWarningMS > 0)
    startUnlockTimer();
  
  return 0;
}

/**
   Try to lock the mutex. This function will not block if another thread has
   the mutex locked. It will return instantly if that is the case. It will
   return STATUS_ALREADY_LOCKED if another thread has the mutex locked. If it
   obtains the lock, it will return 0.
*/

int MvrMutex::tryLock()
{
  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::tryLock: Initialization of mutex '%s' form thread ('%s' %d pid %d) failed, failed lock",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::lock: Initialization of mutex failed , faild lock"); 
    return STATUS_FAILED_INIT;
  }  
  if (myLog)
    MvrLog::logNoLock(MvrLog::Terse, 
                      "Try locking '%s' from thread '%s' %d pid %d",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
  int ret;
  if ((ret = pthread_mutex_trylock(&myMutex)) != 0)
  {
    if (ret == EBUSY)
    {
      MvrLog::logNoLock(MvrLog::Terse,
                        "MvrMutex::tryLock: Mutex %s is already locked", 
                        myLogName.c_str());
      return STATUS_ALREADY_LOCKED;
    }
    else
    {
      MvrLog::logNoLock(MvrLog::Terse,
                        "MvrMutex::tryLock: Failed to trylock a mutex ('%s') from thread ('%s' %d pid %d) due to an unknown error",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid());
      return STATUS_FAILED;                          
    } 
  }                     
  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
      MvrLog::logNoLock(MvrLog::Terse, 
                        "MvrMutex: '%s' tried to lock recursively even through it is nonrecursive, from thread '%s' %d pid %d, invoking functor '%s'",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid(),
                        ourNonRecursiveDeadlockFunctor->getName());
      MvrLog::logBacktrace(MvrLog::Normal);
      ourNonRecursiveDeadlockFunctor->invoke();
      exit(255);                                                  
      }
      else
      {
        MvrLog::logNoLock(MvrLog::Terse,
                          "MvrMutex: '%s' tried to lock recursively even through it is nonrecursive, from thread '%s' %d pid %d, calling Mvria::shutdown",
                          myLogName.c_str(),
                          MvrThread::getThisThreadName(),
                          MvrThread::getThisOSThread(),
                          getpid());
        MvrLog::logBacktrace(MvrLog::Normal);
        Mvria::shutdown();
        exit(255);
      }
    }
    myWasAlreadyLocked = true;
  }
  if (myLog)
  {
    MvrLog::logNoLock(MvrLog::Terse,
                      "Try locked '%s' from thread '%s' %d pid %d",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
  }
  return 0;
}

int MvrMutex::unlock()
{
  if (myLog)
  {
    MvrLog::logNoLock(MvrLog::Terse,
                      "Unlocking '%s' from thread '%s' %d pid %d",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
  }
  if (ourUnlockWarningMS > 0)
    checkUnlockTime();
  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::unlock: Initialization of mutex '%s' form thread ('%s' %d pid %d) failed, failed unlock",
                      myLogName.c_str(),
                      MvrThread::getThisThreadName(),
                      MvrThread::getThisThread(),
                      getpid());
    MvrLog::logNoLock(MvrLog::Terse, 
                      "MvrMutex::lock: Initialization of mutex failed , faild lock"); 
    return STATUS_FAILED_INIT;    
  }
  int ret;
  if ((ret = pthread_mutex_unlock(&myMutex)) != 0)
  {
    if (ret == EPERM)
    {
      MvrLog::logNoLock(MvrLog::Terse,
                        "MvrMutex::unLock: Trying to unlock a mutex ('%s') which this thread ('%s' %d pid %d) does not own", 
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid());
      return STATUS_ALREADY_LOCKED;
    }
    else
    {
      MvrLog::logNoLock(MvrLog::Terse,
                        "MvrMutex::unLock: Failed to trylock a mutex ('%s') from thread ('%s' %d pid %d) due to an unknown error",
                        myLogName.c_str(),
                        MvrThread::getThisThreadName(),
                        MvrThread::getThisThread(),
                        getpid());
      return STATUS_FAILED;                          
    } 
  }  
  if (myNonRecursive)
    myWasAlreadyLocked = false;
  return 0;                   
}

MVREXPORT const char *MvrMutex::getError(int messageNumber) const
{
  MvrStrMap::const_iterator it;
  if((it = myStrMap.find(messageNumber)) != myStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}

MVREXPORT void MvrMutex::setLogNameVar(const char *logName, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, logName);
  vsnprintf(arg, sizeof(arg), logName, ptr);
  arg[sizeof(arg)-1] = '\0';
  va_end(ptr);
  return setLogName(arg);
}

void MvrMutex::initLockTiming(void)
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