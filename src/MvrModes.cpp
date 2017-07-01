#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrMode.h"
#include "MvrModes.h"
#include "MvrKeyHandler.h"
#include "MvrSonyPTZ.h"
#include "MvrVCC4.h"
#include "MvrDPPTU.h"
#include "MvrAMPTU.h"
#include "MvrRVisionPTZ.h"
#include "MvrSick.h"
#include "MvrAnalogGyro.h"
#include "MvrRobotConfigPacketReader.h"
#include "mvriaInternal.h"

/** 
  @param robot MvrRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
*/
MVREXPORT MvrModeTeleop::MvrModeTeleop(MvrRobot *robot, const char *name, char key, char key2): 
  MvrMode(robot, name, key, key2),
  myGroup(robot),
  myEnableMotorsCB(robot, &MvrRobot::enableMotors)
{
  myGroup.deactivate();
}

MVREXPORT MvrModeTeleop::~MvrModeTeleop()
{
  
}

MVREXPORT void MvrModeTeleop::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('e', &myEnableMotorsCB);
  myGroup.activateExclusive();
}

MVREXPORT void MvrModeTeleop::deactivate(void)
{
  remKeyHandler(&myEnableMotorsCB);
  if (!baseDeactivate())
    return;
  myGroup.deactivate();
}

MVREXPORT void MvrModeTeleop::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	   "Teleop mode will drive under your joystick or keyboard control.");
  MvrLog::log(MvrLog::Terse, 
	     "It will not allow you to drive into obstacles it can see,");
  MvrLog::log(MvrLog::Terse, 
      "though if you are presistent you may be able to run into something.");
  MvrLog::log(MvrLog::Terse, "For joystick, hold in the trigger button and then move the joystick to drive.");
  MvrLog::log(MvrLog::Terse, "For keyboard control these are the keys and their actions:");
  MvrLog::log(MvrLog::Terse, "%13s:  speed up if forward or no motion, slow down if going backwards", "up arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  slow down if going forwards, speed up if backward or no motion", "down arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  turn left", "left arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  turn right", "right arrow");
  if (myRobot->hasLatVel())
  {
    MvrLog::log(MvrLog::Terse, "%13s:  move left", "z");
    MvrLog::log(MvrLog::Terse, "%13s:  move right", "x");
  }
  MvrLog::log(MvrLog::Terse, "%13s:  stop", "space bar");
  MvrLog::log(MvrLog::Terse, "%13s:  (re)enable motors", "e");
  if (!myRobot->hasLatVel())
    printf("%10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "x", "y", "th", "volts");
  else
    printf("%10s %10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "latVel", "x", "y", "th", "volts");
  if(myRobot->haveStateOfCharge())
    printf(" %10s", "soc");
  printf(" %10s", ""); //flags
  printf("\n");
  fflush(stdout);
}

MVREXPORT void MvrModeTeleop::userTask(void)
{
  if (!myRobot->hasLatVel())
    printf("\r%10.0f %10.0f %10.0f %10.0f %10.1f %10.1f", myRobot->getVel(), 
	   myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	   myRobot->getTh(), myRobot->getRealBatteryVoltage());
  else
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", 
	   myRobot->getVel(), myRobot->getRotVel(), myRobot->getLatVel(),
	   myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	   myRobot->getRealBatteryVoltage());
  if(myRobot->haveStateOfCharge())
    printf(" %9.1f", myRobot->getStateOfCharge());
  if(myRobot->isEStopPressed()) printf(" [ESTOP]"); 
  if(myRobot->isLeftMotorStalled() || myRobot->isRightMotorStalled()) printf(" [STALL] "); 
  if(!myRobot->areMotorsEnabled()) printf(" [DISABLED] "); 
  
  // spaces to cover previous output
  if(!myRobot->isEStopPressed() || !(myRobot->isLeftMotorStalled() && myRobot->isRightMotorStalled()) || myRobot->areMotorsEnabled())
    printf("                 ");

  fflush(stdout);
}

MVREXPORT MvrModeUnguardedTeleop::MvrModeUnguardedTeleop(MvrRobot *robot,
						      const char *name, 
						      char key, char key2): 
  MvrMode(robot, name, key, key2),
  myGroup(robot),
  myEnableMotorsCB(robot, &MvrRobot::enableMotors)
{
  myGroup.deactivate();
}

MVREXPORT MvrModeUnguardedTeleop::~MvrModeUnguardedTeleop()
{
  
}

MVREXPORT void MvrModeUnguardedTeleop::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('e', &myEnableMotorsCB);
  myGroup.activateExclusive();
}

MVREXPORT void MvrModeUnguardedTeleop::deactivate(void)
{
  remKeyHandler(&myEnableMotorsCB);
  if (!baseDeactivate())
    return;
  myGroup.deactivate();
}

MVREXPORT void MvrModeUnguardedTeleop::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	   "Unguarded teleop mode will drive under your joystick or keyboard control.");
  MvrLog::log(MvrLog::Terse, 
	     "\n### THIS MODE IS UNGUARDED AND UNSAFE, BE CAREFUL DRIVING");
  MvrLog::log(MvrLog::Terse,
	     "\nAs it will allow you to drive into things or down stairs.");
  MvrLog::log(MvrLog::Terse, "For joystick, hold in the trigger button and then move the joystick to drive.");
  MvrLog::log(MvrLog::Terse, "For keyboard control these are the keys and their actions:");
  MvrLog::log(MvrLog::Terse, "%13s:  speed up if forward or no motion, slow down if going backwards", "up arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  slow down if going forwards, speed up if backward or no motion", "down arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  turn left", "left arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  turn right", "right arrow");
  if (myRobot->hasLatVel())
  {
    MvrLog::log(MvrLog::Terse, "%13s:  move left", "z");
    MvrLog::log(MvrLog::Terse, "%13s:  move right", "x");
  }
  MvrLog::log(MvrLog::Terse, "%13s:  stop", "space bar");
  MvrLog::log(MvrLog::Terse, "%13s:  (re)enable motors", "e");
  if (!myRobot->hasLatVel())
    printf("%10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "x", "y", "th", "volts");
  else
    printf("%10s %10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "latVel", "x", "y", "th", "volts");
  if(myRobot->haveStateOfCharge())
    printf(" %10s", "soc");
  printf(" %10s", ""); //flags
  printf("\n");
  fflush(stdout);
}

MVREXPORT void MvrModeUnguardedTeleop::userTask(void)
{
  if (!myRobot->hasLatVel())
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", myRobot->getVel(), 
	   myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	   myRobot->getTh(), myRobot->getRealBatteryVoltage());
  else
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", 
	   myRobot->getVel(), myRobot->getRotVel(), myRobot->getLatVel(),
	   myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	   myRobot->getRealBatteryVoltage());
  if(myRobot->haveStateOfCharge())
    printf(" %9.1f", myRobot->getStateOfCharge());
  if(myRobot->isEStopPressed()) printf(" [ESTOP] ");
  if(myRobot->isLeftMotorStalled() || myRobot->isRightMotorStalled()) printf(" [STALL] ");
  if(!myRobot->areMotorsEnabled()) printf(" [DISABLED] ");

  // spaces to cover previous output
  if(!myRobot->isEStopPressed() || !(myRobot->isLeftMotorStalled() && myRobot->isRightMotorStalled()) || myRobot->areMotorsEnabled())
    printf("                 ");

  fflush(stdout);
}

MVREXPORT MvrModeWander::MvrModeWander(MvrRobot *robot, const char *name, char key, char key2): 
  MvrMode(robot, name, key, key2),
  myGroup(robot)
{
  myGroup.deactivate();
}

MVREXPORT MvrModeWander::~MvrModeWander()
{
  
}

MVREXPORT void MvrModeWander::activate(void)
{
  if (!baseActivate())
    return;
  myGroup.activateExclusive();
}

MVREXPORT void MvrModeWander::deactivate(void)
{
  if (!baseDeactivate())
    return;
  myGroup.deactivate(); 
}

MVREXPORT void MvrModeWander::help(void)
{
  MvrLog::log(MvrLog::Terse, "Wander mode will simply drive around forwards until it finds an obstacle,");
  MvrLog::log(MvrLog::Terse, "then it will turn until its clear, and continue.");
  printf("%10s %10s %10s %10s %10s %10s\n", "transVel", "rotVel", "x", "y", "th", "volts");
  fflush(stdout);
}

MVREXPORT void MvrModeWander::userTask(void)
{
  printf("\r%10.0f %10.0f %10.0f %10.0f %10.1f %10.1f", myRobot->getVel(), 
	 myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	 myRobot->getTh(), myRobot->getRealBatteryVoltage());
  fflush(stdout);
}

MVREXPORT MvrModeGripper::MvrModeGripper(MvrRobot *robot, const char *name, 
				      char key, char key2): 
  MvrMode(robot, name, key, key2),
  myGripper(robot),
  myOpenCB(this, &MvrModeGripper::open),
  myCloseCB(this, &MvrModeGripper::close),
  myUpCB(this, &MvrModeGripper::up),
  myDownCB(this, &MvrModeGripper::down),
  myStopCB(this, &MvrModeGripper::stop),
  myExerciseCB(this, &MvrModeGripper::exercise)
{
  myExercising = false;
}

MVREXPORT MvrModeGripper::~MvrModeGripper()
{
  
}

MVREXPORT void MvrModeGripper::activate(void)
{
  if (!baseActivate())
    return;

  addKeyHandler(MvrKeyHandler::UP, &myUpCB);
  addKeyHandler(MvrKeyHandler::DOWN, &myDownCB);
  addKeyHandler(MvrKeyHandler::RIGHT, &myOpenCB);
  addKeyHandler(MvrKeyHandler::LEFT, &myCloseCB);  
  addKeyHandler(MvrKeyHandler::SPACE, &myStopCB);
  addKeyHandler('e', &myExerciseCB);
  addKeyHandler('E', &myExerciseCB);
}

MVREXPORT void MvrModeGripper::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myOpenCB);
  remKeyHandler(&myCloseCB);
  remKeyHandler(&myStopCB);
  remKeyHandler(&myExerciseCB);
}

MVREXPORT void MvrModeGripper::userTask(void)
{
  int val;
  printf("\r");
  if (myGripper.getBreakBeamState() & 2) // outer 
    printf("%13s", "blocked");
  else 
    printf("%13s", "clear");
  if (myGripper.getBreakBeamState() & 1) // inner
    printf("%13s", "blocked");
  else 
    printf("%13s", "clear");
  val = myGripper.getGripState(); // gripper portion
  if (val == 0)
    printf("%13s", "between");
  else if (val == 1)
    printf("%13s", "open");
  else if (val == 2)
    printf("%13s", "closed");
  if (myGripper.isLiftMaxed()) // lift
    printf("%13s", "maxed");
  else
    printf("%13s", "clear");
  val = myGripper.getPaddleState(); // paddle section
  if (val & 1) // left paddle
    printf("%13s", "triggered");
  else
    printf("%13s", "clear");
  if (val & 2) // right paddle
    printf("%13s", "triggered");
  else
    printf("%13s", "clear");
  fflush(stdout);

  // exercise the thing
  if (myExercising)
  {
    switch (myExerState) {
    case UP_OPEN:
      if ((myLastExer.mSecSince() > 3000 && myGripper.isLiftMaxed()) ||
	  myLastExer.mSecSince() > 30000)
      {
	myGripper.gripClose();
	myExerState = UP_CLOSE;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 30000)
	  MvrLog::log(MvrLog::Terse, "\nLift took more than thirty seconds to raise, there is probably a problem with it.\n");
      }
      break;
    case UP_CLOSE:
      if (myGripper.getGripState() == 2 || myLastExer.mSecSince() > 10000)
      {
	myGripper.liftDown();
	myExerState = DOWN_CLOSE;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 10000)
	  MvrLog::log(MvrLog::Terse, "\nGripper took more than 10 seconds to close, there is probably a problem with it.\n");
      }
      break;
    case DOWN_CLOSE:
      if ((myLastExer.mSecSince() > 3000 && myGripper.isLiftMaxed()) ||
	  myLastExer.mSecSince() > 30000)
      {
	myGripper.gripOpen();
	myExerState = DOWN_OPEN;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 30000)
	  MvrLog::log(MvrLog::Terse, "\nLift took more than thirty seconds to raise, there is probably a problem with it.\n");
      }
      break;
    case DOWN_OPEN:
      if (myGripper.getGripState() == 1 || myLastExer.mSecSince() > 10000)
      {
	myGripper.liftUp();
	myExerState = UP_OPEN;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 10000)
	  MvrLog::log(MvrLog::Terse, "\nGripper took more than 10 seconds to open, there is probably a problem with it.\n");
      }
      break;
    }      
    
  }
}

