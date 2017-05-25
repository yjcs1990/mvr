/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSonarAutoDisabler.h
 > Description  : Class for automatically disabling sonar when the robot is stopped
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRSONARAUTODISABLER_H
#define MVRSONARAUTODISABLER_H

/**
   If you create one of this class it will disable the sonar when the
   robot stops moving and then enable the sonar when the robot moves.
   Later this may get more parameters and the ability to be turned on
   and off and things like that (email on aria-users if you want
   them).

   Note that this class assumes it is the only class turning the sonar
   on or off and that the sonar start on.
 **/

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrRobot;

class MvrSonarAutoDisabler
{
public:
  /// Constructor
  MVREXPORT MvrSonarAutoDisabler(MvrRobot *robot);
  /// Destructor
  MVREXPORT virtual ~MvrSonarAutoDisabler();
  /// Supresses this disabler (which turns off the sonar)
  void supress(void) 
  { MvrLog::log(MvrLog::Normal, "MvrSonarAutoDisabler::supress:"); 
    mySupressed = true; }
  /// Gets the callback to supress the autodisabler
  MvrFunctor *getSupressCallback(void) { return &mySupressCB; }
  /// Unsupresses this disabler (goes back to auto enabling/disabling)
  void unsupress(void) 
  { MvrLog::log(MvrLog::Normal, "MvrSonarAutoDisabler::unsupress:"); 
    mySupressed = false; }
  /// Gets the callback to supress the autodisabler
  MvrFunctor *getUnsupressCallback(void) { return &myUnsupressCB; }

  /// Sets that we're autonomous drivign so we only enable some sonar
  void setAutonomousDriving(void) 
  { MvrLog::log(MvrLog::Normal, "MvrSonarAutoDisabler::setAutonomousDriving:"); 
    myAutonomousDriving = true; }
  /// Gets the callback to set that we're driving autonomously
  MvrFunctor *getSetAutonomousDrivingCallback(void) { return &mySetAutonomousDrivingCB; }
  /// Sets that we're driving non-autonomously so we enable all sonar
  void clearAutonomousDriving(void) 
  { MvrLog::log(MvrLog::Normal, "MvrSonarAutoDisabler::clearAutonomousDriving:"); 
    myAutonomousDriving = false; }
  /// Gets the callback to set that we're not driving autonomously
  MvrFunctor *getClearAutonomousDrivingCallback(void) 
  { return &myClearAutonomousDrivingCB; }
protected:
  /// our user task
  MVREXPORT void userTask(void);
  MvrRobot *myRobot;
  MvrTime myLastMoved;
  MvrTime myLastSupressed;
  bool mySupressed;
  bool myAutonomousDriving;

  MvrFunctorC<MvrSonarAutoDisabler> myUserTaskCB;
  MvrFunctorC<MvrSonarAutoDisabler> mySupressCB;
  MvrFunctorC<MvrSonarAutoDisabler> myUnsupressCB;
  MvrFunctorC<MvrSonarAutoDisabler> mySetAutonomousDrivingCB;
  MvrFunctorC<MvrSonarAutoDisabler> myClearAutonomousDrivingCB;
};

#endif // MVRSONARAUTODISABLER_H
