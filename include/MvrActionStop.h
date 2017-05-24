/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionStop.h
 > Description  : Action for stopping the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONSTOP_H
#define MVRACTIONSTOP_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionStop : public MvrAction
{
  /// Constructor
  AREXPORT MvrActionStop(const char *name = "stop");
  /// Destructor
  AREXPORT virtual ~MvrActionStop();
  AREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  AREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif
protected:
  MvrActionDesired myDesired;
};

#endif  // MVRACTIONSTOP_H