MVREXPORT void MvrModeGripper::open(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripOpen();
}

MVREXPORT void MvrModeGripper::close(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripClose();
}

MVREXPORT void MvrModeGripper::up(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.liftUp();
}

MVREXPORT void MvrModeGripper::down(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.liftDown();
}

MVREXPORT void MvrModeGripper::stop(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripperHalt();
}

MVREXPORT void MvrModeGripper::exercise(void)
{
  if (myExercising == false)
  {
    MvrLog::log(MvrLog::Terse, 
       "\nGripper will now be exercised until another command is given.");
    myExercising = true;
    myExerState = UP_OPEN;
    myGripper.liftUp();
    myGripper.gripOpen();
    myLastExer.setToNow();
  }
}

MVREXPORT void MvrModeGripper::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "Gripper mode will let you control or exercise the gripper.");
  MvrLog::log(MvrLog::Terse, 
      "If you start exercising the gripper it will stop your other commands.");
  MvrLog::log(MvrLog::Terse, 
	     "If you use other commands it will interrupt the exercising.");
  MvrLog::log(MvrLog::Terse, "%13s:  raise lift", "up arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  lower lift", "down arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  close gripper paddles", "left arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  open gripper paddles", "right arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  stop gripper paddles and lift", 
	     "space bar");
  MvrLog::log(MvrLog::Terse, "%13s:  exercise the gripper", "'e' or 'E'");
  MvrLog::log(MvrLog::Terse, "\nGripper status:");
  MvrLog::log(MvrLog::Terse, "%13s%13s%13s%13s%13s%13s", "BB outer", "BB inner",
	     "Paddles", "Lift", "LeftPaddle", "RightPaddle");
  
}



MVREXPORT MvrModeCamera::MvrModeCamera(MvrRobot *robot, const char *name, 
				      char key, char key2): 
  MvrMode(robot, name, key, key2),
  myUpCB(this, &MvrModeCamera::up),
  myDownCB(this, &MvrModeCamera::down),
  myLeftCB(this, &MvrModeCamera::left),
  myRightCB(this, &MvrModeCamera::right),
  myCenterCB(this, &MvrModeCamera::center),
  myZoomInCB(this, &MvrModeCamera::zoomIn),  
  myZoomOutCB(this, &MvrModeCamera::zoomOut),
  myExerciseCB(this, &MvrModeCamera::exercise),
  mySonyCB(this, &MvrModeCamera::sony),
  myCanonCB(this, &MvrModeCamera::canon),
  myDpptuCB(this, &MvrModeCamera::dpptu),
  myAmptuCB(this, &MvrModeCamera::amptu),
  myCanonInvertedCB(this, &MvrModeCamera::canonInverted),
  mySonySerialCB(this, &MvrModeCamera::sonySerial),
  myCanonSerialCB(this, &MvrModeCamera::canonSerial),
  myDpptuSerialCB(this, &MvrModeCamera::dpptuSerial),
  myAmptuSerialCB(this, &MvrModeCamera::amptuSerial),
  myCanonInvertedSerialCB(this, &MvrModeCamera::canonInvertedSerial),
  myRVisionSerialCB(this, &MvrModeCamera::rvisionSerial),
  myCom1CB(this, &MvrModeCamera::com1),
  myCom2CB(this, &MvrModeCamera::com2),
  myCom3CB(this, &MvrModeCamera::com3),
  myCom4CB(this, &MvrModeCamera::com4),
  myUSBCom0CB(this, &MvrModeCamera::usb0),
  myUSBCom9CB(this, &MvrModeCamera::usb9),
  myAux1CB(this, &MvrModeCamera::aux1),
  myAux2CB(this, &MvrModeCamera::aux2),
  myPanAmount(5),
  myTiltAmount(3),
  myAutoFocusOn(true),
  myToggleAutoFocusCB(this, &MvrModeCamera::toggleAutoFocus)
{
  myState = STATE_CAMERA;
  myExercising = false;
}

MVREXPORT MvrModeCamera::~MvrModeCamera()
{
  
}

MVREXPORT void MvrModeCamera::activate(void)
{
  MvrKeyHandler *keyHandler;
  if (!baseActivate())
    return;
  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse,"MvrModeCamera::activate: There should already be a key handler, but there isn't... mode won't work");
    return;
  }
  if (myState == STATE_CAMERA)
    takeCameraKeys();
  else if (myState == STATE_PORT)
    takePortKeys();
  else if (myState == STATE_MOVEMENT)
    takeMovementKeys();
  else
    MvrLog::log(MvrLog::Terse,"MvrModeCamera in bad state.");
  
}

MVREXPORT void MvrModeCamera::deactivate(void)
{
  if (!baseDeactivate())
    return;
  if (myState == STATE_CAMERA)
    giveUpCameraKeys();
  else if (myState == STATE_PORT)
    giveUpPortKeys();
  else if (myState == STATE_MOVEMENT)
    giveUpMovementKeys();
  else
    MvrLog::log(MvrLog::Terse,"MvrModeCamera in bad state.");
}

MVREXPORT void MvrModeCamera::userTask(void)
{
  if (myExercising && myCam != NULL && myLastExer.mSecSince() > 7000)
  {
    switch (myExerState) {
    case CENTER:
      myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxPosTilt());
      myExerState = UP_LEFT;
      myLastExer.setToNow();
      break;
    case UP_LEFT:
      myCam->panTilt(myCam->getMaxPosPan(), myCam->getMaxPosTilt());
      myExerState = UP_RIGHT;
      myLastExer.setToNow();
      break;
    case UP_RIGHT:
      myCam->panTilt(myCam->getMaxPosPan(), myCam->getMaxNegTilt());
      myExerState = DOWN_RIGHT;
      myLastExer.setToNow();
      break;
    case DOWN_RIGHT:
      myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxNegTilt());
      myExerState = DOWN_LEFT;
      myLastExer.setToNow();
      break;
    case DOWN_LEFT:
      myCam->panTilt(0, 0);
      myExerState = CENTER;
      myLastExer.setToNow();
      break;
    }      
  }
  if (myExercising && myCam != NULL && myCam->canZoom() && 
      myLastExerZoomed.mSecSince() > 35000)
  {
    if (myExerZoomedIn)
      myCam->zoom(myCam->getMinZoom());
    else
      myCam->zoom(myCam->getMaxZoom());
    myLastExerZoomed.setToNow();
  }
}

MVREXPORT void MvrModeCamera::left(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panRel(-myPanAmount);
}

MVREXPORT void MvrModeCamera::right(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panRel(myPanAmount);
}

MVREXPORT void MvrModeCamera::up(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->tiltRel(myTiltAmount);
}

MVREXPORT void MvrModeCamera::down(void)
{  
  if (myExercising == true)
    myExercising = false;
  myCam->tiltRel(-myTiltAmount);
}

MVREXPORT void MvrModeCamera::center(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panTilt(0, 0);
  myCam->zoom(myCam->getMinZoom());
}

MVREXPORT void MvrModeCamera::exercise(void)
{
  if (myExercising == false)
  {
    MvrLog::log(MvrLog::Terse, 
       "Camera will now be exercised until another command is given.");
    myExercising = true;
    myExerState = UP_LEFT;
    myLastExer.setToNow();
    myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxPosTilt());
    myLastExerZoomed.setToNow();
    myExerZoomedIn = true;
    if (myCam->canZoom())
      myCam->zoom(myCam->getMaxZoom());
  }
}

MVREXPORT void MvrModeCamera::toggleAutoFocus()
{
  MvrLog::log(MvrLog::Terse, "Turning autofocus %s", myAutoFocusOn?"off":"on");
  if(myCam->setAutoFocus(!myAutoFocusOn))
    myAutoFocusOn = !myAutoFocusOn;
}

MVREXPORT void MvrModeCamera::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "Camera mode will let you control or exercise the camera.");
  MvrLog::log(MvrLog::Terse, 
      "If you start exercising the camera it will stop your other commands.");
  if (myState == STATE_CAMERA)
    helpCameraKeys();
  else if (myState == STATE_PORT)
    helpPortKeys();
  else if (myState == STATE_MOVEMENT)
    helpMovementKeys();
  else
    MvrLog::log(MvrLog::Terse, "Something is horribly wrong and mode camera is in no state.");
}

MVREXPORT void MvrModeCamera::zoomIn(void)
{
  if (myCam->canZoom())
  {
    myCam->zoom(myCam->getZoom() + 
	 MvrMath::roundInt((myCam->getMaxZoom() - myCam->getMinZoom()) * .01));
  }
}

MVREXPORT void MvrModeCamera::zoomOut(void)
{
  if (myCam->canZoom())
  {
    myCam->zoom(myCam->getZoom() - 
	MvrMath::roundInt((myCam->getMaxZoom() - myCam->getMinZoom()) * .01));
  }
}

MVREXPORT void MvrModeCamera::sony(void)
{
  myCam = new MvrSonyPTZ(myRobot);
  MvrLog::log(MvrLog::Terse, "\nSony selected, now need to select the aux port.");
  cameraToAux();
}

MVREXPORT void MvrModeCamera::canon(void)
{
  myCam = new MvrVCC4(myRobot);
  MvrLog::log(MvrLog::Terse, "\nCanon selected, now need to select the aux port.");
  cameraToAux();
}

MVREXPORT void MvrModeCamera::dpptu(void)
{
  myCam = new MvrDPPTU(myRobot);
  MvrLog::log(MvrLog::Terse, "\nDPPTU selected, now need to select the aux port.");
  cameraToAux();
}

MVREXPORT void MvrModeCamera::amptu(void)
{
  myCam = new MvrAMPTU(myRobot);
  MvrLog::log(MvrLog::Terse, 
	     "\nActivMedia Pan Tilt Unit selected, now need to select the aux port.");
  cameraToAux();
}

MVREXPORT void MvrModeCamera::canonInverted(void)
{
  myCam = new MvrVCC4(myRobot, true);
  MvrLog::log(MvrLog::Terse, "\nInverted Canon selected, now need to select the aux port.");
  cameraToAux();
}

