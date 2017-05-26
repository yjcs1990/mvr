/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLaserLogger.h
 > Description  : This class can be used to create log files for the laser mapper
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRLASERLOGGER_H
#define MVRLASERLOGGER_H

#include <stdio.h>

#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrLaser;
class MvrRobot;
class MvrJoyHandler;
class MvrRobotJoyHandler;
class MvrRobotPacket;

/// This class can be used to create log files for the laser mapper

class MvrLaserLogger
{
public:
  /// Constructor
  MVREXPORT MvrLaserLogger(
            MvrRobot *robot, MvrLaser *laser, double distDiff, 
            double degDiff, const char *fileName, 
            bool addGoals = false, 
            MvrJoyHandler *joyHandler = NULL,
            const char *baseDirectory = NULL,
            bool useReflectorValues = false,
            MvrRobotJoyHandler *robotJoyHandler = NULL,
            const std::map<std::string, 
            MvrRetFunctor3<int, MvrTime, MvrPose *, MvrPoseWithTime *> *, 
            MvrStrCaseCmpOp> *extraLocationData = NULL,
            std::list<MvrLaser *> *extraLasers = NULL);
  /// Destructor
  MVREXPORT virtual ~MvrLaserLogger();

#ifndef SWIG
  ///@brief Adds a string to the log file with a tag at the given moment
  MVREXPORT void addTagToLog(const char *str, ...);
#endif

  /// Same ass addToLog, but no varargs, wrapper for java
  MVREXPORT void addTagToLogPlain(const char *str);

#ifndef SWIG
  /// @brief Adds a string to the log file without a tag for where or when we are

  MVREXPORT void addInfoToLog(const char *str, ...);
#endif

  /// Same as addToInfo, but does it without marking robot position
  MVREXPORT void addInfoToLogPlain(const char *str);
  /// Sets the distance at which the robot will take a new reading
  void setDistDiff(double distDiff) { myDistDiff = MvrMath::fabs(distDiff); }
  /// Gets the distance at which the robot will take a new reading
  double getDistDiff(void) { return myDistDiff; }
  /// Sets the degrees to turn at which the robot will take a new reading
  void setDegDiff(double degDiff) { myDistDiff = MvrMath::fabs(degDiff); }
  /// Gets the degrees to turn at which the robot will take a new reading
  double getDegDiff(void) { return myDegDiff; }
  /// Explicitly tells the robot to take a reading
  void takeReading(void) { myTakeReadingExplicit = true; }
  /// Adds a goal where the robot is at the moment
  void addGoal(void) { myAddGoalExplicit = true; }
  /// Sees if the file was opened successfully
  bool wasFileOpenedSuccessfully(void) 
    { if (myFile != NULL) return true; else return false; }
  /// Gets if we're taking old (sick1:) readings
  bool takingOldReadings(void) { return myOldReadings; }
  /// Sets if we're taking old (sick1:) readings
  void takeOldReadings(bool takeOld) { myOldReadings = takeOld; }
  /// Gets if we're taking new (scan1:) readings
  bool takingNewReadings(void) { return myNewReadings; }
  /// Sets if we're taking old (scan1:) readings
  void takeNewReadings(bool takeNew) { myNewReadings = takeNew; }
protected:
  /// The task which gets attached to the robot
  MVREXPORT void robotTask(void);
  // internal function that adds goals if needed (and specified)
  void internalAddGoal(void);
  // internal function that writes tags
  void internalWriteTags(void);
  // internal function that takes a reading
  void internalTakeReading(void);
  // internal function that takes a reading from one laser
  void internalTakeLaserReading(MvrLaser *laser, int laserNumber);
  // internal function that prints the position
  void internalPrintPos(MvrPose encoderPoseTaken, MvrPose goalPoseTaken, MvrTime timeTaken);
  // internal function that logs the pose and conf
  void internalPrintLaserPoseAndConf(MvrLaser *laser, int laserNumber);
  // internal packet for handling the loop packets
  MVREXPORT bool loopPacketHandler(MvrRobotPacket *packet);


  // what type of readings we are taking
  bool myOldReadings;
  // what type of readings we are taking
  bool myNewReadings;
  // if we're taking reflector values too
  bool myUseReflectorValues;
  std::list<std::string> myTags;
  std::list<std::string> myInfos;
  bool myWrote;
  MvrRobot *myRobot;
  // note that this is now in the list of lasers, but this pointer is
  // kept to denote the primary laser (so that it can always be called
  // number 1)
  MvrLaser *myLaser;
  bool myAddGoals;
  MvrJoyHandler *myJoyHandler;
  MvrRobotJoyHandler *myRobotJoyHandler;
  std::string myFileName;
  std::string myBaseDirectory;
  FILE *myFile;
  bool myFirstTaken;

  MvrPose myLast;
  double myLastVel;
  double myDistDiff;
  double myDegDiff;
  MvrSectors mySectors;
  MvrFunctorC<MvrLaserLogger> myTaskCB;
  int myScanNumber;
  MvrTime myStartTime;
  bool myTakeReadingExplicit;
  bool myAddGoalExplicit;
  bool myAddGoalKeyboard;
  bool myLastAddGoalKeyboard;
  bool myLastJoyButton;
  bool myLastRobotJoyButton;
  bool myFirstGoalTaken;
  int myNumGoal;
  MvrPose myLastGoalTakenPose;
  MvrTime myLastGoalTakenTime;

  void goalKeyCallback(void);
  unsigned char myLastLoops;
  
  // the lasers all have this, so shouldn't need it anymore...
  //bool myFlipped;
  
  bool myIncludeRawEncoderPose;
  std::map<std::string, MvrRetFunctor3<int, MvrTime, MvrPose *, MvrPoseWithTime *> *, 
	   MvrStrCaseCmpOp> myExtraLocationData;
  
  // holders for the extra lasers 
  std::list<MvrLaser *> myLasers;

  MvrFunctorC<MvrLaserLogger> myGoalKeyCB;
  MvrRetFunctor1C<bool, MvrLaserLogger, MvrRobotPacket *> myLoopPacketHandlerCB;
};

/// @deprecated
typedef MvrLaserLogger MvrSickLogger;

#endif // MVRLASERLOGGER_H