/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionLimiterForwards.h
 > Description  : Action to limit the forwards motion of the robot based on range sensor readings.
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONLIMITERFORWARDS_H
#define MVRACTIONLIMITERFORWARDS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrRangeDevice;

class MvrActionLimiterForwards : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionLimiterForwards(const char *name = "speed limiter", 
				   double stopDistance = 250,
				   double slowDistance = 1000,
				   double slowSpeed = 200,
				   double widthRatio = 1);
  /// Destructor
  MVREXPORT virtual ~MvrActionLimiterForwards();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif
  MVREXPORT void setParameters(double stopDistance = 250,
			      double slowDistance = 1000,
			      double slowSpeed = 200,
			      double widthRatio = 1);

  bool getStopped() const { return myLastStopped; } 
  MvrPose getLastSensorReadingPos() const { return myLastSensorReadingPos; } 
  const MvrRangeDevice* getLastSensorReadingDevice() const { return myLastSensorReadingDev; } 
protected:
  bool myLastStopped;
  double myStopDist;
  double mySlowDist;
  double mySlowSpeed;
  double myWidthRatio;
  MvrActionDesired myDesired;
  MvrPose myLastSensorReadingPos;
  const MvrRangeDevice *myLastSensorReadingDev;
};

#endif  // MVRACTIONLIMITERFORWARDS_H