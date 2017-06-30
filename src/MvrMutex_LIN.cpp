/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

/* Need to define this to get pthread_mutexattr_settype when using GCC 2.96 */
#define _XOPEN_SOURCE 500

#include "MvrExport.h"
#include <errno.h>
#include "ariaOSDef.h"
#include "MvrMutex.h"
#include "MvrLog.h"
#include "MvrThread.h"
#include "ariaUtil.h"
#include "ariaInternal.h"
#include "MvrFunctor.h"

#include <sys/types.h>
#include <unistd.h>     // for getpid()



/**

**/
ArMutex::ArMutex(bool recursive) :
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
    myFailedInit=true;
    ArLog::logNoLock(MvrLog::Terse, "MvrMutex::ArMutex: Failed to initialize mutex");
  }
  // MPL took this out when I made it recursive since otherwise we'd
  //get warnings on the unlock
  //else 
  //unlock();

  pthread_mutexattr_destroy(&attr);

  myNonRecursive = !recursive;
  myWasAlreadyLocked = false;

  myStrMap[STATUS_FAILED_INIT]="Failed to initialize";
  myStrMap[STATUS_FAILED]="General failure";
  myStrMap[STATUS_ALREADY_LOCKED]="Mutex already locked";

  //myStrMap.insert(std::pair<int, std::string>(STATUS_FAILED_INIT, "Failed to initialize"));
  //myStrMap.insert(std::pair<int, std::string>(STATUS_FAILED, "General failure"));
  //myStrMap.insert(std::pair<int, std::string>(STATUS_ALREADY_LOCKED, "Mutex already locked"));
}

ArMutex::~ArMutex()
{
  int ret;
  if (!myFailedInit && (ret = pthread_mutex_destroy(&myMutex)) != 0)
  {
    if (ret == EBUSY)
      ArLog::logNoLock(MvrLog::Verbose, 
		       "MvrMutex::~ArMutex: Failed to destroy mutex %s. A thread is currently blocked waiting for this mutex.", 
		       myLogName.c_str());
    else
      ArLog::logNoLock(MvrLog::Terse, 
	     "MvrMutex::~ArMutex: Failed to destroy mutex %s. Unknown error.",
		       myLogName.c_str());
  }      
  uninitLockTiming();
}

ArMutex::ArMutex(const ArMutex &mutex)
{
  myLog = mutex.myLog;
  if (pthread_mutex_init(&myMutex, 0) != 0)
  {
    myFailedInit=true;
    ArLog::logNoLock(MvrLog::Terse, "MvrMutex::ArMutex: Failed to initialize mutex");
  }
  else
    unlock();

  myLogName = mutex.myLogName;
  initLockTiming();
  myStrMap[STATUS_FAILED_INIT]="Failed to initialize";
  myStrMap[STATUS_FAILED]="General failure";
  myStrMap[STATUS_ALREADY_LOCKED]="Mutex already locked";
}



/**
   Lock the mutex. This function will block until no other thread has this
   mutex locked. If it returns 0, then it obtained the lock and the thread
   is free to use the critical section that this mutex protects. Else it
   returns an error code. See getError().
*/
int ArMutex::lock() 
{
  if(ourLockWarningMS > 0) startLockTimer();

  if (myLog)
    ArLog::logNoLock(MvrLog::Terse, "Locking '%s' from thread '%s' %d pid %d", 
	       myLogName.c_str(),
	       ArThread::getThisThreadName(), 
	       ArThread::getThisThread(), getpid());

  if (myFailedInit)
  {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Initialization of mutex '%s' from thread ('%s' %d pid %d) failed, failed lock", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
    ArLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Initialization of mutex failed, failed lock");
    return(STATUS_FAILED_INIT);
  }

  int ret;
  if ((ret = pthread_mutex_lock(&myMutex)) != 0)
  {
    if (ret == EDEADLK)
    {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Trying to lock mutex '%s' from thread ('%s' %d pid %d) which is already locked by this thread", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
      return(STATUS_ALREADY_LOCKED);
    }
    else
    {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::lock: Failed to lock mutex ('%s') from thread ('%s' %d pid %d) due to an unknown error", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
      return(STATUS_FAILED);
    }
  }

  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
	ArLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d pid %d, invoking functor '%s'", 
			 myLogName.c_str(), 
			 ArThread::getThisThreadName(),
			 ArThread::getThisThread(), getpid(),
			 ourNonRecursiveDeadlockFunctor->getName());
	ArLog::logBacktrace(MvrLog::Normal);
	ourNonRecursiveDeadlockFunctor->invoke();
	exit(255);
      }
      else
      {
	ArLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d pid %d, calling Aria::shutdown", 
			 myLogName.c_str(), ArThread::getThisThreadName(), 
			 ArThread::getThisThread(), getpid());
	ArLog::logBacktrace(MvrLog::Normal);
	Aria::shutdown();
	exit(255);
      }
	
    }
    myWasAlreadyLocked = true;
  }

  if(ourLockWarningMS > 0) checkLockTime();
  if(ourUnlockWarningMS > 0) startUnlockTimer();

  
  return(0);
}
  
