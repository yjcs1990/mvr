/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrAction.h
 > Description  : This class is the Base class for actions
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTION_H
#define MVRACTION_H

#include "mvriaTypedefs.h"
#include "MvrArg.h"
#include "MvrActionDesired.h"
#include <map>
#include <string>

class MvrRobot;

class MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrAction(const char *name, const char *description="");
  /// Destructor
  MVREXPORT virtual ~MvrAction();
  /// Returns whether the action is active or not
  MVREXPORT virtual bool isActive(void) const;
  /// Activate the action
  MVREXPORT virtual void activate(void);
  /// Deactivate the action
  MVREXPORT virtual void deactivate(void);
  /// Fires the action, returning what the action wants to do
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired) = 0;
  /// Sets the robot this action is driving
  MVREXPORT virtual setRobot(MvrRobot *robot);
  /// Find the number of arguments this action takes
  MVREXPORT virtual int getNumArgs(void) const;
#ifndef SWIG
  /// Gets the numbered argument
  MVREXPORT virtual const MvrArg *getArg(int number) const;
#endif  // SWIG
  /// Gets the numbered argument
  MVREXPORT virtual MvrArg *getArg(int number);
  /// Gets the name of the action
  MVREXPORT virtual const char *getName(void) const;
  /// Gets the long description of the action
  MVREXPORT virtual const char *getDescription(void) const;
  /// Gets what this action wants to do
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return NULL; }
  /// Gets what this action wants to do
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const { return NULL; }
  /// Log information about this action using MvrLog
  MVREXPORT virtual void log(bool verbose=true) const;

  /// Get the robot we are controlling, which was set by setRobot()
  MVREXPORT MvrRobot *getRobot() const { return myRobot; }
  /// Sets the default activation state for all MvrActions
  static void setDefaultActivationState(bool defaultActivationState)
  { ourDefaultActivationState = defaultActivationState; }
  /// Gest the defuault activation state for all mvrActions
  static bool getDefaultActivationState(void)
  { return ourDefaultActivationState; }
protected:
  /// Sets the argument type for the next argument
  MVREXPORT void setNextArgument(MvrArg const &arg);

  /// The robot we are controlling, set by the action using robot
  MvrRobot *myRobot;

  /// These are mostly for internal use by MvrAction,
  bool myIsActive;
  int myNumArgs;
  std::map<int, MvrArg> myArgumentMap;
  std::string myName;
  std::string myDescription;

  MVREXPORT static bool ourDefaultActivationState;
};

#endif  // MVRACTION_H