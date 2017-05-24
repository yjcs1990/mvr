/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionAvoidFront.h
 > Description  : This action does obstacle avoidance, controlling both trans and rot
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONAVOIDFRONT_H
#define MVRACTIONAVOIDFRONT_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"
#include "MvrAction.h"

class MvrActionAvoidFront : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionAvoidFront(const char *name = "avoid front obstacles", 
		     double obstacleDistance = 450, double avoidVelocity = 200,
		     double turnAmount = 15, bool useTableIRIfAvail = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionAvoidFront();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double myTurnAmount;
  double myObsDist;
  double myAvoidVel;
  double myTurnAmountParam;
  bool myUseTableIRIfAvail;
  int myTurning; // 1 for turning left, 0 for not turning, -1 for turning right
  MvrActionDesired myDesired;
  MvrSectors myQuadrants;
  MvrFunctorC<MvrActionAvoidFront> myConnectCB;
};

#endif  // MVRACTIONAVOIDFRONT_H