MVREXPORT void MvrModeCamera::sonySerial(void)
{
  myCam = new MvrSonyPTZ(myRobot);
  MvrLog::log(MvrLog::Terse, "\nSony selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::canonSerial(void)
{
  myCam = new MvrVCC4(myRobot);
  MvrLog::log(MvrLog::Terse, 
	     "\nCanon VCC4 selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::dpptuSerial(void)
{
  myCam = new MvrDPPTU(myRobot);
  MvrLog::log(MvrLog::Terse, "\nDPPTU selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::amptuSerial(void)
{
  myCam = new MvrAMPTU(myRobot);
  MvrLog::log(MvrLog::Terse, "\nAMPTU selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::canonInvertedSerial(void)
{
  myCam = new MvrVCC4(myRobot, true);
  MvrLog::log(MvrLog::Terse, 
	     "\nInverted Canon VCC4 selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::rvisionSerial(void)
{
  myCam = new MvrRVisionPTZ(myRobot);
  MvrLog::log(MvrLog::Terse, "\nRVision selected, now need to select serial port.");
  cameraToPort();
}

MVREXPORT void MvrModeCamera::com1(void)
{
  myConn.setPort(MvrUtil::COM1);
  portToMovement();
}

MVREXPORT void MvrModeCamera::com2(void)
{
  myConn.setPort(MvrUtil::COM2);
  portToMovement();
}

MVREXPORT void MvrModeCamera::com3(void)
{
  myConn.setPort(MvrUtil::COM3);
  portToMovement();
}

MVREXPORT void MvrModeCamera::com4(void)
{
  myConn.setPort(MvrUtil::COM4);
  portToMovement();
}

MVREXPORT void MvrModeCamera::usb0(void)
{
  myConn.setPort("/dev/ttyUSB0");
  portToMovement();
}

MVREXPORT void MvrModeCamera::usb9(void)
{
  myConn.setPort("/dev/ttyUSB9");
  portToMovement();
}

MVREXPORT void MvrModeCamera::aux1(void)
{
  myCam->setAuxPort(1);
  auxToMovement();
}
MVREXPORT void MvrModeCamera::aux2(void)
{
  myCam->setAuxPort(2);
  auxToMovement();
}

void MvrModeCamera::cameraToMovement(void)
{
  myState = STATE_MOVEMENT;
  myCam->init();
  myRobot->setPTZ(myCam);
  giveUpCameraKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void MvrModeCamera::cameraToPort(void)
{
  myState = STATE_PORT;
  giveUpCameraKeys();
  takePortKeys();
  helpPortKeys();
}

void MvrModeCamera::cameraToAux(void)
{
  giveUpCameraKeys();
  takeAuxKeys();
  helpAuxKeys();
}

void MvrModeCamera::portToMovement(void)
{
  MvrLog::log(MvrLog::Normal, "MvrModeCamera: Opening connection to camera on port %s", myConn.getPortName());
  if (!myConn.openSimple())
  {
    MvrLog::log(MvrLog::Terse, 
	       "\n\nMvrModeCamera: Could not open camera on that port, try another port.\n");
    helpPortKeys();
    return;
  }
  if(!myCam->setDeviceConnection(&myConn))
  {
    MvrLog::log(MvrLog::Terse, "\n\nMvrModeCamera: Error setting device connection!\n");
    return;
  }
  myCam->init();
  myRobot->setPTZ(myCam);
  myState = STATE_MOVEMENT;
  giveUpPortKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void MvrModeCamera::auxToMovement(void)
{
  myCam->init();
  myRobot->setPTZ(myCam);
  myState = STATE_MOVEMENT;
  giveUpAuxKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void MvrModeCamera::takeCameraKeys(void)
{
  addKeyHandler('1', &mySonyCB);
  addKeyHandler('2', &myCanonCB);
  addKeyHandler('3', &myDpptuCB);
  addKeyHandler('4', &myAmptuCB);
  addKeyHandler('5', &myCanonInvertedCB);
  addKeyHandler('!', &mySonySerialCB);
  addKeyHandler('@', &myCanonSerialCB);
  addKeyHandler('#', &myDpptuSerialCB);
  addKeyHandler('$', &myAmptuSerialCB);
  addKeyHandler('%', &myCanonInvertedSerialCB);
  addKeyHandler('^', &myRVisionSerialCB);
}

void MvrModeCamera::giveUpCameraKeys(void)
{
  remKeyHandler(&myCanonCB);
  remKeyHandler(&mySonyCB);
  remKeyHandler(&myDpptuCB);
  remKeyHandler(&myAmptuCB);
  remKeyHandler(&myCanonInvertedCB);
  remKeyHandler(&mySonySerialCB);
  remKeyHandler(&myCanonSerialCB);
  remKeyHandler(&myDpptuSerialCB);
  remKeyHandler(&myAmptuSerialCB);
  remKeyHandler(&myCanonInvertedSerialCB);
  remKeyHandler(&myRVisionSerialCB);
}

void MvrModeCamera::helpCameraKeys(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "You now need to select what type of camera you have.");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a SONY PTZ camera attached to the robot", "'1'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a Canon VCC4 camera attached to the robot", "'2'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a DPPTU camera attached to the robot", "'3'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select an AMPTU camera attached to the robot", "'4'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select an inverted Canon VCC4 camera attached to the robot", "'5'");

  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a SONY PTZ camera attached to a serial port", 
	     "'!'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a Canon VCC4 camera attached to a serial port", 
	     "'@'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select a DPPTU camera attached to a serial port",
	     "'#'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select an AMPTU camera attached to a serial port", 
	     "'$'");
  MvrLog::log(MvrLog::Terse, 
	     "%13s: select an inverted Canon VCC4 camera attached to a serial port", 
	     "'%'");
  MvrLog::log(MvrLog::Terse,
	     "%13s: select an RVision camera attached to a serial port",
	     "'^'");
}

void MvrModeCamera::takePortKeys(void)
{
  addKeyHandler('1', &myCom1CB);
  addKeyHandler('2', &myCom2CB);
  addKeyHandler('3', &myCom3CB);
  addKeyHandler('4', &myCom4CB);
  addKeyHandler('5', &myUSBCom0CB);
  addKeyHandler('6', &myUSBCom9CB);
}

void MvrModeCamera::giveUpPortKeys(void)
{
  remKeyHandler(&myCom1CB);
  remKeyHandler(&myCom2CB);
  remKeyHandler(&myCom3CB);
  remKeyHandler(&myCom4CB);
  remKeyHandler(&myUSBCom0CB);
  remKeyHandler(&myUSBCom9CB);
}

void MvrModeCamera::helpPortKeys(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "You now need to select what port your camera is on.");
  MvrLog::log(MvrLog::Terse, "%13s:  select COM1 or /dev/ttyS0", "'1'");
  MvrLog::log(MvrLog::Terse, "%13s:  select COM2 or /dev/ttyS1", "'2'");
  MvrLog::log(MvrLog::Terse, "%13s:  select COM3 or /dev/ttyS2", "'3'");
  MvrLog::log(MvrLog::Terse, "%13s:  select COM4 or /dev/ttyS3", "'4'");
  MvrLog::log(MvrLog::Terse, "%13s:  select /dev/ttyUSB0", "'5'");
  MvrLog::log(MvrLog::Terse, "%13s:  select /dev/ttyUSB9", "'6'");
}

void MvrModeCamera::takeAuxKeys(void)
{
  addKeyHandler('1', &myAux1CB);
  addKeyHandler('2', &myAux2CB);
}

void MvrModeCamera::giveUpAuxKeys(void)
{
  remKeyHandler(&myAux1CB);
  remKeyHandler(&myAux2CB);
}

void MvrModeCamera::helpAuxKeys(void)
{
  MvrLog::log(MvrLog::Terse,
             "You now need to select what aux port your camera is on.");
  MvrLog::log(MvrLog::Terse, "%13s:  select AUX1", "'1'");
  MvrLog::log(MvrLog::Terse, "%13s:  select AUX2", "'2'");
}

void MvrModeCamera::takeMovementKeys(void)
{
  addKeyHandler(MvrKeyHandler::UP, &myUpCB);
  addKeyHandler(MvrKeyHandler::DOWN, &myDownCB);
  addKeyHandler(MvrKeyHandler::LEFT, &myLeftCB);
  addKeyHandler(MvrKeyHandler::RIGHT, &myRightCB);
  addKeyHandler(MvrKeyHandler::SPACE, &myCenterCB);
  addKeyHandler('e', &myExerciseCB);
  addKeyHandler('E', &myExerciseCB);
  if (myCam->canZoom())
  {
    addKeyHandler('z', &myZoomInCB);
    addKeyHandler('Z', &myZoomInCB);
    addKeyHandler('x', &myZoomOutCB);
    addKeyHandler('X', &myZoomOutCB);
  }
  addKeyHandler('f', &myToggleAutoFocusCB);
  addKeyHandler('F', &myToggleAutoFocusCB);
}

void MvrModeCamera::giveUpMovementKeys(void)
{
  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myLeftCB);
  remKeyHandler(&myRightCB);
  remKeyHandler(&myCenterCB);
  remKeyHandler(&myExerciseCB);
  if (myCam->canZoom())
  {
    remKeyHandler(&myZoomInCB);
    remKeyHandler(&myZoomOutCB);
  }
  remKeyHandler(&myToggleAutoFocusCB);
}

void MvrModeCamera::helpMovementKeys(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "Camera mode will now let you move the camera.");
  MvrLog::log(MvrLog::Terse, "%13s:  tilt camera up by %d", "up arrow", myTiltAmount);
  MvrLog::log(MvrLog::Terse, "%13s:  tilt camera down by %d", "down arrow", myTiltAmount);
  MvrLog::log(MvrLog::Terse, "%13s:  pan camera left by %d", "left arrow", myPanAmount);
  MvrLog::log(MvrLog::Terse, "%13s:  pan camera right by %d", "right arrow", myPanAmount);
  MvrLog::log(MvrLog::Terse, "%13s:  center camera and zoom out", 
	     "space bar");
  MvrLog::log(MvrLog::Terse, "%13s:  exercise the camera", "'e' or 'E'");
  if (myCam->canZoom())
  {
    MvrLog::log(MvrLog::Terse, "%13s:  zoom in", "'z' or 'Z'");
    MvrLog::log(MvrLog::Terse, "%13s:  zoom out", "'x' or 'X'");
  }
  MvrLog::log(MvrLog::Terse, "%13s:  toggle auto/fixed focus", "'f' or 'F'");
}

MVREXPORT MvrModeSonar::MvrModeSonar(MvrRobot *robot, const char *name, char key,
				  char key2) :
  MvrMode(robot, name, key, key2),
  myAllSonarCB(this, &MvrModeSonar::allSonar),
  myFirstSonarCB(this, &MvrModeSonar::firstSonar),
  mySecondSonarCB(this, &MvrModeSonar::secondSonar),
  myThirdSonarCB(this, &MvrModeSonar::thirdSonar),
  myFourthSonarCB(this, &MvrModeSonar::fourthSonar)
{
  myState = STATE_FIRST;
}

MVREXPORT MvrModeSonar::~MvrModeSonar()
{

}

MVREXPORT void MvrModeSonar::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('1', &myAllSonarCB);
  addKeyHandler('2', &myFirstSonarCB);
  addKeyHandler('3', &mySecondSonarCB);
  addKeyHandler('4', &myThirdSonarCB);
  addKeyHandler('5', &myFourthSonarCB);
}

MVREXPORT void MvrModeSonar::deactivate(void)
{
  if (!baseDeactivate())
    return;
  remKeyHandler(&myAllSonarCB);
  remKeyHandler(&myFirstSonarCB);
  remKeyHandler(&mySecondSonarCB);
  remKeyHandler(&myThirdSonarCB);
  remKeyHandler(&myFourthSonarCB);
}

MVREXPORT void MvrModeSonar::help(void)
{
  int i;
  MvrLog::log(MvrLog::Terse, "This mode displays different segments of sonar.");
  MvrLog::log(MvrLog::Terse, 
	     "You can use these keys to switch what is displayed:");
  MvrLog::log(MvrLog::Terse, "%13s: display all sonar", "'1'");
  MvrLog::log(MvrLog::Terse, "%13s: display sonar 0 - 7", "'2'");
  MvrLog::log(MvrLog::Terse, "%13s: display sonar 8 - 15", "'3'");
  MvrLog::log(MvrLog::Terse, "%13s: display sonar 16 - 23", "'4'");
  MvrLog::log(MvrLog::Terse, "%13s: display sonar 24 - 31", "'5'");
  MvrLog::log(MvrLog::Terse, "Sonar readings:");
  if (myState == STATE_ALL)
  {
    MvrLog::log(MvrLog::Terse, "Displaying all sonar.");
    for (i = 0; i < myRobot->getNumSonar(); ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_FIRST)
  {
    MvrLog::log(MvrLog::Terse, "Displaying 0-7 sonar.");
    for (i = 0; i < myRobot->getNumSonar() && i <= 7; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_SECOND)
  {
    MvrLog::log(MvrLog::Terse, "Displaying 8-15 sonar.");
    for (i = 8; i < myRobot->getNumSonar() && i <= 15; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_THIRD)
  {
    MvrLog::log(MvrLog::Terse, "Displaying 16-23 sonar.");
    for (i = 16; i < myRobot->getNumSonar() && i <= 23; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_FOURTH)
  {
    MvrLog::log(MvrLog::Terse, "Displaying 24-31 sonar.");
    for (i = 24; i < myRobot->getNumSonar() && i <= 31; ++i)
      printf("%6d", i); 
  }
  printf("\n");
}

MVREXPORT void MvrModeSonar::userTask(void)
{
  int i;
  printf("\r");
  if (myState == STATE_ALL)
  {
    for (i = 0; i < myRobot->getNumSonar(); ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  else if (myState == STATE_FIRST)
  {
     for (i = 0; i < myRobot->getNumSonar() && i <= 7; ++i)
      printf("%6d", myRobot->getSonarRange(i));
  }
  else if (myState == STATE_SECOND)
  {
     for (i = 8; i < myRobot->getNumSonar() && i <= 15; ++i)
      printf("%6d", myRobot->getSonarRange(i));
  }
  else if (myState == STATE_THIRD)
  {
    for (i = 16; i < myRobot->getNumSonar() && i <= 23; ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  else if (myState == STATE_FOURTH)
  {
    for (i = 24; i < myRobot->getNumSonar() && i <= 31; ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  fflush(stdout);
}

MVREXPORT void MvrModeSonar::allSonar(void)
{
  myState = STATE_ALL;
  printf("\n");
  help();
}

MVREXPORT void MvrModeSonar::firstSonar(void)
{
  myState = STATE_FIRST;
  printf("\n");
  help();
}

MVREXPORT void MvrModeSonar::secondSonar(void)
{
  myState = STATE_SECOND;
  printf("\n");
  help();
}

MVREXPORT void MvrModeSonar::thirdSonar(void)
{
  myState = STATE_THIRD;
  printf("\n");
  help();
}

MVREXPORT void MvrModeSonar::fourthSonar(void)
{
  myState = STATE_FOURTH;
  printf("\n");
  help();
}

MVREXPORT MvrModeBumps::MvrModeBumps(MvrRobot *robot, const char *name, char key, char key2): 
  MvrMode(robot, name, key, key2)
{
}

MVREXPORT MvrModeBumps::~MvrModeBumps()
{
  
}

MVREXPORT void MvrModeBumps::activate(void)
{
  if (!baseActivate())
    return;
}

MVREXPORT void MvrModeBumps::deactivate(void)
{
  if (!baseDeactivate())
    return;
}

MVREXPORT void MvrModeBumps::help(void)
{
  unsigned int i;
  MvrLog::log(MvrLog::Terse, "Bumps mode will display whether bumpers are triggered or not...");
  MvrLog::log(MvrLog::Terse, "keep in mind it is assuming you have a full bump ring... so you should");
  MvrLog::log(MvrLog::Terse, "ignore readings for where there aren't bumpers.");
  MvrLog::log(MvrLog::Terse, "Bumper readings:");
  for (i = 0; i < myRobot->getNumFrontBumpers(); i++)
  {
    printf("%6d", i + 1);
  }
  printf(" |");
  for (i = 0; i < myRobot->getNumRearBumpers(); i++)
  {
    printf("%6d", i + 1);
  }
  printf("\n");
}

MVREXPORT void MvrModeBumps::userTask(void)
{
  unsigned int i;
  int val;
  int bit;
  if (myRobot == NULL)
    return;
  printf("\r");
  val = ((myRobot->getStallValue() & 0xff00) >> 8);
  for (i = 0, bit = 2; i < myRobot->getNumFrontBumpers(); i++, bit *= 2)
  {
    if (val & bit)
      printf("%6s", "trig");
    else
      printf("%6s", "clear");
  }
  printf(" |");
  val = ((myRobot->getStallValue() & 0xff));
  for (i = 0, bit = 2; i < myRobot->getNumRearBumpers(); i++, bit *= 2)
  {
    if (val & bit)
      printf("%6s", "trig");
    else
      printf("%6s", "clear");
  }

}

MVREXPORT MvrModePosition::MvrModePosition(MvrRobot *robot, const char *name, char key, char key2, MvrAnalogGyro *gyro): 
  MvrMode(robot, name, key, key2),
  myUpCB(this, &MvrModePosition::up),
  myDownCB(this, &MvrModePosition::down),
  myLeftCB(this, &MvrModePosition::left),
  myRightCB(this, &MvrModePosition::right),
  myStopCB(this, &MvrModePosition::stop),
  myResetCB(this, &MvrModePosition::reset),
  myModeCB(this, &MvrModePosition::mode),
  myGyroCB(this, &MvrModePosition::gyro),
  myIncDistCB(this, &MvrModePosition::incDistance),
  myDecDistCB(this, &MvrModePosition::decDistance)
{
  myGyro = gyro;
  myMode = MODE_BOTH;
  myModeString = "both";
  myInHeadingMode = false;
  myDistance = 1000;

  if (myGyro != NULL && !myGyro->hasNoInternalData())
    myGyroZero = myGyro->getHeading();
  myRobotZero = myRobot->getRawEncoderPose().getTh();
  myInHeadingMode = true;
  myHeading = myRobot->getTh();
}

MVREXPORT MvrModePosition::~MvrModePosition()
{
  
}

MVREXPORT void MvrModePosition::activate(void)
{
  if (!baseActivate())
    return;

  addKeyHandler(MvrKeyHandler::UP, &myUpCB);
  addKeyHandler(MvrKeyHandler::DOWN, &myDownCB);
  addKeyHandler(MvrKeyHandler::LEFT, &myLeftCB);  
  addKeyHandler(MvrKeyHandler::RIGHT, &myRightCB);
  addKeyHandler(MvrKeyHandler::SPACE, &myStopCB);
  addKeyHandler(MvrKeyHandler::PAGEUP, &myIncDistCB);
  addKeyHandler(MvrKeyHandler::PAGEDOWN, &myDecDistCB);
  addKeyHandler('r', &myResetCB);
  addKeyHandler('R', &myResetCB);
  addKeyHandler('x', &myModeCB);
  addKeyHandler('X', &myModeCB);
  addKeyHandler('z', &myGyroCB);
  addKeyHandler('Z', &myGyroCB);
}

MVREXPORT void MvrModePosition::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myLeftCB);
  remKeyHandler(&myRightCB);
  remKeyHandler(&myStopCB);
  remKeyHandler(&myResetCB);
  remKeyHandler(&myModeCB);
  remKeyHandler(&myGyroCB);
  remKeyHandler(&myIncDistCB);
  remKeyHandler(&myDecDistCB);
}

MVREXPORT void MvrModePosition::up(void)
{
  myRobot->move(myDistance);
  if (myInHeadingMode)
  {
    myInHeadingMode = false;
    myHeading = myRobot->getTh();
  }
}

MVREXPORT void MvrModePosition::down(void)
{
  myRobot->move(-myDistance);
  if (myInHeadingMode)
  {
    myInHeadingMode = false;
    myHeading = myRobot->getTh();
  }
}

MVREXPORT void MvrModePosition::incDistance(void)
{
  myDistance += 500;
  puts("\n");
  help();
}

MVREXPORT void MvrModePosition::decDistance(void)
{
  myDistance -= 500;
  if(myDistance < 500) myDistance = 500;
  puts("\n");
  help();
}

MVREXPORT void MvrModePosition::left(void)
{
  myRobot->setDeltaHeading(90);
  myInHeadingMode = true;
}

MVREXPORT void MvrModePosition::right(void)
{
  myRobot->setDeltaHeading(-90);
  myInHeadingMode = true;
}

MVREXPORT void MvrModePosition::stop(void)
{
  myRobot->stop();
  myInHeadingMode = true;
}

MVREXPORT void MvrModePosition::reset(void)
{
  myRobot->stop();
  myRobot->moveTo(MvrPose(0, 0, 0));
  if (myGyro != NULL && !myGyro->hasNoInternalData())
    myGyroZero = myGyro->getHeading();
  myRobotZero = myRobot->getRawEncoderPose().getTh();
  myInHeadingMode = true;
  myHeading = myRobot->getTh();
}

MVREXPORT void MvrModePosition::mode(void)
{
  if (myMode == MODE_BOTH)
  {
    myMode = MODE_EITHER;
    myModeString = "either";
    myInHeadingMode = true;
    myRobot->stop();
  }
  else if (myMode == MODE_EITHER)
  {
    myMode = MODE_BOTH;
    myModeString = "both";
  }
}

MVREXPORT void MvrModePosition::gyro(void)
{
  if (myGyro == NULL || !myGyro->haveGottenData())
    return;

  if (myGyro != NULL && myGyro->isActive())
    myGyro->deactivate();
  else if (myGyro != NULL && !myGyro->isActive() && 
	   myGyro->hasGyroOnlyMode() && !myGyro->isGyroOnlyActive())
    myGyro->activateGyroOnly();
  else if (myGyro != NULL && !myGyro->isActive())
    myGyro->activate();

  help();
}

MVREXPORT void MvrModePosition::help(void)
{
  MvrLog::log(MvrLog::Terse, "Mode is one of two values:");
  MvrLog::log(MvrLog::Terse, "%13s: heading and move can happen simultaneously", 
	     "both");
  MvrLog::log(MvrLog::Terse, "%13s: only heading or move is active (move holds heading)", "either");
  MvrLog::log(MvrLog::Terse, "");
  MvrLog::log(MvrLog::Terse, "%13s:  forward %.1f meter(s)", "up arrow", myDistance/1000.0);
  MvrLog::log(MvrLog::Terse, "%13s:  backward %.1f meter(s)", "down arrow", myDistance/1000.0);
  MvrLog::log(MvrLog::Terse, "%13s:  increase distance by 1/2 meter", "page up");
  MvrLog::log(MvrLog::Terse, "%13s:  decrease distance by 1/2 meter", "page down");
  MvrLog::log(MvrLog::Terse, "%13s:  turn left 90 degrees", "left arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  turn right 90 degrees", "right arrow");
  MvrLog::log(MvrLog::Terse, "%13s:  stop", "space bar");
  MvrLog::log(MvrLog::Terse, "%13s:  reset ARIA position to (0, 0, 0)", "'r' or 'R'");
  MvrLog::log(MvrLog::Terse, "%13s:  switch heading/velocity mode","'x' or 'X'");
  if (myGyro != NULL && myGyro->haveGottenData() && !myGyro->hasGyroOnlyMode())
    MvrLog::log(MvrLog::Terse, "%13s:  turn gyro on or off (stays this way in other modes)","'z' or 'Z'");
  if (myGyro != NULL && myGyro->haveGottenData() && myGyro->hasGyroOnlyMode())
    MvrLog::log(MvrLog::Terse, "%13s:  turn gyro on or off or gyro only (stays this way in other modes)","'z' or 'Z'");

  MvrLog::log(MvrLog::Terse, "");
  MvrLog::log(MvrLog::Terse, "Position mode shows the position stats on a robot.");
  if (myGyro != NULL && myGyro->haveGottenData() && 
      !myGyro->hasNoInternalData())
    MvrLog::log(MvrLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%6s%10s%10s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "gyro", "gyro_th", "robot_th", "raw");
  else if (myGyro != NULL && myGyro->haveGottenData() && 
	   myGyro->hasNoInternalData())
    MvrLog::log(MvrLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%6s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "gyro", "raw");
  else
    MvrLog::log(MvrLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "raw");

  
}

MVREXPORT void MvrModePosition::userTask(void)
{
  if (myRobot == NULL)
    return;
  // if we're in either mode and not in the heading mode try to keep the
  // same heading (in heading mode its controlled by those commands)
  if (myMode == MODE_EITHER && !myInHeadingMode)
  {
    myRobot->setHeading(myHeading);
  }
  double voltage;
  if (myRobot->getRealBatteryVoltage() > 0)
    voltage = myRobot->getRealBatteryVoltage();
  else
    voltage = myRobot->getBatteryVoltage();

  std::string gyroString;
  if (myGyro == NULL)
    gyroString = "none";
  else if (myGyro->isActive())
    gyroString = "on";
  else if (myGyro->hasGyroOnlyMode() && myGyro->isGyroOnlyActive())
    gyroString = "only";
  else
    gyroString = "off";

  MvrPose raw = myRobot->getRawEncoderPose();

  if (myGyro != NULL && myGyro->haveGottenData() && 
      !myGyro->hasNoInternalData())
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%6s%10.2f%10.2f %10.2f,%.2f,%.2f", 
	   myRobot->getX(),  myRobot->getY(), myRobot->getTh(), 
	   myRobot->getCompass(), voltage,
	   myRobot->getMotorPacCount(),
	   myMode == MODE_BOTH ? "both" : "either", 
	   gyroString.c_str(),
	   MvrMath::subAngle(myGyro->getHeading(), myGyroZero), 
	   MvrMath::subAngle(myRobot->getRawEncoderPose().getTh(),myRobotZero),
	   raw.getX(), raw.getY(), raw.getTh()
	);
  else if (myGyro != NULL && myGyro->haveGottenData() && 
      myGyro->hasNoInternalData())
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%6s%10.2f,%.2f,%.2f", 
	   myRobot->getX(),  myRobot->getY(), myRobot->getTh(), 
	   myRobot->getCompass(), voltage,
	   myRobot->getMotorPacCount(),
	   myMode == MODE_BOTH ? "both" : "either", 
	   gyroString.c_str(),
	   raw.getX(), raw.getY(), raw.getTh()

  );
  else
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%10.2f,%.2f,%.2f", myRobot->getX(), 
	   myRobot->getY(), myRobot->getTh(), myRobot->getCompass(), 
	   voltage, myRobot->getMotorPacCount(), 
	   myMode == MODE_BOTH ? "both" : "either",
	   raw.getX(), raw.getY(), raw.getTh()
    );
  fflush(stdout);
}

MVREXPORT MvrModeIO::MvrModeIO(MvrRobot *robot, const char *name, char key, char key2): 
  MvrMode(robot, name, key, key2)
{
}

MVREXPORT MvrModeIO::~MvrModeIO()
{
  
}

MVREXPORT void MvrModeIO::activate(void)
{
  if (!baseActivate())
    return;
  if (myRobot == NULL)
    return;
  myRobot->comInt(MvrCommands::IOREQUEST, 2);
  myOutput[0] = '\0';
  myLastPacketTime = myRobot->getIOPacketTime();
}

MVREXPORT void MvrModeIO::deactivate(void)
{
  if (!baseDeactivate())
    return;
  if (myRobot == NULL)
    return;
  myRobot->comInt(MvrCommands::IOREQUEST, 0);
}

MVREXPORT void MvrModeIO::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "IO mode shows the IO (digin, digout, a/d) from the robot.");
  myExplanationReady = false;
  myExplained = false;
}

MVREXPORT void MvrModeIO::userTask(void)
{
  int num;
  int i, j;
  unsigned int value;
  int bit;
  char label[256];
  myOutput[0] = '\0';

  //if (myLastPacketTime.mSecSince(myRobot->getIOPacketTime()) == 0)
  //  return;

  if (!myExplanationReady)
    myExplanation[0] = '\0';

  value = myRobot->getFlags();
  if (!myExplanationReady)
  {
    sprintf(label, "flags");
    sprintf(myExplanation, "%s%17s  ", myExplanation, label);
  }
  for (j = 0, bit = 1; j < 16; ++j, bit *= 2)
  {
    if (j == 8)
      sprintf(myOutput, "%s ", myOutput);
    if (value & bit)
      sprintf(myOutput, "%s%d", myOutput, 1);
    else
      sprintf(myOutput, "%s%d", myOutput, 0);
  }
  sprintf(myOutput, "%s  ", myOutput);

  if (myRobot->hasFaultFlags())
  {
    value = myRobot->getFaultFlags();
    if (!myExplanationReady)
    {
      sprintf(label, "fault_flags");
      sprintf(myExplanation, "%s%17s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 16; ++j, bit *= 2)
    {
      if (j == 8)
	sprintf(myOutput, "%s ", myOutput);
      if (value & bit)
	sprintf(myOutput, "%s%d", myOutput, 1);
      else
	sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }

  num = myRobot->getIODigInSize();
  for (i = 0; i < num; ++i)
  {
    value = myRobot->getIODigIn(i);
    if (!myExplanationReady)
    {
      sprintf(label, "digin%d", i);
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }

  num = myRobot->getIODigOutSize();
  for (i = 0; i < num; ++i)
  {
    value = myRobot->getIODigOut(i);
    if (!myExplanationReady)
    {
      sprintf(label, "digout%d", i);
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }

  num = myRobot->getIOAnalogSize();
  for (i = 0; i < num; ++i)
  {
    if (!myExplanationReady)
    {
      sprintf(label, "a/d%d", i);
      sprintf(myExplanation, "%s%6s", myExplanation, label);
    }
    
    /*
      int ad = myRobot->getIOAnalog(i);
      double adVal;
    ad &= 0xfff;
    adVal = ad * .0048828;
    sprintf(myOutput, "%s%6.2f", myOutput,adVal);
    */
    sprintf(myOutput, "%s%6.2f", myOutput, myRobot->getIOAnalogVoltage(i));
    
  }

  if (!myExplained)
  {
    printf("\n%s\n", myExplanation);
    myExplained = true;
  }

  printf("\r%s", myOutput);
  fflush(stdout);
}

MVREXPORT MvrModeLaser::MvrModeLaser(MvrRobot *robot, const char *name, 
				  char key, char key2, MvrSick *obsolete) :
  MvrMode(robot, name, key, key2),
  myTogMiddleCB(this, &MvrModeLaser::togMiddle)
{
  myPrintMiddle = false;

  MvrLaser *laser;
  int i;
  for (i = 1; i <= 10; i++)
  {
    if ((laser = myRobot->findLaser(i)) != NULL)
    {
      myLaserCallbacks[i] = new MvrFunctor1C<MvrModeLaser, 
      int>(this, &MvrModeLaser::switchToLaser, i),
      myLasers[i] = laser;
    }
  }
  
  myLaser = NULL;
  myState = STATE_UNINITED;
}

MVREXPORT MvrModeLaser::~MvrModeLaser()
{
}

MVREXPORT void MvrModeLaser::activate(void)
{
  // this is here because there needs to be the laser set up for the
  // help to work right
  std::map<int, MvrLaser *>::iterator it;
  if (myLaser == NULL)
  {
    if ((it = myLasers.begin()) == myLasers.end())
    {
      MvrLog::log(MvrLog::Normal, "Laser mode tried to activate, but has no lasers");
    }
    else
    {
      myLaser = (*it).second;
      myLaserNumber = (*it).first;
    }
  }


  bool alreadyActive = false;

  if (ourActiveMode == this)
    alreadyActive = true;

  if (!alreadyActive && !baseActivate())
    return;

  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "Laser mode activated but there is no robot.");
    return;
  }

  if (myLaser == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "Laser mode activated but there are no lasers.");
	return;
  }

  if (!alreadyActive)
  {
    
    addKeyHandler('z', &myTogMiddleCB);
    addKeyHandler('Z', &myTogMiddleCB);
    
    std::map<int, MvrFunctor1C<MvrModeLaser, int> *>::iterator kIt;
    for (kIt = myLaserCallbacks.begin(); kIt != myLaserCallbacks.end(); kIt++)
    {
      if ((*kIt).first >= 1 || (*kIt).first <= 9)
	addKeyHandler('0' + (*kIt).first, (*kIt).second);
    }   
  }

  if (myState == STATE_UNINITED)
  {
    myLaser->lockDevice();
    if (myLaser->isConnected())
    {
      MvrLog::log(MvrLog::Verbose, 
		 "\nMvrModeLaser using already existing and connected laser.");
      myState = STATE_CONNECTED;
    }
    else if (myLaser->isTryingToConnect())
    {
      MvrLog::log(MvrLog::Terse, "\nMvrModeLaser already connecting to %s.",
		 myLaser->getName());
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
		 "\nMvrModeLaser is connecting to %s.",
		 myLaser->getName());
      myLaser->asyncConnect();
      myState = STATE_CONNECTING;
    }
    myLaser->unlockDevice();
  } 
}

MVREXPORT void MvrModeLaser::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myTogMiddleCB);
  
  std::map<int, MvrFunctor1C<MvrModeLaser, int> *>::iterator it;
  for (it = myLaserCallbacks.begin(); it != myLaserCallbacks.end(); it++)
  {
    remKeyHandler((*it).second);
  }   
}

MVREXPORT void MvrModeLaser::help(void)
{
  if (myLaser == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "There are no lasers, this mode cannot do anything");
    return;
  }

  MvrLog::log(MvrLog::Terse, 
	     "Laser mode connects to a laser, or uses a previously established connection.");
  MvrLog::log(MvrLog::Terse,
	     "Laser mode then displays the closest and furthest reading from the laser.");
  MvrLog::log(MvrLog::Terse, "%13s:  toggle between far reading and middle reading with reflectivity", "'z' or 'Z'");

  std::map<int, MvrFunctor1C<MvrModeLaser, int> *>::iterator it;
  for (it = myLaserCallbacks.begin(); it != myLaserCallbacks.end(); it++)
  {
    MvrLog::log(MvrLog::Terse, "%13d:  %s", (*it).first, 
	       myLasers[(*it).first]->getName());
  }   
}


MVREXPORT void MvrModeLaser::userTask(void)
{
  double dist = HUGE_VAL, angle = -1;
  int reflec = -1;
  double midDist = HUGE_VAL, midAngle = -1;
  int midReflec = -1;
  double farDist = -HUGE_VAL, farAngle = -1;

  if (myRobot == NULL || myLaser == NULL)
    return;


  if (myState == STATE_CONNECTED && !myPrintMiddle)
  {
    const std::list<MvrPoseWithTime *> *readings;
    std::list<MvrPoseWithTime *>::const_iterator it;
    bool found = false;
  
    myLaser->lockDevice();
    if (!myLaser->isConnected())
    {
      MvrLog::log(MvrLog::Terse, "\n\nLaser mode lost connection to the laser.");
      MvrLog::log(MvrLog::Terse, "Select that laser or laser mode again to try reconnecting to the laser.\n");
      myState = STATE_UNINITED;
    }
    dist = myLaser->currentReadingPolar(-90, 90, &angle);
    if (dist < myLaser->getMaxRange())
      printf("\rClose: %8.0fmm %5.1f deg   ", dist, angle);
    else
      printf("\rNo close reading.         ");

    readings = myLaser->getCurrentBuffer();
    for (it = readings->begin(), found = false; it != readings->end(); it++)
    {
      dist = myRobot->findDistanceTo(*(*it));
      angle = myRobot->findDeltaHeadingTo(*(*it));
      if (!found || dist > farDist)
      {
	found = true;
	farDist = dist;
	farAngle = angle;
      }
    }
    if (found)
      printf("Far: %8.0fmm %5.1f deg", 
	     farDist, farAngle);
    else
      printf("No far reading found");
    printf("         %lu readings   ", readings->size());
    myLaser->unlockDevice();
    fflush(stdout);
  }
  else if (myState == STATE_CONNECTED && myPrintMiddle)
  {
    const std::list<MvrSensorReading *> *rawReadings;
    std::list<MvrSensorReading *>::const_iterator rawIt;
    myLaser->lockDevice();
    if (!myLaser->isConnected())
    {
      MvrLog::log(MvrLog::Terse, "\n\nLaser mode lost connection to the laser.");
      MvrLog::log(MvrLog::Terse, "Switch out of this mode and back if you want to try reconnecting to the laser.\n");
      myState = STATE_UNINITED;
    }
    rawReadings = myLaser->getRawReadings();
    int middleReading = rawReadings->size() / 2;
    if (rawReadings->begin() != rawReadings->end())
    {
      int i;
      for (rawIt = rawReadings->begin(), i = 0; 
	   rawIt != rawReadings->end(); 
	   rawIt++, i++)
      {
	if ((*rawIt)->getIgnoreThisReading())
	  continue;
	if (rawIt == rawReadings->begin() || 
	    (*rawIt)->getRange() < dist)
	{
	  dist = (*rawIt)->getRange();
	  angle = (*rawIt)->getSensorTh();
	  reflec = (*rawIt)->getExtraInt();
	}
	if (i == middleReading)
	{
	  midDist = (*rawIt)->getRange();
	  midAngle = (*rawIt)->getSensorTh();
	  midReflec = (*rawIt)->getExtraInt();
	}
      }
      printf(
      "\rClose: %8.0fmm %5.1f deg %d refl          Middle: %8.0fmm %5.1fdeg, %d refl", 
	      dist, angle, reflec, midDist, midAngle, midReflec);
    }
    else
      printf("\rNo readings");
    myLaser->unlockDevice(); 
  }
  else if (myState == STATE_CONNECTING)
  {
    myLaser->lockDevice();
    if (myLaser->isConnected())
    {
      MvrLog::log(MvrLog::Terse, "\nLaser mode has connected to the laser.\n");
      myState = STATE_CONNECTED;
    }
    else if (!myLaser->isTryingToConnect())
    {
      MvrLog::log(MvrLog::Terse, "\nLaser mode failed to connect to the laser.\n");
      MvrLog::log(MvrLog::Terse, 
		 "Switch out of this mode and back to try reconnecting.\n");
      myState = STATE_UNINITED;
    }
    myLaser->unlockDevice();
  }
}


void MvrModeLaser::togMiddle(void)
{
  myPrintMiddle = !myPrintMiddle;
}

MVREXPORT void MvrModeLaser::switchToLaser(int laserNumber)
{
  if (laserNumber == myLaserNumber && myLaser->isConnected())
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrModeLaser::switchToLaser: Already on laser %s", myLaser->getName());
    return;
  }

  std::map<int, MvrLaser *>::iterator it;
  if ((it = myLasers.find(laserNumber)) == myLasers.end())
  {
    MvrLog::log(MvrLog::Normal, "MvrModeLaser::switchToLaser: told to switch to laser %d but that laser does not exist"); 
    return;
  }
  myLaser = (*it).second;
  MvrLog::log(MvrLog::Normal, "\r\n\nSwitching to laser %s\n", 
	     myLaser->getName()); 
  myState = STATE_UNINITED;
  myLaserNumber = laserNumber;

  activate();
}

/**
  @param robot MvrRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
  @param acts MvrACTS_1_2 instance to use. If not given, then an internally
maintained instance is created by MvrModeActs.
 **/
MVREXPORT MvrModeActs::MvrModeActs(MvrRobot *robot, const char *name, char key, 
				char key2, MvrACTS_1_2 *acts): 
  MvrMode(robot, name, key, key2),
  myChannel1CB(this, &MvrModeActs::channel1),
  myChannel2CB(this, &MvrModeActs::channel2),
  myChannel3CB(this, &MvrModeActs::channel3),
  myChannel4CB(this, &MvrModeActs::channel4),
  myChannel5CB(this, &MvrModeActs::channel5),
  myChannel6CB(this, &MvrModeActs::channel6),
  myChannel7CB(this, &MvrModeActs::channel7),
  myChannel8CB(this, &MvrModeActs::channel8),
  myStopCB(this, &MvrModeActs::stop),
  myStartCB(this, &MvrModeActs::start),
  myToggleAcquireCB(this, &MvrModeActs::toggleAcquire)
{
  if (acts != NULL)
    myActs = acts;
  else
    myActs = new MvrACTS_1_2;
  myRobot = robot;
  myActs->openPort(myRobot);
  myGroup = new MvrActionGroupColorFollow(myRobot, myActs, camera);
  myGroup->deactivate();
}

// Destructor
MVREXPORT MvrModeActs::~MvrModeActs()
{
  
}

// Activate the mode
MVREXPORT void MvrModeActs::activate(void)
{
  // Activate the group
  if (!baseActivate())
    return;
  myGroup->activateExclusive();
  
  // Add key handlers for keyboard input
  addKeyHandler(MvrKeyHandler::SPACE, &myStopCB);
  addKeyHandler('z', &myStartCB);
  addKeyHandler('Z', &myStartCB);
  addKeyHandler('x', &myToggleAcquireCB);
  addKeyHandler('X', &myToggleAcquireCB);
  addKeyHandler('1', &myChannel1CB);
  addKeyHandler('2', &myChannel2CB);
  addKeyHandler('3', &myChannel3CB);
  addKeyHandler('4', &myChannel4CB);
  addKeyHandler('5', &myChannel5CB);
  addKeyHandler('6', &myChannel6CB);
  addKeyHandler('7', &myChannel7CB);
  addKeyHandler('8', &myChannel8CB);

  // Set the camera
  camera = myRobot->getPTZ();
  
  // Tell us whether we are connected to ACTS or not
  if(myActs->isConnected())
    { 
      printf("\nConnected to ACTS.\n");
    }
  else printf("\nNot connected to ACTS.\n");

  // Tell us whether a camera is defined or not
  if(camera != NULL)
    {
      printf("\nCamera defined.\n\n");
      myGroup->setCamera(camera);
    }
  else
    {  
      printf("\nNo camera defined.\n");
      printf("The robot will not tilt its camera up or down until\n");
      printf("a camera has been defined in camera mode ('c' or 'C').\n\n");
    }
}

// Deactivate the group
MVREXPORT void MvrModeActs::deactivate(void)
{
  if (!baseDeactivate())
    return;

  // Remove the key handlers
  remKeyHandler(&myStopCB);
  remKeyHandler(&myStartCB);
  remKeyHandler(&myToggleAcquireCB);
  remKeyHandler(&myChannel1CB);
  remKeyHandler(&myChannel2CB);
  remKeyHandler(&myChannel3CB);
  remKeyHandler(&myChannel4CB);
  remKeyHandler(&myChannel5CB);
  remKeyHandler(&myChannel6CB);
  remKeyHandler(&myChannel7CB);
  remKeyHandler(&myChannel8CB);

  myGroup->deactivate();
}

// Display the available commands
MVREXPORT void MvrModeActs::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	   "ACTS mode will drive the robot in an attempt to follow a color blob.\n");

  MvrLog::log(MvrLog::Terse, "%20s:  Pick a channel",     "1 - 8    ");
  MvrLog::log(MvrLog::Terse, "%20s:  toggle acquire mode", "'x' or 'X'");
  MvrLog::log(MvrLog::Terse, "%20s:  start movement",     "'z' or 'Z'");
  MvrLog::log(MvrLog::Terse, "%20s:  stop movement",      "space bar");
  MvrLog::log(MvrLog::Terse, "");
  
}

// Display data about this mode
MVREXPORT void MvrModeActs::userTask(void)
{
  int myChannel;

  const char *acquire;
  const char *move;
  const char *blob;

  myChannel = myGroup->getChannel();
  if(myGroup->getAcquire()) acquire = "actively acquiring";
  else acquire = "passively acquiring";
  
  if(myGroup->getMovement()) move = "movement on";
  else move = "movement off";

  if(myGroup->getBlob()) blob = "blob in sight";
  else blob = "no blob in sight";

  printf("\r Channel: %d  %15s %25s %20s", myChannel, move, acquire, blob);
  fflush(stdout);
}

// The channels
MVREXPORT void MvrModeActs::channel1(void)
{
  myGroup->setChannel(1);
}

MVREXPORT void MvrModeActs::channel2(void)
{
  myGroup->setChannel(2);
}

MVREXPORT void MvrModeActs::channel3(void)
{
  myGroup->setChannel(3);
}

MVREXPORT void MvrModeActs::channel4(void)
{
  myGroup->setChannel(4);
}

MVREXPORT void MvrModeActs::channel5(void)
{
  myGroup->setChannel(5);
}

MVREXPORT void MvrModeActs::channel6(void)
{
  myGroup->setChannel(6);
}

MVREXPORT void MvrModeActs::channel7(void)
{
  myGroup->setChannel(7);
}

MVREXPORT void MvrModeActs::channel8(void)
{
  myGroup->setChannel(8);
}

// Stop the robot from moving
MVREXPORT void MvrModeActs::stop(void)
{
  myGroup->stopMovement();
}

// Allow the robot to move
MVREXPORT void MvrModeActs::start(void)
{
  myGroup->startMovement();
}

// Toggle whether or not the robot is allowed
// to aquire anything
MVREXPORT void MvrModeActs::toggleAcquire()
{
  if(myGroup->getAcquire())
    myGroup->setAcquire(false);
  else myGroup->setAcquire(true);

}

MVREXPORT MvrModeCommand::MvrModeCommand(MvrRobot *robot, const char *name, char key, char key2): 
  MvrMode(robot, name, key, key2),
  my0CB(this, &MvrModeCommand::addChar, '0'),
  my1CB(this, &MvrModeCommand::addChar, '1'),
  my2CB(this, &MvrModeCommand::addChar, '2'),
  my3CB(this, &MvrModeCommand::addChar, '3'),
  my4CB(this, &MvrModeCommand::addChar, '4'),
  my5CB(this, &MvrModeCommand::addChar, '5'),
  my6CB(this, &MvrModeCommand::addChar, '6'),
  my7CB(this, &MvrModeCommand::addChar, '7'),
  my8CB(this, &MvrModeCommand::addChar, '8'),
  my9CB(this, &MvrModeCommand::addChar, '9'),
  myMinusCB(this, &MvrModeCommand::addChar, '-'),
  myBackspaceCB(this, &MvrModeCommand::addChar, MvrKeyHandler::BACKSPACE),
  mySpaceCB(this, &MvrModeCommand::addChar, MvrKeyHandler::SPACE),
  myEnterCB(this, &MvrModeCommand::finishParsing)

{
  reset(false);
}

MVREXPORT MvrModeCommand::~MvrModeCommand()
{
  
}

MVREXPORT void MvrModeCommand::activate(void)
{
  reset(false);
  if (!baseActivate())
    return;
  myRobot->stopStateReflection();
  takeKeys();
  reset(true);
}

MVREXPORT void MvrModeCommand::deactivate(void)
{
  if (!baseDeactivate())
    return;
  giveUpKeys();
}

MVREXPORT void MvrModeCommand::help(void)
{
  
  MvrLog::log(MvrLog::Terse, "Command mode has three ways to send commands");
  MvrLog::log(MvrLog::Terse, "%-30s: Sends com(<command>)", "<command>");
  MvrLog::log(MvrLog::Terse, "%-30s: Sends comInt(<command>, <integer>)", "<command> <integer>");
  MvrLog::log(MvrLog::Terse, "%-30s: Sends com2Bytes(<command>, <byte1>, <byte2>)", "<command> <byte1> <byte2>");
}

void MvrModeCommand::addChar(int ch)
{
  if (ch < '0' && ch > '9' && ch != '-' && ch != MvrKeyHandler::BACKSPACE && 
      ch != MvrKeyHandler::SPACE)
  {
    MvrLog::log(MvrLog::Terse, "Something horribly wrong in command mode since number is < 0 || > 9 (it is the value %d)", ch);
    return;
  }

  size_t size = sizeof(myCommandString);
  size_t len = strlen(myCommandString);

  if (ch == MvrKeyHandler::BACKSPACE)
  {
    // don't overrun backwards
    if (len < 1)
      return;
    myCommandString[len-1] = '\0';
    printf("\r> %s  \r> %s", myCommandString, myCommandString);
    return;
  }
  if (ch == MvrKeyHandler::SPACE)
  {
    // if we're at the start or have a space or - just return
    if (len < 1 || myCommandString[len-1] == ' ' || 
	myCommandString[len-1] == '-')
      return;
    myCommandString[len] = ' ';
    myCommandString[len+1] = '\0';
    printf(" ");
    return;
  }
  if (ch == '-')
  {
    // make sure it isn't the command trying to be negated or that its the start of the byte
    if (len < 1 || myCommandString[len-1] != ' ')
      return;
    printf("%c", '-');
    myCommandString[len] = '-';
    myCommandString[len+1] = '\0';
    return;
  }
  if (len + 1 >= size)
  {
    printf("\n");
    MvrLog::log(MvrLog::Terse, "Command is too long, abandoning command");
    reset();
    return;
  }
  else
  {
    printf("%c", ch);
    myCommandString[len] = ch;
    myCommandString[len+1] = '\0';
    return;
  }
}

void MvrModeCommand::finishParsing(void)
{
  
  MvrArgumentBuilder builder;
  builder.addPlain(myCommandString);
  int command;
  int int1;
  int int2;

  if (myCommandString[0] == '\0')
    return;

  printf("\n");
  if (builder.getArgc() == 0)
  {
    MvrLog::log(MvrLog::Terse, "Syntax error, no arguments.");
  }
  if (builder.getArgc() == 1)
  {
    command = builder.getArgInt(0);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      MvrLog::log(MvrLog::Terse, 
		 "Invalid command, must be an integer between 0 and 255");
      reset();
      return;
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "com(%d)", command);
      myRobot->com(command);
      reset();
      return;
    }
  }
  else if (builder.getArgc() == 2)
  {
    command = builder.getArgInt(0);
    int1 = builder.getArgInt(1);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      MvrLog::log(MvrLog::Terse, 
		 "Invalid command, must be an integer between 0 and 255");
      reset();
      return;
    }
    else if (int1 < -32767 || int1 > 32767 || !builder.isArgInt(1))
    {
      MvrLog::log(MvrLog::Terse, 
	 "Invalid integer, must be an integer between -32767 and 32767");
      reset();
      return;
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "comInt(%d, %d)", command,
		 int1);
      myRobot->comInt(command, int1);
      reset();
      return;
    }
  }
  else if (builder.getArgc() == 3)
  {
    command = builder.getArgInt(0);
    int1 = builder.getArgInt(1);
    int2 = builder.getArgInt(2);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      MvrLog::log(MvrLog::Terse, 
		 "Invalid command, must be between 0 and 255");
      reset();
      return;
    }
    else if (int1 < -128 || int1 > 255 || !builder.isArgInt(1))
    {
      MvrLog::log(MvrLog::Terse, 
	 "Invalid byte1, must be an integer between -128 and 127, or between 0 and 255");
      reset();
      return;
    }
    else if (int2 < -128 || int2 > 255 || !builder.isArgInt(2))
    {
      MvrLog::log(MvrLog::Terse, 
	 "Invalid byte2, must be an integer between -128 and 127, or between 0 and 255");
      reset();
      return;
    }
    else
    {
      MvrLog::log(MvrLog::Terse, 
		 "com2Bytes(%d, %d, %d)", 
		 command, int1, int2);
      myRobot->com2Bytes(command, int1, int2);
      reset();
      return;
    }
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "Syntax error, too many arguments");
    reset();
    return;
  }
}

