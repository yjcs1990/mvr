/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvriaInternal.h
 > Description  : Contains global initialization, deinitialization and other global functions
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRIAINTERNAL_H
#define MVRIAINTERNAL_H

#include "MvrMutex.h"
#include "MvrFunctor.h"
#include "MvrConfig.h"
#include "MvrDeviceConnection.h"

#ifndef MVRINTERFACE
#include "MvrStringInfoGroup.h"

class MvrRobot;
class MvrRobotJoyHandler;
class MvrSonarMTX;
class MvrBatteryMTX;
class MvrLCDMTX;
#endif  // MVRINTERFACE

class MvrKeyHandler;
class MvrJoyHandler;

class Mvria
{
public:
  typedef enum {
    SIGHANDLE_SINGLE,     ///< Setup signal handler in a global, non-thread way
    SIGHANDLE_THREAD,     ///< Setup a dedicated signal handler thread
    SIGHANDLE_NONE        ///< Do no signal handling
  } SigHandleMethod;

  /// Initialize Mvria global data struture and perform OS-specific initialization, including adding 
  /// OS signal handlers on Linux, initializing sockets library on Windows, etc.
  MVREXPORT static void init(SigHandleMethod method=SIGHANDLE_THREAD,
                             bool initSocket=true, bool sigHandleExitNotShutdown=true);
  /// Perform OS-specific deinitialization, used by shutdown() and exit()
  MVREXPORT static void uninit();

  /// Adds a callback to call when Mvria is initialized using init()
  MVREXPORT static void addInitCallBack(MvrFunctor *cb, MvrListPos::Pos position);

  /// Adds a callback to call when Mvria is uninitialized using uninit()
  MVREXPORT static void addUninitCallBack(MvrFunctor *cb, MvrListPos::Pos position);

  /// Shutdown all Mvria Processes/Threads
  MVREXPORT static void shutdown();

  /// Shutdown all Mvria processes/threads, call exit callbacks, and exit the program
  MVREXPORT static void exit(int exitCode=0);

  /// Sees if Mvria is still running (mostly for the thread in main)
  MVREXPORT static bool getRunning(void);

  /// Sets the directory that MVRIA resides in, to override default
  MVREXPORT static void setDirectory(const char *directory);
  
  /// Parses the arguments for the program (calls all the callbacks added with addParseArgsCB())
  MVREXPORT static bool parseArgs(void);

  /// Logs all the options for the program (calls all the callbacks added with addLogOptionsCB())
  MVREXPORT static void logOptions(void);

  /// Sets the key handler, so that other classes can find it using getKeyHandler()
  MVREXPORT static void setKeyHandler(MvrKeyHandler *keyHandler);

  /// Gets a pointer to the global key handler, if one has been set with setKeyHandler()
  MVREXPORT static MvrKeyHandler *getKeyHandler(void);

  /// Sets the joystick handler, so that other classes can find it using getJoyHandler()
  MVREXPORT static void setJoyHandler(MvrJoyHandler *joyHandler);
  /// Gets a pointer to the joystick handler if one has been set with setJoyHandler()
  MVREXPORT static MvrJoyHandler *getJoyHandler(void);

  /// Adds a functor to by called before program exit by Mvria::exit()
  MVREXPORT static void addExitCallback(MvrFunctor *functor, int position=50);
  /// Adds a callback for when we parse arguments
  MVREXPORT static void addParseArgsCB(MvrRetFunctor<bool> *functor, int position=50);

  /// Sets the log level for the parsing function
  MVREXPORT static void setParseArgLogLevel(MvrLog::LogLevel level);

  /// Adds a callback for when we log options
  MVREXPORT static void addLogOptionsCB(MvrFunctor *functor, int position=50);
  /// Adds a type of deviceConnection for Mvria to be able to create
  MVREXPORT static bool deviceConnectionAddCreator(
            const char *deviceConnectionType,
            MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *creator);
  /// Gets a list of the possible deviceConnection types
  MVREXPORT static const char *deviceConnectionGetTypes(void);

  /// Gets a list of the possible deviceConnection types (for use in the config)
  MVREXPORT static const char *deviceConnectionGetChoices(void);
  
