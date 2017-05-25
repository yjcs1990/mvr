/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionLimterRot.h
 > Description  : Action to limit the forwards motion of the robot based on range sensor readings
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONLIMITERROT_H
#define MVRACTIONLIMITERROT_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/**
   This action uses the robot's range sensors (e.g. sonar, laser) to find a 
   maximum speed at which to travel
   and will increase the deceleration so that the robot doesn't hit
   anything.  If it has to, it will trigger an estop to avoid a
   collision.
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
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const { return &myDesired; }
#endif
  /// Sets the parameters (don't use this if you're using the addToConfig)
  MVREXPORT void setParameters(bool checkRadius = false, double inRadiusSpeed = 0);
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section, const char *prefix = NULL);
  /// Sets if we're using locationDependent range devices or not
  bool getUseLocationDependentDevices(void) { return myUseLocationDependentDevices; }
  /// Sets if we're using locationDependent range devices or not
  void setUseLocationDependentDevices(bool useLocationDependentDevices)
  { myUseLocationDependentDevices = useLocationDependentDevices; }
protected:
  bool myCheckRadius;
  double myInRadiusSpeed;
  bool myUseLocationDependentDevices;
  MvrActionDesired myDesired;
};

#endif // MVRACTIONLIMITERROT_H