void MvrModeCommand::reset(bool print)
{
  myCommandString[0] = '\0';
  if (print)
  {
    MvrLog::log(MvrLog::Terse, "");
    printf("> ");
  }
}

void MvrModeCommand::takeKeys(void)
{
  addKeyHandler('0', &my0CB);
  addKeyHandler('1', &my1CB);
  addKeyHandler('2', &my2CB);
  addKeyHandler('3', &my3CB);
  addKeyHandler('4', &my4CB);
  addKeyHandler('5', &my5CB);
  addKeyHandler('6', &my6CB);
  addKeyHandler('7', &my7CB);
  addKeyHandler('8', &my8CB);
  addKeyHandler('9', &my9CB);
  addKeyHandler('-', &myMinusCB);
  addKeyHandler(MvrKeyHandler::BACKSPACE, &myBackspaceCB);
  addKeyHandler(MvrKeyHandler::ENTER, &myEnterCB);
  addKeyHandler(MvrKeyHandler::SPACE, &mySpaceCB);
}

void MvrModeCommand::giveUpKeys(void)
{
  remKeyHandler(&my0CB);
  remKeyHandler(&my1CB);
  remKeyHandler(&my2CB);
  remKeyHandler(&my3CB);
  remKeyHandler(&my4CB);
  remKeyHandler(&my5CB);
  remKeyHandler(&my6CB);
  remKeyHandler(&my7CB);
  remKeyHandler(&my8CB);
  remKeyHandler(&my9CB);
  remKeyHandler(&myBackspaceCB);
  remKeyHandler(&myMinusCB);
  remKeyHandler(&myEnterCB);
  remKeyHandler(&mySpaceCB);
}

