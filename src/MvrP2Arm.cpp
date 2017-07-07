#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrP2Arm.h"
#include "mvriaUtil.h"
#include "MvrLog.h"
#include "mvriaInternal.h"

int MvrP2Arm::NumJoints=6;
const unsigned int MvrP2Arm::ARMpac=0xa0;
const unsigned int MvrP2Arm::ARMINFOpac=0xa1;
const unsigned char MvrP2Arm::ComArmInfo=70;
const unsigned char MvrP2Arm::ComArmStats=71;
const unsigned char MvrP2Arm::ComArmInit=72;
const unsigned char MvrP2Arm::ComArmCheckArm=73;
const unsigned char MvrP2Arm::ComArmPower=74;
const unsigned char MvrP2Arm::ComArmHome=75;
const unsigned char MvrP2Arm::ComArmPark=76;
const unsigned char MvrP2Arm::ComArmPos=77;
const unsigned char MvrP2Arm::ComArmSpeed=78;
const unsigned char MvrP2Arm::ComArmStop=79;
const unsigned char MvrP2Arm::ComArmAutoPark=80;
const unsigned char MvrP2Arm::ComArmGripperPark=81;
const int MvrP2Arm::ArmJoint1=0x1;
const int MvrP2Arm::ArmJoint2=0x2;
const int MvrP2Arm::ArmJoint3=0x4;
const int MvrP2Arm::ArmJoint4=0x8;
const int MvrP2Arm::ArmJoint5=0x10;
const int MvrP2Arm::ArmJoint6=0x20;
const int MvrP2Arm::ArmGood=0x100;
const int MvrP2Arm::ArmInited=0x200;
const int MvrP2Arm::ArmPower=0x400;
const int MvrP2Arm::ArmHoming=0x800;


MVREXPORT MvrP2Arm::MvrP2Arm() :
  myInited(false),
  myRobot(0),
  //  myModel(),
  myLastStatusTime(),
  myLastInfoTime(),
  myVersion(),
  myStatusRequest(MvrP2Arm::StatusOff),
  myLastStatus(0),
  myStatus(0),
  myCon(),
  myMvrUninitCB(this, &MvrP2Arm::uninit),
  myArmPacketHandler(this, &MvrP2Arm::armPacketHandler),
  myPacketCB(0),
  myStoppedCB(0)
{
  myArmPacketHandler.setName("MvrP2Arm");
}

MVREXPORT MvrP2Arm::~MvrP2Arm()
{
  //uninit();
  if (myRobot != NULL)
    myRobot->remPacketHandler(&myArmPacketHandler);
}

