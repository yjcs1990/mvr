/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrPriorityResolver.h
 > Description  : This class takes the action list and uses the priority to resolve
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRPRIORITYRESOLVER_H
#define MVRPRIORITYRESOLVER_H

#include "MvrResolver.h"

class MvrPriorityResolver : public MvrResolver
{
public:
  /// Constructor
  MVREXPORT MvrPriorityResolver();
  /// Destructor
  MVREXPORT virtual ~MvrPriorityResolver();
  MVREXPORT virtual MvrActionDesired *resolve(MvrResolver::ActionMap *actions,
                                              MvrRobot *robot, bool logActions=false);
protected:
  MvrActionDesired myActionDesired;
};
#endif  // MVRPRIORITYRESOLVER_H
