#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionInput.h"
#include "MvrRobot.h"
/**
   @param name name of the action
*/
MVREXPORT MvrActionInput::MvrActionInput(const char *name) :
    MvrAction(name, "Inputs vel and heading")
{
  clear();
}

MVREXPORT MvrActionInput::~MvrActionInput()
{
}

MVREXPORT void MvrActionInput::setVel(double vel)
{
  myUsingVel = true;
  myVelSet = vel;
}

MVREXPORT void MvrActionInput::setRotVel(double rotVel)
{
  myRotRegime = ROTVEL;
  myRotVal = rotVel;
}

MVREXPORT void MvrActionInput::deltaHeadingFromCurrent(double delta)
{
  myRotRegime = DELTAHEADING;
  myRotVal = delta;
}

MVREXPORT void MvrActionInput::setHeading(double heading)
{
  myRotRegime = SETHEADING;
  myRotVal = heading;
}

MVREXPORT void MvrActionInput::clear(void)
{
  myUsingVel = false;
  myRotRegime = NONE;
}

MVREXPORT MvrActionDesired *MvrActionInput::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  if (myUsingVel)
    myDesired.setVel(myVelSet);
  
  if (myRotRegime == ROTVEL)
    myDesired.setRotVel(myRotVal);
  else if (myRotRegime == DELTAHEADING)
  {
    myDesired.setDeltaHeading(myRotVal);
    myRotVal = 0;
  }
  else if (myRotRegime == SETHEADING)
    myDesired.setHeading(myRotVal);
  else if (myRotRegime != NONE)
    MvrLog::log(MvrLog::Normal, "MvrActionInput::fire: Bad rot regime %d", 
	       myRotRegime);

  return &myDesired;
}
