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
#include "MvrSignalHandler.h"
#include "MvrLog.h"
#include "ariaInternal.h"

ArSignalHandler *ArSignalHandler::ourSignalHandler=0;
ArStrMap ArSignalHandler::ourSigMap;
sigset_t ArSignalHandler::ourBlockSigSet;
sigset_t ArSignalHandler::ourHandleSigSet;
std::list<ArFunctor1<int>*> ArSignalHandler::ourHandlerList;


void ArSignalHandler::signalCB(int sig)
{
  std::list<ArFunctor1<int>*>::iterator iter;

  ArLog::log(MvrLog::Verbose,
	     "MvrSignalHandler::runThread: Received signal '%s' Number %d ",
	     ourSigMap[sig].c_str(), sig);
  for (iter=ourHandlerList.begin(); iter != ourHandlerList.end(); ++iter)
    (*iter)->invoke(sig);
  if (ourHandlerList.begin() == ourHandlerList.end())
    ArLog::log(MvrLog::Terse,
  "MvrSignalHandler::runThread: No handler function. Unhandled signal '%s' Number %d", 
	       ourSigMap[sig].c_str(), sig);
}  

/**
   Sets up the signal handling for a non-threaded program. When the program
   This uses the system call signal(2). This should not be used if you have
   a threaded program.
   @see createHandlerThreaded
*/
MVREXPORT void ArSignalHandler::createHandlerNonThreaded()
{
  int i;
  initSigMap();
  signal(SigSEGV, &signalCB);
  signal(SigFPE, &signalCB);
  for (i=1; i <= SigPWR; ++i)
  {
    if (sigismember(&ourBlockSigSet, i))
      signal(i, SIG_IGN);
    if (sigismember(&ourHandleSigSet, i))
      signal(i, &signalCB);
  }
  
}

/**
   Sets up the signal handling for a threaded program. This call is
   only useful for Linux. This will create a dedicated thread in which
   to handle signals. The thread calls sigwait(3) and waits for a
   signal to be sent.  By default all ArThread instances block all
   signals. Thus the signal is sent to the signal handler thread. This
   will allow the other threads to continue uninterrupted and not skew
   their timing loops.  
   @see createHandlerNonThreaded 
**/
MVREXPORT void ArSignalHandler::createHandlerThreaded()
{
  signal(SigSEGV, &signalCB);
  signal(SigFPE, &signalCB);
  getHandler()->create(false);
}

/**
   Sets the signal handler to block all the common signals. The
   'common' signals are SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGSEGV, and
   SIGPIPE. Call this before calling createHandlerNonThreaded or
   createHandlerThreaded.  
**/
MVREXPORT void ArSignalHandler::blockCommon()
{
  unblockAll();
  block(SigHUP);
  block(SigPIPE);
  block(SigINT);
  block(SigQUIT);
  block(SigTERM);
}

/**
   Unblock all the signals. Call this before calling createHandlerNonThreaded
   or createHandlerThreaded.
*/
MVREXPORT void ArSignalHandler::unblockAll()
{
  sigemptyset(&ourBlockSigSet);
}

/**
   Block the given signal. Call this before calling createHandlerNonThreaded
   or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void ArSignalHandler::block(Signal sig)
{
  sigaddset(&ourBlockSigSet, sig);
}

/**
   Unblock the given signal. Call this before calling createHandlerNonThreaded
   or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void ArSignalHandler::unblock(Signal sig)
{
  sigdelset(&ourBlockSigSet, sig);
}

/**
   Handle the given signal. All the handler callbacks will be called with this
   signal when it is received. Call this before calling
   createHandlerNonThreaded or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void ArSignalHandler::handle(Signal sig)
{
  unblock(sig);
  sigaddset(&ourHandleSigSet, sig);
}

/**
   Do not handle the given signal. Call this before calling
   createHandlerNonThreaded or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void ArSignalHandler::unhandle(Signal sig)
{
  sigdelset(&ourHandleSigSet, sig);
}

/**
   Add a handler callback to the list of callbacks. When there is a signal
   sent to the process, the list of callbacks are invoked and passed the signal
   number.
   @param func functor created from ArFunctorC1<int> which refers to the 
   function to call.
   @param position whether to place the functor first or last
*/
MVREXPORT void ArSignalHandler::addHandlerCB(MvrFunctor1<int> *func,
					    ArListPos::Pos position)
{

  if (position == ArListPos::FIRST)
    ourHandlerList.push_front(func);
  else if (position == ArListPos::LAST)
    ourHandlerList.push_back(func);
  else
    ArLog::log(MvrLog::Terse, 
	       "MvrSignalHandler::addHandler: Invalid position.");
}

/**
   Remove a handler callback from the list of callbacks. 
   @param func functor created from ArFunctorC1<int> which refers to the 
   function to call.
*/
MVREXPORT void ArSignalHandler::delHandlerCB(MvrFunctor1<int> *func)
{
  ourHandlerList.remove(func);
}

/**
   Removes all of the signal handler callback from the list of callbacks. 
**/
MVREXPORT void ArSignalHandler::delAllHandlerCBs(void)
{
  ourHandlerList.clear();
}

