#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionLimiterForwards.h"
#include "MvrRobot.h"
#include "MvrRangeDevice.h"

/**
   @param name name of the action
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param slowSpeed speed allowed at slowDistance, scales to 0 at slow 
   distance (mm/sec)
   @param widthRatio Ratio of the width of the box to look at to the robot radius (multiplier)
*/
MVREXPORT MvrActionLimiterForwards::MvrActionLimiterForwards(const char *name, 
							  double stopDistance,
							  double slowDistance,
							  double slowSpeed,
							  double widthRatio) :
  MvrAction(name,
	   "Slows the robot down so as not to hit anything in front of it.")
{
  setNextArgument(MvrArg("stop distance", &myStopDist, 
			"Distance at which to stop. (mm)"));
  myStopDist = stopDistance;

  setNextArgument(MvrArg("slow distance", &mySlowDist, 
			"Distance at which to slow down. (mm)"));
  mySlowDist = slowDistance;

  setNextArgument(MvrArg("slow speed", &mySlowSpeed, 
			 "Speed at which to slow to at the slow distance, (mm/sec)"));
  mySlowSpeed = slowSpeed;
  
  setNextArgument(MvrArg("width ratio", &myWidthRatio,
			"Ratio of the width of the box to look at to the robot radius (multiplier)"));
  myWidthRatio = widthRatio;
  myLastStopped = false;
  myLastSensorReadingDev = NULL;
}

MVREXPORT MvrActionLimiterForwards::~MvrActionLimiterForwards()
{

}

/**
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param slowSpeed speed allowed at slowDistance, scales to 0 at slow 
   distance (mm/sec)
   @param widthRatio Ratio of the width of the box to look at to the robot radius (multiplier)
*/
MVREXPORT void MvrActionLimiterForwards::setParameters(double stopDistance,
						     double slowDistance,
						     double slowSpeed,
						     double widthRatio)
{
  myStopDist = stopDistance;
  mySlowDist = slowDistance;
  mySlowSpeed = slowSpeed;
  myWidthRatio = widthRatio;
}

MVREXPORT MvrActionDesired *
MvrActionLimiterForwards::fire(MvrActionDesired currentDesired)
{
  double dist;
  double maxVel;
  bool printing = false;
  double checkDist;

  if (myStopDist > mySlowDist)
    checkDist = myStopDist;
  else
    checkDist = mySlowDist;

  myDesired.reset();
  dist = myRobot->checkRangeDevicesCurrentBox(
        0,
				-myRobot->getRobotWidth()/2.0 * myWidthRatio,
  			checkDist + myRobot->getRobotLength()/2,
				myRobot->getRobotWidth()/2.0 * myWidthRatio,
        &myLastSensorReadingPos,
        &myLastSensorReadingDev
  );
  dist -= myRobot->getRobotLength() / 2;
  //printf("%.0f\n", dist);

  if (dist < myStopDist)
  {
    if(!myLastStopped)
    {
      if (printing) printf("Stopping\n");
      MvrLog::log(MvrLog::Verbose, "%s: Stopping due to sensor reaoding", getName());
    }
    myLastStopped = true;
    myDesired.setMaxVel(0);
    return &myDesired;
  }

  if(myLastStopped)
  {
    if (printing) printf("Going\n");
    MvrLog::log(MvrLog::Verbose, "%s: Allowing motion", getName());
  } 

  myLastStopped = false;
  //printf("%f ", dist);
  if (dist > mySlowDist)
  {
    //printf("Nothing\n");
    return NULL;
    //return &myDesired;
  }
      
			
  maxVel = mySlowSpeed * ((dist - myStopDist) / (mySlowDist - myStopDist));
  //printf("Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f\n", maxVel,	 myStopDist, mySlowDist, mySlowSpeed);
  myDesired.setMaxVel(maxVel);
  return &myDesired;
  
}
