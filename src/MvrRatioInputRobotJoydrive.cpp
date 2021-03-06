#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRatioInputRobotJoydrive.h"
#include "MvrRobot.h"
#include "MvrRobotJoyHandler.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"

/**
   @param robot robot
   @param input Action to attach to and use to drive the robot. 
   @param priority Priority of this joystick input handler with respect to other input
   objects attached to the @a input action object.
   @param requireDeadmanPushed if this is true the joystick "dead man" button must be
   pushed for us to drive. If  this is false we'll follow the
   joystick input no matter what
**/

MVREXPORT MvrRatioInputRobotJoydrive::MvrRatioInputRobotJoydrive(
	MvrRobot *robot, MvrActionRatioInput *input, 
	int priority, bool requireDeadmanPushed) :
  myFireCB(this, &MvrRatioInputRobotJoydrive::fireCallback)
{
  myRobot = robot;
  myInput = input;
  myRequireDeadmanPushed = requireDeadmanPushed;
  myDeadZoneLast = false;
  
  if ((myRobotJoyHandler = Mvria::getRobotJoyHandler()) == NULL)
  {
    myRobotJoyHandler = new MvrRobotJoyHandler(robot);
    Mvria::setRobotJoyHandler(myRobotJoyHandler);
  }

  myFireCB.setName("RobotJoydrive");
  myInput->addFireCallback(priority, &myFireCB);

}

MVREXPORT MvrRatioInputRobotJoydrive::~MvrRatioInputRobotJoydrive()
{

}

MVREXPORT void MvrRatioInputRobotJoydrive::fireCallback(void)
{
  bool printing = false;

  bool button1 = myRobotJoyHandler->getButton1();
  // if we need the deadman to activate and it isn't pushed just bail
  if (myRequireDeadmanPushed && !button1)
  {
    if (printing)
      printf("Nothing\n");
    myDeadZoneLast = false;
    return;
  }

  double rotRatio;
  double transRatio;
  double throttleRatio;

  myRobotJoyHandler->getDoubles(&rotRatio, &transRatio, &throttleRatio);
  
  rotRatio *= 100.0;
  transRatio *= 100.0;
  throttleRatio *= 100.0;

  bool doTrans = true;
  bool doRot = true;

  
  if (!myRequireDeadmanPushed)
  {
    doTrans = MvrMath::fabs(transRatio) > 33;
    doRot = MvrMath::fabs(rotRatio) > 33;
  }

  if (!doTrans && !doRot)
  {
    // if the joystick is in the center, we don't need the deadman,
    // and we were stopped lasttime, then just let other stuff go
    if (myDeadZoneLast && !myRequireDeadmanPushed) 
    {
      if (printing)
	printf("deadzone Nothing\n");
      return;
    }
    // if the deadman doesn't need to be pushed let something else happen here
    if (printing)
      printf("deadzone\n");
    //myInput->setRatios(transRatio, rotRatio, throttleRatio);
    myInput->setRatios(0, 0, throttleRatio);
    myDeadZoneLast = true;
    return;
  }

  myDeadZoneLast = false;
  if (!doRot)
    rotRatio = 0;
  if (!doTrans)
    transRatio = 0;

  if (printing)
    printf("%.0f %.0f %.0f\n", transRatio, rotRatio, throttleRatio);
  
  if (printing)
    printf("(%ld ms ago) we got %d %d %.2f %.2f %.2f (speed %.0f %.0f)\n", 
	   myRobotJoyHandler->getDataReceivedTime().mSecSince(),
	   button1, myRobotJoyHandler->getButton2(), transRatio, rotRatio, 
	   throttleRatio, myRobot->getVel(), myRobot->getRotVel());

  myInput->setRatios(transRatio, rotRatio, throttleRatio);
}
