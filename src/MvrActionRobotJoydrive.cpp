#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionRobotJoydrive.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"

/**
 * @param name Name for this action
   @param requireDeadmanPushed if true the button must be pushed to drive, 
    if false we'll follow the joystick input no matter what
**/

MVREXPORT MvrActionRobotJoydrive::MvrActionRobotJoydrive(
	const char *name, bool requireDeadmanPushed) :
  MvrAction(name, "This action reads the joystick on the robot and sets the translational and rotational velocities based on this."),
  myHandleJoystickPacketCB(this, &MvrActionRobotJoydrive::handleJoystickPacket),
  myConnectCB(this, &MvrActionRobotJoydrive::connectCallback)
{
  myRequireDeadmanPushed = requireDeadmanPushed;
  setNextArgument(MvrArg("whether to require the deadman to be pushed or not", &myRequireDeadmanPushed, "If this is true then deadman will need to be pushed to drive, if false we'll drive based on the joystick all the time"));
  myDeadZoneLast = false;
  myHandleJoystickPacketCB.setName("MvrActionRobotJoydrive");
}

MVREXPORT MvrActionRobotJoydrive::~MvrActionRobotJoydrive()
{

}

MVREXPORT void MvrActionRobotJoydrive::setRobot(MvrRobot *robot)
{
  MvrAction::setRobot(robot);
  if (myRobot != NULL)
  {
    myRobot->addConnectCB(&myConnectCB);
    myRobot->addPacketHandler(&myHandleJoystickPacketCB);
    if (robot->isConnected())
      connectCallback();
  }
}

MVREXPORT void MvrActionRobotJoydrive::connectCallback(void)
{
  myRobot->comInt(MvrCommands::JOYINFO, 2);
}

MVREXPORT bool MvrActionRobotJoydrive::handleJoystickPacket(
	MvrRobotPacket *packet)
{
  if (packet->getID() != 0xF8)
    return false;
  
  myPacketReceivedTime.setToNow();

  myButton1 = packet->bufToUByte();
  myButton2 = packet->bufToUByte();
  myJoyX = packet->bufToUByte2();
  myJoyY = packet->bufToUByte2();
  myThrottle = packet->bufToUByte2();

  //printf("%d %d %d %d %d\n", myButton1, myButton2, myJoyX, myJoyY, myThrottle);
  return true;
}

MVREXPORT MvrActionDesired *MvrActionRobotJoydrive::fire(MvrActionDesired currentDesired)
{
  bool printing = false;
  myDesired.reset();
  // if we need the deadman to activate and it isn't pushed just bail
  if (myRequireDeadmanPushed && !myButton1)
  {
    if (printing)
      printf("MvrActionRobotJoydrive: Nothing\n");
    myDeadZoneLast = false;
    return NULL;
  }

  // these should vary between 1 and -1
  double ratioRot = -(myJoyX - 512) / 512.0;
  double ratioTrans = (myJoyY - 512) / 512.0;
  double ratioThrottle = myThrottle / 1024.0;
  
  bool doTrans = MvrMath::fabs(ratioTrans) > .33;
  bool doRot = MvrMath::fabs(ratioRot) > .33;

  if (0)
    printf("%.0f %.0f (x %.3f y %.3f throttle %.3f)\n", ratioTrans * ratioThrottle * 1000,
	   ratioRot * ratioThrottle * 50, ratioTrans, ratioRot, ratioThrottle);
  if (!doTrans && !doRot)
  {
    // if the joystick is in the center, we don't need the deadman,
    // and we were stopped lasttime, then just let other stuff go
    if (myDeadZoneLast && !myRequireDeadmanPushed) 
    {
      if (printing)
	printf("MvrActionRobotJoydrive: deadzone Nothing\n");
      return NULL;
    }
    // if the deadman doesn't need to be pushed let something else happen here
    if (printing)
      printf("MvrActionRobotJoydrive: deadzone\n");
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    myDeadZoneLast = true;
    return &myDesired;
  }

  myDeadZoneLast = false;
  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if (doTrans && ((myRobot->getVel() > 0 && ratioTrans < -0.5) || 
		  (myRobot->getVel() < 0 && ratioTrans > 0.5)))
  {
    if (printing)
      printf("MvrActionRobotJoydrive: Decelerating trans more\n");
    myDesired.setTransDecel(myRobot->getTransDecel() * 3);
  }

  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if (doRot && ((myRobot->getRotVel() > 0 && ratioRot < -0.5) || 
		  (myRobot->getRotVel() < 0 && ratioRot > 0.5)))
  {
    if (printing)
      printf("MvrActionRobotJoydrive: Decelerating rot more\n");
    myDesired.setRotDecel(myRobot->getRotDecel() * 3);
  }

  if (doTrans)
    myDesired.setVel(ratioTrans * ratioThrottle * myRobot->getTransVelMax());
  else
    myDesired.setVel(0);

  printf("%.0f %.0f\n", ratioTrans * ratioThrottle * myRobot->getTransVelMax(),
	 ratioRot * ratioThrottle * myRobot->getRotVelMax());

  
  if (doRot)
    myDesired.setRotVel(ratioRot * ratioThrottle * myRobot->getRotVelMax());
  else
    myDesired.setRotVel(0);

  if(printing)
    printf("MvrActionRobotJoydrive: (%ld ms ago) we got %d %d %.2f %.2f %.2f (speed %.0f %.0f)\n", 
	 myPacketReceivedTime.mSecSince(),
	 myButton1, myButton2, ratioTrans, ratioRot, ratioThrottle,
	 myRobot->getVel(), myRobot->getRotVel());
  return &myDesired;
}
