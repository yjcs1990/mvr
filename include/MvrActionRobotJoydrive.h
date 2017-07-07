#ifndef MVRACTIONROBOTJOYDRIVE_H
#define MVRACTIONROBOTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrRobotPacket;

/// This action will use the joystick for input to drive the robot
/**
   This class creates its own MvrJoyHandler to get input from the
   joystick.  Then it will scale the speed between 0 and the given max
   for velocity and turning, up and down on the joystick go
   forwards/backwards while right and left go right and left.  You
   must press in one of the two joystick buttons for the class to pay
   attention to the joystick.
   
   NOTE: The joystick does not save calibration information, so you
   must calibrate the joystick before each time you use it.  To do
   this, press the button for at least a half a second while the
   joystick is in the middle.  Then let go of the button and hold the
   joystick in the upper left for at least a half second and then in
   the lower right corner for at least a half second.

   @ingroup ActionClasses
**/
class MvrActionRobotJoydrive : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionRobotJoydrive(const char * name = "robotJoyDrive", 
				 bool requireDeadmanPushed = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionRobotJoydrive();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  MVREXPORT virtual void setRobot(MvrRobot *robot);
protected:
  MVREXPORT bool handleJoystickPacket(MvrRobotPacket *packet);
  MVREXPORT void connectCallback(void);
  // whether we require the deadman to be pushed to drive
  bool myRequireDeadmanPushed;

  bool myDeadZoneLast;
  int myButton1;
  int myButton2;
  int myJoyX;
  int myJoyY;
  int myThrottle;
  MvrTime myPacketReceivedTime;
  // action desired
  MvrActionDesired myDesired;
  MvrRetFunctor1C<bool, MvrActionRobotJoydrive, 
      MvrRobotPacket *> myHandleJoystickPacketCB;
  MvrFunctorC<MvrActionRobotJoydrive> myConnectCB;
};

#endif //MVRACTIONROBOTJOYDRIVE_H
