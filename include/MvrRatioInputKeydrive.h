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
#ifndef ARRATIOINPUTKEYDRIVE_H
#define ARRATIOINPUTKEYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrFunctor.h"
#include "MvrActionRatioInput.h"


/// This will use the keyboard arrow keys and the MvrActionRatioInput to drive the robot
/**
   You have to make an MvrActionRatioInput and add it to the robot like
   a normal action for this to work.

  @ingroup OptionalClasses
**/
class MvrRatioInputKeydrive 
{
public:
  /// Constructor
  MVREXPORT MvrRatioInputKeydrive(MvrRobot *robot, MvrActionRatioInput *input, 
				int priority = 25, double velIncrement = 5);
  /// Destructor
  MVREXPORT virtual ~MvrRatioInputKeydrive();
  /// Takes the keys this action wants to use to drive
  MVREXPORT void takeKeys(void);
  /// Gives up the keys this action wants to use to drive
  MVREXPORT void giveUpKeys(void);
  /// Internal, callback for up arrow
  MVREXPORT void up(void);
  /// Internal, callback for down arrow
  MVREXPORT void down(void);
  /// Internal, callback for z
  MVREXPORT void z(void);
  /// Internal, callback for x 
  MVREXPORT void x(void);
  /// Internal, callback for left arrow
  MVREXPORT void left(void);
  /// Internal, callback for right arrow
  MVREXPORT void right(void);
  /// Internal, callback for space key
  MVREXPORT void space(void);
  /// Internal, gets our firecb
  MVREXPORT MvrFunctor *getFireCB(void) { return &myFireCB; }
protected:
  MVREXPORT void activate(void);
  MVREXPORT void deactivate(void);
  MVREXPORT void fireCallback(void);
  MvrFunctorC<MvrRatioInputKeydrive> myUpCB;
  MvrFunctorC<MvrRatioInputKeydrive> myDownCB;
  MvrFunctorC<MvrRatioInputKeydrive> myLeftCB;
  MvrFunctorC<MvrRatioInputKeydrive> myRightCB;
  MvrFunctorC<MvrRatioInputKeydrive> myZCB;
  MvrFunctorC<MvrRatioInputKeydrive> myXCB;
  MvrFunctorC<MvrRatioInputKeydrive> mySpaceCB;

  double myPrinting;
  double myTransRatio;
  double myRotRatio;
  double myThrottle;
  double myLatRatio;

  MvrRobot *myRobot;
  bool myHaveKeys;
  double myVelIncrement;
  double myLatVelIncrement;
  MvrActionRatioInput *myInput;
  MvrFunctorC<MvrRatioInputKeydrive> myFireCB;
  MvrFunctorC<MvrRatioInputKeydrive> myActivateCB;
  MvrFunctorC<MvrRatioInputKeydrive> myDeactivateCB;
};


#endif // ARRATIOINPUTKEYDRIVE_H
