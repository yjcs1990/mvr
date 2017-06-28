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
    MvrLog::log(MvrLog::Terse, "The robot is already running, cannot run it again");
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
    MvrLog::log(MvrLog::Terse, "The robot is already running, cannot run it again");
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
 MVREXPORT void MvrRobot::setDeviceConnection(MvrDevieConnection *connection)
 {
   myConn = connection;
   myConn->setDeviceName("uC");
   mySender.setDeviceConnection(myConn);
   myReceiver.setDeviceConnection(myConn);
 }


/*
 * Gets the connection this instance uses to the actual robot.  This is where
 * commands will be sent and packets will be received from
 * @return the deviceConnection used for this robot
 */
MVREXPORT MvrDeviceConnection *MvrRobot::getDeviceConnection(void) const
{
  return myConn;
}

/*
 * Sets the number of milliseconds to go without a response from the robot
 * until it is assumed that the connection with the robot has been
 * broken and the disconnect on error events will happen.  Note that
 * this will only happen with the default packet handler.

 * @param mSecs if seconds is 0 then the connection timeout feature
 * will be disabled, otherwise disconnect on error will be triggered
 * after this number of seconds...  
 */
MVREXPORT void MvrRobot::setConnectionTimeoutTime(int mSecs)
{
  myConnectionTimeoutMutex.lock();

  MvrLog::log(MvrLog::Normal, "MvrRobot::setConnectionTimeoutTime: Setting timeout to %d mSecs", mSecs);
  myLastOdometryReceivedTime.setToNow();
  myLastPacketReceivedTime.setToNow();

  if (mSecs > 0)
    myTimeoutTime = mSecs;
  else
    myTimeoutTime = 0;
  
  myConnectionTimeoutMutex.unlock();
}
/*
 * Get the time at which the most recent packet of any type was received
 * This timestamp is updated whenever any packet is received from the robot.
 * @return the time the last packet was received
 */
MVREXPORT MvrTime MvrRobot::getLastPacketTime(void)
{
  myConnectionTimeoutMutex.lock();
  MvrTime ret = myLastPacketReceivedTime;
  myConnectionTimeoutMutex.unlock();
  return ret;
}

/*
 * Get the time at which thet most recent SIP packet was received.
 * Whenever a SIP packet (aka "motor" packet)  is received, this time is set to the receiption
 * time minus the robot's reported internal odometry delay. The SIP provides
 * robot position (see getPose()) and some other information (see robot
 * manual for details).

 * @return the time the last SIP was received
 */
MVREXPORT MvrTime MvrRobot::getLastOdometryTime(void)
{
  myConnectionTimeoutMutex.lock();
  MvrTime ret = myLastOdometryReceivedTime;
  myConnectionTimeoutMutex.unlock();
  return ret;  
}

/* 
 * Sets up the robot to connect, then returns, but the robot must be
 * running (ie from runAsync) before you do this.  

 * Most programs should use MvrRobotConnector to perform the connection
 * rather than using thin method directly.

 * This function will fail if the robot is already connected. If you 
 * want to know what happened because of the connect then look at
 * the callbacks.  
 * NOTE:
 * this will not lock robot before setting values, so you MUST lock
 * the robot before you call this function and unlock the robot after
 * you call this function.  If you fail to lock the robot, you'll may
 * wind up with wierd behavior.  Other than the aspect of blocking or
 * not the only difference between async and blocking connects (other
 * than the blocking) is that async is run every robot cycle, whereas
 * blocking runs as fast as it can... also blocking will try to
 * reconnect a radio modem if it looks like it didn't get connected
 * in the first place, so blocking can wind up taking 10 or 12
 * seconds to decide it can't connect, whereas async doesn't try hard
 * at all to reconnect the radio modem (beyond its first try) (under
 * the assumption the async connect is user driven, so they'll just
 * try again, and so that it won't mess up the sync loop by blocking
 * for so long).

 * @return true if the robot is running and the robot will try to
 * connect, false if the robot isn't running so won't try to connect
 * or if the robot is already connected
 */
MVREXPORT bool MvrRobot::asyncConnect(void)
{
  if (!mySyncLoop.getRunning() || isConnected())
    return false;
  
  myAsyncConnectFlag    = true;
  myBlockingConnectRun  = false;
  myAsyncConnectState   = -1;
  return true;
}

/*
 * Connects to the robot, returning only when a connection has been
 * made or it has been established a connection can't be made.  

 * Most programs should use an MvrRobotConnector object to perform
 * the connection rather than using this method directly.

 * This
 * connection usually is fast, but can take up to 30 seconds if the
 * robot is in a wierd state (this is not often).  If the robot is
 * connected via MvrSerialConnection then the connect will also
 * connect the radio modems.  Upon a successful connection all of the
 * Connection Callback Functors that have been registered will be
 * called.  NOTE, this will lock the robot before setting values, so
 * you MUST not have the robot locked from where you call this
 * function.  If you do, you'll wind up in a deadlock.  This behavior
 * is there because otherwise you'd have to lock the robot before
 * calling this function, and normally blockingConnect will be called
 * from a separate thread, and that thread won't be doing anything
 * else with the robot at that time.  Other than the aspect of
 * blocking or not the only difference between async and blocking
 * connects (other than the blocking) is that async is run every
 * robot cycle, whereas blocking runs as fast as it can... also
 * blocking will try to reconnect a radio modem if it looks like it
 * didn't get connected in the first place, so blocking can wind up
 * taking 10 or 12 seconds to decide it can't connect, whereas async
 * doesn't try hard at all to reconnect the radio modem (under the
 * assumption the async connect is user driven, so they'll just try
 * again, and so that it won't mess up the sync loop by blocking for
 * so long).
 * @return true if a connection could be made, false otherwise 
 */
MVREXPORT bool MvrRobot::blockingConnect(void)
{
  int ret = 0;
  lock();
  if (myIsConnected)
    disconnect();
  myBlockingConnectRun = true;
  myAsyncConnectState  = -1;
  while ((ret = asyncConnectHandler(true)) == 0 && myBlockingConnectRun)
    MvrUtil::sleep(MvrUtil::roundInt(getCycleTime() * 0.80));
  unlock();
  if (ret == 1)
    return true;
  else
    return false;
}

/*
 * This is an internal function that is used both for async connects and 
 * blocking connects use to connect. It does about the same thing for both
 * and it should only be used by asyncConnect and blockingConnect really.
 * But here it is.  The only difference between when its being used 
 * by blocking/async connect is that in blocking mode if it thinks there
 * may be problems with the radio modem it pauses for two seconds trying
 * to deal with this... whereas in async mode it tries to deal with this in a
 * simpler way.
 * @param tryHarderToConnect if this is true, then if the radio modems look
 * like they aren't working, it'll take about 2 seconds to try and connect 
 * them, whereas if its false, it'll do a little try, but won't try very hard
 * @return 0 if its still trying to connect, 1 if it connected, 2 if it failed
 */
MVREXPORT int MvrRobot::asyncConnectHandler(bool tryHarderToConnect)
{
  int ret = 0;
  MvrRobotPacket *packet;
  MvrRobotPacket *tempPacket = NULL;
  char robotSubType[255];
  int i;
  int len;
  std::string str;
  MvrTime endTime;
  int timeToWait;
  MvrSerialConnection *serConn;

  endTime.setToNow();
  endTime.addMSec(getCycleTime() * myConnectionCycleMultiplier);

  myNoTimeWarningThisCycle = true;

  // if this is -1, then we're doing initialization stuff, then returning
  if (myAsyncConnectState == -1)
  {
    myAsyncConnectSentChangeBaud = false;
    myAsyncConnectNoPacketCount  = 0;
    myAsyncConnectTimesTried     = 0;
    reset();
    if (myConn == NULL)
    {
      MvrLog::log(MvrLog::Terse, "Cannot connect, no connection has been set.");
      failedConnect();
      return 2;
    }
    if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
    {
      if (!myConn->openSimple())
      {
        MvrLog::log(MvrLog::Terse, "Counld not connect, because open on the device connection failed.");
        failedConnect();
        return 2;
      }
    }
    // check for log file connection: just return success
    if (dynamic_cast<MvrLogFileConnection *>(myConn))
    {
      MvrLogFileConnection *con = dynamic_cast<MvrLogFileConnection *>(myConn);
      myRobotName = con->myName;
      myRobotType = con->myType;
      myRobotSubType = con->mySubType;
      if (con->havePose)      // we know where to move
        moveTo(con->myPose);
      setCycleChained(false); // don't process packets all at once
      madeConnection();
      finishedConnection();
      return 1;
    }

    // needed for the old infowave radio modem
    if (dynamic_cast<MvrSerialConnection *>(myConn))
    {
      myConn->write("WMS2\15", strlen("WMS2\15"));
    }

    // here we're flushing out all previously received packets
    while (endTime.mSecTo() > 0 && myReceiver.receivePacket(0) != NULL);

    myAsyncConnectState = 0;
    return 0;
  }

  if (myAsyncConnectState >= 3)
  {
    bool handled;
    std::list<MvrRetFunctor1<bool, MvrRobotPacket *> *>::iterator it;
    while ((packet = myReceiver.receivePacket(0)) != NULL)
    {
      for (handled = false, it = myPacketHandlerList.begin();
           it != myPacketHandlerList.end() && handled == false;
           it++)
      {
        if ((*it) != NULL && (*it)->invokeR(packet))
          handled = true;
        else
          packet->resetRead();
      }     
    }
  }

  if (myAsyncConnectState == 3)
  {
    if (!myOrigRobotConfig > hasPacketArrived())
    {
      myOrigRobotConfig->requestPacket();
    }
    // if we've gotten our config packet or if we've timed out then
    // set our vel and acc/decel param and skip to the next part
    if (myOrigRobotConfig->hasPacketArrived() || myAsyncStartedConnection.mSecSince() > 1000)
    {
      bool gotConfig;
      // if we have data from the robot use that
      if (myOrigRobotConfig->hasPacketArrived())
      {
        gotConfig = true;
        setAbsoluteMaxTransVel(myOrigRobotConfig->getTransVelTop());
        setAbsoluteMaxTransNegVel(-myOrigRobotConfig->getTransVelTop());
        setAbsoluteMaxTransAccel(myOrigRobotConfig->getTransAccelTop());
        setAbsoluteMaxTransDecel(myOrigRobotConfig->getTransAccelTop());
        setAbsoluteMaxRotVel(myOrigRobotConfig->getRotVelTop());
        setAbsoluteMaxRotAccel(myOrigRobotConfig->getRotAccelTop());
        setAbsoluteMaxRotDecel(myOrigRobotConfig->getRotAccelTop());
        setTransVelMax(myOrigRobotConfig->getTransVelMax());
        setTransNegVelMax(-myOrigRobotConfig->getTransVelMax());
        setTransAccel(myOrigRobotConfig->getTransAccel());
        setTransDecel(myOrigRobotConfig->getTransDecel());
        setRotVelMax(myOrigRobotConfig->getRotVelMax());
        setRotAccel(myOrigRobotConfig->getRotAccel());
        setRotDecel(myOrigRobotConfig->getRotDecel());
        if (hasLatVel())
        {
          setAbsoluteMaxLatVel(myOrigRobotConfig->getLatVelTop());
          setAbsoluteMaxLatAccel(myOrigRobotConfig->getLatAccelTop());
          setAbsoluteMaxLatDecel(myOrigRobotConfig->getLatAccelTop());
          setLatVelMax(myOrigRobotConfig->getLatVelMax());
          setLatAccel(myOrigRobotConfig->getLatAccel());
          setLatDecel(myOrigRobotConfig->getLatDecel());
        }
        if (myOrigRobotConfig->getKinematicsDelay() != 0)
          setOdometryDelay(myOrigRobotConfig->getKinematicsDelay());
        if (myOrigRobotConfig->getStateOfChargeLow() > 0)
          setStateOfChargeLow(myOrigRobotConfig->getStateOfChargeLow());
        if (myOrigRobotConfig->getStateOfChargeShutdown() > 0)
          setStateOfChargeShutdown(myOrigRobotConfig->getStateOfChargeShutdown());
        MvrLog::log(MvrLog::Normal, "Robot Serial Number: %s", myOrigRobotConfig->getSerialNumber());
        }
      else if (myRequireConfigPacket)
      {
        MvrLog::log(MvrLog::Terse, "Could not connect, config packet required and no config packet received.");
        failedConnect();
        return 2;
      }
      // if our absolute maximums weren't set then set them
      else
      {
        gotConfig = false;
        setAbsoluteMaxTransVel(myParams->getAbsoluteMaxVelocity());
        setAbsoluteMaxTransAccel(1000);
        setAbsoluteMaxTransDecel(1000);
        setAbsoluteMaxRotVel(myParams->getAbsoluteMaxRotVelocity());
        setAbsoluteMaxRotAccel(100);
        setAbsoluteMaxRotDecel(100);
        setAbsoluteMaxLatVel(myParams->getAbsoluteMaxLatVelocity());
        setAbsoluteMaxLatAccel(1000);
        setAbsoluteMaxLatDecel(1000);
      }
      // okay we got in that data, now put over it any of the things
      // that we got from the robot parameter file... if we don't have
      // max vels from above or the parameters then use the absolutes
      // which we do have for everything
      if (MvrMath::fabs(myParams->getTransVelMax()) > 1)
        setTransVelMax(myParams->getTransVelMax());
      else if (!gotConfig)
        setTransVelMax(myParams->getAbsoluteMaxVelocity());
      if (MvrMath::fabs(myParams->getTransVelMax()) > 1)
        setTransNegVelMax(-myParams->getTransVelMax());
      else if (!gotConfig)
        setTransNegVelMax(-myParams->getAbsoluteMaxVelocity());
      if (MvrMath::fabs(myParams->getRotVelMax()) > 1)
        setRotVelMax(myParams->getRotVelMax());
      else if (!gotConfig)
        setRotVelMax(myParams->getAbsoluteMaxRotVelocity());
      if (MvrMath::fabs(myParams->getTransAccel()) > 1)
        setTransAccel(myParams->getTransAccel());
      if (MvrMath::fabs(myParams->getTransDecel()) > 1)
        setTransDecel(myParams->getTransDecel());
      if (MvrMath::fabs(myParams->getRotAccel()) > 1)
        setRotAccel(myParams->getRotAccel());
      if (MvrMath::fabs(myParams->getRotDecel()) > 1)
        setRotDecel(myParams->getRotDecel());

      if (MvrMath::fabs(myParams->getLatVelMax()) > 1)
        setLatVelMax(myParams->getLatVelMax());
      else if (!gotConfig)
        setLatVelMax(myParams->getAbsoluteMaxLatVelocity());
      if (MvrMath::fabs(myParams->getLatAccel()) > 1)
        setLatAccel(myParams->getLatAccel());
      if (MvrMath::fabs(myParams->getLatDecel()) > 1)
        setLatDecel(myParams->getLatDecel());
      myAsyncConnectState = 4;
    }
    else
      return 0;
  }
  // we want to see if we should switch baud
  if (myAsyncConnectState == 4)
  {
    serConn = dynamic_cast<MvrSerialConnection *>(myConn);
    // if we didn't get a config packet or we can't change the baud or
    // we shouldn't change the baud or if we'd change it to a slower
    // baud rate or we aren't using a serial port then don't switch
    // the baud
    if (!myOrigRobotConfig->hasPacketArrived() || !myOrigRobotConfig->getResetBaud() || 
        serConn == NULL || myParams->getSwitchToBaudRate() == 0 || 
	      (serConn != NULL && serConn->getBaud() >= myParams->getSwitchToBaudRate()) || myDoNotSwitchBaud)
    {
      // if we're using a serial connection store our baud rate
      if (serConn != NULL)
	      myAsyncConnectStartBaud = serConn->getBaud();
      myAsyncConnectState = 5;
    }
    // if we did get a packet and can change baud send the command
    else if (!myAsyncConnectSentChangeBaud)
    {
      // if we're using a serial connection store our baud rate
      if (serConn != NULL)
        myAsyncConnectStartBaud = serConn->getBaud();

      int baudNum = -1;
      
      // first suck up all the packets we have now
      while ((packet = myReceiver.receivePacket(0)) != NULL);
      if (myParams->getSwitchToBaudRate() == 9600)
	      baudNum = 0;
      else if (myParams->getSwitchToBaudRate() == 19200)
	      baudNum = 1;
      else if (myParams->getSwitchToBaudRate() == 38400)
        baudNum = 2;
      else if (myParams->getSwitchToBaudRate() == 57600)
        baudNum = 3;
      else if (myParams->getSwitchToBaudRate() == 115200)
        baudNum = 4;
      else
      {
      	MvrLog::log(MvrLog::Normal,
                    "Warning: SwitchToBaud is set to %d baud, ignoring.",
      		          myParams->getSwitchToBaudRate());
      	MvrLog::log(MvrLog::Normal, "\tGood bauds are 9600 19200 38400 56800 115200.");
      	myAsyncConnectState = 5;
      	baudNum = -1;
      	return 0;
      }
      if (baudNum != -1)
      {
        // now switch it over
        comInt(MvrCommands::HOSTBAUD, baudNum);
        MvrUtil::sleep(10);
        myAsyncConnectSentChangeBaud = true;
        myAsyncConnectStartedChangeBaud.setToNow();
        serConn->setBaud(myParams->getSwitchToBaudRate());
        //serConn->setBaud(19200);
        MvrUtil::sleep(10);
        com(0);
        return 0;
      }
    }
    // if we did send the command then wait and see if we get any packets back
    else
    {
      packet = myReceiver.receivePacket(100);
      com(0);
      // if we got any packet we're good
      if (packet != NULL)
      {
        myAsyncConnectState = 5;
      }
      // if we didn't get it and its been 500 ms then fail
      else if (myAsyncConnectStartedChangeBaud.mSecSince() > 900)
      {
        MvrLog::log(MvrLog::Normal, "Controller did not switch to baud, reset to %d baud.", myAsyncConnectStartBaud);
        serConn->setBaud(myAsyncConnectStartBaud);
        myAsyncConnectState = 5;
      }
      else
        return 0;
    }    
  }

  if (myAsyncConnectState == 5)
  {
    if (!myIsStabilizing)
      startStabilization();
    if (myStabilizingTime == 0 || myStartedStabilizing.mSecSince() > myStabilizingTime)
      {
        finishedConnection();
        return 1;
      }
    else
      return 0;
  }

  // here we've gone beyond the initialization, so read set a time limit,
  // read in one packet, then if its a bad packet type, read in all the 
  // packets there are to read... if its a good packet, continue with sequence
  myAsyncConnectTimesTried++;
  MvrLog::log(MvrLog::Normal, "Syncing %d", myAsyncConnectState);
  mySender.com(myAsyncConnectState);
  //  packet = myReceiver.receivePacket(endTime.mSecTo());
  packet = myReceiver.receivePacket(1000);
  
  if (packet != NULL) 
  {
    ret = packet->getID();

    //printf("Got a packet %d\n", ret);
	
    if (ret == 50)
    {
      MvrLog::log(MvrLog::Normal, "Attempting to close previous connection.");
      comInt(MvrCommands::CLOSE,1);
      while (endTime.mSecTo() > 0)
      {
        timeToWait = endTime.mSecTo();
        if (timeToWait < 0)
          timeToWait = 0;
        tempPacket = myReceiver.receivePacket(timeToWait);
        if (tempPacket != NULL)
          MvrLog::log(MvrLog::Verbose, "Got in another packet!");
        if (tempPacket != NULL && tempPacket->getID() == 50)
          comInt(MvrCommands::CLOSE,1);
      }
      myAsyncConnectState = 0;
    } 
    else if (ret == 255)
    {
      MvrLog::log(MvrLog::Normal, "Attempting to correct syncCount");
      /*
      while (endTime.mSecTo() > 0)
      {
        timeToWait = endTime.mSecTo();
        if (timeToWait < 0)
          timeToWait = 0;
        tempPacket = myReceiver.receivePacket(timeToWait);
        if (tempPacket != NULL)
          MvrLog::log(MvrLog::Verbose, "Got in another packet!");
      }
      */
      while ((tempPacket = myReceiver.receivePacket(0)) != NULL);

      if (tempPacket != NULL && tempPacket->getID() == 0)
        myAsyncConnectState = 1;
      else
        myAsyncConnectState = 0;
      return 0;
    } 
    else if  (ret != myAsyncConnectState++)
    {
      myAsyncConnectState = 0;
    }
  }
  else 
  {
    MvrLog::log(MvrLog::Normal, "No packet.");
    myAsyncConnectNoPacketCount++;
    if ((myAsyncConnectNoPacketCount > 5
	 && myAsyncConnectNoPacketCount >= myAsyncConnectTimesTried) || (myAsyncConnectNoPacketCount > 10))
    {
      MvrLog::log(MvrLog::Terse, "Could not connect, no robot responding.");
      failedConnect();
      return 2;
    }

          
    // If we get no response first we dump close commands at the thing
    // in different bauds (if its a serial connection)
    if (myAsyncConnectNoPacketCount == 2 && 
        myAsyncConnectNoPacketCount >= myAsyncConnectTimesTried &&
        (serConn = dynamic_cast<MvrSerialConnection *>(myConn)) != NULL)
    {
      int origBaud;
      MvrLog::log(MvrLog::Normal, "Trying to close possible old connection");
      origBaud = serConn->getBaud();
      serConn->setBaud(9600);
      comInt(MvrCommands::CLOSE,1);
      MvrUtil::sleep(3);
      serConn->setBaud(38400);
      comInt(MvrCommands::CLOSE,1);
      MvrUtil::sleep(3);
      serConn->setBaud(115200);
      comInt(MvrCommands::CLOSE,1);
      MvrUtil::sleep(3);
      serConn->setBaud(19200);
      comInt(MvrCommands::CLOSE,1);
      MvrUtil::sleep(3);
      serConn->setBaud(57600);
      comInt(MvrCommands::CLOSE,1);
      MvrUtil::sleep(3);
      serConn->setBaud(origBaud);
    }

    if (myAsyncConnectNoPacketCount > 3)
    {
      MvrLog::log(MvrLog::Normal, "Robot may be connected but not open, trying to dislodge.");
      mySender.comInt(MvrCommands::OPEN,1);
    }
    
    myAsyncConnectState = 0;
  }
  // if we've connected and have a packet get the connection
  // information from it
  if (myAsyncConnectState == 3 && packet != NULL) 
  {
    char nameBuf[512];
    MvrLog::log(MvrLog::Terse, "Connected to robot.");
    packet->bufToStr(nameBuf, 512);
    myRobotName = nameBuf;
    MvrLog::log(MvrLog::Normal, "Name: %s", myRobotName.c_str());
    packet->bufToStr(nameBuf, 512);
    myRobotType = nameBuf;
    MvrLog::log(MvrLog::Normal, "Type: %s", myRobotType.c_str());
    packet->bufToStr(nameBuf, 512);
    myRobotSubType = nameBuf;
    strcpy(robotSubType, myRobotSubType.c_str());
    len = strlen(robotSubType);
    for (i = 0; i < len; i++)
      robotSubType[i] = tolower(robotSubType[i]);
    myRobotSubType = robotSubType;
    MvrLog::log(MvrLog::Normal, "Subtype: %s", myRobotSubType.c_str());
    MvrUtil::sleep(getCycleTime());
    mySender.comInt(MvrCommands::OPEN,1);
    if (!madeConnection())
    {
      mySender.comInt(MvrCommands::CLOSE,1);
      failedConnect();
      return 2;
    }

    // now we return off so we can handle the rest of connecting, if
    // you're just using this for your own connections you can skip
    // this part because its really connected now
    myAsyncStartedConnection.setToNow();
    return asyncConnectHandler(tryHarderToConnect);
  }
  if (myAsyncConnectTimesTried > 50)
  {
    failedConnect();
    return 2;
  }
  return 0;
}

