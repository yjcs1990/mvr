#ifndef MVRTHREAD_H
#define MVRTHREAD_H


#include <map>
#if !defined(WIN32) || defined(MINGW)
#include <pthread.h>
#endif
#include "mvriaTypedefs.h"
#include "MvrMutex.h"
#include "MvrFunctor.h"
#include "MvrLog.h"

#ifdef MINGW
#include <vector>
#else
#include <map>
#endif

/// POSIX/WIN32 thread wrapper class. 
/**
  create() will create the thread. That thread will run the given Functor.

  A thread can either be in a detached state or a joinable state. If the
  thread is in a detached state, that thread can not be join()'ed upon. The
  thread will simply run until the program exits, or its function exits.
  A joinable thread means that another thread and call join() upon it. If
  this function is called, the caller will block until the thread exits
  its function. This gives a way to synchronize upon the lifespan of threads.

  Calling cancel() will cancel the thread.

  The static function self() will return a thread

  @sa MvrASyncTask which provides a different approach with a simpler interface.

*/
class MvrThread
{
public:

#if defined(WIN32) && !defined(MINGW)
  typedef DWORD ThreadType;
#else
  typedef pthread_t ThreadType;
#endif

// pthread_t on Linux happens to be an integer or pointer that can be used in a map. On other platforms, pthread_t may be a struct or similar, and this is true on MINGW, so store them differently.  Use the access methods that follow to access the map.
#ifdef MINGW
  typedef std::vector< std::pair<ThreadType, MvrThread*> > MapType;
#else
  typedef std::map<ThreadType, MvrThread*> MapType;
#endif

protected:
  static MvrThread* findThreadInMap(ThreadType t);
  static void removeThreadFromMap(ThreadType t); 
  static void addThreadToMap(ThreadType pt, MvrThread *at);
  
public:
  typedef enum {
    STATUS_FAILED=1, ///< Failed to create the thread
    STATUS_NORESOURCE, ///< Not enough system resources to create the thread
    STATUS_NO_SUCH_THREAD, ///< The thread can no longer be found
    STATUS_INVALID, ///< Thread is detached or another thread is joining on it
    STATUS_JOIN_SELF, ///< Thread is your own thread. Can't join on self.
    STATUS_ALREADY_DETATCHED ///< Thread is already detatched
  } Status;

  /// Constructor
  MVREXPORT MvrThread(bool blockAllSignals=true);
  /// Constructor - starts the thread
  MVREXPORT MvrThread(ThreadType thread, bool joinable,
		    bool blockAllSignals=true);
  /// Constructor - starts the thread
  MVREXPORT MvrThread(MvrFunctor *func, bool joinable=true,
		    bool blockAllSignals=true);
  /// Destructor
  MVREXPORT virtual ~MvrThread();

  /// Initialize the internal book keeping structures
  MVREXPORT static void init(void);
  /// Returns the instance of your own thread (the current one)
  MVREXPORT static MvrThread * self(void);
  /// Returns the os self of the current thread
  MVREXPORT static ThreadType osSelf(void);
  /// Stop all threads
  MVREXPORT static void stopAll();
  /// Cancel all threads
  MVREXPORT static void cancelAll(void);
  /// Join on all threads
  MVREXPORT static void joinAll(void);

  /// Shuts down and deletes the last remaining thread; call after joinAll
  MVREXPORT static void shutdown();

  /// Yield the processor to another thread
  MVREXPORT static void yieldProcessor(void);
  /// Gets the logging level for thread information
  static MvrLog::LogLevel getLogLevel(void) { return ourLogLevel; }
  /// Sets the logging level for thread information
  static void setLogLevel(MvrLog::LogLevel level) { ourLogLevel = level; }

  /// Create and start the thread
  MVREXPORT virtual int create(MvrFunctor *func, bool joinable=true,
			      bool lowerPriority=true);
  /// Stop the thread
  virtual void stopRunning(void) {myRunning=false;}
  /// Join on the thread
  MVREXPORT virtual int join(void **ret=NULL);
  /// Detatch the thread so it cant be joined
  MVREXPORT virtual int detach(void);
  /// Cancel the thread
  MVREXPORT virtual void cancel(void);

