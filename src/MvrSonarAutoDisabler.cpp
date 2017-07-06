#include "mvriaOSDef.h"
#include "MvrCommands.h"
#include "MvrExport.h"
#include "MvrSonarAutoDisabler.h"
#include "MvrRobot.h"

MVREXPORT MvrSonarAutoDisabler::MvrSonarAutoDisabler(MvrRobot *robot) :
  myUserTaskCB(this, &MvrSonarAutoDisabler::userTask),
  mySupressCB(this, &MvrSonarAutoDisabler::supress),
  myUnsupressCB(this, &MvrSonarAutoDisabler::unsupress),
  mySetAutonomousDrivingCB(this, &MvrSonarAutoDisabler::setAutonomousDriving),
  myClearAutonomousDrivingCB(this, &MvrSonarAutoDisabler::clearAutonomousDriving)
{
  myRobot = robot;
  myLastMoved.setToNow();
  mySupressed = false;
  myAutonomousDriving = false;

  if (!myRobot->isConnected() || myRobot->getNumSonar() > 0)
  {
    myUserTaskCB.setName("SonarAutoDisabler");
    myRobot->addUserTask("SonarAutoDisabler", -50, &myUserTaskCB);
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSonarAutoDisabler not active since there are no sonar");
  }
}

MVREXPORT MvrSonarAutoDisabler::~MvrSonarAutoDisabler()
{
  myRobot->remUserTask("SonarAutoDisabler");
}

MVREXPORT void MvrSonarAutoDisabler::userTask(void)
{
  if (mySupressed && (myRobot->areSonarsEnabled() || 
		      myRobot->areAutonomousDrivingSonarsEnabled()))
  {
    MvrLog::log(MvrLog::Normal, "SonarAutoDisabler: Supression turning off sonar");
    myRobot->disableSonar();
  }
  
  if (mySupressed)
  {
    myLastSupressed.setToNow();
    return;
  }

  /*
  // if it was supressed in the last few cycles, then don't turn the
  // sonar back on yet... this isn't perfect, but looks like it works
  if ((myLastSupressed.mSecSince() < 175 && fabs(myRobot->getVel()) < 200 &&
	  fabs(myRobot->getRotVel()) < 25) ||
      (myLastSupressed.mSecSince() < 500 && fabs(myRobot->getVel()) < 50 &&
       fabs(myRobot->getRotVel()) < 5))
  {
    return;
  }
  */

  //if (myRobot->isTryingToMove() || fabs(myRobot->getVel()) > 10 || 
  //fabs(myRobot->getRotVel()) > 5 || 
  //(myRobot->hasLatVel() && fabs(myRobot->getLatVel()) > 10))
  if (myRobot->isTryingToMove() || !myRobot->isStopped())
  {
    myLastMoved.setToNow();
    // if our sonar are disabled and we moved and our motors are
    // enabled then turn 'em on
    if (!myAutonomousDriving && !myRobot->areSonarsEnabled() && 
	myRobot->areMotorsEnabled())
    {
      MvrLog::log(MvrLog::Normal, 
		 "SonarAutoDisabler: Turning on all sonar (%d %.0f %.0f)",
		 myRobot->isTryingToMove(), fabs(myRobot->getVel()),
		 fabs(myRobot->getRotVel()));
      myRobot->enableSonar();
    }
    // if our sonar are disabled and we moved and our motors are
    // enabled then turn 'em on
    if (myAutonomousDriving && 
	!myRobot->areAutonomousDrivingSonarsEnabled() && 
	myRobot->areMotorsEnabled())
    {
      MvrLog::log(MvrLog::Normal, 
 "SonarAutoDisabler: Turning on sonar for autonomous driving (%d %.0f %.0f)",
		 myRobot->isTryingToMove(), fabs(myRobot->getVel()),
		 fabs(myRobot->getRotVel()));
      myRobot->enableAutonomousDrivingSonar();
    }
  }
  else
  {
    // if the sonar are on and we haven't moved in a while then turn
    // 'em off
    if ((myRobot->areSonarsEnabled() || 
	 myRobot->areAutonomousDrivingSonarsEnabled()) && 
	myLastMoved.mSecSince() > 1000)
    {
      MvrLog::log(MvrLog::Normal, "SonarAutoDisabler: Turning off sonar");
      myRobot->disableSonar();
    }
  }
}
