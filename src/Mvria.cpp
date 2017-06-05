/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : Mvria.cpp
 > Description  : Some head file 
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月04日
***************************************************************************************************/
#include "MvrExport.h"
#include "Mvria.h"
#include "MvrSocket.h"
#include "MvrSingalHandler.h"
#include "MvrKeyHandler.h"
#include "MvrJoyHandler.h"
#ifndef MVRINTERFACE
#include "MvrModuleLoader.h"
#include "MvrRobotJoyHandler.h"
#include "MvrSystemStatus.h"
#endif  // MVRINTERFACE

/// To register PTZ types with PTZConnector
#include "MvrPTZConnector.h"
#include "MvrRVisionPTZ.h"
#include "MvrVCC4.h"
#include "MvrDPPTU.h"
#include "MvrSonyPTZ.h"

MvrGlobalFunctor1<int> Mvria::ourSignalHandlerCB(&Mvria::signalHandlerCB);
MvrMutex Mvria::ourShuttingDownMutex;
bool Mvria::ourShuttingDown = false;
bool Mvria::ourExiting      = false;
std::string Mvria::ourDirectory = "";
std::list<MvrFunctor *> Mvria::ourInitCBs;
std::list<MvrFunctor *> Mvria::ourUninitCBs;
MvrKeyHandler *Mvria::ourKeyHandler = NULL;
MvrKeyHandler *Mvria::ourJoyHandler = NULL;
bool Mvria::ourInited       = false;
bool Mvria::ourRunning      = false;
MvrMutex Mvria::ourExitCallbacksMutex;
std::multimap<int, MvrFunctor *> Mvria::ourExitCallbacks;
bool Mvria::ourSigHandleExitNotShutdown = true;
std::multimap<int, MvrRetFunctor<bool> *> Mvria::ourParseArgCBs;
MvrLog::LogLevel Mvria::ourParseArgsLogLevel = MvrLog::Verbose;
std::multimap<int, MvrFunctor *> Mvria::ourlogOptionsCBs;
MvrLog::LogLevel Mvria::ourExitCallbacksLogLevel = MvrLog::Verbose;
std::map<std::string, MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *, MvrStrCaseCmpOp> Mvria::ourDeviceConnectionCreatorMap;
std::string Mvria::ourDeviceConnectionTypes;
std::string Mvria::ourDeviceConnectionChoices = "Choices: ";

#ifndef MVRINTERFACE
std::list<MvrRobot *> Mvria::ourRobots;
MvrConfig Mvria::ourConfig;
MvrRobotJoyHandler *Mvria::ourRobotJoyHandler = NULL;
MvrStringInfoGroup Mvria::ourInfoGroup;
int Mvria::ourMaxNumLasers = 2;
int Mvria::ourMaxNumSonarBoards = 1;
int Mvria::ourMaxNumBatteries = 1;
int Mvria::ourMaxNumLCDs = 1;
std::map<std::string, MvrRetFunctor2<MvrLaser *, int, const char *> *, MvrStrCaseCmpOp> Mvria::ourLaserCreatorMap;
std::string Mvria::ourLaserTypes;
std::string Mvria::ourLaserChoices = "Choices:";
std::map<std::string, MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *, MvrStrCaseCmpOp> Mvria::ourBatteryCreatorMap;
std::string Mvria::ourBatteryTypes;
std::string Mvria::ourBatteryChoices = "Choices:";
std::map<std::string, MvrRetFunctor2<MvrLCDMTX *, int, const char *> *, MvrStrCaseCmpOp> Mvria::ourLCDCreatorMap;
std::string Mvria::ourLCDTypes;
std::string Mvria::ourLCDChoices = "Choices:";
std::map<std::string, MvrRetFunctor2<MvrSonarMTX *, int, const char *> *, MvrStrCaseCmpOp> Mvria::ourSonarCreatorMap;
std::string Mvria::ourSonarTypes;
std::string Mvria::ourSonarChoices = "Choices:";

size_t Mvria::ourMaxNumVideoDevices = 8;
size_t Mvria::ourMaxNumPTZs = 8;
#endif MVRINTERFACE

std::string Mvria::ourIdentifier = "generic";

/**
   This must be called first before any other Mvria functions.
   It initializes the thread layer and the signal handling method. 

   For Linux the default signal handling method is to intercept OS signals
   in a dedicated "signal handling thread", which cleanly closes down the
   program, causes all the instances of MvrRobot to stop their run loop and
   disconnect from their robot. The program will exit on the following signals:
   SIGHUP, SIGINT, SIGQUIT, and SIGTERM.

   This method also adds the file /etc/Mvria.args and the environment variable
   MVRIAARGS as locations for MvrArgumentParser to obtain default argument values
   from. 

   @param method the method in which to handle signals. Defaulted to SIGHANDLE_SINGLE, or the method indicated by the MVRIA_SIGHANDLE_METHOD environment variable (NONE, SINGLE or THREAD), if it exists. 
   @param initSockets specify whether or not to initialize the socket layer. This is only meaningfull for Windows. Defaulted to true.
   @param sigHandleExitNotShutdown if this is true and a signal
   happens Mvria will use exit() to quit instead of shutdown(), false will
   still use shutdown which is the old behavior.
 */
