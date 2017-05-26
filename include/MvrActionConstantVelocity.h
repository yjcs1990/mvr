/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionConstantVelocity.h
 > Description  : Action for going straight at a constant velocity
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONCONSTANCTVELOCITY_H
#define MVRACTIONCONSTANCTVELOCITY_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

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

#endif  // MVRACTIONCONSTANCTVELOCITY_H