/**
  @param robot MvrRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
   @param tcm2 if a tcm2 class is passed in it'll use that instance
   otherwise it'll make its own MvrTCMCompassRobot instance.
**/

MVREXPORT MvrModeTCM2::MvrModeTCM2(MvrRobot *robot, const char *name, char key, char key2, MvrTCM2 *tcm2): 
  MvrMode(robot, name, key, key2)
{
  if (tcm2 != NULL)
    myTCM2 = tcm2;
  else
    myTCM2 = new MvrTCMCompassRobot(robot);

  myOffCB = new MvrFunctorC<MvrTCM2>(myTCM2, &MvrTCM2::commandOff);
  myCompassCB = new MvrFunctorC<MvrTCM2>(myTCM2, &MvrTCM2::commandJustCompass);
  myOnePacketCB = new MvrFunctorC<MvrTCM2>(myTCM2, &MvrTCM2::commandOnePacket);
  myContinuousPacketsCB = new MvrFunctorC<MvrTCM2>(
	  myTCM2, &MvrTCM2::commandContinuousPackets);
  myUserCalibrationCB = new MvrFunctorC<MvrTCM2>(
	  myTCM2, &MvrTCM2::commandUserCalibration);
  myAutoCalibrationCB = new MvrFunctorC<MvrTCM2>(
	  myTCM2, &MvrTCM2::commandAutoCalibration);
  myStopCalibrationCB = new MvrFunctorC<MvrTCM2>(
	  myTCM2, &MvrTCM2::commandStopCalibration);
  myResetCB = new MvrFunctorC<MvrTCM2>(
	  myTCM2, &MvrTCM2::commandSoftReset);

}

