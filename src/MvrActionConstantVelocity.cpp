#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionConstantVelocity.h"

/**
   @param name name of the action
   @param velocity velocity to travel at (mm/sec)
*/
AREXPORT MvrActionConstantVelocity::MvrActionConstantVelocity(const char *name,
						   double velocity) :
  MvrAction(name, "Sets the robot to travel straight at a constant velocity.")
{
  setNextMvrgument(MvrArg("velocity", &myVelocity, 
			"The velocity to make the robot travel at. (mm/sec)"));
  myVelocity = velocity;  
}

AREXPORT MvrActionConstantVelocity::~MvrActionConstantVelocity()
{

}

AREXPORT MvrActionDesired *MvrActionConstantVelocity::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  myDesired.setVel(myVelocity);
  myDesired.setDeltaHeading(0);

  return &myDesired;
}
