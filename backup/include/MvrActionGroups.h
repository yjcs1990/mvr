/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionGroups.h
 > Description  : Action group to use to drive the robot with input actions 
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
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
/// This class is just useful for having the robot stopped
class MvrActionGroupStop : public MvrActionGroup
{
public:
  MVREXPORT MvrActionGroupStop(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupStop();
  MVREXPORT MvrActionStop *getActionStop(void);
public:
  MvrActionStop *myActionStop;
};
/// Action group to teleoperate the robot using MvrActionJoydrive
/// and the Limiter actions to avoid collisions.
class MvrActionGroupTeleop : public MvrActionGroup
{
public:
  MVREXPORT MvrActionGroupTeleop(MvrRobot *robot);
  MVREXPORT virtual ~MvrActionGroupTeleop();
  MVREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
protected:
  MvrActionJoydrive *myJoydrive;
};
/// Action group to teleoperate the robot using MvrActionJoydrive
/// but without any Limiter actions to avoid collisions.
class MrActionGroupUnguardedTeleop : public MvrActionGroup
{
public:
  MVREXPORT MrActionGroupUnguardedTeleop(MvrRobot *robot);
  MVREXPORT virtual ~MrActionGroupUnguardedTeleop();
  MVREXPORT void setThrottleParams(int lowSpeed, int highSpeed);
protected:
  MvrActionJoydrive *myJoydrive;
};
/// Action group to make the robot wander, avoiding obstacles.
class MvrActionGroupWander : public MvrActionGroup
{
public:
  MVREXPORT MvrActionGroupWander(MvrRobot *robot, int forwardVel=400,
            int avoidFrontDist = 450, int avoidVel = 200, int avoidTurnAmt = 15);
  MVREXPORT virtual ~MvrActionGroupWander();
};
/// Follows a blob of color
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
#endif  // MVRACTIONGROUPS_H