#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionDriveDistance.h"
#include "MvrRobot.h"

MVREXPORT MvrActionDriveDistance::MvrActionDriveDistance(const char *name,
						      double speed,
						      double deceleration) :
  MvrAction(name, "Drives a given distance.")
{
  myPrinting = false;

  setNextArgument(MvrArg("speed", &mySpeed, 
			"Speed to travel to at. (mm/sec)"));
  setNextArgument(MvrArg("deceleration", &myDeceleration, 
			"Speed to decelerate at. (mm/sec/sec)"));
  mySpeed = speed;
  myDeceleration = deceleration;
  myState = STATE_NO_DISTANCE;
}

MVREXPORT MvrActionDriveDistance::~MvrActionDriveDistance()
{

}

MVREXPORT bool MvrActionDriveDistance::haveAchievedDistance(void)
{
  if (myState == STATE_ACHIEVED_DISTANCE)
    return true;
  else
    return false;
}

MVREXPORT void MvrActionDriveDistance::cancelDistance(void)
{
  myState = STATE_NO_DISTANCE;
}

MVREXPORT void MvrActionDriveDistance::setDistance(
	double distance, bool useEncoders)
{
  myState = STATE_GOING_DISTANCE;
  myDistance = distance;
  myUseEncoders = useEncoders;
  if (myUseEncoders)
    myLastPose = myRobot->getEncoderPose();
  else
    myLastPose = myRobot->getPose();
  myDistTravelled = 0;
  myLastVel = 0;
}


MVREXPORT MvrActionDesired *MvrActionDriveDistance::fire(
	MvrActionDesired currentDesired)
{
  double distToGo;
  double vel;

  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_DISTANCE || myState == STATE_NO_DISTANCE)
    return NULL;


  if (myUseEncoders)
  {
    myDistTravelled += myRobot->getEncoderPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getEncoderPose();
  }
  else
  {
    myDistTravelled += myRobot->getPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getPose();
  }

  if (myDistance >= 0)
    distToGo = myDistance - myDistTravelled;
  else
    distToGo = -myDistance - myDistTravelled;

  if (distToGo <= 0 && MvrMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
    {
      MvrLog::log(MvrLog::Normal, 
		 "Achieved distToGo %.0f realVel %.0f realVelDelta %.0f", 
		 distToGo, myRobot->getVel(), myRobot->getVel() - myLastVel);
    }
    myState = STATE_ACHIEVED_DISTANCE;
    myDesired.setVel(0);
    myDesired.setRotVel(0);
    return &myDesired;  
  }

  myDesired.setRotVel(0);
  // if we're close, stop
  if (distToGo <= 0)
  {
    myDesired.setVel(0);
    vel = 0;
  }
  else
  {
    vel = sqrt(distToGo * myDeceleration * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myDistance < 0)
      vel *= -1;
    myDesired.setVel(vel);
  }
  if (myPrinting)
    MvrLog::log(MvrLog::Normal, 
	       "distToGo %.0f cmdVel %.0f realVel %.0f realVelDelta %.0f", 
	       distToGo, vel, myRobot->getVel(), 
	       myRobot->getVel() - myLastVel);
  myLastVel = myRobot->getVel();
  return &myDesired;
}

