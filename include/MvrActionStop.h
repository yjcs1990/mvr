#ifndef MVRACTIONSTOP_H
#define MVRACTIONSTOP_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action for stopping the robot
/**
   This action simply sets the robot to a 0 velocity and a deltaHeading of 0.
   @ingroup ActionClasses
*/
class MvrActionStop : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionStop(const char *name = "stop");
  /// Destructor
  MVREXPORT virtual ~MvrActionStop();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  MvrActionDesired myDesired;
};

#endif // ARACTIONSTOP_H
