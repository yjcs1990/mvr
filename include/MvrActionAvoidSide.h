#ifndef MVRACTIONAVOIDSIDE_H
#define MVRACTIONAVOIDSIDE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to avoid impacts by firening into walls at a shallow angle
/**
   This action watches the sensors to see if it is close to firening into a wall
   at a shallow enough angle that other avoidance may not avoid.

  @ingroup ActionClasses
*/
class MvrActionAvoidSide : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionAvoidSide(const char *name = "Avoid side", 
		    double obstacleDistance = 300,
		    double turnAmount = 5);
  /// Destructor
  MVREXPORT virtual ~MvrActionAvoidSide();
  MVREXPORT virtual MvrActionDesired * fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double myObsDist;
  double myTurnAmount;
  bool myTurning;
  MvrActionDesired myDesired;

};

#endif // MVRACTIONAVOIDSIDE_H