MVREXPORT void Mvria::init(SigHandleMethod method, bool initSockets, bool sigHandleExitNotShutdown)
{
#ifndef MVRINTERFACE
  /// get this here so that the program update can be accurate
  MvrSystemStatus::getUptime();
#endif

  std::list<MvrFunctor *>::iterator iter;
  std::string str;
  char buf[1024];

  /// make an MvrTime and set it to now so that the good clock value will be set
  MvrTime timeInit;
  timeInit.setToNow();

  ourShuttingDownMutex.setLogName("Mvria::ourShuttingDownMutex");
  ourExitCallbackMutex.setLogName("Mvria::ourExitCallbacksMutex");
  if (ourInited == true)
    return;
  
  ourRunning = true;
#ifndef WIN32
  srand48(time(NULL));
#endif  // WINew

  MvrThread::init(); 

  char *overrideSigMethod = getenv("MVRIA_SIGHANDLE_METHOD");
  if (overrideSigMethod)
  {
    MvrLog::log(MvrLog::Terse, "Overriding signal handler method with %s from MVRIA_SIGHANDLE_METHOD environment variable.", overrideSigMethod);
    if (!strcmp(overrideSigMethod, "NONE"))
      method = SIGHANDLE_NONE;
    else if (!strcmp(overrideSigMethod, "SINGLE"))
      method = SIGHANDLE_SINGLE;
    else if (!strcmp(overrideSigMethod, "THREAD"))
      method = SIGHANDLE_THREAD;
  }

  if (method != SIGHANDLE_NONE)
  {
    MvrSignalHandler::addHandlerCB(&ourSignalHandlerCB, MvrListPos::LAST);
    MvrSignalHandler::blockCommon();
    MvrSignalHandler::handle(MvrSignalHandler::SigHUP);
    MvrSignalHandler::handle(MvrSignalHandler::SigINT);
    MvrSignalHandler::handle(MvrSignalHandler::SigQUIT);
    MvrSignalHandler::handle(MvrSignalHandler::SigTERM);
    MvrSignalHandler::handle(MvrSignalHandler::SigPIPE);
    if (method == SIGHANDLE_SINGLE)
        MvrSignalHandler::createHandlerNonThreaded();
    else if (method == SIGHANDLE_THREAD)
    {
      MvrSignalHandler::blockCommonThisThread();
      MvrSignalHandler::createHandlerThreaded();
    }
  }

  if (initSockets)
    MvrSocket::init();

  if (ourDirectory.length() == 0)
  {
    if (getenv("MVRIA") != NULL)
    {
      setDirectory(getenv("MVRIA"));
    }
    else
    {
#ifndef WIN32
      MvrUtil::getStringFromFile("/etc/Mvria", buf, sizeof(buf));
      str = buf;
#else // WIN32
      if (MvrUtil::findFirstStringInRegistry(
          "SOFTWARE\\MobileRobots\\Mvria",
          "Install Directory", buf, 1024))
        str = buf;
      else
        if (MvrUtil::findFirstStringInRegistry(
            "SOFTWARE\\ActivMedia Robotics\\Mvria",
            "Install Directory", buf, 1024))
          str = buf;
        else
          str = "";
        
  #endif // WIN32
        if (str.length() > 0)
        {
    setDirectory(str.c_str());
        }
        else
        {
  #ifndef MVRINTERFACE
    MvrLog::log(MvrLog::Terse, "NonCritical Error: MVRIA could not find where it is located.");
  #else
    MvrLog::log(MvrLog::Verbose, "NonCritical Error: MVRIA could not find where it is located.");
  #endif
        }
      }
    }
  ourSigHandleExitNotShutdown = sigHandleExitNotShutdown;
  
#ifndef MVRINTERFACE
  Mvria::laserAddCreator("lms2xx", MvrLaserCreatorHelper::getCreateLMS2xxCB());
  Mvria::laserAddCreator("urg", MvrLaserCreatorHelper::getCreateUrgCB());
  Mvria::laserAddCreator("lms1XX", MvrLaserCreatorHelper::getCreateLMS1XXCB());
  Mvria::laserAddCreator("urg2.0", MvrLaserCreatorHelper::getCreateUrg_2_0CB());
  Mvria::laserAddCreator("s3series", MvrLaserCreatorHelper::getCreateS3SeriesCB());
  Mvria::laserAddCreator("lms5XX", MvrLaserCreatorHelper::getCreateLMS5XXCB());
  Mvria::laserAddCreator("tim3XX", MvrLaserCreatorHelper::getCreateTiM3XXCB());
  Mvria::laserAddCreator("tim510", MvrLaserCreatorHelper::getCreateTiM3XXCB());
  Mvria::laserAddCreator("tim551", MvrLaserCreatorHelper::getCreateTiM551CB());
  Mvria::laserAddCreator("tim561", MvrLaserCreatorHelper::getCreateTiM561CB());
  Mvria::laserAddCreator("tim571", MvrLaserCreatorHelper::getCreateTiM571CB());
  Mvria::laserAddCreator("sZseries", MvrLaserCreatorHelper::getCreateSZSeriesCB());
  Mvria::batteryAddCreator("mtx", MvrBatteryMTXCreatorHelper::getCreateBatteryMTXCB());
  Mvria::lcdAddCreator("mtx", MvrLCDMTXCreatorHelper::getCreateLCDMTXCB());
  Mvria::sonarAddCreator("mtx", MvrSonarMTXCreatorHelper::getCreateSonarMTXCB());
  //Mvria::batteryAddCreator("mtxbatteryv1", MvrBatteryMTXCreatorHelper::getCreateBatteryMTXCB());
  //Mvria::lcdAddCreator("mtxlcdv1", MvrLCDMTXCreatorHelper::getCreateLCDMTXCB());
  //Mvria::sonarAddCreator("mtxsonarv1", MvrSonarMTXCreatorHelper::getCreateSonarMTXCB());
	#endif // MVRINTERFACE

  Mvria::deviceConnectionAddCreator(
	  "serial", MvrDeviceConnectionCreatorHelper::getCreateSerialCB());
  Mvria::deviceConnectionAddCreator(
	  "serial422", MvrDeviceConnectionCreatorHelper::getCreateSerial422CB());
  Mvria::deviceConnectionAddCreator(
	  "tcp", MvrDeviceConnectionCreatorHelper::getCreateTcpCB());

  ourInited = true;

  for (iter=ourInitCBs.begin(); iter !=  ourInitCBs.end(); ++iter)
    (*iter)->invoke();

  MvrArgumentParser::addDefaultArgumentFile("/etc/Mvria.args");
  MvrArgumentParser::addDefaultArgumentEnv("MVRIAARGS");
  
  MvrVCC4::registerPTZType();
  MvrRVisionPTZ::registerPTZType();
  MvrDPPTU::registerPTZType();
  MvrSonyPTZ::registerPTZType();
}

