/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMutex.h
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#ifndef MVRMUTEX_H
#define MVRMUTEX_H

#if !ifdefined(WIN32) || defined(MINGW)
#include <pthread.h>
#endif
#include <string>
#include <mvriaTypedefs.h>

class MvrTime;
class MvrFunctor;

class MvrMutex
{
public:
#if defined(WIN32) && !defined(MINGW)
  typedef HANDLE MutexType;
#else
  typedef pthread_mutex_t MutexType;
#endif  // WIN32

  typedef enum{
    STATUS_FAILED_INIT=1, ///< Failed to initialize
    STATUS_FAILED, ///< General failure
    STATUS_ALREADY_LOCKED ///< Mutex already locked
  };
  /// Constructor
  MVREXPORT MvrMutex(bool recursive = true);
  /// Destructor
  MVREXPORT virtual ~MvrMutex();
  /// Copy constructor
  MVREXPORT MvrMutex(const MvrMutex &mutex);

  /** Lock the mutex
   *
   *  If this is a recursive mutex (the default type), and a different
   *  thread has locked this mutex, then block until it is unlocked; if
   *  this thread has locked this mutex, then continue immediately and do
   *  not block.
   *
   *  If this is not a recursive mutex, then block if any thread
   *  (including this thread) has locked this mutex.
   *
   *  Call setLog(true) to enable extra logging.
   *
   *  @return 0 if the mutex was successfully locked (after blocking if it was
   *  already locked by another thread, or by any thread if this is not a
   *  recursive mutex).
   *  @return MvrMutex::STATUS_ALREADY_LOCKED immediately if this is a recursive mutex
   *  (default) and the current thread has already locked this mutex.
   *  @return MvrMutex::STATUS_FAILED on an error from the platform mutex
   *  implementation.
   *  @return MvrMutex::STATUS_FAILED_INIT if the platform threading is not 
   *  enabled, initialized, etc.
   */
  MVREXPORT virtual int lock();

  /** Try to lock the mutex, but do not block
   *
   *  If this is a recursive mutex (the default type), and a different
   *  thread has locked this mutex, then return MvrMutex::STATUS_ALREADY_LOCKED; if
   *  this thread has locked this mutex, then return 0.
   *
   *  If this is not a recursive mutex, then return 0 if any thread
   *  (including this thread) has locked this mutex.
   *
   *  Returns MvrMutex::STATUS_FAILED or MvrMutex::STATUS_FAILED_INIT on an error (such as threading
   *  not initialized or supported).
   *
   *  Call setLog(true) to enable extra logging.
   *
   *  @return 0 If tryLock() acquires the lock, or mutex is a recursive mutex
   *  (default) and is already locked by this thread
   *  @return MvrMutex::STATUS_ALREADY_LOCKED if this mutex is currently locked
   *  by another thread, or if mutex is not recursive, by any thread
   *  including the current thread.
   *  @return MvrMutex::STATUS_FAILED on an error from the platform mutex
   *  implementation.
   *  @return MvrMutex::STATUS_FAILED_INIT if the platform threading is not 
   *  enabled, initialized, etc.
   */
  MVREXPORT virtual int tryLock();

  /// Unlock the mutex, allowing another thread to obtain the lock
  MVREXPORT virtual int unlock();

  /// Get a human readable error message from an error code
  MVREXPORT virtual const char * getError(int messageNumber) const;

  /** Sets a flag that will log out when we lock and unlock. Use setLogName() to
   *  set a descriptive name for this mutex, and MvrThread::setThreadName() to set a
   *  descriptive name for a thread.
   */
  void setLog(bool log) { myLog = log; } 
  /// Sets a name we'll use to log with
  void setLogName(const char *logName) { myLogName = logName; } 

#ifndef SWIG
  /// Sets a name we'll use to log with formatting
  /** @swigomit use setLogName() */
  MVREXPORT void setLogNameVar(const char *logName, ...);
#endif
  /// Get a reference to the underlying OS-specific mutex variable
  MVREXPORT virtual MutexType & getMutex() { return(myMutex); }
  /** Sets the lock warning time (sec). If it takes more than @param lockWarningSeconds to perform the mutex lock in lock(), log a warning.
      @linuxonly
  */
  static void setLockWarningTime(double lockWarningSeconds) { ourLockWarningMS = (unsigned int)(lockWarningSeconds*1000.0); }
  /** Gets the lock warning time (sec)
      @linuxonly
  */
  static double getLockWarningTime(void) { return ourLockWarningMS/1000.0; }
  /** Sets the unlock warning time (sec). If it takes more than @param unlockWarningSeconds between the mutex being locked with lock() then unlocked with unlock(), log a warning.
      @linuxonly
  */
  static void setUnlockWarningTime(double unlockWarningSeconds) { ourUnlockWarningMS = (unsigned int)(unlockWarningSeconds*1000.0); }
  /** Gets the lock warning time (sec)
      @linuxonly
  */
  static double getUnlockWarningTime(void) { return ourUnlockWarningMS/1000.0; }  
protected:
  bool myFailedInit;
  MutexType myMutex;
// Eliminating this from Windows in an attempt to debug a memory issue
#if !defined(WIN32) || defined(MINGW)
  MvrStrMap myStrMap;
#endif 

  bool myLog;
  std::string myLogName;

  bool myNonRecursive;
  bool myWasAlreadyLocked;

  bool myFirstLock;
  MvrTime *myLockTime;
  MVREXPORT static unsigned int ourLockWarningMS;
  MVREXPORT static unsigned int ourUnlockWarningMS;
  MvrTime *myLockStarted;

  // Initialize lock timing state. Call in MvrMutex constructor.
  void initLockTiming();
  // Destroy lock timing state. Call in destructor.
  void uninitLockTiming();
  // Start timing how long it takes to perform the mutex lock.
  void startLockTimer();
  // Check time it took to lock the mutex against ourLockWarningMS and log about it
  void checkLockTime();
  // Start timing how long it takes between this mutex lock and its next unlock.
  void startUnlockTimer();
  // Check time it took between lock and unlock against ourUnlockWarningMS and log about it
  void checkUnlockTime();

  static MvrFunctor *ourNonRecursiveDeadlockFunctor;
};

class MvrScopedLock {
private:
	MvrMutex& mtx;
public:
	MvrScopedLock(MvrMutex& m) : mtx(m) {
		mtx.lock();
	}
	~MvrScopedLock() {
		mtx.unlock();
	}
  void lock() { mtx.lock(); }
  void unlock() { mtx.unlock(); }
};

#endif  // MVRMUTEX_H