#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrResolver.h"
#include "MvrAction.h"
#include "MvrLog.h"
#include "MvrRobot.h"
#include <string.h>

AREXPORT bool MvrAction::ourDefaultActivationState = true;

AREXPORT MvrAction::MvrAction(const char *name, const char *description)
{
  myRobot = NULL;
  myNumArgs = 0;
  myName = name;
  myDescription = description;
  myIsActive = ourDefaultActivationState;
}

AREXPORT MvrAction::~MvrAction()
{
  if (myRobot != NULL)
    myRobot->remAction(this);
}

AREXPORT const char *MvrAction::getName(void) const
{
  return myName.c_str();
}

AREXPORT const char *MvrAction::getDescription(void) const
{
  return myDescription.c_str();
}

AREXPORT int MvrAction::getNumArgs(void) const
{
  return myNumArgs;
}

AREXPORT void MvrAction::setNextArgument(MvrArg const &arg)
{
  myArgumentMap[myNumArgs] = arg;
  myNumArgs++;
}

AREXPORT MvrArg *MvrAction::getArg(int number) 
{
  std::map<int, MvrArg>::iterator it;
  
  it = myArgumentMap.find(number);
  if (it != myArgumentMap.end())
    return &(*it).second;
  else
    return NULL;
}

AREXPORT const MvrArg *MvrAction::getArg(int number) const
{
  std::map<int, MvrArg>::const_iterator it;
  
  it = myArgumentMap.find(number);
  if (it != myArgumentMap.end())
    return &(*it).second;
  else
    return NULL;
}

/**
 *  @swignote If you override this method in a Java or Python subclass, use
 *  setActionRobotObj(MvrRobot) instead of trying to call super.setRobot() or 
 *  MvrAction.setRobot(). (SWIG's subclassing "directors" feature cannot properly
 *  direct the call to the parent class, an infinite recursion results instead.)
 */
AREXPORT void MvrAction::setRobot(MvrRobot *robot)
{
  myRobot = robot;
}

AREXPORT bool MvrAction::isActive(void) const
{
  return myIsActive;
}

AREXPORT void MvrAction::activate(void)
{
  myIsActive = true;
}

AREXPORT void MvrAction::deactivate(void)
{
  myIsActive = false;
}

AREXPORT void MvrAction::log(bool verbose) const
{
  int i;
  std::string str;
  const MvrArg *arg;
  const MvrActionDesired *desired;

  MvrLog::log(MvrLog::Terse, "Action %s isActive %d", getName(), myIsActive);
  if (myIsActive && (desired = getDesired()) != NULL)
    desired->log();
  if (!verbose)
    return;
  if (strlen(getDescription()) != 0)
    MvrLog::log(MvrLog::Terse, "Action %s is described as: %s", 
	       getName(), getDescription());
  else
    MvrLog::log(MvrLog::Terse, "Action %s has no description.", 
	       getName());
  if (getNumArgs() == 0)
    MvrLog::log(MvrLog::Terse, "Action %s has no arguments.\n", 
	       getName());
  else
  {
    MvrLog::log(MvrLog::Terse, "Action %s has %d arguments, of type(s):", 
	       (getName()), getNumArgs());

    for (i = 0; i < getNumArgs(); i++) 
    {
      arg = getArg(i);
      if (arg == NULL)
	continue;
      arg->log();
    }
    MvrLog::log(MvrLog::Terse, "");
  }
}