MVREXPORT MvrModeTCM2::~MvrModeTCM2()
{
  
}

MVREXPORT void MvrModeTCM2::activate(void)
{
  if (!baseActivate())
    return;
  myTCM2->commandContinuousPackets();
  addKeyHandler('0', myOffCB);
  addKeyHandler('1', myCompassCB);
  addKeyHandler('2', myOnePacketCB);
  addKeyHandler('3', myContinuousPacketsCB);
  addKeyHandler('4', myUserCalibrationCB);
  addKeyHandler('5', myAutoCalibrationCB);
  addKeyHandler('6', myStopCalibrationCB);
  addKeyHandler('7', myResetCB);
}

MVREXPORT void MvrModeTCM2::deactivate(void)
{
  if (!baseDeactivate())
    return;
  myTCM2->commandJustCompass();
  remKeyHandler(myOffCB);
  remKeyHandler(myCompassCB);
  remKeyHandler(myOnePacketCB);
  remKeyHandler(myContinuousPacketsCB);
  remKeyHandler(myUserCalibrationCB);
  remKeyHandler(myAutoCalibrationCB);
  remKeyHandler(myStopCalibrationCB);
  remKeyHandler(myResetCB);
}

MVREXPORT void MvrModeTCM2::help(void)
{
  MvrLog::log(MvrLog::Terse, 
	     "TCM2 mode shows the data from the TCM2 compass and lets you send the TCM2 commands");
  MvrLog::log(MvrLog::Terse, "%20s:  turn TCM2 off", "'0'");  
  MvrLog::log(MvrLog::Terse, "%20s:  just get compass readings", "'1'");  
  MvrLog::log(MvrLog::Terse, "%20s:  get a single set of TCM2 data", "'2'");  
  MvrLog::log(MvrLog::Terse, "%20s:  get continuous TCM2 data", "'3'");  
  MvrLog::log(MvrLog::Terse, "%20s:  start user calibration", "'4'");  
  MvrLog::log(MvrLog::Terse, "%20s:  start auto calibration", "'5'");  
  MvrLog::log(MvrLog::Terse, "%20s:  stop calibration and get a single set of data", "'6'");  
  MvrLog::log(MvrLog::Terse, "%20s:  soft reset of compass", "'7'");  

  printf("%6s %5s %5s %6s %6s %6s %6s %10s %4s %4s %6s %3s\n", 
	 "comp", "pitch", "roll", "magX", "magY", "magZ", "temp", "error",
	 "calH", "calV", "calM", "cnt");
}