/*
 * @return true if the file could be loaded, false othrewise
 */ 
MVREXPORT bool MvrRobot::loadParamFile(const char *fileName)
{
  if (myParams != NULL)
    delete myParams;
  
  myParams = new MvrRobotGeneric("");
  if (!myParams->parseFile(fileName, false, true))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrRobot::loadParamFile: Could not find file '%s' to load", file);
    return false;
  }
  processParamFile();
  MvrLog::log(MvrLog::Normal, "Loaded robot parameters from %s.", fileName);
  return true;
}

/*
 * This function will take over ownership of the passed in params
 */
MVREXPORT void MvrRobot::setRobotParams(MvrRobotParams *params)
{
  if (myParams != NULL)
    delete myParams;
  
  myParams = params;
  processParamFile();
  MvrLog::log(MvrLog::Verbose, "Took new passed in robot params.");
}

MVREXPORT void MvrRobot::processParamFile(void)
{
  for (int i=0; i< myParams->getNumSonar(); ++i)
  {
    if (mySonars.find(i) == mySonars.end())
    {
      MvrLog::log(MvrLog::Verbose,
                  "MvrRobot::processParamFile creating new sonar %d %d %d %d",
                  i, myParams->getSonarX(i), myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i] = new MvrSensorReading(myParams->getSonarX(i), myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i]->setIgnoreThisReading(true);
    }
    else
    {
      MvrLog::log(MvrLog::Verbose,
                  "MvrRobot::processParamFile resetting sonar %d %d %d %d",
                  i, myParams->getSonarX(i), myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i] = new MvrSensorReading(myParams->getSonarX(i), myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i]->setIgnoreThisReading(true);      
    }
    if ((i + 1) > myNumSonar)
      myNumSonar = i + 1;
  }

  if (!MvrRobotParams::internalGetUseDefaultBehavior())
  {
    setAbsoluteMaxTransVel(myParams->getAbsoluteMaxVelocity());
    setAbsoluteMaxTransNegVel(myParams->getAbsoluteMaxVelocity());
    setAbsoluteMaxRotVel(myParams->getAbsoluteMaxRotVelocity());
    setAbsoluteMaxLatVel(myParams->getAbsoluteMaxLatVelocity());    
  }

  if (myParams->getRobotLengthFront() == 0)
    myRobotLengthFront = myParams->getRobotLength() / 2.0;
  else
    myRobotLengthFront = myParams->getRobotLengthFront();

  if (myParams->getRobotLengthRear() == 0)
    myRobotLengthRear = myParams->getRobotLength() / 2.0;
  else
    myRobotLengthRear = myParams->getRobotLengthRear();
}

