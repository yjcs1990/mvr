#ifndef MVRACTIONLIMITERTABLESNSOR_H
#define MVRACTIONLIMITERTABLESNSOR_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to limit speed (and stop) based on whether the "table"-sensors see anything
/**
   This action limits speed to 0 if the table-sensors see anything in front
   of the robot.  The action will only work if the robot has table sensors,
   meaning that the robots parameter file has them listed as true.

   @ingroup ActionClasses
*/
class MvrActionLimiterTableSensor : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionLimiterTableSensor(const char *name = "TableSensorLimiter");
  /// Destructor
  MVREXPORT virtual ~MvrActionLimiterTableSensor();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  MvrActionDesired myDesired;
};



#endif // MVRACTIONLIMITERTABLESNSOR_H
