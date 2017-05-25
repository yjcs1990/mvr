/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRangeDeviceThreaded.h
 > Description  : A range device which can run in its own thread
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRANGEDEVICETHREADED_H
#define MVRRANGEDEVICETHREADED_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctorASyncTask.h"

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

#endif  // MVRRANGEDEVICETHREADED_H