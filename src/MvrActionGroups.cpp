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
#include "MvrActionGroup.h"
#include "MvrActionGroups.h"
#include "MvrActionLimiterTableSensor.h"
#include "MvrActionLimiterForwards.h"
#include "MvrActionLimiterBackwards.h"
#include "MvrActionInput.h"
#include "MvrActionStop.h"
#include "MvrActionStallRecover.h"
#include "MvrActionBumpers.h"
#include "MvrActionAvoidFront.h"
#include "MvrActionConstantVelocity.h"
#include "MvrActionJoydrive.h"
#include "MvrActionKeydrive.h"
#include "MvrActionDeceleratingLimiter.h"
#include "MvrActionRatioInput.h"
#include "MvrRatioInputKeydrive.h"
#include "MvrRatioInputJoydrive.h"
#include "MvrRatioInputRobotJoydrive.h"

MVREXPORT MvrActionGroupInput::MvrActionGroupInput(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  addAction(new MvrActionLimiterTableSensor, 100);
  addAction(new MvrActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new MvrActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new MvrActionLimiterBackwards, 80);
  myInput = new MvrActionInput;
  addAction(myInput, 70);
}

MVREXPORT MvrActionGroupInput::~MvrActionGroupInput()
{
  removeActions();
  deleteActions();
}

MVREXPORT void MvrActionGroupInput::setVel(double vel)
{
  myInput->setVel(vel);
}

MVREXPORT void MvrActionGroupInput::setRotVel(double rotVel)
{
  myInput->setRotVel(rotVel);
}

MVREXPORT void MvrActionGroupInput::deltaHeadingFromCurrent(double delta)
{
  myInput->deltaHeadingFromCurrent(delta);
}

MVREXPORT void MvrActionGroupInput::setHeading(double heading)
{
  myInput->setHeading(heading);
}

MVREXPORT void MvrActionGroupInput::clear(void)
{
  myInput->clear();
}

MVREXPORT MvrActionInput *MvrActionGroupInput::getActionInput(void)
{
  return myInput;
}

MVREXPORT MvrActionGroupStop::MvrActionGroupStop(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  myActionStop = new MvrActionStop;
  addAction(myActionStop, 100);
}

MVREXPORT MvrActionGroupStop::~MvrActionGroupStop()
{
  removeActions();
  deleteActions();
}

MVREXPORT MvrActionStop *MvrActionGroupStop::getActionStop(void)
{
  return myActionStop;
}

MVREXPORT MvrActionGroupTeleop::MvrActionGroupTeleop(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  addAction(new MvrActionLimiterTableSensor, 100);
  addAction(new MvrActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new MvrActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new MvrActionLimiterBackwards, 80);
  myJoydrive = new MvrActionJoydrive;
  myJoydrive->setStopIfNoButtonPressed(false);
  addAction(myJoydrive, 70);
  addAction(new MvrActionKeydrive, 69);
}

MVREXPORT MvrActionGroupTeleop::~MvrActionGroupTeleop()
{
  removeActions();
  deleteActions();
}

MVREXPORT void MvrActionGroupTeleop::setThrottleParams(int lowSpeed, 
						     int highSpeed)
{
  myJoydrive->setThrottleParams(lowSpeed, highSpeed);
}

MVREXPORT MvrActionGroupUnguardedTeleop::MvrActionGroupUnguardedTeleop(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  myJoydrive = new MvrActionJoydrive;
  myJoydrive->setStopIfNoButtonPressed(false);
  addAction(myJoydrive, 70);
  addAction(new MvrActionKeydrive, 69);
}

MVREXPORT MvrActionGroupUnguardedTeleop::~MvrActionGroupUnguardedTeleop()
{
  removeActions();
  deleteActions();
}

MVREXPORT void MvrActionGroupUnguardedTeleop::setThrottleParams(int lowSpeed, 
							 int highSpeed)
{
  myJoydrive->setThrottleParams(lowSpeed, highSpeed);
}

MVREXPORT MvrActionGroupWander::MvrActionGroupWander(MvrRobot *robot, int forwardVel, int avoidFrontDist, int avoidVel, int avoidTurnAmt)
  : MvrActionGroup(robot)
{
  addAction(new MvrActionBumpers, 100);
  addAction(new MvrActionStallRecover, 90);
  //addAction(new MvrActionAvoidFront("Avoid Front Near", 250, 0), 80);
  addAction(new MvrActionAvoidFront("Avoid Front", avoidFrontDist, avoidVel, avoidTurnAmt), 79);
  addAction(new MvrActionConstantVelocity("Constant Velocity",
                                         forwardVel),
					 50);

}

MVREXPORT MvrActionGroupWander::~MvrActionGroupWander()
{
  removeActions();
  deleteActions();
}

// The color follow action group
MVREXPORT MvrActionGroupColorFollow::MvrActionGroupColorFollow(MvrRobot *robot, MvrACTS_1_2 *acts, MvrPTZ *camera)
  : MvrActionGroup(robot)
{
  // Add the limiters so the robot is less likely to run into things
  addAction(new MvrActionLimiterTableSensor, 100);
  addAction(new MvrActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new MvrActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new MvrActionLimiterBackwards, 80);

  // Construct the color follower and add it
  myColorFollow = new MvrActionColorFollow("Follow a color.", acts, camera);
  addAction(myColorFollow, 70);
}