/**
   Initialize the P2 Arm class. This must be called before anything else. The
   setRobot() must be called to let MvrP2Arm know what instance of an MvrRobot
   to use. It talks to the robot and makes sure that there is an arm on it
   and it is in a good condition. The AROS/P2OS arm servers take care of AUX
   port serial communications with the P2 Arm controller.
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::init()
{
  MvrLog::log(MvrLog::Normal, "Initializing the arm.");

  MvrTime now;

  if (myInited)
    return(ALREADY_INITED);

  if (!myRobot || !myRobot->isRunning() || !myRobot->isConnected())
    return(ROBOT_NOT_SETUP);

  Mvria::addUninitCallBack(&myMvrUninitCB, MvrListPos::FIRST);
  MvrLog::log(MvrLog::Verbose, "Adding the P2 Arm packet handler.");
  myRobot->addPacketHandler(&myArmPacketHandler, MvrListPos::FIRST);
  now.setToNow();
  if (!comArmStats(StatusSingle))
    return(COMM_FAILED);
  MvrUtil::sleep(100);
  if (!comArmInfo())
    return(COMM_FAILED);
  MvrUtil::sleep(300);

  if (!now.isAfter(myLastStatusTime) || !now.isAfter(myLastInfoTime))
    return(COMM_FAILED);

  if (!(myStatus & ArmGood))
    return(NO_ARM_FOUND);

  myInited=true;

  return(SUCCESS);
}

/**
   Uninitialize the arm class. This simply asks the arm to park itself
   and cleans up its internal state. To completely uninitialize the P2 Arm
   itself have the MvrRobot disconnect from P2OS.
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::uninit()
{
  bool ret;

  if (!myInited)
    return(NOT_INITED);

  ret=comArmPark();

  myInited=false;
  myVersion="";
  myStatusRequest=MvrP2Arm::StatusOff;
  myLastStatus=0;
  myStatus=0;
  if (ret)
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   Powers on the arm. The arm will shake for up to 2 seconds after powering
   on. If the arm is told to move before it stops shaking, that vibration
   can be amplified by moving. The default is to wait the 2 seconds for the
   arm to settle down.
   @param doSleep if true, sleeps 2 seconds to wait for the arm to stop shaking
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::powerOn(bool doSleep)
{
  if (isGood())
  {
    MvrLog::log(MvrLog::Normal, "MvrP2Arm::powerOn: Powering arm.");
    if (!comArmPower(true))
      return(COMM_FAILED);
    // Sleep for 2 seconds to wait for the arm to stop shaking from the
    // effort of turning on
    if (doSleep)
      MvrUtil::sleep(2000);
    return(SUCCESS);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Powers off the arm. This should only be called when the arm is in a good
   position to power off. Due to the design, it will go limp when the power
   is turned off. A more safe way to power off the arm is to use the
   MvrP2Arm::park() function. Which will home the arm, then power if off.
   @see park
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::powerOff()
{
  if (isGood())
  {
    MvrLog::log(MvrLog::Normal, "MvrP2Arm::powerOff: Powering off arm.");
    if (comArmPower(false))
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests the arm info packet from P2OS and immediately returns. This packet
   will be sent during the next 100ms cycle of P2OS. Since there is a very
   noticable time delay, the user should use the MvrP2Arm::setPacketCB() to set
   a callback so the user knows when the packet has been received.
   @see setPacketCB
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::requestInfo()
{
  if (isGood())
  {
    if (comArmInfo())
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests the arm status packet from P2OS and immediately returns. This
   packet will be sent during the next 100ms cycle of P2OS. Since there is a
   very noticable time delay, the user should use the MvrP2Arm::setPacketCB() to
   set a callback so the user knows when the packet has been received.
   @see setPacketCB
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::requestStatus(StatusType status)
{
  if (isGood())
  {
    if (comArmStats(status))
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests that P2OS initialize the arm and immediately returns. The arm
   initialization procedure takes about 700ms to complete and a little more
   time for the status information to be relayed back to the client. Since
   there is a very noticable time delay, the user should use the
   MvrP2Arm::setPacketCB() to set a callback so the user knows when the arm info
   packet has been received. Then wait about 800ms, and send a 
   MvrP2Arm::requestStatus() to get the results of the init request. While the
   init is proceding, P2OS will ignore all arm related commands except
   requests for arm status and arm info packets.

   MvrP2Arm::checkArm() can be used to periodicly check to make sure that the
   arm controller is still alive and responding.

   @see checkArm
   @see setPacketCB
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::requestInit()
{
  if (isGood())
  {
    if (comArmInit())
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests that P2OS checks to see if the arm is still alive and immediately
   exits. This is not a full init and differs that P2OS will still accept
   arm commands and the arm will not be parked. If P2OS fails to find the arm
   it will change the status byte accordingly and stop accepting arm related
   commands except for init commands. If the parameter waitForResponse is true
   then checkArm() will wait the appropriate amoutn of time and check the
   status of the arm. If you wish to do the waiting else where the arm check
   sequence takes about 200ms, so the user should wait 300ms then send a
   MvrP2Arm::requestStatus() to get the results of the check arm request. Since
   there is a very noticable time delay, the user should use the 
   MvrP2Arm::setPacketCB() to set a callback so the user knows when the packet
   has been received.

   This can be usefull for telling if the arm is still alive. The arm
   controller can be powered on/off separately from the robot. 

   @param waitForResponse cause the function to block until their is a response
   @see requestInit
   @see setPacketCB
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::checkArm(bool waitForResponse)
{
  MvrTime now;

  if (isGood())
  {
    now.setToNow();
    if (!comArmInfo())
      return(COMM_FAILED);
    if (waitForResponse)
    {
      MvrUtil::sleep(300);
      if (!myLastInfoTime.isAfter(now))
	return(COMM_FAILED);
      if (isGood())
	return(SUCCESS);
      else
	return(NO_ARM_FOUND);
    }
    else
      return(SUCCESS);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Tells the arm to go to the home position. While the arm is homing, the
   status byte will reflect it with the MvrP2Arm::ArmHoming flag. If joint is set
   to -1, then all the joints will be homed at a safe speed. If a single joint
   is specified, that joint will be told to go to its home position at the
   current speed its set at.

   @param joint home only that joint
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::home(int joint)
{
  if (!isGood())
    return(NOT_INITED);

  if ((joint < 0) && !comArmHome(0xff))
    return(COMM_FAILED);
  else if ((joint > 0) && (joint <= NumJoints) && !comArmHome(joint))
    return(COMM_FAILED);
  else
    return(INVALID_JOINT);

  return(SUCCESS);
}

/**
   Move the joint to the position at the given speed. If vel is 0, then the
   currently set speed will be used. The position is in degrees. Each joint
   has about a +-90 degree range, but they all differ due to the design.

   See MvrP2Arm::moveToTicks() for a description of how positions are defined.
   See MvrP2Arm::moveVel() for a description of how speeds are defined.

   @param joint the joint to move
   @param pos the position in degrees to move to
   @param vel the speed at which to move. 0 will use the currently set speed
   @see moveToTicks
   @see moveVel
*/