MVREXPORT bool MvrRobot::madeConnection(bool resetConnectionTime)
{
  if (resetConnectionTime)
    myConnectionOpenedTime.setToNow();
  
  if (!MvrRobotParams::internalGetUseDefaultBehavior())
  {
    MvrLog::log(MvrLog::Normal, "Explicitly not loading MVRIA robot parameter files");
    myRobotType = myParams->getClassName();
    myRobotSubType = myParams->getSubClassName();
    processParamFile();
    return true;
  }

  std::string subtypeParamFileName;
  std::string nameParamFileName;
  bool loadedSubTypeParam;
  bool loadedNameParam;
  bool hadDefault = true;

  if (myParams != NULL)
    delete myParams;
  
  // Find the robot parameters to load and get them into the structure we have
    if (MvrUtil::strcasecmp(myRobotSubType, "p2dx") == 0)
      myParams = new MvrRobotP2DX;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2ce") == 0)
      myParams = new MvrRobotP2CE;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2de") == 0)
      myParams = new MvrRobotP2DXe;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2df") == 0)
      myParams = new MvrRobotP2DF;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2d8") == 0)
      myParams = new MvrRobotP2D8;
    else if (MvrUtil::strcasecmp(myRobotSubType, "amigo") == 0)
      myParams = new MvrRobotAmigo;
    else if (MvrUtil::strcasecmp(myRobotSubType, "amigo-sh") == 0)
      myParams = new MvrRobotAmigoSh;
    else if (MvrUtil::strcasecmp(myRobotSubType, "amigo-sh-tim5xx") == 0)
      myParams = new MvrRobotAmigoShTim5xxWibox;
    else if (MvrUtil::strcasecmp(myRobotSubType, "amigo-sh-tim3xx") == 0)
      myParams = new MvrRobotAmigoShTim5xxWibox;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2at") == 0)
      myParams = new MvrRobotP2AT;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2at8") == 0)
      myParams = new MvrRobotP2AT8;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2it") == 0)
      myParams = new MvrRobotP2IT;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2pb") == 0)
      myParams = new MvrRobotP2PB;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2pp") == 0)
      myParams = new MvrRobotP2PP;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p3at") == 0)
      myParams = new MvrRobotP3AT;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p3dx") == 0)
      myParams = new MvrRobotP3DX;
    else if (MvrUtil::strcasecmp(myRobotSubType, "perfpb") == 0)
      myParams = new MvrRobotPerfPB;
    else if (MvrUtil::strcasecmp(myRobotSubType, "pion1m") == 0)
      myParams = new MvrRobotPion1M;
    else if (MvrUtil::strcasecmp(myRobotSubType, "pion1x") == 0)
      myParams = new MvrRobotPion1X;
    else if (MvrUtil::strcasecmp(myRobotSubType, "psos1m") == 0)
      myParams = new MvrRobotPsos1M;
    else if (MvrUtil::strcasecmp(myRobotSubType, "psos43m") == 0)
      myParams = new MvrRobotPsos43M;
    else if (MvrUtil::strcasecmp(myRobotSubType, "psos1x") == 0)
      myParams = new MvrRobotPsos1X;
    else if (MvrUtil::strcasecmp(myRobotSubType, "pionat") == 0)
      myParams = new MvrRobotPionAT;
    else if (MvrUtil::strcasecmp(myRobotSubType, "mappr") == 0)
      myParams = new MvrRobotMapper;
    else if (MvrUtil::strcasecmp(myRobotSubType, "powerbot") == 0)
      myParams = new MvrRobotPowerBot;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2d8+") == 0)
      myParams = new MvrRobotP2D8Plus;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p2at8+") == 0)
      myParams = new MvrRobotP2AT8Plus;
    else if (MvrUtil::strcasecmp(myRobotSubType, "perfpb+") == 0)
      myParams = new MvrRobotPerfPBPlus;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p3dx-sh") == 0)
      myParams = new MvrRobotP3DXSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p3at-sh") == 0)
      myParams = new MvrRobotP3ATSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "p3atiw-sh") == 0)
      myParams = new MvrRobotP3ATIWSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "patrolbot-sh") == 0)
      myParams = new MvrRobotPatrolBotSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "peoplebot-sh") == 0)
      myParams = new MvrRobotPeopleBotSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "powerbot-sh") == 0)
      myParams = new MvrRobotPowerBotSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "wheelchair-sh") == 0)
      myParams = new MvrRobotWheelchairSH;
    else if (MvrUtil::strcasecmp(myRobotSubType, "seekur") == 0)
      myParams = new MvrRobotSeekur;
    else if (MvrUtil::strcasecmp(myRobotSubType, "powerbot-sh-uarcs") == 0)
      myParams = new MvrRobotPowerBotSHuARCS;
    else if (MvrUtil::strcasecmp(myRobotSubType, "mt400") == 0)
      myParams = new MvrRobotMT400;
    else if (MvrUtil::strcasecmp(myRobotSubType, "researchPB") == 0)
      myParams = new MvrRobotResearchPB;
    else if (MvrUtil::strcasecmp(myRobotSubType, "seekurjr") == 0)
      myParams = new MvrRobotSeekurJr;  
    else if(MvrUtil::strcasecmp(myRobotSubType, "p3dx-sh-lms1xx") == 0)
      myParams = new MvrRobotP3DXSH_lms1xx;
    else if(MvrUtil::strcasecmp(myRobotSubType, "p3at-sh-lms1xx") == 0)
      myParams = new MvrRobotP3ATSH_lms1xx;
    else if(MvrUtil::strcasecmp(myRobotSubType, "peoplebot-sh-lms1xx") == 0)
      myParams = new MvrRobotPeopleBotSH_lms1xx;
    else if(MvrUtil::strcasecmp(myRobotSubType, "p3dx-sh-lms500") == 0)
      myParams = new MvrRobotP3DXSH_lms500;
    else if(MvrUtil::strcasecmp(myRobotSubType, "p3at-sh-lms500") == 0)
      myParams = new MvrRobotP3ATSH_lms500;
    else if(MvrUtil::strcasecmp(myRobotSubType, "peoplebot-sh-lms500") == 0)
      myParams = new MvrRobotPeopleBotSH_lms500;
    else if(MvrUtil::strcasecmp(myRobotSubType, "powerbot-sh-lms500") == 0)
      myParams = new MvrRobotPowerBotSH_lms500;
    else if(MvrUtil::strcasecmp(myRobotSubType, "researchPB-lms500") == 0)
      myParams = new MvrRobotResearchPB_lms500;
    else if(MvrUtil::strcasecmp(myRobotSubType, "pioneer-lx") == 0 ||  // real type for Pioneer LX
      MvrUtil::strcasecmp(myRobotSubType, "lx") == 0 ||   // subtype used in MobileSim 0.7.2
      MvrUtil::strcasecmp(myRobotSubType, "marc_devel") == 0 || // subtype used in early versions of MVRCOS firmware
      MvrUtil::strcasecmp(myRobotSubType, "lynx") == 0
    )
      myParams = new MvrRobotPioneerLX;
    else
    {
      hadDefault = false;
      myParams = new MvrRobotGeneric(myRobotName.c_str());
    }

    // load up the param file for the subtype
    subtypeParamFileName = Mvria::getDirectory();
    subtypeParamFileName += "params/";

  if (MvrUtil::strcasecmp(myRobotSubType, "marc_devel") == 0)
  {
    MvrLog::log(MvrLog::Verbose, "Note, Using pioneer-lx.p as parameter file for marc_devel robot subtype.");
    subtypeParamFileName += "pioneer-lx";
  }
  else
  {
    subtypeParamFileName += myRobotSubType;
  }

  subtypeParamFileName += ".p";
  if ((loadedSubTypeParam = myParams->parseFile(subtypeParamFileName.c_str(), true, true)))
      MvrLog::log(MvrLog::Normal, "Loaded robot parameters from %s", subtypeParamFileName.c_str());
  /* If the above line was replaced with this one line
     paramFile->load(); 
     then the sonartest (and lots of other stuff probably) would break
  */
  // then the one for the particular name, if we can
  nameParamFileName = Mvria::getDirectory();
  nameParamFileName += "params/";
  nameParamFileName += myRobotName;
  nameParamFileName += ".p";
  if ((loadedNameParam = myParams->parseFile(nameParamFileName.c_str(), true, true)))
  {
    if (loadedSubTypeParam)
      MvrLog::log(MvrLog::Normal, 
                  "Loaded robot parameters from %s on top of %s robot parameters", 
                  nameParamFileName.c_str(), subtypeParamFileName.c_str());
    else
      MvrLog::log(MvrLog::Normal, "Loaded robot parameters from %s", 
		 nameParamFileName.c_str());
  }
   
  if (!loadedSubTypeParam && !loadedNameParam)
  {
    if (hadDefault)
      MvrLog::log(MvrLog::Normal, "Using default parameters for a %s robot", 
		 myRobotSubType.c_str());
    else
    {
      MvrLog::log(MvrLog::Terse, "Error: Have no parameters for this robot, bad configuration or out of date Mvria");
      // in the default state (not connecting if we don't have params)
      // we will return false... if we can connect without params then
      // we'll keep going (this really shouldn't be used except by
      // downloaders and such)
      if (!myConnectWithNoParams)
	      return false;
    }
  }

  processParamFile();

  if (myParams->getRequestIOPackets() || myRequestedIOPackets)
  {
    myRequestedIOPackets = true;
    comInt(MvrCommands::IOREQUEST, 2);
  }

  if(myParams->getRequestEncoderPackets() || myRequestedEncoderPackets)
  {
    myRequestedEncoderPackets = true;
    comInt(MvrCommands::ENCODER, 2);
  }

  return true;
}

/* Encoder packet data is stored by MvrRobot and can be read using
 * getLeftEncoder() and getRightEncoder().
 *
 *  Encoder packets may be stopped with stopEncoderPackets().
 *
 * @note Encoder data is not available with all robot types.  It is currently
 * only available from robots with SH controllers running MVRCOS (Pioneer 3,
 * PowerBot, PeopleBot, AmigoBot).  If a robot does not support encoder data,
 * this request will be ignored.
 * 
 * Encoder packets are sent after the main SIP (motor packet) by MVRCOS. If you want to 
 * handle encoder packets immediately upon reception, add a general robot packet handler to
 * MvrRobot with addPacketHandler(), and check the packet for the encoder packet ID
 * 0x90. See addPacketHandler() for details on how to write a general packet
 * handler.
 */
MVREXPORT void MvrRobot::requestEncoderPackets(void)
{
  comInt(MvrCommands::ENCODER, 2);
  myRequestedEncoderPackets = true;
}

/*
 * Note that not all robots have built-in GPIO, and so will not return any
 * response to this request.  Refer to your robot's operations manual for
 * information on whether and what kind of digital and analog IO interface
 * it has.  (Pioneer 3, PeopleBot and PowerBot 
 * have digital IO built in to the robot.  On Pioneer LX, use MvrMTXIO instead.
 * If running on Linux on a Versalogic EBX-12 computer, MvrVersalogicIO can be
 * used to access the EBX-12 computer's IO interface.  Otherwise, a user-added
 * peripheral DAC interface is suggested.  )
 */
MVREXPORT void MvrRobot::requestIOPackets(void)
{
  comInt(MvrCommands::IOREQUEST, 2);
  myRequestedIOPackets = true;
}

/**
  @see requestEncoderPackets()
*/
MVREXPORT void MvrRobot::stopEncoderPackets(void)
{
  comInt(MvrCommands::ENCODER, 0);
  myRequestedEncoderPackets = false;
}

MVREXPORT void MvrRobot::stopIOPackets(void)
{
  comInt(MvrCommands::IOREQUEST, 0);
  myRequestedIOPackets = false;
}


MVREXPORT bool MvrRobot::haveRequestedEncoderPackets(void)
{
  return myRequestedEncoderPackets;
}

MVREXPORT bool MvrRobot::haveRequestedIOPackets(void)
{
  return myRequestedIOPackets;
}

MVREXPORT void MvrRobot::startStabilization(void)
{
  std::list<MvrFunctor *>::iterator it;
  myIsStabilizing = true;
  myStartedStabilizing.setToNow();

  for (it = myStabilizingCBList.begin();
       it != myStabilizingCBList.end();
       it++)
    (*it)->invoke();
}

MVREXPORT void MvrRobot::finishedConnection(void)
{
  std::list<MvrFunctor *>::iterator it;

  myIsStabilizing = false;
  myIsConnected   = true;
  myAsyncConnectFlag = false;
  myBlockingConnectRun = false;
  resetTripOdometer();

  for (it = myConnectCBList.begin();
       it != myConnectCBList.end();
       it++)
    (*it)->invoke();

  myConnectionTimeoutMutex.lock();
  myLastPacketReceivedTime.setToNow();
  myLastOdometryReceivedTime.setToNow();
  myConnectionTimeoutMutex.unlock();

  wakeAllConnWaitingThreads();       
}

MVREXPORT void MvrRobot::failedConnect(void)
{
  std::list<MvrFunctor *>::iterator it;

  myAsyncConnectFlag = false;
  myBlockingConnectRun = false;
  MvrLog::log(MvrLog::Terse, "Failed to connect to robot.");
  myIsConnected = false;
  for (it = myFailedConnectCBList.begin();
       it != myFailedConnectCBList.end();
       it++)
    (*it)->invoke();
  
  if (myConn != NULL)
    myConn->close();
    wakeAllConnOrFailWaitingThreads();
}

/* 
 * Disconnects from a robot.  This also calls of the DisconnectNormally 
 * Callback Functors if the robot was actually connected to a robot when 
 * this member was called. Then it disables the robot motors, sends CLOSE
 * command to robot, waits one seconds, and closes the communications connection.
 * This disconnection normally happens automatically when the program exits
 * by calling Mvria::exit(), or if communication to the robot fails during 
 * the run cycle, and this method does not need to be explicitly called.
 * @return true if not connected to a robot (so no disconnect can happen, but 
 * it didn't failed either), also true if the command could be sent to the 
 * robot (ie connection hasn't failed)
 */
MVREXPORT bool MvrRobot::disconnect(void)
{
  std::list<MvrFunctor *>::iterator it;
  bool ret;  
  MvrSerialConnection *serConn;

  if (!myIsConnected && !myIsStabilizing)
    return true;
  
  MvrLog::log(MvrLog::Terse, "Disconnecting from robot.");
  if (myIsConnected)
  {
    for (it = myDisconnectNormallyCBList.begin();
         it != myDisconnectNormallyCBList.end();
         it++)
      (*it)->invoke();
  }
  myNoTimeWarningThisCycle = true;
  myIsConnected = false;
  myIsStabilizing = false;
  mySender.comInt(MvrCommands::ENABLE, 0);
  MvrUtil::sleep(100);
  ret = mySender.comInt(MvrCommands::CLOSE, 1);
  MvrUtil::sleep(1000);
  if (ret == true)
  {
    if (myConn != NULL)
    {
      ret = myConn->close();
      if ((serConn = dynamic_cast<MvrSerialConnection *>(myConn)) != NULL)
        serConn->setBaud(myAsyncConnectStartBaud);
    }
    else
      ret = false;
  }
  else if (myConn != NULL)
    myConn->close();
  
  return ret;
}

MVREXPORT void MvrRobot::dropConnection(const char *technicalReason, const char *userReason)
{
  std::list<MvrFunctor *>::iterator it;

  if (!myIsConnected)
    return;
  if (technicalReason != NULL)
    myDropConnectionReason = technicalReason;
  else
    myDropConnectionReason = "Lost connection to the microcontroller because of unknown error.";
  
  if (userReason != NULL)
    myDropConnectionUserReason = userReason;
  else
    myDropConnectionUserReason = myDropConnectionReason;
  
  MvrLog::log(MvrLog::Terse, myDropConnectionReason.c_str());
  myIsConnected = false;
  for (it = myDisconnectOnErrorCBList.begin();
       it != myDisconnectOnErrorCBList.end();
       it++)
    (*it)->invoke();
  
  if (myConn != NULL)
    myConn->close();
  return;
}

MVREXPORT void MvrRobot::cancelConnection(void)
{
  MvrLog::log(MvrLog::Verbose, "Cancelled connection to the microcontroller because of command.");
  myIsConnected = false;
  myNoTimeWarningThisCycle = true;
  myIsStabilizing = false;
  return;
}

/*
 * Stops the robot, by telling it to have a translational velocity and 
 * rotational velocity of 0.  Also note that if you are using actions, this 
 * will cause the actions to be ignored until the direct motion precedence 
 * timeout has been exceeded or clearDirectMotion is called.
 * @see setDirectMotionPrecedenceTime
 * @see getDirectMotionPrecedenceTime
 * @see clearDirectMotion
 */
MVREXPORT void MvrRobot::stop(void)
{
  comInt(MvrCommands::VEL, 0);
  comInt(MvrCommands::RVEL, 0);
  if (hasLatVel())
    comInt(MvrCommands::LATVEL, 0);
  setVel(0);
  setRotVel(0);
  setLatVel(0);
  myLastActionTransVal   = 0;
  myLastActionRotStopped = true;
  myLastActionRotHeading = false;
  myLastActionLatVal     = 0;
}

/*
 * Sets the desired translational velocity of the robot. 
 * MvrRobot caches this value, and sends it with a VEL command during the next
 * cycle. 
 * The value is rounded to the nearest whole number mm/sec before
 * sent to the robot controller.

 * @param velocity the desired translational velocity of the robot (mm/sec)
 */
MVREXPORT void MvrRobot::setVal(double velocity)
{
  myTransType = TRANS_VEL;
  myTransVal  = velocity;
  myTransVal2 = 0;
  myTransSetTime.setToNow();
}

/*
 * Sets the desired lateral (sideways on local Y axis) translational velocity of the robot. 
 * MvrRobot caches this value, and sends it with a LATVEL command during the next cycle.
 * (Only has effect on robots that are capable of translating laterally, i.e.
 * Seekur. Other robots will ignore the command.)
 * The value is rounded to the nearest whole number mm/sec before
 * sent to the robot controller.

 * @param latVelocity the desired translational velocity of the robot (mm/sec)
 */
MVREXPORT void MvrRobot::setLatVel(double latVelocity)
{
  myLatType = LAT_VEL;
  myLatVal  = latVelocity;
  myLatSetTime.setToNow();
}

/* 
 * Sends command to set the robot's velocity differentially, with
 * separate velocities for left and right sides of the robot. The
 * MvrRobot caches these values, and sends them with 
 * a VEL2 command during the next
 * cycle.  Note that this cancels both translational velocity AND
 * rotational velocity, and is canceled by any of the other direct
 * motion commands.  
 * The values are rounded to the nearest whole number mm/sec before
 * sent to the robot controller.

 * @sa setVel
 * @sa setRotVel
 * @sa clearDirectMotion

 * @deprecated

 * @param leftVelocity the desired velocity of the left wheel 
 * @param rightVelocity the desired velocity of the right wheel
 */
MVREXPORT void MvrRobot::setVel2(double leftVelocity, double rightVelocity)
{
  myTransType = TRANS_VEL2;
  myTransVal  = leftVelocity;
  myTransVal2 = rightVelocity;
  myRotType   = ROT_IGNORE;
  myRotVal    = 0;
  myTransSetTime.setToNow();
}

/*
 * Tells the robot to begin moving the specified distance forward/backwards.
 * MvrRobot caches this value, and sends it during the next cycle (with a MOVE
 * or VEL command, depending on the robot).

 * @param distance the distance for the robot to move (millimeters). Note, 
 * due to the implementation of the MOVE command on some robots, it is best to
 * restrict the distance to the range (5000mm, 5000mm] if possible. Not all
 * robots have this restriction, however.
 */
