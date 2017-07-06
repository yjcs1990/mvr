#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrPriorityResolver.h"
#include "MvrAction.h"
#include "MvrRobot.h"

MVREXPORT MvrPriorityResolver::MvrPriorityResolver() :
  MvrResolver("MvrPriorityResolver", "Resolves strictly by using priority, the highest priority action to act is the one that gets to go.  Does no mixing of any variety.")
{
}


MVREXPORT MvrPriorityResolver::~MvrPriorityResolver()
{
}

MVREXPORT MvrActionDesired *MvrPriorityResolver::resolve(
	MvrResolver::ActionMap *actions, MvrRobot *robot, bool logActions)
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
      // from here
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
  /*
  printf(
      "desired delta %.0f strength %.3f, desired speed %.0f strength %.3f\n",
      myActionDesired.getDeltaHeading(), myActionDesired.getHeadingStrength(), 
      myActionDesired.getVel(), myActionDesired.getVelStrength());
  */
  return &myActionDesired;
}