/// This must be called last, after all other Mvria functions.
MVREXPORT void Mvria::uninit()
{
  std::list<MvrFunctor *>::iterator iter;

  for (iter=ourUninitCBs.begin(); iter!=ourUninitCBs.end(); ++iter)
    (*iter)->invoke();
#ifndef MVRINTERFACE
  MvrModuleLoader::closeAll();
#endif  // MVRINTERFACE
  MvrSocket::shutdown();
  MvrThread::shutdown();
}

/// This will adda a callback to the list of callbacks to call when Mvria
/// has been initialized. It can be called before anything else
MVREXPORT void Mvria::addInitCallBack(MvrFunctor *cb, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    ourInitCBs.push_front(cb);
  else
    ourInitCBs.push_back(cb);
}

/**
   This will add a callback to the list of callbacks to call right before Mvria
   is un-initialized. It can be called before anything else.
*/
MVREXPORT void Mvria::addUninitCallBack(MvrFunctor *cb, MvrListPos::pos position)
{
  if (position == MvrListPos::FIRST)
    ourUninitCBs.push_front(cb);
  else
    ourUninitCBs.push_back(cb);
}


/**
   Use this function to clean up or uninitialize Mvria, in particular,
   to stop background threads.

   This calls stop() on all ArThread's and ArASyncTask's. It will
   block until all ArThread's and ArASyncTask's exit. It is expected
   that all the tasks will obey the ArThread::myRunning variable and
   exit when it is false. Note, this only stop Mvria's background threads,
   it does not exit the program. 
*/
MVREXPORT void Mvria::shutdown()
{
  bool doExit = false;

  ourShuttingDownMutex.lock();
  ourRunning = false;
  if (ourShuttingDown)
    doExit = true;
  else
    ourShuttingDown = true;
  ourShuttingDownMutex.unlock();

  if (doExit)
    return;
  MvrThread.stopAll();
  MvrThread.jointAll();
  uninit();
}

/*
  This will call the list of Mvria exit callbacks (added by addExitCallback())
  and then exit the program with the given exit code.   
  This method may be used as a replacement for the standard system ::exit() call

  Note that this could be called from anywhere,
  mutexes may be locked when called-- all exit
  callbacks MUST return and cannot wait for a lock, since this could result 
  in the program hanging due to double-lock (deadlock).

  @sa addExitCallback()
 */
MVREXPORT void Mvria::exit(int exitCode)
{
  bool doReturn = false;
  ourShuttingDownMutex.lock();
  ourRunning = false;
  if (ourExiting)
    doReturn = true;
  else
    ourExiting = true;
  ourShuttingDownMutex.unlock();

  if (doReturn)
    return;
  callExitCallbacks();
  ::exit(exitCode);
}

