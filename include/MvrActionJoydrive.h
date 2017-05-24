/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionJoydrive.h
 > Description  : This action will use the joystick for input to drive the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONJOYDRIVE_H
#define MVRACTIONJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrJoyHandler.h"

class MvrActionJoydrive : public MvrAction
{
public:
  /// Constructor
  AREXPORT MvrActionJoydrive(const char * name = "joydrive", 
			    double transVelMax = 400, 
			    double turnAmountMax = 15, 
			    bool stopIfNoButtonPressed = true,
			    bool useOSCalForJoystick = true);
  /// Destructor
  AREXPORT virtual ~MvrActionJoydrive();
  AREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  /// Whether the joystick is initalized or not
  AREXPORT bool joystickInited(void);
  /// Set Speeds
  AREXPORT void setSpeeds(double transVelMax, double turnAmountMax);
  /// Set if we'll stop if no button is pressed, otherwise just do nothing
  AREXPORT void setStopIfNoButtonPressed(bool stopIfNoButtonPressed);
  /// Get if we'll stop if no button is pressed, otherwise just do nothing
  AREXPORT bool getStopIfNoButtonPressed(void);
  /// Sets the params on the throttle (throttle unused unless you call this)
  AREXPORT void setThrottleParams(double lowSpeed, double highSpeed);
  /// Sets whether to use OSCalibration the joystick or not
  AREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether OSCalibration is being used for the joystick or not
  AREXPORT bool getUseOSCal(void);
  /// Gets the joyHandler
  AREXPORT MvrJoyHandler *getJoyHandler(void) { return myJoyHandler; }
  AREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const MvrActionDesired *getDesired(void) const { return &myDesired; }
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

#endif  // MVRACTIONJOYDRIVE_H