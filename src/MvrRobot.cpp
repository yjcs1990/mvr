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

MVREXPORT MvrRobot::~MvrRobot()
{
  MvrResolver::ActionMap::iterator it;

  stopRunning();
  delete mySyncTaskRoot;
  MvrUTil::deleteSetPairs(mySonars.begin(), mySonars.end());
  Mvria::delRobot(this);

  if (myKeyHandlerCB != NULL)
    delete myKeyHandlerCB;
  
  for (it = myActions.begin(); it != myActions.end(); ++it)
  {
    (*it).second->setRobot();
  }
}

/*
 * Sets up the packet handlers, sets up the sync list and make the default priority resolver
 */
MVREXPORT voide MvrRobot::init(void)
{
  setUpPacketHandlers();
  setUpSyncList();
  myOwnTheResolver = true;
  myResolution = new MvrPriorityResolver;
}

/*
 * This starts the ongoing main loop, which invokes robot tasks untio
 * stopped.
 * This function does not return until the loop is stopped by a call to stopRunning()
 * of if 'true' is given for @param stopRunIfNotConnected, and the robot connection
 * is closed or fails.
 * @param stopRunIfNotConnected if true, the run will return if there
 * is no connection to the robot at any given point, this is good for
 * one-shot programs... if it is false the run won't return unless
 * stop is called on the instance

 * @param runNonThreaded if true, the robot won't make the usual
 * thread that it makes for reading the packets...you generally
 * shouldn't use this, but it's provided in case folks are using
 * programs without threading 
 */
MVREXPORT void MvrRobot::run(bool stopRunIfNotConnected, bool runNonThreaded)
{
  if (mySyncLoop.getRunning)
  {
    MvrLog::log(MvrLog::Terse,
                "The robot is already running, cannot run it again");
    return ;
  }
  mySyncLoop.setRunning(true);
  mySyncLoop.stopRunIfNotConnected(stopRunIfNotConnected);
  mySyncLoop.runInThisThread();

  myRunningNonThreaded = runNonThreaded;
  if (!runNonThreaded)
    // Joinable, but do NOT lower priority. The robot packet reader
    // thread is the most important and around. since all the timing
    // is based off the one.
    myPacketReader.create(true, false);
}

/*
 * @return true if the robot is currently running in a run or runAsync otherwise false
 */
MVREXPORT bool MvrRobot::isRunning(void) const
{
  return (mySyncLoop.getRunning() && (myRunningNonThreaded || myPacketReader.getRunning()));
}

/*
 * This starts a new thread then has runs through the tasks until stopped
 * This function doesn't return until something calls stop on the instance
 * This function returns immediately
 * @param stopRunIfNotConnected if true, the run will stop if there is no
 * connection to the robot at any given point, this is good for one-shot
 * params. If false then the thread will continue until stopRunning() is called.
 * @param runNonThreadedPacketReader selects whether the packet reader object
 * (receives and parses packets from the robot) should run in its own internal
 * asychronous thread. Mostly for internal use.
 */
MVREXPORT void MvrRobot::runAsync(bool stopRunIfNotConnected, bool runNonThreadedPacketReader)
{
  if (mySyncLoop.getRunning)
  {
    MvrLog::log(MvrLog::Terse,
                "The robot is already running, cannot run it again");
    return ;    
  }
  if (runNonThreadedPacketReader)
  {
    mySyncLoop.stopRunIfNotConnected(stopRunIfNotConnected);
    // Joinable, but do NOT lower priority. The robot thread is the most
    // important one around, (this isn't true anymore, since the robot
    // packet reading one is more important for timing)
    mySyncLoop.create(true, false);
    myRunningNonThreaded = true;
  }
  else
  {
    myRunningNonThreaded = false;
    // Joinable, but do NOT lower priority. The robot thread is the most
    // important one around, (this isn't true anymore, since the robot
    // packet reading one is more important for timing)
    mySyncLoop.create(true, true);
    // Joinable, but do NOT lower priority. The robot packet reader
    // thread is the most important one around, since all the timing is
    // based off that one.
    myPacketReader.create(true, false);
  }
}

/*
 * This stops this robot's processing cycle.  If it is running
 * in a background thread (from runAsync()), it will cause that thread
 * to exit.  If it is running synchronously (from run()), then run() will
 * return.  
 * @param doDisconnect If true, also disconnect from the robot connection (default is true). 
 */
