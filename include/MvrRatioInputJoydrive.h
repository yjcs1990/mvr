/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRatioInputJoydrive.h
 > Description  : Use computer joystick to control an MvrActionRatioInput and drive the robot.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRATIOINPUTJOYDRIVE_H
#define MVRRATIOINPUTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrActionRatioInput.h"
#include "MvrJoyHandler.h"

class MvrRobot;
/*
   This class obtains data from a joystick attached to the computer
   and provides it to an MvrActionRatioInput which drives the robot. 

   A callback is attached to the MvrActionRatioInput object which reads joystick
   information and sets requested drive rations on the MvrActionRatioInput

   If the joystick button is pressed, then input values are set in the
   MvrActionRatioDrive action object to request motion.
   If the button is not pressed, then either the robot will be stopped via the
   action, or no action will be requested and lower priority actions can take over
   
   You may need to calibrate the joystick for
   it to work right, for details about this see MvrJoyHandler.  
   
   This class creates its own MvrJoyHandler object to get input from the
   joystick, or uses the global MvrJoyHandler object in the global Mvria class if present.
   Then it will scale the speed between 0 and the given max
   for velocity and turning, up and down on the joystick go
   forwards/backwards while right and left go right and left.  
  
*/
class MvrRatioInputJoydrive
{
public:
  /// Constructor
  MVREXPORT MvrRatioInputJoydrive(MvrRobot *robot, MvrActionRatioInput *input,
				int priority = 50,
				bool stopIfNoButtonPressed = false,
				bool useOSCalForJoystick = true);
  /// Destructor
  MVREXPORT virtual ~MvrRatioInputJoydrive();
  /// Whether the joystick is initalized or not
  MVREXPORT bool joystickInited(void);
  /// Set if we'll stop if no button is pressed, otherwise just do nothing
  MVREXPORT void setStopIfNoButtonPressed(bool stopIfNoButtonPressed);
  /// Get if we'll stop if no button is pressed, otherwise just do nothing
  MVREXPORT bool getStopIfNoButtonPressed(void);
  /// Sets whether to use OSCalibration the joystick or not
  MVREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether OSCalibration is being used for the joystick or not
  MVREXPORT bool getUseOSCal(void);
  /// Gets the joyHandler
  MVREXPORT MvrJoyHandler *getJoyHandler(void) { return myJoyHandler; }
protected:
  void fireCallback(void);
  MvrRobot *myRobot;
  MvrActionRatioInput *myInput;
  // if we're printing extra information for tracing and such
  bool myPrinting;
  // joystick handler
  MvrJoyHandler *myJoyHandler;
  bool myFiredLast;
  // if we want to stop when no button is presesd
  bool myStopIfNoButtonPressed;
  // if we're using os cal for the joystick
  bool myUseOSCal;
  bool myPreviousUseOSCal;
  MvrFunctorC<MvrRatioInputJoydrive> myFireCB;
};

#endif //MVRRATIOINPUTJOYDRIVE_H
