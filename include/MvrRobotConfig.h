#ifndef MVRROBOTCONFIG_H
#define MVRROBOTCONFIG_H

#include "MvrFunctor.h"

class MvrRobot;
class MvrAnalogGyro;

/// Class for controlling robot movement parameters from config
class MvrRobotConfig
{
public:
  /// Constructor
  MVREXPORT MvrRobotConfig(MvrRobot *robot);
  /// Destructor
  MVREXPORT virtual ~MvrRobotConfig();
  /// Adds a gyro to turn on and off
  MVREXPORT void addAnalogGyro(MvrAnalogGyro *gyro);
  /// Called when we process the config
  MVREXPORT bool processFile(void);
  /// Called when we connect to the robot
  MVREXPORT void connectCallback(void);
protected:
  MvrRobot *myRobot;
  MvrAnalogGyro *myAnalogGyro;

  bool mySavedOriginalMovementParameters;
  int myOriginalTransVelMax;
  int myOriginalTransAccel;
  int myOriginalTransDecel;
  int myOriginalRotVelMax;
  int myOriginalRotAccel;
  int myOriginalRotDecel;

  bool myAddedMovementParams;
  int myTransVelMax;
  int myTransAccel;
  int myTransDecel;
  int myRotVelMax;
  int myRotAccel;
  int myRotDecel;

  bool myAddedGyro;
  bool myUseGyro;

  MvrFunctorC<MvrRobotConfig> myConnectCB;  
  MvrRetFunctorC<bool, MvrRobotConfig> myProcessFileCB;
};

#endif
