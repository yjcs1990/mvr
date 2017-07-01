#ifndef MVRACTIONGOTO_H
#define MVRACTIONGOTO_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrAction.h"

/// This action goes to a given MvrPose very naively

/**
   This action naively drives straight towards a given MvrPose. the
   action stops when it gets to be a certain distance (closeDist) 
   from the goal pose.  It travels at the given speed (mm/sec). 

   You can give it a new goal with setGoal(), clear the current goal
   with cancelGoal(), and see if it got there with haveAchievedGoal().
   Once the goal is reached, this action stops requesting any action.

   This doesn't avoid obstacles or anything, you could have an avoid
   routine at a higher priority to avoid on the way there... but for
   real and intelligent looking navigation you should use something
   like ARNL, or build on these actions.
  @ingroup ActionClasses
**/


class MvrActionGoto : public MvrAction
{
public:
  MVREXPORT MvrActionGoto(const char *name = "goto", 
			MvrPose goal = MvrPose(0.0, 0.0, 0.0), 
			double closeDist = 100, double speed = 400,
			double speedToTurnAt = 150, double turnAmount = 7);
  MVREXPORT virtual ~MvrActionGoto();

  /** Sees if the goal has been achieved. The goal is achieved when
   *  the robot's repordet position is within a certain distance
   *  (given in the constructor or in setCloseDist) from the goal pose. */
  MVREXPORT bool haveAchievedGoal(void);

  /** Cancels the goal; this action will stop requesting movement. However,
   *  any currently requested motion (either previously requested by this
   *  action or by another action) will continue to be used. Use an MvrActionStop
   *  action (activate it, or set it at a lower priority) to stop the robot.
   */
  MVREXPORT void cancelGoal(void);

  /// Sets a new goal and sets the action to go there
  MVREXPORT void setGoal(MvrPose goal);

  /// Gets the goal the action has
  MVREXPORT MvrPose getGoal(void) { return myGoal; }

  /// Set the distance which is close enough to the goal (mm);
  MVREXPORT void setCloseDist(double closeDist) { myCloseDist = closeDist; }
  /// Gets the distance which is close enough to the goal (mm)
  MVREXPORT double getCloseDist(void) { return myCloseDist; }
  /// Sets the speed the action will travel to the goal at (mm/sec)
  MVREXPORT void setSpeed(double speed) { mySpeed = speed; }
  /// Gets the speed the action will travel to the goal at (mm/sec)
  MVREXPORT double getSpeed(void) { return mySpeed; }

  /** Called by the action resover; request movement towards goal if we
   *  have one. 
   *  @param currentDesired Current desired action from the resolver
   */
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);

  /** Used by the action resolvel; return current desired action. */
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  MvrPose myGoal;
  double myCloseDist;
  double mySpeed;
  double mySpeedToTurnAt;
  double myDirectionToTurn;
  double myCurTurnDir;
  double myTurnAmount;
  MvrActionDesired myDesired;
  bool myTurnedBack;
  bool myPrinting;
  MvrPose myOldGoal;
  
  enum State
  {
    STATE_NO_GOAL, 
    STATE_ACHIEVED_GOAL,
    STATE_GOING_TO_GOAL
  };
  State myState;
};

#endif // MVRACTIONGOTO