// Destructor
MVREXPORT MvrActionGroupColorFollow::~MvrActionGroupColorFollow()
{
  removeActions();
  deleteActions();
}

// Set the channel to get blob info from
MVREXPORT void MvrActionGroupColorFollow::setChannel(int channel)
{
  myColorFollow->setChannel(channel);
}

// Set the camera to control
MVREXPORT void MvrActionGroupColorFollow::setCamera(MvrPTZ *camera)
{
  myColorFollow->setCamera(camera);
}

// Allow the robot to move
MVREXPORT void MvrActionGroupColorFollow::startMovement()
{
  myColorFollow->startMovement();
}

// Keep the robot from moving
MVREXPORT void MvrActionGroupColorFollow::stopMovement()
{
  myColorFollow->stopMovement();
}

// Toggle whether or not the robot will try to actively
// acquire a color blob
MVREXPORT void MvrActionGroupColorFollow::setAcquire(bool acquire)
{
  myColorFollow->setAcquire(acquire);
}

// Return the channel that the robot is looking on
MVREXPORT int MvrActionGroupColorFollow::getChannel()
{
  return myColorFollow->getChannel();
}

// Return whether the robot is allowed to actively
// acquire a color blob
MVREXPORT bool MvrActionGroupColorFollow::getAcquire()
{
  return myColorFollow->getAcquire();
}

// Return whether the robot is allowed to move
MVREXPORT bool MvrActionGroupColorFollow::getMovement()
{
  return myColorFollow->getMovement();
}

// Return if the robot is targeting a color blob
MVREXPORT bool MvrActionGroupColorFollow::getBlob()
{
  return myColorFollow->getBlob();
}

MVREXPORT MvrActionGroupRatioDrive::MvrActionGroupRatioDrive(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  // add the actions, put the ratio input on top, then have the
  // limiters since the ratio doesn't touch decel except lightly
  // whereas the limiter will touch it strongly

  myInput = new MvrActionRatioInput;
  addAction(myInput, 50); 

  myKeydrive = new MvrRatioInputKeydrive(robot, myInput);
  myJoydrive = new MvrRatioInputJoydrive(robot, myInput);
  myRobotJoydrive = new MvrRatioInputRobotJoydrive(robot, myInput);

  myDeceleratingLimiterForward = new MvrActionDeceleratingLimiter(
	  "DeceleratingLimiterForward", MvrActionDeceleratingLimiter::FORWARDS);
  addAction(myDeceleratingLimiterForward, 40);

  myDeceleratingLimiterBackward = new MvrActionDeceleratingLimiter(
	  "DeceleratingLimiterBackward", 
	  MvrActionDeceleratingLimiter::BACKWARDS);
  addAction(myDeceleratingLimiterBackward, 39);

  myDeceleratingLimiterLateralLeft = NULL;
  myDeceleratingLimiterLateralRight = NULL;
  if (myRobot->hasLatVel())
  {
    myDeceleratingLimiterLateralLeft = new MvrActionDeceleratingLimiter(
	    "DeceleratingLimiterLateral", 
	    MvrActionDeceleratingLimiter::LATERAL_LEFT);
    addAction(myDeceleratingLimiterLateralLeft, 38);
    myDeceleratingLimiterLateralRight = new MvrActionDeceleratingLimiter(
	    "DeceleratingLimiterLateralRight", 
	    MvrActionDeceleratingLimiter::LATERAL_RIGHT);
    addAction(myDeceleratingLimiterLateralRight, 37);
  }

}

MVREXPORT MvrActionGroupRatioDrive::~MvrActionGroupRatioDrive()
{
  removeActions();
  deleteActions();
}


MVREXPORT MvrActionRatioInput *MvrActionGroupRatioDrive::getActionRatioInput(void)
{
  return myInput;
}

MVREXPORT void MvrActionGroupRatioDrive::addToConfig(MvrConfig *config, 
						 const char *section)
{
  myInput->addToConfig(config, section);
  myDeceleratingLimiterForward->addToConfig(config, section, "Forward");
  myDeceleratingLimiterBackward->addToConfig(config, section, "Backward");
}

MVREXPORT MvrActionGroupRatioDriveUnsafe::MvrActionGroupRatioDriveUnsafe(MvrRobot *robot)
  : MvrActionGroup(robot)
{
  // add the actions, put the ratio input on top, then have the
  // limiters since the ratio doesn't touch decel except lightly
  // whereas the limiter will touch it strongly

  myInput = new MvrActionRatioInput;
  addAction(myInput, 50); 

  myKeydrive = new MvrRatioInputKeydrive(robot, myInput);
  myJoydrive = new MvrRatioInputJoydrive(robot, myInput);
  myRobotJoydrive = new MvrRatioInputRobotJoydrive(robot, myInput);
}

MVREXPORT MvrActionGroupRatioDriveUnsafe::~MvrActionGroupRatioDriveUnsafe()
{
  removeActions();
  deleteActions();  
}


MVREXPORT MvrActionRatioInput *MvrActionGroupRatioDriveUnsafe::getActionRatioInput(void)
{
  return myInput;
}

MVREXPORT void MvrActionGroupRatioDriveUnsafe::addToConfig(MvrConfig *config, 
						 const char *section)
{
  myInput->addToConfig(config, section);
}
