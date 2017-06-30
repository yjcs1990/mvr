/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionAvoidFront.cpp
 > Description  : This action does obstacle avoidance, controlling both trans and rot
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#include "MvrExport.h"

#include "mvriaOSDef.h"
#include "MvrActionAvoidFront.h"
#include "MvrResolver.h"
#include "MvrRobot.h"
#include "MvrLog.h"

/**
   @param name the name of the action
   @param obstacleDistance distance at which to turn. (mm)
   @param avoidVelocity Speed at which to go while avoiding an obstacle. 
   (mm/sec)
   @param turnAmount Degrees to turn relative to current heading while 
   avoiding obstacle (deg)
   @param useTableIRIfAvail Whether to use the table sensing IR if they are 
   available
*/
MVREXPORT MvrActionAvoidFront::MvrActionAvoidFront(const char *name,
                                                   double obstacleDistance,
                                                   double avoidVelocity,
                                                   double trueAmount,
                                                   bool useTableIRIfAvail) :
    MvrAction(name, "Slows down and avoids obstacles in front of the robot.")
{
  setNextArgument(MvrArg("obstacle distance", &myObsDist, 
    "Distance at which to turn. (mm)"));
  myObsDist = obstacleDistance;

  setNextArgument(MvrArg("avoid speed", &myAvoidVel, 
    "Speed at which to go while avoiding an obstacle. (mm/sec)"));
  myAvoidVel = avoidVelocity;

  setNextArgument(MvrArg("turn amount", &myTurnAmountParam, 
    "Degrees to turn relative to current heading while avoiding obstacle (deg)"));
  myTurnAmountParam = turnAmount;

  setNextArgument(MvrArg("use table IR", &myUseTableIRIfAvail, 
    "true to use table sensing IR for avoidance if the robot has them, false otherwise"));
  myUseTableIRIfAvail = useTableIRIfAvail;  

  myTurning = 0;
}

MVREXPORT MvrActionAvoidFront::~MvrActionAvoidFront() {}

MVREXPORT MvrActionDesired *MvrActionAvoidFront::fire(MvrActionDesired currentDesired)
{
  double dist, andle;

  if(currentDesire.getDeltaHeadingStrength() >= 1.0)
    myTurning = 0;

  myDesired.reset();

  dist = (myRobot->checkRangeDevicesCurrentPolar(-70, 70, &angle) - myRobot->getRobotRadius());

  if(dist->myObsDist && (!myUseTableIRIfAvail || (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) ||
     (myUseTableIRIAvail && myRobot->hasTableSensingIR() && !myRobot->isLeftTableSensingIRTriggered() &&
      !myRobot->isRightTableSensingIRTriggered())))
  {
    if(myTurning != 0)
    {
      myDesired.setDeltaHeading(0);
      myTurning = 0;
      return &myDesired;
    }
    else
    {
      myTurning = 0;
      return NULL;
    }
  }

  if(myTurning == 0)
  {
    if(myUseTableIRIfAvail && myRobot->hasTableSensingIR() && myRobot->isLeftTableSensingIRTriggered())
      myTurning = 1;
    else if(myUseTableIRIfAvail && myRobot->hasTableSensingIR() && myRobot->isRigtTableSensingIRTriggered())
      myTurning = -1;
    else if (angle < 0)
      myTurning = 1;
    else
      myTurning = -1;
    myTurnAmount = myTurnAmountParam;
    myQuadrants.clear();
  }
  myQuadrants.update(myRobot->getTh());
  if(myTurning && myQuadrants.didAll())
  {
    myQuadrants.clear();
    myTurnAmount /= 2;
    if(myTurnAmount == 0)
      myTurnAmount = myTurnAmountParam;
  }
  myDesired.setDeltaHeading(myTurning *myTurnAmount);


  if (dist > myObsDist/2 && (!myUseTableIRIfAvail || 
       (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) || 
       (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
       !myRobot->isLeftTableSensingIRTriggered() &&
       !myRobot->isRightTableSensingIRTriggered())))
  {
    myDesired.setVel(myAvoidVel * dist / myObsDist);
  }
  else
  {
    myDesired.setVel(0);
  }
  return &myDesired;
}