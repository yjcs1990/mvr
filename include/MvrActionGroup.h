/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionGroup.h
 > Description  : Group a set of ArAction objects together 
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONGROUP_H
#define MVRACTIONGROUP_H

#include "mvriaTypedefs.h"
#include <list>

class MvrRobot;
class MvrAction;

class MvrActionGroup
{
public:
  /// Constructor
  MVREXPORT MvrActionGroup(MvrRobot *robot);
  /// Destructor, it also deletes the actions in its group
  MVREXPORT virtual ~MvrActionGroup();
  /// Adds an action to this group's robot, and associates the action with this group 
  MVREXPORT virtual void addAction(MvrAction *action, int priority);
  /// Removes the action from this group's robot, and disassociates the action with this group 
  MVREXPORT virtual void remAction(MvrAction *action);
  /// Activates all the actions in this group
  MVREXPORT virtual void activate(void);
  /// Activates all the actions in this group and deactivates all others
  MVREXPORT virtual void activateExcusive(void);
  /// Deactivates all the actions in this group
  MVREXPORT virtual void deactivate(void);
  /// Removes all the actions in this group from the robot
  MVREXPORT virtual void removeActions(void);
  /// Delets all the actions in this group
  MVREXPORT virtual void deleteActions(void);
  /// Gets the action list
  MVREXPORT virtual std::list<MvrAction *> *getActionList(void);
protected:
  std::list<MvrAction *> myActions;
  MvrRobot *myRobot;
};


#endif  // MVRACTIONGROUP_H