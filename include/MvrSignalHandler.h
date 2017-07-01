#ifndef MVRSIGNALHANDLER_H
#define MVRSIGNALHANDLER_H


#include <list>
#ifdef WIN32
#else
#include <signal.h>
#endif
#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"
#include "MvrFunctor.h"


/// Signal handling class
/**
   This is a signal handling class. It has both a threaded and non-threaded
   mode of operation. The non-threaded mode will work in a threaded application
   but it is best to use the threaded mode. The benefit of the threaded mode
   is that if the signal incures some processing, but does not shutdown the
   program (ie. SIGUSR1 or SIGUSR2), the threaded mode will handle the signal
   in its own thread and hopefully that will not hurt the performance of the
   tight loop robot control. Exaclty how much performance you get out of this
   depends on your machines physical hardware and exactly what the processing
   the signal handler does. For instance, a multi-processor machine has a much
   greater chance of the signal handler not interfering with the robot control
   loop.

   See the Mvria main class for how to initialize a default setup of the
   signal handling.

   There are functions to block, unblock, handle and unhandle signals. These
   functions all must be called before creating the signalhandler. In either
   single or multi-threaded mode. The functions to block and handle signals
   creates a set of blocking and handling which is then used by the create
   functions to tell the Linux kernel what to do.

   In the threaded mode, there is a signal handler thread that is created.
   That thread is created in a detached state, which means it can not be
   joined on. When the program exits, the signal handler thread will be
   ignored and that thread will never exit its run loop. This is perfectly
   fine behavior. There is no state that can be messed up in this fashion.
   It is just easier to exit the program than to try to wake up that thread
   and get it to exit itself.

   This class is for Linux only. Windows has virtualy no support for signals
   and the little support that it does have is not realy usefull. There is
   an empty implementation of this class for Windows so that code can compile
   in both Linux and Windows. Just do not expect the code that uses this
   signal handling to do anything in Windows. This should not be a problem
   since signals are not used in Windows.
*/
class MvrSignalHandler : public MvrASyncTask
{
public:

  typedef enum
  {
    SigHUP=1, SigINT, SigQUIT, SigILL, SigTRAP, SigABRT, SigBUS, SigFPE, 
    SigKILL, SigUSR1, SigSEGV, SigUSR2, SigPIPE, SigALRM, SigTERM, SigSTKFLT, 
    SigCHLD, SigCONT, SigSTOP, SigTSTP, SigTTIN, SigTTOU, SigURG,
    SigXCPU, SigXFSZ, SigVTALRM, SigPROF, SigWINCH, SigIO, SigPWR
  } Signal;

  /// Setup the signal handling for a non-threaded program
  MVREXPORT static void createHandlerNonThreaded();

  /// Setup the signal handling for a multi-threaded program
  MVREXPORT static void createHandlerThreaded();

  /// Block all the common signals the kill a program
  MVREXPORT static void blockCommon();

  /// Unblock all the signals
  MVREXPORT static void unblockAll();

  /// Block the given signal
  MVREXPORT static void block(Signal sig);

  /// Unblock the given signal
  MVREXPORT static void unblock(Signal sig);

  /// Handle the given signal
  MVREXPORT static void handle(Signal sig);

  /// Dont handle the given signal
  MVREXPORT static void unhandle(Signal sig);

  /// Add a handler callback
  MVREXPORT static void addHandlerCB(MvrFunctor1<int> *func,
				    MvrListPos::Pos position);

  /// Remove a handler callback
  MVREXPORT static void delHandlerCB(MvrFunctor1<int> *func);

  /// Removes all the handlers
  MVREXPORT static void delAllHandlerCBs(void);

  /// Get a pointer to the single MvrSignalHandler instance
  MVREXPORT static MvrSignalHandler * getHandler();

  /// Get the name of the given signal
  MVREXPORT static const char * nameSignal(int sig);

  /// Block all the common signals for the calling thread only
  MVREXPORT static void blockCommonThisThread();

  /// Block all the signals for the calling thread only
  MVREXPORT static void blockAllThisThread();

  /// Destructor
  virtual ~MvrSignalHandler();

  MVREXPORT virtual void * runThread(void *arg);

  MVREXPORT static void signalCB(int sig);

  MVREXPORT static void logThread(void);
protected:

  MvrSignalHandler();

  static void initSigMap();

  bool ourIgnoreQUIT;

  static MvrSignalHandler *ourSignalHandler;
  static MvrStrMap ourSigMap;
#ifdef WIN32
#else
  static sigset_t ourBlockSigSet;
  static sigset_t ourHandleSigSet;
#endif
  static std::list<MvrFunctor1<int>*> ourHandlerList;
};


#endif // MVRSIGNALHANDLER_H
