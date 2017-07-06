#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionLimiterBackwards.h"
#include "MvrRobot.h"

/**
   @param name name of the action
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param maxBackwardsSpeed maximum backwards speed, speed allowed scales
     from this to 0 at the stop distance (mm/sec)
   @param widthRatio The ratio of robot width to the width of the region this action checks for sensor readings. 
   @param avoidLocationDependentObstacles If true, stop as the robot nears location-dependent sensed obstacles, if false, ignore them.
*/
MVREXPORT MvrActionLimiterBackwards::MvrActionLimiterBackwards(
	const char *name, double stopDistance, double slowDistance, 
	double maxBackwardsSpeed, double widthRatio, 
	bool avoidLocationDependentObstacles) :
  MvrAction(name,
	   "Slows the robot down so as not to hit anything behind it.")
{
  setNextArgument(MvrArg("stop distance", &myStopDist, 
			"Distance at which to stop. (mm)"));
  myStopDist = stopDistance;

  setNextArgument(MvrArg("slow distance", &mySlowDist, 
			"Distance at which to slow down. (mm)"));
  mySlowDist = slowDistance;

  setNextArgument(MvrArg("maximum backwards speed", &myMaxBackwardsSpeed, 
			 "Maximum backwards speed, scales from this to 0 at stopDistance (-mm/sec)"));
  myMaxBackwardsSpeed = maxBackwardsSpeed;

  setNextArgument(MvrArg("width ratio", &myWidthRatio, 
			 "The ratio of robot width to how wide an area to check (ratio)"));
  myWidthRatio = widthRatio;

  setNextArgument(MvrArg("avoid location dependent obstacles", 
			&myAvoidLocationDependentObstacles, 
			 "Whether to avoid location dependent obstacles or not"));
  myAvoidLocationDependentObstacles = avoidLocationDependentObstacles;
}

MVREXPORT MvrActionLimiterBackwards::~MvrActionLimiterBackwards()
{

}

MVREXPORT MvrActionDesired *
MvrActionLimiterBackwards::fire(MvrActionDesired currentDesired)
{
  double dist;
  double maxVel;
  
  double slowStopDist = MvrUtil::findMax(myStopDist, mySlowDist);
  

  myDesired.reset();
  dist = myRobot->checkRangeDevicesCurrentBox(
	  // changing this to 0 since right now it won't stop on
	  //bumper hits since they are inside the robot...
	  //-myRobot->getRobotLength()/2,
	  0,
	  -(myRobot->getRobotWidth()/2.0 * myWidthRatio),
	  slowStopDist + (-myRobot->getRobotLength()),
	  (myRobot->getRobotWidth()/2.0 * myWidthRatio),
	  NULL,
	  NULL,
	  myAvoidLocationDependentObstacles);
  dist -= myRobot->getRobotRadius();
  if (dist < -myStopDist)
  {
    //printf("backwards stop\n");
    myDesired.setMaxNegVel(0);
    return &myDesired;
  }
  if (dist > -mySlowDist)
  {
    //printf("backwards nothing\n");
    myDesired.setMaxNegVel(-MvrMath::fabs(myMaxBackwardsSpeed));
    return &myDesired;
  }
      
			
  maxVel = -MvrMath::fabs(myMaxBackwardsSpeed) * ((-dist - myStopDist) / (mySlowDist - myStopDist));
  //printf("Neg Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f\n", maxVel,	 myStopDist, mySlowDist, myMaxBackwardsSpeed);
  myDesired.setMaxNegVel(maxVel);
  return &myDesired;
  
}
