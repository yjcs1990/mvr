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
#include "ariaOSDef.h"
#include "MvrActionKeydrive.h"
#include "MvrRobot.h"
#include "ariaInternal.h"
#include "MvrKeyHandler.h"

AREXPORT MvrActionKeydrive::ArActionKeydrive(const char *name,
					    double transVelMax,
					    double turnAmountMax,
					    double velIncrement,
					    double turnIncrement)
  :
  MvrAction(name, "This action reads the keyboard arrow keys and sets the translational and rotational velocities based on this."),
  myUpCB(this, &ArActionKeydrive::up),
  myDownCB(this, &ArActionKeydrive::down),
  myLeftCB(this, &ArActionKeydrive::left),
  myRightCB(this, &ArActionKeydrive::right),
  mySpaceCB(this, &ArActionKeydrive::space)
{
  setNextArgument(MvrArg("trans vel max", &myTransVelMax, "The maximum speed to go (mm/sec)"));
  myTransVelMax = transVelMax;

  setNextArgument(MvrArg("turn amount max", &myTurnAmountMax, "The maximum amount to turn (deg/cycle)"));
  myTurnAmountMax = turnAmountMax;

  setNextArgument(MvrArg("vel increment per keypress", &myVelIncrement, "The amount to increment velocity by per keypress (mm/sec)"));
  myVelIncrement = velIncrement;
  
  setNextArgument(MvrArg("turn increment per keypress", &myVelIncrement, "The amount to turn by per keypress (deg)"));
  myTurnIncrement = turnIncrement;

  myDesiredSpeed = 0;
  myDeltaVel = 0;
  myTurnAmount = 0;
  mySpeedReset = true;
}

AREXPORT MvrActionKeydrive::~ArActionKeydrive()
{

}

AREXPORT void MvrActionKeydrive::setRobot(MvrRobot *robot)
{
  MvrKeyHandler *keyHandler;
  myRobot = robot;
  if (robot == NULL)
    return;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    keyHandler = new MvrKeyHandler;
    Mvria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}

AREXPORT void MvrActionKeydrive::takeKeys(void)
{
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrActionKeydrive::takeKeys: There is no key handler, keydrive will not work.");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->addKeyHandler(MvrKeyHandler::UP, &myUpCB))
    MvrLog::log(MvrLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::DOWN, &myDownCB))
    MvrLog::log(MvrLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::LEFT, &myLeftCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::RIGHT, &myRightCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for right, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::SPACE, &mySpaceCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for space, keydrive will not work correctly.");
}

AREXPORT void MvrActionKeydrive::giveUpKeys(void)
{
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrActionKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    MvrLog::log(MvrLog::Terse, "MvrActionKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    MvrLog::log(MvrLog::Terse, "MvrActionKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler didn't have a key for space, something is wrong.");
}

AREXPORT void MvrActionKeydrive::setSpeeds(double transVelMax, 
					  double turnAmountMax)
{
  myTransVelMax = transVelMax;
  myTurnAmountMax = turnAmountMax;
}

AREXPORT void MvrActionKeydrive::setIncrements(double velIncrement, 
					      double turnIncrement)
{
  myVelIncrement = velIncrement;
  myTurnIncrement = turnIncrement;
}

AREXPORT void MvrActionKeydrive::up(void)
{
  myDeltaVel += myVelIncrement;
}

AREXPORT void MvrActionKeydrive::down(void)
{
  myDeltaVel -= myVelIncrement;
}

AREXPORT void MvrActionKeydrive::left(void)
{
  myTurnAmount += myTurnIncrement;
  if (myTurnAmount > myTurnAmountMax)
    myTurnAmount = myTurnAmountMax;
}

AREXPORT void MvrActionKeydrive::right(void)
{
  myTurnAmount -= myTurnIncrement;
  if (myTurnAmount < -myTurnAmountMax)
    myTurnAmount = -myTurnAmountMax;
}

AREXPORT void MvrActionKeydrive::space(void)
{
  mySpeedReset = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

AREXPORT void MvrActionKeydrive::activate(void)
{
  if (!myIsActive)
    takeKeys();
  myIsActive = true;
}

AREXPORT void MvrActionKeydrive::deactivate(void)
{
  if (myIsActive)
    giveUpKeys();
  myIsActive = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

AREXPORT MvrActionDesired *ArActionKeydrive::fire(MvrActionDesired currentDesired)
{
  myDesired.reset();

  // if we don't have any strength left
  if (fabs(currentDesired.getVelStrength() - 1.0) < .0000000000001)
  {
    mySpeedReset = true;
  }

  // if our speed was reset, set our desired to how fast we're going now
  if (mySpeedReset && myDesiredSpeed > 0 && myDesiredSpeed > myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  if (mySpeedReset && myDesiredSpeed < 0 && myDesiredSpeed < myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  mySpeedReset = false;

  if (currentDesired.getMaxVelStrength() && 
      myDesiredSpeed > currentDesired.getMaxVel())
    myDesiredSpeed = currentDesired.getMaxVel();

  if (currentDesired.getMaxNegVelStrength() && 
      myDesiredSpeed < currentDesired.getMaxNegVel())
    myDesiredSpeed = currentDesired.getMaxNegVel();

  myDesiredSpeed += myDeltaVel;
  if (myDesiredSpeed > myTransVelMax)
    myDesiredSpeed = myTransVelMax;
  if (myDesiredSpeed < -myTransVelMax)
    myDesiredSpeed = -myTransVelMax;

  myDesired.setVel(myDesiredSpeed);
  myDeltaVel = 0;
  
  myDesired.setDeltaHeading(myTurnAmount);
  myTurnAmount = 0;

  
  return &myDesired;
}