MVREXPORT void MvrModeTCM2::userTask(void)
{
  printf("\r%6.1f %5.1f %5.1f %6.2f %6.2f %6.2f %6.1f 0x%08x %4.0f %4.0f %6.2f %3d", 
	 myTCM2->getCompass(), myTCM2->getPitch(), myTCM2->getRoll(), 
	 myTCM2->getXMagnetic(), myTCM2->getYMagnetic(), 
	 myTCM2->getZMagnetic(), 
	 myTCM2->getTemperature(), myTCM2->getError(), 
	 myTCM2->getCalibrationH(), myTCM2->getCalibrationV(), 
	 myTCM2->getCalibrationM(), myTCM2->getPacCount());
  fflush(stdout);

}

MVREXPORT MvrModeConfig::MvrModeConfig(MvrRobot *robot, const char *name, char key1, char key2) :
  MvrMode(robot, name, key1, key2),
  myRobot(robot),
  myConfigPacketReader(robot, false, &myGotConfigPacketCB),
  myGotConfigPacketCB(this, &MvrModeConfig::gotConfigPacket)
{
}

MVREXPORT void MvrModeConfig::help()
{
  MvrLog::log(MvrLog::Terse, "Robot Config mode requests a CONFIG packet from the robot and displays the result.");
}

MVREXPORT void MvrModeConfig::activate()
{
  baseActivate();  // returns false on double activate, but we want to use this signal to request another config packet, so ignore.
  if(!myConfigPacketReader.requestPacket())
    MvrLog::log(MvrLog::Terse, "MvrModeConfig: Warning: config packet reader did not request (another) CONFIG packet.");
}

MVREXPORT void MvrModeConfig::deactivate()
{
}

void MvrModeConfig::gotConfigPacket()
{
  MvrLog::log(MvrLog::Terse, "\nRobot CONFIG packet received:");
  myConfigPacketReader.log();
  myConfigPacketReader.logMovement();
  MvrLog::log(MvrLog::Terse, "Additional robot information:");
  MvrLog::log(MvrLog::Terse, "HasStateOfCharge %d", myRobot->haveStateOfCharge());
  MvrLog::log(MvrLog::Terse, "StateOfChargeLow %f", myRobot->getStateOfChargeLow());
  MvrLog::log(MvrLog::Terse, "StateOfChargeShutdown %f", myRobot->getStateOfChargeShutdown());
  MvrLog::log(MvrLog::Terse, "HasFaultFlags %d", myRobot->hasFaultFlags());
  MvrLog::log(MvrLog::Terse, "HasTableIR %d", myRobot->hasTableSensingIR());
  MvrLog::log(MvrLog::Terse, "NumSonar (rec'd) %d", myRobot->getNumSonar());
  MvrLog::log(MvrLog::Terse, "HasTemperature (rec'd) %d", myRobot->hasTemperature());
  MvrLog::log(MvrLog::Terse, "HasSettableVelMaxes %d", myRobot->hasSettableVelMaxes());
  MvrLog::log(MvrLog::Terse, "HasSettableAccsDecs %d", myRobot->hasSettableAccsDecs());
  MvrLog::log(MvrLog::Terse, "HasLatVel %d", myRobot->hasLatVel());
  MvrLog::log(MvrLog::Terse, "HasMoveCommand %d", myRobot->getRobotParams()->hasMoveCommand());
  MvrLog::log(MvrLog::Terse, "Radius %f Width %f Length %f LengthFront %f LengthRear %f Diagonal %f", 
    myRobot->getRobotRadius(),
    myRobot->getRobotWidth(),
    myRobot->getRobotLength(),
    myRobot->getRobotLengthFront(),
    myRobot->getRobotLengthRear(),
    myRobot->getRobotDiagonal() 
  );
}


