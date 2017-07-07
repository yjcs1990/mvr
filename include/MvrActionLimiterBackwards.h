#ifndef MVRACTIONBACKWARDSSPEEDLIMITER_H
#define MVRACTIONBACKWARDSSPEEDLIMITER_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to limit the backwards motion of the robot based on range sensor readings
/**
   This class limits the backwards motion of the robot according to range sensor
   readings (e.g. sonar, laser), and the parameters given. When the range
   sensor (e.g. sonar or laser) detects rearward obstacles closer than the given parameters,
   this action requests that the robot decelerate or stop any current backwards movement.
   @ingroup ActionClasses
*/
class MvrActionLimiterBackwards : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionLimiterBackwards(const char *name = "speed limiter", 
				    double stopDistance = -250,
				    double slowDistance = -600,
				    double maxBackwardsSpeed = -250,
				    double widthRatio = 1.5,
				    bool avoidLocationDependentObstacles = true);
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

#endif // ARACTIONBACKWARDSSPEEDLIMITER_H

