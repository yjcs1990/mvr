/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionLimiterTableSensor.h
 > Description  : Action to limit speed (and stop) based on whether the "table"-sensors see anything
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONLIMITERTABLESENSOR_H
#define MVRACTIONLIMITERTABLESENSOR_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionLimiterTableSensor : public MvrAction
{
public:
  ///Constructor
  MVREXPORT MvrActionLimiterTableSensor(const char *name="TableSensorLimiter");
  /// Destructor
  MVREXPORT virtual ~MvrActionLimiterTableSensor();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const
  { return &myDesired; }
#endif  // SWIG
protected:
  MvrActionDesired myDesired;
};

#endif  // MVRACTIONLIMITERTABLESENSOR_H