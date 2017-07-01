#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSignalHandler.h"
#include "MvrLog.h"


MvrSignalHandler *MvrSignalHandler::ourSignalHandler=0;
MvrStrMap MvrSignalHandler::ourSigMap;
std::list<MvrFunctor1<int>*> MvrSignalHandler::ourHandlerList;


MVREXPORT void MvrSignalHandler::signalCB(int sig)
{
  std::list<MvrFunctor1<int>*>::iterator iter;

  for (iter=ourHandlerList.begin(); iter != ourHandlerList.end(); ++iter)
    (*iter)->invoke(sig);
  if (ourHandlerList.begin() == ourHandlerList.end())
    MvrLog::log(MvrLog::Terse, "MvrSignalHandler::runThread: No handler function. Unhandled signal '%s'", ourSigMap[sig].c_str());
}  

MVREXPORT void MvrSignalHandler::createHandlerNonThreaded()
{
}

MVREXPORT void MvrSignalHandler::createHandlerThreaded()
{
  getHandler()->create(false);
}

MVREXPORT void MvrSignalHandler::blockCommon()
{
}

MVREXPORT void MvrSignalHandler::unblockAll()
{
}

MVREXPORT void MvrSignalHandler::block(Signal sig)
{
}

MVREXPORT void MvrSignalHandler::unblock(Signal sig)
{
}

MVREXPORT void MvrSignalHandler::handle(Signal sig)
{
}

MVREXPORT void MvrSignalHandler::unhandle(Signal sig)
{
}

MVREXPORT void MvrSignalHandler::addHandlerCB(MvrFunctor1<int> *func,
					    MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    ourHandlerList.push_front(func);
  else if (position == MvrListPos::LAST)
    ourHandlerList.push_back(func);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrSignalHandler::addHandler: Invalid position.");
}

MVREXPORT void MvrSignalHandler::delHandlerCB(MvrFunctor1<int> *func)
{
  ourHandlerList.remove(func);
}

/**
   Removes all of the signal handler callback from the list of callbacks. 
**/
MVREXPORT void MvrSignalHandler::delAllHandlerCBs(void)
{
  ourHandlerList.clear();
}


MVREXPORT MvrSignalHandler * MvrSignalHandler::getHandler()
{
  if (!ourSignalHandler)
    ourSignalHandler=new MvrSignalHandler;

  return(ourSignalHandler);
}

MVREXPORT void MvrSignalHandler::blockCommonThisThread()
{
}

MVREXPORT void MvrSignalHandler::blockAllThisThread()
{
}

MvrSignalHandler::MvrSignalHandler()
{
  setThreadName("MvrSignalHandler");
  initSigMap();
}

MvrSignalHandler::~MvrSignalHandler()
{
}

MVREXPORT void * MvrSignalHandler::runThread(void *arg)
{
  threadStarted();
  threadFinished(); // ??
  return(0);
}

void MvrSignalHandler::initSigMap()
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

MVREXPORT const char * MvrSignalHandler::nameSignal(int sig)
{
  return(ourSigMap[sig].c_str());
}

MVREXPORT void MvrSignalHandler::logThread(void)
{
  if (ourSignalHandler != NULL)
    ourSignalHandler->logThreadInfo();
  else
    MvrLog::log(MvrLog::Normal, "No signal handler thread running");
}
