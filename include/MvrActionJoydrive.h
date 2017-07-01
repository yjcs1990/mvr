#ifndef MVRACTIONJOYDRIVE_H
#define MVRACTIONJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrJoyHandler.h"

/// This action will use the joystick for input to drive the robot
/**
   This class creates its own MvrJoyHandler to get input from the
   joystick.  Then it will scale the speed between 0 and the given max
   for velocity and turning, up and down on the joystick go
   forwards/backwards while right and left go right and left.  You
   must press in one of the two joystick buttons for the class to pay
   attention to the joystick.
   
   @note The joystick does not save calibration information, so you
   must calibrate the joystick before each time you use it.  To do
   this, press the button for at least a half a second while the
   joystick is in the middle.  Then let go of the button and hold the
   joystick in the upper left for at least a half second and then in
   the lower right corner for at least a half second.

   @ingroup ActionClasses
**/
class MvrActionJoydrive : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionJoydrive(const char * name = "joydrive", 
			    double transVelMax = 400, 
			    double turnAmountMax = 15, 
			    bool stopIfNoButtonPressed = true,
			    bool useOSCalForJoystick = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionJoydrive();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  /// Whether the joystick is initalized or not
  MVREXPORT bool joystickInited(void);
  /// Set Speeds
  MVREXPORT void setSpeeds(double transVelMax, double turnAmountMax);
  /// Set if we'll stop if no button is pressed, otherwise just do nothing
  MVREXPORT void setStopIfNoButtonPressed(bool stopIfNoButtonPressed);
  /// Get if we'll stop if no button is pressed, otherwise just do nothing
  MVREXPORT bool getStopIfNoButtonPressed(void);
  /// Sets the params on the throttle (throttle unused unless you call this)
  MVREXPORT void setThrottleParams(double lowSpeed, double highSpeed);
  /// Sets whether to use OSCalibration the joystick or not
  MVREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether OSCalibration is being used for the joystick or not
  MVREXPORT bool getUseOSCal(void);
  /// Gets the joyHandler
  MVREXPORT MvrJoyHandler *getJoyHandler(void) { return myJoyHandler; }
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const { return &myDesired; }
#endif
protected:
  bool myUseThrottle;
  double myLowThrottle;
  double myHighThrottle;
  // action desired
  MvrActionDesired myDesired;
  // joystick handler
  MvrJoyHandler *myJoyHandler;
  // full spped
  double myTransVelMax;
  // full amount to turn
  double myTurnAmountMax;
  // if we want to stop when no button is presesd
  bool myStopIfNoButtonPressed;
  // if we're using os cal for the joystick
  bool myUseOSCal;
  bool myPreviousUseOSCal;
};

#endif //MvrACTIONJOYDRIVE_H
