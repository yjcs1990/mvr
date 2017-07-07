#include "MvrExport.h"
// Recurrent task class
#ifndef WIN32
#include <pthread.h>
#include <unistd.h>
#endif
#include "mvriaOSDef.h"
#include "MvrASyncTask.h"
#include "MvrLog.h"
#include "MvrRecurrentTask.h"

//
// Async recurrent tasks
//
// This class must be subclassed with the particular Task that will 
//   be run
//

// constructor: start up thread, leave it ready for go()

MVREXPORT 
MvrRecurrentTask::MvrRecurrentTask()
{
  setThreadName("MvrRecurrentTask");
  running = go_req = killed = false;
  create();			// create the thread
}


MVREXPORT 
MvrRecurrentTask::~MvrRecurrentTask()
{
  kill();
}

// Entry to the thread's main process
// Here we check if a Go request has been made, and
//   if so, we run Task()
// When done, set running to false, and wait for
//   the next request

MVREXPORT void *
MvrRecurrentTask::runThread(void *ptr) 
{
  threadStarted();
#ifndef WIN32
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
#endif
  while (myRunning)
    {
      bool doit;

      while (myRunning)
	{
	  lock();
	  doit = go_req;
	  unlock();
	  if (doit)
	    break;
//	  yield();		// don't hog resources
#ifndef WIN32
	  usleep(10000);
#else
	  Sleep(10);
#endif
	}
      if (!myRunning)
	break;
      lock();
      go_req = false;
      running = true;		// we've been requested to go
      unlock();
      task();			// do what we've got to do...
      lock();
      running = false;		// say we're done
      unlock();
    }

  threadFinished();
  return NULL;
}

MVREXPORT void MvrRecurrentTask::go()
{
  lock();
  go_req = true;
  running = true;
  killed = false;
  unlock();
}

MVREXPORT int MvrRecurrentTask::done()
{
  lock();
  bool is_running = running;
  bool is_killed = killed;
  unlock();
  if (is_running) return 0;
  if (is_killed) return 2;	// we didn't complete, were killed
  else return 1;
}

MVREXPORT void MvrRecurrentTask::reset()
{
  lock();
  go_req = false;
  if (running)			// async task is going, kill and restart
    {
      killed = true;
      running = false;
      unlock();
      cancel();
      create();
    }
  else
    unlock();
}

MVREXPORT void MvrRecurrentTask::kill()
{
  lock();
  go_req = false;
  killed = true;
  running = false;
  unlock();
  cancel();
}
