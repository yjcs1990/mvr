/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobot.cpp
 > Description  : Central class for communicating with and operating the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月22日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include <time.h>
#include <ctype.h>

#include "MvrRobot.h"
#include "MvrLog.h"
#include "MvrDeviceConnection.h"
#include "MvrTcpConnection.h"
#include "MvrSerialConnection.h"
#include "MvrLogFileConnection.h"
#include "mvriaUtil.h"
#include "MvrSocket.h"
#include "MvrCommands.h"
#include "MvrRobotTypes.h"
#include "MvrSignalHandler.h"
#include "MvrPriorityResolver.h"
#include "MvrAction.h"
#include "MvrRangeDevice.h"
#include "MvrRobotConfigPacketReader.h"
#include "MvrRobotBatteryPacketReader.h"
#include "mvriaInternal.h"
#include "MvrLaser.h"
#include "MvrBatteryMTX.h"
#include "MvrSonarMTX.h"
#include "MvrLCDMTX.h"

/*
 * The parameters only rarely need to be specified.
 *
 * @param name A name for this robot, useful if a program has more than one
 * MvrRobot object 
 * @param obsolete This parameter is ignored.

 * @param doSigHandle do normal signal handling and have this robot
 * instance stopRunning() when the program is signaled

 * @param normalInit whether the robot should initializes its
 * structures or the calling program will take care of it.  No one
 * will probalby ever use this value, since if they are doing that
 * then overriding will probably be more useful, but there it is.

 * @param addMvriaExitCallback If true (default), add callback to global Mvria class
 * to stop running the processing loop and disconnect from robot 
 * when Mvria::exit() is called. If false, do not disconnect on Mvria::exit()
 */
MVREXPORT MvrRobot::MvrRobot(const char *name, bool obsolete, bool doSigHandle, bool normalInit, bool addMvriaExitCallback) :
          myMotorPacketCB(this, &MvrRobot::processMotorPacket),
          myEncoderPacketCB(this, &MvrRobot::processEncoderPacket),
          myIOPacketCB(this, &MvrRobot::processIOPacket),
          myPacketHandlerCB(this, &MvrRobot::packetHandler),
          myActionHandlerCB(this, &MvrRobot::actionHandler),
          myStateReflectorCB(this, &MvrRobot::stateReflector),
          myRobotLockerCB(this, &MvrRobot::robotLocker),
          myRobotUnlockerCB(this, &MvrRobot::robotUnlocker),
          myKeyHandlerExitCB(this, &MvrRobot::keyHandlerExit),
          myGetCycleWarningTimeCB(this, &MvrRobot::getCycleWarningTime),
          myGetNoTimeWarningThisCycleCB(this, &MvrRobot::getNoTimeWarningThisCycle),
          myBatteryAverager(20),
          myRealBatteryAverager(20),
          myMvriaExitCB(this, &Mvria::mvriaExitCallback),
          myPoseInterpPositionCB(this, &MvrRobot::getPoseInterpPosition),
          myEncoderPoseInterpPositionCB(this, &MvrRobot::getEncoderPoseInterpPositionCallback)
{
  myMutex.setLogName("MvrRobot::myMutex");
  myPacketMutex.setLogName("MvrRobot::myPacketMutex");
  myConnectionTimeoutMutex.setLogName("MvrRobot::myConnectionTimeoutMutex");

  setName(name);
  myMvriaExitCB.setName("MvrRobotExit");
  myNoTimeWarningThisCycle = false;
  myGlobalPose.setPose(0, 0, 0);
  mySetEncoderTransformCBList.setName("SetEncoderTransformCBList");

  myParam = new MvrRobotGeneric("");
  processParamFile();

  myRunningNonThreaded = true;

  myMotorPacketCB.setName("MvrRobot::motorPacket");
  myEncoderPacketCB.setName("MvrRobot::encoderPacket");
  myIOPacketCB.setName("MvrRobot::IOPacket");

  myInterpolation.setName("MvrRobot::Interpolation");
  myEncoderInterpolation.setName("MvrRobot::EncoderInterpolation");

  myPtz           = NULL;
  myKeyHandler    = NULL;
  myKeyHandlerCB  = NULL;
  myMTXTimeUSecCB = NULL;

  myConn = NULL;

  myOwnTheResolver = false;

  myBlockingConnectRun = false;
  myAsyncConnectFlag   = false;

  myRequireConfigPacket= false;

  myLogMovementSent       = false;
  myLogMovementReceived   =false;
  myLogVelocitiesReceived = false;
  myLogActions            = false;
  myLastVel               = 0;
  myLastRotVal            = 0;
  myLastHeading           = 0;
  myLastDeltaHeading      = 0;

  myKeepControlRaw = false;

  myPacketsSentTracking = false;
  myPacketsReceivedTracking = false;
  myPacketsReceivedTrackingCount = false;
  myPacketsReceivedTrackingStarted.setToNow();

  myLogSIPContents = false;

  myCycleTime         = 100;
  myCycleWarningTime  = 250;
  myConnectionCycleMultiplier = 2;
  myTimeoutTime       = 8000;
  myStabilizingTime   = 0;
  mycounter           = 1;
  myResolver          = NULL;
  myNumSonar          = 0;

  myCycleChained      = true;

  myMoveDoneDist      = 40;
  myHeadingDoneDiff   = 3;

  myStoppedVel        = 4;
  myStoppedRotVel     = 1;
  myStoppedLatVel     = 4;

  myOrigRobotConfig   = NULL;
  myBatteryPacketReader = NULL;

  reset();
  if (normalInit)
    init();
  
  mySyncLoop.setRobot(this);
  myPacketReader.setRobot(this);

  if (doSigHandle)
    Mvria::addRobot(this);
  if (addMvriaExitCallback)
  {
    Mvria::addExitCallback(&myMvriaExitCB, 0);
    myAddedMvriaExitCB = true;
  }
  else
  {
    myAddedMvriaExitCB = false;
  }

  myConnectWithNoParams = false;
  myDoNotSwitchBaud     = false;

  myPacketReceivedCondition.setLogName("MvrRobot::myPacketReceivedCondition");
  myConnectCond.setLogName("MvrRobot::myConnectCond");
  myConnOrFailCond.setLogName("MvrRobot::myConnOrFailCond");
  myRunExitCond.setLogName("MvrRobot::myRunExitCond");
}