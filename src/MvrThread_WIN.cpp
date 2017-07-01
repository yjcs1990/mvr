#include "MvrExport.h"
#include "mvriaOSDef.h"
#include <list>
#include "MvrThread.h"
#include "MvrLog.h"
#include "MvrSignalHandler.h"
#include "mvriaUtil.h"


static DWORD WINAPI run(void *arg)
{
  MvrThread *t=(MvrThread*)arg;
  void *ret=NULL;

  if (t->getBlockAllSignals())
    MvrSignalHandler::blockCommonThisThread();

  if (dynamic_cast<MvrRetFunctor<void*>*>(t->getFunc()))
    ret=((MvrRetFunctor<void*>*)t->getFunc())->invokeR();
  else
    t->getFunc()->invoke();

  return((DWORD)ret);
}

void MvrThread::init()
{
  MvrThread *main;
  ThreadType pt;
  MapType::iterator iter;

  pt=GetCurrentThreadId();

  ourThreadsMutex.lock();
  if (ourThreads.size())
  {
    ourThreadsMutex.unlock();
    return;
  }
  main=new MvrThread;
  main->myJoinable=true;
  main->myRunning=true;
  main->myThread=pt;
  ourThreads.insert(MapType::value_type(pt, main));
  ourThreadsMutex.unlock();
}

MVREXPORT void MvrThread::shutdown()
{
  ourThreadsMutex.lock();

  // At this point, the ourThreads map should only contain the main thread 
  // that was created in init (presuming that joinAll was called, from 
  // the main thread).
  // 
  // Do not use deleteSetPairs because this causes the ourThreads map 
  // to be updated recursively (because the destructor updates the map).
  //
  std::list<MvrThread *> threadList;

  for (MapType::iterator mapIter = ourThreads.begin(); 
       mapIter != ourThreads.end();
       mapIter++) {
    if (mapIter->second != NULL) {
      threadList.push_back(mapIter->second);
    }
  }
  for (std::list<MvrThread *>::iterator listIter = threadList.begin();
      listIter != threadList.end();
      listIter++) {
    delete (*listIter);
  }
  if (!ourThreads.empty()) {
    MvrLog::log(MvrLog::Normal,
               "MvrThread::shutdown() unexpected thread leftover");
  }
  ourThreadsMutex.unlock();

} // end method shutdown


MVREXPORT MvrThread::~MvrThread()
{
  CloseHandle(myThreadHandle);

  // Just make sure the thread is no longer in the map.
  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadsMutex.unlock();
}


MVREXPORT MvrThread * MvrThread::self()
{
  ThreadType pt;
  MapType::iterator iter;

  ourThreadsMutex.lock();
  pt=GetCurrentThreadId();
  iter=ourThreads.find(pt);
  ourThreadsMutex.unlock();

  if (iter != ourThreads.end())
    return((*iter).second);
  else
    return(NULL);
}

MVREXPORT MvrThread::ThreadType MvrThread::osSelf()
{ 
  return GetCurrentThreadId();
}

MVREXPORT void MvrThread::cancelAll()
{
  DWORD ret=0;
  std::map<HANDLE, MvrThread *>::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreadHandles.begin(); iter != ourThreadHandles.end(); ++iter)
    TerminateThread((*iter).first, ret);
  ourThreadHandles.clear();
  ourThreads.clear();
  ourThreadsMutex.unlock();
}


MVREXPORT int MvrThread::create(MvrFunctor *func, bool joinable,
                              bool lowerPriority)
{
  // Log a warning (until desired behavior is determined)
  if (myThreadHandle != 0) {
    MvrLog::log(MvrLog::Terse, "MvrThread::create: Thread %s (ID %d) already created.",
               (myName.empty() ? "[anonymous]" : myName.c_str()),
		           myThread);
  }


  DWORD err;

  myJoinable=joinable;
  myFunc=func;
  myRunning=true;

  myThreadHandle = CreateThread(0, 0, &run, this, 0, &myThread);
  err=GetLastError();
  if (myThreadHandle == 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrThread::create: Failed to create thread.");
    return(STATUS_FAILED);
  }
  else
  {
    if (myName.size() == 0)
    {
      MvrLog::log(ourLogLevel, "Created anonymous thread with ID %d", 
		 myThread);
      //MvrLog::logBacktrace(MvrLog::Normal);
    }
    else
    {
      MvrLog::log(ourLogLevel, "Created %s thread with ID %d", myName.c_str(),
		 myThread);
    }
    ourThreadsMutex.lock();
    ourThreads.insert(MapType::value_type(myThread, this));
	ourThreadHandles.insert(std::map<HANDLE, MvrThread *>::value_type(myThreadHandle, this));
    ourThreadsMutex.unlock();
    if (lowerPriority)
      SetThreadPriority(myThreadHandle, THREAD_PRIORITY_IDLE);
    return(0);
  }
}

MVREXPORT int MvrThread::doJoin(void **iret)
{
  DWORD ret;

  ret=WaitForSingleObject(myThreadHandle, INFINITE);
  if (ret == WAIT_FAILED)
  {
    MvrLog::log(MvrLog::Terse, "MvrThread::doJoin: Failed to join on thread.");
    return(STATUS_FAILED);
  }

  return(0);
}

MVREXPORT int MvrThread::detach()
{
  return(0);
}

MVREXPORT void MvrThread::cancel()
{
  DWORD ret=0;

  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadHandles.erase(myThreadHandle);
  ourThreadsMutex.unlock();
  TerminateThread(myThreadHandle, ret);
}

MVREXPORT void MvrThread::yieldProcessor()
{
  Sleep(0);
}


MVREXPORT void MvrThread::threadStarted(void)
{
  myStarted = true;
  if (myName.size() == 0)
    MvrLog::log(ourLogLevel, "Anonymous thread (%d) is running",
	             myThread);
  else
    MvrLog::log(ourLogLevel, "Thread %s (%d) is running", 
	             myName.c_str(), myThread);
}

MVREXPORT void MvrThread::threadFinished(void)
{
  myFinished = true;
  if (myName.size() == 0)
    MvrLog::log(ourLogLevel, "Anonymous thread (%d) is finished",
	             myThread);
  else
    MvrLog::log(ourLogLevel, "Thread %s (%d) is finished", 
	             myName.c_str(), myThread);
}

MVREXPORT void MvrThread::logThreadInfo(void)
{
  if (myName.size() == 0)
    MvrLog::log(ourLogLevel, "Anonymous thread (%d) is running",
	             myThread);
  else
    MvrLog::log(ourLogLevel, "Thread %s (%d) is running", 
	             myName.c_str(), myThread);
}
