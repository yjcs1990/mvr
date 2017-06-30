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
#ifndef ARIAINTERNAL_H
#define ARIAINTERNAL_H


#include "MvrMutex.h"
#include "MvrFunctor.h"
#include "MvrConfig.h"

#ifndef ARINTERFACE
#include "MvrStringInfoGroup.h"
class MvrRobot;
class MvrRobotJoyHandler;
class MvrSonarMTX;
class MvrBatteryMTX;
class MvrLCDMTX;
#endif // ARINTERFACE

class MvrKeyHandler;
class MvrJoyHandler;


/** Contains global initialization, deinitialization and other global functions
    @ingroup ImportantClasses
*/
class Mvria
{
public:

  typedef enum {
    SIGHANDLE_SINGLE, ///< Setup signal handlers in a global, non-thread way
    SIGHANDLE_THREAD, ///< Setup a dedicated signal handling thread
    SIGHANDLE_NONE ///< Do no signal handling
  } SigHandleMethod;

	/// Initialize Mvria global data struture and perform OS-specific initialization, including adding OS signal handlers on Linux, initializing sockets library on Windows, etc.
  AREXPORT static void init(SigHandleMethod method = SIGHANDLE_THREAD,
			    bool initSockets = true, 
			    bool sigHandleExitNotShutdown = true);

  /// Performs OS-specific deinitialization, used by shutdown() and exit().
  AREXPORT static void uninit();

  /// Adds a callback to call when Mvria is initialized using init()
  AREXPORT static void addInitCallBack(MvrFunctor *cb, MvrListPos::Pos position);

  /// Adds a callback to call when Mvria is uninititialized using uninit()
  AREXPORT static void addUninitCallBack(MvrFunctor *cb,
					 MvrListPos::Pos position);

  /// Shutdown all Mvria processes/threads
  AREXPORT static void shutdown();

  /// Shutdown all Mvria processes/threads, call exit callbacks, and exit the program
  AREXPORT static void exit(int exitCode = 0);

  /// Sees if Mvria is still running (mostly for the thread in main)
  AREXPORT static bool getRunning(void);

  /// Sets the directory that ARIA resides in, to override default
  AREXPORT static void setDirectory(const char * directory);

  /// Gets the directory that ARIA resides in
  AREXPORT static const char *getDirectory(void);

  /// Parses the arguments for the program (calls all the callbacks added with addParseArgsCB())
  AREXPORT static bool parseArgs(void);

  /// Logs all the options for the program (Calls all the callbacks added with addLogOptionsCB())
  AREXPORT static void logOptions(void);

  /// Sets the key handler, so that other classes can find it using getKeyHandler()
  AREXPORT static void setKeyHandler(MvrKeyHandler *keyHandler);

  /// Gets a pointer to the global key handler, if one has been set with setKeyHandler()
  AREXPORT static MvrKeyHandler *getKeyHandler(void);

  /// Sets the joystick handler, so that other classes can find it using getJoyHandler()
  AREXPORT static void setJoyHandler(MvrJoyHandler *joyHandler);

  /// Get a pointer to the joystick handler if one has been set with setJoyHandler()
  AREXPORT static MvrJoyHandler *getJoyHandler(void);

  /// Adds a functor to by called before program exit by Mvria::exit()
  AREXPORT static void addExitCallback(MvrFunctor *functor, int position = 50);

  /// Removes a functor to by called before program exit by Mvria::exit()
  AREXPORT static void remExitCallback(MvrFunctor *functor);

  /// Sets the log level for the exit callbacks
  AREXPORT static void setExitCallbacksLogLevel(MvrLog::LogLevel level);

  /// Force an exit of all Mvria processes/threads (the old way)
  AREXPORT static void exitOld(int exitCode = 0);

  /// Internal, the callback for the signal handling
  AREXPORT static void signalHandlerCB(int sig);

  /// Internal, calls the exit callbacks
  AREXPORT static void callExitCallbacks(void);

  /// Adds a callback for when we parse arguments 
  AREXPORT static void addParseArgsCB(MvrRetFunctor<bool> *functor, 
				      int position = 50);

  /// Sets the log level for the parsing function
  AREXPORT static void setParseArgLogLevel(MvrLog::LogLevel level);

  /// Adds a callback for when we log options
  AREXPORT static void addLogOptionsCB(MvrFunctor *functor, int position = 50);

