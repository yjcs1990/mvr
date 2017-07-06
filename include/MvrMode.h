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
#ifndef ARMODE_H
#define ARMODE_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include <string>
#include <list>

class MvrRobot;


/// A class for different modes, mostly as related to keyboard input
/**
  Each mode is going to need to add its keys to the keyHandler...
  each mode should only use the keys 1-0, the arrow keys (movement),
  the space bar (stop), z (zoom in), x (zoom out), and e (exercise)...
  then when its activate is called by that key handler it needs to
  first deactivate the ourActiveMode (if its not itself, in which case
  its done) then add its key handling stuff... activate and deactivate
  will need to add and remove their user tasks (or call the base class
  activate/deactivate to do it) as well as the key handling things for
  their other part of modes.  This mode will ALWAYS bind help to /, ?, h,
  and H when the first instance of an MvrMode is made.

 @ingroup OptionalClasses
**/
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
  /** 
      This is called as soon as a mode is activated, and should give
      directions on to what keys do what and what this mode will do
  **/
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
