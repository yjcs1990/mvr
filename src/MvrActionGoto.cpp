#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionGoto.h"
#include "MvrRobot.h"

MVREXPORT MvrActionGoto::MvrActionGoto(const char *name, MvrPose goal, 
				    double closeDist, double speed,
				    double speedToTurnAt, double turnAmount) :
  MvrAction(name, "Goes to the given goal.")
{
  myDirectionToTurn = myCurTurnDir = 1;
  myTurnedBack = false;
  myPrinting = false;

  setNextArgument(MvrArg("goal", &myGoal, "MvrPose to go to. (MvrPose)"));
  setGoal(goal);
  
  setNextArgument(MvrArg("close dist", &myCloseDist, 
			"Distance that is close enough to goal. (mm)"));
  myCloseDist = closeDist;

  setNextArgument(MvrArg("speed", &mySpeed, 
			"Speed to travel to goal at. (mm/sec)"));
  mySpeed = speed;

  setNextArgument(MvrArg("speed to turn at", &mySpeedToTurnAt,
			"Speed to start obstacle avoiding at (mm/sec)"));
  mySpeedToTurnAt = speedToTurnAt;
  
  setNextArgument(MvrArg("amount to turn", &myTurnAmount,
			"Amount to turn when avoiding (deg)"));
  myTurnAmount = turnAmount;
  
}

MVREXPORT MvrActionGoto::~MvrActionGoto()
{

}

MVREXPORT bool MvrActionGoto::haveAchievedGoal(void)
{
  if (myState == STATE_ACHIEVED_GOAL)
    return true;
  else
    return false;
}

MVREXPORT void MvrActionGoto::cancelGoal(void)
{
  myState = STATE_NO_GOAL;
}

MVREXPORT void MvrActionGoto::setGoal(MvrPose goal)
{
  myState = STATE_GOING_TO_GOAL;
  myGoal = goal;
  myTurnedBack = false;
  myCurTurnDir = myDirectionToTurn;
  myOldGoal = myGoal;
}

MVREXPORT MvrActionDesired *MvrActionGoto::fire(MvrActionDesired currentDesired)
{
  double angle;
  double dist;
  double vel;

  /* If myGoal changed since the last time setGoal() was called (it's an action
   * argument) call setGoal() to reset to the new goal.
   */
  if (myGoal.findDistanceTo(myOldGoal) > 5) 
    setGoal(myGoal);
  
  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_GOAL || myState == STATE_NO_GOAL)
    return NULL;

  dist = myRobot->getPose().findDistanceTo(myGoal);
  if (dist < myCloseDist && MvrMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
      printf("Achieved goal\n");
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;
  }

  if (myPrinting)
    printf("%.0f  ", dist);
  // see where we want to point
  angle = myRobot->getPose().findAngleTo(myGoal);

  if (MvrMath::fabs(MvrMath::subAngle(angle, myRobot->getTh())) > 120)
  {
    myCurTurnDir *= -1;
  }


  // see if somethings in front of us
  if (currentDesired.getMaxVelStrength() > 0 &&
      currentDesired.getMaxVel() < mySpeedToTurnAt)
  {
    if (myPrinting)
      printf("Something slowing us down.  ");
    myDesired.setDeltaHeading(myTurnAmount * myCurTurnDir);
  }
  else
  {
    if (myPrinting)
      printf("Can speed up and turn back again.  ");
    // see if we want to just point at the goal or not
    if (MvrMath::fabs(
	    MvrMath::subAngle(angle, 
			     MvrMath::addAngle(myTurnAmount * 
					      myCurTurnDir * -1, 
					      myRobot->getTh())))
	> myTurnAmount/2)
    {
      if (myPrinting)
	printf("Pointing to goal  ");
      myDesired.setHeading(angle);
    }
    else
    {
      if (myPrinting)
	printf("turning back  ");
      myDesired.setDeltaHeading(myTurnAmount * myCurTurnDir * -1);
    }
  }
  if (dist < myCloseDist && MvrMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
      printf("#achieved\n");
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
  }
  // if we're close, stop
  else if (dist < myCloseDist)
  {
    if (myPrinting)
      printf("#stop\n");
    myDesired.setVel(0);
  }
  else
  {
    vel = sqrt(dist * 200 * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myPrinting)
      printf("#go %.0f\n", vel);
    myDesired.setVel(vel);
  }
  return &myDesired;
}
