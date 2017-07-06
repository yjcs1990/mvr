/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionJoydrive.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"

/**
   This action is for driving around the robot with a joystick, you
   must hold in a button on the joystick and then lean the joytsick
   over to have it drive.  You may need to calibrate the joystick for
   it to work right, for details about this see MvrJoyHandler.  If the
   Mvr static class already has a joyhandler this class will use that
   otherwise it'll make and initialize one and use that (setting it in
   the Mvr class)

   @param name the name of this action

   @param transVelMax the maximum velocity the joydrive action will
   go, it reachs this when the joystick is all the way forwards

   @param turnAmountMax the maximum amount the joydrive action will
   turn, it reachs this when the joystick is all the way forwards

   @param stopIfNoButtonPressed if this is true and there is a
   joystick and no button is pressed, the action will have the robot
   stop... otherwise it'll do nothing (letting lower priority actions
   fire)

   @param useOSCalForJoystick If true, then the existing OS calibration 
   for the joystick will be used, otherwise our own autocalibration will 
   be used. Default is true.


   @see MvrJoyHandler::setUseOSCal()
**/

MVREXPORT MvrActionJoydrive::MvrActionJoydrive(const char *name, 
					    double transVelMax, 
					    double turnAmountMax,
					    bool stopIfNoButtonPressed,
					    bool useOSCalForJoystick) :
  MvrAction(name, "This action reads the joystick and sets the translational and rotational velocities based on this.")
{
  if ((myJoyHandler = Mvria::getJoyHandler()) == NULL)
  {
    myJoyHandler = new MvrJoyHandler;
    myJoyHandler->init();
    Mvria::setJoyHandler(myJoyHandler);
  }

  setSpeeds(transVelMax, turnAmountMax);

  setNextArgument(MvrArg("trans vel max", &myTransVelMax, "The full speed to go when the joystick is maxed forwards/backwards (mm/sec)"));
  
  setNextArgument(MvrArg("turn amount max", &myTurnAmountMax, "The full amount to turn if the joystick is pushed all the way right/left (deg)"));

  setNextArgument(MvrArg("stop if no button pressed", &myStopIfNoButtonPressed, "If this is true, then joydrive will stop the robot if there is no button pressed, otherwise it will just let whatever lower priority things go."));
  myStopIfNoButtonPressed = stopIfNoButtonPressed;

  setNextArgument(MvrArg("use os calibration for joystick", &myUseOSCal, "If this is true then the os calibration for the joystick will be used, otherwise autocalibration will be used."));
  myUseOSCal = useOSCalForJoystick;
  myPreviousUseOSCal = myUseOSCal;

  myUseThrottle = false;
}

MVREXPORT MvrActionJoydrive::~MvrActionJoydrive()
{

}

MVREXPORT void MvrActionJoydrive::setStopIfNoButtonPressed(
	bool stopIfNoButtonPressed)
{
  myStopIfNoButtonPressed = stopIfNoButtonPressed;
}

MVREXPORT bool MvrActionJoydrive::getStopIfNoButtonPressed(void)
{
  return myStopIfNoButtonPressed;
}

MVREXPORT bool MvrActionJoydrive::joystickInited(void)
{
  return myJoyHandler->haveJoystick();
}

/**
   @see MvrJoyHandler::setUseOSCal
**/
MVREXPORT void MvrActionJoydrive::setUseOSCal(bool useOSCal)
{
  myUseOSCal = useOSCal;
  myPreviousUseOSCal = useOSCal;
  myJoyHandler->setUseOSCal(useOSCal);
}

/**
   @see MvrJoyHandler::getUseOSCal
**/
MVREXPORT bool MvrActionJoydrive::getUseOSCal(void)
{
  return myUseOSCal;
}

MVREXPORT void MvrActionJoydrive::setSpeeds(double transVelMax, 
					  double turnAmountMax)
{
  myTransVelMax = transVelMax;
  myTurnAmountMax = turnAmountMax;
}

MVREXPORT void MvrActionJoydrive::setThrottleParams(double lowSpeed, double highSpeed)
{
  myUseThrottle = true;
  myLowThrottle = lowSpeed;
  myHighThrottle = highSpeed;
}

MVREXPORT MvrActionDesired *MvrActionJoydrive::fire(MvrActionDesired currentDesired)
{
  double rot, trans, throttle;

  myDesired.reset();
  if (myPreviousUseOSCal != myUseOSCal)
  {
    myJoyHandler->setUseOSCal(myUseOSCal);
    myPreviousUseOSCal = myUseOSCal;
  }

  if (myJoyHandler->haveJoystick() && myJoyHandler->getButton(1))
  {
    // get the readings from the joystick
    myJoyHandler->getDoubles(&rot, &trans);
    rot *= myTurnAmountMax;
    // if we're not using the throttle just mult simply, or if we
    // don't know if we have a throttle
    if (!myUseThrottle || !myJoyHandler->haveZAxis()) 
    {
      trans *= myTransVelMax;
    }
    // if we are using the throttle, interpolate its position between
    // low and high throttle values
    else
    {
      throttle = myJoyHandler->getAxis(3);
      throttle += 1.0;
      throttle /= 2.0;
      trans = trans * (myLowThrottle + 
		       (throttle * (myHighThrottle - myLowThrottle)));
    }
    // set what we want to do
    myDesired.setVel(trans);
    myDesired.setDeltaHeading(-rot);
    // return the actionDesired
    return &myDesired;
  }
  else if (myJoyHandler->haveJoystick() && myStopIfNoButtonPressed)
  {
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;
  }
  
  return NULL;
}
