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
#include <time.h>
#include "MvrRobot.h"
#include "MvrResolver.h"
#include "MvrActionStallRecover.h"

/**
   @param name name of the action
   @param obstacleDistance distance at which not to move because of 
   obstacle. (mm)
   @param cyclesToMove default number of cycles to move (# of cycles) (may be
changed via MvrConfig)
   @param speed default speed at which to back up or go forward (mm/sec) (may be
changed via MvrConfig)
   @param degreesToTurn default number of degrees to turn (deg) (may be changed vi
MvrConfig)
   @param enabled default value of "enabled" state configuration parameter (may
be changed via MvrConfig)
*/
MVREXPORT
MvrActionStallRecover::MvrActionStallRecover(const char * name,
					   double obstacleDistance, 
					   int cyclesToMove, 
					   double speed, 
					   double degreesToTurn,
					   bool enabled) :
    MvrAction(name, "Recovers the robot from a stall.")
{
  setNextArgument(MvrArg("obstacle distance", &myObstacleDistance, 
		"Distance at which not to move because of obstacle. (mm)"));
  myObstacleDistance = obstacleDistance;
  setNextArgument(MvrArg("cycles to move", &myCyclesToMove, 
			"Number of cycles to move (# of cycles)"));
  myCyclesToMove = cyclesToMove;
  setNextArgument(MvrArg("speed", &mySpeed, 
			"Speed at which to back up or go forward (mm/sec)"));
  mySpeed = speed;
  setNextArgument(MvrArg("degrees to turn", &myDegreesToTurn, 
			"Number of Degrees to turn (deg)"));
  myDegreesToTurn = degreesToTurn;
  setNextArgument(MvrArg("enabled", &myEnabled, 
			"Whether stall recover is enabled or not"));
  myEnabled = enabled;

  myState = STATE_NOTHING;

  myResolver = NULL;
  mySequenceNum = 0;
  addSequence(BACK | TURN);
  addSequence(FORWARD | TURN);
  addSequence(BACK);
  addSequence(FORWARD);
  addSequence(TURN_LEFT);
  addSequence(TURN_RIGHT);
  addSequence(BACK | TURN_LEFT);
  addSequence(BACK | TURN_RIGHT);
  addSequence(FORWARD | TURN_LEFT);
  addSequence(FORWARD | TURN_RIGHT);
  mySequencePos = 0;
  myLastFired = 0;
}

MVREXPORT
MvrActionStallRecover::~MvrActionStallRecover()
{


}

MVREXPORT void MvrActionStallRecover::activate(void)
{
  myState = STATE_NOTHING;
  MvrAction::activate();
}

void MvrActionStallRecover::addSequence(int sequence)
{
  mySequence[mySequenceNum] = sequence;
  ++mySequenceNum;
}

MVREXPORT
MvrActionDesired *MvrActionStallRecover::fire(MvrActionDesired currentDesired)
{
  std::string doingString;

  if (!myEnabled)
    return NULL;
  
  if (myRobot->isLeftMotorStalled())
    MvrLog::log(MvrLog::Verbose, "########## Left stall");
  if (myRobot->isRightMotorStalled())
    MvrLog::log(MvrLog::Verbose, "########## Right stall");

  if ((currentDesired.getVelStrength() >= 1.0 || 
       currentDesired.getDeltaHeadingStrength() >= 1.0 ||
       currentDesired.getHeadingStrength() >= 1.0 ||
       currentDesired.getRotVelStrength() >= 1.0))
  {
    if (myState != STATE_NOTHING)
    {
      myState = STATE_NOTHING;
      myCount = -1;
      MvrLog::log(MvrLog::Normal, "StallRecover: done (interrupted)");
    }
    return NULL;
  }

  myActionDesired.reset();
  switch (myState)
  {
  case STATE_NOTHING:
    if (!myRobot->isLeftMotorStalled() && !myRobot->isRightMotorStalled())
      return NULL;
    if (myRobot->isLeftMotorStalled() && myRobot->isRightMotorStalled())
      doingString = "StallRecover: Both motors stalled";
    else if (myRobot->isLeftMotorStalled())
      doingString = "StallRecover: Left motor stalled";
    else if (myRobot->isRightMotorStalled())
      doingString = "StallRecover: Right motor stalled";
    if (time(NULL) - myLastFired <= 5)
    {
      //printf("Didn't reset... %ld time diff\n", time(NULL) - myLastFired);
      ++mySequencePos;
    }
    else 
    {
      //printf("Reset\n");
      mySequencePos = 0;
    }
    if (mySequencePos >= mySequenceNum)
      mySequencePos = 0;
    myDoing = mySequence[mySequencePos];
    if (myDoing & FORWARD)
      doingString += " driving_forward";
    if (myDoing & BACK)
      doingString += " driving_backward";
    if (myDoing & TURN)
      doingString += " turning";
    if (myDoing & TURN_LEFT)
      doingString += " turning_left";
    if (myDoing & TURN_RIGHT)
      doingString += " turning_right";
    if (myDoing == 0)
      doingString += " messed";
    MvrLog::log(MvrLog::Normal, doingString.c_str());
    myActionDesired.setVel(0);
    myActionDesired.setDeltaHeading(0);
  case STATE_GOING:
    myLastFired = time(NULL);
    doit();
    break;
  default:
    MvrLog::log(MvrLog::Normal, "StallRecover: Bad state");
    break;
  }
  return &myActionDesired;
}

