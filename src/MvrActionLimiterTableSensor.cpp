#include "MvrExport.h"

#include "mvriaOSDef.h"
#include "MvrActionLimiterTableSensor.h"
#include "MvrRobot.h"

MVREXPORT MvrActionLimiterTableSensor::MvrActionLimiterTableSensor(
	const char *name) :
  MvrAction(name, "Limits speed to 0 if a table is seen")
{

}

MVREXPORT MvrActionLimiterTableSensor::~MvrActionLimiterTableSensor()
{
}

MVREXPORT MvrActionDesired *MvrActionLimiterTableSensor::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  if (myRobot->hasTableSensingIR() && 
      ((myRobot->isLeftTableSensingIRTriggered()) ||
      (myRobot->isRightTableSensingIRTriggered())))
  {
    myDesired.setMaxVel(0);
    return &myDesired;
  }
  return NULL;  
}