MVREXPORT void MvrRobot::move(double distance)
{
  myTransType = TRANS_DIST_NEW;
  myTransDistStart = getPose();
  myTransVal  = distance;
  mytransval2 = 0;
  myTransSetTime.setToNow();
}

/*
 * Determines if a move command is finished, to within a small
 * distance threshold, "delta".  If delta = 0 (default), the delta distance 
 * set with setMoveDoneDist() will be used.

 * @param delta how close to the goal distance the robot must be, or 0 for previously set default

 * @return true if the robot has finished the distance given in a move
 * command or if the robot is no longer in a move mode (because its
 * now running off of actions, or setVel() or setVel2() was called).
 */
MVREXPORT bool MvrRobot::isMoveDone(double delta)
{
  if (fabs(delta) < 0.001)
    delta = myMoveDoneDist;
  if (myTransType != TRANS_DIST && myTransType != TRANS_DIST_NEW)
    return true;
  if (myTransDistStart.findDistanceTo(getPose()) < fabs(myTransVal) - delta)
    return false;
  return true;
}

/*
 * Determines if a setHeading() command is finished, to within a small
 * distance.  If delta = 0 (default), the delta distance used is that which was
 * set with setHeadingDoneDiff() (you can get that distnace value with
 * getHeadingDoneDiff(), the default is 3).

 * @param delta how close to the goal distance the robot must be

 * @return true if the robot has achieved the heading given in a move
 * command or if the robot is no longer in heading mode mode (because
 * its now running off of actions, setDHeading(), or setRotVel() was called).
 */
MVREXPORT bool MvrRobot::isHeadingDone(double delta) const
{
  if (fabs(delta) < 0.001)
    delta = myHeadingDoneDiff;
  if (myRotType != ROT_HEADING)
    return true;
  if (fabs(MvrMath::subAngle(getTh(), myRotVal)) > delta)
    return false;
  return true;
}

/*
 * Sets the heading of the robot, it caches this value, and sends it
 * during the next cycle.  The heading value is rounded to the nearest
 * whole degree before sending to the robot.

 * @param heading the desired heading of the robot (degrees)
 */
MVREXPORT void MvrRobot::setHeading(double heading)
{
  myRotVal = heading;
  myRotType = ROT_HEADING;
  myRotSetTime.setToNow();
  if (myTransType == TRANS_VEL)
  {
    myTransType = TRANS_IGNORE;
    myTransVal  = 0;
    myTransVal2 = 0;
  }
}

/*
 * Sets the rotational velocity of the robot, it caches this value,
 * and during the next command cycle, the value is rounded to the 
 * nearest whole integer degree, and sent to the robot as an RVEL command.

 * @param velocity the desired rotational velocity of the robot (deg/sec)
 */
MVREXPORT void MvrRobot::setRotVel(double velocity)
{
  myRotVal = velocity;
  myRotType = ROT_VEL:
  myRotSetTime.setToNow();
  if (myTransType == TRANS_VEL2)
  {
    myTransType = TRANS_IGNORE;
    myTransVal = 0;
    myTransVal2 = 0;
  }
}

/*
 * Sets a delta heading to the robot, it caches this value, and sends
 * it during the next cycle.  

 * @param deltaHeading the desired amount to change the heading of the robot by
 */
MVREXPORT void MvrRobot::setDeltaHeading(double deltaHeading)
{
  myRotVal = MvrMath::addAngle(getTh(), deltaHeading);
  myRotType = ROT_HEADING;
  myRotSetTime.setToNow();
  if (myTransType == TRANS_VEL2)
  {
    myTransType = TRANS_IGNORE;
    myTransVal = 0;
    myTransVal2 = 0;
  }
}

MVREXPORT bool MvrRobot::isStopped(double stoppedVel, double stoppedRotVel, double stoppedLatVel)
{
  if (stoppedVel < 0.001)
    stoppedVel = myStoppedVel;
  if (stoppedRotVel < 0.001)
    stoppedRotVel = myStoppedRotVel;
  if (stoppedLatVel < 0.001)
    stoppedLatVel = myStoppedLatVel;

  if (fabs(myVel) <= stoppedVel && fabs(myRotVel) <= stoppedRotVel && 
      (!hasLatVel() || fabs(myLatVel) < stoppedLatVel))
    return true;
  else
    return false;
}

// Sets the vels required to be stopped
MVREXPORT void MvrRobot::setStoppedVels(double stoppedVel, double stoppedRotVel, double stoppedLatVel)
{
	myStoppedVel    = stoppedVel;
	myStoppedRotVel = stoppedRotVel;
	myStoppedLatVel = stoppedLatVel;  
}

/*
 * This sets the absolute maximum velocity the robot will go... the
 * maximum velocity can also be set by the actions and by
 * setTransVelMax, but it will not be allowed to go higher than this
 * value.  You should not set this very often, if you want to
 * manipulate this value you should use the actions or setTransVelMax.

 * @param maxVel the maximum velocity to be set, it must be a non-zero
 * number and is in mm/sec

 * @return true if the value is good, false othrewise
 */
