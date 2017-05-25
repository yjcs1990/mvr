/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMode.h
 > Description  : A class for different modes, mostly as related to keyboard input
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRMODE_H
#define MVRMODE_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include <string>
#include <list>

class MvrRobot;

class MvrMode 
{
public:
  /// Constructor
  MVREXPORT MvrMode(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrMode();
  /// Gets the name of the mode
  MVREXPORT const char *getName(void);
  /// The function called when the mode is activated, subclass must provide
  MVREXPORT virtual void activate(void) = 0;
  /// The function called when the mode is deactivated, subclass must provide
  MVREXPORT virtual void deactivate(void) = 0;
  /// The MvrMode's user task, don't need one, subclass must provide if needed
  MVREXPORT virtual void userTask(void) {}
  /// The mode's help print out... subclass must provide if needed
  MVREXPORT virtual void help(void) {}
  /// The base activation, it MUST be called by inheriting classes,
  /// and inheriting classes MUST return if this returns false
  MVREXPORT bool baseActivate(void); 
  /// The base deactivation, it MUST be called by inheriting classes,
  /// and inheriting classes MUST return if this returns false
  MVREXPORT bool baseDeactivate(void);
  /// This is the base help function, its internal, bound to ? and h and H
  MVREXPORT static void baseHelp(void);
  /// An internal function to get the first key this is bound to
  MVREXPORT char getKey(void);
  /// An internal function to get the second key this is bound to
  MVREXPORT char getKey2(void);
protected:
  MVREXPORT void addKeyHandler(int keyToHandle, MvrFunctor *functor);
  MVREXPORT void remKeyHandler(MvrFunctor *functor);
  // Our activeMvrMode
  MVREXPORT static MvrMode *ourActiveMode;
  std::string myName;
  MvrRobot *myRobot;
  MvrFunctorC<MvrMode> myActivateCB;
  MvrFunctorC<MvrMode> myDeactivateCB;
  MvrFunctorC<MvrMode> myUserTaskCB;
  char myKey;
  char myKey2;
  // our help callback, its NULL until its initialized
  static MvrGlobalFunctor *ourHelpCB;
  static std::list<MvrMode *> ourModes;
};

#endif // ARMODE_H


#endif  // MVRMODE_H