  /// Creates a deviceConnection of the given type
  MVREXPORT static MvrDeviceConnection *deviceConnectionCreate(
	  const char *deviceConnectionType, const char *port, 
	  const char *defaultInfo, 
	  const char *prefix = "Mvria::deviceConnectionCreate");

#ifndef MVRINTERFACE
  /// Sets the robot joystick handler, so that other classes can find it
  MVREXPORT static void setRobotJoyHandler(MvrRobotJoyHandler *robotJoyHandler);

  /// Gets the robot joystick handler if one has been set
  MVREXPORT static MvrRobotJoyHandler *getRobotJoyHandler(void);

  /// Gets the MvrConfig for this program
  MVREXPORT static MvrConfig *getConfig(void);

  /// Gets the MvrStringInfoGroup for this program
  MVREXPORT static MvrStringInfoGroup *getInfoGroup(void);

  /// Add a robot to the global list of robots
  MVREXPORT static void addRobot(MvrRobot *robot);

  /// Remove a robot from the global list of robots
  MVREXPORT static void delRobot(MvrRobot *robot);

  /// Finds a robot in the global list of robots, by name
  MVREXPORT static MvrRobot *findRobot(char *name);

  /// Get a copy of the global robot list
  MVREXPORT static std::list<MvrRobot*> * getRobotList();
  
  /// Gets the maximum number of lasers to use
  MVREXPORT static int getMaxNumLasers(void);

  /// Sets the maximum number of lasers to use
  MVREXPORT static void setMaxNumLasers(int maxNumLasers);

  /// Gets the maximum number of sonars to use
  MVREXPORT static int getMaxNumSonarBoards(void);

  /// Sets the maximum number of sonars to use
  MVREXPORT static void setMaxNumSonarBoards(int maxNumSonarBoards);

  /// Gets the maximum number of batteris to use
  MVREXPORT static int getMaxNumBatteries(void);

  /// Sets the maximum number of batteries to use
  MVREXPORT static void setMaxNumBatteries(int maxNumBatteries);

  /// Gets the maximum number of lcds to use
  MVREXPORT static int getMaxNumLCDs(void);

  /// Sets the maximum number of batteries to use
  MVREXPORT static void setMaxNumLCDs(int maxNumLCDs);

  /// Creates a laser of the given type
  MVREXPORT static MvrLaser *laserCreate(
	  const char *laserType, int laserNumber,
	  const char *prefix = "Mvria::laserCreate");

  /// Adds a type of laser for Mvria to be able to create
  MVREXPORT static bool laserAddCreator(
	  const char *laserType, 
	  MvrRetFunctor2<MvrLaser *, int, const char *> *creator);
  
  /// Gets a list of the possible laser types
  MVREXPORT static const char *laserGetTypes(void);

  /// Gets a list of the possible laser types (for use in the config)
  MVREXPORT static const char *laserGetChoices(void);
  
  /// Creates a battery of the given type
  MVREXPORT static MvrBatteryMTX *batteryCreate(
	  const char *batteryType, int batteryNumber,
	  const char *prefix = "Mvria::batteryCreate");