MVREXPORT MvrP2Arm::State MvrP2Arm::moveTo(int joint, float pos,  unsigned char vel)
{
  unsigned char ticks;

  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  //  if ((vel < 0) && !comArmSpeed(joint, 0-vel))
  //  return(COMM_FAILED);
  else if ((vel > 0) && !comArmSpeed(joint, vel))
    return(COMM_FAILED);

  if (!convertDegToTicks(joint, pos, &ticks))
    return(INVALID_POSITION);

  return(moveToTicks(joint, ticks));
}

/**
   Move the joint to the given position in ticks. A tick is the arbitrary
   position value that the arm controller uses. The arm controller uses a
   single unsigned byte to represent all the possible positions in the range
   of the servo for each joint. So the range of ticks is 0-255 which is 
   mapped to the physical range of the servo. Due to the design of the arm,
   certain joints range are limited by the arm itself. P2OS will bound the
   position to physical range of each joint. This is a lower level of
   controlling the arm position than using MvrP2Arm::moveTo(). MvrP2Arm::moveTo()
   uses a conversion factor which converts degrees to ticks.

   @param joint the joint to move
   @param pos the position, in ticks, to move to
   @see moveTo
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::moveToTicks(int joint, unsigned char pos)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if (!comArmPos(joint, pos))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Step the joint pos degrees from its current position at the given speed.
   If vel is 0, then the currently set speed will be used.

   See MvrP2Arm::moveToTicks() for a description of how positions are defined.
   See MvrP2Arm::moveVel() for a description of how speeds are defined.

   @param joint the joint to move
   @param pos the position in degrees to step
   @param vel the speed at which to move. 0 will use the currently set speed
   @see moveTo
   @see moveVel
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::moveStep(int joint, float pos, unsigned char vel)
{
  unsigned char ticks;

  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  //  if ((vel < 0) && !comArmSpeed(joint, 0-vel))
  //  return(COMM_FAILED);
  else if ((vel > 0) && !comArmSpeed(joint, vel))
    return(COMM_FAILED);

  if (!convertDegToTicks(joint, pos, &ticks))
    return(INVALID_POSITION);

  return(moveStepTicks(joint, ticks));
}

/**
   Move the joint pos ticks from its current position. A tick is the arbitrary
   position value that the arm controller uses. The arm controller uses a
   single unsigned byte to represent all the possible positions in the range
   of the servo for each joint. So the range of ticks is 0-255 which is 
   mapped to the physical range of the servo. Due to the design of the arm,
   certain joints range are limited by the arm itself. P2OS will bound the
   position to physical range of each joint. This is a lower level of
   controlling the arm position than using MvrP2Arm::moveTo(). MvrP2Arm::moveStep()
   uses a conversion factor which converts degrees to ticks.

   @param joint the joint to move
   @param pos the position, in ticks, to move to
   @see moveStep
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::moveStepTicks(int joint, signed char pos)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if (!comArmPos(joint, getJoint(joint)->myPos + pos))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Set the joints velocity. The arm controller has no way of controlling the
   speed of the servos in the arm. So to control the speed of the arm, P2OS
   will incrementaly send a string of position commands to the arm controller
   to get the joint to move to its destination. To vary the speed, the amount
   of time to wait between each point in the path is varied. The velocity
   parameter is simply the number of milliseconds to wait between each point
   in the path. 0 is the fastest and 255 is the slowest. A reasonable range
   is around 10-40. 
   @param joint the joint to move
   @param vel the velocity to move at
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::moveVel(int joint, int vel)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if ((vel < 0) && (!comArmSpeed(joint, 0-vel) || !comArmPos(joint, 0)))
    return(COMM_FAILED);
  else if ((vel > 0) && (!comArmSpeed(joint, vel) || !comArmPos(joint, 255)))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Stop the arm from moving. This overrides all other actions except for the
   arms initilization sequence.
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::stop()
{
  if (!isGood())
    return(NOT_INITED);

  if (!comArmStop())
    return(COMM_FAILED);

  return(SUCCESS);
}

MVREXPORT float MvrP2Arm::getJointPos(int joint)
{
  float val;

  if (isGood() && (joint > 0) && (joint <= NumJoints) &&
      convertTicksToDeg(joint, getJoint(joint)->myPos, &val))
    return(val);
  else
    return(0.0);
}

MVREXPORT unsigned char MvrP2Arm::getJointPosTicks(int joint)
{
  if (isGood() && (joint > 0) && (joint <= NumJoints))
    return(getJoint(joint)->myPos);
  else
    return(0);
}

MVREXPORT P2ArmJoint * MvrP2Arm::getJoint(int joint)
{
  if ((joint > 0) && (joint <= NumJoints))
    return(&myJoints[joint-1]);
  else
    return(0);
}

bool MvrP2Arm::armPacketHandler(MvrRobotPacket *packet)
{
  bool doWake;
  int i;

  if (packet->getID() == ARMpac)
  {
    myLastStatusTime.setToNow();
    myLastStatus=myStatus;
    myStatus=packet->bufToUByte2();
    for (i=1; i<=NumJoints; ++i)
      getJoint(i)->myPos=packet->bufToUByte();

    // Wake up all threads waiting for the arm to stop moving
    for (doWake=false, i=0; i<8; ++i)
    {
      if (((myLastStatus & (1 << i)) != (myStatus & (1 << i))) &&
	  (myStatus & (1 << i)))
	doWake=true;
    }
    if (doWake && myStoppedCB)
      myStoppedCB->invoke();
    if (myPacketCB)
      myPacketCB->invoke(StatusPacket);
    return(true);
  }
  else if (packet->getID() == ARMINFOpac)
  {
    char version[512];
    myLastInfoTime.setToNow();
    packet->bufToStr(version, 512);
    myVersion=version;
    NumJoints=packet->bufToUByte();
    for (i=1; i<=NumJoints; ++i)
    {
      getJoint(i)->myVel=packet->bufToUByte();
      getJoint(i)->myHome=packet->bufToUByte();
      getJoint(i)->myMin=packet->bufToUByte();
      getJoint(i)->myCenter=packet->bufToUByte();
      getJoint(i)->myMax=packet->bufToUByte();
      getJoint(i)->myTicksPer90=packet->bufToUByte();
    }
    if (myPacketCB)
      myPacketCB->invoke(InfoPacket);
    return(true);
    }
  else
    return(false);
}

bool MvrP2Arm::comArmInfo()
{
  return(myRobot->com(ComArmInfo));
}

bool MvrP2Arm::comArmStats(StatusType stats)
{
  return(myRobot->comInt(ComArmStats, (int)stats));
}

bool MvrP2Arm::comArmInit()
{
  return(myRobot->com(ComArmInit));
}

bool MvrP2Arm::comArmCheckArm()
{
  return(myRobot->com(ComArmCheckArm));
}

bool MvrP2Arm::comArmPower(bool on)
{
  if (on)
    return(myRobot->comInt(ComArmPower, 1));
  else
    return(myRobot->comInt(ComArmPower, 0));
}

bool MvrP2Arm::comArmHome(unsigned char joint)
{
  return(myRobot->comInt(ComArmHome, joint));
}

bool MvrP2Arm::comArmPos(unsigned char joint, unsigned char pos)
{
  return(myRobot->com2Bytes(ComArmPos, joint, pos));
}

bool MvrP2Arm::comArmSpeed(unsigned char joint, unsigned char speed)
{
  return(myRobot->com2Bytes(ComArmSpeed, joint, speed));
}

bool MvrP2Arm::comArmStop(unsigned char joint)
{
  return(myRobot->comInt(ComArmStop, joint));
}

bool MvrP2Arm::comArmPark()
{
  return(myRobot->com(ComArmPark));
}

bool MvrP2Arm::comArmAutoPark(int waitSecs)
{
  return(myRobot->comInt(ComArmAutoPark, waitSecs));
}

bool MvrP2Arm::comArmGripperPark(int waitSecs)
{
  return(myRobot->comInt(ComArmGripperPark, waitSecs));
}

MVREXPORT bool MvrP2Arm::getMoving(int joint)
{
  if ((joint < 0) && (myStatus & 0xf))
    return(true);
  else if (myStatus & (1 << joint))
    return(true);
  else
    return(false);
}

MVREXPORT bool MvrP2Arm::isPowered()
{
  if (myStatus & ArmPower)
    return(true);
  else
    return(false);
}

MVREXPORT bool MvrP2Arm::isGood()
{
  if (myRobot && myRobot->isRunning() && myRobot->isConnected() &&
      myInited && (myStatus & ArmGood) && (myStatus & ArmInited))
    return(true);
  else
    return(false);
}

MVREXPORT MvrP2Arm::State MvrP2Arm::park()
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmPark())
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   P2OS will automaticly park the arm if it gets no arm related packets after
   waitSecs. This is to help protect the arm when the program looses
   connection with P2OS. Set the value to 0 to disable this timer. Default
   wait is 10 minutes.

   @param waitSecs seconds to wait till parking the arm when idle
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::setAutoParkTimer(int waitSecs)
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmAutoPark(waitSecs))
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   P2OS/AROS automatically park the gripper after its been closed for more than
   waitSecs. The gripper servo can overheat and burnout if it is holding
   something for more than 10 minutes. Care must be taken to ensure that this
   does not happen. If you wish to manage the gripper yourself, you can
   disable this timer by setting it to 0.

   @param waitSecs seconds to wait till parking the gripper once it has begun to grip something
*/
MVREXPORT MvrP2Arm::State MvrP2Arm::setGripperParkTimer(int waitSecs)
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmGripperPark(waitSecs))
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

