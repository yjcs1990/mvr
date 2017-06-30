/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "MvrExport.h"
#include "ariaOSDef.h"
#include "MvrActionGroup.h"
#include "MvrAction.h"
#include "MvrRobot.h"
#include "MvrLog.h"

/**
   @param robot The robot that this action group is attached to. New actions added to this group (using addAction()) will be added to this robot object for evaluation in its action resolution task.
**/

MVREXPORT MvrActionGroup::ArActionGroup(MvrRobot *robot)
{
  myRobot = robot;
}

MVREXPORT MvrActionGroup::~MvrActionGroup()
{
  removeActions();
}


/**
 * The given action will be included in this group, and then added to this
 * group's robot (specified in the constructor) by using
 * MvrRobot::addAction().
   @param action the action to add to the robot and to this group
   @param priority the priority to give the action; same meaning as in MvrRobot::addAction
   @see MvrRobot::addAction
*/
MVREXPORT void MvrActionGroup::addAction(MvrAction *action, int priority)
{
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::addAction: NULL robot pointer... failed.");
    return;
  }
  myActions.push_front(action);
  myRobot->addAction(action, priority);
}

/**
   @param action the action to remove from the robot
   @see MvrRobot::remAction
*/
MVREXPORT void MvrActionGroup::remAction(MvrAction *action)
{
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::remAction: NULL robot pointer... failed.");
    return;
  }
  myActions.remove(action);
  myRobot->remAction(action);
}

MVREXPORT void MvrActionGroup::activate(void)
{
  std::list<ArAction *>::iterator it;
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::activate: NULL robot pointer... failed.");
    return;
  }
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->activate();
}

MVREXPORT void MvrActionGroup::activateExclusive(void)
{
  std::list<ArAction *>::iterator it;
  
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::activateExclusive: NULL robot pointer... failed.");
    return;
  }
  myRobot->deactivateActions();
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->activate();
}

MVREXPORT void MvrActionGroup::deactivate(void)
{
  std::list<ArAction *>::iterator it;
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::deactivate: NULL robot pointer... failed.");
    return;
  }
  // this was around since 2003 but it shouldn't deactivate actions on
  // the robot in deactivate...

  //myRobot->deactivateActions();
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->deactivate();
}

MVREXPORT std::list<ArAction *> *ArActionGroup::getActionList(void)
{
  return &myActions;
}

MVREXPORT void MvrActionGroup::removeActions(void)
{
  std::list<ArAction *>::iterator it;

  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
            "MvrActionGroup::removeActions: NULL robot pointer... very bad.");
    return;
  }

  for (it = myActions.begin(); it != myActions.end(); ++it)
    myRobot->remAction((*it));
}

MVREXPORT void MvrActionGroup::deleteActions(void)
{
  /* MPL removed this since it doesn't differentiate between actions it added and actions added to it, double deletes are no fun
   */
  /*
  std::list<ArAction *>::iterator it;
  for (it = myActions.begin(); it != myActions.end(); ++it)
  {
    delete (*it);
  }
  */
}
