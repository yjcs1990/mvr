#ifndef MVRRANGEDEVICETHREADED_H
#define MVRRANGEDEVICETHREADED_H

#include "mvriaTypedefs.h"
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
  /// Get the running status of the thread, locking around the vmvriable
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
