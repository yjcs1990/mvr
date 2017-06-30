/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionTurn.h
 > Description  : Action to turn when the behaviors with more priority have limited the speed
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONTURN_H
#define MVRACTIONTURN_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

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

#endif  // MVRACTIONTURN_H