MVREXPORT bool MvrRobot::setAbsoluteMaxTransVel(double maxVel)
{
  if (maxVel < 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransVel: given a value <= 0 (%g) and will not set it", maxVel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() &&
      maxVel > myOrigRobotConfig->getTransVelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransVel: given a value (%g) over TransVelTop (%g) and will cap it", maxVel, myOrigRobotConfig->getTransVelTop());
    maxVel = myOrigRobotConfig->getTransVelTop();
  }

  if (fabs(maxVel - myAbsoluteMaxTransVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxTransVel: Setting to %g",maxVel);

  myAbsoluteMaxTransVel = maxVel;
  if (getTransVelMax() > myAbsoluteMaxTransVel)
    setTransVelMax(myAbsoluteMaxTransVel);
  return true;  
}

/*
 * This sets the absolute maximum velocity the robot will go... the
 * maximum velocity can also be set by the actions and by
 * setTransVelMax, but it will not be allowed to go higher than this
 * value.  You should not set this very often, if you want to
 * manipulate this value you should use the actions or setTransVelMax.

 * @param maxVel the maximum velocity to be set, it must be a non-zero
 * number and is in -mm/sec (use negative values)

 * @return true if the value is good, false othrewise
 */
MVREXPORT bool MvrRobot::setAbsoluteMaxTransNegVel(double maxNegVel)
{
  if (maxNegVel >= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransNegVel: given a value >= 0 (%g) and will not set it", maxNegVel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && maxNegVel < -myOrigRobotConfig->getTransVelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransNegVel: given a value (%g) below TransVelTop (-%g) and will cap it", maxNegVel, myOrigRobotConfig->getTransVelTop());
    maxNegVel = -myOrigRobotConfig->getTransVelTop();
  }

  if (fabs(maxNegVel - myAbsoluteMaxTransNegVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxTransNegVel: Setting to %g",maxNegVel);

  myAbsoluteMaxTransNegVel = maxNegVel;
  if (getTransNegVelMax() < myAbsoluteMaxTransNegVel)
    setTransNegVelMax(myAbsoluteMaxTransNegVel);
  return true;
}

/*
 * This sets the absolute maximum translational acceleration the robot
 * will do... the acceleration can also be set by the actions and by
 * setTransAccel, but it will not be allowed to go higher than this
 * value.  You should not set this very often, if you want to
 * manipulate this value you should use the actions or setTransAccel.

 * @param maxAccel the maximum acceleration to be set, it must be a non-zero
 * number 

 * @return true if the value is good, false othrewise
 */
MVREXPORT bool MvrRobot::setAbsoluteMaxTransAccel(double maxAccel)
{
  if (maxAccel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransAccel: given a value <= 0 (%g) and will not set it", maxAccel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxAccel > myOrigRobotConfig->getTransAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransAccel: given a value (%g) over TransAccelTop (%g) and will cap it", maxAccel, myOrigRobotConfig->getTransAccelTop());
    maxAccel = myOrigRobotConfig->getTransAccelTop();
  }

  if (fabs(maxAccel - myAbsoluteMaxTransAccel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxTransAccel: Setting to %g",maxAccel);

  myAbsoluteMaxTransAccel = maxAccel;
  if (getTransAccel() > myAbsoluteMaxTransAccel)
    setTransAccel(myAbsoluteMaxTransAccel);
  return true;
}

/**
  This sets the absolute maximum translational deceleration the robot
  will do... the deceleration can also be set by the actions and by
  setTransDecel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setTransDecel.

  @param maxDecel the maximum deceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxTransDecel(double maxDecel)
{
  if (maxDecel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransDecel: given a value <= 0 (%g) and will not set it", maxDecel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxDecel > myOrigRobotConfig->getTransAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransDecel: given a value (%g) over TransAccelTop (%g) and will cap it", maxDecel, myOrigRobotConfig->getTransAccelTop());
    maxDecel = myOrigRobotConfig->getTransAccelTop();
  }

  if (fabs(maxDecel - myAbsoluteMaxTransDecel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxTransDecel: Setting to %g",maxDecel);

  myAbsoluteMaxTransDecel = maxDecel;
  if (getTransDecel() > myAbsoluteMaxTransDecel)
    setTransDecel(myAbsoluteMaxTransDecel);
  return true;
}

/**
  This sets the absolute maximum velocity the robot will go... the
  maximum velocity can also be set by the actions and by
  setRotVelMax, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setRotVelMax.

  @param maxVel the maximum velocity to be set, it must be a non-zero number
  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxRotVel(double maxVel)
{
  if (maxVel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotVel: given a value <= 0 (%g) and will not use it", maxVel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxVel > myOrigRobotConfig->getRotVelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotVel: given a value (%g) over RotVelTop (%g) and will cap it", maxVel, myOrigRobotConfig->getRotVelTop());
    maxVel = myOrigRobotConfig->getRotVelTop();
  }

  if (fabs(maxVel - myAbsoluteMaxRotVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxRotVel: Setting to %g", maxVel);

  myAbsoluteMaxRotVel = maxVel;
  if (getRotVelMax() > myAbsoluteMaxRotVel)
    setRotVelMax(myAbsoluteMaxRotVel);
  return true;
}

/**
  This sets the absolute maximum rotational acceleration the robot
  will do... the acceleration can also be set by the actions and by
  setRotAccel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setRotAccel.

  @param maxAccel the maximum acceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxRotAccel(double maxAccel)
{
  if (maxAccel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotAccel: given a value <= 0 (%g) and will not use it", maxAccel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxAccel > myOrigRobotConfig->getRotAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotAccel: given a value (%g) over RotAccelTop (%g) and will cap it", maxAccel, myOrigRobotConfig->getRotAccelTop());
    maxAccel = myOrigRobotConfig->getRotAccelTop();
  }

  if (fabs(maxAccel - myAbsoluteMaxRotAccel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxRotAccel: Setting to %g",maxAccel);

  myAbsoluteMaxRotAccel = maxAccel;
  if (getRotAccel() > myAbsoluteMaxRotAccel)
    setRotAccel(myAbsoluteMaxRotAccel);
  return true;
}

/**
  This sets the absolute maximum rotational deceleration the robot
  will do... the deceleration can also be set by the actions and by
  setRotDecel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setRotDecel.

  @param maxDecel the maximum deceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxRotDecel(double maxDecel)
{
  if (maxDecel <= 0)
  {    
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotDecel: given a value <= 0 (%g) and will not use it", maxDecel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxDecel > myOrigRobotConfig->getRotAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxRotDecel: given a value (%g) over RotAccelTop (%g) and will cap it", maxDecel, myOrigRobotConfig->getRotAccelTop());
    maxDecel = myOrigRobotConfig->getRotAccelTop();
  }

  if (fabs(maxDecel - myAbsoluteMaxRotDecel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxRotDecel: Setting to %g",maxDecel);

  myAbsoluteMaxRotDecel = maxDecel;
  if (getRotDecel() > myAbsoluteMaxRotDecel)
    setRotDecel(myAbsoluteMaxRotDecel);
  return true;
}

/**
  This sets the absolute maximum lateral velocity the robot will
  go... the maximum velocity can also be set by the actions and by
  setLatVelMax, but it will not be allowed to go higher than this

  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setLatVelMax.

  @param maxLatVel the maximum velocity to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxLatVel(double maxLatVel)
{
  if (maxLatVel <= 0)
  {    
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatVel: given a value <= 0 (%g) and will not use it", maxLatVel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxLatVel > myOrigRobotConfig->getLatVelTop())
  {
    MvrLog::log(MvrLog::Normal,
                "MvrRobot::setAbsoluteMaxLatVel: given a value (%g) over LatVelTop (%g) and will cap it", 
	              maxLatVel, myOrigRobotConfig->getLatVelTop());
    maxLatVel = myOrigRobotConfig->getLatVelTop();
  }

  if (fabs(maxLatVel - myAbsoluteMaxLatVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxLatVel: Setting to %g", maxLatVel);

  myAbsoluteMaxLatVel = maxLatVel;
  if (getLatVelMax() > myAbsoluteMaxLatVel)
    setLatVelMax(myAbsoluteMaxLatVel);
  return true;
}

/**
  This sets the absolute maximum lateral acceleration the robot
  will do... the acceleration can also be set by the actions and by
  setLatAccel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setLatAccel.

  @param maxAccel the maximum acceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxLatAccel(double maxAccel)
{
  if (maxAccel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatAccel: given a value <= 0 (%g) and will not use it", maxAccel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxAccel > myOrigRobotConfig->getLatAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatAccel: given a value (%g) over LatAccelTop (%g) and will cap it", 
                maxAccel, myOrigRobotConfig->getLatAccelTop());
    maxAccel = myOrigRobotConfig->getLatAccelTop();
  }

  if (fabs(maxAccel - myAbsoluteMaxLatAccel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxLatAccel: Setting to %g", maxAccel);

  myAbsoluteMaxLatAccel = maxAccel;
  if (getLatAccel() > myAbsoluteMaxLatAccel)
    setLatAccel(myAbsoluteMaxLatAccel);
  return true;
}

/**
  This sets the absolute maximum lateral deceleration the robot
  will do... the deceleration can also be set by the actions and by
  setLatDecel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setLatDecel.

  @param maxDecel the maximum deceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxLatDecel(double maxDecel)
{
  if (maxDecel <= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatDecel: given a value <= 0 (%g) and will not use it", maxDecel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxDecel > myOrigRobotConfig->getLatAccelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatDecel: given a value (%g) over LatAccelTop (%g) and will cap it", 
                maxDecel, myOrigRobotConfig->getLatAccelTop());
    maxDecel = myOrigRobotConfig->getLatAccelTop();
  }

  if (fabs(maxDecel - myAbsoluteMaxLatDecel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxLatDecel: Setting to %g",maxDecel);

  myAbsoluteMaxLatDecel = maxDecel;
  if (getLatDecel() > myAbsoluteMaxLatDecel)
    setLatDecel(myAbsoluteMaxLatDecel);
  return true;
}

/*
 * This gets the raw IO Analog value, which is a number between 0 an 1024(10^2)
 * @see requestIOPackets()
 */
MVREXPORT int MvrRobot::getIOAnalog(int num) const
{
  if (num <= getIOAnalog())
    return myIOAnalog[num];
  else
    return 0;
}

/*
 * This gets the IO Analog value converted to a voltage between 0 and 5 votes
 * @see requestIOPackets()
 */
MVREXPORT double MvrRobot::getIOAnalogVoltage(int num) const
{
  if (num <= getIOAnalogSize())
  {
    return (myIOAnalog[num] & 0xfff) * 0.0048828;
  }
  else
    return 0;
}

MVREXPORT unsigned char MvrRobot::getIODigIn(int num) const
{
  if (num <= getIODigInSize)
    return myIODigIn[num];
  else
    return (unsigned char) 0;
}

MVREXPORT unsigned char MvrRobot::getIODigOut(int num) const
{
  if (num <= getIODigOutSize)
    return myIODigOut[num];
  else
    return (unsigned char) 0;
}

// @return the MvrRobotParams instance the robot is using for its parameters
MVREXPORT MvrRobotParams *MvrRobot::getRobotParams(void) const
{
  return myParams;
}
// @return the MvrRobotParams instance the robot is using for its parameters
MVREXPORT MvrRobotParams *MvrRobot::getRobotParamsInternal(void) const
{
  return myParams;
}
// @return the MvrRobotConfigPacketReader taken when this instance got connected to the robot
MVREXPORT const MvrRobotConfigPacketReader *MvrRobot::getOrigRobotConfig(void) const
{
  return myOrigRobotConfig;
}

/*
 *  Adds a packet handler.  A packet handler is an MvrRetFunctor1<bool, MvrRobotPacket*>, 
 *  (e.g.  created as an instance of MvrRetFunctor1C.  The return is a boolean, while the functor
 *  takes an MvrRobotPacket pointer as the argument.  This functor is placed in
 *  a list of functors to call when a packet arrives. This list is processed
 *  in order until one of the handlers returns true. Your packet handler
 *  function may be invoked for any packet, so it should test the packet type
 *  ID (see MvrRobotPacket::getID()). If you handler gets data from the packet
 *  (it "handles" it) it should return true, to prevent MvrRobot from invoking
 *  other handlers with the packet (with data removed). If you hander
 *  cannot interpret the packet, it should leave it unmodified and return
 *  false to allow other handlers a chance to receive it.
 *  @param functor the functor to call when the packet comes in
 *  @param position whether to place the functor first or last
 */
MVREXPORT void MvrRobot::addPacketHandler(MvrRetFunctor1<bool, MvrRobotPacket *> *functor,
                                          MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myPacketHandlerList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myPacketHandlerList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addPacketHandler: Invalid position");
}

/*
 *@param functor the functor to remove from the list of packet handlers
 *@see addPacketHandler
 */
MVREXPORT void MvrRobot::remPacketHandler(MvrRetFunctor1<bool, MvrRobotPacket *> *functor)
{
  myPacketHandlerList.remove(functor);
}

/*
 * Adds a connect callback, which is an MvrFunctor, (created as an MvrFunctorC).
 * The entire list of connect callbacks is called when a connection is made
 * with the robot.  If you have some sort of module that adds a callback, 
 * that module must remove the callback when the module is removed.
 * @param functor A functor (created from MvrFunctorC) which refers to the 
 * function to call.
 * @param position whether to place the functor first or last
 * @see remConnectCB
 */
MVREXPORT void MvrRobot::addConnectCB(MvrFunctor *functor, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myConnectCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myConnectCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addConnectCallback: Invalid position");
}

/*
 *@param functor the functor to remove from the list of connect callbacks
 *@see addConnectCB
 */
MVREXPORT void MvrRobot::remConnectCB(MvrFunctor *functor)
{
  myConnectCBList.remove(functor);
}

/* Adds a failed connect callback,which is an MvrFunctor, created as an 
 * MvrFunctorC.  This whole list of failed connect callbacks is called when
 * an attempt is made to connect to the robot, but fails.   The usual reason 
 * for this failure is either that there is no robot/sim where the connection
 * was tried to be made, the robot wasn't given a connection, or the radio
 * modems that communicate with the robot aren't on.  If you have some sort
 * of module that adds a callback, that module must remove the callback
 * when the module removed.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param position whether to place the functor first or last
 * @see remFailedConnectCB
 */
MVREXPORT void MvrRobot::addFailedConnectCB(MvrFunctor *functor, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myFailedConnectCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myFailedConnectCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addFailedConnectCallback: Invalid position");
}

/* 
 * @param functor the functor to remove from the list of connect callbacks
 * @see addFailedConnectCB
 */
MVREXPORT void MvrRobot::remFailedConnectCB(MvrFunctor *functor)
{
  myFailedConnectCBList.remove(functor);
}

/* Adds a disconnect normally callback,which is an MvrFunctor, created as an 
 * MvrFunctorC.  This whole list of disconnect normally callbacks is called 
 * when something calls disconnect if the instance isConnected.  If there is 
 * no connection and disconnect is called nothing is done.  If you have some
 * sort of module that adds a callback, that module must remove the callback
 * when the module is removed.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param position whether to place the functor first or last
 * @see remFailedConnectCB
 */
MVREXPORT void MvrRobot::addDisconnectNormallyCB(MvrFunctor *functor, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myDisconnectNormallyCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myDisconnectNormallyCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addDisconnectNormallyCallback: Invalid position");
}

/* 
 * @param functor the functor to remove from the list of connect callbacks
 * @see addDisconnectNormallyCB
 */
MVREXPORT void MvrRobot::remDisconnectNormallyCB(MvrFunctor *functor)
{
  myDisconnectNormallyCBList.remove(functor);
}

/* Adds a disconnect on error callback, which is an MvrFunctor, created as an 
 * MvrFunctorC.  This whole list of disconnect on error callbacks is called 
 * when MVRIA loses connection to a robot because of an error.  This can occur
 * if the physical connection (ie serial cable) between the robot and the 
 * computer is severed/disconnected, if one of a pair of radio modems that 
 * connect the robot and computer are disconnected, if someone presses the
 * reset button on the robot, or if the simulator is closed while MVRIA
 * is connected to it.  Note that if the link between the two is lost the 
 * MVRIA assumes it is temporary until it reaches a timeout value set with
 * setConnectionTimeoutTime.  If you have some sort of module that adds a 
 * callback, that module must remove the callback when the module removed.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param position whether to place the functor first or last
 * @see remDisconnectOnErrorCB
 */
MVREXPORT void MvrRobot::addDisconnectOnErrorCB(MvrFunctor *functor, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myDisconnectOnErrorCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myDisconnectOnErrorCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addDisconnectOnErrorCB: Invalid position");
}

/* 
 * @param functor the functor to remove from the list of connect callbacks
 * @see addDisconnectNormallyCB
 */
MVREXPORT void MvrRobot::remDisconnectOnErrorCB(MvrFunctor *functor)
{
  myDisconnectOnErrorCBList.remove(functor);
}

/*
 * Adds a callback that is called when the run loop exits. The functor is
 * which is an MvrFunctor, created as an MvrFunctorC. The whole list of
 * functors is called when the run loop exits. This is most usefull for
 * threaded programs that run the robot using MvrRobot::runAsync. This will
 * allow user threads to know when the robot loop has exited.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param position whether to place the functor first or last
 * @see remRunExitCB
 */ 
MVREXPORT void MvrRobot::addRunExitCB(MvrFunctor *functor, MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myRunExitCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myRunExitCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addRunExitCB: Invalid position");
}

/* 
 * @param functor the functor to remove from the list of connect callbacks
 * @see addRunExitCB
 */
MVREXPORT void MvrRobot::remRunExitCB(MvrFunctor *functor)
{
  myRunExitCBList.remove(functor);
}

/*
 * Adds a stablizing callback, which is an MvrFunctor, created as an
 * MvrFunctorC.  The entire list of connect callbacks is called just
 * before the connection is called done to the robot.  This time can
 * be used to calibtrate readings (on things like gyros).
 * @param functor The functor to call (e.g. MvrFunctorC)
 * @param position whether to place the functor first or last
 * @see remConnectCB
 */
MVREXPORT void MvrRobot::addStabilizingCB(MvrFunctor *functor, 
				       MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myStabilizingCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myStabilizingCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addConnectCallback: Invalid position.");
}

/* 
 * @param functor the functor to remove from the list of stabilizing callbacks
 * @see addConnectCB
 */
MVREXPORT void MvrRobot::remStabilizingCB(MvrFunctor *functor)
{
  myStabilizingCBList.remove(functor);
}

MVREXPORT std::list<MvrFunctor *> * MvrRobot::getRunExitListCopy()
{
  return (new std::list<MvrFunctor *>(myRunExitCBList));
}

/*
 * This will suspend the calling thread until the MvrRobot's run loop has
 * managed to connect with the robot. There is an optional paramater of
 * milliseconds to wait for the MvrRobot to connect. If msecs is set to 0,
 * it will wait until the MvrRobot connects. This function will never
 * return if the robot can not be connected with. If you want to be able
 * to handle that case within the calling thread, you must call
 * waitForConnectOrConnFail().
 * @param msecs milliseconds in which to wait for the MvrRobot to connect
 * @return WAIT_CONNECTED for success
 */
MVREXPORT MvrRobot::WaitState MvrRobot::waitForConnect(unsigned int msecs)
{
  int ret;

  if (isConnected())
    return (WAIT_CONNECTED);
  
  if (msecs == 0)
    ret = myConnectCond.wait();
  else
    ret = myConnectCond.timedWait(msecs);
  
  if (ret == MvrCondition::STATUS_WAIT_INTR)
    return WAIT_INTR;
  else if (ret == MvrCondition::STATUS_WAIT_TIMEOUT)
    return WAIT_CONNECTED;
  else
    return WAIT_FAIL;
}

/*
 * This will suspend the calling thread until the MvrRobot's run loop has
 * managed to connect with the robot or fails to connect with the robot.
 * There is an optional paramater of milliseconds to wait for the MvrRobot
 * to connect. If msecs is set to 0, it will wait until the MvrRobot connects.
 * @param msecs milliseconds in which to wait for the MvrRobot to connect
 * @return WAIT_CONNECTED for success
 * @see waitForConnect
 */
MVREXPORT MvrRobot::WaitState MvrRobot::waitForConnectOrConnFail(unsigned int msecs)
{
  int ret;

  if (isConnected())
    return (WAIT_CONNECTED);
  
  if (msecs == 0)
    ret = myConnectCond.wait();
  else
    ret = myConnectCond.timedWait(msecs);
  
  if (ret == MvrCondition::STATUS_WAIT_INTR)
    return WAIT_INTR;
  else if (ret == MvrCondition::STATUS_WAIT_TIMEOUT)
    return WAIT_CONNECTED;
  else if (ret == 0)
  {
    if (isConnected())
      return WAIT_CONNECTED;
    else
      return WAIT_FAILED_CONN;
  }
    return WAIT_FAIL;
}


/*
 * This will suspend the calling thread until the MvrRobot's run loop has
 * exited. There is an optional paramater of milliseconds to wait for the
 * MvrRobot run loop to exit . If msecs is set to 0, it will wait until
 * the MvrRrobot run loop exits.
 * @param msecs milliseconds in which to wait for the robot to connect
 * @return WAIT_RUN_EXIT for success
 */
MVREXPORT MvrRobot::WaitState MvrRobot::waitForRunExit(unsigned int msecs)
{
  int ret;

  if (!isRunning())
    return(WAIT_RUN_EXIT);

  if (msecs == 0)
    ret=myRunExitCond.wait();
  else
    ret=myRunExitCond.timedWait(msecs);

  if (ret == MvrCondition::STATUS_WAIT_INTR)
    return(WAIT_INTR);
  else if (ret == MvrCondition::STATUS_WAIT_TIMEDOUT)
    return(WAIT_TIMEDOUT);
  else if (ret == 0)
    return(WAIT_RUN_EXIT);
  else
    return(WAIT_FAIL);
}

/*
 * This will wake all the threads waiting for various major state changes
 * in this particular MvrRobot. This includes all threads waiting for the
 * robot to be connected and all threads waiting for the run loop to exit.
 */
MVREXPORT void MvrRobot::wakeAllWaitingThreads()
{
  wakeAllConnWaitingThreads();
  wakeAllRunExitWaitingThreads();
}

/*
 * This will wake all the threads waiting for the robot to be connected.
 */
MVREXPORT void MvrRobot::wakeAllConnWaitingThreads()
{
  myConnectCond.broadcast();
  myConnOrFailCond.broadcast();
}

/*
 * This will wake all the threads waiting for the robot to be connected or
 * waiting for the robot to fail to connect
 */
MVREXPORT void MvrRobot::wakeAllConnOrFailWaitingThreads()
{
  myConnOrFailCond.broadcast();
}

/*
 * This will wake all thre threads waiting for the run loop to exit
 */
MVREXPORT void MvrRobot::wakeAllRunExitWaitingThreads()
{
  myRunExitCond.broadcast();
}

/*
 * This gets the root of the synchronous task tree, so that someone can add
 * their own new types of tasks, or find out more information about each
 * task...
 * @return the root of the sychronous tasks tree
 */
MVREXPORT MvrSyncTask *MvrRobot::getSyncTaskRoot(void)
{
  return mySyncTaskRoot;
}

/*
 * The synchronous tasks get called every robot cycle (every 100 ms by 
 * default).  
 * @warning Not thread safe; if robot thread is running in background (from
 * runAsync()), you must lock the MvrRobot object before calling and unlock after
 * calling this method.
 * @param name the name to give to the task, should be unique
 * @param position the place in the list of user tasks to place this
 * task, this can be any integer, though by convention 0 to 100 is
 * used.  The tasks are called in order of highest number to lowest
 * position number.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param state Optional pointer to external MvrSyncTask state variable; normally not needed
 * and may be NULL or omitted.
 */
MVREXPORT bool MvrRobot::addUserTask(const char *name, int position, MvrFunctor *functor, MvrTaskState::State *state)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return false;
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return false;
  proc->addNewLeaf(name, position, functor, state);
  return true;
}

MVREXPORT void MvrRobot::remUserTask(const char *name)
{
  MvrSyncTask *proc;
  MvrSyncTask *userProc;

  if (mySyncTaskRoot == NULL)
    return;
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return;
  userProc = proc->findNonRecursive(name);
  if (userProc == NULL)
    return;
  
  delete userProc;
}

MVREXPORT void MvrRobot::remUserTask(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  MvrSyncTask *userProc;

  if (mySyncTaskRoot == NULL)
    return;
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return;
  userProc = proc->findNonRecursive(functor);
  if (userProc == NULL)
    return;
  
  delete userProc;  
}

/**
 * The synchronous tasks get called every robot cycle (every 100 ms by 
 * default).  
 * @warning Not thread safe; if robot thread is running in background (from
 *  runAsync()), you must lock the MvrRobot object before calling and unlock after
 *  calling this method.
 * @param name the name to give to the task, should be unique
 * @param position the place in the list of user tasks to place this 
 * task, this can be any integer, though by convention 0 to 100 is used.
 * The tasks are called in order of highest number to lowest number.
 * @param functor functor created from MvrFunctorC which refers to the 
 * function to call.
 * @param state Optional pointer to external MvrSyncTask state variable; normally not needed
 * and may be NULL or omitted.
 */
MVREXPORT bool MvrRobot::addSensorInterpTask(const char *name, int position, MvrFunctor *functor, MvrTaskState::State state)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return false;
  proc = mySyncTaskRoot->findNonRecursive("Sensor Interp");
  if (proc == NULL)
    return false;
  
  proc->addNewLeaf(name, position, functor, state);
  return true;
}

MVREXPORT void MvrRobot::remSensorInterpTask(const char *name)
{
  MvrSyncTask *proc;
  MvrSyncTask *sensorInterpProc;

  if (mySyncTaskRoot == NULL)
    return;
  proc = mySyncTaskRoot->findNonRecursive("Sensor Interp");
  if (proc == NULL)
    return;
  sensorInterpProc = proc->findNonRecursive(name); 
  if (sensorInterpProc == NULL)
    return;
  delete sensorInterpProc;
}

MVREXPORT void MvrRobot::remSensorInterpTask(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  MvrSyncTask *sensorInterpProc;

  if (mySyncTaskRoot == NULL)
    return;
  proc = mySyncTaskRoot->findNonRecursive("Sensor Interp");
  if (proc == NULL)
    return;
  sensorInterpProc = proc->findNonRecursive(functor);
  if (sensorInterpProc == NULL)
    return;
  
  delete sensorInterpProc;   
}

MVREXPORT void MvrRobot::logUserTasks(void) const
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return;
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return;
  
  proc->log();
}

MVREXPORT void MvrRobot::logAllTasks(void) const
{
  if (mySyncTaskRoot != NULL)
    mySyncTaskRoot->log();
}

/*
 * Finds a user task by its name, searching the entire space of tasks
 * @return NULL if no user task of that name found, otherwise a pointer to
 * the MvrSyncTask for the first task found with that name
 */
MVREXPORT MvrSyncTask *MvrRobot::findUserTask(const char *name)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return NULL;
  
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return NULL:
  return proc->find(name);
}

