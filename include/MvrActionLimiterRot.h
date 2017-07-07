#ifndef MVRACTIONLIMITERROT_H
#define MVRACTIONLIMITERROT_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to limit the forwards motion of the robot based on range sensor readings
/**
   This action uses the robot's range sensors (e.g. sonar, laser) to find a 
   maximum speed at which to travel
   and will increase the deceleration so that the robot doesn't hit
   anything.  If it has to, it will trigger an estop to avoid a
   collision.

   Note that this cranks up the deceleration with a strong strength,
   but it checks to see if there is already something decelerating
   more strongly... so you can put these actions lower in the priority list so
   things will play together nicely.

   @ingroup ActionClasses
**/
class MvrActionLimiterRot : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionLimiterRot(const char *name = "limitRot");
  /// Destructor
  MVREXPORT virtual ~MvrActionLimiterRot();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  /// Sets the parameters (don't use this if you're using the addToConfig)
  MVREXPORT void setParameters(bool checkRadius = false,
			      double inRadiusSpeed = 0);
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section,
			    const char *prefix = NULL);
  /// Sets if we're using locationDependent range devices or not
  bool getUseLocationDependentDevices(void) 
    { return myUseLocationDependentDevices; }
  /// Sets if we're using locationDependent range devices or not
  void setUseLocationDependentDevices(bool useLocationDependentDevices)
    { myUseLocationDependentDevices = useLocationDependentDevices; }
protected:
  bool myCheckRadius;
  double myInRadiusSpeed;
  bool myUseLocationDependentDevices;
  MvrActionDesired myDesired;
};

#endif // ARACTIONSPEEDLIMITER_H