/// @internal
MVREXPORT void Mvria::callExitCallbacks(void)
{
  ourRunning = false;

  ourShuttingDown.lock();
  ourExiting = true;
  ourShuttingDownMutex.unlock();

  std::multimap<int, MvrFunctor *>::reverse_iterator it;

  ourExitCallbacksMutex.lock();
  MvrLog::log(ourExitCallbacksLogLevel, "Mvria::exit: Starting exit callbacks");
  for (it=ourExitCallbacks.rbegin(); it!=ourExitCallbacks.rend(); it++){
    MvrLog::log(ourExitCallbacksLogLevel, "Mvria::exit: Calling callback at position %d with name '%s'",
                (*it).first, (*it).second->getName());
    (*it).second->invoke();
    MvrLog::log(ourExitCallbacksLogLevel, "Mvria::exit: Called callback at position %d with name '%s'",
                (*it).first, (*it).second->getName());
  }
  MvrLog::log(ourExitCallbacksLogLevela, "Mvria::exit: Finished exit callbacks");
  ourExitCallbacksMutex.unlock();
}

/**
 * Note, an exit callback may be called at any time; it should not
 * block or wait on any locked mutexes or similar.
 */
 MVREXPORT void Mvria::addExitCallback(MvrFunctor *functor, int position)
 {
   if (ourExiting)
   {
     MvrLog::log(MvrLog::Normal,
                 "Mvria::addExitCallback: Could not add exit callback '%s' since in the middle of exiting", functor->getName());
     return;
   }
   ourExitCallbacksMutex.lock();
   MvrLog::log(ourExitCallbacksLogLevel, "Mvria::addExitCallback: Adding callback at position %d with name '%s'",
               position, functor->getName());
   ourExitCallbacks.insert(std::pair<int, MvrFunctor *>(position, functor));
   ourExitCallbacks.unlock();
 }

 /*
  Note, an exit callback may be called at any time; it should not *
  block or wait on any locked mutexes or similar.
  */
MVREXPORT void Mvria::remExitCallback(MvrFunctor *functor)
{
  if (ourExiting)
  {
    MvrLog::log(MvrLog::Normal,
                "Mvria::remExitCallremExitCallback: Could not remove exit callback '%s' since in the middle of exiting", 
                functor->getName());
    return;
  }

  std::multimap<int, MvrFunctor *>::iterator it;

  ourExitCallbacksMutex.lock();
  for (it = ourExitCallbacks.begin(); it != ourExitCallbacks.end(); it++)
  {
    if ((*it).second == functor)
    {
      ourExitCallbacks.erase(it);
      ourExitCallbacksMutex.unlock();
      return remExitCallback(functor);
    }
  }
  ourExitCallbacksMutex.unlock();
}

/*
  This method is roughly obsolete, it simply forces all the threads to die
  immediately and then exits... The new method exit() is preferred.
  @deprecated
*/
MVREXPORT void Mvria::exitOld(int exitCode)
{
  ourRunning = false;
  MvrThread::cancelAll();
  uninit();
  ::exit(exitCode);
}

#ifndef MVRINTERFACE
MVREXPORT void Mvria::addRobot(MvrRobot *robot)
{
  ourRobots.push_back(robot);
}

MVREXPORT void Mvria::delRobot(MvrRobot *robot)
{
  ourRobots.remove(robot);
}

/*
 * @param name the name of the robot you want to find
 * @return NULL if there is no robot of that name, otherwise the robot with that name
 */
MVREXPORT MvrRobot *Mvria::findRobot(char *name)
{
  std::string rname;
  std::list<MvrRobot *>::iterator it;
  if (name == NULL)
    return NULL;
  rname = name;
  for (it = ourRobots.begin(); it != ourRobots.end(); it++)
  {
    if ((*it)->getName == rname)
      return (*it);
  }
  return NULL;
}

MVREXPORT std::list<MvrRobot*> *Mvria::getRobotList()
{
  return(&ourRobots);
}
#endif  // MVRINTERFACE

/// @internal
MVREXPORT void Mvria::signalHandlerCB(int sig)
{
  /// if we want to exit instead of shutdown then do that
  if (ourSigHandleExitNotShutdown)
  {
    MvrLog::log(MvrLog::Normal,
                "Mvria: Received signal '%s' . Exiting", 
                MvrSignalHandler::nameSignal(sig));
    Mvria::exit(0);

    // We shouldn't need this here, since the program should already
    // exit... just in case
    ::exit(0);
  }

  ourShuttingDownMutex.unlock();
  if (!ourRunning)
  {
    ourShuttingDownMutex.unlock();
    return;
  }
  ourShuttingDownMutex.unlock();
  MvrLog::log(MvrLog::Normal, "Mvria: Received signal '%s'. Shutting down.",
	            MvrSignalHandler::nameSignal(sig)); 

#ifndef MVRINTERFACE
  std::list<ArRobot*>::iterator iter;
  if ((sig == MvrSignalHandler::SigINT) || (sig == MvrSignalHandler::SigHUP) ||
      (sig == MvrSignalHandler::SigTERM))
  {
    for (iter=ourRobots.begin(); iter != ourRobots.end(); ++iter)
      (*iter)->stopRunning();
  }
#endif //MVRINTERFACE              

  shutdown();
}

