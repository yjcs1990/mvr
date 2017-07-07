#ifndef MVRACTIONGROUP_H
#define MVRACTIONGROUP_H

#include "mvriaTypedefs.h"

#include <list>

class MvrRobot;
class MvrAction;

/// Group a set of MvrAction objects together 
/**
   This class is used to collect a group of related MvrActions together, 
   and easily turn them on and off in aggregate. The group list may also
   be retrieved for performing any other operation you wish (e.g. to delete 
   or get information about them.)
   
   @see @ref actions overview
   @see MvrAction
   @see @ref actionGroupExample.cpp
  @ingroup OptionalClasses
  @ingroup ActionClasses
**/
class MvrActionGroup
{
public:
  /// Constructor
  MVREXPORT MvrActionGroup(MvrRobot * robot);
  /// Destructor, it also deletes the actions in its group
  MVREXPORT virtual ~MvrActionGroup();
  /// Adds an action to this group's robot, and associates the action with this group.
  MVREXPORT virtual void addAction(MvrAction *action, int priority);
  /// Removes the action from this group's robot and dissasociates it from this group.
  MVREXPORT virtual void remAction(MvrAction *action);
  /// Activates all the actions in this group
  MVREXPORT virtual void activate(void);
  /// Activates all the actions in this group and deactivates all others
  MVREXPORT virtual void activateExclusive(void);
  /// Deactivates all the actions in this group
  MVREXPORT virtual void deactivate(void);
  /// Removes all the actions in this group from the robot
  MVREXPORT virtual void removeActions(void);
  /// Delets all the actions in this group (doesn't delete them right now)
  MVREXPORT virtual void deleteActions(void);
  /// Gets the action list (use this to delete actions after doing removeActions)
  MVREXPORT virtual std::list<MvrAction *> *getActionList(void);
protected:
  std::list<MvrAction *> myActions;
  MvrRobot *myRobot;
};

#endif // ARACTIONGROUP_H
