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
#include "MvrActionGotoStraight.h"
#include "MvrRobot.h"

MVREXPORT MvrActionGotoStraight::ArActionGotoStraight(const char *name,
						    double speed) :
  MvrAction(name, "Goes to the given goal.")
{
  myPrinting = false;

  setNextArgument(MvrArg("speed", &mySpeed, 
			"Speed to travel to goal at. (mm/sec)"));
  mySpeed = speed;
  myState = STATE_NO_GOAL;

  myUseEncoderGoal = false;
  myBacking = false;
  setCloseDist();
}

MVREXPORT MvrActionGotoStraight::~MvrActionGotoStraight()
{

}

MVREXPORT bool MvrActionGotoStraight::haveAchievedGoal(void)
{
  if (myState == STATE_ACHIEVED_GOAL)
    return true;
  else
    return false;
}

MVREXPORT void MvrActionGotoStraight::cancelGoal(void)
{
  myState = STATE_NO_GOAL;
}

MVREXPORT void MvrActionGotoStraight::setGoal(MvrPose goal, bool backToGoal, 
					    bool justDistance)
{
  myState = STATE_GOING_TO_GOAL;
  myGoal = goal;
  myUseEncoderGoal = false;
  myBacking = backToGoal;
  myLastPose = myRobot->getPose();
  myDist = myRobot->getPose().findDistanceTo(goal);
  myJustDist = true;
  myDistTravelled = 0;
}

MVREXPORT void MvrActionGotoStraight::setGoalRel(double dist, 
					       double deltaHeading,
					       bool backToGoal, 
					       bool justDistance)
{
  MvrPose goal;
  goal.setX(dist * MvrMath::cos(deltaHeading));
  goal.setY(dist * MvrMath::sin(deltaHeading));
  goal = myRobot->getToGlobalTransform().doTransform(goal);
  setGoal(goal, backToGoal, justDistance);
}

MVREXPORT void MvrActionGotoStraight::setEncoderGoal(MvrPose encoderGoal, 
						   bool backToGoal,
						   bool justDistance)
{
  myState = STATE_GOING_TO_GOAL;
  myEncoderGoal = encoderGoal;
  myUseEncoderGoal = true;
  myBacking = backToGoal;
  myDist = myRobot->getEncoderPose().findDistanceTo(encoderGoal);
  myJustDist = justDistance;
  myDistTravelled = 0;
  myLastPose = myRobot->getEncoderPose();
}

MVREXPORT void MvrActionGotoStraight::setEncoderGoalRel(double dist, 
						      double deltaHeading,
						      bool backToGoal,
						      bool justDistance)
{
  MvrPose goal;
  goal.setX(dist * MvrMath::cos(deltaHeading));
  goal.setY(dist * MvrMath::sin(deltaHeading));
  goal = myRobot->getToGlobalTransform().doTransform(goal);
  goal = myRobot->getEncoderTransform().doInvTransform(goal);
  setEncoderGoal(goal, backToGoal, justDistance);
}

MVREXPORT MvrActionDesired *ArActionGotoStraight::fire(MvrActionDesired currentDesired)
{
  double angle;
  double dist;
  double distToGo;
  double vel;

  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_GOAL || myState == STATE_NO_GOAL)
    return NULL;


  MvrPose goal;
  if (!myUseEncoderGoal)
  {
    goal = myGoal;
    myDistTravelled += myRobot->getPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getPose();
  }
  else
  {
    goal = myRobot->getEncoderTransform().doTransform(myEncoderGoal);
    myDistTravelled += myRobot->getEncoderPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getEncoderPose();
  }

  if (myJustDist)
  {
    distToGo = myDist - myDistTravelled;
    dist = fabs(distToGo);
  }
  else
  {
    dist = myRobot->getPose().findDistanceTo(goal);
  }

  if (((myJustDist && distToGo <= 0) || 
       (!myJustDist && dist < myCloseDist))
      && MvrMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Achieved goal");
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;  
  }

  // see where we want to point
  angle = myRobot->getPose().findAngleTo(goal);
  if (myBacking)
    angle = MvrMath::subAngle(angle, 180);
  myDesired.setHeading(angle);
  // if we're close, stop
  if ((myJustDist && distToGo <= 0) || 
      (!myJustDist && dist < myCloseDist))
  {
    myDesired.setVel(0);
    vel = 0;
  }
  else
  {
    vel = sqrt(dist * 200 * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myBacking)
      vel *= -1;
    myDesired.setVel(vel);
  }
  if (myPrinting)
    MvrLog::log(MvrLog::Normal, "dist %.0f angle %.0f vel %.0f", 
	       dist, angle, vel);
  return &myDesired;
}