MVREXPORT void MvrRobot::stopRunning(bool doDisconnect)
{
  if (myKeyHandler != NULL)
    myKeyHandler->restore();
  mySyncLoop.stopRunning();
  myBlockingConnectRun = false;
  if (doDisconnect &&(isConnected() || myIsStabilizing))
  {
    waitForRunExit();
    disconnect();
  }
  wakeAllWaitingThreads();
}

MVREXPORT void MvrRobot::setUpSyncList(void)
{
  mySyncTaskRoot = new MvrSyncTask("SyncTasks");
  mySyncTaskRoot->setWarningTimeCB(&myGetCycleWarningTimeCB);
  mySyncTaskRoot->setNoTimeWarningCB(&myGetNoTimeWarningThisCycleCB);
  mySyncTaskRoot->addNewLeaf("Packet Handler",85, &myPacketHandlerCB);
  mySyncTaskRoot->addNewLeaf("Robot Locker", 70, &myRobotLockerCB);
  mySyncTaskRoot->addNewBranch("Sensor Interp", 65);
  mySyncTaskRoot->addNewLeaf("Action Handler", 55, &myActionHandlerCB);
  mySyncTaskRoot->addNewLeaf("State Reflection", 45, &myStateReflectorCB);
  mySyncTaskRoot->addNewBranch("User Tasks", 25);
  mySyncTaskRoot->addNewLeaf("Robot unlock", 20, &myRobotUnlockerCB);
}

MVREXPORT void MvrRobot::setUpPacketHandlers(void)
{
  addPacketHandler(&myMotorPacketCB, MvrListPos::FIRST);
  addPacketHandler(&myEncoderPacketCB, MvrList::LAST);
  addPacketHandler(&myIOPacketCB, MvrListPos::LAST);
}

