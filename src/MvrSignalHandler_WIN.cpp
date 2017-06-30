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
#include "ariaOSDef.h"
#include "MvrSignalHandler.h"
#include "MvrLog.h"


ArSignalHandler *ArSignalHandler::ourSignalHandler=0;
ArStrMap ArSignalHandler::ourSigMap;
std::list<ArFunctor1<int>*> ArSignalHandler::ourHandlerList;


MVREXPORT void ArSignalHandler::signalCB(int sig)
{
  std::list<ArFunctor1<int>*>::iterator iter;

  for (iter=ourHandlerList.begin(); iter != ourHandlerList.end(); ++iter)
    (*iter)->invoke(sig);
  if (ourHandlerList.begin() == ourHandlerList.end())
    ArLog::log(MvrLog::Terse, "MvrSignalHandler::runThread: No handler function. Unhandled signal '%s'", ourSigMap[sig].c_str());
}  

MVREXPORT void ArSignalHandler::createHandlerNonThreaded()
{
}

MVREXPORT void ArSignalHandler::createHandlerThreaded()
{
  getHandler()->create(false);
}

MVREXPORT void ArSignalHandler::blockCommon()
{
}

MVREXPORT void ArSignalHandler::unblockAll()
{
}

MVREXPORT void ArSignalHandler::block(Signal sig)
{
}

MVREXPORT void ArSignalHandler::unblock(Signal sig)
{
}

MVREXPORT void ArSignalHandler::handle(Signal sig)
{
}

MVREXPORT void ArSignalHandler::unhandle(Signal sig)
{
}

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


MVREXPORT ArSignalHandler * ArSignalHandler::getHandler()
{
  if (!ourSignalHandler)
    ourSignalHandler=new ArSignalHandler;

  return(ourSignalHandler);
}

MVREXPORT void ArSignalHandler::blockCommonThisThread()
{
}

MVREXPORT void ArSignalHandler::blockAllThisThread()
{
}

ArSignalHandler::ArSignalHandler()
{
  setThreadName("MvrSignalHandler");
  initSigMap();
}

ArSignalHandler::~MvrSignalHandler()
{
}

MVREXPORT void * ArSignalHandler::runThread(void *arg)
{
  threadStarted();
  threadFinished(); // ??
  return(0);
}

void ArSignalHandler::initSigMap()
{
  ourSigMap[SigHUP]="SIGHUP";
  ourSigMap[SigINT]="SIGINT";
  ourSigMap[SigQUIT]="SIGQUIT";
  ourSigMap[SigILL]="SIGILL";
  ourSigMap[SigTRAP]="SIGTRAP";
  ourSigMap[SigABRT]="SIGABRT";
  //ourSigMap[SigIOT]="SIGIOT";
  ourSigMap[SigBUS]="SIGBUS";
  ourSigMap[SigFPE]="SIGFPE";
  ourSigMap[SigKILL]="SIGKILL";
  ourSigMap[SigUSR1]="SIGUSR1";
  ourSigMap[SigSEGV]="SIGSEGV";
  ourSigMap[SigUSR2]="SIGUSR2";
  ourSigMap[SigPIPE]="SIGPIPE";
  ourSigMap[SigALRM]="SIGALRM";
  ourSigMap[SigTERM]="SIGTERM";
  //ourSigMap[SigSTKFLT]="SIGSTKFLT";
  ourSigMap[SigCHLD]="SIGCHLD";
  ourSigMap[SigCONT]="SIGCONT";
  ourSigMap[SigSTOP]="SIGSTOP";
  ourSigMap[SigTSTP]="SIGTSTP";
  ourSigMap[SigTTIN]="SIGTTIN";
  ourSigMap[SigTTOU]="SIGTTOU";
  ourSigMap[SigURG]="SIGURG";
  ourSigMap[SigXCPU]="SIGXCPU";
  ourSigMap[SigXFSZ]="SIGXFSZ";
  ourSigMap[SigVTALRM]="SIGVTALRM";
  ourSigMap[SigPROF]="SIGPROF";
  ourSigMap[SigWINCH]="SIGWINCH";
  ourSigMap[SigIO]="SIGIO";
  ourSigMap[SigPWR]="SIGPWR";
}

MVREXPORT const char * ArSignalHandler::nameSignal(int sig)
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
