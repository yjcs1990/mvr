/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionGotoStraight.h
 > Description  : This action goes to a given MvrPose very naively
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef ARACTIONGOTOSTRAIGHT_H
#define ARACTIONGOTOSTRAIGHT_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrAction.h"

/// This action goes to a given MvrPose very naively

/**
   This action naively drives straight towards a given MvrPose. The
   action stops the robot when it has travelled the distance that that
   pose is away. It travels at 'speed' mm/sec.

   You can give it a new goal pose with setGoal(), cancel its movement
   with cancelGoal(), and see if it got there with haveAchievedGoal().

   For arguments to the goals and encoder goals you can tell it to go
   backwards by calling them with the backwards parameter true.  If
   you set the justDistance to true it will only really care about
   having driven the distance, if false it'll try to get to the spot
   you wanted within close distance.

   This doesn't avoid obstacles or anything, you could add have an obstacle
   avoidance MvrAction at a higher priority to try to do this. (For
   truly intelligent navigation, see the ARNL and SONARNL software libraries.)
  @ingroup ActionClasses
**/

class MvrActionGotoStraight : public MvrAction
{
public:
  MVREXPORT MvrActionGotoStraight(const char *name = "goto", 
				                          double speed = 400);
  MVREXPORT virtual ~MvrActionGotoStraight();

  /// Sees if the goal has been achieved
  MVREXPORT bool haveAchievedGoal(void);
  /// Cancels the goal the robot has
  MVREXPORT void cancelGoal(void);
  /// Sets a new goal and sets the action to go there
  MVREXPORT void setGoal(MvrPose goal, bool backwards = false, 
			                   bool justDistance = true);
  /// Sets the goal in a relative way
  MVREXPORT void setGoalRel(double dist, double deltaHeading, 
			                      bool backwards = false, bool justDistance = true);
  /// Gets the goal the action has
  MvrPose getGoal(void) { return myGoal; }
  /// Gets whether we're using the encoder goal or the normal goal
  bool usingEncoderGoal(void) { return myUseEncoderGoal; }
  /// Sets a new goal and sets the action to go there
  MVREXPORT void setEncoderGoal(MvrPose encoderGoal, bool backwards = false,
			                          bool justDistance = true);
  /// Sets the goal in a relative way
  MVREXPORT void setEncoderGoalRel(double dist, double deltaHeading, 
                                   bool backwards = false, 
                                   bool justDistance = true);
  /// Gets the goal the action has
  MvrPose getEncoderGoal(void) { return myEncoderGoal; }
  /// Sets the speed the action will travel to the goal at (mm/sec)
  void setSpeed(double speed) { mySpeed = speed; }
  /// Gets the speed the action will travel to the goal at (mm/sec)
  double getSpeed(void) { return mySpeed; }
  /// Sets how close we have to get if we're not in just distance mode
  void setCloseDist(double closeDist = 100) { myCloseDist = closeDist; } 
  /// Gets how close we have to get if we're not in just distance mode
  double getCloseDist(void) { return myCloseDist; }
  /// Sets whether we're backing up there or not (set in the setGoals)
  bool getBacking(void) { return myBacking; }
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif
protected:
  MvrPose myGoal;
  bool myUseEncoderGoal;
  MvrPose myEncoderGoal;
  double mySpeed;
  bool myBacking;
  MvrActionDesired myDesired;
  bool myPrinting;
  double myDist;
  double myCloseDist;

  bool myJustDist;

  double myDistTravelled;
  MvrPose myLastPose;
  
  enum State
  {
    STATE_NO_GOAL, 
    STATE_ACHIEVED_GOAL,
    STATE_GOING_TO_GOAL
  };
  State myState;
};

#endif // ARACTIONGOTO
