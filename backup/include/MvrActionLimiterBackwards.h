/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionLimiterBackwards.h
 > Description  : Action to limit the Backwards motion of the robot based on range sensor readings.
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONLIMITERBACKWARDS_H
#define MVRACTIONLIMITERBACKWARDS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrRangeDevice;

class MvrActionLimiterBackwards : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionLimiterBackwards(const char *name = "speed limiter", 
				   double stopDistance = -250,
				   double slowDistance = -1000,
				   double slowSpeed = -250,
				   double widthRatio = 1.5,
           bool avoidLocationDependentObstacles=true);
  /// Destructor
  MVREXPORT virtual ~MvrActionLimiterBackwards();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif

protected:
  double myStopDist;
  double mySlowDist;
  double myMaxBackwardsSpeed;
  double myWidthRatio;
  bool myAvoidLocationDependentObstacles;
  MvrActionDesired myDesired;
};

#endif  // MVRACTIONLIMITERBACKWARDS_H