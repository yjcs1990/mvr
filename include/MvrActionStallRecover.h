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
#ifndef ARACTIONSTALLRECOVER_H
#define ARACTIONSTALLRECOVER_H

#include "ariaTypedefs.h"
#include "MvrAction.h"

class MvrResolver;

/// Action to recover from a stall
/**
   This action tries to recover if one of the wheels has stalled, it has a 
   series of actions it tries in order to get out of the stall.
   @ingroup ActionClasses
*/
class MvrActionStallRecover : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionStallRecover(const char * name = "stall recover", 
				double obstacleDistance = 225, int cyclesToMove = 50, 
				double speed = 150, double degreesToTurn = 45,
				bool enabled = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionStallRecover();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) 
    { return &myActionDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                   { return &myActionDesired; }
#endif
  MVREXPORT void addToConfig(MvrConfig* config, const char* sectionName, MvrPriority::Priority priority = MvrPriority::NORMAL);
  MVREXPORT virtual void activate(void);
protected:
  // these are internal things, don't touch unless you know what you are doing
  void doit(void); // does whatever should be done
  void addSequence(int sequence);
  int myDoing; // what we're doing, uses the stuff from the enum What
  int myState; // holds the state
  int myCount; // count down variable, -1 if first time in this state
  int mySideStalled; // 1 for left, 2 for right, 3 for both

  enum State 
  { 
    STATE_NOTHING = 0, // waiting
    STATE_GOING // do something
  };

  enum What
  {
    BACK=0x1, // back up
    FORWARD=0x2, // go forward
    TURN=0x4, // turn away from obstacles
    TURN_LEFT=0x8, // turn left
    TURN_RIGHT=0x10, // turn right
    MOVEMASK = BACK | FORWARD,
    TURNMASK = TURN | TURN_LEFT | TURN_RIGHT
  };

  std::map<int, int> mySequence; // list of things to do as stall continues
  int mySequenceNum;
  int mySequencePos;
  time_t myLastFired;
  double myObstacleDistance;
  int myCyclesToMove;
  bool myEnabled;
  double mySpeed;
  int myCyclesToTurn;
  double myDegreesToTurn;
  double myDesiredHeading;
  MvrActionDesired myActionDesired;
  
  MvrResolver *myResolver;
};

#endif //ARACTIONSTALLRECOVER_H