  /// Get the running status of the thread
  virtual bool getRunning(void) const {return(myRunning);}
  /// Get the running status of the thread, locking around the vmvriable
  virtual bool getRunningWithLock(void) 
    { bool running; lock(); running = myRunning; unlock(); return running; }
  /// Get the joinable status of the thread
  virtual bool getJoinable(void) const {return(myJoinable);}
  /// Get the underlying thread type
  virtual const ThreadType * getThread(void) const {return(&myThread);}
  /// Get the underlying os thread type
  virtual ThreadType getOSThread(void) const {return(myThread);}
  /// Get the functor that the thread runs
  virtual MvrFunctor * getFunc(void) const {return(myFunc);}

  /// Set the running value on the thread
  virtual void setRunning(bool running) {myRunning=running;}

#ifndef SWIG
  /** Lock the thread instance
      @swigomit
  */
  int lock(void) {return(myMutex.lock());}
  /** Try to lock the thread instance without blocking
      @swigomit
  */
  int tryLock(void) {return(myMutex.tryLock());}
  /** Unlock the thread instance
      @swigomit
  */
  int unlock(void) {return(myMutex.unlock());}
#endif

  /// Do we block all process signals at startup?
  bool getBlockAllSignals(void) {return(myBlockAllSignals);}

  /// Gets the name of the thread
  virtual const char *getThreadName(void) { return myName.c_str();  }

  /// Sets the name of the thread
  MVREXPORT virtual void setThreadName(const char *name);

  /// Gets a string that describes what the thread is doing NULL if it
  /// doesn't know
  virtual const char *getThreadActivity(void) { return NULL; }

  /// Marks the thread as started and logs useful debugging information.
  /**
     If you call this function in your functor (ie runThread) it'll
     then call some things for logging (to make debugging easier)
     This method should be called before the main thread loop begins.
   **/
  MVREXPORT virtual void threadStarted(void);

  /// Marks the thread as finished and logs useful debugging information.
  /**
     This method should be called after the main thread loop ends.  It
     enables the creator of the thread to determine that the thread has
     actually been completed and can be deleted.
   **/
  MVREXPORT virtual void threadFinished(void);

  /// Returns whether the thread has been started.
  /**
   * This is dependent on the thread implementation calling the 
   * threadStarted() method.
  **/
  MVREXPORT virtual bool isThreadStarted() const;

  /// Returns whether the thread has been completed and can be deleted.
  /**
   * This is dependent on the thread implementation calling the 
   * threadFinished() method.
  **/
  MVREXPORT virtual bool isThreadFinished() const;


  /// Logs the information about this thread
  MVREXPORT virtual void logThreadInfo(void);

#ifndef WIN32
  pid_t getPID(void) { return myPID; }
  pid_t getTID(void) { return myTID; }
#endif

  /// Gets the name of the this thread
  MVREXPORT static const char *getThisThreadName(void);
  /// Get the underlying thread type of this thread
  MVREXPORT static const ThreadType * getThisThread(void);
  /// Get the underlying os thread type of this thread
  MVREXPORT static ThreadType getThisOSThread(void);

protected:
  static MvrMutex ourThreadsMutex;
  static MapType ourThreads;
#if defined(WIN32) && !defined(MINGW)
  static std::map<HANDLE, MvrThread *> ourThreadHandles;
#endif 
  MVREXPORT static MvrLog::LogLevel ourLogLevel; 

  MVREXPORT virtual int doJoin(void **ret=NULL);

  std::string myName;

  MvrMutex myMutex;
  /// State vmvriable to denote when the thread should continue or exit
  bool myRunning;
  bool myJoinable;
  bool myBlockAllSignals;

  bool myStarted;
  bool myFinished;

  MvrStrMap myStrMap;
  MvrFunctor *myFunc;
  ThreadType myThread;
#if defined(WIN32) && !defined(MINGW)
  HANDLE myThreadHandle;
#endif

  
#if !defined(WIN32) || defined(MINGW)
  pid_t myPID;
  pid_t myTID;
#endif

  static std::string ourUnknownThreadName;
};


#endif // ARTHREAD_H