/*
 * Finds a user task by its functor, searching the entire space of tasks
 * @return NULL if no user task of that name found, otherwise a pointer to
 * the MvrSyncTask for the first task found with that name
 */
MVREXPORT MvrSyncTask *MvrRobot::findUserTask(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return NULL;
  
  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return NULL:
  return proc->find(functor);
}

/*
 * Finds a task by its name, searching the entire space of tasks
 * @return NULL if no user task of that name found, otherwise a pointer to
 * the MvrSyncTask for the first task found with that name
 */
MVREXPORT MvrSyncTask *MvrRobot::findTask(const char *name)
{
  if (mySyncTaskRoot == NULL)
    return mySyncTaskRoot->find(name);
  else
    return NULL;
} 

/*
 * Finds a task by its functor, searching the entire space of tasks
 * @return NULL if no user task of that name found, otherwise a pointer to
 * the MvrSyncTask for the first task found with that name
 */
MVREXPORT MvrSyncTask *MvrRobot::findTask(MvrFunctor *functor)
{
  if (mySyncTaskRoot == NULL)
    return mySyncTaskRoot->find(functor);
  else
    return NULL;
} 

/* 
 * Adds an action to the list of actions with the given priority. In
 * the case of two (or more) actions with the same priority, the
 * default resolver (MvrPriorityResolver) averages the the multiple
 * readings. The priority can be any integer, but as a convention 0
 * to 100 is used, with 100 being the highest priority. The default
 * resolver (MvrPriorityResolver) resolves the actions in order of descending
 * priority. For example, an action with priority 100 is evaluated before 
 * one with priority 99, followed by 50, etc.  This means that an action with 
 * a higher priority may be able to supercede a lower-priority action's
 * desired value for a certain output to a lesser or greater degree, depending 
 * on how high a "strength" value it sets.  See the overview of ARIA in this 
 * reference manual for more discussion on Actions.

 * @param action the action to add 
 * @param priority what importance to give the action; how to order the actions.  High priority actions are evaluated by the action resolvel before lower priority actions.
 * @return true if the action was successfully added, false on error (e.g. the action was NULL)
*/
MVREXPORT bool MvrRobot::addAction(MvrAction *action, int priority)
{
  if (action == NULL)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot::addAction: an attempt was made to add a NULL action pointer");
    return false;
  }
  action->setRobot(this);
  myActions.insert(std::pair<int, MvrAction *>(priority, action));
  return true;
}

/*
 * Finds the action with the given name and removes it from the actions...
 * if more than one acion has that name it find the one with the lowest priority
 * @param actionName the name of the action we want to find
 * @return whether remAction found anything with that action to remove or not
 */
MVREXPORT bool MvrRobot::remAction(const char *actionName)
{
  MvrResolver::ActionMap::iterator it;
  MvrAction *act;

  for (it = myActions.begin(); it != myActions.end(); it++)
  {
    act = (*it).second;
    if (strcmp(actionName, act->getName()) == 0)
      break;
  }

  if (it != myActions.end())
  {
    myActions.erase(it);
    return true;
  }
  return false;
}

/*
 * Finds the action with the given pointer and removes it from the actions... 
 * if more than one action has that pointer it find the one with the lowest
 * priority
 * @param action the action we want to remove
 * @return whether remAction found anything with that action to remove or not
 */
MVREXPORT bool MvrRobot::remAction(MvrAction *action)
{
  MvrResolver::ActionMap::iterator it;
  MvrAction *act;

  for (it = myActions.begin(); it != myActions.end(); it++)
  {
    act = (*it).second;
    if (act == action)
      break;
  }

  if (it != myActions.end())
  {
    myActions.erase(it);
    return true;
  }
  return false;
}

/*
 * Finds the action with the given name... if more than one action
 * has that name it find the one with the highest priority
 * @param actionName the name of the action we want to find
 * @return the action, if found.  If not found, NULL
 */
MVREXPORT MvrAction *MvrRobot::findAction(const char *actionName)
{
  MvrResolver::ActionMap::reverse_iterator it;
  MvrAction *it;

  for (it = myActions.rbegin(); it != myActions.rend(); ++it)
  {
    act = (*it).second;
    if (strcmp(actionName, act->getName()) == 0)
      return act;
  }
  return NULL;
}
/*
 * @internal
 * This returns the actionMap the robot has... do not mess with this
 * list except by using MvrRobot::addAction() and MvrRobot::remAction()...
 * This is jsut for the things like MvrActionGroup that want to
 * deactivate or activate all the actions (well, only deactivating
 * everything makes sense).
 * @return the actions the robot is using
 */
MVREXPORT MvrResolver::ActionMap *MvrRobot::getActionMap(void)
{
  return &myActions;
}

MVREXPORT void MvrRobot::deactivateActions(void)
{
  MvrResolver::ActionMap *am;
  MvrResolver::ActionMap::iterator amit;

  am = getActionMap();
  if (am == NULL)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot::deactivateActions: NULL action Map... failed");
    return;
  } 
  for (amit = am->begin(); amit != am->end(); amit++)
    (*amit).second->deactivate();
}

MVREXPORT void MvrRobot::logActions(bool logDeactivated) const
{
  MvrResolver::ActionMap::const_reverse_iterator it;
  int lastPriority;
  bool first = true;
  const MvrAction *action;

  if (logDeactivated)
    MvrLog::log(MvrLog::Terse, "The action list (%d total);" myActions.size());
  else
    MvrLog::log(MvrLog::Terse, "Teh active action list:");
  for (it = myActions.rbegin(); it != myActions.rend(); ++it)
  {
    action = (*it).second;
    if ((logDeactivated || action->isActive()) && (first || lastPriority != (*it).first))
    {
      MvrLog::log(MvrLog::Terse, "Priority %d:", (*it).first);
      first = false;
      lastPriority = (*it).first;
    }
    if (logDeactivated || action->isActive())
      action->log(false);
  }
}

MVREXPORT MvrResolver *MvrRobot::getResolver(void)
{
  return myResolver;
}

MVREXPORT void MvrRobot::setResolver(MvrResolver *resolver)
{
  if (myOwnTheResolver)
  {
    delete myOwnTheResolver;
    myResolver = NULL:
  }
  myResolver = resolver;
}

/*
 * @internal
 *
 * If state reflecting (really direct motion command reflecting) was
 * enabled in the constructor (MvrRobot::MvrRobot) then this will see if
 * there are any direct motion commands to send, and if not then send
 * the command given by the actions.  If state reflection is disabled
 * this will send a pulse to the robot every state reflection refresh
 * time (setStateReflectionRefreshTime), if you don't wish this to happen
 * simply set this to a very large value.
 */
