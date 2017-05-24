/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionIRs.h
 > Description  : Action to back up if short-range IR sensors trigger
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONIRS_H
#define MVRACTIONIRS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrRobotParams.h"
#include <vector>

class MvrActionIRs : public MvrAction
{
  /// Constructor
  MvREXPORT MvrActionIRs(const char *name = "IRs", 
		       double backOffSpeed = 100, int backOffTime = 5000,
		       int turnTime = 3000, bool setMaximums = false);
  /// Destructor
  MvREXPORT virtual ~MvrActionIRs();
  MvREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MvREXPORT virtual void setRobot(MvrRobot *robot);
  MvREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MvREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  MvrActionDesired myDesired;
  bool mySetMaximums;
  double myBackOffSpeed;
  int myBackOffTime;
  int myTurnTime;
  int myStopTime;
  bool myFiring;
  double mySpeed;
  double myHeading;
  MvrTime myStartBack;
  MvrTime stoppedSince;
  MvrRobotParams myParams;
  std::vector<int> cycleCounters;
};

#endif  // MVRACTIONIRS_H