/**
   Get a pointer to the single instance of the ArSignalHandler. The signal
   handler uses the singleton model, which means there can only be one
   instance of ArSignalHandler. If the single instance of ArSignalHandler
   has not been created, getHandler will create it. This is how the handler
   should be created.
   @return returns a pointer to the instance of the signal handler
*/
MVREXPORT ArSignalHandler * ArSignalHandler::getHandler()
{
  if (!ourSignalHandler)
    ourSignalHandler=new ArSignalHandler;

  return(ourSignalHandler);
}

/**
   Block all the common signals for the calling thread. The calling thread
   will never receive the common signals which are SIGHUP, SIGINT, SIGQUIT,
   and SIGTERM. This function can be called at any time.
*/
MVREXPORT void ArSignalHandler::blockCommonThisThread()
{
  sigset_t commonSet;
  sigemptyset(&commonSet);
  sigaddset(&commonSet, SigHUP);
  sigaddset(&commonSet, SigPIPE);
  sigaddset(&commonSet, SigINT);
  sigaddset(&commonSet, SigQUIT);
  sigaddset(&commonSet, SigTERM);
  //sigaddset(&commonSet, SigSEGV);
  pthread_sigmask(SIG_SETMASK, &commonSet, 0);
}

MVREXPORT void ArSignalHandler::blockAllThisThread()
{
  sigset_t fullSet;
  sigfillset(&fullSet);
  pthread_sigmask(SIG_SETMASK, &fullSet, 0);
}


ArSignalHandler::ArSignalHandler() :
  ourIgnoreQUIT(false)
{
  setThreadName("MvrSignalHandler");
  initSigMap();
}

ArSignalHandler::~MvrSignalHandler()
{
}

MVREXPORT void * ArSignalHandler::runThread(void *arg)
{
  setThreadName("MvrSignalHandler");
  threadStarted();

  // I think the old code was broken in that it didn't block all the
  // signals it wanted to wait for, which sigwait is supposed to
  // do... it also didn't check the return... for some reason system
  // on a debian box (at least a newer one) causes sigwait to return
  // with an error state (return of 4)... the old sigwait from rh 7.x
  // said it never returned an error... I don't entirely understand
  // it, and thats why both blocks of code are here

  // old code
  /*
  int sig;
  
  pthread_sigmask(SIG_SETMASK, &ourBlockSigSet, 0);

  while (myRunning)
  {
    sigwait(&ourHandleSigSet, &sig);
    signalCB(sig);
  }

  return(0);
*/
  // new code
  int sig = 0;
  
  while (myRunning)
  {
    pthread_sigmask(SIG_SETMASK, &ourBlockSigSet, 0);
    pthread_sigmask(SIG_BLOCK, &ourHandleSigSet, 0);

    if (sigwait(&ourHandleSigSet, &sig) == 0)
      signalCB(sig);
  }
  return(0);
}

MVREXPORT void ArSignalHandler::initSigMap()
{
  ourSigMap[SIGHUP]="SIGHUP";
  ourSigMap[SIGINT]="SIGINT";
  ourSigMap[SIGQUIT]="SIGQUIT";
  ourSigMap[SIGILL]="SIGILL";
  ourSigMap[SIGTRAP]="SIGTRAP";
  ourSigMap[SIGABRT]="SIGABRT";
#ifdef linux
  ourSigMap[SIGIOT]="SIGIOT";
#endif
  ourSigMap[SIGBUS]="SIGBUS";
  ourSigMap[SIGFPE]="SIGFPE";
  ourSigMap[SIGKILL]="SIGKILL";
  ourSigMap[SIGUSR1]="SIGUSR1";
  ourSigMap[SIGSEGV]="SIGSEGV";
  ourSigMap[SIGUSR2]="SIGUSR2";
  ourSigMap[SIGPIPE]="SIGPIPE";
  ourSigMap[SIGALRM]="SIGALRM";
  ourSigMap[SIGTERM]="SIGTERM";
  //ourSigMap[SIGSTKFLT]="SIGSTKFLT";
  ourSigMap[SIGCHLD]="SIGCHLD";
  ourSigMap[SIGCONT]="SIGCONT";
  ourSigMap[SIGSTOP]="SIGSTOP";
  ourSigMap[SIGTSTP]="SIGTSTP";
  ourSigMap[SIGTTIN]="SIGTTIN";
  ourSigMap[SIGTTOU]="SIGTTOU";
  ourSigMap[SIGURG]="SIGURG";
  ourSigMap[SIGXCPU]="SIGXCPU";
  ourSigMap[SIGXFSZ]="SIGXFSZ";
  ourSigMap[SIGVTALRM]="SIGVTALRM";
  ourSigMap[SIGPROF]="SIGPROF";
  ourSigMap[SIGWINCH]="SIGWINCH";
  ourSigMap[SIGIO]="SIGIO";
#ifdef linux
  ourSigMap[SIGPWR]="SIGPWR";
#endif
}

MVREXPORT const char *ArSignalHandler::nameSignal(int sig)
{
  return(ourSigMap[sig].c_str());
}

MVREXPORT void ArSignalHandler::logThread(void)
{
  if (ourSignalHandler != NULL)
    ourSignalHandler->logThreadInfo();
  else
    ArLog::log(MvrLog::Normal, "No signal handler thread running");
}