/**
   This sets the directory that MVRIA is located in, so MVRIA can find param
   files and the like.  This can also be controlled by the environment variable
   MVRIA, which this is set to (if it exists) when Mvria::init is done.  So 
   for setDirectory to be effective, it must be done after the Mvria::init.
   @param directory the directory Mvria is located in
*/
MVREXPORT void Mvria::setDirectory(const char *directory)
{
  int ind;
  if (directory != NULL)
  {
    ourDirectory = directory;
    ind = stelen(directory) - 1;
    if (ind < 0)
      ind = 0;
    if (directory[ind] != '/' && directory[ind] != '\\')
    {
#ifdef WIN32
      ourDirectory += "\\";
#else
      ourDirectory += "/";
#endif
    }
#ifndef MVRINTERFACE 
  ourConfig.setBaseDirectory(ourDirectory.c_str());
#endif // MVRINTERFACE
  }
}

/**
   This gets the directory that MVRIA is located in, this is so MVRIA can find 
   param files and the like.  
   @return the directory MVRIA is located in
   @see setDirectory
*/
MVREXPORT const char *Mvria::getDirectory(void)
{
  return ourDirectory.c_str();
}

/// Sets the key handler, so that other classes can find it
MVREXPORT void Mvria::setKeyHandler(MvrKeyHandler *keyHandler)
{
  ourKeyHandler = keyHandler;
}

/// Gets the key handler if one has been set
MVREXPORT MvrKeyHandler *Mvria::getKeyHandler(void)
{
  return ourKeyHandler;
}

/// Sets the joy handler, so that other classes can find it
MVREXPORT void Mvria::setJoyHandler(MvrJoyHandler *joyHandler)
{
  ourJoyHandler = joyHandler;
}

/// Gets the joy handler if one has been set
MVREXPORT MvrJoyHandler *Mvria::getJoyHandler(void)
{
  return ourJoyHandler;
}

/**
   This gets the global config mvria uses.
 **/
MVREXPORT MvrConfig *Mvria::getConfig(void)
{
  return &ourConfig;
}

/**
   This gets the global string group aria uses.
 **/
MVREXPORT MvrStringInfoGroup *Mvria::getInfoGroup(void)
{
  return &ourInfoGroup;
}

/**
   Gets the maximum number of lasers to check for and use
**/
MVREXPORT int Mvria::getMaxNumLasers(void)
{
  return ourMaxNumLasers;
}

/**
   Sets the maximum number of lasers to check for and use, if you are
   going to set this you should do it after the Mvria::init before you
   start making laser connectors or robots or robot params.
**/
MVREXPORT void Mvria::setMaxNumLasers(int maxNumLasers)
{
  ourMaxNumLasers = maxNumLasers;
}

