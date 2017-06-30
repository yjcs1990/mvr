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
AREXPORT MvrActionAvoidFront::MvrActionAvoidFront(const char *name, 
						double obstacleDistance,
						double avoidVelocity,
						double turnAmount, 
						bool useTableIRIfAvail) :
  MvrAction(name, "Slows down and avoids obstacles in front of the robot.")
{
  setNextArgument(MvrArg("obstacle distance", &myObsDist, 
			"Distance at which to turn. (mm)"));
  myObsDist = obstacleDistance;
  
  setNextArgument(MvrArg("avoid speed", &myAvoidVel,
	"Speed at which to go while avoiding an obstacle. (mm/sec)"));
  myAvoidVel = avoidVelocity;

  setNextArgument(MvrArg("turn ammount", &myTurnAmountParam, 
	"Degrees to turn relative to current heading while avoiding obstacle (deg)"));
  myTurnAmountParam = turnAmount;

  setNextArgument(MvrArg("use table IR", &myUseTableIRIfAvail,
		"true to use table sensing IR for avoidance if the robot has them, false otherwise"));
  myUseTableIRIfAvail = useTableIRIfAvail;

  myTurning = 0;
}

AREXPORT MvrActionAvoidFront::~MvrActionAvoidFront()
{

}

AREXPORT MvrActionDesired *MvrActionAvoidFront::fire(MvrActionDesired currentDesired)
{
  double dist, angle;

  if (currentDesired.getDeltaHeadingStrength() >= 1.0)
    myTurning = 0;

  myDesired.reset();

  dist = (myRobot->checkRangeDevicesCurrentPolar(-70, 70, &angle) 
	  - myRobot->getRobotRadius());
  
  //  printf("%5.0f %3.0f ", dist, angle);

  if (dist > myObsDist && 
      (!myUseTableIRIfAvail || 
       (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) || 
       (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
       !myRobot->isLeftTableSensingIRTriggered() &&
       !myRobot->isRightTableSensingIRTriggered())))
  {
    if (myTurning != 0)
    {
      myDesired.setDeltaHeading(0);
      myTurning = 0;
      return &myDesired;
    }
    else
    {
      //printf("\n");
      myTurning = 0;
      return NULL;
    }
  }
  
//  printf("Avoiding ");
  
  if (myTurning == 0)
  {
    if (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
        myRobot->isLeftTableSensingIRTriggered())
      myTurning = 1;
    else if (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
             myRobot->isRightTableSensingIRTriggered())
      myTurning = -1;
    else if (angle < 0)
      myTurning = 1;
    else
      myTurning = -1;
    myTurnAmount = myTurnAmountParam;
    myQuadrants.clear();
  }

  myQuadrants.update(myRobot->getTh());
  if (myTurning && myQuadrants.didAll())
  {
    myQuadrants.clear();
    myTurnAmount /= 2;
    if (myTurnAmount == 0)
      myTurnAmount = myTurnAmountParam;
  }

  myDesired.setDeltaHeading(myTurning * myTurnAmount);

  if (dist > myObsDist/2 && 
      (!myUseTableIRIfAvail || 
       (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) || 
       (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
       !myRobot->isLeftTableSensingIRTriggered() &&
       !myRobot->isRightTableSensingIRTriggered())))
  {
    //printf(" scaling %f %f %f ", myAvoidVel * dist / myObsDist, 
    //dist, myObsDist);
    myDesired.setVel(myAvoidVel * dist / myObsDist);
  }
  else
  {
//   printf(" zerovel\n");
    myDesired.setVel(0);
  }
    
  //printf("\n");
  return &myDesired;
}
