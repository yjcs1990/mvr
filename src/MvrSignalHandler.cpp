#include "MvrExport.h"
#include "MvrSignalHandler.h"
#include "MvrLog.h"
#include "mvriaInternal.h"

MvrSignalHandler *MvrSignalHandler::ourSignalHandler=0;
MvrStrMap MvrSignalHandler::ourSigMap;
sigset_t MvrSignalHandler::ourBlockSigSet;
sigset_t MvrSignalHandler::ourHandleSigSet;
std::list<MvrFunctor1<int>*> MvrSignalHandler::ourHandlerList;


void MvrSignalHandler::signalCB(int sig)
{
  std::list<MvrFunctor1<int>*>::iterator iter;

  MvrLog::log(MvrLog::Verbose,
	     "MvrSignalHandler::runThread: Received signal '%s' Number %d ",
	     ourSigMap[sig].c_str(), sig);
  for (iter=ourHandlerList.begin(); iter != ourHandlerList.end(); ++iter)
    (*iter)->invoke(sig);
  if (ourHandlerList.begin() == ourHandlerList.end())
    MvrLog::log(MvrLog::Terse,
  "MvrSignalHandler::runThread: No handler function. Unhandled signal '%s' Number %d", 
	       ourSigMap[sig].c_str(), sig);
}  

/**
   Sets up the signal handling for a non-threaded program. When the program
   This uses the system call signal(2). This should not be used if you have
   a threaded program.
   @see createHandlerThreaded
*/
MVREXPORT void MvrSignalHandler::createHandlerNonThreaded()
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
   signal to be sent.  By default all MvrThread instances block all
   signals. Thus the signal is sent to the signal handler thread. This
   will allow the other threads to continue uninterrupted and not skew
   their timing loops.  
   @see createHandlerNonThreaded 
**/
MVREXPORT void MvrSignalHandler::createHandlerThreaded()
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
MVREXPORT void MvrSignalHandler::blockCommon()
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
MVREXPORT void MvrSignalHandler::unblockAll()
{
  sigemptyset(&ourBlockSigSet);
}

/**
   Block the given signal. Call this before calling createHandlerNonThreaded
   or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void MvrSignalHandler::block(Signal sig)
{
  sigaddset(&ourBlockSigSet, sig);
}

/**
   Unblock the given signal. Call this before calling createHandlerNonThreaded
   or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void MvrSignalHandler::unblock(Signal sig)
{
  sigdelset(&ourBlockSigSet, sig);
}

/**
   Handle the given signal. All the handler callbacks will be called with this
   signal when it is received. Call this before calling
   createHandlerNonThreaded or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void MvrSignalHandler::handle(Signal sig)
{
  unblock(sig);
  sigaddset(&ourHandleSigSet, sig);
}

/**
   Do not handle the given signal. Call this before calling
   createHandlerNonThreaded or createHandlerThreaded.
   @param sig the number of the signal
*/
MVREXPORT void MvrSignalHandler::unhandle(Signal sig)
{
  sigdelset(&ourHandleSigSet, sig);
}

/**
   Add a handler callback to the list of callbacks. When there is a signal
   sent to the process, the list of callbacks are invoked and passed the signal
   number.
   @param func functor created from MvrFunctorC1<int> which refers to the 
   function to call.
   @param position whether to place the functor first or last
*/
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

/**
   Remove a handler callback from the list of callbacks. 
   @param func functor created from MvrFunctorC1<int> which refers to the 
   function to call.
*/
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

/**
   Get a pointer to the single instance of the MvrSignalHandler. The signal
   handler uses the singleton model, which means there can only be one
   instance of MvrSignalHandler. If the single instance of MvrSignalHandler
   has not been created, getHandler will create it. This is how the handler
   should be created.
   @return returns a pointer to the instance of the signal handler
*/
MVREXPORT MvrSignalHandler * MvrSignalHandler::getHandler()
{
  if (!ourSignalHandler)
    ourSignalHandler=new MvrSignalHandler;

  return(ourSignalHandler);
}

/**
   Block all the common signals for the calling thread. The calling thread
   will never receive the common signals which are SIGHUP, SIGINT, SIGQUIT,
   and SIGTERM. This function can be called at any time.
*/
MVREXPORT void MvrSignalHandler::blockCommonThisThread()
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

MVREXPORT void MvrSignalHandler::blockAllThisThread()
{
  sigset_t fullSet;
  sigfillset(&fullSet);
  pthread_sigmask(SIG_SETMASK, &fullSet, 0);
}


MvrSignalHandler::MvrSignalHandler() :
  ourIgnoreQUIT(false)
{
  setThreadName("MvrSignalHandler");
  initSigMap();
}

MvrSignalHandler::~MvrSignalHandler()
{
}

MVREXPORT void * MvrSignalHandler::runThread(void *arg)
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

MVREXPORT void MvrSignalHandler::initSigMap()
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

MVREXPORT const char *MvrSignalHandler::nameSignal(int sig)
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