  /// Adds a type of battery for Mvria to be able to create
  MVREXPORT static bool batteryAddCreator(
	  const char *batteryType, 
	  MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible battery types
  MVREXPORT static const char *batteryGetTypes(void);
  /// Gets a list of the possible battery types (for use in the config)
  MVREXPORT static const char *batteryGetChoices(void);

  /// Creates a lcd of the given type
  MVREXPORT static MvrLCDMTX *lcdCreate(
	  const char *lcdType, int lcdNumber,
	  const char *prefix = "Mvria::lcdCreate");

  /// Adds a type of lcd for Mvria to be able to create
  MVREXPORT static bool lcdAddCreator(
	  const char *lcdType, 
	  MvrRetFunctor2<MvrLCDMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible lcd types
  MVREXPORT static const char *lcdGetTypes(void);
  /// Gets a list of the possible lcd types (for use in the config)
  MVREXPORT static const char *lcdGetChoices(void);

  /// Creates a sonar of the given type
  MVREXPORT static MvrSonarMTX *sonarCreate(
	  const char *sonarType, int sonarNumber,
	  const char *prefix = "Mvria::sonarCreate");

  /// Adds a type of sonar for Mvria to be able to create
  MVREXPORT static bool sonarAddCreator(
	  const char *sonarType, 
	  MvrRetFunctor2<MvrSonarMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible sonar types
  MVREXPORT static const char *sonarGetTypes(void);
  /// Gets a list of the possible sonar types (for use in the config)
  MVREXPORT static const char *sonarGetChoices(void);
  
  /// Set maximum limit on video devices (used by MvrVideo library)
  MVREXPORT static void setMaxNumVideoDevices(size_t n); 
  /// Get maximum limit on video devices (used by MvrVideo library)
  MVREXPORT static size_t getMaxNumVideoDevices(); 
 
  /// Set maximum limit on PTZ or PTU devices, used by MvrPTZConnector. Call before connecting to PTZ devices with MvrPTZConnector. 
  MVREXPORT static void setMaxNumPTZs(size_t n); 
  /// Set maximum limit on PTZ or PTU devices, used by MvrPTZConnector.
  MVREXPORT static size_t getMaxNumPTZs();  
#endif // MVRINTERFACE

  /// Gets the identifier (for humans) used for this instance of Mvria
  MVREXPORT static const char *getIdentifier(void);
  /// Sets the identifier (for humans) used for this instance of Mvria
  MVREXPORT static void setIdentifier(const char *identifier);


protected:
  static bool ourInited;
  static MvrGlobalFunctor1<int> ourSignalHandlerCB;
  static bool ourRunning;
  static MvrMutex ourShuttingDownMutex;
  static bool ourShuttingDown;
  static bool ourExiting;
  static std::string ourDirectory;
  static std::list<MvrFunctor*> ourInitCBs;
  static std::list<MvrFunctor*> ourUninitCBs;
  static MvrKeyHandler *ourKeyHandler;
  static MvrJoyHandler *ourJoyHandler;
#ifndef MVRINTERFACE
  static std::list<MvrRobot*> ourRobots;
  static MvrRobotJoyHandler *ourRobotJoyHandler;
  static MvrConfig ourConfig;
  static MvrStringInfoGroup ourInfoGroup;
  static int ourMaxNumLasers;
	static int ourMaxNumSonarBoards;
	static int ourMaxNumBatteries;
	static int ourMaxNumLCDs;
  static std::map<std::string, MvrRetFunctor2<MvrLaser *, int, const char *> *, MvrStrCaseCmpOp> ourLaserCreatorMap;
  static std::string ourLaserTypes;
  static std::string ourLaserChoices;
  static std::map<std::string, MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *, MvrStrCaseCmpOp> ourBatteryCreatorMap;
  static std::string ourBatteryTypes;
  static std::string ourBatteryChoices;
  static std::map<std::string, MvrRetFunctor2<MvrLCDMTX *, int, const char *> *, MvrStrCaseCmpOp> ourLCDCreatorMap;
  static std::string ourLCDTypes;
  static std::string ourLCDChoices;
  static std::map<std::string, MvrRetFunctor2<MvrSonarMTX *, int, const char *> *, MvrStrCaseCmpOp> ourSonarCreatorMap;
  static std::string ourSonarTypes;
  static std::string ourSonarChoices;
#endif // MVRINTERFACE
  static MvrMutex ourExitCallbacksMutex;
  static std::multimap<int, MvrFunctor *> ourExitCallbacks;
  static bool ourSigHandleExitNotShutdown;
  static std::multimap<int, MvrRetFunctor<bool> *> ourParseArgCBs;
  static MvrLog::LogLevel ourParseArgsLogLevel;
  static std::multimap<int, MvrFunctor *> ourLogOptionsCBs;
  static MvrLog::LogLevel ourExitCallbacksLogLevel;
  static std::map<std::string, MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *, MvrStrCaseCmpOp> ourDeviceConnectionCreatorMap;
  static std::string ourDeviceConnectionTypes;
  static std::string ourDeviceConnectionChoices;
  static std::string ourIdentifier;
#ifndef MVRINTERFACE
  static size_t ourMaxNumVideoDevices;
  static size_t ourMaxNumPTZs;
#endif
};
#endif  // MVRIAINTERNAL_H