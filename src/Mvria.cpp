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
   ARIAARGS as locations for MvrArgumentParser to obtain default argument values
   from. 

   @param method the method in which to handle signals. Defaulted to SIGHANDLE_SINGLE, or the method indicated by the ARIA_SIGHANDLE_METHOD environment variable (NONE, SINGLE or THREAD), if it exists. 
   @param initSockets specify whether or not to initialize the socket layer. This is only meaningfull for Windows. Defaulted to true.
   @param sigHandleExitNotShutdown if this is true and a signal
   happens Mvria will use exit() to quit instead of shutdown(), false will
   still use shutdown which is the old behavior.

   @see MvrSignalHandler
   @see MvrSocket
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
    if (getenv("ARIA") != NULL)
    {
      setDirectory(getenv("ARIA"));
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
    MvrLog::log(MvrLog::Terse, "NonCritical Error: ARIA could not find where it is located.");
  #else
    MvrLog::log(MvrLog::Verbose, "NonCritical Error: ARIA could not find where it is located.");
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
  MvrArgumentParser::addDefaultArgumentEnv("ARIAARGS");
  
  MvrVCC4::registerPTZType();
  MvrRVisionPTZ::registerPTZType();
  MvrDPPTU::registerPTZType();
  MvrSonyPTZ::registerPTZType();
}