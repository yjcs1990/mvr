/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionDeceleratingLimiter.h
 > Description  : Action to limit the forwards motion of the robot based on range sensor readings
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONDECELERATINGLIMITER_H
#define MVRACTIONDECELERATINGLIMITER_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionDeceleratingLimiter : public MvrAction
{
public:
  enum LimiterType {
    FORWARDS,       ///< Limit forwards
    BACKWARDS,      ///<Limit backwards
    LATERAL_LEFT,   ///<Limit lateral left
    LATERAL_RIGHT   ///< Limit lateral right
  };
  /// Constructor
  MVREXPORT MvrActionDeceleratingLimiter(const char *name="limitAndDecel",
            LimiterType type=FORWARDS);
  /// Destructor
  MVREXPORT virtual ~MvrActionDeceleratingLimiter();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const
  { return &myDesired; }
#endif  // SWIG
  /// Sets the parameters (don't use this if you're using the addToConfig)
  MVREXPORT void setParameters(double clearance = 100,
			      double sideClearanceAtSlowSpeed = 50,
			      double paddingAtSlowSpeed = 50,
			      double slowSpeed = 200,
			      double sideClearanceAtFastSpeed = 400,
			      double paddingAtFastSpeed = 300,
			      double fastSpeed = 1000,
			      double preferredDecel = 600,
			      bool useEStop = false,
			      double maxEmergencyDecel = 0); 
  /// Gets if this will control us when going forwards
  LimiterType getType(void) { return myType; }
  /// Sets if this will control us when going forwards
  void setType(LimiterType type) { myType = type; }
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section,
			      const char *prefix = NULL);
  /// Sets if we're using locationDependent range devices or not
  bool getUseLocationDependentDevices(void) 
    { return myUseLocationDependentDevices; }
  /// Sets if we're using locationDependent range devices or not
  void setUseLocationDependentDevices(bool useLocationDependentDevices)
    { myUseLocationDependentDevices = useLocationDependentDevices; }
  // sets if we should stop rotation too if this action has stopped the robot
  void setStopRotationToo(bool stopRotationToo)
    { myStopRotationToo = stopRotationToo; }
protected:
  bool myLastStopped;
  LimiterType myType;
  double myClearance;
  double mySideClearanceAtSlowSpeed;
  double myPaddingAtSlowSpeed;
  double mySlowSpeed;
  double mySideClearanceAtFastSpeed;
  double myPaddingAtFastSpeed;
  double myFastSpeed;
  double myPreferredDecel;
  double myMaxEmergencyDecel;
  bool myUseEStop;
  bool myUseLocationDependentDevices;
  bool myStopRotationToo;

  //double myDecelerateDistance;
  MvrActionDesired myDesired;             
};

#endif  // MVRACTIONDECELERATINGLIMITER_H

