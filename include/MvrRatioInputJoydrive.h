#ifndef MVRRATIOINPUTJOYDRIVE_H
#define MVRRATIOINPUTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrActionRatioInput.h"
#include "MvrJoyHandler.h"

class MvrRobot;

/// Use computer joystick to control an MvrActionRatioInput and drive the robot.

/**
   This class obtains data from
   a joystick attached to the computer
   and provides it to an MvrActionRatioInput which drives the robot. 
   (See MvrRatioInputRobotJoydrive for a similar class that uses the robot's built in
   joystick interface.)
   A callback is attached to the MvrActionRatioInput object which reads joystick
   information and sets requested drive rations on the MvrActionRatioInput

   If the joystick button is pressed, then input values are set in the
   MvrActionRatioDrive action object to request motion.
   If the button is not pressed, then either the robot will be stopped via the
   action (if @a stopIfNoButtonPressed is true), or no action will be requested
   and lower priority actions can take over (if @a stopIfNoButtonPressed is
   false) 
   
   You may need to calibrate the joystick for
   it to work right, for details about this see MvrJoyHandler.  
   
   This class creates its own MvrJoyHandler object to get input from the
   joystick, or uses the global MvrJoyHandler object in the global Mvr class if present.  Then it will scale the speed between 0 and the given max
   for velocity and turning, up and down on the joystick go
   forwards/backwards while right and left go right and left.  
   
   NOTE: The joystick does not save calibration information, so you
   may need to calibrate the joystick before each time you use it.  To do
   this, press the button for at least a half a second while the
   joystick is in the middle.  Then let go of the button and hold the
   joystick in the upper left for at least a half second and then in
   the lower right corner for at least a half second. See also MvrJoyHandler. 

    @sa MvrRatioInputRobotJoydrive
    @sa MvrActionRatioInput
    @sa MvrJoyHandler

  @ingroup OptionalClasses
**/
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
