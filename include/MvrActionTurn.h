#ifndef MVRACTIONTURN
#define MVRACTIONTURN

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to turn when the behaviors with more priority have limited the speed
/**
   This action is basically made so that you can just have a ton of
   limiters of different kinds and types to keep speed under control,
   then throw this into the mix to have the robot wander.  Note that
   the turn amount ramps up to turnAmount starting at 0 at
   speedStartTurn and hitting the full amount at speedFullTurn.

   @ingroup ActionClasses
**/
class MvrActionTurn : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionTurn(const char *name = "turn",
			double speedStartTurn = 200,
			double speedFullTurn = 100,
			double turnAmount = 15);
  /// Destructor
  MVREXPORT virtual ~MvrActionTurn();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double mySpeedStart;
  double mySpeedFull;
  double myTurnAmount;
  double myTurning;

  MvrActionDesired myDesired;

};

#endif // MVRACTIONTURN
