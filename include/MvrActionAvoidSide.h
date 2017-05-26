/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionAvoidSide.h
 > Description  : Action to avoid impacts by firing into walls at a shallow angle
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONAVOIDFRONT_H
#define MVRACTIONAVOIDFRONT_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionAvoidSide : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionAvoidSide(const char *name = "avoid side", 
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

#endif  // MVRACTIONAVOIDFRONT_H