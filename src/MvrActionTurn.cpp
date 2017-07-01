#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionTurn.h"
#include "MvrRobot.h"

MVREXPORT MvrActionTurn::MvrActionTurn(const char *name, double speedStartTurn,
				    double speedFullTurn, double turnAmount) :
  MvrAction(name, 
	   "Turns the robot depending on actions by higher priority actions.")
{
  setNextArgument(MvrArg("speed start turn", &mySpeedStart,
			"max vel at which to start turning (mm/sec)"));
  mySpeedStart = speedStartTurn;

  setNextArgument(MvrArg("speed full turn", &mySpeedFull,
			"max vel at which to turn the full amount (mm/sec)"));
  mySpeedFull = speedFullTurn;

  setNextArgument(MvrArg("turn amount", &myTurnAmount,
			"max vel at which to start turning (mm/sec)"));
  myTurnAmount = turnAmount;

  myTurning = 0;

}

MVREXPORT MvrActionTurn::~MvrActionTurn()
{

}

MVREXPORT MvrActionDesired *MvrActionTurn::fire(MvrActionDesired currentDesired)
{
  myDesired.reset();
  double turnAmount;
  double angle;

  // if there's no strength, bail
  // if our max velocity is higher than our start turn, bail
  if (myRobot->getVel() > mySpeedStart)
  {
    if (myTurning != 0)
    {
      //printf("Resetting\n");
      myTurning = 0;
    }
    return NULL;
  }

  // we're going to turn now... so figure out the amount to turn

  // if our max vel is lower than the turn amount, just do the full turn
  if (myRobot->getVel() < mySpeedFull)
  {
    //printf("full\n");
    turnAmount = myTurnAmount;
  }
  // otherwise scale it
  else
  {
    turnAmount = ((mySpeedStart - myRobot->getVel()) /
		  (mySpeedStart - mySpeedFull)) * myTurnAmount;
  
    //printf("%.2f\n", (mySpeedStart - currentDesired.getMaxVel()) / (mySpeedStart - mySpeedFull));
  }
  // if we're already turning, turn that direction
  if (myTurning != 0)
    turnAmount *= myTurning;
  // find out which side the closest obstacle is, and turn away
  else
  {
    if (myRobot->checkRangeDevicesCurrentPolar(-90, 90, &angle) < 3000)
    {
      if (angle > 0)
      {
	//printf("### right\n");
	myTurning = -1;
      }
      else
      {
	//printf("### left\n");
	myTurning = 1;
      }
    }
    else
    {
      //printf("### left\n");
      myTurning = 1;
    }
    turnAmount *= myTurning;
  }
  myDesired.setDeltaHeading(turnAmount);
  return &myDesired;
}
