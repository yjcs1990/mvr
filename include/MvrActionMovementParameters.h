/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionMovementParameters.h
 > Description  : This is a class for setting max velocities and accels and decels via MvrConfig parameters 
                  (see addToConfig()) or manually (using setParameters())
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef ARACTIONMOVEMENTPARAMTERS_H
#define ARACTIONMOVEMENTPARAMTERS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrMapObject.h"

class MvrActionMovementParameters : public MvrAction
{
public: 
  /// Constructor
  MVREXPORT MvrActionMovementParameters(const char *name = "MovementParameters",
                                        bool overrideFaster = true, 
                                        bool addLatVelIfAvailable = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionMovementParameters();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif
  /// Sees if this action is enabled (separate from activating it)
  MVREXPORT bool isEnabled(void) { return myEnabled; }
  /// Enables this action (separate from activating it)
  MVREXPORT void enable(void) { myEnabled = true; }
  /// Enables this action in a way that'll work from the sector callbacks
  MVREXPORT void enableOnceFromSector(MvrMapObject *mapObject) 
    { myEnableOnce = true; }
  /// Disables this action (separate from deactivating it)
  MVREXPORT void disable(void) { myEnabled = false; }
  /// Sets the parameters (don't use this if you're using the addToConfig)
  MVREXPORT void setParameters(double maxVel = 0, double maxNegVel = 0,
                               double transAccel = 0, double transDecel = 0,
                               double rotVelMax = 0, double rotAccel = 0,
                               double rotDecel = 0, double latVelMax = 0, 
                               double latAccel = 0, double latDecel = 0);
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section, const char *prefix = NULL);
protected:
  bool myEnabled;
  bool myEnableOnce;
  bool myOverrideFaster;
  bool myAddLatVelIfAvailable;

  double myMaxVel;
  double myMaxNegVel;
  double myTransAccel;
  double myTransDecel;
  double myMaxRotVel;
  double myRotAccel;
  double myRotDecel;
  double myMaxLatVel;
  double myLatAccel;
  double myLatDecel;
  
  MvrActionDesired myDesired;
};

#endif // ARACTIONMOVEMENTPARAMTERS_H
