#ifndef MVRACTIONIRS_H
#define MVRACTIONIRS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrRobotParams.h"
#include <vector>

/// Action to back up if short-range IR sensors trigger
/**
 * If the robot has front-mounted binary (triggered/not triggered) IR sensors, 
 * this action will respond to a sensor trigger by backing up and perhaps
 * turning, similar to bumpers.  This action assumes that if an IR triggers, the
 * robot caused it by moving forward into or under an obstacle, and backing up
 * is a good reaction.

* @ingroup ActionClasses
 */

class MvrActionIRs : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionIRs(const char *name = "IRs", 
		       double backOffSpeed = 100, int backOffTime = 5000,
		       int turnTime = 3000, bool setMaximums = false);
  /// Destructor
  MVREXPORT virtual ~MvrActionIRs();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
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

#endif // ARACTIONIRS
