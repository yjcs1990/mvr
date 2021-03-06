#include "MvrExport.h"
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "mvriaOSDef.h"
#include "MvrCondition.h"
#include "MvrLog.h"

#include <time.h>


MvrStrMap MvrCondition::ourStrMap;


MVREXPORT MvrCondition::MvrCondition() :
  myFailedInit(false),
  myCond(),
  myMutex(false)
{
  myMutex.setLogName("MvrCondition::myMutex");
  pthread_condattr_t attr;

  pthread_condattr_init(&attr);
  if (pthread_cond_init(&myCond, &attr) != 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::MvrCondition: Unknown error trying to create the condition.");
    myFailedInit=true;
  }

  pthread_condattr_destroy(&attr);

  ourStrMap[STATUS_FAILED]="General failure";
  ourStrMap[STATUS_FAILED_DESTROY]=
  "Another thread is waiting on this condition so it can not be destroyed";
  ourStrMap[STATUS_FAILED_INIT] =
  "Failed to initialize thread. Requested action is imposesible";
  ourStrMap[STATUS_MUTEX_FAILED_INIT]="The underlying mutex failed to init";
  ourStrMap[STATUS_MUTEX_FAILED]="The underlying mutex failed in some fashion";
}

MVREXPORT MvrCondition::~MvrCondition()
{
  int ret;

  ret=pthread_cond_destroy(&myCond);
  if (ret == EBUSY)
    MvrLog::log(MvrLog::Terse, "MvrCondition::~MvrCondition: Trying to destroy a condition that another thread is waiting on.");
  else if (ret != 0)
    MvrLog::log(MvrLog::Terse, "MvrCondition::~MvrCondition: Unknown error while trying to destroy the condition.");
}

MVREXPORT int MvrCondition::signal()
{
  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::signal: Initialization of condition failed, failed to signal");
    return(STATUS_FAILED_INIT);
  }

  if (pthread_cond_signal(&myCond) != 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::signal: Unknown error while trying to signal the condition.");
    return(STATUS_FAILED);
  }

  return(0);
}

MVREXPORT int MvrCondition::broadcast()
{
  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::broadcast: Initialization of condition failed, failed to broadcast");
    return(STATUS_FAILED_INIT);
  }

  if (pthread_cond_broadcast(&myCond) != 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::broadcast: Unknown error while trying to broadcast the condition.");
    return(STATUS_FAILED);
  }

  return(0);
}

MVREXPORT int MvrCondition::wait()
{
  int ret;

  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::wait: Initialization of condition failed, failed to wait");
    return(STATUS_FAILED_INIT);
  }

  ret=myMutex.lock();
  if (ret != 0)
  {
    if (ret == MvrMutex::STATUS_FAILED_INIT)
      return(STATUS_MUTEX_FAILED_INIT);
    else
      return(STATUS_MUTEX_FAILED);
  }

  ret=pthread_cond_wait(&myCond, &myMutex.getMutex());
  if (ret != 0)
  {
    if (ret == EINTR)
      return(STATUS_WAIT_INTR);
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrCondition::wait: Unknown error while trying to wait on the condition.");
      return(STATUS_FAILED);
    }
  }

  ret=myMutex.unlock();
  if (ret != 0)
  {
    if (ret == MvrMutex::STATUS_FAILED_INIT)
      return(STATUS_MUTEX_FAILED_INIT);
    else
      return(STATUS_MUTEX_FAILED);
  }

  return(0);
}

MVREXPORT int MvrCondition::timedWait(unsigned int msecs)
{
  int ret;
  int retUnlock;

  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition::wait: Initialization of condition failed, failed to wait");
    return(STATUS_FAILED_INIT);
  }

  ret=myMutex.lock();
  if (ret != 0)
  {
    if (ret == MvrMutex::STATUS_FAILED_INIT)
      return(STATUS_MUTEX_FAILED_INIT);
    else
      return(STATUS_MUTEX_FAILED);
  }

  /*
  gettimeofday(&tp, NULL);
  // convert time of day to pthread time structure
  spec.tv_sec = tp.tv_sec;
  spec.tv_nsec = tp.tv_usec * 1000;

  // add on time specified by msecs
  spec.tv_sec += (long int)rint(((float)msecs)/1000.0);
  // 1 millisecond = 1000 micro seconds = 1000000 nanoseconds
  spec.tv_nsec += (long int)( ( msecs % 1000 ) * 1000000);
//   printf("input millisecond=%d :: sec=%ld nsec=%ld curtime=%ld %ld\n", msecs, spec.tv_sec, spec.tv_nsec, tp.tv_sec, tp.tv_usec * 1000);
*/
  struct timespec spec;
#ifdef _POSIX_TIMERS
  clock_gettime(CLOCK_REALTIME, &spec);
#else
#warning posix realtime timer not available so using gettimeofday instead of clock_gettime
  struct timeval tp;
  gettimeofday(&tp, NULL);
  spec.tv_sec = tp.tv_sec;
  spec.tv_nsec = tp.tv_usec * 1000;
#endif

  spec.tv_sec += msecs / 1000;
  spec.tv_nsec += msecs * 1000 * 1000;
  if (spec.tv_nsec > 1000 * 1000 * 1000)
  {
    spec.tv_sec += 1;
    spec.tv_nsec -= 1000 * 1000 * 1000;
  }
  

  int timedWaitErrno;
  
  ret=pthread_cond_timedwait(&myCond, &myMutex.getMutex(), &spec);
  timedWaitErrno = errno;

  /*
  if (ret != 0)
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrCondition::timedWait: Unknown error while trying to wait on the condition. Ret %d, %s", ret, strerror(ret));
  */

  // must unlock the mutex, even if we fail, since we reacquire lock
  // after timedwait times out
  retUnlock=myMutex.unlock();
  
  if (ret != 0)
  {
    if (ret == EINTR)
      return(STATUS_WAIT_INTR);
    else if (ret == ETIMEDOUT)
      return(STATUS_WAIT_TIMEDOUT);
    else
    {
      MvrLog::logErrorFromOS(MvrLog::Terse, "MvrCondition::timedWait: Unknown error while trying to wait on the condition. Ret %d, %s.  Errno %d, %s.", 
			    ret, strerror(ret),
			    timedWaitErrno, strerror(timedWaitErrno));

      return(STATUS_FAILED);
    }
  }

  if (retUnlock != 0)
  {
    if (retUnlock == MvrMutex::STATUS_FAILED_INIT)
      return(STATUS_MUTEX_FAILED_INIT);
    else
      return(STATUS_MUTEX_FAILED);
  }

  return(0);
}

MVREXPORT const char * MvrCondition::getError(int messageNumber) const
{
  MvrStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}