void MvrActionStallRecover::doit(void)
{
  double leftDist, rightDist;
  double dist;
  int turnDirection;
  bool transFired, rotFired;

  if (myCount == -1 || myState != STATE_GOING)
  {
    myCount = myCyclesToMove;
    myState = STATE_GOING;
    mySideStalled = 0;
    if (myRobot->isLeftMotorStalled())
      mySideStalled += 1;
    if (myRobot->isRightMotorStalled())
      mySideStalled += 2;
    if (myDoing & TURN) 
    {
      rightDist = myRobot->checkRangeDevicesCurrentPolar(-120, -60);
      leftDist = myRobot->checkRangeDevicesCurrentPolar(60, 120);
      if (mySideStalled == 1 || rightDist < 0)
	turnDirection = -1;
      else if (mySideStalled == 2 || leftDist < 0)
	turnDirection = 1;
      else if (rightDist < leftDist)
	turnDirection = 1;
      else
	turnDirection = -1;
      myDesiredHeading = myRobot->getTh() + turnDirection * myDegreesToTurn;
    }
    else if (myDoing & TURN_LEFT)
      myDesiredHeading = myRobot->getTh() + myDegreesToTurn;
    else if (myDoing & TURN_RIGHT)
      myDesiredHeading = myRobot->getTh() + -1 * myDegreesToTurn;
  }
  else
    --myCount;

  if (myDoing & MOVEMASK)
  {
    // if we're moving but not turning then don't turn
    if (!(myDoing & TURNMASK))
	myActionDesired.setRotVel(0);

    if (myDoing & FORWARD)
      dist = (myRobot->checkRangeDevicesCurrentPolar(-120, 120) -
	      myRobot->getRobotRadius());
    else if (myDoing & BACK)
      dist = (myRobot->checkRangeDevicesCurrentPolar(120, -120) -
	      myRobot->getRobotRadius());
    if ((myCount <= 0 || 
	 (dist  > 0 && dist < myObstacleDistance &&
	  myCount <= myCyclesToMove / 2)))
    {
      //printf("Not backing ");
      myActionDesired.setVel(0);
      transFired = false;
    } 
    else
    {
      //printf("backing ");
      if (myDoing & BACK)
	myActionDesired.setVel(mySpeed * -1);
      if (myDoing & FORWARD)
	myActionDesired.setVel(mySpeed);
      transFired = true;
    }
  } 
  else
  {
    transFired = false;
  }

  if ((myDoing & TURNMASK))
  {
    // if we're turning but not moving then don't move
    if (!(myDoing & MOVEMASK))
      myActionDesired.setVel(0);

    if ((myCount <= 0 || myDegreesToTurn == 0 || 
	 MvrMath::fabs(MvrMath::subAngle(myRobot->getTh(),myDesiredHeading)) < 3))
    {
      //printf("Not turning \n");
      myActionDesired.setRotVel(0);
      rotFired = false;
    }
    else 
    {
      //printf("turning \n");
      myActionDesired.setHeading(myDesiredHeading);
      rotFired = true;
    } 
  }
  else
  {
    rotFired = false;
  }


  if (myCount <= 0 || (!transFired && !rotFired))
  {
    myState = STATE_NOTHING;
    MvrLog::log(MvrLog::Normal, "StallRecover: done");
    myActionDesired.reset();
    return;
  }
}

void MvrActionStallRecover::addToConfig(MvrConfig* config, const char* sectionName, MvrPriority::Priority priority)
{
  if (config == NULL || sectionName == NULL)
  {
    MvrLog::log(MvrLog::Terse, "Could not add MvrActionStallRecoverToConfig because config (%p) or section (%p) are null", config, sectionName);
    return;
  }
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), sectionName, priority);
  config->addParam(MvrConfigArg("StallRecoverEnabled", &myEnabled, "Whether a stall recover should be done when the robot stalls."), sectionName, priority);
  config->addParam(MvrConfigArg("StallRecoverSpeed", &mySpeed, "Speed at which to back away when stalled.", 0), sectionName, priority);
  config->addParam(MvrConfigArg("StallRecoverDuration", &myCyclesToMove, "Cycles of operation to move when recovering from stall.", 0), sectionName, priority);
  config->addParam(MvrConfigArg("StallRecoverRotation", &myDegreesToTurn, "Amount of rotation when recovering (degrees).", 0), sectionName, priority);
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), sectionName, priority);
}

