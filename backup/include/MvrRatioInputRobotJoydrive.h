/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRatioInputRobotJoydrive.h
 > Description  : Use robot's joystick to control an MvrActionRatioInput action and drive the robot.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRATIOINPUTROBOTJOYDRIVE_H
#define MVRRATIOINPUTROBOTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrActionRatioInput.h"

class ArRobotPacket;
class ArRobot;
class ArRobotJoyHandler;

/*
   This class connects the joystick data obtained from  the robot's built
   in joystick port (if it has one, not all robots have a joystick port)
   to an MvrActionRatioInput which drives the robot. (See MvrRatioInputJoydrive
   for a similar class that uses a joystick plugged in to the computer.)

   A callback is attached to the MvrActionRatioInput object which reads joystick
   information using an MvrRobotJoyHandler object, and sets requested drive rations on the MvrActionRatioInput
   object.
*/
class MvrRatioInputRobotJoydrive 
{
public:
  /// Constructor
  MVREXPORT MvrRatioInputRobotJoydrive(MvrRobot *robot, 
                                       MvrActionRatioInput *input,
                                       int priority = 75,
                                       bool requireDeadmanPushed = true);
  /// Destructor
  MVREXPORT virtual ~MvrRatioInputRobotJoydrive();
protected:
  MVREXPORT void fireCallback(void);

  MvrRobot *myRobot;
  MvrActionRatioInput *myInput;
  bool myRequireDeadmanPushed;
  bool myDeadZoneLast;

  MvrRobotJoyHandler *myRobotJoyHandler;
  MvrFunctorC<MvrRatioInputRobotJoydrive> myFireCB;
};

#endif // MVRRATIOINPUTROBOTJOYDRIVE_H
