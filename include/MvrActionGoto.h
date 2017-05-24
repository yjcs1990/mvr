/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionGoto.h
 > Description  : This action goes to a given MvrPose very naively
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONGOTO_H
#define MVRACTIONGOTO_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrAction.h"

class MvrActionGoto : public MvrAction
{
public:
  MVREXPORT MvrActionGoto(const char *name = "goto", 
                          MvrPose goal = MvrPose(0.0, 0.0, 0.0), 
                          double closeDist = 100, double speed = 400,
                          double speedToTurnAt = 150, double turnAmount = 7);
  MVREXPORT virtual ~MvrActionGoto();

  MVREXPORT bool haveAchievedGoal(void);

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
#endif  // MVRACTIONGOTO_H