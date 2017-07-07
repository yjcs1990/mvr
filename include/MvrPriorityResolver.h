#ifndef MVRPRIORITYRESOLVER_H
#define MVRPRIORITYRESOLVER_H

#include "MvrResolver.h"

/// (Default resolver), takes the action list and uses the priority to resolve
/** 
    This is the default resolver for MvrRobot, meaning if you don't do a 
    non-normal init on the robot, or a setResolver, you'll have one these.
*/
class MvrPriorityResolver : public MvrResolver
{
public:
  /// Constructor
  MVREXPORT MvrPriorityResolver();
  /// Destructor
  MVREXPORT virtual ~MvrPriorityResolver();
  MVREXPORT virtual MvrActionDesired *resolve(MvrResolver::ActionMap *actions,
					    MvrRobot *robot,
					    bool logActions = false);
protected:
  MvrActionDesired myActionDesired;
};

#endif // ARPRIORITYRESOLVER_H