MVREXPORT MvrModeRobotStatus::MvrModeRobotStatus(MvrRobot *robot, const char *name, char key1, char key2) :
  MvrMode(robot, name, key1, key2),
  myRobot(robot),
  myDebugMessageCB(this, &MvrModeRobotStatus::handleDebugMessage),
  mySafetyStateCB(this, &MvrModeRobotStatus::handleSafetyStatePacket),
  mySafetyWarningCB(this, &MvrModeRobotStatus::handleSafetyWarningPacket)
{
}

MVREXPORT void MvrModeRobotStatus::help()
{
  MvrLog::log(MvrLog::Terse, "Robot diagnostic flags mode prints the current state of the robot's error and diagnostic flags."); 
  MvrLog::log(MvrLog::Terse, "Additional debug and status information will also be requested from the robot and logged if received.");
}

MVREXPORT void MvrModeRobotStatus::activate()
{
  if(baseActivate())
  {
    // only do the following on the first activate. they remain activated.
    myRobot->lock();
    myRobot->addPacketHandler(&myDebugMessageCB);
    myRobot->addPacketHandler(&mySafetyStateCB);
    myRobot->addPacketHandler(&mySafetyWarningCB);
    myRobot->unlock();
  }

  // do the following on every activate, even if this mode already activated.

  puts("");
  printFlagsHeader();
  printFlags();
  puts("\n");
    
    
  myRobot->lock();
  int flags = myRobot->getFlags();
  int faults = myRobot->hasFaultFlags() ? myRobot->getFaultFlags() : 0;
  int flags3 = myRobot->hasFlags3() ? myRobot->getFlags3() : 0;
  int configflags = 0;
  const MvrRobotConfigPacketReader *configreader = myRobot->getOrigRobotConfig();
  if(configreader)
    configflags = configreader->getConfigFlags();
  myRobot->unlock();

  if(flags)
  {
    puts("Flags:");
    if(flags & MvrUtil::BIT0)
      puts("\tMotors enabled (flag 0)");
    else
      puts("\tMotors disabled (flag 0)");
    if(flags & MvrUtil::BIT5)
      puts("\tESTOP (flag 5)");
    if(flags & MvrUtil::BIT9)
      puts("\tJoystick button pressed (flag 9)");
    if(flags & MvrUtil::BIT11)
      puts("\tHigh temperature. (flag 11)");
    puts("\t(Flags 1-4 are sonar array enabled, flags 7-8 are legacy IR sensors");
  }

  if(faults)
  {
    puts("Fault Flags:");
    if(faults & MvrUtil::BIT0)
      puts("\tPDB Laser Status Error (fault 0)");
    if(faults & MvrUtil::BIT1)
      puts("\tHigh Temperature (fault 1)");
    if(faults & MvrUtil::BIT2)
      puts("\tPDB Error (fault 2)");
    if(faults & MvrUtil::BIT3)
      puts("\tUndervoltage/Low Battery (fault 3)");
    if(faults & MvrUtil::BIT4)
      puts("\tGyro Critical Fault (fault 4)");
    if(faults & MvrUtil::BIT5)
      puts("\tBattery Overtemperature (fault 5)");
    if(faults & MvrUtil::BIT6)
      puts("\tBattery balance required (fault 6)");
    if(faults & MvrUtil::BIT7)
      puts("\tEncoder degradation (fault 7)");
    if(faults & MvrUtil::BIT8)
      puts("\tEncoder failure (fault 8)");
    if(faults & MvrUtil::BIT9)
      puts("\tCritical general driving fault (fault 9)");
    if(faults & MvrUtil::BIT10)
      puts("\tESTOP Mismatch Warning. One ESTOP channel may be intermittent or failing. Check connections to control panel. (ESTOP_MISMATCH_FLAG, 10)");
    if(faults & MvrUtil::BIT11)
      puts("\tESTOP Safety Fault. ESTOP circuitry has failed. Motors disabled until safety system recommision or disabled. (ESTOP_SAFETY_FAULT, 11)");
    if(faults & MvrUtil::BIT12)
      puts("\tLaser/speed zone failure or zone mismatch. Speed limited until safety system recommisioa or disabled.  (SPEED_ZONE_SAFETY_FAULT, 12)");
    if(faults & MvrUtil::BIT13)
      puts("\tSAFETY_UNKNOWN_FAULT (fault 13)");
    if(faults & MvrUtil::BIT14)
      puts("\tBacked up too fast. Reduce speed to avoid or disable safety system to allow faster reverse motion. (fault 14)");
    if(faults & MvrUtil::BIT15)
      puts("\tJoydrive unsafe mode warning (fault 15)");
  }

  
  if(flags3)
  {
    puts("Flags3:");
    if(flags3 & MvrUtil::BIT0)
      puts("\tJoystick override mode enabled (0)");
    if(flags3 & MvrUtil::BIT1)
      puts("\tAmp. comm. error (1)");
    if(flags3 & MvrUtil::BIT2)
      puts("\tSilent E-Stop (2)");
    if(flags3 & MvrUtil::BIT3)
      puts("\tLaser safety circuit error (S300 error 'n') (3)");
    if(~flags3 & MvrUtil::BIT4)
      puts("\tRotation control loop not enabled (4)");
    if(flags3 & MvrUtil::BIT5)
      puts("\tRotation integrator saturated (5)");
  }
    
  
  if(configflags & MvrUtil::BIT0)
  {
    puts("ConfigFlags:");
    puts("\tFirmware boot error. Robot controller bootloader detected but no firmware. (config flag 0 set)");
  }

  fflush(stdout);

  // TODO keys to enable/disable: LogMovementSent, LogMovementReceived,
  // LogVelocitiesReceived, PacketsReceivedTracking, LogSIPContents,
  // PacketsSentTracking.
  // TODO keys to start/stop WHEEL_INFO
  
  // TODO get simulator info

  myRobot->comInt(214, 1); // request state of safety systems

  // print first header line for user task refresh
  puts("");
  printFlagsHeader();
}

MVREXPORT void MvrModeRobotStatus::deactivate()
{
  if(!baseDeactivate()) return;
  // commented out to keep packet handlers active so we can use other modes and see responses.
  //myRobot->remPacketHandler(&myDebugMessageCB);
  //myRobot->remPacketHandler(&mySafetyStateCB);
  //myRobot->remPacketHandler(&mySafetyWarningCB);
}

MVREXPORT void MvrModeRobotStatus::userTask()
{
  printFlags();
  printf("\r");
  fflush(stdout);
}
  

void MvrModeRobotStatus::printFlagsHeader()
{
  const char* headerfmt = 
    "%-5s "  // volts
    "%-5s "  // soc
    "%-5s "     // temp
    "%-12s "     // mot.enable?
    "%-6s "      // estop?
    "%-7s "      // stallL?
    "%-7s "      // stallR?
    "%-16s "     // stallval
    "%-9s "      // #sip/sec    
    "%-10s "     // cycletime
    "%-16s "     // flags
    "%-16s "     // fault flags
    "%-32s "     // flags3
    "%-13s "     // chargestate
    "%-13s "     // chargerpower
    "\n";


  printf(headerfmt,
        "volt",
        "soc",
        "temp",
        "mot.enable",
        "estop",
        "stall l",
        "stall r",
        "stallval",
        "sips/sec",
        "cycletime",
        "flags",
        "faults",
        "flags3",
        "chargestate",
        "chargepower"
    );
}

void MvrModeRobotStatus::printFlags()
{
  const char* datafmt = 
    "%-03.02f "  // volts
    "%-03.02f "  // soc
    "%- 03d "     // temp
    "%-12s "     // mot.enable?
    "%-6s "      // estop?
    "%-7s "      // stallL?
    "%-7s "      // stallR?
    "%-16s "     // stallval
    "%-9d "      // #sip/sec    
    "%-10d "     // cycletime
    "%-16s "     // flags
    "%-16s "     // fault flags
    "%-32s "     // flags3
    "%-12s "     // chargestate
    "%-13s "     // chargerpower
  ;

  myRobot->lock();

  printf(datafmt,
    myRobot->getRealBatteryVoltage(),
    myRobot->getStateOfCharge(),
    myRobot->getTemperature(),
    myRobot->areMotorsEnabled()?"yes":"NO",
    myRobot->isEStopPressed()?"YES":"no",
    myRobot->isLeftMotorStalled()?"YES":"no",
    myRobot->isRightMotorStalled()?"YES":"no",
    int16_as_bitstring(myRobot->getStallValue()).c_str(),
    myRobot->getMotorPacCount() ,
    myRobot->getCycleTime() ,
    int16_as_bitstring(myRobot->getFlags()).c_str(),
    myRobot->hasFaultFlags() ? int16_as_bitstring(myRobot->getFaultFlags()).c_str() : "n/a",
    myRobot->hasFlags3() ? int32_as_bitstring(myRobot->getFlags3()).c_str() : "n/a",
    myRobot->getChargeStateName(),
    myRobot->isChargerPowerGood() ? "YES" : "no"
  );
  myRobot->unlock();
}

bool MvrModeRobotStatus::handleDebugMessage(MvrRobotPacket *pkt)
{
  if(pkt->getID() != MvrCommands::MARCDEBUG) return false;
  char msg[256];
  pkt->bufToStr(msg, sizeof(msg));
  msg[255] = 0;
  MvrLog::log(MvrLog::Terse, "Firmware Debug Message Received: %s", msg);
  return true;
}


/// return unsigned byte as string of 8 '1' and '0' characters (MSB first, so
/// bit 0 will be last character in string, bit 7 will be first character.)
/// @todo generalize with others to any number of bits
std::string MvrModeRobotStatus::byte_as_bitstring(unsigned char byte) 
{
  char tmp[9];
  int bit; 
  int ch;
  for(bit = 7, ch = 0; bit >= 0; bit--,ch++)
    tmp[ch] = ((byte>>bit)&1) ? '1' : '0';
  tmp[8] = 0;
  return std::string(tmp);
}

/// return unsigned 16-bit value as string of 16 '1' and '0' characters (MSB first, so
/// bit 0 will be last character in string, bit 15 will be first character.)
/// @todo separate every 8 bits. 
/// @todo generalize with others to any number of bits
std::string MvrModeRobotStatus::int16_as_bitstring(MvrTypes::Byte2 n) 
{
  char tmp[17];
  int bit;
  int ch;
  for(bit = 15, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[16] = 0;
  return std::string(tmp);
}

/// @todo separate every 8 bits. 
/// @todo generalize with others to any number of bits
std::string MvrModeRobotStatus::int32_as_bitstring(MvrTypes::Byte4 n)
{
  char tmp[33];
  int bit;
  int ch;
  for(bit = 31, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[32] = 0;
  return std::string(tmp);
}

const char *MvrModeRobotStatus::safetyStateName(int state)
{
  switch (state) {
    case 0:
      return "unknown/initial";
    case 0x10:
      return "failure";
    case 0x20: 
      return "warning";
    case 0x40:
      return "commissioned";
    case 0x50:
      return "decommissioned/disabled";
  }
  return "invalid/unknown";
}

bool MvrModeRobotStatus::handleSafetyStatePacket(MvrRobotPacket *p)
{
  if(p->getID() != 214) return false;
  int state = p->bufToUByte();
  int estop_state = p->bufToUByte();
  int laser_state = p->bufToUByte();
  MvrLog::log(MvrLog::Normal, "Safety system state: 0x%x, system0(estop)=0x%x, %s, system1(laser)=0x%x, %s\n", state, estop_state, safetyStateName(estop_state), laser_state, safetyStateName(laser_state));
  return true;
}

bool MvrModeRobotStatus::handleSafetyWarningPacket(MvrRobotPacket *p)
{
  if(p->getID() != 217) return false;
  MvrLog::log(MvrLog::Terse, "Safety system warning received!");
  return false; // let other stuff also handle it
}