MVREXPORT bool MvrP2Arm::convertDegToTicks(int joint, float pos,
				       unsigned char *ticks)
{
  long val;

  if ((joint <= 0) || (joint > NumJoints))
    return(false);

  if (joint == 6)
    *ticks=(unsigned char)pos;
  else
  {
    val=MvrMath::roundInt(getJoint(joint)->myTicksPer90*(pos/90.0));
    if ((joint >= 1) && (joint <= 3))
      val=-val;
    val+=getJoint(joint)->myCenter;
    if (val < getJoint(joint)->myMin)
      *ticks=getJoint(joint)->myMin;
    else if (val > getJoint(joint)->myMax)
      *ticks=getJoint(joint)->myMax;
    else
      *ticks=val;
  }

  return(true);
}

MVREXPORT bool MvrP2Arm::convertTicksToDeg(int joint, unsigned char pos,
				       float *degrees)
{
  long val;

  if ((joint <= 0) || (joint > NumJoints))
    return(false);

  if (joint == 6)
    *degrees=pos;
  else
  {
    val=MvrMath::roundInt(90.0/getJoint(joint)->myTicksPer90*
			 (pos-getJoint(joint)->myCenter));
    if ((joint >= 1) && (joint <= 3))
      val=-val;
    *degrees=val;
  }

  return(true);
}

MVREXPORT P2ArmJoint::P2ArmJoint() :
  myPos(0),
  myVel(0),
  myHome(0),
  myMin(0),
  myCenter(0),
  myMax(0),
  myTicksPer90(0)
{
}

MVREXPORT P2ArmJoint::~P2ArmJoint()
{
}

