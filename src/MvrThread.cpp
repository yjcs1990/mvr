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
#include "MvrExport.h"
// ArThread.cc -- Thread classes


#include "ariaOSDef.h"
#include <errno.h>
#include <list>
#include "MvrThread.h"
#include "MvrLog.h"


ArMutex ArThread::ourThreadsMutex;
ArThread::MapType ArThread::ourThreads;
#if defined(WIN32) && !defined(MINGW)
std::map<HANDLE, ArThread *> ArThread::ourThreadHandles;
#endif
MVREXPORT ArLog::LogLevel ArThread::ourLogLevel = ArLog::Verbose; // todo, instead of MVREXPORT move accessors into .cpp?
std::string ArThread::ourUnknownThreadName = "unknown";

MVREXPORT void ArThread::stopAll()
{
  MapType::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
    (*iter).second->stopRunning();
  ourThreadsMutex.unlock();
}

MVREXPORT void ArThread::joinAll()
{
  MapType::iterator iter;
  ArThread *thread = self();
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

MVREXPORT ArThread::ArThread(bool blockAllSignals) :
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

MVREXPORT ArThread::ArThread(ThreadType thread, bool joinable,
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

MVREXPORT ArThread::ArThread(MvrFunctor *func, bool joinable,
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
MVREXPORT ArThread::~MvrThread()
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

MVREXPORT int ArThread::join(void **iret)
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

MVREXPORT void ArThread::setThreadName(const char *name)
{ 
  myName = name; 
  std::string mutexLogName;
  mutexLogName = name;
  mutexLogName += "ThreadMutex";
  myMutex.setLogName(mutexLogName.c_str());
}

MVREXPORT bool ArThread::isThreadStarted() const
{
  return myStarted;
}

MVREXPORT bool ArThread::isThreadFinished() const
{
  return myFinished;
}

MVREXPORT const char *ArThread::getThisThreadName(void) 
{
  ArThread *self;
  if ((self = ArThread::self()) != NULL)
    return self->getThreadName();
  else
    return ourUnknownThreadName.c_str();
}

MVREXPORT const ArThread::ThreadType * ArThread::getThisThread(void)
{
  ArThread *self;
  if ((self = ArThread::self()) != NULL)
    return self->getThread();
  else
    return NULL;
}

MVREXPORT ArThread::ThreadType ArThread::getThisOSThread(void)
{
  ArThread *self;
  if ((self = ArThread::self()) != NULL)
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

ArThread* ArThread::findThreadInMap(ThreadType t) 
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

void ArThread::removeThreadFromMap(ThreadType t) 
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

void ArThread::addThreadToMap(ThreadType pt, ArThread *at) 
{
	ourThreadsMutex.lock();
	ourThreads.push_back(std::pair<ThreadType, ArThread*>(pt, at));
	ourThreadsMutex.unlock();
}
  
#else

ArThread* ArThread::findThreadInMap(ThreadType pt) 
{
	ourThreadsMutex.lock();
	MapType::iterator iter = ourThreads.find(pt);
	ArThread *r = NULL;
	if (iter != ourThreads.end())
		r = (*iter).second;
	ourThreadsMutex.unlock();
	return r;
}

void ArThread::removeThreadFromMap(ThreadType t) 
{  
	ourThreadsMutex.lock();
	MapType::iterator iter = ourThreads.find(t);
	if (iter != ourThreads.end()) {
		ourThreads.erase(iter);
	}
	ourThreadsMutex.unlock();
}

void ArThread::addThreadToMap(ThreadType pt, ArThread *at) 
{
	ourThreadsMutex.lock();
	ourThreads[pt] = at;
	ourThreadsMutex.unlock();
}

#endif

