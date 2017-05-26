/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrCondition.h
 > Description  : Threading condition wrapper class
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRCONDITION_H
#define MVRCONDITION_H

#if !defined(WIN32) || defined(MINGW)
#include <pthread.h>
#include "MvrMutex.h"
#endif
#include "mvriaTypedefs.h"

/// @ingroup UtilityClasses
class MvrCondition
{
public:
  enum {
    STATUS_FAILED=1,          ///< General failure
    STATUS_FAILED_DESTROY,    ///< Another thread is waiting on this condition so it can not be destroyed
    STATUS_FAILED_INIT,       ///< Failed to initialize thread. Requested action is impossible
    STATUS_WAIT_TIMEDOUT,     ///< The time timed out before signaling
    STATUS_WAIT_INTR,         ///< The wait was interrupted by a signal
    STATUS_MUTEX_FAILED_INIT, ///< The underlying mutex failed to init
    STATUS_MUTEX_FAILED       ///< The underlying mutex failed in some fashion
  };
  /*@internal */
#if defined(WIN32) && !defined(MINGW)
  typedef HANDLE CondType;
#else
  typedef pthread_cond_t CondType;
#endif  // WIN32

  /// Constructor
  MVREXPORT MvrCondition();
  /// Desctructor
  MVREXPORT virtual ~MvrCondition();

  /// Signal the thread waiting
  MVREXPORT int signal();
  /// Broadcast a signal to all threads waiting
  MVREXPORT int broadcast();
  /** @brief Wait for a signal */
  MVREXPORT int wait();
  /// Wait for a signal for a period of time in milliseconds
  MVREXPORT int timedWait(unsigned int msecs);
  /// Translate error into string
  MVREXPORT const char *getError(int messageNumber) const;

  // Set a name to be included in log messages
  void setLogName(const char *logName) {myLogName = logName;}
  const char *getLogName() 
  { return (myLogName=="")?"unnamed":myLogName.c_str();}
protected:
  static MvrStrMap ourStrMap;

  bool myFailedInit;
  CondType myCond;
#if defined(WIN32) && !defined(MINGW)
  int myCount;
#else
  MvrMutex myMutex;
#endif
  std::string myLogName;
};

#endif  // MVRCONDITION_H