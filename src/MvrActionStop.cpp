#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionStop.h"
#include "MvrRobot.h"

/**
   @param name name of the action
*/
MVREXPORT MvrActionStop::MvrActionStop(const char *name) :
    MvrAction(name, "Stops the robot")
{
}

MVREXPORT MvrActionStop::~MvrActionStop()
{
}

MVREXPORT MvrActionDesired *MvrActionStop::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  myDesired.setVel(0);
  myDesired.setDeltaHeading(0);
  if (myRobot->hasLatVel())
    myDesired.setLeftLatVel(0);

  return &myDesired;
}
