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
#ifndef ARACTION_H
#define ARACTION_H

#include "ariaTypedefs.h"
#include "MvrArg.h"
#include "MvrActionDesired.h"
#include <map>
#include <string>

class MvrRobot;

/** @brief Base class for actions
  
    @ref actions Actions are objects queried for desired behavior by
    MvrActionResolver to determine robot movement commands.

    To implement an action object, define a subclass of MvrAction,
    and implement the fire() method. You may also override
    setRobot() to obtain information from MvrRobot, but you
    must also call MvrAction::setRobot() so that the MvrRobot pointer
    is stored by MvrAction.

    Several predefined action objects are also included in ARIA,
    they are listed here as MvrActions's subclasses.

    If an action is not active (it has been deactivated), then
    it will be ignored by the action resolver. 
    Actions may be grouped using MvrActionGroup, and activated/deactivated as a group. For example, MvrMode, and MvrServerMode (from MvrNetworking), activate/deactivate action groups when switching modes.

    @see @ref actions description in the ARIA overview.
    @see MvrActionGroup
    @see MvrResolver
    @see MvrRobot

    @ingroup ActionClasses
    @ingroup OptionalClasses
*/
class MvrAction
{
public:
  /// Constructor
  AREXPORT MvrAction(const char * name, const char * description = "");
  /// Desructor
  AREXPORT virtual ~ArAction();
  /// Returns whether the action is active or not
  AREXPORT virtual bool isActive(void) const;
  /// Activate the action
  AREXPORT virtual void activate(void);
  /// Deactivate the action
  AREXPORT virtual void deactivate(void);
  /// Fires the action, returning what the action wants to do
  /** 
      @param currentDesired this is the tentative result, based
      on the resolver's processing of previous, higher-priority actions.
      This is only for the purpose of giving information to the 
      action, changing it has no effect.
      @return pointer to what this action wants to do, NULL if it wants to do 
      nothing. Common practice is to keep an MvrActionDesired
      object in your action subclass, and return a pointer to
      that object. This avoids the need to create 
      new objects during each invocation (which could never
      be deleted).
      Clear your stored MvrActionDesired
      before modifying it with MvrActionDesired::reset().
  */
  AREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired) = 0;
  /// Sets the robot this action is driving
  AREXPORT virtual void setRobot(MvrRobot *robot);
  /// Find the number of arguments this action takes
  AREXPORT virtual int getNumArgs(void) const;
#ifndef SWIG
  /** Gets the numbered argument
   * @swignote Not available
   */
  AREXPORT virtual const MvrArg *getArg(int number) const;
#endif // SWIG
  /// Gets the numbered argument
  AREXPORT virtual MvrArg *getArg(int number);
  /// Gets the name of the action
  AREXPORT virtual const char *getName(void) const;
  /// Gets the long description of the action
  AREXPORT virtual const char *getDescription(void) const;
  /// Gets what this action wants to do (for display purposes)
  AREXPORT virtual MvrActionDesired *getDesired(void) { return NULL; }
  /// Gets what this action wants to do (for display purposes)
  AREXPORT virtual const MvrActionDesired *getDesired(void) const { return NULL; }
  /// Log information about this action using MvrLog.
  AREXPORT virtual void log(bool verbose = true) const;

  /// Get the robot we are controlling, which was set by setRobot()
  AREXPORT MvrRobot* getRobot() const { return myRobot; }

  /// Sets the default activation state for all MvrActions
  static void setDefaultActivationState(bool defaultActivationState)
    { ourDefaultActivationState = defaultActivationState; }
  /// Gets the default activation state for all MvrActions
  static bool getDefaultActivationState(void)
    { return ourDefaultActivationState; }
protected:  
  /// Sets the argument type for the next argument (must only be used in a constructor!)
  AREXPORT void setNextArgument(MvrArg const &arg);

  /// The robot we are controlling, set by the action resolver using setRobot()
  MvrRobot *myRobot;

  // These are mostly for internal use by MvrAction, not subclasses:
  bool myIsActive;
  int myNumArgs;
  std::map<int, MvrArg> myArgumentMap;
  std::string myName;
  std::string myDescription;

  AREXPORT static bool ourDefaultActivationState;
};


#endif //ARACTION_H
