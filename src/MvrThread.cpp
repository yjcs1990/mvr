#include "MvrExport.h"
// MvrThread.cc -- Thread classes


#include "mvriaOSDef.h"
#include <errno.h>
#include <list>
#include "MvrThread.h"
#include "MvrLog.h"


MvrMutex MvrThread::ourThreadsMutex;
MvrThread::MapType MvrThread::ourThreads;
#if defined(WIN32) && !defined(MINGW)
std::map<HANDLE, MvrThread *> MvrThread::ourThreadHandles;
#endif
MVREXPORT MvrLog::LogLevel MvrThread::ourLogLevel = MvrLog::Verbose; // todo, instead of MVREXPORT move accessors into .cpp?
std::string MvrThread::ourUnknownThreadName = "unknown";

MVREXPORT void MvrThread::stopAll()
{
  MapType::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
    (*iter).second->stopRunning();
  ourThreadsMutex.unlock();
}

MVREXPORT void MvrThread::joinAll()
{
  MapType::iterator iter;
  MvrThread *thread = self();
  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
  {
    if ((*iter).second->getJoinable() && thread && (thread != (*iter).second))
    {
      (*iter).second->doJoin();
    }
  }
  ourThreads.clear();

  // KMC I think that the insert was there because "thread" still exists
  // but the entire map was cleared.

  // MPL BUG I'm not to sure why this insert was here, as far as I can
  // tell all it would do is make it so you could join the threads
  // then start them all up again, but I don't see much utility in
  // that so I'm not going to worry about it now
 
  if (thread != NULL) {
	addThreadToMap(thread->myThread, thread); // Note: Recursive lock of ourThreadsMutex!
    //ourThreads.insert(MapType::value_type(thread->myThread, thread));
  }
  ourThreadsMutex.unlock();
}

MVREXPORT MvrThread::MvrThread(bool blockAllSignals) :
  myName("(unnamed)"),
  myRunning(false),
  myJoinable(false),
  myBlockAllSignals(blockAllSignals),
  myStarted(false),
  myFinished(false),
  myStrMap(),
  myFunc(0),
  myThread(),
#if defined(WIN32) && !defined(MINGW)
  myThreadHandle(0)
#else
  myPID(0)
#endif

{
}

MVREXPORT MvrThread::MvrThread(ThreadType thread, bool joinable,
			    bool blockAllSignals) :
  myRunning(false),
  myJoinable(joinable),
  myBlockAllSignals(blockAllSignals),
  myStarted(false),
  myFinished(false),
  myStrMap(),
  myFunc(0),
  myThread(thread),
#if defined(WIN32) && !defined(MINGW)
  myThreadHandle(0)
#else
  myPID(0)
#endif
{
}

MVREXPORT MvrThread::MvrThread(MvrFunctor *func, bool joinable,
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
  myThreadHandle(0)
#else
  myPID(0)
#endif
{
  create(func, joinable);
}

#if !defined(WIN32) || defined(MINGW)
MVREXPORT MvrThread::~MvrThread()
{
  // Just make sure the thread is no longer in the map.
  removeThreadFromMap(myThread);
  /*
  ourThreadsMutex.lock();
  MapType::iterator iter = ourThreads.find(myThread);
  if (iter != ourThreads.end()) {
    ourThreads.erase(iter);
  }
  ourThreadsMutex.unlock();
  */
}
#endif 

MVREXPORT int MvrThread::join(void **iret)
{
  int ret;
  ret=doJoin(iret);
  if (ret)
    return(ret);

  removeThreadFromMap(myThread);
  /*
  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadsMutex.unlock();
 */
 
  return(0);
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

MVREXPORT const char *MvrThread::getThisThreadName(void) 
{
  MvrThread *self;
  if ((self = MvrThread::self()) != NULL)
    return self->getThreadName();
  else
    return ourUnknownThreadName.c_str();
}

MVREXPORT const MvrThread::ThreadType * MvrThread::getThisThread(void)
{
  MvrThread *self;
  if ((self = MvrThread::self()) != NULL)
    return self->getThread();
  else
    return NULL;
}

MVREXPORT MvrThread::ThreadType MvrThread::getThisOSThread(void)
{
  MvrThread *self;
  if ((self = MvrThread::self()) != NULL)
    return self->getOSThread();
  else
#ifdef MINGW
	return {NULL, 0};
#else
    return 0;
#endif
}


// ourThreads is a vector on MINGW and a map on Linux and Windows (where native ThreadType just happens to be a scalar and usable as a key in a map)

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

void MvrThread::addThreadToMap(ThreadType pt, MvrThread *at) 
{
	ourThreadsMutex.lock();
	ourThreads.push_back(std::pair<ThreadType, MvrThread*>(pt, at));
	ourThreadsMutex.unlock();
}
  
#else

MvrThread* MvrThread::findThreadInMap(ThreadType pt) 
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
	if (iter != ourThreads.end()) {
		ourThreads.erase(iter);
	}
	ourThreadsMutex.unlock();
}

void MvrThread::addThreadToMap(ThreadType pt, MvrThread *at) 
{
	ourThreadsMutex.lock();
	ourThreads[pt] = at;
	ourThreadsMutex.unlock();
}

#endif