/**
   Try to lock the mutex. This function will not block if another thread has
   the mutex locked. It will return instantly if that is the case. It will
   return STATUS_ALREADY_LOCKED if another thread has the mutex locked. If it
   obtains the lock, it will return 0.
*/
int ArMutex::tryLock() 
{
  if (myFailedInit)
  {
    ArLog::logNoLock(MvrLog::Terse, "MvrMutex::trylock: Initialization of mutex '%s' from thread ('%s' %d pid %d) failed, failed trylock",
		     myLogName.c_str(), ArThread::getThisThreadName(),
		     ArThread::getThisThread(), getpid());
    return(STATUS_FAILED_INIT);
  }

  if (myLog)
    ArLog::logNoLock(MvrLog::Terse, "Try locking %s from thread %s %d pid %d", 
	       myLogName.c_str(),
	       ArThread::getThisThreadName(), 
	       ArThread::getThisThread(), getpid());

  int ret;
  if ((ret = pthread_mutex_trylock(&myMutex)) != 0)
  {
    if (ret == EBUSY)
    {
      if(myLog)
        ArLog::logNoLock(MvrLog::Terse, "MvrMutex::tryLock: Mutex %s is already locked", myLogName.c_str());
      return(STATUS_ALREADY_LOCKED);
    }
    else
    {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::trylock: Failed to trylock a mutex ('%s') from thread ('%s' %d pid %d) due to an unknown error", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
      return(STATUS_FAILED);
    }
  }

  if (myNonRecursive)
  {
    if (myWasAlreadyLocked)
    {
      
      if (ourNonRecursiveDeadlockFunctor != NULL)
      {
	ArLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d pid %d, invoking functor '%s'", 
			 myLogName.c_str(), 
			 ArThread::getThisThreadName(),
			 ArThread::getThisThread(), getpid(),
			 ourNonRecursiveDeadlockFunctor->getName());
	ArLog::logBacktrace(MvrLog::Normal);
	ourNonRecursiveDeadlockFunctor->invoke();
	exit(255);
      }
      else
      {
	ArLog::logNoLock(MvrLog::Terse, 
			 "MvrMutex: '%s' tried to lock recursively even though it is nonrecursive, from thread '%s' %d pid %d, calling Aria::shutdown", 
			 myLogName.c_str(), ArThread::getThisThreadName(), 
			 ArThread::getThisThread(), getpid());
	ArLog::logBacktrace(MvrLog::Normal);
	Aria::shutdown();
	exit(255);
      }
	
    }
    myWasAlreadyLocked = true;
  }

  if (myLog)
    ArLog::logNoLock(MvrLog::Terse, 
		     "Try locked '%s' from thread '%s' %d pid %d", 
		     myLogName.c_str(),
		     ArThread::getThisThreadName(), 
		     ArThread::getThisThread(), getpid());

  return(0);
}

int ArMutex::unlock() 
{
  if (myLog)
    ArLog::logNoLock(MvrLog::Terse, 
		     "Unlocking '%s' from thread '%s' %d pid %d", 
		     myLogName.c_str(),
		     ArThread::getThisThreadName(), 
		     ArThread::getThisThread(), getpid());

  if(ourUnlockWarningMS > 0) checkUnlockTime();

  if (myFailedInit)
  {
    ArLog::logNoLock(MvrLog::Terse, "MvrMutex::unlock: Initialization of mutex '%s' from thread '%s' %d pid %d failed, failed unlock", 		       
		     myLogName.c_str(), ArThread::getThisThreadName(),
		     ArThread::getThisThread(), getpid());
    return(STATUS_FAILED_INIT);
  }

  int ret;
  if ((ret = pthread_mutex_unlock(&myMutex)) != 0)
  {
    if (ret == EPERM)
    {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::unlock: Trying to unlock a mutex ('%s') which this thread ('%s' %d pid %d) does not own", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
      return(STATUS_ALREADY_LOCKED);
    }
    else
    {
      ArLog::logNoLock(MvrLog::Terse, "MvrMutex::unlock: Failed to unlock mutex ('%s') from thread ('%s' %d pid %d) due to an unknown error", 
		       myLogName.c_str(), ArThread::getThisThreadName(),
		       ArThread::getThisThread(), getpid());
      return(STATUS_FAILED);
    }
  }
  if (myNonRecursive)
    myWasAlreadyLocked = false;
  return(0);
}

AREXPORT const char *ArMutex::getError(int messageNumber) const
{
  ArStrMap::const_iterator it;
  if ((it = myStrMap.find(messageNumber)) != myStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}
