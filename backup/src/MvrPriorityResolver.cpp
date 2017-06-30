/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrPriorityResolver.h
 > Description  : This class takes the action list and uses the priority to resolve
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrAction.h"
#include "MvrRobot.h"
#include "MvrPriorityResolver.h"

MVREXPORT MvrPriorityResolver::MvrPriorityResolver() :
          MvrResolver("MvrPriorityResolver","Resolver strictly by using priority, the highest priority action to act is the one that gets to go.  Does no mixing of any variety.")
{

}

MVREXPORT MvrPriorityResolver::~MvrPriorityResolver()
{

}

MVREXPORT MvrPriorityResolver *MvrPriorityResolver::resolve(MvrResolver::MvrActionMap *actions,
                                                            MvrRobot *robot, bool logActions)
{
  MvrResolver::ActionMap::reverse_iterator it;
  MvrAction *action;
  MvrActionDesired *act;
  MvrActionDesired averaging;
  bool first = true;
  int lastPriority;
  bool printedFirst = true;
  int printedLast;
  
  if (actions == NULL)
    return NULL;

  myActionDesired.reset();
  averaging.reset();
  averaging.startAverage();
  for (it = actions->rbegin(); it != actions->rend(); ++it)
  {
    action = (*it).second;
    if (action != NULL && action->isActive())
    {
      if (first || (*it).first != lastPriority)
      {
        averaging.endAverage();
        myActionDesired.merge(&averaging);
        
        averaging.reset();
        averaging.startAverage();
        first = false;
        lastPriority = (*it).first;
      }
      act = action->fire(myActionDesired);
      if (robot != NULL && act != NULL)
	      act->accountForRobotHeading(robot->getTh());
      if (act != NULL)
	      act->sanityCheck(action->getName());
      averaging.addAverage(act);
      if (logActions && act != NULL && act->isAnythingDesired())
      {
        if (printedFirst || printedLast != (*it).first)
        {
          MvrLog::log(MvrLog::Terse, "Priority %d:", (*it).first);
          printedLast = (*it).first;
          printedFirst = false;
        }
        MvrLog::log(MvrLog::Terse, "Action: %s", action->getName());
        act->log();
      }
    }
  }
  
  averaging.endAverage();
  myActionDesired.merge(&averaging);
 
  return &myActionDesired;
}
