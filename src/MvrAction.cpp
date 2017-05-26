/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrAction.cpp
 > Description  : This class is the Base class for actions
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaOSDef.h"
#include "MvrResolver.h"
#include "MvrAction.h"
#include "MvrLog.h"
#include "MvrRobot.h"
#include <string.h>

MVREXPORT bool MvrAction::MvrAction::ourDefaultActivationState = true;

MVREXPORT MvrAction::MvrAction(const char *name, const char *description)
{
  myRobot = NULL;
  myNumArgs = 0;
  myName = name;
  myDescription = description;
  myIsActive = ourDefaultActivationState;
}

MVREXPORT MvrAction::~MvrAction()
{
  if (myRobot != NULL)
    myRobot->remAction(this);
}

MVREXPORT const char *MvrAction::getName(void) const
{ return myName.c_str(); }

MVREXPORT const char *MvrAction::getDescription(void) const
{ return myDescription.c_str(); }

MVREXPORT int MvrAction::getNumArgs(void) const
{ return myNumArgs; }

MVREXPORT void MvrAction::setNextArgument(MvrArg const &arg)
{
  myArgumentMap[myNumArgs] = arg;
  myNumArgs++;
}

MVREXPORT MvrArg *MvrAction::getArg(int number)
{
  std::map<int, MvrArg>::iterator it;
  
   it = myArgumentMap.find(number);
   if(it != myArgumentMap.end())
     return &(*it).second;
   else
     return NULL;
}

MVREXPORT MvrArg *MvrAction::getArg(int number) const
{
  std::map<int, MvrArg>::const_iterator it;
  
   it = myArgumentMap.find(number);
   if(it != myArgumentMap.end())
     return &(*it).second;
   else
     return NULL;
}

MVREXPORT void MvrAction::setRobot(MvrRobot *robot)
{ myRObot = robot; }

MVREXPORT bool MvrAction::isActive(void)
{ return myIsActive; }

MVREXPORT void MvrAction::activate(void)
{ myIsActive = true; }

MVREXPORT void MvrAction::deactivate(void)
{ myIsActive = false; }

MVREXPORT void MvrAction::log(bool verbose) const
{
  int i;
  std::string std;
  const MvrArg *arg;
  const MvrActionDesired *desired;

  MvrLog::log(MvrLog::Terser, "Action %s isActive %d", getName(), myIsActive());
  if(myIsActive && (desired = getDesired()) != NULL)
    desired->log();
  if(!verbose)
    return;
  if(strlen(getDescription()) != 0)
    MvrLog::log(MvrLog::Terse, "Action %s is described as: %s", getName(), getDescription());
  else
    MvrLog::log(MvrLog::Terse, "Action %s has no description.", getName());
  if(getNumArgs() == 0)
    MvrLog::log(MvrLog::Terse, "Action %s has no arguments.\n", getName());
  else
  {
    MvrLog::log(MvrLog::Terse, "Action %s has %d arguments, of type(s): \n", getName(), getNumArgs());
    for(i=0; i < getNumArgs(); i++)
    {
      arg = getArg(i);
      if(arg == NULL)
        continue;
      arg->log();
    }
    MvrLog::log(MvrLog::Terse, "");   
  }
}