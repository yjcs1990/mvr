#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionAvoidSide.h"
#include "MvrRobot.h"

/**
   @param name name of the action
   @param obstacleDistance distance at which to start avoiding (mm)
   @param turnAmount degrees at which to turn (deg)
*/

MVREXPORT MvrActionAvoidSide::MvrActionAvoidSide(const char *name,
					      double obstacleDistance,
					      double turnAmount) :
  MvrAction(name, "Avoids side obstacles, ie walls")
{
  setNextArgument(MvrArg("obstacle distance", &myObsDist, 
			"Distance at which to start avoiding (mm)"));
  myObsDist = obstacleDistance;
  setNextArgument(MvrArg("turn amount", &myTurnAmount,
			"Degrees at which to turn (deg)"));
  myTurnAmount = turnAmount;

  myTurning = false;

}

MVREXPORT MvrActionAvoidSide::~MvrActionAvoidSide()
{

}

MVREXPORT MvrActionDesired *MvrActionAvoidSide::fire(
	MvrActionDesired currentDesired)
{
  double leftDist, rightDist;

  leftDist = (myRobot->checkRangeDevicesCurrentPolar(60, 120) - 
	      myRobot->getRobotRadius());
  rightDist = (myRobot->checkRangeDevicesCurrentPolar(-120, -60) - 
	      myRobot->getRobotRadius());
  
  myDesired.reset();
  if (leftDist < myObsDist)
  {
    myDesired.setDeltaHeading(-myTurnAmount);
    myDesired.setVel(0);
    myTurning = true;
  }
  else if (rightDist < myObsDist)
  {
    myDesired.setDeltaHeading(myTurnAmount);
    myDesired.setVel(0);
    myTurning = true;
  }
  else if (myTurning)
    myDesired.setDeltaHeading(0);

  return &myDesired;
}
