#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrMutex.h"
#include "MvrLog.h"
#include "MvrFunctor.h"
#include "MvrThread.h"
#include "mvriaInternal.h"
#include "MvrThread.h"

//#include <process.h> // for getpid()

MvrMutex::MvrMutex(bool recursive) :
  myFailedInit(false),
  myMutex(),
  // KMC TESTING myStrMap(),
  myLog(false),
  myLogName("(unnamed)"),
  myNonRecursive(!recursive),
  myWasAlreadyLocked(false),
  myFirstLock(true),
  myLockTime(NULL),
  myLockStarted(NULL)
{
  myMutex=CreateMutex(0, true, 0);
  if (!myMutex)
  {
    myFailedInit=true;
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::MvrMutex: Failed to initialize mutex %s", myLogName.c_str());
  }
  else
    unlock();

  /** KMC TESTING
  myStrMap[STATUS_FAILED_INIT]="Failed to initialize";
  myStrMap[STATUS_FAILED]="General failure";
  myStrMap[STATUS_ALREADY_LOCKED]="Mutex already locked";
  ***/

  initLockTiming();
}

MVREXPORT MvrMutex::MvrMutex(const MvrMutex &mutex) :
  myFailedInit(false),
  myMutex(),
  // KMC TESTING myStrMap(),
  myLog(mutex.myLog),
  myLogName(mutex.myLogName),
  myNonRecursive(mutex.myNonRecursive),
  myWasAlreadyLocked(false),
  myFirstLock(true),
  myLockTime(NULL),
  myLockStarted(NULL)
{
  myMutex = CreateMutex(0, true, 0);
  if(!myMutex)
  {
    myFailedInit=true;
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::MvrMutex: Failed to initialize mutex in copy of %s", myLogName.c_str());
  }
  else
    unlock();

  /*** KMC TESTING
  myStrMap[STATUS_FAILED_INIT]="Failed to initialize";
  myStrMap[STATUS_FAILED]="General failure";
  myStrMap[STATUS_ALREADY_LOCKED]="Mutex already locked";
  ***/

  initLockTiming();
}

MvrMutex::~MvrMutex()
{
  if (!myFailedInit && !CloseHandle(myMutex))
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::~MvrMutex: Failed to destroy mutex.");

  uninitLockTiming();
}

int MvrMutex::lock()
{
  DWORD ret;

  if (myLog)
    MvrLog::log(MvrLog::Terse, "Locking %s", myLogName.c_str());
  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Initialization of mutex %s failed, failed lock", myLogName.c_str());
    return(STATUS_FAILED_INIT);
  }

  if(ourLockWarningMS > 0) startLockTimer();
  ret=WaitForSingleObject(myMutex, INFINITE);
  if (ret == WAIT_ABANDONED)
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Tried to lock a mutex %s which was locked by a different thread and never unlocked before that thread exited. This is a recoverable error", myLogName.c_str());
    return(lock());
  }
  else if (ret == WAIT_OBJECT_0)
  {
    // locked
	if(ourLockWarningMS > 0) checkLockTime();
	if(ourUnlockWarningMS > 0) startUnlockTimer();
    return(0);
  }
  else
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Failed to lock %s due to an unknown error", myLogName.c_str());
    return(STATUS_FAILED);
  }

  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
	MvrLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, thread '%s' %d, invoking functor '%s'", 
			 myLogName.c_str(),
			 MvrThread::getThisThreadName(), MvrThread::getThisThread(),
			 ourNonRecursiveDeadlockFunctor->getName());
	ourNonRecursiveDeadlockFunctor->invoke();
	exit(255);
      }
      else
      {
	MvrLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d, calling Mvria::shutdown", 
			 myLogName.c_str(),
			 MvrThread::getThisThreadName(), MvrThread::getThisThread());
	Mvria::shutdown();
	exit(255);
      }
	
    }
    myWasAlreadyLocked = true;
  }

  if(ourLockWarningMS > 0) checkLockTime();
  if(ourUnlockWarningMS > 0) startUnlockTimer();

  return(0);
}

int MvrMutex::tryLock()
{
  DWORD ret;

  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Initialization of mutex %s failed, failed lock", myLogName.c_str());
    return(STATUS_FAILED_INIT);
  }

  // Attempt to wait as little as posesible
  ret=WaitForSingleObject(myMutex, 1);
  if (ret == WAIT_ABANDONED)
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Tried to lock mutex %s nwhich was locked by a different thread and never unlocked before that thread exited. This is a recoverable error", myLogName.c_str());
    return(lock());
  }
  else if (ret == WAIT_TIMEOUT)
  {
	// we really don't need to log when the reason for this call at all happens
    //MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::tryLock: Could not lock mutex %s because it is already locked", myLogName.c_str());
    return(STATUS_ALREADY_LOCKED);
  }
  else if (ret == WAIT_OBJECT_0)
    return(0);
  else
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Failed to lock %s due to an unknown error", myLogName.c_str());
    return(STATUS_FAILED);
  }

  
  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
	MvrLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, thread '%s' %d, invoking functor '%s'", 
			 myLogName.c_str(),
			 MvrThread::getThisThreadName(), MvrThread::getThisThread(),
			 ourNonRecursiveDeadlockFunctor->getName());
	ourNonRecursiveDeadlockFunctor->invoke();
	exit(255);
      }
      else
      {
	MvrLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d, calling Mvria::shutdown", 
			 myLogName.c_str(),
			 MvrThread::getThisThreadName(), MvrThread::getThisThread());
	Mvria::shutdown();
	exit(255);
      }
	
    }
    myWasAlreadyLocked = true;
  }


  return(0);
}

int MvrMutex::unlock()
{
  if (myLog)
    MvrLog::log(MvrLog::Terse, "Unlocking %s", myLogName.c_str());
  if (myFailedInit)
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::unlock: Initialization of mutex %s failed, failed unlock", myLogName.c_str());
    return(STATUS_FAILED_INIT);
  }

  if(ourUnlockWarningMS > 0) checkUnlockTime();

  if (!ReleaseMutex(myMutex))
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrMutex::unlock: Failed to unlock %s due to an unknown error", myLogName.c_str());
    return(STATUS_FAILED);
  }
  if (myNonRecursive)
    myWasAlreadyLocked = false;
  return(0);
}

MVREXPORT const char * MvrMutex::getError(int messageNumber) const
{
  switch (messageNumber) {
  case STATUS_FAILED_INIT:
    return "Failed to initialize";
  case STATUS_FAILED:
    return "General failure";
  case STATUS_ALREADY_LOCKED:
    return "Mutex already locked";
  default:
    return NULL;
  }

}
