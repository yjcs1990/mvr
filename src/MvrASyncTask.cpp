/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrASyncTask.h
 > Description  : Asynchronous task (runs in its own thread)
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月10日
***************************************************************************************************/
#include "MvrExport.h"
#if !defined(WIN32) || defined(MINGW)
#include <pthread.h>
#endif
#include "mvriaOSDef.h"
#include "MvrASyncTask.h"
#include "MvrLog.h"

MVREXPORT MvrASyncTask::MvrASyncTask() :
          myFunc(this, &MvrASyncTask::runThread, NULL)
{

}          
MVREXPORT MvrASyncTask::~MvrASyncTask()
{

}

MVREXPORT int MvrASyncTask::create(bool joinable, bool lowerPriority)
{
  return (MvrThread::create(&myFunc, joinable, lowerPriority));
}

/**
   This will run the code of the ArASyncTask without creating a new
   thread to run it in. It performs the needed setup then calls runThread().
   This is good if you have a task which you wish to run multiple
   instances of and you want to use the main() thread  instead of having
   it block, waiting for exit of the program.
*/
MVREXPORT void *MvrASyncTask::runInThisThread(void *arg)
{
  myJoinable = true;
  myRunning  = true;
#if defined(WIN32) && !defined(MINGW)
  myThread=GetCurrentThreadId();
#else
  myThread = pthread_self();
#endif 

  if (myName.size() == 0)
  {
    MvrLog::log(ourLogLevel, "Running anonymous thread with ID %d", myThread);
  }
  else
  {
    MvrLog::log(ourLogLevel, "Running %s thread", myName.c_str());
  }

  addThreadToMap(myTrhead, this);

  return (runThread(arg));
}
