#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionGroup.h"
#include "MvrAction.h"
#include "MvrRobot.h"
#include "MvrLog.h"

/**
   @param robot The robot that this action group is attached to. New actions added to this group (using addAction()) will be added to this robot object for evaluation in its action resolution task.
**/

MVREXPORT MvrActionGroup::MvrActionGroup(MvrRobot *robot)
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
  std::list<MvrAction *>::iterator it;
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
  std::list<MvrAction *>::iterator it;
  
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
  std::list<MvrAction *>::iterator it;
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

MVREXPORT std::list<MvrAction *> *MvrActionGroup::getActionList(void)
{
  return &myActions;
}

MVREXPORT void MvrActionGroup::removeActions(void)
{
  std::list<MvrAction *>::iterator it;

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
  std::list<MvrAction *>::iterator it;
  for (it = myActions.begin(); it != myActions.end(); ++it)
  {
    delete (*it);
  }
  */
}