  /// Adds a type of deviceConnection for Mvria to be able to create
  AREXPORT static bool deviceConnectionAddCreator(
	  const char *deviceConnectionType, 
	  MvrRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *creator);

  
  /// Gets a list of the possible deviceConnection types
  AREXPORT static const char *deviceConnectionGetTypes(void);

  /// Gets a list of the possible deviceConnection types (for use in the config)
  AREXPORT static const char *deviceConnectionGetChoices(void);
  
  /// Creates a deviceConnection of the given type
  AREXPORT static MvrDeviceConnection *deviceConnectionCreate(
	  const char *deviceConnectionType, const char *port, 
	  const char *defaultInfo, 
	  const char *prefix = "Mvria::deviceConnectionCreate");

#ifndef ARINTERFACE
  /// Sets the robot joystick handler, so that other classes can find it
  AREXPORT static void setRobotJoyHandler(MvrRobotJoyHandler *robotJoyHandler);

  /// Gets the robot joystick handler if one has been set
  AREXPORT static MvrRobotJoyHandler *getRobotJoyHandler(void);

  /// Gets the MvrConfig for this program
  AREXPORT static MvrConfig *getConfig(void);

  /// Gets the MvrStringInfoGroup for this program
  AREXPORT static MvrStringInfoGroup *getInfoGroup(void);

  /// Add a robot to the global list of robots
  AREXPORT static void addRobot(MvrRobot *robot);

  /// Remove a robot from the global list of robots
  AREXPORT static void delRobot(MvrRobot *robot);

  /// Finds a robot in the global list of robots, by name
  AREXPORT static MvrRobot *findRobot(char *name);

  /// Get a copy of the global robot list
  AREXPORT static std::list<ArRobot*> * getRobotList();
  
  /// Gets the maximum number of lasers to use
  AREXPORT static int getMaxNumLasers(void);

  /// Sets the maximum number of lasers to use
  AREXPORT static void setMaxNumLasers(int maxNumLasers);

  /// Gets the maximum number of sonars to use
  AREXPORT static int getMaxNumSonarBoards(void);

  /// Sets the maximum number of sonars to use
  AREXPORT static void setMaxNumSonarBoards(int maxNumSonarBoards);

  /// Gets the maximum number of batteris to use
  AREXPORT static int getMaxNumBatteries(void);

  /// Sets the maximum number of batteries to use
  AREXPORT static void setMaxNumBatteries(int maxNumBatteries);

	// Gets the maximum number of lcds to use
  AREXPORT static int getMaxNumLCDs(void);

  /// Sets the maximum number of batteries to use
  AREXPORT static void setMaxNumLCDs(int maxNumLCDs);

  /// Creates a laser of the given type
  AREXPORT static MvrLaser *laserCreate(
	  const char *laserType, int laserNumber,
	  const char *prefix = "Mvria::laserCreate");

  /// Adds a type of laser for Mvria to be able to create
  AREXPORT static bool laserAddCreator(
	  const char *laserType, 
	  MvrRetFunctor2<ArLaser *, int, const char *> *creator);
  
  /// Gets a list of the possible laser types
  AREXPORT static const char *laserGetTypes(void);

  /// Gets a list of the possible laser types (for use in the config)
  AREXPORT static const char *laserGetChoices(void);
  
  /// Creates a battery of the given type
  AREXPORT static MvrBatteryMTX *batteryCreate(
	  const char *batteryType, int batteryNumber,
	  const char *prefix = "Mvria::batteryCreate");