/**
   Gets the maximum number of sonar boards to check for and use
**/
MVREXPORT int Mvria::getMaxNumSonarBoards(void)
{
  return ourMaxNumSonarBoards;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Mvria::init
**/
MVREXPORT void Mvria::setMaxNumSonarBoards(int maxNumSonarBoards)
{
  ourMaxNumSonarBoards = maxNumSonarBoards;
}

/**
   Gets the maximum number of batteries to check for and use
**/
MVREXPORT int Mvria::getMaxNumBatteries(void)
{
  return ourMaxNumBatteries;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Mvria::init
**/
MVREXPORT void Mvria::setMaxNumBatteries(int maxNumBatteries)
{
  ourMaxNumBatteries = maxNumBatteries;
}

/**
   Gets the maximum number of lcds to check for and use
**/
MVREXPORT int Mvria::getMaxNumLCDs(void)
{
  return ourMaxNumLCDs;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Mvria::init
**/
MVREXPORT void Mvria::setMaxNumLCDs(int maxNumLCDs)
{
  ourMaxNumLCDs = maxNumLCDs;
}


#endif // MVRINTERFACE

/**
   returns true if MVRIA is initialized (Mvria::init() has been called) and has not been shut down by
   a call to Mvria::shutdown() or Mvria::exit() and an operating system signal has not occured (e.g. 
   external KILL signal)
**/
MVREXPORT bool Mvria::getRunning(void)
{
  return ourRunning;
}

/** @sa addParseArgsCB() */
MVREXPORT bool Mvria::parseArgs(void)
{
  std::multimap<int, MvrRetFunctor<bool> *>::reverse_iterator it;
  MvrRetFunctor<bool> *callback;

  MvrLog::log(ourParseArgsLogLevel, "Mvria: Parsing arguments");
  for (it = ourParseArgCBs.rbegin(); it != ourParseArgCBs.rend(); it++)
  {
    callback = (*it).second;
    if (callback->getName() != NULL && callback->getName()[0] != '\0')
      Mvria::log(ourParseArgsLogLevel, 
                 "Mvria: Calling parse arg functor '%s' (%d)", 
                 callback->getName(), (*it).first);
    else
      MvrLog::log(ourParseArgsLogLevel, 
                  "Mvria: Calling unnamed parse arg functor (%d)", 
                  (*it).first);

    if (!callback->invokeR())
    {
      if (callback->getName() != NULL && callback->getName()[0] != '\0')
        MvrLog::log(MvrLog::Terse,
                    "Mvria::parseArgs: Failed, parse arg functor '%s' (%d) returned false", 
                    callback->getName(), (*it).first);
      else
        MvrLog::log(MvrLog::Terse,
                    "Mvria::parseArgs: Failed unnamed parse arg functor (%d) returned false", 
                    (*it).first);
      return false;
    }
  }
  return true;
}

/** @sa addLogOptionsCB() */
MVREXPORT void Mvria::logOptions(void)
{
  std::multimap<int, MvrFunctor *>::reverse_iterator it;

  MvrLog::log(MvrLog::Terse, "");
  MvrLog::log(MvrLog::Terse, "");
  for (it = ourLogOptionsCBs.rbegin(); it != ourLogOptionsCBs.rend(); it++)
  {
    (*it).second->invoke();
    MvrLog::log(MvrLog::Terse, "");
    MvrLog::log(MvrLog::Terse, "");
  }
  MvrLog::log(MvrLog::Terse, "");
  MvrLog::log(MvrLog::Terse, "");
}

MVREXPORT void Mvria::addParseArgsCB(MvrRetFunctor<bool> *functor, 
					  int position)
{
  ourParseArgCBs.insert(std::pair<int, MvrRetFunctor<bool> *>(position, 
							     functor));
}

MVREXPORT void Mvria::setParseArgLogLevel(MvrLog::LogLevel level)
{
  ourParseArgsLogLevel = level;
}

MVREXPORT void Mvria::addLogOptionsCB(MvrFunctor *functor, int position)
{
  ourLogOptionsCBs.insert(std::pair<int, MvrFunctor *>(position, functor));
}

MVREXPORT void Mvria::setExitCallbacksLogLevel(MvrLog::LogLevel level)
{
  ourExitCallbacksLogLevel = level;
}

#ifndef MVRINTERFACE
/**
   This adds a functor which can create a laser of a given type. 

   @param laserType The laser type that the creator will make.

   @param creator A functor which takes an int (laser number) and a
   const char * (logPrefix) and returns a new laser of the laserType
**/
MVREXPORT bool Mvria::laserAddCreator(const char *laserType, MvrRetFunctor2<MvrLaser *, int, const char *> *creator)
{
  if (ourLaserCreatorMap.find(laserType) != ourLaserCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "Mvria::laserAddCreator: There is already a laser creator for %s, replacing it", laserType);
    ourLaserCreatorMap.erase(laserType);
  }
  else
  {
    if (!ourLaserTypes.empty())
      ourLaserChoices += ";;";
    ourLaserChoices += laserType;

    if (!ourLaserTypes.empty())
      ourLaserTypes += "|";
    ourLaserTypes += laserType;
  }

  ourLaserCreatorMap[laserType] = creator;
  return true;
}

/**
   Gets a string that is the types of lasers that can be created
   separated by | characters.  Mostly for internal use by ArLaserConnector.
**/
MVREXPORT const char *Mvria::laserGetTypes(void)
{
  return ourLaserTypes.c_str();
}

/**
   Gets a string that is the types of lasers that can be created
   separated by ;; characters.  Mostly for internal use by the config
**/
MVREXPORT const char *Mvria::laserGetChoices(void)
{
  return ourLaserChoices.c_str();
}

/**
   Creates a laser of a given type, with the given laserNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by MvrLaserConnector.
   @param laserType The type of laser to create
   @param laserNumber The laser number to use for the created laser
  
   @param logPrefix The prefix to use when logging 
*/

MVREXPORT MvrLaser *Mvria::laserCreate(const char *laserType, int laserNumber, const char *logPrefix)
{
  std::map<std::string, MvrRetFunctor2<MvrLaser *, int, const char *> *, MvrStrCaseCmpOp>::iterator it;
  if ((it = ourLaserCreatorMap.find(laserType)) == ourLaserCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%sCannot create a laser of type %s options are <%s>", logPrefix, laserType, laserGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(laserNumber, logPrefix);
}


/**
   This adds a functor which can create a battery of a given type. 

   @param batteryType The battery type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (battery number) and a
   const char * (logPrefix) and returns a new battery of the batteryType
**/
MVREXPORT bool Mvria::batteryAddCreator(const char *batteryType, MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *creator)
{
  if (ourBatteryCreatorMap.find(batteryType) != ourBatteryCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "Mvria::batteryAddCreator: There is already a battery creator for %s, replacing it", batteryType);
    ourBatteryCreatorMap.erase(batteryType);
  }
  else
  {
    if (!ourBatteryTypes.empty())
      ourBatteryChoices += ";;";
    ourBatteryChoices += batteryType;

    if (!ourBatteryTypes.empty())
      ourBatteryTypes += "|";
    ourBatteryTypes += batteryType;
  }

  ourBatteryCreatorMap[batteryType] = creator;
  return true;
}

/**
   Gets a string that is the types of batteries that can be created
   separated by | characters.  Mostly for internal use by MvrBatteryConnector.
**/
MVREXPORT const char *Mvria::batteryGetTypes(void)
{
  return ourBatteryTypes.c_str();
}

/**
   Gets a string that is the types of batteries that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
MVREXPORT const char *Mvria::batteryGetChoices(void)
{
  return ourBatteryChoices.c_str();
}

/**
   Creates a battery of a given type, with the given batteryNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by MvrBatteryConnector.

   @param batteryType The type of battery to create

   @param batteryNumber The battery number to use for the created battery
   
   @param logPrefix The prefix to use when logging 
*/

MVREXPORT MvrBatteryMTX *Mvria::batteryCreate(const char *batteryType, int batteryNumber, const char *logPrefix)
{
  std::map<std::string, MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *, MvrStrCaseCmpOp>::iterator it;
  if ((it = ourBatteryCreatorMap.find(batteryType)) == ourBatteryCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%sCannot create a battery of type %s options are <%s>", logPrefix, batteryType, batteryGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(batteryNumber, logPrefix);
}


/**
   This adds a functor which can create a lcd of a given type. 

   @param lcdType The lcd type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (lcd number) and a
   const char * (logPrefix) and returns a new lcd of the lcdType
**/
MVREXPORT bool Mvria::lcdAddCreator(
	const char *lcdType, 
	MvrRetFunctor2<MvrLCDMTX *, int, const char *> *creator)
{
  if (ourLCDCreatorMap.find(lcdType) != ourLCDCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "Mvria::lcdAddCreator: There is already a lcd creator for %s, replacing it", lcdType);
    ourLCDCreatorMap.erase(lcdType);
  }
  else
  {
    if (!ourLCDTypes.empty())
      ourLCDChoices += ";;";
    ourLCDChoices += lcdType;

    if (!ourLCDTypes.empty())
      ourLCDTypes += "|";
    ourLCDTypes += lcdType;

  }

  ourLCDCreatorMap[lcdType] = creator;
  return true;
}

/**
   Gets a string that is the types of batteries that can be created
   separated by | characters.  Mostly for internal use by MvrLCDConnector.
**/
MVREXPORT const char *Mvria::lcdGetTypes(void)
{
  return ourLCDTypes.c_str();
}

/**
   Gets a string that is the types of batteries that can be created
   separated by ;; characters.  Mostly for internal use by MvrLCDConnector.
**/
MVREXPORT const char *Mvria::lcdGetChoices(void)
{
  return ourLCDChoices.c_str();
}

/**
   Creates a lcd of a given type, with the given lcdNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by MvrLCDConnector.

   @param lcdType The type of lcd to create

   @param lcdNumber The lcd number to use for the created lcd
   
   @param logPrefix The prefix to use when logging 
*/

MVREXPORT MvrLCDMTX *Mvria::lcdCreate(const char *lcdType, int lcdNumber,
				    const char *logPrefix)
{
  std::map<std::string, MvrRetFunctor2<MvrLCDMTX *, int, const char *> *, MvrStrCaseCmpOp>::iterator it;
  if ((it = ourLCDCreatorMap.find(lcdType)) == ourLCDCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%sCannot create a lcd of type %s options are <%s>", logPrefix, lcdType, lcdGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(lcdNumber, logPrefix);
}


/**
   This adds a functor which can create a sonar of a given type. 

   @param sonarType The sonar type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (sonar number) and a
   const char * (logPrefix) and returns a new sonar of the sonarType
**/
MVREXPORT bool Mvria::sonarAddCreator(
	const char *sonarType, 
	MvrRetFunctor2<MvrSonarMTX *, int, const char *> *creator)
{
  if (ourSonarCreatorMap.find(sonarType) != ourSonarCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "Mvria::sonarAddCreator: There is already a sonar creator for %s, replacing it", sonarType);
    ourSonarCreatorMap.erase(sonarType);
  }
  else
  {
    if (!ourSonarTypes.empty())
      ourSonarChoices += ";;";
    ourSonarChoices += sonarType;

    if (!ourSonarTypes.empty())
      ourSonarTypes += "|";
    ourSonarTypes += sonarType;
  }

  ourSonarCreatorMap[sonarType] = creator;
  return true;
}

/**
   Gets a string that is the types of sonars that can be created
   separated by | characters.  Mostly for internal use by MvrSonarConnector.
**/
MVREXPORT const char *Mvria::sonarGetTypes(void)
{
  return ourSonarTypes.c_str();
}

/**
   Gets a string that is the types of sonars that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
MVREXPORT const char *Mvria::sonarGetChoices(void)
{
  return ourSonarChoices.c_str();
}

/**
   Creates a sonar of a given type, with the given sonarNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by MvrSonarConnector.

   @param sonarType The type of sonar to create

   @param sonarNumber The sonar number to use for the created sonar
   
   @param logPrefix The prefix to use when logging 
*/

MVREXPORT MvrSonarMTX *Mvria::sonarCreate(const char *sonarType, int sonarNumber, const char *logPrefix)
{
  std::map<std::string, MvrRetFunctor2<MvrSonarMTX *, int, const char *> *, MvrStrCaseCmpOp>::iterator it;
  if ((it = ourSonarCreatorMap.find(sonarType)) == ourSonarCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%sCannot create a sonar of type %s options are <%s>", logPrefix, sonarType, sonarGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(sonarNumber, logPrefix);
}

#endif // MVRINTERFACE

/**
   This adds a functor which can create a deviceConnection of a given
   type. 

   @param deviceConnectionType The deviceConnection type that the
   creator will make (these are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes a const char * (port name), a
   const char * (defaultInfo, for example default tcp port), a const
   char * (logPrefix) and returns a new deviceConnection of the
   deviceConnectionType if it can create the desired connection.  If
   the connection failes then it should return NULL.
**/
MVREXPORT bool Mvria::deviceConnectionAddCreator(
	const char *deviceConnectionType, 
	MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *creator)
{
  if (ourDeviceConnectionCreatorMap.find(deviceConnectionType) != ourDeviceConnectionCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "Mvria::deviceConnectionAddCreator: There is already a deviceConnection creator for %s, replacing it", deviceConnectionType);
    ourDeviceConnectionCreatorMap.erase(deviceConnectionType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)
    if (!ourDeviceConnectionTypes.empty())
      ourDeviceConnectionChoices += ";;";
    ourDeviceConnectionChoices += deviceConnectionType;

    if (!ourDeviceConnectionTypes.empty())
      ourDeviceConnectionTypes += "|";
    ourDeviceConnectionTypes += deviceConnectionType;
  }

  ourDeviceConnectionCreatorMap[deviceConnectionType] = creator;
  return true;
}

/**
   Gets a string that is the types of device connections that can be created
   separated by | characters.  Mostly for internal use by MvrLaserConnector.
**/
MVREXPORT const char *Mvria::deviceConnectionGetTypes(void)
{
    return ourDeviceConnectionTypes.c_str();
}

/**
   Gets a string that is the types of device connections that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
MVREXPORT const char *Mvria::deviceConnectionGetChoices(void)
{
    return ourDeviceConnectionChoices.c_str();
}

/** Creates a device connection of a given type, connecting to a given
    port, with the given defaultInfo, logging messages with the given
    prefix.  This is mostly for internal use by MvrLaserConnector

   @param deviceConnectionType The type of device connection to create

   @param port The port to connect to (for serial, an example is COM1
   or /dev/ttyS0, for tcp an example is p3dx:8101)

   @param defaultInfo Default info to use, the only use for this right
   now is for a default port for a tcp connection (so that if it
   doesn't have hostname:port format but only hostname then it will
   use this port)
   
   @param logPrefix The prefix to use when logging 
*/
MVREXPORT MvrDeviceConnection *Mvria::deviceConnectionCreate(const char *deviceConnectionType, const char *port, 
                                                             const char *defaultInfo, const char *logPrefix)
{
  std::map<std::string, MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *, MvrStrCaseCmpOp>::iterator it;

  if ((it = ourDeviceConnectionCreatorMap.find(deviceConnectionType)) == ourDeviceConnectionCreatorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%sCannot create a deviceConnection of type %s options are <%s>", logPrefix, deviceConnectionType, deviceConnectionGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(port, defaultInfo, logPrefix);
}


MVREXPORT  void Mvria::setMaxNumVideoDevices(size_t n) { ourMaxNumVideoDevices = n; }
MVREXPORT  size_t Mvria::getMaxNumVideoDevices() { return ourMaxNumVideoDevices; }
MVREXPORT  void Mvria::setMaxNumPTZs(size_t n) { ourMaxNumVideoDevices = n; }
MVREXPORT  size_t Mvria::getMaxNumPTZs() { return ourMaxNumVideoDevices; }

MVREXPORT const char *Mvria::getIdentifier(void)
{
  return ourIdentifier.c_str();
}

MVREXPORT void Mvria::setIdentifier(const char *identifier)
{
  ourIdentifier = identifier;

  // MPL fixing the problem caused by whitespace or bad chars in the
  // identifier (bug 14486).
  MvrUtil::fixBadCharacters(&ourIdentifier, true);
}


