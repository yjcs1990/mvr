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
#ifndef ARRANGEDEVICETHREADED_H
#define ARRANGEDEVICETHREADED_H

#include "ariaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctorASyncTask.h"

/// A range device which can run in its own thread
/** 
    This is a range device thats threaded, it doesn't do
    multipleInheritance from both MvrASyncTask and MvrRangeDevice any
    more since JAVA doesn't support this and the wrapper software
    can't deal with it.  Its still functionally the same however.
 **/
class MvrRangeDeviceThreaded : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrRangeDeviceThreaded(size_t currentBufferSize, 
				 size_t cumulativeBufferSize,
				 const char *name, unsigned int maxRange,
				 int maxSecondsTokeepCurrent = 0,
				 int maxSecondsToKeepCumulative = 0,
				 double maxDistToKeepCumulative = 0,
				 bool locationDependent = false);
  /// Destructor
  MVREXPORT virtual ~MvrRangeDeviceThreaded();
  /// The functor you need to implement that will be the one executed by the thread
  MVREXPORT virtual void * runThread(void *arg) = 0;
  /// Run in this thread
  MVREXPORT virtual void run(void) { myTask.run(); }
  /// Run in its own thread
  MVREXPORT virtual void runAsync(void) { myTask.runAsync(); }
  /// Stop the thread
  MVREXPORT virtual void stopRunning(void) { myTask.stopRunning(); }
  /// Get the running status of the thread
  MVREXPORT virtual bool getRunning(void) { return myTask.getRunning();}
  /// Get the running status of the thread, locking around the variable
  MVREXPORT virtual bool getRunningWithLock(void) 
    { return myTask.getRunningWithLock(); }

  MVREXPORT virtual int lockDevice(void) { return myTask.lock(); }
  MVREXPORT virtual int tryLockDevice(void) { return myTask.tryLock(); }
  MVREXPORT virtual int unlockDevice(void) { return myTask.unlock(); }
protected:
  MvrRetFunctor1C<void *, MvrRangeDeviceThreaded, void *> myRunThreadCB;
  MvrFunctorASyncTask myTask;

};

#endif // ARRANGEDEVICETHREADED_H
