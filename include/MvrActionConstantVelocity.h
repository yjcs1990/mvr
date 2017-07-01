#ifndef MVRACTIONCONSTANTVELOCITY_H
#define MVRACTIONCONSTANTVELOCITY_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action for going straight at a constant velocity
/**
   This action simply goes straight at a constant velocity.
  @ingroup ActionClasses
*/
class MvrActionConstantVelocity : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionConstantVelocity(const char *name = "Constant Velocity", 
			   double velocity = 400);
  /// Destructor
  MVREXPORT virtual ~MvrActionConstantVelocity();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double myVelocity;
  MvrActionDesired myDesired;
};

#endif // MVRACTIONCONSTANTVELOCITY_H