void MvrRobot::reset(void)
{
  resetTripOdometer();

  myOdometerDistance = 0;
  myOdometerDegrees  = 0;
  myOdometerStart.setToNow();

  myIgnoreMicroControllerBatteryInfo = false;

  myHaveStateOfCharge = false;
  myStateOfCharge = 0;
  myStateOfChargeLow = 0;
  myStateOfChargeShutdown = 0;
  myInterpolation.reset();
  myEncoderInterpolation.reset();

  if (myOrigRobotConfig != NULL)
    delete myOrigRobotConfig;
  myOrigRobotConfig = new MvrRobotConfigPacketReader(this, true);
  if (myBatteryPacketReader != NULL)
    delete myBatteryPacketReader;
  myBatteryPacketReader = new MvrRobotBatteryPacketReader(this);

  myFirstEncoderPose      = true;
  myFakeFirstEncoderPose  = false;
  myIgnoreNextPacket      = false;
  //myEncoderPose.setPose(0, 0, 0);
  //myEncoderPoseTaken.setToNow();
  //myRawEncoderPose.setPose(0, 0, 0);
  //myGlobalPose.setPose(0, 0, 0);
  //myEncoderTransform.setTransform(myEncoderPose, myGlobalPose);
  myTransVelMax     = 0;
  myTransNegVelMax  = 0;
  myTransAccel      = 0;
  myTransDecel      = 0;
  myRotVelMax       = 0;
  myRotAccel        = 0;
  myRotDecel        = 0;
  myLatVelMax       = 0;
  myLatAccel        = 0;
  myLatDecel        = 0;

  myAbsoluteMaxTransVel     = 0;
  myAbsoluteMaxTransNegVel  = 0;
  myAbsoluteMaxTransAccel   = 0;
  myAbsoluteMaxTransDecel   = 0;
  myAbsoluteMaxRotVel       = 0;
  myAbsoluteMaxRotAccel     = 0;
  myAbsoluteMaxRotDecel     = 0;
  myAbsoluteMaxLatVel       = 0;
  myAbsoluteMaxLatAccel     = 0;
  myAbsoluteMaxLatDecel     = 0;

  myLeftVel   = 0;
  myRightVel  = 0;
  myBatteryVoltage      = 13;
  myRealBatteryAverager = 13;
  myBatteryAverager.clear();
  myBatteryAverager.add(myBatteryVoltage);

  myStallValue  = 0;
  myControl     = 0;
  myFlags       = 0;
  myFaultFlags  = 0;
  myHasFaultFlags = 0;
  myCompass     = 0;
  myAnalogPortSelected = 0;
  myAnalog      = 0;
  myDigIn       = 0;
  myDigOut      = 0;
  myIOAnalogSize = 0;
  myIODigInSize  = 0;
  myIODigOutSize = 0;
  myLastIOPacketReceivedTime.setSec(0);
  myLastIOPacketReceivedTime.setMSec(0);
  myVel     = 0;
  myRotVel  = 0;
  myLatVel  = 0;
  myOverriddenChargeState = false;
  myChargeState = CHARGING_UNKNOWN;
  myOverriddenIsChargerPowerGood = false;

  myLastX = 0;
  myLastY = 0;
  myLastTh = 0;
  mySentPulse = false;
  myIsConnected = false;
  myIsStabilizing = false;

  myTransVal      = 0;
  myTransVal2     = 0;
  myTransType     = TRANS_NONE;
  myLastTransVal  = 0;
  myLastTransVal2 = 0;
  myLastTransType = TRANS_NONE;
  myLastTransSent.setToNow();
  myActionTransSet = false;

  myLastActionRotStopped = false;
  myLastActionRotHeading = false;
  myRotVal = 0;
  myLastRotVal = 0;
  myRotType = ROT_NONE;
  myLastRotType = ROT_NONE;
  myLastRotSent.setToNow();
  myActionRotSet = false;

  myLatVal  = 0;
  myLatType = LAT_NONE;
  myLastLatVal  = 0;
  myLastLatType = LAT_NONE;
  myLastLatSent.setToNow();
  myActionLatSet = false;

  myLastSentTransVelMax  = 0;
  myLastSentTransAccel   = 0;
  myLastSentTransDecel   = 0;
  myLastSentRotVelMax    = 0;
  myLastSentRotVelPosMax = 0;
  myLastSentRotVelNegMax = 0;
  myLastSentRotAccel     = 0;
  myLastSentRotDecel     = 0;
  myLastSentLatVelMax    = 0;
  myLastSentLatAccel     = 0;
  myLastSentLatDecel     = 0;

  myLastPulseSent.setToNow();

  myDirectPrecedenceTime = 0;
  myStateReflectionRefreshTime = 500;

  myActionDesired.reset();

  myMotorPacCurrentCount = 0;
  myMotorPacCount        = 0;
  myTimeLastMotorPacket  = 0;

  mySonarPacCurrentCount = 0;
  mySonarPacCount        = 0;
  myTimeLastSonarPacket  = 0;

  myLeftEncoder  = 0;
  myRightEncoder = 0;

  myTemperature = -128;

  myConnectionOpenedTime.setSecLL(0);
  myConnectionOpenedTime.setMSecLL(0);

  mySonarEnabled = true;
  myAutonomousDrivingSonarEnabled = false;
}

MVREXPORT void MvrRobot::setTransVelMax(double vel)
{
  if (vel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransVelMax of %g is below 0, ignoring it", vel);
    return;
  }

  if (vel > myAbsoluteMaxTransVel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransVelMax of %g is over the absolute max vel of %g, capping it", vel, myAbsoluteMaxTransVel);
    vel = myAbsoluteMaxTransVel;
  }
  myTransVelMax = vel;
}

MVREXPORT void MvrRobot::setTransNegVelMax(double negVel)
{
  if (negVel >= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransNegVelMax of %g is greater than 0, ignoring it",negVelvel);
    return;
  }

  if (negVel < myAbsoluteMaxTransNegVel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransNegVelMax of %g is over the absolute max vel of %g, capping it", negVel, myAbsoluteMaxTransNegVel);
    negVal = myAbsoluteMaxTransNegVel;
  }
  myTransNegVelMax = negVel; 
}

MVREXPORT void MvrRobot::setTransAccel(double acc)
{
  if (acc <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransAccel of %g is below 0, ignoring it", acc);
    return;
  }
  
  if (acc > myAbsoluteMaxTransAccel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransAccel of %g is over the absolute max of %g, capping it", acc, myAbsoluteMaxTransAccel);
    acc = myAbsoluteMaxTransAccel;
  }
  myTransAccel = acc;
}

