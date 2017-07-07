#ifndef MVRACTIONMOVEMENTPARAMTERSDEBUGGING_H
#define MVRACTIONMOVEMENTPARAMTERSDEBUGGING_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrMapObject.h"

/// This is a class for setting max velocities and accels and decels via MvrConfig parameters (see addToConfig());
/**
   @ingroup ActionClasses
 **/
class MvrActionMovementParametersDebugging : public MvrAction
{
public: 
  /// Constructor
  MVREXPORT MvrActionMovementParametersDebugging(const char *name = "MovementParametersDebugging");
  /// Destructor
  MVREXPORT virtual ~MvrActionMovementParametersDebugging();
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
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section,
			    const char *prefix = NULL);
protected:
  bool myEnabled;
  bool myEnableOnce;

  bool mySetMaxVel;
  double myMaxVel;
  bool mySetMaxNegVel;
  double myMaxNegVel;
  bool mySetTransAccel;
  double myTransAccel;
  bool mySetTransDecel;
  double myTransDecel;
  bool mySetMaxRotVel;
  double myMaxRotVel;
  bool mySetRotAccel;
  double myRotAccel;
  bool mySetRotDecel;
  double myRotDecel;
  bool mySetMaxLeftLatVel;
  double myMaxLeftLatVel;
  bool mySetMaxRightLatVel;
  double myMaxRightLatVel;
  bool mySetLatAccel;
  double myLatAccel;
  bool mySetLatDecel;
  double myLatDecel;
  
  MvrActionDesired myDesired;


};

#endif // ARACTIONMOVEMENTPARAMTERS_H
