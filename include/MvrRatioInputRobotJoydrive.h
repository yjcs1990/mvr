#ifndef MVRRATIOINPUTROBOTJOYDRIVE_H
#define MVRRATIOINPUTROBOTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrActionRatioInput.h"

class MvrRobotPacket;
class MvrRobot;
class MvrRobotJoyHandler;

/// Use robot's joystick to control an MvrActionRatioInput action and drive the robot.
/**
   This class connects the joystick data obtained from  the robot's built
   in joystick port (if it has one, not all robots have a joystick port)
   to an MvrActionRatioInput which drives the robot. (See MvrRatioInputJoydrive
   for a similar class that uses a joystick plugged in to the computer.)

   A callback is attached to the MvrActionRatioInput object which reads joystick
   information using an MvrRobotJoyHandler object, and sets requested drive rations on the MvrActionRatioInput
   object.


    @sa MvrRatioInputJoydrive
    @sa MvrActionRatioInput


  @ingroup OptionalClasses
**/
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

#endif //MVRRATIOINPUTROBOTJOYDRIVE_H