MVREXPORT void MvrRobot::setTransDecel(double decel)
{
  if (decel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransDecel of %g is below 0, ignoring it", decel);
    return;
  }

  if (fabs(decel) > myAbsoluteMaxTransDecel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransDecel of %g is over the absolute max of %g, capping it", decel, myAbsoluteMaxTransDecel);
    decel = myAbsoluteMaxTransDecel;
  }
  myTransDecel = MvrMath::fabs(decel);
}

MVREXPORT void MvrRobot::setRotVelMax(double vel)
{
  if (vel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setRotVelMax of %g is below 0, ignoring it", vel);
    return;
  }

  if (vel > myAbsoluteMaxRotVel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: rotVelMax of %g is over the absolute max of %g, capping it", vel, myAbsoluteMaxRotVel);
    vel = myAbsoluteMaxRotVel;
  }
  myRotVelMax = vel;
}

MVREXPORT void MvrRobot::setRotAccel(double acc)
{
  if (acc <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setRotAccel of %g is below 0, ignoring it", acc);
    return;
  }

  if (acc > myAbsoluteMaxRotAccel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setRotAccel of %g is over the absolute max of %g, capping it", acc, myAbsoluteMaxRotAccel);
    acc = myAbsoluteMaxRotAccel;
  }
  myRotAccel = acc;
}

MVREXPORT void MvrRobot::setRotDecel(double decel)
{
  if (decel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setRotDecel of %g is below 0, ignoring it", decel);
    return;
  }

  if (fabs(decel) > myAbsoluteMaxRotDecel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setRotDecel of %g is over the absolute max of %g, capping it", decel, myAbsoluteMaxRotDecel);
    decel = myAbsoluteMaxRotDecel;
  }
  myRotDecel = MvrMath::fabs(decel);
}

MVREXPORT void MvrRobot::setLatVelMax(double vel)
{
  if (vel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatVelMax of %g is below 0, ignoring it", vel);
    return;
  }

  if (vel > myAbsoluteMaxLatVel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatVelMax of %g is over the absolute max of %g, capping it", vel, myAbsoluteMaxLatVel);
    vel = myAbsoluteMaxLatVel;
  }
  myLatVelMax = vel;
}

MVREXPORT void MvrRobot::setLatAccel(double acc)
{
  if (acc <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatAccel of %g is below 0, ignoring it", acc);
    return;
  }

  if (acc > myAbsoluteMaxLatAccel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatAccel of %g is over the absolute max of %g, capping it", acc, myAbsoluteMaxLatAccel);
    acc = myAbsoluteMaxLatAccel;
  }
  myLatAccel = acc;
}

MVREXPORT void MvrRobot::setLatDecel(double decel)
{
  if (decel <= 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatDecel of %g is below 0, ignoring it", decel);
    return;
  }

  if (fabs(decel) > myAbsoluteMaxLatDecel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setLatDecel of %g is over the absolute max of %g, capping it", decel, myAbsoluteMaxLatDecel);
    decel = myAbsoluteMaxLatDecel;
  }
  myLatDecel = MvrMath::fabs(decel);
}


MVREXPORT double MvrRobot::getTransVelMax(void) const
{
  return myTransVelMax;
}

MVREXPORT double MvrRobot::getTransNegVelMax(void) const
{
  return myTransNegVelMax;
}

MVREXPORT double MvrRobot::getTransAccel(void) const
{
  return myTransAccel;
}

MVREXPORT double MvrRobot::getTransDecel(void) const
{
  return myTransDecel;
}

MVREXPORT double MvrRobot::getRotVelMax(void) const
{
  return myRotVelMax;
}

MVREXPORT double MvrRobot::getRotAccel(void) const
{
  return myRotAccel;
}

MVREXPORT double MvrRobot::getRotDecel(void) const
{
  return myRotDecel;
}

MVREXPORT double MvrRobot::getLatVelMax(void) const
{
  return myLatVelMax;
}

MVREXPORT double MvrRobot::getLatAccel(void) const
{
  return myLatAccel;
}

MVREXPORT double MvrRobot::getLatDecel(void) const
{
  return myLatDecel;
}

/*
 * Sets the connection this instance uses to the actual robot.  This is where
 * commands will be sent and packets will be received from
 * @param connection The deviceConnection to use for this robot
 */
 