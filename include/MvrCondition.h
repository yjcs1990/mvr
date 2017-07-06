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
#ifndef ARCONDITION_H
#define ARCONDITION_H


#if !defined(WIN32) || defined(MINGW)
#include <pthread.h>
#include "MvrMutex.h"
#endif
#include "mvriaTypedefs.h"


/** Threading condition wrapper class
 @ingroup UtilityClasses
*/
class MvrCondition
{
public:

  enum {
    STATUS_FAILED=1, ///< General failure
    STATUS_FAILED_DESTROY, ///< Another thread is waiting on this condition so it can not be destroyed
    STATUS_FAILED_INIT, ///< Failed to initialize thread. Requested action is imposesible
    STATUS_WAIT_TIMEDOUT, ///< The timedwait timed out before signaling
    STATUS_WAIT_INTR, ///< The wait was interupted by a signal
    STATUS_MUTEX_FAILED_INIT, ///< The underlying mutex failed to init
    STATUS_MUTEX_FAILED ///< The underlying mutex failed in some fashion
  };

  /** @internal */
#if defined(WIN32) && !defined(MINGW)
  typedef HANDLE CondType;
#else
  typedef pthread_cond_t CondType;
#endif

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
  { 
    return (myLogName=="")?"unnamed":myLogName.c_str(); 
  }


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


#endif // ARCONDITION_H
