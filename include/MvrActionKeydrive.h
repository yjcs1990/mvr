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
#ifndef ARACTIONKEYDRIVE_H
#define ARACTIONKEYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrFunctor.h"

class MvrRobot;

/// This action will use the keyboard arrow keys for input to drive the robot
/// @ingroup ActionClasses
class MvrActionKeydrive : public MvrAction
{
public:
  /// Constructor
 MVREXPORT MvrActionKeydrive(const char *name = "keydrive",
			   double transVelMax = 400,
			   double turnAmountMax = 24,
			   double velIncrement = 25,
			   double turnIncrement = 8);
  /// Destructor
  MVREXPORT virtual ~MvrActionKeydrive();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  /// For setting the maximum speeds
  MVREXPORT void setSpeeds(double transVelMax, double turnAmountMax);
  /// For setting the increment amounts
  MVREXPORT void setIncrements(double velIncrement, double turnIncrement);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  /// Takes the keys this action wants to use to drive
  MVREXPORT void takeKeys(void);
  /// Gives up the keys this action wants to use to drive
  MVREXPORT void giveUpKeys(void);
  /// Internal, callback for up arrow
  MVREXPORT void up(void);
  /// Internal, callback for down arrow
  MVREXPORT void down(void);
  /// Internal, callback for left arrow
  MVREXPORT void left(void);
  /// Internal, callback for right arrow
  MVREXPORT void right(void);
  /// Internal, callback for space key
  MVREXPORT void space(void);

protected:
  MvrFunctorC<MvrActionKeydrive> myUpCB;
  MvrFunctorC<MvrActionKeydrive> myDownCB;
  MvrFunctorC<MvrActionKeydrive> myLeftCB;
  MvrFunctorC<MvrActionKeydrive> myRightCB;
  MvrFunctorC<MvrActionKeydrive> mySpaceCB;
  // action desired
  MvrActionDesired myDesired;
  // full speed
  double myTransVelMax;
  // full amount to turn
  double myTurnAmountMax;
  // amount to increment vel by on an up arrow or decrement on a down arrow
  double myVelIncrement;
  // amount to increment turn by on a left arrow or right arrow
  double myTurnIncrement;
  // amount we want to speed up
  double myDeltaVel;
  // amount we want to turn 
  double myTurnAmount;
  // what speed we want to go
  double myDesiredSpeed;
  // if our speeds been reset
  bool mySpeedReset;
};


#endif // ARACTIONKEYDRIVE_H