  /// Adds a type of battery for Mvria to be able to create
  AREXPORT static bool batteryAddCreator(
	  const char *batteryType, 
	  MvrRetFunctor2<ArBatteryMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible battery types
  AREXPORT static const char *batteryGetTypes(void);
  /// Gets a list of the possible battery types (for use in the config)
  AREXPORT static const char *batteryGetChoices(void);

  /// Creates a lcd of the given type
  AREXPORT static MvrLCDMTX *lcdCreate(
	  const char *lcdType, int lcdNumber,
	  const char *prefix = "Mvria::lcdCreate");

  /// Adds a type of lcd for Mvria to be able to create
  AREXPORT static bool lcdAddCreator(
	  const char *lcdType, 
	  MvrRetFunctor2<ArLCDMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible lcd types
  AREXPORT static const char *lcdGetTypes(void);
  /// Gets a list of the possible lcd types (for use in the config)
  AREXPORT static const char *lcdGetChoices(void);

  /// Creates a sonar of the given type
  AREXPORT static MvrSonarMTX *sonarCreate(
	  const char *sonarType, int sonarNumber,
	  const char *prefix = "Mvria::sonarCreate");

  /// Adds a type of sonar for Mvria to be able to create
  AREXPORT static bool sonarAddCreator(
	  const char *sonarType, 
	  MvrRetFunctor2<ArSonarMTX *, int, const char *> *creator);
  
  /// Gets a list of the possible sonar types
  AREXPORT static const char *sonarGetTypes(void);
  /// Gets a list of the possible sonar types (for use in the config)
  AREXPORT static const char *sonarGetChoices(void);
  
  /// Set maximum limit on video devices (used by MvrVideo library)
  AREXPORT static void setMaxNumVideoDevices(size_t n); 
  /// Get maximum limit on video devices (used by MvrVideo library)
  AREXPORT static size_t getMaxNumVideoDevices(); 
 
  /// Set maximum limit on PTZ or PTU devices, used by MvrPTZConnector. Call before connecting to PTZ devices with MvrPTZConnector. 
  AREXPORT static void setMaxNumPTZs(size_t n); 
  /// Set maximum limit on PTZ or PTU devices, used by MvrPTZConnector.
  AREXPORT static size_t getMaxNumPTZs();  
#endif // ARINTERFACE

  /// Gets the identifier (for humans) used for this instance of Mvria
  AREXPORT static const char *getIdentifier(void);
  /// Sets the identifier (for humans) used for this instance of Mvria
  AREXPORT static void setIdentifier(const char *identifier);


protected:
  static bool ourInited;
  static MvrGlobalFunctor1<int> ourSignalHandlerCB;
  static bool ourRunning;
  static MvrMutex ourShuttingDownMutex;
  static bool ourShuttingDown;
  static bool ourExiting;
  static std::string ourDirectory;
  static std::list<ArFunctor*> ourInitCBs;
  static std::list<ArFunctor*> ourUninitCBs;
  static MvrKeyHandler *ourKeyHandler;
  static MvrJoyHandler *ourJoyHandler;
#ifndef ARINTERFACE
  static std::list<ArRobot*> ourRobots;
  static MvrRobotJoyHandler *ourRobotJoyHandler;
  static MvrConfig ourConfig;
  static MvrStringInfoGroup ourInfoGroup;
  static int ourMaxNumLasers;
	static int ourMaxNumSonarBoards;
	static int ourMaxNumBatteries;
	static int ourMaxNumLCDs;
  static std::map<std::string, MvrRetFunctor2<ArLaser *, int, const char *> *, MvrStrCaseCmpOp> ourLaserCreatorMap;
  static std::string ourLaserTypes;
  static std::string ourLaserChoices;
  static std::map<std::string, MvrRetFunctor2<ArBatteryMTX *, int, const char *> *, MvrStrCaseCmpOp> ourBatteryCreatorMap;
  static std::string ourBatteryTypes;
  static std::string ourBatteryChoices;
  static std::map<std::string, MvrRetFunctor2<ArLCDMTX *, int, const char *> *, MvrStrCaseCmpOp> ourLCDCreatorMap;
  static std::string ourLCDTypes;
  static std::string ourLCDChoices;
  static std::map<std::string, MvrRetFunctor2<ArSonarMTX *, int, const char *> *, MvrStrCaseCmpOp> ourSonarCreatorMap;
  static std::string ourSonarTypes;
  static std::string ourSonarChoices;
#endif // ARINTERFACE
  static MvrMutex ourExitCallbacksMutex;
  static std::multimap<int, MvrFunctor *> ourExitCallbacks;
  static bool ourSigHandleExitNotShutdown;
  static std::multimap<int, MvrRetFunctor<bool> *> ourParseArgCBs;
  static MvrLog::LogLevel ourParseArgsLogLevel;
  static std::multimap<int, MvrFunctor *> ourLogOptionsCBs;
  static MvrLog::LogLevel ourExitCallbacksLogLevel;
  static std::map<std::string, MvrRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *, MvrStrCaseCmpOp> ourDeviceConnectionCreatorMap;
  static std::string ourDeviceConnectionTypes;
  static std::string ourDeviceConnectionChoices;
  static std::string ourIdentifier;
#ifndef ARINTERFACE
  static size_t ourMaxNumVideoDevices;
  static size_t ourMaxNumPTZs;
#endif
};


#endif // ARIAINTERNAL_H
