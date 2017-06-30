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
#ifndef ARACTIONGROUPS_H
#define ARACTIONGROUPS_H

#include "ariaTypedefs.h"
#include "MvrActionGroup.h"
#include "MvrActionColorFollow.h"
#include "MvrACTS.h"
#include "MvrPTZ.h"

class MvrActionStop;
class MvrActionInput;
class MvrActionJoydrive;
class MvrActionDeceleratingLimiter;
class MvrActionRatioInput;
class MvrRatioInputKeydrive;
class MvrRatioInputJoydrive;
class MvrRatioInputRobotJoydrive;

/// Action group to use to drive the robot with input actions (keyboard, joystick, etc.)
/** 
   This class is just useful for teleoping the robot under your own
   joystick and keyboard control... Note that you the predefined
   MvrActionGroups in ARIA are made only to be used exclusively... only
   one can be active at once.

   This class is largely now obsolete (it is used by MvrServerModeDrive
   but that is now obsolete and was replaced by a class that just
   makes its own action group)

   MvrActionGroupRatioDrive is better.
**/
class MvrActionGroupInput : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupInput(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupInput();
  AREXPORT void setVel(double vel);
  AREXPORT void setRotVel(double rotVel);
  AREXPORT void setHeading(double heading);
  AREXPORT void deltaHeadingFromCurrent(double delta);
  AREXPORT void clear(void);
  AREXPORT MvrActionInput *getActionInput(void);
protected:
  MvrActionInput *myInput;
};

/// Action group to stop the robot
/** 
   This class is just useful for having the robot stopped... Note that
   you the predefined MvrActionGroups in ARIA are made only to be used
   exclusively... they won't combine.
**/
class MvrActionGroupStop : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupStop(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupStop();
  AREXPORT MvrActionStop *getActionStop(void);
public:
  MvrActionStop *myActionStop;
};

/// Action group to teleopoperate the robot using MvrActionJoydrive, and the Limiter actions to avoid collisions.
/** 
   This class is just useful for teleoping the robot and having these
   actions read the joystick and keyboard... Note that you the
   predefined MvrActionGroups in ARIA are made only to be used
   exclusively... only one can be active at once.
**/
class MvrActionGroupTeleop : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupTeleop(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupTeleop();
  AREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
protected:
  MvrActionJoydrive *myJoydrive;
};

/// Action group to teleoperate the robot using MvrActionJoydrive, but without any Limiter actions to avoid collisions.
/** 
   This class is just useful for teleoping the robot in an unguarded
   and unsafe manner and having these actions read the joystick and
   keyboard... Note that you the predefined MvrActionGroups in ARIA are
   made only to be used exclusively... only one can be active at once.
**/
class MvrActionGroupUnguardedTeleop : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupUnguardedTeleop(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupUnguardedTeleop();
  AREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
protected:
  MvrActionJoydrive *myJoydrive;
};

/// Action group to make the robot wander, avoiding obstacles.
/** 
   This class is useful for having the robot wander... Note that
   you the predefined MvrActionGroups in ARIA are made only to be used
   exclusively... only one can be active at once.
**/
class MvrActionGroupWander : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupWander(MvrRobot *robot, int forwardVel = 400, int avoidFrontDist = 450, int avoidVel = 200, int avoidTurnAmt = 15);
  AREXPORT virtual ~ArActionGroupWander();
};

/// Follows a blob of color
/** 
   This class has the robot follow a blob of color... Note that you the
   predefined MvrActionGroups in ARIA are made only to be used
   exclusively... only one can be active at once.
**/

class MvrActionGroupColorFollow : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupColorFollow(MvrRobot *robot, MvrACTS_1_2 *acts, MvrPTZ *camera);
  AREXPORT virtual ~ArActionGroupColorFollow();
  AREXPORT void setCamera(MvrPTZ *camera);
  AREXPORT void setChannel(int channel);
  AREXPORT void startMovement();
  AREXPORT void stopMovement();
  AREXPORT void setAcquire(bool acquire);
  AREXPORT int getChannel();
  AREXPORT bool getAcquire();
  AREXPORT bool getMovement();
  AREXPORT bool getBlob();
protected:
  MvrActionColorFollow *myColorFollow;
};

/// Use keyboard and joystick input to to drive the robot, with Limiter actions to avoid obstacles.
/** 
   This class is just useful for teleoping the robot under your own
   joystick and keyboard control... Note that you the predefined
   MvrActionGroups in ARIA are made only to be used exclusively (one at
   a time)... only one can be active at once.
**/
class MvrActionGroupRatioDrive : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupRatioDrive(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupRatioDrive();
  AREXPORT MvrActionRatioInput *getActionRatioInput(void);
  AREXPORT void addToConfig(MvrConfig *config, const char *section);
protected:
  MvrActionDeceleratingLimiter *myDeceleratingLimiterForward;
  MvrActionDeceleratingLimiter *myDeceleratingLimiterBackward;
  MvrActionDeceleratingLimiter *myDeceleratingLimiterLateralLeft;
  MvrActionDeceleratingLimiter *myDeceleratingLimiterLateralRight;
  MvrActionRatioInput *myInput;
  MvrRatioInputKeydrive *myKeydrive;
  MvrRatioInputJoydrive *myJoydrive;
  MvrRatioInputRobotJoydrive *myRobotJoydrive;

};


/// Use keyboard and joystick input to to drive the robot, but without Limiter actions to avoid obstacles.
/** 
   This class is just useful for teleoping the robot under your own
   joystick and keyboard control... Note that you the predefined
   MvrActionGroups in ARIA are made only to be used exclusively (one at
   a time)... only one can be active at once.
**/
class MvrActionGroupRatioDriveUnsafe : public MvrActionGroup
{
public:
  AREXPORT MvrActionGroupRatioDriveUnsafe(MvrRobot *robot);
  AREXPORT virtual ~ArActionGroupRatioDriveUnsafe();
  AREXPORT MvrActionRatioInput *getActionRatioInput(void);
  AREXPORT void addToConfig(MvrConfig *config, const char *section);
protected:
  MvrActionRatioInput *myInput;
  MvrRatioInputKeydrive *myKeydrive;
  MvrRatioInputJoydrive *myJoydrive;
  MvrRatioInputRobotJoydrive *myRobotJoydrive;

};

#endif // ARACTIONGROUPS_H
