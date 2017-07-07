#ifndef MVRACTIONGROUPS_H
#define MVRACTIONGROUPS_H

#include "mvriaTypedefs.h"
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
  MVREXPORT MvrActionGroupInput(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupInput();
  MVREXPORT void setVel(double vel);
  MVREXPORT void setRotVel(double rotVel);
  MVREXPORT void setHeading(double heading);
  MVREXPORT void deltaHeadingFromCurrent(double delta);
  MVREXPORT void clear(void);
  MVREXPORT MvrActionInput *getActionInput(void);
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
  MVREXPORT MvrActionGroupStop(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupStop();
  MVREXPORT MvrActionStop *getActionStop(void);
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
  MVREXPORT MvrActionGroupTeleop(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupTeleop();
  MVREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
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
  MVREXPORT MvrActionGroupUnguardedTeleop(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupUnguardedTeleop();
  MVREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
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
  MVREXPORT MvrActionGroupWander(MvrRobot *robot, int forwardVel = 400, int avoidFrontDist = 450, int avoidVel = 200, int avoidTurnAmt = 15);
  MVREXPORT virtual ~MvrActionGroupWander();
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
  MVREXPORT MvrActionGroupColorFollow(MvrRobot *robot, MvrACTS_1_2 *acts, MvrPTZ *camera);
  MVREXPORT virtual ~MvrActionGroupColorFollow();
  MVREXPORT void setCamera(MvrPTZ *camera);
  MVREXPORT void setChannel(int channel);
  MVREXPORT void startMovement();
  MVREXPORT void stopMovement();
  MVREXPORT void setAcquire(bool acquire);
  MVREXPORT int getChannel();
  MVREXPORT bool getAcquire();
  MVREXPORT bool getMovement();
  MVREXPORT bool getBlob();
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
  MVREXPORT MvrActionGroupRatioDrive(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupRatioDrive();
  MVREXPORT MvrActionRatioInput *getActionRatioInput(void);
  MVREXPORT void addToConfig(MvrConfig *config, const char *section);
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
  MVREXPORT MvrActionGroupRatioDriveUnsafe(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupRatioDriveUnsafe();
  MVREXPORT MvrActionRatioInput *getActionRatioInput(void);
  MVREXPORT void addToConfig(MvrConfig *config, const char *section);
protected:
  MvrActionRatioInput *myInput;
  MvrRatioInputKeydrive *myKeydrive;
  MvrRatioInputJoydrive *myJoydrive;
  MvrRatioInputRobotJoydrive *myRobotJoydrive;

};

#endif // ARACTIONGROUPS_H