MVREXPORT void MvrRobot::stateReflector(void)
{
  short transVal;
  short transVal2;
  short maxVel;
  short maxNegVel;
  double maxTransVel;
  double maxNegTransVel;
  double transAccel;
  double transDecel;

  double maxRotVel = -1;
  double maxRotVelPos = -1;
  double maxRotVelNeg = -1;
  short rotVal = 0;
  double rotAccel;
  double rotDecel;
  bool rotStopped = false;
  bool rotHeading = false;
  double encTh;
  double rawTh;

  short latVal;
  short maxLatVel;
  double maxLeftLatVel;
  double maxRightLatVel;
  double latAccel;
  double latDecel;

  if (!myIsConnected)
    return;

  myTryingToMove = false;

  // if this is true actions can't go
  if ((myTransType != TRANS_NONE && myDirectPrecedenceTime == 0) ||
      (myTransType != TRANS_NONE && myDirectPrecedenceTime != 0 && 
       myTransSetTime.mSecSince() < myDirectPrecedenceTime))
  {
    myActionTransSet = false;
    transVal = MvrMath::roundShort(myTransVal);
    transVal2 = 0;

    if (hasSettableVelMaxes() && MvrMath::fabs(myLastSentTransVelMax - myTransVelMax) >= 1)
    {
      comInt(MvrCommands::SETV, MvrMath::roundShort(myTransVelMax));
      myLastSentTransVelMax = myTransVelMax;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "Non-action trans max vel of %d", MvrMath::roundShort(myTransVelMax));
    }

    if (hasSettableAccsDecs() && MvrMath::fabs(myTransAccel) > 1 && 
	      MvrMath::fabs(myLastSentTransAccel - myTransAccel) >= 1)
    {
      comInt(MvrCommands::SETA, MvrMath::roundShort(myTransAccel));
      myLastSentTransAccel = myTransAccel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "Non-action trans accel of %d", MvrMath::roundShort(myTransAccel));
    }

    if (hasSettableAccsDecs() && MvrMath::fabs(myTransDecel) > 1 &&
      	MvrMath::fabs(myLastSentTransDecel - myTransDecel) >= 1)
    {
      comInt(MvrCommands::SETA, -MvrMath::roundShort(myTransDecel));
      myLastSentTransDecel = myTransDecel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "Non-action trans decel of %d",  -MvrMath::roundShort(myTransDecel));
    }

    if (myTransType == TRANS_VEL)
    {
      maxVel = MvrMath::roundShort(myTransVelMax);
      maxNegVel = MvrMath::roundShort(myTransNegVelMax);
      if (transVal > maxVel)
	      transVal = maxVel;
      if (transVal < maxNegVel)
        transVal = maxNegVel;
      if (myLastTransVal != transVal || myLastTransType != myTransType ||
          (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime))
      {
        comInt(MvrCommands::VEL, MvrMath::roundShort(transVal));
        myLastTransSent.setToNow();
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Non-action trans vel of %d", MvrMath::roundShort(transVal));
        //printf("Sent command vel!\n");
      }
      if (fabs((double)transVal) > (double).5)
        myTryingToMove = true;
    }
    else if (myTransType == TRANS_VEL2)
    {
      if (MvrMath::roundShort(myTransVal/myParams->getVel2Divisor()) > 128)
        transVal = 128;
      else if (MvrMath::roundShort(myTransVal/myParams->getVel2Divisor()) < -128)
        transVal = -128;
      else 
        transVal = MvrMath::roundShort(myTransVal/myParams->getVel2Divisor());
      if (MvrMath::roundShort(myTransVal2/myParams->getVel2Divisor()) > 128)
        transVal2 = 128;
      else if (MvrMath::roundShort(myTransVal2/myParams->getVel2Divisor()) < -128)
        transVal2 = -128;
      else 
        transVal2 = MvrMath::roundShort(myTransVal2/myParams->getVel2Divisor());
      if (myLastTransVal != transVal || myLastTransVal2 != transVal2 || 
          myLastTransType != myTransType || (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime))
      {
        com2Bytes(MvrCommands::VEL2, transVal, transVal2);
        myLastTransSent.setToNow();
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Non-action vel2 of %d %d", transVal, transVal2);
        //printf("Sent command vel2!\n");
      }
      if (fabs((double)transVal) > (double).5 || fabs((double)transVal2) > (double).5)
        myTryingToMove = true;
    }
    else if (myTransType == TRANS_DIST_NEW || myTransType == TRANS_DIST)
    {
      // if the robot doesn't have its own distance command
      if (!myParams->hasMoveCommand())
      {
        double distGone;
        double distToGo;
        double vel;
      
        myTransType = TRANS_DIST;
        distGone = myTransDistStart.findDistanceTo(getPose());
        distToGo = fabs(fabs(myTransVal) - distGone);
        if (distGone > fabs(myTransVal) || (distToGo < 10 && fabs(getVel()) < 30))
        {
          comInt(MvrCommands::VEL, 0);
          myTransType = TRANS_VEL;
          myTransVal = 0;
        }
      else
        myTryingToMove = true;
      vel = sqrt(distToGo * 200 * 2);
      if (vel > getTransVelMax())
        vel = getTransVelMax();
      if (myTransVal < 0)
        vel *= -1;
      comInt(MvrCommands::VEL, MvrMath::roundShort(vel));
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "Non-action move-helper of %d", MvrMath::roundShort(vel));
      }
      else if (myParams->hasMoveCommand() && myTransType == TRANS_DIST_NEW) 
      {
        comInt(MvrCommands::MOVE, transVal);
        myLastTransSent.setToNow();
        myTransType = TRANS_DIST;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Non-action move of %d", transVal);
        myTryingToMove = true;
      }
      else if (myTransType == TRANS_DIST && (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime))
      {
        com(0);
        myLastPulseSent.setToNow();
        myLastTransSent.setToNow();
        //printf("Sent pulse for dist!\n");
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Non-action pulse for dist");
      }
      //printf("Sent command move!\n");
    }
    else if (myTransType == TRANS_IGNORE)
    {
      //printf("No trans command sent\n");
    }
    else
      MvrLog::log(MvrLog::Terse, "MvrRobot::stateReflector: Invalid translational type %d.",myTransType);
    myLastTransVal  = transVal;
    myLastTransVal2 = transVal2;
    myLastTransType = myTransType;
  }
  else // if actions can go
  {
    if (hasSettableVelMaxes() && MvrMath::fabs(myLastSentTransVelMax - myTransVelMax) >= 1)
    {
      comInt(MvrCommands::SETV,
	     MvrMath::roundShort(myTransVelMax));
      myLastSentTransVelMax = myTransVelMax;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "Action-but-robot trans max vel of %d", MvrMath::roundShort(myTransVelMax));
    }
    
    // first we'll handle all of the accel decel things
    if (myActionDesired.getTransAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      transAccel = MvrMath::roundShort(myActionDesired.getTransAccel());
      if (hasSettableAccsDecs() && MvrMath::fabs(transAccel) > 1 &&
	        MvrMath::fabs(myLastSentTransAccel - transAccel) >= 1)
      {
        comInt(MvrCommands::SETA, MvrMath::roundShort(transAccel));
        myLastSentTransAccel = transAccel;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Action trans accel of %d", MvrMath::roundShort(transAccel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myTransAccel) > 1 && 
	           MvrMath::fabs(myLastSentTransAccel - myTransAccel) >= 1)
    {
      comInt(MvrCommands::SETA, MvrMath::roundShort(myTransAccel));
      myLastSentTransAccel = myTransAccel;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "Action-but-robot trans accel of %d", MvrMath::roundShort(myTransAccel));
    }

    if (myActionDesired.getTransDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      transDecel = MvrMath::roundShort(myActionDesired.getTransDecel());
      if (hasSettableAccsDecs() && MvrMath::fabs(transDecel) > 1 &&
          MvrMath::fabs(myLastSentTransDecel - transDecel) >= 1)
      {
        comInt(MvrCommands::SETA, -MvrMath::roundShort(transDecel));
        myLastSentTransDecel = transDecel;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "Action trans decel of %d", -MvrMath::roundShort(transDecel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myTransDecel) > 1 &&
	           MvrMath::fabs(myLastSentTransDecel - myTransDecel) >= 1)
    {
      comInt(MvrCommands::SETA, -MvrMath::roundShort(myTransDecel));
      myLastSentTransDecel = myTransDecel;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "Action-but-robot trans decel of %d", -MvrMath::roundShort(myTransDecel));
    }

    if (myActionDesired.getMaxVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      maxTransVel = myActionDesired.getMaxVel();
      if (maxTransVel > myTransVelMax)
        maxTransVel = myTransVelMax;
    }
    else
      maxTransVel = myTransVelMax;

    if (myActionDesired.getMaxNegVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      maxNegTransVel = -MvrMath::fabs(myActionDesired.getMaxNegVel());
      if (maxNegTransVel < myTransNegVelMax)
        maxNegTransVel = myTransNegVelMax;
    }
    else
      maxNegTransVel = myTransNegVelMax;
    
    if (myActionDesired.getVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      transVal = MvrMath::roundShort(myActionDesired.getVel());
      myActionTransSet = true;
    }
    else
    {

      //transVal = myLastActionTransVal; 
      transVal = 0;
    }

    if (fabs((double)transVal) > (double).5)
      myTryingToMove = true;

    maxVel = MvrMath::roundShort(maxTransVel);
    maxNegVel = MvrMath::roundShort(maxNegTransVel);
    if (transVal > maxVel)
      transVal = maxVel;
    if (transVal < maxNegVel)
      transVal = maxNegVel;

    if (myActionTransSet && (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime || transVal != myLastActionTransVal))			     
    {
      comInt(MvrCommands::VEL, MvrMath::roundShort(transVal));
      myLastTransSent.setToNow();
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "Action trans vel of %d", MvrMath::roundShort(transVal));      
    }
    myLastActionTransVal = transVal;
  }

  // if this is true actions can't go
  if ((myRotType != ROT_NONE && myDirectPrecedenceTime == 0) ||
      (myRotType != ROT_NONE && myDirectPrecedenceTime != 0 && 
       myRotSetTime.mSecSince() < myDirectPrecedenceTime))
  {
    if (hasSettableVelMaxes() && MvrMath::fabs(myLastSentRotVelMax - myRotVelMax) >= 1)
    {
      //comInt(MvrCommands::SETRVDIR, 0);
      comInt(MvrCommands::SETRV, MvrMath::roundShort(myRotVelMax));

      myLastSentRotVelMax = myRotVelMax;
      myLastSentRotVelPosMax = -1;
      myLastSentRotVelNegMax = -1;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "%25sNon-action rot vel max of %d", "",MvrMath::roundShort(myRotVelMax));      

    }
    if (hasSettableAccsDecs() && MvrMath::fabs(myRotAccel) > 1 &&
      	MvrMath::fabs(myLastSentRotAccel - myRotAccel) >= 1)
    {
      comInt(MvrCommands::SETRA,
	     MvrMath::roundShort(myRotAccel));
      myLastSentRotAccel = myRotAccel;
      if (myLogMovementSent)
      	MvrLog::log(MvrLog::Normal, "%25sNon-action rot accel of %d", "", MvrMath::roundShort(myRotAccel));      
    }
    if (hasSettableAccsDecs() && MvrMath::fabs(myRotDecel) > 1 &&
	      MvrMath::fabs(myLastSentRotDecel - myRotDecel) >= 1)
    {
      comInt(MvrCommands::SETRA, -MvrMath::roundShort(myRotDecel));
      myLastSentRotDecel = myRotDecel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%25sNon-action rot decel of %d", "", -MvrMath::roundShort(myRotDecel));      
    }

    myActionRotSet = false;
    rotVal = MvrMath::roundShort(myRotVal);
    if (myRotType == ROT_HEADING)
    {
      encTh = MvrMath::subAngle(myRotVal, myEncoderTransform.getTh());
      rawTh = MvrMath::addAngle(encTh, MvrMath::subAngle(myRawEncoderPose.getTh(), myEncoderPose.getTh()));
      rotVal = MvrMath::roundShort(rawTh);

      // if we were using a different heading type, a different heading
      // our heading doesn't match what we want it to be, or its been a while
      // since we sent the heading, send it again
      if (myLastRotVal != rotVal || myLastRotType != myRotType ||
          fabs(MvrMath::subAngle(rotVal, getTh())) > 1 || 
          (myLastRotSent.mSecSince() >= myStateReflectionRefreshTime))  
      {
        comInt(MvrCommands::HEAD, rotVal);
        myLastRotSent.setToNow();
        //printf("sent command, heading\n");
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, 
                      "%25sNon-action rot heading of %d (encoder %d, raw %d)",
                      "",
                      MvrMath::roundShort(myRotVal),
                      MvrMath::roundShort(encTh),
                      MvrMath::roundShort(rotVal));
      }
      if (fabs(MvrMath::subAngle(rotVal, getTh())) > 1)
	      myTryingToMove = true;
    }
    else if (myRotType == ROT_VEL)
    {
      if (myLastRotVal != rotVal || myLastRotType != myRotType ||
	        (myLastRotSent.mSecSince() >= myStateReflectionRefreshTime))
      {
        comInt(MvrCommands::RVEL, rotVal);
        myLastRotSent.setToNow();
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sNon-action rot vel of %d", "", rotVal);      
        //printf("sent command, rot vel\n");
        if (fabs((double)rotVal) > (double).5)
          myTryingToMove = true;
      }
    }
    else if (myRotType == ROT_IGNORE)
    {
      //printf("Not sending any command, rot is set to ignore");
    }
    else
      MvrLog::log(MvrLog::Terse, "MvrRobot::stateReflector: Invalid rotation type %d.",myRotType);
    myLastRotVal = rotVal;
    myLastRotType = myRotType;
  }
  else // if the action can fire
  {
    // first we'll handle all of the accel decel things
    // if ONLY rot vel is sent handle it the way we always have
    if (myActionDesired.getMaxRotVelStrength() >=
        MvrActionDesired::MIN_STRENGTH &&
        myActionDesired.getMaxRotVelPosStrength() <
        MvrActionDesired::MIN_STRENGTH &&
        myActionDesired.getMaxRotVelNegStrength() < 
        MvrActionDesired::MIN_STRENGTH)
    {
      maxRotVel = myActionDesired.getMaxRotVel();
      if (maxRotVel > myAbsoluteMaxRotVel)
        maxRotVel = myAbsoluteMaxRotVel;
      maxRotVel = MvrMath::roundShort(maxRotVel);
      if (MvrMath::fabs(myLastSentRotVelMax - maxRotVel) >= 1)
      {
        myLastSentRotVelMax    = maxRotVel;
        myLastSentRotVelPosMax = -1;
        myLastSentRotVelNegMax = -1;
        //comInt(MvrCommands::SETRVDIR, 0);
        comInt(MvrCommands::SETRV, MvrMath::roundShort(maxRotVel));
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot vel max of %d", "", MvrMath::roundShort(maxRotVel));
      }
    }
    // if a max pos or neg rot vel is set then use that
    else if (myActionDesired.getMaxRotVelPosStrength() >= 
             MvrActionDesired::MIN_STRENGTH ||
             myActionDesired.getMaxRotVelNegStrength() >=
             MvrActionDesired::MIN_STRENGTH)
    {
      if (myActionDesired.getMaxRotVelStrength() >= MvrActionDesired::MIN_STRENGTH)
        maxRotVel = myActionDesired.getMaxRotVel();
      else
        maxRotVel = myRotVelMax;
      
      if (maxRotVel > myAbsoluteMaxRotVel)
        maxRotVel = myAbsoluteMaxRotVel;

      if (myActionDesired.getMaxRotVelPosStrength() >= MvrActionDesired::MIN_STRENGTH)
        maxRotVelPos = MvrUtil::findMin(maxRotVel, myActionDesired.getMaxRotVelPos());
      else
        maxRotVelPos = maxRotVel;

      if (maxRotVelPos < .5)
        maxRotVelPos = 1;

      if (myActionDesired.getMaxRotVelNegStrength() >= MvrActionDesired::MIN_STRENGTH)
        maxRotVelNeg = MvrUtil::findMin(maxRotVel, myActionDesired.getMaxRotVelNeg());
      else
        maxRotVelNeg = maxRotVel;

      // 1 here actually means 0 (since there's no -0 and its not
      // worth two commands)
      if (maxRotVelNeg < .5)
	      maxRotVelNeg = 1;

      if (MvrMath::fabs(myLastSentRotVelPosMax - maxRotVelPos) >= 1 ||
	        MvrMath::fabs(myLastSentRotVelNegMax - maxRotVelNeg) >= 1)
      {
        myLastSentRotVelMax = -1;
        myLastSentRotVelPosMax = maxRotVelPos;
        myLastSentRotVelNegMax = maxRotVelNeg;
	
        // this command doesn't exist just yet...
        comInt(MvrCommands::SETRVDIR, MvrMath::roundShort(maxRotVelPos));
        comInt(MvrCommands::SETRVDIR, MvrMath::roundShort(-maxRotVelNeg));
        if (myLogMovementSent)
        {
          MvrLog::log(MvrLog::Normal, "%25sAction rot vel pos max of %d", "", MvrMath::roundShort(maxRotVelPos));
          MvrLog::log(MvrLog::Normal, "%25sAction rot vel neg max of %d", "", MvrMath::roundShort(-maxRotVelNeg));
        }
      }
    }
    else if (hasSettableVelMaxes() && MvrMath::fabs(myLastSentRotVelMax - myRotVelMax) >= 1)
    {
      //comInt(MvrCommands::SETRVDIR, 0);
      comInt(MvrCommands::SETRV, MvrMath::roundShort(myRotVelMax));
      myLastSentRotVelMax = myRotVelMax;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%25sAction-but-robot rot vel max of %d", "", MvrMath::roundShort(myRotVelMax));      
    }

    if (myActionDesired.getRotAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      rotAccel = MvrMath::roundShort(myActionDesired.getRotAccel());
      if (MvrMath::fabs(myLastSentRotAccel - rotAccel) >= 1)
      {
        comInt(MvrCommands::SETRA, MvrMath::roundShort(rotAccel));
        myLastSentRotAccel = rotAccel;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot accel of %d", "", MvrMath::roundShort(rotAccel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myRotAccel) > 1 &&
             MvrMath::fabs(myLastSentRotAccel - myRotAccel) >= 1)
    {
      comInt(MvrCommands::SETRA, MvrMath::roundShort(myRotAccel));
      myLastSentRotAccel = myRotAccel;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "%25sAction-but-robot rot accel of %d", "", MvrMath::roundShort(myRotAccel));      
    }

    if (myActionDesired.getRotDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      rotDecel = MvrMath::roundShort(myActionDesired.getRotDecel());
      if (MvrMath::fabs(myLastSentRotDecel - rotDecel) >= 1)
      {
        comInt(MvrCommands::SETRA, -MvrMath::roundShort(rotDecel));
        myLastSentRotDecel = rotDecel;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot decel of %d", "", -MvrMath::roundShort(rotDecel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myRotDecel) > 1 &&
	           MvrMath::fabs(myLastSentRotDecel - myRotDecel) >= 1)
    {
      comInt(MvrCommands::SETRA, -MvrMath::roundShort(myRotDecel));
      myLastSentRotDecel = myRotDecel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%25sAction-but-robot rot decel of %d", "", -MvrMath::roundShort(myRotDecel));      
    }

    if (myActionDesired.getDeltaHeadingStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      if (MvrMath::roundShort(myActionDesired.getDeltaHeading()) == 0)
      {
        rotStopped = true;
        rotVal = 0;
        rotHeading = false;
      }
      else
      {
        encTh = MvrMath::subAngle(MvrMath::addAngle(myActionDesired.getDeltaHeading(), getTh()), myEncoderTransform.getTh());
        rawTh = MvrMath::addAngle(encTh, MvrMath::subAngle(myRawEncoderPose.getTh(), myEncoderPose.getTh()));
        rotVal = MvrMath::roundShort(rawTh);
        rotStopped = false;
        rotHeading = true;
        myTryingToMove = true;
      }
      myActionRotSet = true;
    } 
    else if (myActionDesired.getRotVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      if (MvrMath::roundShort(myActionDesired.getRotVel()) == 0)
      {
        rotStopped = true;
        rotVal = 0;
        rotHeading = false;
      }
      else
      {
        double rotVelocity = MvrMath::roundShort(myActionDesired.getRotVel());
        if (maxRotVelPos > -.5 && rotVelocity > 0)
        {
          if (maxRotVelPos < 1.1)
          {
            rotVelocity = 0;
            rotStopped = false;
            rotVal = 0;
            rotHeading = false;
          }
          else
          {
            rotVelocity = MvrUtil::findMin(rotVelocity, maxRotVelPos);
            rotStopped = false;
            rotVal = MvrMath::roundShort(rotVelocity);
            rotHeading = false;
	        }
      	}
        else if (maxRotVelNeg > -.5 && rotVelocity < 0)
        {
          if (maxRotVelNeg < 1.1)
          {
            rotVelocity = 0;
            rotStopped = false;
            rotVal = 0;
            rotHeading = false;
          }
          else
          {
            rotVelocity = MvrUtil::findMax(rotVelocity, -maxRotVelNeg);
            rotStopped = false;
            rotVal = MvrMath::roundShort(rotVelocity);
            rotHeading = false;
          }
        }
        else
        {
          rotStopped = false;
          rotVal = MvrMath::roundShort(myActionDesired.getRotVel());
          rotHeading = false;
        }
          myTryingToMove = true;
      }
    myActionRotSet = true;
    }
    else
    {
     //rotStopped = myLastActionRotStopped;
      //rotVal = myLastActionRotVal;
      //rotHeading = myLastActionRotHeading;
      rotStopped = true;
    }
      
    if (myActionRotSet && (myLastRotSent.mSecSince() > myStateReflectionRefreshTime ||
        rotStopped != myLastActionRotStopped || 
        rotVal != myLastActionRotVal || 
        rotHeading != myLastActionRotHeading))
    {
      if (rotStopped)
      {
        comInt(MvrCommands::RVEL, 0);
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot vel of 0 (rotStopped)", "");
      }
      else if (rotHeading)
      {
        comInt(MvrCommands::HEAD, rotVal);
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot heading of %d (encoder %d, raw %d)", "",
                      MvrMath::roundShort(MvrMath::addAngle(myActionDesired.getDeltaHeading(), getTh())),
                      MvrMath::roundShort(encTh),
                      MvrMath::roundShort(rotVal));
      }
      else
      {
        comInt(MvrCommands::RVEL, rotVal);
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%25sAction rot vel of %d", "", rotVal);
      }
      myLastRotSent.setToNow();
    }		    
    
    myLastActionRotVal = rotVal;
    myLastActionRotStopped = rotStopped;
    myLastActionRotHeading = rotHeading;
  }
  
  // start lat

  // if we don't have lat vel then don't do any of this
  if (!hasLatVel())
  {
  }
  // if this is true actions can't go
  else if ((myLatType != LAT_NONE && myDirectPrecedenceTime == 0) ||
	   (myLatType != LAT_NONE && myDirectPrecedenceTime != 0 && 
	    myLatSetTime.mSecSince() < myDirectPrecedenceTime))
  {
    myActionLatSet = false;
    latVal = MvrMath::roundShort(myLatVal);

    if (MvrMath::fabs(myLastSentLatVelMax - myLatVelMax) >= 1)
    {
      comInt(MvrCommands::SETLATV, MvrMath::roundShort(myLatVelMax));
      myLastSentLatVelMax = myLatVelMax;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "%12sNon-action lat max vel of %d", "", MvrMath::roundShort(myLatVelMax));
    }

    if (MvrMath::fabs(myLatAccel) > 1 && MvrMath::fabs(myLastSentLatAccel - myLatAccel) >= 1)
    {
      comInt(MvrCommands::LATACCEL, MvrMath::roundShort(myLatAccel));
      myLastSentLatAccel = myLatAccel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%12sNon-action lat accel of %d", "", MvrMath::roundShort(myLatAccel));
    }

    if (MvrMath::fabs(myLatDecel) > 1 && MvrMath::fabs(myLastSentLatDecel - myLatDecel) >= 1)
    {
      comInt(MvrCommands::LATACCEL, -MvrMath::roundShort(myLatDecel));
      myLastSentLatDecel = myLatDecel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%12sNon-action lat decel of %d", "", -MvrMath::roundShort(myLatDecel));
    }

    if (myLatType == LAT_VEL)
    {
      maxLatVel = MvrMath::roundShort(myLatVelMax);
      if (latVal > maxLatVel)
        latVal = maxLatVel;
      if (latVal < -maxLatVel)
        latVal = -maxLatVel;
      if (myLastLatVal != latVal || myLastLatType != myLatType || (myLastLatSent.mSecSince() >= myStateReflectionRefreshTime))
      {
        comInt(MvrCommands::LATVEL, MvrMath::roundShort(latVal));
        myLastLatSent.setToNow();
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%12sNon-action lat vel of %d", "", MvrMath::roundShort(latVal));
      }
      if (fabs((double)latVal) > (double).5)
      	myTryingToMove = true;
    }
    else if (myLatType == LAT_IGNORE)
    {
      //printf("No lat command sent\n");
    }
    else
      MvrLog::log(MvrLog::Terse, "MvrRobot::stateReflector: Invalid lat type %d.", myLatType);
    myLastLatVal = latVal;
    myLastLatType = myLatType;
  }
  else // if actions can go
  {
    if (MvrMath::fabs(myLastSentLatVelMax - myLatVelMax) >= 1)
    {
      comInt(MvrCommands::SETLATV, MvrMath::roundShort(myLatVelMax));
      myLastSentLatVelMax = myLatVelMax;
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat max vel of %d", "", MvrMath::roundShort(myLatVelMax));
    }
    
    // first we'll handle all of the accel decel things
    if (myActionDesired.getLatAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      latAccel = MvrMath::roundShort(myActionDesired.getLatAccel());
      if (MvrMath::fabs(latAccel) > 1 && MvrMath::fabs(myLastSentLatAccel - latAccel) >= 1)
      {
        comInt(MvrCommands::LATACCEL,MvrMath::roundShort(latAccel));
        myLastSentLatAccel = latAccel;
        if (myLogMovementSent)
          MvrLog::log(MvrLog::Normal, "%12sAction lat accel of %d", "", MvrMath::roundShort(latAccel));
      }
    }
    else if (MvrMath::fabs(myLatAccel) > 1 && MvrMath::fabs(myLastSentLatAccel - myLatAccel) >= 1)
    {
      comInt(MvrCommands::LATACCEL,
	     MvrMath::roundShort(myLatAccel));
      myLastSentLatAccel = myLatAccel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat accel of %d", "", MvrMath::roundShort(myLatAccel));
    }

    if (myActionDesired.getLatDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      latDecel = MvrMath::roundShort(myActionDesired.getLatDecel());
      if (MvrMath::fabs(latDecel) > 1 && MvrMath::fabs(myLastSentLatDecel - latDecel) >= 1)
      {
	      comInt(MvrCommands::LATACCEL, -MvrMath::roundShort(latDecel));
	      myLastSentLatDecel = latDecel;
	      if (myLogMovementSent)
	        MvrLog::log(MvrLog::Normal, "%12sAction lat decel of %d", "", -MvrMath::roundShort(latDecel));
      }
    }
    else if (MvrMath::fabs(myLatDecel) > 1 && MvrMath::fabs(myLastSentLatDecel - myLatDecel) >= 1)
    {
      comInt(MvrCommands::LATACCEL, -MvrMath::roundShort(myLatDecel));
      myLastSentLatDecel = myLatDecel;
      if (myLogMovementSent)
	      MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat decel of %d", "", -MvrMath::roundShort(myLatDecel));
    }

    if (myActionDesired.getMaxLeftLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      maxLeftLatVel = myActionDesired.getMaxLeftLatVel();
      if (maxLeftLatVel > myLatVelMax)
        maxLeftLatVel = myLatVelMax;
    }
    else
      maxLeftLatVel = myLatVelMax;

    if (myActionDesired.getMaxRightLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      maxRightLatVel = myActionDesired.getMaxRightLatVel();
      if (maxRightLatVel > myLatVelMax)
        maxRightLatVel = myLatVelMax;
    }
    else
      maxRightLatVel = myLatVelMax;

    if (myActionDesired.getLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      latVal = MvrMath::roundShort(myActionDesired.getLatVel());
      myActionLatSet = true;
    }
    else
    {
      //latVal = myLastActionLatVal; 
      latVal = 0;
    }

    if (fabs((double)latVal) > (double).5)
      myTryingToMove = true;

    //maxLatVel = MvrMath::roundShort(maxLatVel);
    if (latVal > 0 && latVal > maxLeftLatVel)
      latVal = maxLeftLatVel;
    if (latVal < 0 && latVal < -maxRightLatVel)
      latVal = -maxRightLatVel;

    if (myActionLatSet && (myLastLatSent.mSecSince() >= myStateReflectionRefreshTime || latVal != myLastActionLatVal))			     
    {
      comInt(MvrCommands::LATVEL, MvrMath::roundShort(latVal));
      myLastLatSent.setToNow();
      if (myLogMovementSent)
        MvrLog::log(MvrLog::Normal, "%12sAction lat vel of %d", "", MvrMath::roundShort(latVal));      
    }
    myLastActionLatVal = latVal;
  }
  // end lat

  if (myLastRotSent.mSecSince() > myStateReflectionRefreshTime &&
      myLastTransSent.mSecSince() > myStateReflectionRefreshTime &&
      myLastPulseSent.mSecSince() > myStateReflectionRefreshTime && 
      (!hasLatVel() || (hasLatVel() && myLastLatSent.mSecSince() > myStateReflectionRefreshTime)))
  {
    com(MvrCommands::PULSE);
    myLastPulseSent.setToNow();
    if (myLogMovementSent)
      MvrLog::log(MvrLog::Normal, "Pulse"); 
  }
}

