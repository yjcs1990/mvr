/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrThread.h
 > Description  : POSIX/WIN32 thread wrapper class. 
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月05日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrThread.h"
#include "MvrSignalHandler.h"

#include <errno.h>
#include <list>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef MINGW
#include <sys/syscall.h>
#endif


MvrMutex MvrThread::ourThreadsMutex;
MvrThread::MapType MvrThread::ourThreads;
#if defined(WIN32) && !defined(MINGW)
std::map<HANDLE, MvrThread *> MvrThread::ourThreadHandles;
#endif  // WIN32 
MVREXPORT MvrLog::LogLevel MvrThread::ourLogLevel = MvrLog::Verbose;
// TODO, instead of MVREXPORT move accessors into .cpp
std::string MvrThread::ourUnknownThreadName = "unknown";

static void *run(void *arg)
{
  MvrThread *t = (MvrThread*) arg;
  void *ret = NULL;

  if (t->getBlockAllSignals())
    MvrSignalHandler::blockCommonThisThread();
  
  if (dynamic_cast<MvrRetFunctor<void *>*>(t->getFunc()))
    ret = ((MvrRetFunctor<void *>*)t->getFunc())->invokeR();
  else
    t->getFunc()->invoke();
  return ret;
}

/**
   Initializes the internal structures which keep track of what thread is
   what. This is called by Mvria::init(), so the user will not normaly need
   to call this function themselves. This funtion *must* be called from the
   main thread of the application. In otherwords, it should be called by
   main().
*/

MVREXPORT void MvrThread::stopAll()
{
  MapType::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
  {
    (*iter).second->stopRunning();
  }
  ourThreadsMutex.unlock();
}

MVREXPORT void MvrThread::joinAll()
{
  MapType::iterator iter;
  MvrThread *thread = self();
  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter!=ourThreads.end(); ++iter)
  {
    if ((*iter).second->getJoinable() && thread && (thread != (*iter).second))
    {
      (*iter).second->doJoin();
    }
  }
  ourThreads.clear();

  if (thread != NULL){
    addThreadToMap(thread->myThread, thread);
  }
  ourThreadsMutex.unlock();
}

MVREXPORT MvrThread::MvrThread(bool blockAllSignals) :
          myName("(unnamed"),
          myRunning(false),
          myJoinable(false),
          myBlockAllSignals(blockAllSignals),
          myStarted(false),
          myFinished(false),
          myStrMap(),
          myFunc(0),
          myThread(),
#if defined(WIN32) && !defined(MINGW)
          myThreadHandle(0),
#else
          myPID(0),
#endif          
{
}

MVREXPORT MvrThread::MvrThread(MvrFunctor *func,
                               bool joinable,
                               bool blockAllSignals) :
          myRunning(false),
          myJoinable(false),
          myBlockAllSignals(blockAllSignals),
          myStarted(false),
          myFinished(false),
          myStrMap(),
          myFunc(func),
          myThread(),
#if defined(WIN32) && !defined(MINGW)
          myThreadHandle(0),
#else
          myPID(0),
#endif 
{
  create(funct, joinable);
}

#if !defined(WIN32) || defined(MINGW)
MVREXPORT MvrThread::~MvrThread()
{
  // Just make sure the thread is no longer in the map.
  removeThreadFromMap(myThread);
}
#endif

MVREXPORT int MvrThread::join(void **iret)
{
  int ret;
  ret = doJoin(iret);
  if (ret)
    return (ret)
  removeThreadFromMap(myThread);
  return 0;
}

MVREXPORT void MvrThread::setThreadName(const char *name)
{
  myName = name;
  std::string mutexLogName;
  mutexLogName = name;
  mutexLogName += "ThreadMutex";
  myMutex.setLogName(mutexLogName.c_str());
}

MVREXPORT bool MvrThread::isThreadStarted() const
{
  return myStarted;
}

MVREXPORT bool MvrThread::isThreadFinished() const
{
  return myFinished;
}

MVREXPORT const char *MvrThread::getThisThreadName(const)
{
  MvrThread *self;
  if ((self = MvrThread::self() != NULL))
    return self->getThreadName();
  else
    return ourUnknownThreadName.c_str();
}

MVREXPORT const MvrThread::ThreadType *MvrThread::getThisThread(void)
{
  MvrThread *self;
  if ((self = MvrThread::self() != NULL))
    return self->getThread();
  else
    return NULL; 
}

MVREXPORT MvrThread::ThreadType MvrThread::getThisOSThread(void)
{
  MvrThread *self;
  if ((self = MvrThread::self() != NULL))
    return self->getOSThread();
  else
#ifdef MINGW
  return (NULL, 0);
#else
  return 0;
#endif
}

// ourThreads is a vector on MINGW and a map on Linux and Windows
#ifdef MINGW
MvrThread* MvrThread::findThreadInMap(ThreadType t) 
{
	ourThreadsMutex.lock();
	for(MapType::iterator i = ourThreads.begin(); i != ourThreads.end(); ++i)
	{
		if(pthread_equal(t, (*i).first))
		{
			ourThreadsMutex.unlock();
			return (*i).second;
		}
	}
	ourThreadsMutex.unlock();
	return NULL;
}

void MvrThread::removeThreadFromMap(ThreadType t) 
{
	ourThreadsMutex.lock();
	MapType::iterator found = ourThreads.end();
	for(MapType::iterator i = ourThreads.begin(); i != ourThreads.end(); ++i)
		if(pthread_equal(t, (*i).first))
			found = i;
	if(found != ourThreads.end())
		ourThreads.erase(found);
	ourThreadsMutex.unlock();
}

void MvrThread::addThreadToMap(ThreadType pt, ArThread *at) 
{
	ourThreadsMutex.lock();
	ourThreads.push_back(std::pair<ThreadType, ArThread*>(pt, at));
	ourThreadsMutex.unlock();
}
#else
MvrThread *MvrThread::findThreadInMap(ThreadType pt)
{
  ourThreadsMutex.lock();
  MapType::iterator iter = ourThreads.find(pt);
  MvrThread *r = NULL;
  if (iter != ourThreads.end())
    r = (*iter).second;
  ourThreadsMutex.unlock();
  return r;
}
void MvrThread::removeThreadFromMap(ThreadType t)
{
  ourThreadsMutex.lock();
  MapType::iterator iter = ourThreads.find(t);
  if (iter != ourThreads.end())
    ourThreads.erase(iter);
  ourThreadsMutex.unlock();
}

void MvrThread::addThreadToMap(ThreadType pt, MvrThread *at)
{
  ourThreadsMutex.lock();
  ourThreads[pt] = at;
  ourThreadsMutex.unlock();
}
#endif