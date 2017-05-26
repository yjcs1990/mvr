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
  MVREXPORT MvrActionJoydrive(const char * name = "joydrive", 
			    double transVelMax = 400, 
			    double turnAmountMax = 15, 
			    bool stopIfNoButtonPressed = true,
			    bool useOSCalForJoystick = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionJoydrive();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  /// Whether the joystick is initialized or not
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
  // full speed
  double myTransVelMax;
  // full amount to turn
  double myTurnAmountMax;
  // if we want to stop when no button is pressed
  bool myStopIfNoButtonPressed;
  // if we're using os cal for the joystick
  bool myUseOSCal;
  bool myPreviousUseOSCal;
};

#endif  // MVRACTIONJOYDRIVE_H