MVREXPORT bool MvrRobot::handlePacket(MvrRobotPacket *packet)
{
  std::list<MvrRetFunctor1<bool, MvrRobotPacket *> *>::iterator it;
  bool handled;

  lock();

  if (myIgnoreNextPacket)
  {
    if ((packet->getID() == 0x32 || packet->getID() == 0x33))
    {
      MvrLog::log(MvrLog::Normal, "MvrRobot: Ignoring motor packet of type 0x%x", packet->getID());
      myIgnoreNextPacket = false;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, "MvrRobot: Ignoring packet of type 0x%x", packet->getID());
    }
    unlock();
    return false;
  }

  myConnectionTimeoutMutex.lock();
  myLastPacketReceivedTime = packet->getTimeReceived();
  myConnectionTimeoutMutex.unlock();

  if (packet->getID() == 0xff)
  {
    dropConnection("Losing connection because microcontroller reset.","because microcontroller reset");
    unlock();
    return false;
  }
  if (packet->getID() == 0xfe)
  {
    char buf[10000];
    sprintf(buf, "Losing connection because microcontroller reset with reset data");

    while (packet->getDataLength() - packet->getDataReadLength() > 0)
      sprintf(buf, "%s 0x%x", buf, packet->bufToUByte());
    dropConnection(buf, "Because microcontroller reset");
    unlock();
    return false;
  }

  for (handled = false, it = myPacketHandlerList.begin();
       it != myPacketHandlerList.end() && handled = false;
       it++)
  {
    if ((*it) != NULL && (*it)->invokeR(packet))
    {
      if (myPacketsReceivedTracking)
        MvrLog::log(MvrLog::Normal, "Handled by %s", (*it)->getName());
      handled = true;
    }
    else
    {
      packet->resetRead();
    }
  }
  if (!handled)
    MvrLog::log(MvrLog::Normal, "No packet handler wanted packet with ID: 0x%x", packet->getID());
  unlock();
  return handled;
}

/* @note You must first start the encoder packet stream by calling
 * requestEncoderPackets() before this function will return encoder values.
 */
MVREXPORT long ing MvrRobot::getLeftEncoder()
{
  return myLeftEncoder;
}

/* @note You must first start the encoder packet stream by calling
 * requestEncoderPackets() before this function will return encoder values.
 */
MVREXPORT long ing MvrRobot::getRightEncoder()
{
  return myRightEncoder;
}

/*
 * @internal
 *  This just locks the robot, so that its locked for all the user tasks
 */
MVREXPORT void MvrRobot::robotLocker(void)
{
  lock();
}

/*
 * @internal
 * This just unlocks the robot
 */
MVREXPORT void MvrRobot::robotUnlocker(void)
{
  unlock();
}

MVREXPORT void MvrRobot::packetHandler(void)
{
  if (myRunningNonThreaded)
    packetHandlerNonThreaded();
  else
    packetHandlerThreadedProcessor();
}

/*
 * This is here for use if the robot is running without threading
 * Reads in all of the packets that are available to read in, then runs through
 * the list of packet handlers and tries to get each packet handled.
 */
MVREXPORT void MvrRobot::packetHandlerNonThreaded(void)
{
  MvrRobotPacket *packet;
  int timeToWait;
  MvrTime start;
  bool sipHandled = false;

  if (myAsyncConnectFlag)
  {
    lock();
    asyncConnectHandler(false);
    unlock();
    return;
  }

  if (!isConnected())
    return;
  start.setToNow();
/*
 * The basic idea is that if we're chained to the sip we run through
 * and see if we have any packets available now (like if we got
 * backed up), we only check this for half the cycle time
 * though... if we know the cycle time of the robot (from config)
 * then we go for half that, if we don't know the cycle time of the
 * robot (from config) then we go for half of whatever our cycle time
 * is set to
 * 
 * if we don't have any packets waiting then we chill and wait for
 * it, if we got one, just get on with it
 */
  packet = NULL;
  // read all the packets that are available
  while ((packet = myReceiver.receivePacket(0)) != NULL)
  {
    if (myPacketsReceivedTracking)
    {
      MvrLog::log(MvrLog::Normal, "Rcvd: prePacket (%ld) 0x%x at %ld (%ld)",
                  myPacketsReceivedTrackingCount,
                  packet->getID(), start.mSecSince(),
                  myPacketsReceivedTrackingStarted.mSecSince());
    }
    handlePacket(packet);
    if ((packet->getID() & 0xf0) == 0x30)
      sipHandled = true;
    packet = NULL;

    // if we've taken too long here then break
    if ((getOrigRobotConfig()->hasPacketArrived() && start.mSecSince() > getOrigRobotConfig()->getSipCycleTime() / 2) ||
	      (!getOrigRobotConfig()->hasPacketArrived() && (unsigned int) start.mSecSince() > myCycleTime / 2))
    {
      break;
    }
  }

  if (isCycleChained())
    timeToWait = getCycleTime() * 2 - start.mSecSince();

  // if we didn't get sip and we're chained to the sip, wait for the sip
  while (isCycleChained() !sipHandled && isRunning() &&  (packet = myReceiver.receivePacket(timeToWait)) != NULL)
  {
    if (myPacketsReceivedTracking)
    {
      MvrLog::log(MvrLog::Normal, "Revd: Packet (%ld) 0x%x at %ld (%ld)",
                  myPacketsReceivedTrackingCount, 
                  packet->getID(), start.mSecSince(), 
                  myPacketsReceivedTrackingStarted.mSecSince());
      myPacketsReceivedTrackingCount++;
    }

    handlePacket(packet);
    if ((packet->getID() & 0xf0) == 0x30)
      break;
    timeToWait = getCycleTime() * 2 - start.mSecSince();
    if (timeToWait < 0)
      timeToWait = 0;
  }
  
  myConnectionTimeoutMutex.lock();
  if (myTimeoutTime > 0 && ((-myLastOdometryReceivedTime.mSecTo()) > myTimeoutTime))
  {
    char buf[10000];
    sprintf(buf, "Losing connection because no odometry received from microcontroller in %ld milliseconds (greater than the timeout of %d).", 
            (-myLastOdometryReceivedTime.mSecTo()),
            myTimeoutTime);
    myConnectionTimeoutMutex.unlock();
    dropConnection(buf, "because lost connection to microcontroller");
  }
  else
  {
    myConnectionTimeoutMutex.unlock();
  }

  myConnectionTimeoutMutex.lock();
  if (myTimeoutTime > 0 && ((-myLastPacketReceivedTime.mSecTo()) > myTimeoutTime))
  {
    char buf[10000];
    sprintf(buf, "Losing connection because nothing received from robot in %ld milliseconds (greater than the timeout of %d).", 
            (-myLastPacketReceivedTime.mSecTo()),
            myTimeoutTime);
    myConnectionTimeoutMutex.unlock();
    dropConnection(buf, "because lost connection to microcontroller");
  }
  else
  {
    myConnectionTimeoutMutex.unlock();
  }

  if (myPacketsReceivedTracking)
    MvrLog::log(MvrLog::Normal, "Rcvd(nt): time taken %ld", start.mSecSince());
}

MVREXPORT void MvrRobot::packetHandlerThreadedProcessor(void)
{
  MvrRobotPacket *packet;
  int timeToWait;
  MvrTime start;
  bool sipHandled = false;
  bool anothersip = false;
  std::list<MvrRobotPacket *>::iterator it;

  if (myAsyncConnectFlag)
  {
    lock();
    asyncConnectHandler(false);
    unlock();
    return;
  }

  if (!isConnected)
    return;
  //MvrLog::log(MvrLog::Normal, "Rcvd: start %ld", myPacketsReceivedTrackingStarted.mSecSince());

  start.setToNow();
  // read all the packets that are available in our time window (twice
  // packet cycle), if we get the sip we stop...
  while (!sipHandled && isRunning())
  {
    packet = NULL;
    anotherSip = false;
    myPacketMutex.lock();
    if (!myPacketList.empty())
    {
      packet = myPacketList.front();
      myPacketList.pop_front();
      // see if there are more sips, since if so we'll keep chugging
      // throw the list
      for (it = myPacketList.begin();
           !anotherSip && it != myPacketList.end();
           it++)
      {
        if (((*it)->getID() & 0xf0) == 0x30)
          anotherSip = true;
      }
      myPacketMutex.unlock();
    }
  }
}