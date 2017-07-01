#ifndef MVRACTIONSPEEDLIMITER_H
#define MVRACTIONSPEEDLIMITER_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrRangeDevice;

/// Action to limit the forwards motion of the robot based on range sensor readings.
/**
   This action uses the sensors to find a maximum forwared speed to travel at; when the range
   sensor (e.g. sonar or laser) detects obstacles closer than the given parameters,
   this action requests that the robot decelerate or stop.
   @ingroup ActionClasses
*/
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

#endif // MVRACTIONSPEEDLIMITER_H
