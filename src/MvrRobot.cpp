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


/**
 * The parameters only rarely need to be specified.
 *
 * @param name A name for this robot, useful if a program has more than one
 *  MvrRobot object 

 
   @param obsolete This parameter is ignored.
   (It used to turn off state reflection if false, but that
   is no longer possible.)

   @param doSigHandle do normal signal handling and have this robot
   instance stopRunning() when the program is signaled

   @param normalInit whether the robot should initializes its
   structures or the calling program will take care of it.  No one
   will probalby ever use this value, since if they are doing that
   then overriding will probably be more useful, but there it is.

   @param addMvriaExitCallback If true (default), add callback to global Mvria class
    to stop running the processing loop and disconnect from robot 
    when Mvria::exit() is called. If false, do not disconnect on Mvria::exit()

**/

MVREXPORT MvrRobot::MvrRobot(const char *name, bool obsolete, 
			  bool doSigHandle, bool normalInit,
			  bool addMvriaExitCallback) :
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
  myMvriaExitCB(this, &MvrRobot::ariaExitCallback),
  myPoseInterpPositionCB(this, &MvrRobot::getPoseInterpPosition),
  myEncoderPoseInterpPositionCB(this, &MvrRobot::getEncoderPoseInterpPosition)
{
  myMutex.setLogName("MvrRobot::myMutex");
  myPacketMutex.setLogName("MvrRobot::myPacketMutex");
  myConnectionTimeoutMutex.setLogName("MvrRobot::myConnectionTimeoutMutex");

  setName(name);
  myMvriaExitCB.setName("MvrRobotExit");
  myNoTimeWarningThisCycle = false;
  myGlobalPose.setPose(0, 0, 0);
  mySetEncoderTransformCBList.setName("SetEncoderTransformCBList");

  myParams = new MvrRobotGeneric("");
  processParamFile();

  myRunningNonThreaded = true;

  myMotorPacketCB.setName("MvrRobot::motorPacket");
  myEncoderPacketCB.setName("MvrRobot::encoderPacket");
  myIOPacketCB.setName("MvrRobot::IOPacket");

  myInterpolation.setName("MvrRobot::Interpolation");
  myEncoderInterpolation.setName("MvrRobot::EncoderInterpolation");

  myPtz = NULL;
  myKeyHandler = NULL;
  myKeyHandlerCB = NULL;
  myMTXTimeUSecCB = NULL;

  myConn = NULL;

  myOwnTheResolver = false;

  myBlockingConnectRun = false;
  myAsyncConnectFlag = false;

  myRequireConfigPacket = false;

  myLogMovementSent = false;
  myLogMovementReceived = false;
  myLogVelocitiesReceived = false;
  myLogActions = false;
  myLastVel = 0;
  myLastRotVel = 0;
  myLastHeading = 0;
  myLastDeltaHeading = 0;

  myKeepControlRaw = false;

  myPacketsSentTracking = false;
  myPacketsReceivedTracking = false;
  myPacketsReceivedTrackingCount = false;
  myPacketsReceivedTrackingStarted.setToNow();

  myLogSIPContents = false;

  myCycleTime = 100;
  myCycleWarningTime = 250;
  myConnectionCycleMultiplier = 2;
  myTimeoutTime = 8000;
  myStabilizingTime = 0;
  myCounter = 1;
  myResolver = NULL;
  myNumSonar = 0;

  myRequestedIOPackets = false;
  myRequestedEncoderPackets = false;
  myEncoderCorrectionCB = NULL;

  myCycleChained = true;

  myMoveDoneDist = 40;
  myHeadingDoneDiff = 3;

  myStoppedVel = 4;
  myStoppedRotVel = 1;
  myStoppedLatVel = 4;

  myOrigRobotConfig = NULL;
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
  myDoNotSwitchBaud = false;

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
  MvrUtil::deleteSetPairs(mySonars.begin(), mySonars.end());
  Mvria::delRobot(this);

  if (myKeyHandlerCB != NULL)
    delete myKeyHandlerCB;

  for (it = myActions.begin(); it != myActions.end(); ++it)
  {
    (*it).second->setRobot(NULL);
  }
}


/**
   Sets up the packet handlers, sets up the sync list and makes the default
   priority resolver.
**/
MVREXPORT void MvrRobot::init(void)
{
  setUpPacketHandlers();
  setUpSyncList();
  myOwnTheResolver = true;
  myResolver = new MvrPriorityResolver;
}

/**
   This starts the ongoing main loop, which invokes robot tasks until
   stopped.
   This function does not return until the loop is stopped by a call to stopRunning(),
   or if 'true' is given for @a stopRunIfNotConnected, and the robot connection
   is closed or fails.

   @param stopRunIfNotConnected if true, the run will return if there
   is no connection to the robot at any given point, this is good for
   one-shot programs... if it is false the run won't return unless
   stop is called on the instance

   @param runNonThreaded if true, the robot won't make the usual
   thread that it makes for reading the packets...you generally
   shouldn't use this, but it's provided in case folks are using
   programs without threading

   @sa stopRunning()
**/
MVREXPORT void MvrRobot::run(bool stopRunIfNotConnected, bool runNonThreaded)
{
  if (mySyncLoop.getRunning())
  {
    MvrLog::log(MvrLog::Terse, 
	       "The robot is already running, cannot run it again.");
    return;
  }
  mySyncLoop.setRunning(true);
  mySyncLoop.stopRunIfNotConnected(stopRunIfNotConnected);
  mySyncLoop.runInThisThread();

  myRunningNonThreaded = runNonThreaded;
  if (!myRunningNonThreaded)
    // Joinable, but do NOT lower priority. The robot packet reader
    // thread is the most important one around, since all the timing is
    // based off that one.
    myPacketReader.create(true, false);
}

/**
   @return true if the robot is currently running in a run or runAsync, 
   otherwise false
**/

MVREXPORT bool MvrRobot::isRunning(void) const
{
  return (mySyncLoop.getRunning() && 
	  (myRunningNonThreaded || myPacketReader.getRunning()));
}

/**
   This starts a new thread then has runs through the tasks until stopped.
   This function doesn't return until something calls stop on this instance.
   This function returns immediately
   @param stopRunIfNotConnected if true, the run will stop if there is no 
   connection to the robot at any given point, this is good for one-shot 
   programs. If  false then the thread will continue until stopRunning() is called.
   @param runNonThreadedPacketReader selects whether the packet reader object
(receives and parses packets from the robot) should run in its own internal
asychronous thread.  Mostly for internal use.
   @sa stopRunning()
   @sa waitForRunExit()
   @sa lock()
   @sa unlock()
**/
MVREXPORT void MvrRobot::runAsync(bool stopRunIfNotConnected, bool runNonThreadedPacketReader)
{
  if (mySyncLoop.getRunning())
  {
    MvrLog::log(MvrLog::Terse, 
	       "The robot is already running, cannot run it again.");
    return;
  }
  if (runNonThreadedPacketReader)
  {
    mySyncLoop.stopRunIfNotConnected(stopRunIfNotConnected);
    // Joinable, but do NOT lower priority. The robot thread is the most
    // important one around. (this isn't true anymore, since the robot
    // packet reading one is more important for timing)
    mySyncLoop.create(true, false);
    myRunningNonThreaded = true;
  }
  else
  {
    myRunningNonThreaded = false;
    // Joinable, but do NOT lower priority. The robot thread is the most
    // important one around. (this isn't true anymore, since the robot
    // packet reading one is more important for timing)
    mySyncLoop.create(true, true);
    // Joinable, but do NOT lower priority. The robot packet reader
    // thread is the most important one around, since all the timing is
    // based off that one.
    myPacketReader.create(true, false);
  }
}

/**
   This stops this robot's processing cycle.  If it is running
   in a background thread (from runAsync()), it will cause that thread
   to exit.  If it is running synchronously (from run()), then run() will
   return.  
   @param doDisconnect If true, also disconnect from the robot connection (default is true). 
   @sa isRunning
**/
MVREXPORT void MvrRobot::stopRunning(bool doDisconnect)
{
  if (myKeyHandler != NULL)
    myKeyHandler->restore();
  mySyncLoop.stopRunning();
  myBlockingConnectRun = false;
  if (doDisconnect && (isConnected() || myIsStabilizing))
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
  mySyncTaskRoot->addNewLeaf("Packet Handler", 85, &myPacketHandlerCB);
  mySyncTaskRoot->addNewLeaf("Robot Locker", 70, &myRobotLockerCB);
  mySyncTaskRoot->addNewBranch("Sensor Interp", 65);
  mySyncTaskRoot->addNewLeaf("Action Handler", 55, &myActionHandlerCB);
  mySyncTaskRoot->addNewLeaf("State Reflector", 45, &myStateReflectorCB);
  mySyncTaskRoot->addNewBranch("User Tasks", 25);
  mySyncTaskRoot->addNewLeaf("Robot Unlocker", 20, &myRobotUnlockerCB);
}


MVREXPORT void MvrRobot::setUpPacketHandlers(void)
{
  addPacketHandler(&myMotorPacketCB, MvrListPos::FIRST);
  addPacketHandler(&myEncoderPacketCB, MvrListPos::LAST);
  addPacketHandler(&myIOPacketCB, MvrListPos::LAST);
}

void MvrRobot::reset(void)
{
  resetTripOdometer();

  myOdometerDistance = 0;
  myOdometerDegrees = 0;
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

  myFirstEncoderPose = true;
  myFakeFirstEncoderPose = false;
  myIgnoreNextPacket = false;
  //myEncoderPose.setPose(0, 0, 0);
  //myEncoderPoseTaken.setToNow();
  //myRawEncoderPose.setPose(0, 0, 0);
  //myGlobalPose.setPose(0, 0, 0);
  //myEncoderTransform.setTransform(myEncoderPose, myGlobalPose);
  myTransVelMax = 0;
  myTransNegVelMax = 0;
  myTransAccel = 0;
  myTransDecel = 0;
  myRotVelMax = 0;
  myRotAccel = 0;
  myRotDecel = 0;
  myLatVelMax = 0;
  myLatAccel = 0;
  myLatDecel = 0;

  myAbsoluteMaxTransVel = 0;
  myAbsoluteMaxTransNegVel = 0;
  myAbsoluteMaxTransAccel = 0;
  myAbsoluteMaxTransDecel = 0;
  myAbsoluteMaxRotVel = 0;
  myAbsoluteMaxRotAccel = 0;
  myAbsoluteMaxRotDecel = 0;
  myAbsoluteMaxLatVel = 0;
  myAbsoluteMaxLatAccel = 0;
  myAbsoluteMaxLatDecel = 0;
  

  myLeftVel = 0;
  myRightVel = 0;
  myBatteryVoltage = 13;
  myRealBatteryVoltage = 13;
  myBatteryAverager.clear();
  myBatteryAverager.add(myBatteryVoltage);


  myStallValue = 0;
  myControl = 0;
  myFlags = 0;
  myFaultFlags = 0;
  myHasFaultFlags = 0;
  myCompass = 0;
  myAnalogPortSelected = 0;
  myAnalog = 0;
  myDigIn = 0;
  myDigOut = 0;
  myIOAnalogSize = 0;
  myIODigInSize = 0;
  myIODigOutSize = 0;
  myLastIOPacketReceivedTime.setSec(0);
  myLastIOPacketReceivedTime.setMSec(0);
  myVel = 0;
  myRotVel = 0;
  myLatVel = 0;
  myOverriddenChargeState = false;
  myChargeState = CHARGING_UNKNOWN;
  myOverriddenIsChargerPowerGood = false;


  myLastX = 0;
  myLastY = 0;
  myLastTh = 0;
  mySentPulse = false;
  myIsConnected = false;
  myIsStabilizing = false;

  myTransVal = 0;
  myTransVal2 = 0;
  myTransType = TRANS_NONE;
  myLastTransVal = 0;
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

  myLatVal = 0;
  myLatType = LAT_NONE;
  myLastLatVal = 0;
  myLastLatType = LAT_NONE;
  myLastLatSent.setToNow();
  myActionLatSet = false;

  myLastSentTransVelMax = 0;
  myLastSentTransAccel = 0;
  myLastSentTransDecel = 0;
  myLastSentRotVelMax = 0;
  myLastSentRotVelPosMax = 0;
  myLastSentRotVelNegMax = 0;
  myLastSentRotAccel = 0;
  myLastSentRotDecel = 0;
  myLastSentLatVelMax = 0;
  myLastSentLatAccel = 0;
  myLastSentLatDecel = 0;

  myLastPulseSent.setToNow();

  myDirectPrecedenceTime = 0;
  myStateReflectionRefreshTime = 500;

  myActionDesired.reset();

  myMotorPacCurrentCount = 0;
  myMotorPacCount = 0;
  myTimeLastMotorPacket = 0;

  mySonarPacCurrentCount = 0;
  mySonarPacCount = 0;
  myTimeLastSonarPacket = 0;

  myLeftEncoder = 0;
  myRightEncoder = 0;

  myWarnedAboutExtraSonar = false;
  myOdometryDelay = 0;

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
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransNegVelMax of %g is below 0, ignoring it", negVel);
    return;
  }

  if (negVel < myAbsoluteMaxTransNegVel)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobot: setTransNegVelMax of %g is over the absolute max vel of %g, capping it", negVel, myAbsoluteMaxTransNegVel);
    negVel = myAbsoluteMaxTransNegVel;
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

/**
   Sets the connection this instance uses to the actual robot.  This is where
   commands will be sent and packets will be received from

   @param connection The deviceConnection to use for this robot

   @see MvrDeviceConnection, MvrSerialConnection, MvrTcpConnection
**/
MVREXPORT void MvrRobot::setDeviceConnection(MvrDeviceConnection *connection)
{
  myConn = connection;
  myConn->setDeviceName("uC");
  mySender.setDeviceConnection(myConn);
  myReceiver.setDeviceConnection(myConn);
}

/**
   Gets the connection this instance uses to the actual robot.  This is where
   commands will be sent and packets will be received from
   @return the deviceConnection used for this robot
   @see MvrDeviceConnection
   @see MvrSerialConnection
   @see MvrTcpConnection
**/
MVREXPORT MvrDeviceConnection *MvrRobot::getDeviceConnection(void) const
{
  return myConn;
}

/**
   Sets the number of milliseconds to go without a response from the robot
   until it is assumed that the connection with the robot has been
   broken and the disconnect on error events will happen.  Note that
   this will only happen with the default packet handler.

   @param mSecs if seconds is 0 then the connection timeout feature
   will be disabled, otherwise disconnect on error will be triggered
   after this number of seconds...  
**/
MVREXPORT void MvrRobot::setConnectionTimeoutTime(int mSecs)
{
  myConnectionTimeoutMutex.lock();

  MvrLog::log(MvrLog::Normal, 
	     "MvrRobot::setConnectionTimeoutTime: Setting timeout to %d mSecs", 
	     mSecs);

  myLastOdometryReceivedTime.setToNow();
  myLastPacketReceivedTime.setToNow();

  if (mSecs > 0)
    myTimeoutTime = mSecs;
  else
    myTimeoutTime = 0;

  myConnectionTimeoutMutex.unlock();
}

/**
   Gets the number of seconds to go without response from the robot
   until it is assumed tha tthe connection with the robot has been
   broken and the disconnect on error events will happen.

**/
MVREXPORT int MvrRobot::getConnectionTimeoutTime(void) 
{
  myConnectionTimeoutMutex.lock();
  int ret = myTimeoutTime;
  myConnectionTimeoutMutex.unlock();
  return ret;
}

/**
   Get the time at which the most recent packet of any type was received
   This timestamp is updated whenever any packet is received from the robot.
   @sa getLastOdometryTime

   @return the time the last packet was received
**/
MVREXPORT MvrTime MvrRobot::getLastPacketTime(void) 
{
  myConnectionTimeoutMutex.lock();
  MvrTime ret = myLastPacketReceivedTime;
  myConnectionTimeoutMutex.unlock();
  return ret;
}


/**
   Get the time at which thet most recent SIP packet was received.
   Whenever a SIP packet (aka "motor" packet)  is received, this time is set to the receiption
   time minus the robot's reported internal odometry delay. The SIP provides
   robot position (see getPose()) and some other information (see robot
   manual for details).

   @return the time the last SIP was received
**/
MVREXPORT MvrTime MvrRobot::getLastOdometryTime(void) 
{
  myConnectionTimeoutMutex.lock();
  MvrTime ret = myLastOdometryReceivedTime;
  myConnectionTimeoutMutex.unlock();
  return ret;
}


/** 
    Sets up the robot to connect, then returns, but the robot must be
    running (ie from runAsync) before you do this.  

    Most programs should use MvrRobotConnector to perform the connection
    rather than using thin method directly.

    This function will
    fail if the robot is already connected.  If you want to know what
    happened because of the connect then look at the callbacks.  NOTE,
    this will not lock robot before setting values, so you MUST lock
    the robot before you call this function and unlock the robot after
    you call this function.  If you fail to lock the robot, you'll may
    wind up with wierd behavior.  Other than the aspect of blocking or
    not the only difference between async and blocking connects (other
    than the blocking) is that async is run every robot cycle, whereas
    blocking runs as fast as it can... also blocking will try to
    reconnect a radio modem if it looks like it didn't get connected
    in the first place, so blocking can wind up taking 10 or 12
    seconds to decide it can't connect, whereas async doesn't try hard
    at all to reconnect the radio modem (beyond its first try) (under
    the assumption the async connect is user driven, so they'll just
    try again, and so that it won't mess up the sync loop by blocking
    for so long).

    @return true if the robot is running and the robot will try to
    connect, false if the robot isn't running so won't try to connect
    or if the robot is already connected

    @see addConnectCB

    @see addFailedConnectCB

    @see runAsync
**/

MVREXPORT bool MvrRobot::asyncConnect(void)
{
  if (!mySyncLoop.getRunning() || isConnected())
    return false;
  
  myAsyncConnectFlag = true;
  myBlockingConnectRun = false;
  myAsyncConnectState = -1;
  return true;
}

/** 
    Connects to the robot, returning only when a connection has been
    made or it has been established a connection can't be made.  

    Most programs should use an MvrRobotConnector object to perform
    the connection rather than using this method directly.

    This
    connection usually is fast, but can take up to 30 seconds if the
    robot is in a wierd state (this is not often).  If the robot is
    connected via MvrSerialConnection then the connect will also
    connect the radio modems.  Upon a successful connection all of the
    Connection Callback Functors that have been registered will be
    called.  NOTE, this will lock the robot before setting values, so
    you MUST not have the robot locked from where you call this
    function.  If you do, you'll wind up in a deadlock.  This behavior
    is there because otherwise you'd have to lock the robot before
    calling this function, and normally blockingConnect will be called
    from a separate thread, and that thread won't be doing anything
    else with the robot at that time.  Other than the aspect of
    blocking or not the only difference between async and blocking
    connects (other than the blocking) is that async is run every
    robot cycle, whereas blocking runs as fast as it can... also
    blocking will try to reconnect a radio modem if it looks like it
    didn't get connected in the first place, so blocking can wind up
    taking 10 or 12 seconds to decide it can't connect, whereas async
    doesn't try hard at all to reconnect the radio modem (under the
    assumption the async connect is user driven, so they'll just try
    again, and so that it won't mess up the sync loop by blocking for
    so long).

    @return true if a connection could be made, false otherwise 

**/

MVREXPORT bool MvrRobot::blockingConnect(void)
{
  int ret = 0;

  lock();
  if (myIsConnected)
    disconnect();
  myBlockingConnectRun = true;
  myAsyncConnectState = -1;
  while ((ret = asyncConnectHandler(true)) == 0 && myBlockingConnectRun)
    MvrUtil::sleep(MvrMath::roundInt(getCycleTime() * .80));
  unlock();
  if (ret == 1)
    return true;
  else
    return false;
}

/**
   This is an internal function that is used both for async connects and 
   blocking connects use to connect.  It does about the same thing for both,
   and it should only be used by asyncConnect and blockingConnect really.
   But here it is.  The only difference between when its being used 
   by blocking/async connect is that in blocking mode if it thinks there
   may be problems with the radio modem it pauses for two seconds trying
   to deal with this... whereas in async mode it tries to deal with this in a
   simpler way.
   @param tryHarderToConnect if this is true, then if the radio modems look
   like they aren't working, it'll take about 2 seconds to try and connect 
   them, whereas if its false, it'll do a little try, but won't try very hard
   @return 0 if its still trying to connect, 1 if it connected, 2 if it failed
**/

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
  endTime.addMSec(getCycleTime()*myConnectionCycleMultiplier);
  
  myNoTimeWarningThisCycle = true;

  // if this is -1, then we're doing initialization stuff, then returning
  if (myAsyncConnectState == -1)
  {
    myAsyncConnectSentChangeBaud = false;
    myAsyncConnectNoPacketCount = 0;
    myAsyncConnectTimesTried = 0;
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
      /*str = myConn->getStatusMessage(myConn->getStatus());
        MvrLog::log(MvrLog::Terse, "Trying to connect to robot but connection not opened, it is %s", str.c_str());*/
        MvrLog::log(MvrLog::Terse, "Could not connect, because open on the device connection failed.");
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
      myRobotSubType = con->mySubtype;
      if (con->havePose)        // we know where to move
        moveTo(con->myPose);
      setCycleChained(false);   // don't process packets all at once
      madeConnection();
      finishedConnection();
      return 1;
    }


    // needed for the old infowave radio modem:
    if (dynamic_cast<MvrSerialConnection *>(myConn))
    {
      myConn->write("WMS2\15", strlen("WMS2\15"));
    }

    // here we're flushing out all previously received packets
    while ( endTime.mSecTo() > 0 && myReceiver.receivePacket(0) != NULL);

    myAsyncConnectState = 0;
    return 0;
  }
  MvrLog::log(MvrLog::Normal,"myAsyncConnectState myAsyncConnectState %d", myAsyncConnectState);

  if (myAsyncConnectState >= 3)
  {
    bool handled;
    std::list<MvrRetFunctor1<bool, MvrRobotPacket *> *>::iterator it;
    while ((packet = myReceiver.receivePacket(0)) != NULL)
    {
      //printf("0x%x\n", packet->getID());
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
    //if (!myOrigRobotConfig->hasPacketBeenRequested())
    if (!myOrigRobotConfig->hasPacketArrived())
    {
      myOrigRobotConfig->requestPacket();
    }
    // if we've gotten our config packet or if we've timed out then
    // set our vel and acc/decel params and skip to the next part
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
    if (!myOrigRobotConfig->hasPacketArrived() || !myOrigRobotConfig->getResetBaud() || serConn == NULL ||myParams->getSwitchToBaudRate() == 0 || 
	      (serConn != NULL && serConn->getBaud() >= myParams->getSwitchToBaudRate()) ||myDoNotSwitchBaud)
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
        MvrLog::log(MvrLog::Normal, "Warning: SwitchToBaud is set to %d baud, ignoring.",
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
    if (myStabilizingTime == 0 || 
	myStartedStabilizing.mSecSince() > myStabilizingTime)
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
	 && myAsyncConnectNoPacketCount >= myAsyncConnectTimesTried)
	|| (myAsyncConnectNoPacketCount > 10))
    {
      MvrLog::log(MvrLog::Terse, "Could not connect, no robot responding.");
      failedConnect();
      return 2;
    }

      /* This code to connect the radio modems should never really be needed 
	 so is being removed for now
	 if (myAsyncConnectNoPacketCount >= 4)
	 {
	 
	 if (tryHarderToConnect && dynamic_cast<MvrSerialConnection *>(myConn))
	 {
	 MvrLog::log(MvrLog::Normal, 
	 "Radio modem may not connected, trying to connect it.");
	 MvrUtil::sleep(1000);
	 myConn->write("|||\15", strlen("!!!\15"));
	 MvrUtil::sleep(60);
	 myConn->write("WMN\15", strlen("WMN\15"));
	 MvrUtil::sleep(60);
	 myConn->write("WMS2\15", strlen("WMS2\15"));
	 MvrUtil::sleep(1000);
	 } 
	 else if (dynamic_cast<MvrSerialConnection *>(myConn))
	 {
	 MvrLog::log(MvrLog::Normal,
	 "Radio modem may not connected, trying to connect it.");
	 myConn->write("WMS2\15", strlen("WMS2\15"));
	 MvrUtil::sleep(60);
	 }
	 }
      */
    
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
      MvrLog::log(MvrLog::Normal,
		 " Robot may be connected but not open, trying to dislodge.");
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

/**
   @return true if the file could be loaded, false otherwise
 **/
MVREXPORT bool MvrRobot::loadParamFile(const char *file)
{
  if (myParams != NULL)
    delete myParams;

  myParams = new MvrRobotGeneric("");
  if (!myParams->parseFile(file, false, true))
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::loadParamFile: Could not find file '%s' to load.", file);
    return false;
  }
  processParamFile();
  MvrLog::log(MvrLog::Normal, "Loaded robot parameters from %s.", file);
  return true;
}

/**
   This function will take over ownership of the passed in params
 **/
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


	// PS 9/10/12 - added units to get MTX working but it
	// broke old sonar - so now trying the old way
  //for (int i = 0; i < myParams->getNumSonarUnits(); ++i)
  for (int i = 0; i < myParams->getNumSonar(); ++i)
  {
    //printf("sonar %d %d %d %d\n", i, myParams->getSonarX(i),
    //myParams->getSonarY(i), myParams->getSonarTh(i));
    if (mySonars.find(i) == mySonars.end())
    {
			MvrLog::log(MvrLog::Verbose,"MvrRobot::processParamFile creating new sonar %d %d %d %d", i, myParams->getSonarX(i),
									myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i] = new MvrSensorReading(myParams->getSonarX(i),
					myParams->getSonarY(i), 
					myParams->getSonarTh(i));
      mySonars[i]->setIgnoreThisReading(true);
    }
    else
    {
			MvrLog::log(MvrLog::Verbose,"MvrRobot::processParamFile resetting sonar %d %d %d %d", i, myParams->getSonarX(i),
									myParams->getSonarY(i), myParams->getSonarTh(i));
      mySonars[i]->resetSensorPosition(myParams->getSonarX(i),
				      myParams->getSonarY(i), 
				      myParams->getSonarTh(i));
      mySonars[i]->setIgnoreThisReading(true);
    }
    if ((i + 1) > myNumSonar)
      myNumSonar = i + 1;
  }

  //myRobotType = myParams->getClassName();
  //myRobotSubType = myParams->getSubClassName();

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
    MvrLog::log(MvrLog::Normal, "Explicitly not loading ARIA Pioneer robot parameter files");
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
    MvrUtil::strcasecmp(myRobotSubType, "marc_devel") == 0 || // subtype used in early versions of MARCOS firmware
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

  // "marc_devel" is subtype given by early MTX core firmware,
  // but it ought to be pioneer-lx for research pioneer lx.
  if(MvrUtil::strcasecmp(myRobotSubType, "marc_devel") == 0)
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
      MvrLog::log(MvrLog::Normal, 
		 "Loaded robot parameters from %s", 
		 subtypeParamFileName.c_str());
  /* If the above line was replaced with this one line
     paramFile->load(); 
     then the sonartest (and lots of other stuff probably) would break
  */
  // then the one for the particular name, if we can
  nameParamFileName = Mvria::getDirectory();
  nameParamFileName += "params/";
  nameParamFileName += myRobotName;
  nameParamFileName += ".p";
  if ((loadedNameParam = myParams->parseFile(nameParamFileName.c_str(),
					     true, true)))
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

/** Encoder packet data is stored by MvrRobot and can be read using
 * getLeftEncoder() and getRightEncoder().
 *
 *  Encoder packets may be stopped with stopEncoderPackets().
 *
 * @note Encoder data is not available with all robot types.  It is currently
 * only available from robots with SH controllers running ARCOS (Pioneer 3,
 * PowerBot, PeopleBot, AmigoBot).  If a robot does not support encoder data,
 * this request will be ignored.
 * 
 * Encoder packets are sent after the main SIP (motor packet) by ARCOS. If you want to 
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

/**
  Note that not all robots have built-in GPIO, and so will not return any
  response to this request.  Refer to your robot's operations manual for
  information on whether and what kind of digital and analog IO interface
  it has.  (Pioneer 3, PeopleBot and PowerBot 
  have digital IO built in to the robot.  On Pioneer LX, use MvrMTXIO instead.
  If running on Linux on a Versalogic EBX-12 computer, MvrVersalogicIO can be
  used to access the EBX-12 computer's IO interface.  Otherwise, a user-added
  peripheral DAC interface is suggested.  )
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
  myIsConnected = true;
  myAsyncConnectFlag = false;
  myBlockingConnectRun = false;
  resetTripOdometer();

  for (it = myConnectCBList.begin(); it != myConnectCBList.end(); it++)
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

/** 
  Disconnects from a robot.  This also calls of the DisconnectNormally 
  Callback Functors if the robot was actually connected to a robot when 
  this member was called. Then it disables the robot motors, sends CLOSE
  command to robot, waits one seconds, and closes the communications connection.
  This disconnection normally happens automatically when the program exits
  by calling Mvria::exit(), or if communication to the robot fails during 
  the run cycle, and this method does not need to be explicitly called.
  @return true if not connected to a robot (so no disconnect can happen, but 
  it didn't failed either), also true if the command could be sent to the 
  robot (ie connection hasn't failed)
 **/

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

MVREXPORT void MvrRobot::dropConnection(const char *technicalReason, 
				      const char *userReason)
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
  //std::list<MvrFunctor *>::iterator it;  

  MvrLog::log(MvrLog::Verbose, "Cancelled connection to the microcontroller because of command.");
  myIsConnected = false;
  myNoTimeWarningThisCycle = true;
  myIsStabilizing = false;
  return;
}

/**
  Stops the robot, by telling it to have a translational velocity and 
  rotational velocity of 0.  Also note that if you are using actions, this 
  will cause the actions to be ignored until the direct motion precedence 
  timeout has been exceeded or clearDirectMotion is called.
  @see setDirectMotionPrecedenceTime
  @see getDirectMotionPrecedenceTime
  @see clearDirectMotion
 **/
MVREXPORT void MvrRobot::stop(void)
{
  comInt(MvrCommands::VEL, 0);
  comInt(MvrCommands::RVEL, 0);
  if (hasLatVel())
    comInt(MvrCommands::LATVEL, 0);
  setVel(0);
  setRotVel(0);
  setLatVel(0);
  myLastActionTransVal = 0;
  myLastActionRotStopped = true;
  myLastActionRotHeading = false;
  myLastActionLatVal = 0;
}

/**
  Sets the desired translational velocity of the robot. 
  MvrRobot caches this value, and sends it with a VEL command during the next
cycle. 
  The value is rounded to the nearest whole number mm/sec before
  sent to the robot controller.

  @param velocity the desired translational velocity of the robot (mm/sec)
 **/
MVREXPORT void MvrRobot::setVel(double velocity)
{
  myTransType = TRANS_VEL;
  myTransVal = velocity;
  myTransVal2 = 0;
  myTransSetTime.setToNow();
}

/**
  Sets the desired lateral (sideways on local Y axis) translational velocity of the robot. 
  MvrRobot caches this value, and sends it with a LATVEL command during the next cycle.
  (Only has effect on robots that are capable of translating laterally, i.e.
  Seekur. Other robots will ignore the command.)
  The value is rounded to the nearest whole number mm/sec before
  sent to the robot controller.

  @param latVelocity the desired translational velocity of the robot (mm/sec)
 **/
MVREXPORT void MvrRobot::setLatVel(double latVelocity)
{
  myLatType = LAT_VEL;
  myLatVal = latVelocity;
  myLatSetTime.setToNow();
}

/** 
  Sends command to set the robot's velocity differentially, with
  separate velocities for left and right sides of the robot. The
  MvrRobot caches these values, and sends them with 
  a VEL2 command during the next
  cycle.  Note that this cancels both translational velocity AND
  rotational velocity, and is canceled by any of the other direct
  motion commands.  
  The values are rounded to the nearest whole number mm/sec before
  sent to the robot controller.

  @note This method of controlling the robot is primarily for compatibility with
  old robots and is not recommended.  Instead use setVel() and setRotVel() to
  set translational and rotational components of robot motion.

  @note Not all robots (e.g. Pioneer LX) implement this command it, it may be ignored.
  There may also be some differences in how different robot firmware types
  implement this command. See robot operations manual for more details.

  @sa setVel
  @sa setRotVel
  @sa clearDirectMotion

  @deprecated

  @param leftVelocity the desired velocity of the left wheel 
  @param rightVelocity the desired velocity of the right wheel
 **/
MVREXPORT void MvrRobot::setVel2(double leftVelocity, double rightVelocity)
{
  myTransType = TRANS_VEL2;
  myTransVal = leftVelocity;
  myTransVal2 = rightVelocity;
  myRotType = ROT_IGNORE;
  myRotVal = 0;
  myTransSetTime.setToNow();
}

/**
  Tells the robot to begin moving the specified distance forward/backwards.
  MvrRobot caches this value, and sends it during the next cycle (with a MOVE
  or VEL command, depending on the robot).

  @param distance the distance for the robot to move (millimeters). Note, 
  due to the implementation of the MOVE command on some robots, it is best to
  restrict the distance to the range (5000mm, 5000mm] if possible. Not all
  robots have this restriction, however.
 **/
MVREXPORT void MvrRobot::move(double distance)
{
  myTransType = TRANS_DIST_NEW;
  myTransDistStart = getPose();
  myTransVal = distance;
  myTransVal2 = 0;
  myTransSetTime.setToNow();
}

/**
  Determines if a move command is finished, to within a small
  distance threshold, "delta".  If delta = 0 (default), the delta distance 
  set with setMoveDoneDist() will be used.

  @param delta how close to the goal distance the robot must be, or 0 for previously set default

  @return true if the robot has finished the distance given in a move
  command or if the robot is no longer in a move mode (because its
  now running off of actions, or setVel() or setVel2() was called).

  @sa setMoveDoneDist
  @sa getMoveDoneDist
 **/
MVREXPORT bool MvrRobot::isMoveDone(double delta)
{
  if (fabs(delta) < 0.001)
    delta = myMoveDoneDist;
  if (myTransType != TRANS_DIST && myTransType != TRANS_DIST_NEW)
    return true;		// no distance command operative
  if (myTransDistStart.findDistanceTo(getPose()) <
      fabs(myTransVal) - delta)
    return false;
  return true;
}


/**
  Determines if a setHeading() command is finished, to within a small
  distance.  If delta = 0 (default), the delta distance used is that which was
  set with setHeadingDoneDiff() (you can get that distnace value with
  getHeadingDoneDiff(), the default is 3).

  @param delta how close to the goal distance the robot must be

  @return true if the robot has achieved the heading given in a move
  command or if the robot is no longer in heading mode mode (because
  its now running off of actions, setDHeading(), or setRotVel() was called).
 **/
MVREXPORT bool MvrRobot::isHeadingDone(double delta) const
{
  if (fabs(delta) < 0.001)
    delta = myHeadingDoneDiff;
  if (myRotType != ROT_HEADING)
    return true;		// no heading command operative
  if(fabs(MvrMath::subAngle(getTh(), myRotVal)) > delta)
    return false;
  return true;
}



/**
  Sets the heading of the robot, it caches this value, and sends it
  during the next cycle.  The heading value is rounded to the nearest
  whole degree before sending to the robot.

  @param heading the desired heading of the robot (degrees)
 **/
MVREXPORT void MvrRobot::setHeading(double heading)
{
  myRotVal = heading;
  myRotType = ROT_HEADING;
  myRotSetTime.setToNow();
  if (myTransType == TRANS_VEL2)
  {
    myTransType = TRANS_IGNORE;
    myTransVal = 0;
    myTransVal2 = 0;
  }
}

/**
  Sets the rotational velocity of the robot, it caches this value,
  and during the next command cycle, the value is rounded to the 
  nearest whole integer degree, and sent to the robot as an RVEL command.

  @param velocity the desired rotational velocity of the robot (deg/sec)
 **/
MVREXPORT void MvrRobot::setRotVel(double velocity)
{
  myRotVal = velocity;
  myRotType = ROT_VEL;
  myRotSetTime.setToNow();
  if (myTransType == TRANS_VEL2)
  {
    myTransType = TRANS_IGNORE;
    myTransVal = 0;
    myTransVal2 = 0;
  }
}

/**
  Sets a delta heading to the robot, it caches this value, and sends
  it during the next cycle.  

  @param deltaHeading the desired amount to change the heading of the robot by
 **/
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

MVREXPORT bool MvrRobot::isStopped(double stoppedVel, double stoppedRotVel,
				 double stoppedLatVel)
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

/// Sets the vels required to be stopped
MVREXPORT void MvrRobot::setStoppedVels(double stoppedVel, double stoppedRotVel,
			     double stoppedLatVel) {
	myStoppedVel = stoppedVel;
	myStoppedRotVel = stoppedRotVel;
	myStoppedLatVel = stoppedLatVel;
}


/**
  This sets the absolute maximum velocity the robot will go... the
  maximum velocity can also be set by the actions and by
  setTransVelMax, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setTransVelMax.

  @param maxVel the maximum velocity to be set, it must be a non-zero
  number and is in mm/sec

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxTransVel(double maxVel)
{
  if (maxVel <= 0)
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
    MvrLog::log(MvrLog::Verbose, "MvrRobot::setAbsoluteMaxTransVel: Setting to %g",
	       maxVel);

  myAbsoluteMaxTransVel = maxVel;
  if (getTransVelMax() > myAbsoluteMaxTransVel)
    setTransVelMax(myAbsoluteMaxTransVel);
  return true;
}

/**
  This sets the absolute maximum velocity the robot will go... the
  maximum velocity can also be set by the actions and by
  setTransVelMax, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setTransVelMax.

  @param maxVel the maximum velocity to be set, it must be a non-zero
  number and is in -mm/sec (use negative values)

  @return true if the value is good, false othrewise
 **/

MVREXPORT bool MvrRobot::setAbsoluteMaxTransNegVel(double maxNegVel)
{
  if (maxNegVel >= 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransNegVel: given a value >= 0 (%g) and will not set it", maxNegVel);
    return false;
  }

  if (myOrigRobotConfig->hasPacketArrived() && 
      maxNegVel < -myOrigRobotConfig->getTransVelTop())
  {
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxTransNegVel: given a value (%g) below TransVelTop (-%g) and will cap it", maxNegVel, myOrigRobotConfig->getTransVelTop());
    maxNegVel = -myOrigRobotConfig->getTransVelTop();
  }

  if (fabs(maxNegVel - myAbsoluteMaxTransNegVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxTransNegVel: Setting to %g",
	       maxNegVel);

  myAbsoluteMaxTransNegVel = maxNegVel;
  if (getTransNegVelMax() < myAbsoluteMaxTransNegVel)
    setTransNegVelMax(myAbsoluteMaxTransNegVel);
  return true;
}

/**
  This sets the absolute maximum translational acceleration the robot
  will do... the acceleration can also be set by the actions and by
  setTransAccel, but it will not be allowed to go higher than this
  value.  You should not set this very often, if you want to
  manipulate this value you should use the actions or setTransAccel.

  @param maxAccel the maximum acceleration to be set, it must be a non-zero
  number 

  @return true if the value is good, false othrewise
 **/

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxTransAccel: Setting to %g",
	       maxAccel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxTransDecel: Setting to %g",
	       maxDecel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxRotVel: Setting to %g",
	       maxVel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxRotAccel: Setting to %g",
	       maxAccel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxRotDecel: Setting to %g",
	       maxDecel);

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
    MvrLog::log(MvrLog::Normal, "MvrRobot::setAbsoluteMaxLatVel: given a value (%g) over LatVelTop (%g) and will cap it", 
	       maxLatVel, myOrigRobotConfig->getLatVelTop());
    maxLatVel = myOrigRobotConfig->getLatVelTop();
  }

  if (fabs(maxLatVel - myAbsoluteMaxLatVel) > MvrMath::epsilon())
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxLatVel: Setting to %g",
	       maxLatVel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxLatAccel: Setting to %g",
	       maxAccel);

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
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::setAbsoluteMaxLatDecel: Setting to %g",
	       maxDecel);

  myAbsoluteMaxLatDecel = maxDecel;
  if (getLatDecel() > myAbsoluteMaxLatDecel)
    setLatDecel(myAbsoluteMaxLatDecel);
  return true;
}

/**
  This gets the raw IO Analog value, which is a number between 0 and
  1024 (2^10).

  @see requestIOPackets()
 **/
MVREXPORT int MvrRobot::getIOAnalog(int num) const
{
  if (num <= getIOAnalogSize())
    return myIOAnalog[num];
  else
    return 0;
}

/**
  This gets the IO Analog value converted to a voltage between 0 and 5 voltes.
  @see requestIOPackets()
 **/
MVREXPORT double MvrRobot::getIOAnalogVoltage(int num) const
{
  if (num <= getIOAnalogSize())
  {
    return (myIOAnalog[num] & 0xfff) * .0048828;
  }
  else
    return 0;
}

/**
  @see requestIOPackets()
 */
MVREXPORT unsigned char MvrRobot::getIODigIn(int num) const
{
  if (num <= getIODigInSize())
    return myIODigIn[num];
  else
    return (unsigned char) 0;
}

/**
  @see requestIOPackets()
 */
MVREXPORT unsigned char  MvrRobot::getIODigOut(int num) const
{
  if (num <= getIODigOutSize())
    return myIODigOut[num];
  else
    return (unsigned char) 0;
}

/** 
  @return the MvrRobotParams instance the robot is using for its parameters
 **/
MVREXPORT const MvrRobotParams *MvrRobot::getRobotParams(void) const
{
  return myParams;
}

/** 
  @return the MvrRobotParams instance the robot is using for its parameters
 **/
MVREXPORT MvrRobotParams *MvrRobot::getRobotParamsInternal(void) 
{
  return myParams;
}

/** 
    @return the MvrRobotConfigPacketReader taken when this instance got
    connected to the robot
**/
MVREXPORT const MvrRobotConfigPacketReader *MvrRobot::getOrigRobotConfig(void) const
{
  return myOrigRobotConfig;
}
  
/**
   Adds a packet handler.  A packet handler is an MvrRetFunctor1<bool, MvrRobotPacket*>, 
   (e.g.  created as an instance of MvrRetFunctor1C.  The return is a boolean, while the functor
   takes an MvrRobotPacket pointer as the argument.  This functor is placed in
   a list of functors to call when a packet arrives. This list is processed
   in order until one of the handlers returns true. Your packet handler
   function may be invoked for any packet, so it should test the packet type
   ID (see MvrRobotPacket::getID()). If you handler gets data from the packet
   (it "handles" it) it should return true, to prevent MvrRobot from invoking
   other handlers with the packet (with data removed). If you hander
   cannot interpret the packet, it should leave it unmodified and return
   false to allow other handlers a chance to receive it.
   @param functor the functor to call when the packet comes in
   @param position whether to place the functor first or last
   @see remPacketHandler
**/
MVREXPORT void MvrRobot::addPacketHandler(
	MvrRetFunctor1<bool, MvrRobotPacket *> *functor, 
	MvrListPos::Pos position) 
{
  if (position == MvrListPos::FIRST)
    myPacketHandlerList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myPacketHandlerList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addPacketHandler: Invalid position.");

}

/**
   @param functor the functor to remove from the list of packet handlers
   @see addPacketHandler
**/
MVREXPORT void MvrRobot::remPacketHandler(
	MvrRetFunctor1<bool, MvrRobotPacket *> *functor)
{
  myPacketHandlerList.remove(functor);
}

/**
   Adds a connect callback, which is an MvrFunctor, (created as an MvrFunctorC).
   The entire list of connect callbacks is called when a connection is made
   with the robot.  If you have some sort of module that adds a callback, 
   that module must remove the callback when the module is removed.
   @param functor A functor (created from MvrFunctorC) which refers to the 
   function to call.
   @param position whether to place the functor first or last
   @see remConnectCB
**/
MVREXPORT void MvrRobot::addConnectCB(MvrFunctor *functor, 
				    MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myConnectCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myConnectCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrRobot::addConnectCallback: Invalid position.");
}

/** 
    @param functor the functor to remove from the list of connect callbacks
    @see addConnectCB
**/
MVREXPORT void MvrRobot::remConnectCB(MvrFunctor *functor)
{
  myConnectCBList.remove(functor);
}


/** Adds a failed connect callback,which is an MvrFunctor, created as an 
    MvrFunctorC.  This whole list of failed connect callbacks is called when
    an attempt is made to connect to the robot, but fails.   The usual reason 
    for this failure is either that there is no robot/sim where the connection
    was tried to be made, the robot wasn't given a connection, or the radio
    modems that communicate with the robot aren't on.  If you have some sort
    of module that adds a callback, that module must remove the callback
    when the module removed.
    @param functor functor created from MvrFunctorC which refers to the 
    function to call.
    @param position whether to place the functor first or last
    @see remFailedConnectCB
**/
MVREXPORT void MvrRobot::addFailedConnectCB(MvrFunctor *functor, 
					  MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myFailedConnectCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myFailedConnectCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrRobot::addFailedCallback: Invalid position.");
}

/** 
    @param functor the functor to remove from the list of connect callbacks
    @see addFailedConnectCB
**/
MVREXPORT void MvrRobot::remFailedConnectCB(MvrFunctor *functor)
{
  myFailedConnectCBList.remove(functor);
}

/** Adds a disconnect normally callback,which is an MvrFunctor, created as an 
    MvrFunctorC.  This whole list of disconnect normally callbacks is called 
    when something calls disconnect if the instance isConnected.  If there is 
    no connection and disconnect is called nothing is done.  If you have some
    sort of module that adds a callback, that module must remove the callback
    when the module is removed.
    @param functor functor created from MvrFunctorC which refers to the 
    function to call.
    @param position whether to place the functor first or last
    @see remFailedConnectCB
**/
MVREXPORT void MvrRobot::addDisconnectNormallyCB(MvrFunctor *functor, 
					       MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myDisconnectNormallyCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myDisconnectNormallyCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrRobot::addDisconnectNormallyCB: Invalid position.");
}

/** 
    @param functor the functor to remove from the list of connect callbacks
    @see addDisconnectNormallyCB
**/
MVREXPORT void MvrRobot::remDisconnectNormallyCB(MvrFunctor *functor)
{
  myDisconnectNormallyCBList.remove(functor);
}

/** Adds a disconnect on error callback, which is an MvrFunctor, created as an 
    MvrFunctorC.  This whole list of disconnect on error callbacks is called 
    when ARIA loses connection to a robot because of an error.  This can occur
    if the physical connection (ie serial cable) between the robot and the 
    computer is severed/disconnected, if one of a pair of radio modems that 
    connect the robot and computer are disconnected, if someone presses the
    reset button on the robot, or if the simulator is closed while ARIA
    is connected to it.  Note that if the link between the two is lost the 
    ARIA assumes it is temporary until it reaches a timeout value set with
    setConnectionTimeoutTime.  If you have some sort of module that adds a 
    callback, that module must remove the callback when the module removed.
    @param functor functor created from MvrFunctorC which refers to the 
    function to call.
    @param position whether to place the functor first or last
    @see remDisconnectOnErrorCB
**/
MVREXPORT void MvrRobot::addDisconnectOnErrorCB(MvrFunctor *functor, 
					      MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myDisconnectOnErrorCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myDisconnectOnErrorCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrRobot::addDisconnectOnErrorCB: Invalid position");
}

/** 
    @param functor the functor to remove from the list of connect callbacks
    @see addDisconnectOnErrorCB
**/
MVREXPORT void MvrRobot::remDisconnectOnErrorCB(MvrFunctor *functor)
{
  myDisconnectOnErrorCBList.remove(functor);
}

/**
   Adds a callback that is called when the run loop exits. The functor is
   which is an MvrFunctor, created as an MvrFunctorC. The whole list of
   functors is called when the run loop exits. This is most usefull for
   threaded programs that run the robot using MvrRobot::runAsync. This will
   allow user threads to know when the robot loop has exited.
   @param functor functor created from MvrFunctorC which refers to the 
   function to call.
   @param position whether to place the functor first or last
   @see remRunExitCB
**/   
MVREXPORT void MvrRobot::addRunExitCB(MvrFunctor *functor,
				    MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myRunExitCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myRunExitCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, "MvrRobot::addRunExitCB: Invalid position");
}

/** 
    @param functor  The functor to remove from the list of run exit callbacks
    @see addRunExitCB
**/
MVREXPORT void MvrRobot::remRunExitCB(MvrFunctor *functor)
{
  myRunExitCBList.remove(functor);
}

/**
   Adds a stablizing callback, which is an MvrFunctor, created as an
   MvrFunctorC.  The entire list of connect callbacks is called just
   before the connection is called done to the robot.  This time can
   be used to calibtrate readings (on things like gyros).
   
   @param functor The functor to call (e.g. MvrFunctorC)

   @param position whether to place the functor first or last

   @see remConnectCB
**/
MVREXPORT void MvrRobot::addStabilizingCB(MvrFunctor *functor, 
				       MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myStabilizingCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myStabilizingCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrRobot::addConnectCallback: Invalid position.");
}

/** 
    @param functor the functor to remove from the list of stabilizing callbacks
    @see addConnectCB
**/
MVREXPORT void MvrRobot::remStabilizingCB(MvrFunctor *functor)
{
  myStabilizingCBList.remove(functor);
}


MVREXPORT std::list<MvrFunctor *> * MvrRobot::getRunExitListCopy()
{
  return(new std::list<MvrFunctor *>(myRunExitCBList));
}

/**
   This will suspend the calling thread until the MvrRobot's run loop has
   managed to connect with the robot. There is an optional paramater of
   milliseconds to wait for the MvrRobot to connect. If msecs is set to 0,
   it will wait until the MvrRobot connects. This function will never
   return if the robot can not be connected with. If you want to be able
   to handle that case within the calling thread, you must call
   waitForConnectOrConnFail().
   @param msecs milliseconds in which to wait for the MvrRobot to connect
   @return WAIT_CONNECTED for success
   @see waitForConnectOrConnFail
   @see wakeAllWaitingThreads
   @see wakeAllConnWaitingThreads
   @see wakeAllRunExitWaitingThreads
**/
MVREXPORT MvrRobot::WaitState MvrRobot::waitForConnect(unsigned int msecs)
{
  int ret;

  if (isConnected())
    return(WAIT_CONNECTED);

  if (msecs == 0)
    ret=myConnectCond.wait();
  else
    ret=myConnectCond.timedWait(msecs);

  if (ret == MvrCondition::STATUS_WAIT_INTR)
    return(WAIT_INTR);
  else if (ret == MvrCondition::STATUS_WAIT_TIMEDOUT)
    return(WAIT_TIMEDOUT);
  else if (ret == 0)
    return(WAIT_CONNECTED);
  else
    return(WAIT_FAIL);
}

/**
   This will suspend the calling thread until the MvrRobot's run loop has
   managed to connect with the robot or fails to connect with the robot.
   There is an optional paramater of milliseconds to wait for the MvrRobot
   to connect. If msecs is set to 0, it will wait until the MvrRobot connects.
   @param msecs milliseconds in which to wait for the MvrRobot to connect
   @return WAIT_CONNECTED for success
   @see waitForConnect
**/
MVREXPORT MvrRobot::WaitState
MvrRobot::waitForConnectOrConnFail(unsigned int msecs)
{
  int ret;

  if (isConnected())
    return(WAIT_CONNECTED);

  if (msecs == 0)
    ret=myConnOrFailCond.wait();
  else
    ret=myConnOrFailCond.timedWait(msecs);

  if (ret == MvrCondition::STATUS_WAIT_INTR)
    return(WAIT_INTR);
  else if (ret == MvrCondition::STATUS_WAIT_TIMEDOUT)
    return(WAIT_TIMEDOUT);
  else if (ret == 0)
  {
    if (isConnected())
      return(WAIT_CONNECTED);
    else
      return(WAIT_FAILED_CONN);
  }
  else
    return(WAIT_FAIL);
}

/**
   This will suspend the calling thread until the MvrRobot's run loop has
   exited. There is an optional paramater of milliseconds to wait for the
   MvrRobot run loop to exit . If msecs is set to 0, it will wait until
   the MvrRobot run loop exits.
   @param msecs milliseconds in which to wait for the robot to connect
   @return WAIT_RUN_EXIT for success
**/
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

/**
   This will wake all the threads waiting for various major state changes
   in this particular MvrRobot. This includes all threads waiting for the
   robot to be connected and all threads waiting for the run loop to exit.
   @see wakeAllConnWaitingThreads
   @see wakeAllRunExitWaitingThreads
**/
MVREXPORT void MvrRobot::wakeAllWaitingThreads()
{
  wakeAllConnWaitingThreads();
  wakeAllRunExitWaitingThreads();
}

/**
   This will wake all the threads waiting for the robot to be connected.
   @see wakeAllWaitingThreads
   @see wakeAllRunExitWaitingThreads
**/
MVREXPORT void MvrRobot::wakeAllConnWaitingThreads()
{
  myConnectCond.broadcast();
  myConnOrFailCond.broadcast();
}

/**
   This will wake all the threads waiting for the robot to be connected or
   waiting for the robot to fail to connect.
   @see wakeAllWaitingThreads
   @see wakeAllRunExitWaitingThreads
**/
MVREXPORT void MvrRobot::wakeAllConnOrFailWaitingThreads()
{
  myConnOrFailCond.broadcast();
}

/**
   This will wake all the threads waiting for the run loop to exit.
   @see wakeAllWaitingThreads
   @see wakeAllConnWaitingThreads
**/
MVREXPORT void MvrRobot::wakeAllRunExitWaitingThreads()
{
  myRunExitCond.broadcast();
}

/**
   This gets the root of the synchronous task tree, so that someone can add
   their own new types of tasks, or find out more information about 
   each task... only serious developers should use this.
   @return the root of the sycnhronous task tree
   @see MvrSyncTask
**/
MVREXPORT MvrSyncTask *MvrRobot::getSyncTaskRoot(void)
{
  return mySyncTaskRoot;
}

/**
   The synchronous tasks get called every robot cycle (every 100 ms by 
   default).  

   @warning Not thread safe; if robot thread is running in background (from
    runAsync()), you must lock the MvrRobot object before calling and unlock after
    calling this method.

   @param name the name to give to the task, should be unique

   @param position the place in the list of user tasks to place this
   task, this can be any integer, though by convention 0 to 100 is
   used.  The tasks are called in order of highest number to lowest
   position number.

   @param functor functor created from MvrFunctorC which refers to the 
   function to call.

   @param state Optional pointer to external MvrSyncTask state variable; normally not needed
   and may be NULL or omitted.

   @see remUserTask
**/
MVREXPORT bool MvrRobot::addUserTask(const char *name, int position, 
				      MvrFunctor *functor, 
				      MvrTaskState::State *state)
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

/**
   @see addUserTask
   @see remUserTask(MvrFunctor *functor)
**/
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

/**
   @see addUserTask
   @see remUserTask(std::string name)
**/
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
   The synchronous tasks get called every robot cycle (every 100 ms by 
   default).  
   @warning Not thread safe; if robot thread is running in background (from
    runAsync()), you must lock the MvrRobot object before calling and unlock after
    calling this method.
   @param name the name to give to the task, should be unique
   @param position the place in the list of user tasks to place this 
   task, this can be any integer, though by convention 0 to 100 is used.
   The tasks are called in order of highest number to lowest number.
   @param functor functor created from MvrFunctorC which refers to the 
   function to call.
   @param state Optional pointer to external MvrSyncTask state variable; normally not needed
   and may be NULL or omitted.
   @see remSensorInterpTask
**/
MVREXPORT bool MvrRobot::addSensorInterpTask(const char *name, int position, 
					      MvrFunctor *functor,
					      MvrTaskState::State *state)
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

/**
   @see addSensorInterpTask
   @see remSensorInterpTask(MvrFunctor *functor)
**/
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

/**
   @see addSensorInterpTask
   @see remSensorInterpTask(std::string name)
**/
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

/** 
    @see MvrLog
**/
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

/**
   @see MvrLog
**/
MVREXPORT void MvrRobot::logAllTasks(void) const
{
  if (mySyncTaskRoot != NULL)
    mySyncTaskRoot->log();
}

/**
   Finds a user task by its name, searching the entire space of tasks
   @return NULL if no user task of that name found, otherwise a pointer to 
   the MvrSyncTask for the first task found with that name
**/
MVREXPORT MvrSyncTask *MvrRobot::findUserTask(const char *name)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return NULL;

  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return NULL;

  return proc->find(name);
}

/**
   Finds a user task by its functor, searching the entire space of tasks
   @return NULL if no user task with that functor found, otherwise a pointer
   to the MvrSyncTask for the first task found with that functor
**/
MVREXPORT MvrSyncTask *MvrRobot::findUserTask(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  if (mySyncTaskRoot == NULL)
    return NULL;

  proc = mySyncTaskRoot->findNonRecursive("User Tasks");
  if (proc == NULL)
    return NULL;

  return proc->find(functor);
}

/**
   Finds a task by its name, searching the entire space of tasks
   @return NULL if no task of that name found, otherwise a pointer to the
   MvrSyncTask for the first task found with that name
**/
MVREXPORT MvrSyncTask *MvrRobot::findTask(const char *name)
{
  if (mySyncTaskRoot != NULL)
    return mySyncTaskRoot->find(name);
  else
    return NULL;

}

/**
   Finds a task by its functor, searching the entire space of tasks
   @return NULL if no task with that functor found, otherwise a pointer
   to the MvrSyncTask for the first task found with that functor
**/
MVREXPORT MvrSyncTask *MvrRobot::findTask(MvrFunctor *functor)
{
  if (mySyncTaskRoot != NULL)
    return mySyncTaskRoot->find(functor);
  else
    return NULL;

}

/** 
    Adds an action to the list of actions with the given priority. In
    the case of two (or more) actions with the same priority, the
    default resolver (MvrPriorityResolver) averages the the multiple
    readings. The priority can be any integer, but as a convention 0
    to 100 is used, with 100 being the highest priority. The default
    resolver (MvrPriorityResolver) resolves the actions in order of descending
    priority. For example, an action with priority 100 is evaluated before 
    one with priority 99, followed by 50, etc.  This means that an action with 
    a higher priority may be able to supercede a lower-priority action's
    desired value for a certain output to a lesser or greater degree, depending 
    on how high a "strength" value it sets.  See the overview of ARIA in this 
    reference manual for more discussion on Actions.

    @param action the action to add 
    @param priority what importance to give the action; how to order the actions.  High priority actions are evaluated by the action resolvel before lower priority actions.
    @return true if the action was successfully added, false on error (e.g. the action was NULL)
    @sa remAction(MvrAction*)
    @sa remAction(const char*)
    @sa findAction(const char*)
*/
MVREXPORT bool MvrRobot::addAction(MvrAction *action, int priority)
{
  if (action == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
      "MvrRobot::addAction: an attempt was made to add a NULL action pointer");
    return false;
  }
  
  action->setRobot(this);
  myActions.insert(std::pair<int, MvrAction *>(priority, action));
  return true;
}

/**
   Finds the action with the given name and removes it from the actions... 
   if more than one action has that name it find the one with the lowest
   priority
   @param actionName the name of the action we want to find
   @return whether remAction found anything with that action to remove or not
   @sa addAction()
   @sa remAction(MvrAction*)
   @sa findAction(const char*)
**/
MVREXPORT bool MvrRobot::remAction(const char *actionName)
{
  MvrResolver::ActionMap::iterator it;
  MvrAction *act;

  for (it = myActions.begin(); it != myActions.end(); ++it)
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

/**
   Finds the action with the given pointer and removes it from the actions... 
   if more than one action has that pointer it find the one with the lowest
   priority
   @param action the action we want to remove
   @return whether remAction found anything with that action to remove or not
   *  @sa addAction
   *  @sa remAction(const char*)
   *  @sa findAction(const char*)
**/
MVREXPORT bool MvrRobot::remAction(MvrAction *action)
{
  MvrResolver::ActionMap::iterator it;
  MvrAction *act;

  for (it = myActions.begin(); it != myActions.end(); ++it)
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


/**
   Finds the action with the given name... if more than one action
   has that name it find the one with the highest priority
   @param actionName the name of the action we want to find
   @return the action, if found.  If not found, NULL
   @sa addAction
   @sa remAction(MvrAction*)
   @sa remAction(const char*)
**/
MVREXPORT MvrAction *MvrRobot::findAction(const char *actionName)
{
  MvrResolver::ActionMap::reverse_iterator it;
  MvrAction *act;
  
  for (it = myActions.rbegin(); it != myActions.rend(); ++it)
  {
    act = (*it).second;
    if (strcmp(actionName, act->getName()) == 0)
      return act;
  }
  return NULL;
}

/**
 * @internal
   This returns the actionMap the robot has... do not mess with this
   list except by using MvrRobot::addAction() and MvrRobot::remAction()...
   This is jsut for the things like MvrActionGroup that want to
   deactivate or activate all the actions (well, only deactivating
   everything makes sense).
   @return the actions the robot is using
**/

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
    MvrLog::log(MvrLog::Terse, 
            "MvrRobot::deactivateActions: NULL action map... failed.");
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
    MvrLog::log(MvrLog::Terse, "The action list (%d total):", 
	       myActions.size());
  else
    MvrLog::log(MvrLog::Terse, "The active action list:");

  for (it = myActions.rbegin(); it != myActions.rend(); ++it)
  {
    action = (*it).second;
    if ((logDeactivated || action->isActive()) &&
	(first || lastPriority != (*it).first))
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
    delete myResolver;
    myResolver = NULL;
  }

  myResolver = resolver;
}

/**
 * @internal
 *
   If state reflecting (really direct motion command reflecting) was
   enabled in the constructor (MvrRobot::MvrRobot) then this will see if
   there are any direct motion commands to send, and if not then send
   the command given by the actions.  If state reflection is disabled
   this will send a pulse to the robot every state reflection refresh
   time (setStateReflectionRefreshTime), if you don't wish this to happen
   simply set this to a very large value.
**/
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

    if (hasSettableVelMaxes() && 
	MvrMath::fabs(myLastSentTransVelMax - myTransVelMax) >= 1)
    {
      comInt(MvrCommands::SETV,
	     MvrMath::roundShort(myTransVelMax));
      myLastSentTransVelMax = myTransVelMax;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Non-action trans max vel of %d", 
		   MvrMath::roundShort(myTransVelMax));
    }

    if (hasSettableAccsDecs() && MvrMath::fabs(myTransAccel) > 1 && 
	MvrMath::fabs(myLastSentTransAccel - myTransAccel) >= 1)
    {
      comInt(MvrCommands::SETA,
	     MvrMath::roundShort(myTransAccel));
      myLastSentTransAccel = myTransAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Non-action trans accel of %d", 
		   MvrMath::roundShort(myTransAccel));
    }

    if (hasSettableAccsDecs() && MvrMath::fabs(myTransDecel) > 1 &&
	MvrMath::fabs(myLastSentTransDecel - myTransDecel) >= 1)
    {
      comInt(MvrCommands::SETA,
	     -MvrMath::roundShort(myTransDecel));
      myLastSentTransDecel = myTransDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Non-action trans decel of %d", 
		   -MvrMath::roundShort(myTransDecel));
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
	  MvrLog::log(MvrLog::Normal, "Non-action trans vel of %d", 
		     MvrMath::roundShort(transVal));
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
	  myLastTransType != myTransType ||
	  (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime))
      {
	com2Bytes(MvrCommands::VEL2, transVal, transVal2);
	myLastTransSent.setToNow();
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Non-action vel2 of %d %d", 
		     transVal, transVal2);
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
	if (distGone > fabs(myTransVal) || 
	    (distToGo < 10 && fabs(getVel()) < 30))
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
	  MvrLog::log(MvrLog::Normal, "Non-action move-helper of %d", 
		     MvrMath::roundShort(vel));
      }
      else if (myParams->hasMoveCommand() && myTransType == TRANS_DIST_NEW) 
      {
	comInt(MvrCommands::MOVE, transVal);
	myLastTransSent.setToNow();
	myTransType = TRANS_DIST;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Non-action move of %d", 
		     transVal);
	myTryingToMove = true;
      }
      else if (myTransType == TRANS_DIST && 
	  (myLastTransSent.mSecSince() >= myStateReflectionRefreshTime))
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
      MvrLog::log(MvrLog::Terse, 
		 "MvrRobot::stateReflector: Invalid translational type %d.",
		 myTransType);
    myLastTransVal = transVal;
    myLastTransVal2 = transVal2;
    myLastTransType = myTransType;
  }
  else // if actions can go
  {
    if (hasSettableVelMaxes() && 
	MvrMath::fabs(myLastSentTransVelMax - myTransVelMax) >= 1)
    {
      comInt(MvrCommands::SETV,
	     MvrMath::roundShort(myTransVelMax));
      myLastSentTransVelMax = myTransVelMax;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Action-but-robot trans max vel of %d", 
		   MvrMath::roundShort(myTransVelMax));
    }
    
    // first we'll handle all of the accel decel things
    if (myActionDesired.getTransAccelStrength() >= 
	MvrActionDesired::MIN_STRENGTH)
    {
      transAccel = MvrMath::roundShort(myActionDesired.getTransAccel());
      if (hasSettableAccsDecs() && MvrMath::fabs(transAccel) > 1 &&
	  MvrMath::fabs(myLastSentTransAccel - transAccel) >= 1)
      {
	comInt(MvrCommands::SETA,
	       MvrMath::roundShort(transAccel));
	myLastSentTransAccel = transAccel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Action trans accel of %d", 
		     MvrMath::roundShort(transAccel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myTransAccel) > 1 && 
	MvrMath::fabs(myLastSentTransAccel - myTransAccel) >= 1)
    {
      comInt(MvrCommands::SETA,
	     MvrMath::roundShort(myTransAccel));
      myLastSentTransAccel = myTransAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Action-but-robot trans accel of %d", 
		   MvrMath::roundShort(myTransAccel));
    }

    if (myActionDesired.getTransDecelStrength() >=
	MvrActionDesired::MIN_STRENGTH)
    {
      transDecel = MvrMath::roundShort(myActionDesired.getTransDecel());
      if (hasSettableAccsDecs() && MvrMath::fabs(transDecel) > 1 &&
	  MvrMath::fabs(myLastSentTransDecel - transDecel) >= 1)
      {
	comInt(MvrCommands::SETA,
	       -MvrMath::roundShort(transDecel));
	myLastSentTransDecel = transDecel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Action trans decel of %d", 
		     -MvrMath::roundShort(transDecel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myTransDecel) > 1 &&
	MvrMath::fabs(myLastSentTransDecel - myTransDecel) >= 1)
    {
      comInt(MvrCommands::SETA,
	     -MvrMath::roundShort(myTransDecel));
      myLastSentTransDecel = myTransDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Action-but-robot trans decel of %d", 
		   -MvrMath::roundShort(myTransDecel));
    }

    if (myActionDesired.getMaxVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      maxTransVel = myActionDesired.getMaxVel();
      if (maxTransVel > myTransVelMax)
	maxTransVel = myTransVelMax;
    }
    else
      maxTransVel = myTransVelMax;

    if (myActionDesired.getMaxNegVelStrength() >= 
	MvrActionDesired::MIN_STRENGTH)
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

    if (myActionTransSet && 
	(myLastTransSent.mSecSince() >= myStateReflectionRefreshTime ||
	 transVal != myLastActionTransVal))			     
    {
      comInt(MvrCommands::VEL, MvrMath::roundShort(transVal));
      myLastTransSent.setToNow();
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "Action trans vel of %d", 
		   MvrMath::roundShort(transVal));      
    }
    myLastActionTransVal = transVal;
  }

  // if this is true actions can't go
  if ((myRotType != ROT_NONE && myDirectPrecedenceTime == 0) ||
      (myRotType != ROT_NONE && myDirectPrecedenceTime != 0 && 
       myRotSetTime.mSecSince() < myDirectPrecedenceTime))
  {
    if (hasSettableVelMaxes() && 
	MvrMath::fabs(myLastSentRotVelMax - myRotVelMax) >= 1)
    {
      //comInt(MvrCommands::SETRVDIR, 0);
      comInt(MvrCommands::SETRV,
	     MvrMath::roundShort(myRotVelMax));

      myLastSentRotVelMax = myRotVelMax;
      myLastSentRotVelPosMax = -1;
      myLastSentRotVelNegMax = -1;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%25sNon-action rot vel max of %d", "",
		   MvrMath::roundShort(myRotVelMax));      

    }
    if (hasSettableAccsDecs() && MvrMath::fabs(myRotAccel) > 1 &&
	MvrMath::fabs(myLastSentRotAccel - myRotAccel) >= 1)
    {
      comInt(MvrCommands::SETRA,
	     MvrMath::roundShort(myRotAccel));
      myLastSentRotAccel = myRotAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%25sNon-action rot accel of %d", "",
		   MvrMath::roundShort(myRotAccel));      
    }
    if (hasSettableAccsDecs() && MvrMath::fabs(myRotDecel) > 1 &&
	MvrMath::fabs(myLastSentRotDecel - myRotDecel) >= 1)
    {
      comInt(MvrCommands::SETRA,
	     -MvrMath::roundShort(myRotDecel));
      myLastSentRotDecel = myRotDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%25sNon-action rot decel of %d", "",
		   -MvrMath::roundShort(myRotDecel));      
    }

    myActionRotSet = false;
    rotVal = MvrMath::roundShort(myRotVal);
    if (myRotType == ROT_HEADING)
    {
      encTh = MvrMath::subAngle(myRotVal, myEncoderTransform.getTh());
      rawTh = MvrMath::addAngle(encTh, 
			       MvrMath::subAngle(myRawEncoderPose.getTh(),
						myEncoderPose.getTh()));
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
	  MvrLog::log(MvrLog::Normal, "%25sNon-action rot vel of %d", "",
		     rotVal);      
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
      MvrLog::log(MvrLog::Terse, 
		 "MvrRobot::stateReflector: Invalid rotation type %d.",
		 myRotType);
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
	myLastSentRotVelMax = maxRotVel;
	myLastSentRotVelPosMax = -1;
	myLastSentRotVelNegMax = -1;
	//comInt(MvrCommands::SETRVDIR, 0);
	comInt(MvrCommands::SETRV, 
	       MvrMath::roundShort(maxRotVel));
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%25sAction rot vel max of %d", "",
		     MvrMath::roundShort(maxRotVel));
      }
    }
    // if a max pos or neg rot vel is set then use that
    else if (myActionDesired.getMaxRotVelPosStrength() >= 
	  MvrActionDesired::MIN_STRENGTH ||
	  myActionDesired.getMaxRotVelNegStrength() >=
	  MvrActionDesired::MIN_STRENGTH)
    {
      if (myActionDesired.getMaxRotVelStrength() >=
	  MvrActionDesired::MIN_STRENGTH)
	maxRotVel = myActionDesired.getMaxRotVel();
      else
	maxRotVel = myRotVelMax;
      
      if (maxRotVel > myAbsoluteMaxRotVel)
	maxRotVel = myAbsoluteMaxRotVel;

      if (myActionDesired.getMaxRotVelPosStrength() >= 
	  MvrActionDesired::MIN_STRENGTH)
	maxRotVelPos = MvrUtil::findMin(maxRotVel,
				       myActionDesired.getMaxRotVelPos());
      else
	maxRotVelPos = maxRotVel;

      // 1 here actually means 0 (since there's no -0 and its not
      // worth two commands)
      if (maxRotVelPos < .5)
	maxRotVelPos = 1;

      if (myActionDesired.getMaxRotVelNegStrength() >= 
	  MvrActionDesired::MIN_STRENGTH)
	maxRotVelNeg = MvrUtil::findMin(maxRotVel,
				       myActionDesired.getMaxRotVelNeg());
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
	comInt(MvrCommands::SETRVDIR, 
	       MvrMath::roundShort(maxRotVelPos));
	comInt(MvrCommands::SETRVDIR, 
	       MvrMath::roundShort(-maxRotVelNeg));
	if (myLogMovementSent)
	{
	  MvrLog::log(MvrLog::Normal, "%25sAction rot vel pos max of %d", "",
		     MvrMath::roundShort(maxRotVelPos));
	  MvrLog::log(MvrLog::Normal, "%25sAction rot vel neg max of %d", "",
		     MvrMath::roundShort(-maxRotVelNeg));
	}
      }
    }
    else if (hasSettableVelMaxes() && 
	     MvrMath::fabs(myLastSentRotVelMax - myRotVelMax) >= 1)
    {
      //comInt(MvrCommands::SETRVDIR, 0);
      comInt(MvrCommands::SETRV,
	     MvrMath::roundShort(myRotVelMax));
      myLastSentRotVelMax = myRotVelMax;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, 
		   "%25sAction-but-robot rot vel max of %d", 
		   "",  MvrMath::roundShort(myRotVelMax));      
    }

    if (myActionDesired.getRotAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      rotAccel = MvrMath::roundShort(myActionDesired.getRotAccel());
      if (MvrMath::fabs(myLastSentRotAccel - rotAccel) >= 1)
      {
	comInt(MvrCommands::SETRA,
	       MvrMath::roundShort(rotAccel));
	myLastSentRotAccel = rotAccel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%25sAction rot accel of %d", "",
		     MvrMath::roundShort(rotAccel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myRotAccel) > 1 &&
	MvrMath::fabs(myLastSentRotAccel - myRotAccel) >= 1)
    {
      comInt(MvrCommands::SETRA,
	     MvrMath::roundShort(myRotAccel));
      myLastSentRotAccel = myRotAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%25sAction-but-robot rot accel of %d", 
		   "", MvrMath::roundShort(myRotAccel));      
    }

    if (myActionDesired.getRotDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    {
      rotDecel = MvrMath::roundShort(myActionDesired.getRotDecel());
      if (MvrMath::fabs(myLastSentRotDecel - rotDecel) >= 1)
      {
	comInt(MvrCommands::SETRA,
	       -MvrMath::roundShort(rotDecel));
	myLastSentRotDecel = rotDecel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%25sAction rot decel of %d", "",
		     -MvrMath::roundShort(rotDecel));
      }
    }
    else if (hasSettableAccsDecs() && MvrMath::fabs(myRotDecel) > 1 &&
	MvrMath::fabs(myLastSentRotDecel - myRotDecel) >= 1)
    {
      comInt(MvrCommands::SETRA,
	     -MvrMath::roundShort(myRotDecel));
      myLastSentRotDecel = myRotDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%25sAction-but-robot rot decel of %d", 
		   "", -MvrMath::roundShort(myRotDecel));      
    }



    if (myActionDesired.getDeltaHeadingStrength() >=
	MvrActionDesired::MIN_STRENGTH)
    {
      if (MvrMath::roundShort(myActionDesired.getDeltaHeading()) == 0)
      {
	rotStopped = true;
	rotVal = 0;
	rotHeading = false;
      }
      else
      {
	//printf("delta %.0f\n", myActionDesired.getDeltaHeading());
	//encTh = MvrMath::subAngle(myRotVal, myEncoderTransform.getTh());
	encTh = MvrMath::subAngle(
		MvrMath::addAngle(myActionDesired.getDeltaHeading(), 
				 getTh()),
		myEncoderTransform.getTh());
	//printf("final th %.0f\n", th);
	rawTh = MvrMath::addAngle(encTh, 
				 MvrMath::subAngle(myRawEncoderPose.getTh(),
						  myEncoderPose.getTh()));
	rotVal = MvrMath::roundShort(rawTh);
	rotStopped = false;
	rotHeading = true;
	myTryingToMove = true;
      }
      myActionRotSet = true;
    } 
    else if (myActionDesired.getRotVelStrength() >=
	                                     MvrActionDesired::MIN_STRENGTH)
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

    if (myActionRotSet && 
	(myLastRotSent.mSecSince() > myStateReflectionRefreshTime ||
	 rotStopped != myLastActionRotStopped || 
	 rotVal != myLastActionRotVal || 
	 rotHeading != myLastActionRotHeading))
    {
      if (rotStopped)
      {
	comInt(MvrCommands::RVEL, 0);
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, 
		     "%25sAction rot vel of 0 (rotStopped)",
		     "");
      }
      else if (rotHeading)
      {
	comInt(MvrCommands::HEAD, rotVal);
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, 
		     "%25sAction rot heading of %d (encoder %d, raw %d)",
		     "",
		     MvrMath::roundShort(MvrMath::addAngle(
			     myActionDesired.getDeltaHeading(), 
			     getTh())),
		     MvrMath::roundShort(encTh),
		     MvrMath::roundShort(rotVal));
      }
      else
      {
	comInt(MvrCommands::RVEL, rotVal);
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%25sAction rot vel of %d", "", 
		     rotVal);
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
      comInt(MvrCommands::SETLATV,
	     MvrMath::roundShort(myLatVelMax));
      myLastSentLatVelMax = myLatVelMax;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sNon-action lat max vel of %d", "",
		   MvrMath::roundShort(myLatVelMax));
    }

    if (MvrMath::fabs(myLatAccel) > 1 && 
	MvrMath::fabs(myLastSentLatAccel - myLatAccel) >= 1)
    {
      comInt(MvrCommands::LATACCEL,
	     MvrMath::roundShort(myLatAccel));
      myLastSentLatAccel = myLatAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sNon-action lat accel of %d", "",
		   MvrMath::roundShort(myLatAccel));
    }

    if (MvrMath::fabs(myLatDecel) > 1 && 
	MvrMath::fabs(myLastSentLatDecel - myLatDecel) >= 1)
    {
      comInt(MvrCommands::LATACCEL,
	     -MvrMath::roundShort(myLatDecel));
      myLastSentLatDecel = myLatDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sNon-action lat decel of %d", "",
		   -MvrMath::roundShort(myLatDecel));
    }

    if (myLatType == LAT_VEL)
    {
      maxLatVel = MvrMath::roundShort(myLatVelMax);
      if (latVal > maxLatVel)
	latVal = maxLatVel;
      if (latVal < -maxLatVel)
	latVal = -maxLatVel;
      if (myLastLatVal != latVal || myLastLatType != myLatType ||
	  (myLastLatSent.mSecSince() >= myStateReflectionRefreshTime))
      {
	comInt(MvrCommands::LATVEL, MvrMath::roundShort(latVal));
	myLastLatSent.setToNow();
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%12sNon-action lat vel of %d", "",
		     MvrMath::roundShort(latVal));
	//printf("Sent command vel!\n");
      }
      if (fabs((double)latVal) > (double).5)
	myTryingToMove = true;
    }
    /*
    else if (myLatType == LAT_DIST_NEW || myLatType == LAT_DIST)
    {
      // if the robot doesn't have its own distance command
      if (!myParams->hasMoveCommand())
      {
	double distGone;
	double distToGo;
	double vel;

	myLatType = LAT_DIST;
	distGone = myLatDistStart.findDistanceTo(getPose());
	distToGo = fabs(fabs(myLatVal) - distGone);
	if (distGone > fabs(myLatVal) || 
	    (distToGo < 10 && fabs(getVel()) < 30))
	{
	  comInt(MvrCommands::VEL, 0);
	  myLatType = LAT_VEL;
	  myLatVal = 0;
	}
	else
	  myTryingToMove = true;
	vel = sqrt(distToGo * 200 * 2);
	if (vel > getLatVelMax())
	  vel = getLatVelMax();
	if (myLatVal < 0)
	  vel *= -1;
	comInt(MvrCommands::VEL, MvrMath::roundShort(vel));
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Non-action move-helper of %d", 
		     MvrMath::roundShort(vel));
      }
      else if (myParams->hasMoveCommand() && myLatType == LAT_DIST_NEW) 
      {
	comInt(MvrCommands::MOVE, latVal);
	myLastLatSent.setToNow();
	myLatType = LAT_DIST;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Non-action move of %d", 
		     latVal);
	myTryingToMove = true;
      }
      else if (myLatType == LAT_DIST && 
	  (myLastLatSent.mSecSince() >= myStateReflectionRefreshTime))
      {
	com(0);
	myLastPulseSent.setToNow();
	myLastLatSent.setToNow();
	//printf("Sent pulse for dist!\n");
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "Non-action pulse for dist");
      }
      //printf("Sent command move!\n");
    }
    */
    else if (myLatType == LAT_IGNORE)
    {
      //printf("No lat command sent\n");
    }
    else
      MvrLog::log(MvrLog::Terse, 
		 "MvrRobot::stateReflector: Invalid lat type %d.",
		 myLatType);

    myLastLatVal = latVal;
    myLastLatType = myLatType;
  }
  else // if actions can go
  {
    if (MvrMath::fabs(myLastSentLatVelMax - myLatVelMax) >= 1)
    {
      comInt(MvrCommands::SETLATV,
	     MvrMath::roundShort(myLatVelMax));
      myLastSentLatVelMax = myLatVelMax;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat max vel of %d", "",
		   MvrMath::roundShort(myLatVelMax));
    }
    
    // first we'll handle all of the accel decel things
    if (myActionDesired.getLatAccelStrength() >= 
	MvrActionDesired::MIN_STRENGTH)
    {
      latAccel = MvrMath::roundShort(myActionDesired.getLatAccel());
      if (MvrMath::fabs(latAccel) > 1 &&
	  MvrMath::fabs(myLastSentLatAccel - latAccel) >= 1)
      {
	comInt(MvrCommands::LATACCEL,
	       MvrMath::roundShort(latAccel));
	myLastSentLatAccel = latAccel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%12sAction lat accel of %d", "",
		     MvrMath::roundShort(latAccel));
      }
    }
    else if (MvrMath::fabs(myLatAccel) > 1 && 
	MvrMath::fabs(myLastSentLatAccel - myLatAccel) >= 1)
    {
      comInt(MvrCommands::LATACCEL,
	     MvrMath::roundShort(myLatAccel));
      myLastSentLatAccel = myLatAccel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat accel of %d", "",
		   MvrMath::roundShort(myLatAccel));
    }

    if (myActionDesired.getLatDecelStrength() >=
	MvrActionDesired::MIN_STRENGTH)
    {
      latDecel = MvrMath::roundShort(myActionDesired.getLatDecel());
      if (MvrMath::fabs(latDecel) > 1 &&
	  MvrMath::fabs(myLastSentLatDecel - latDecel) >= 1)
      {
	comInt(MvrCommands::LATACCEL,
	       -MvrMath::roundShort(latDecel));
	myLastSentLatDecel = latDecel;
	if (myLogMovementSent)
	  MvrLog::log(MvrLog::Normal, "%12sAction lat decel of %d", "",
		     -MvrMath::roundShort(latDecel));
      }
    }
    else if (MvrMath::fabs(myLatDecel) > 1 &&
	MvrMath::fabs(myLastSentLatDecel - myLatDecel) >= 1)
    {
      comInt(MvrCommands::LATACCEL,
	     -MvrMath::roundShort(myLatDecel));
      myLastSentLatDecel = myLatDecel;
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sAction-but-robot lat decel of %d", "",
		   -MvrMath::roundShort(myLatDecel));
    }

    if (myActionDesired.getMaxLeftLatVelStrength() >= 
	MvrActionDesired::MIN_STRENGTH)
    {
      maxLeftLatVel = myActionDesired.getMaxLeftLatVel();
      if (maxLeftLatVel > myLatVelMax)
	maxLeftLatVel = myLatVelMax;
    }
    else
      maxLeftLatVel = myLatVelMax;

    if (myActionDesired.getMaxRightLatVelStrength() >= 
	MvrActionDesired::MIN_STRENGTH)
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

    if (myActionLatSet && 
	(myLastLatSent.mSecSince() >= myStateReflectionRefreshTime ||
	 latVal != myLastActionLatVal))			     
    {
      comInt(MvrCommands::LATVEL, MvrMath::roundShort(latVal));
      myLastLatSent.setToNow();
      if (myLogMovementSent)
	MvrLog::log(MvrLog::Normal, "%12sAction lat vel of %d", "",
		   MvrMath::roundShort(latVal));      
    }
    myLastActionLatVal = latVal;
  }
  // end lat

  if (myLastRotSent.mSecSince() > myStateReflectionRefreshTime &&
      myLastTransSent.mSecSince() > myStateReflectionRefreshTime &&
      myLastPulseSent.mSecSince() > myStateReflectionRefreshTime && 
      (!hasLatVel() || 
       (hasLatVel() &&       
	myLastLatSent.mSecSince() > myStateReflectionRefreshTime)))
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
      MvrLog::log(MvrLog::Normal, "MvrRobot: Ignoring motor packet of type 0x%x",
		 packet->getID());
      myIgnoreNextPacket = false;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, "MvrRobot: Ignoring packet of type 0x%x",		 packet->getID());
    }
    unlock();
    return false;
  }

  myConnectionTimeoutMutex.lock();
  //printf("ms since last packet %ld this type 0x%x\n", myLastPacketReceivedTime.mSecSince(packet->getTimeReceived()), packet->getID());
  myLastPacketReceivedTime = packet->getTimeReceived();
  myConnectionTimeoutMutex.unlock();

  if (packet->getID() == 0xff) 
  {
    dropConnection("Losing connection because microcontroller reset.",
		   "because microcontroller reset");
    unlock();
    return false;
  }

  if (packet->getID() == 0xfe) 
  {
    char buf[100000];
    sprintf(buf, "Losing connection because microcontroller reset with reset data");

    while (packet->getDataLength() - packet->getDataReadLength() > 0)
      sprintf(buf, "%s 0x%x", buf, packet->bufToUByte());

    dropConnection(buf, "because microcontroller reset");
    unlock();
    return false;
  }

  for (handled = false, it = myPacketHandlerList.begin(); 
       it != myPacketHandlerList.end() && handled == false; 
       it++)
  {
    if ((*it) != NULL && (*it)->invokeR(packet)) 
    {
      if (myPacketsReceivedTracking)
	MvrLog::log(MvrLog::Normal, "Handled by %s",
		   (*it)->getName());
      handled = true;
    }
    else
    {
      packet->resetRead();
    }
  }
  if (!handled)
    MvrLog::log(MvrLog::Normal, 
	       "No packet handler wanted packet with ID: 0x%x", 
	       packet->getID());
  unlock();
  return handled;
}


/** @note You must first start the encoder packet stream by calling
 * requestEncoderPackets() before this function will return encoder values.
 */
MVREXPORT long int MvrRobot::getLeftEncoder()
{
  return myLeftEncoder;
}

/** @note You must first start the encoder packet stream by calling
 * requestEncoderPackets() before this function will return encoder values.
 */
MVREXPORT long int MvrRobot::getRightEncoder()
{
  return myRightEncoder;
}


/**
 * @internal
   This just locks the robot, so that its locked for all the user tasks
**/
MVREXPORT void MvrRobot::robotLocker(void)
{
  lock();
}

/**
 * @internal
   This just unlocks the robot
**/
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


/**
   This is here for use if the robot is running without threading
   (some customers may use it that way, though we generally don't)

   Reads in all of the packets that are available to read in, then runs through
   the list of packet handlers and tries to get each packet handled.
   @see addPacketHandler
   @see remPacketHandler
**/
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
    The basic idea is that if we're chained to the sip we run through
    and see if we have any packets available now (like if we got
    backed up), we only check this for half the cycle time
    though... if we know the cycle time of the robot (from config)
    then we go for half that, if we don't know the cycle time of the
    robot (from config) then we go for half of whatever our cycle time
    is set to

    if we don't have any packets waiting then we chill and wait for
    it, if we got one, just get on with it
  **/
  packet = NULL;
  // read all the packets that are available
  while ((packet = myReceiver.receivePacket(0)) != NULL)
  {
    if (myPacketsReceivedTracking)
    {
      MvrLog::log(MvrLog::Normal, 
		 "Rcvd: prePacket (%ld) 0x%x at %ld (%ld)", 
		 myPacketsReceivedTrackingCount, 
		 packet->getID(), start.mSecSince(), 
		 myPacketsReceivedTrackingStarted.mSecSince());
      myPacketsReceivedTrackingCount++;
    }

    handlePacket(packet);
    if ((packet->getID() & 0xf0) == 0x30)
      sipHandled = true;
    packet = NULL;

    // if we've taken too long here then break
    if ((getOrigRobotConfig()->hasPacketArrived() &&
	 start.mSecSince() > getOrigRobotConfig()->getSipCycleTime() / 2) ||
	(!getOrigRobotConfig()->hasPacketArrived() &&
	 (unsigned int) start.mSecSince() > myCycleTime / 2))
    {
      break;
    }
  }

  if (isCycleChained())
    timeToWait = getCycleTime() * 2 - start.mSecSince();
  
  // if we didn't get a sip and we're chained to the sip, wait for the sip
  while (isCycleChained() && !sipHandled && isRunning() && 
	 (packet = myReceiver.receivePacket(timeToWait)) != NULL)
  {
    if (myPacketsReceivedTracking)
    {
      MvrLog::log(MvrLog::Normal, "Rcvd: Packet (%ld) 0x%x at %ld (%ld)", 
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
  if (myTimeoutTime > 0 && 
      ((-myLastOdometryReceivedTime.mSecTo()) > myTimeoutTime))
  {
    char buf[10000];
    sprintf(buf, 
	    "Losing connection because no odometry received from microcontroller in %ld milliseconds (greater than the timeout of %d).", 
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
  if (myTimeoutTime > 0 && 
      ((-myLastPacketReceivedTime.mSecTo()) > myTimeoutTime))
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
  bool anotherSip = false;
  std::list<MvrRobotPacket *>::iterator it;

  if (myAsyncConnectFlag)
  {
    lock();
    asyncConnectHandler(false);
    unlock();
    return;
  }

  if (!isConnected())
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
      // through the list
      for (it = myPacketList.begin(); 
	   !anotherSip && it != myPacketList.end(); 
	   it++)
      {
	if (((*it)->getID() & 0xf0) == 0x30)
	  anotherSip = true;
      }
      myPacketMutex.unlock();
    }
    else
    {
      myPacketMutex.unlock();
    }

    if (packet == NULL)
    {
      if (isCycleChained())
	timeToWait = getCycleTime() * 2 - start.mSecSince();
      else
	timeToWait = getCycleTime() - start.mSecSince();

      int ret = 0;

      if (timeToWait <= 0 ||
	  (ret = myPacketReceivedCondition.timedWait(timeToWait)) != 0)
      {
	if (myCycleWarningTime != 0)
	  MvrLog::log(MvrLog::Normal, "MvrRobot::myPacketReader: Timed out (%d) at %d (%d into cycle after sleeping %d)", 	     
		     ret, myPacketsReceivedTrackingStarted.mSecSince(), 
		     start.mSecSince(), timeToWait);
	break;
      }
      else
      {
	continue;
      }
    }
    
    handlePacket(packet);
    if ((packet->getID() & 0xf0) == 0x30)
    {
      // only mark the sip handled if it was the only one in the buffer
      if (!anotherSip)
	sipHandled = true;
      
      if (myPacketsReceivedTracking)
      {
	MvrLog::log(MvrLog::Normal, "Rcvd: Packet (%ld) 0x%x at %ld (%ld)", 
		   myPacketsReceivedTrackingCount, 
		   packet->getID(), start.mSecSince(), 
		   myPacketsReceivedTrackingStarted.mSecSince());
	myPacketsReceivedTrackingCount++;
      }
    }
    else
    {
      if (myPacketsReceivedTracking)
      {
	MvrLog::log(MvrLog::Normal, 
		   "Rcvd: prePacket (%ld) 0x%x at %ld (%ld)", 
		   myPacketsReceivedTrackingCount, 
		   packet->getID(), start.mSecSince(), 
		   myPacketsReceivedTrackingStarted.mSecSince());
	myPacketsReceivedTrackingCount++;
      }
    }

    delete packet;
    packet = NULL;
  }

  myConnectionTimeoutMutex.lock();
  if (myTimeoutTime > 0 && 
      ((-myLastOdometryReceivedTime.mSecTo()) > myTimeoutTime))
  {
    char buf[10000];
    sprintf(buf, 
	    "Losing connection because no odometry received from robot in %ld milliseconds (greater than the timeout of %d).", 
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
  if (myTimeoutTime > 0 && 
      ((-myLastPacketReceivedTime.mSecTo()) > myTimeoutTime))
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
    MvrLog::log(MvrLog::Normal, "Rcvd(t): time taken %ld %d", start.mSecSince(),
	       myPacketsReceivedTrackingStarted.mSecSince());

}


/// This function gets called from the MvrRobotPacketReaderThread, and
/// does the actual reading of packets... so that it their timing
/// isn't affected by the rest of the sync loop
MVREXPORT void MvrRobot::packetHandlerThreadedReader(void)
{
  bool isAllocatingPackets = myReceiver.isAllocatingPackets();
  myReceiver.setAllocatingPackets(true);

  MvrTime lastPacketReceived;

  MvrRobotPacket *packet = NULL;

  while (isRunning())
  {
    if (myConn == NULL || 
	myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
    {
      MvrUtil::sleep(1);
      continue;
    }
    if ((packet = myReceiver.receivePacket(1000)) != NULL)
    {

      lastPacketReceived.setToNow();
      myPacketMutex.lock();
      myPacketList.push_back(packet);
      /*
      MvrLog::log(MvrLog::Normal, "HTR: %x at %d (%x)",
		 packet->getID(),
		 myPacketsReceivedTrackingStarted.mSecSince(),
		 packet->getID() & 0xf0);
      */
      packet = NULL;
      myPacketMutex.unlock();
      myPacketReceivedCondition.broadcast();
    }
    /* this is taken out for now since it'd spam in cases when the receiver returns instantly and fill the log file
    else
    {
      if (lastPacketReceived.mSecSince() > 1000)
      {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::packetReader: Took longer than 1000 mSec for packet (last packet %d mSec ago)",
		   lastPacketReceived.mSecSince());
	lastPacketReceived.setToNow();
      }
    }
    */
  }
  
  myReceiver.setAllocatingPackets(isAllocatingPackets);
}

/**
   Runs the resolver on the actions, it just saves these values for
   use by the stateReflector, otherwise it sends these values straight
   down to the robot.  
   @see addAction 
   @see remAction
**/
MVREXPORT void MvrRobot::actionHandler(void)
{
  MvrActionDesired *actDesired;

  if (myResolver == NULL || myActions.size() == 0 || !isConnected())
    return;
  
  actDesired = myResolver->resolve(&myActions, this, myLogActions);
  
  myActionDesired.reset();

  if (actDesired == NULL)
    return;

  myActionDesired.merge(actDesired);  

  if (myLogActions)
  {
    MvrLog::log(MvrLog::Normal, "Final resolved desired:");
    myActionDesired.log();
  }
}

/**
   Sets a time such that if the number of milliseconds between cycles
   goes over this then there will be an MvrLog::log(MvrLog::Normal)
   warning.

   @param ms the number of milliseconds between cycles to warn over, 0
   turns warning off
 **/
MVREXPORT void MvrRobot::setCycleWarningTime(unsigned int ms)
{
  myCycleWarningTime = ms;
  // we don't have to send it down because the functor gets it each cycle
}

/**
   Sets a time such that if the number of milliseconds between cycles
   goes over this then there will be an MvrLog::log(MvrLog::Normal)
   warning.

   @return the number of milliseconds between cycles to warn over, 0
   means warning is off
**/
MVREXPORT unsigned int MvrRobot::getCycleWarningTime(void) const
{
  return myCycleWarningTime;
}

/**
   Sets a time such that if the number of milliseconds between cycles
   goes over this then there will be an MvrLog::log(MvrLog::Normal)
   warning.

   @return the number of milliseconds between cycles to warn over, 0
   means warning is off
**/
MVREXPORT unsigned int MvrRobot::getCycleWarningTime(void)
{
  return myCycleWarningTime;
}

/**
   Sets the number of milliseconds between cycles, at each cycle is 
   when all packets are processed, all sensors are interpretted, all
   actions are called, and all user tasks are serviced.  Be warned,
   if you set this too small you could overflow your serial connection.
   @param ms the number of milliseconds between cycles
 **/
MVREXPORT void MvrRobot::setCycleTime(unsigned int ms)
{
  myCycleTime = ms;
}

/**
   This is the amount of time the robot will stabilize for after it
   has connected to the robot (it won't report it is connected until
   after this time is over).  By convention you should never set this
   lower than what you find the value at (though it will let you) this
   is so that everything can get itself stabilized before we let
   things drive.


   @param mSecs the amount of time to stabilize for (0 disables)

   @see addStabilizingCB
**/
MVREXPORT void MvrRobot::setStabilizingTime(int mSecs)
{
  if (mSecs > 0)
    myStabilizingTime = mSecs;
  else
    myStabilizingTime = 0;
}

/**
   This is the amount of time the robot will stabilize for after it
   has connected to the robot (it won't report it is connected until
   after this time is over).
**/
MVREXPORT int MvrRobot::getStabilizingTime(void) const
{
  return myStabilizingTime;
}

/**
   Finds the number of milliseconds between cycles, at each cycle is 
   when all packets are processed, all sensors are interpretted, all
   actions are called, and all user tasks are serviced.  Be warned,
   if you set this too small you could overflow your serial connection.
   @return the number of milliseconds between cycles
**/
MVREXPORT unsigned int MvrRobot::getCycleTime(void) const
{
  return myCycleTime;
}


/**
   @param multiplier when the MvrRobot is waiting for a connection
   packet back from a robot, it waits for this multiplier times the
   cycle time for the packet to come back before it gives up on it...
   This should be small for normal connections but if doing something
   over a slow network then you may want to make it larger

 **/
MVREXPORT void MvrRobot::setConnectionCycleMultiplier(unsigned int multiplier)
{
  myConnectionCycleMultiplier = multiplier;
}

/**
   @return when the MvrRobot is waiting for a connection packet back
   from a robot, it waits for this multiplier times the cycle time for
   the packet to come back before it gives up on it...  This should be
   small for normal connections but if doing something over a slow
   network then you may want to make it larger
**/
MVREXPORT unsigned int MvrRobot::getConnectionCycleMultiplier(void) const
{
  return myConnectionCycleMultiplier;
}


/** 
    This function is only for serious developers, it basically runs the 
    loop once.  You would use this function if you were wanting to use robot
    control in some other monolithic program, so you could work within its 
    framework, rather than trying to get it to work in ARIA.
**/
MVREXPORT void MvrRobot::loopOnce(void)
{
  if (mySyncTaskRoot != NULL)
    mySyncTaskRoot->run();
  
  incCounter();
}


// DigIn IR logic is reverse.  0 means broken, 1 means not broken

MVREXPORT bool MvrRobot::isLeftTableSensingIRTriggered(void) const
{
  if (myParams->haveTableSensingIR())
  {
    if (myParams->haveNewTableSensingIR() && myIODigInSize > 3)
      return !(getIODigIn(3) & MvrUtil::BIT1);
    else
      return !(getDigIn() & MvrUtil::BIT0);
  }
  return 0;
}

MVREXPORT bool MvrRobot::isRightTableSensingIRTriggered(void) const
{
  if (myParams->haveTableSensingIR())
  {
    if (myParams->haveNewTableSensingIR() && myIODigInSize > 3) 
      return !(getIODigIn(3) & MvrUtil::BIT0);
    else
      return !(getDigIn() & MvrUtil::BIT1);
  }
  return 0;
}

MVREXPORT bool MvrRobot::isLeftBreakBeamTriggered(void) const
{
  if (myParams->haveTableSensingIR())
  {
    if (myParams->haveNewTableSensingIR() && myIODigInSize > 3) 
      return !(getIODigIn(3) & MvrUtil::BIT2);
    else
      return !(getDigIn() & MvrUtil::BIT3);
  }
  return 0;
}

MVREXPORT bool MvrRobot::isRightBreakBeamTriggered(void) const
{
  if (myParams->haveTableSensingIR())
  {
    if (myParams->haveNewTableSensingIR() && myIODigInSize > 3) 
      return !(getIODigIn(3) & MvrUtil::BIT3);
    else
      return !(getDigIn() & MvrUtil::BIT2);
  }
  return 0;
}

MVREXPORT int MvrRobot::getMotorPacCount(void) const
{
  if (myTimeLastMotorPacket == time(NULL))
    return myMotorPacCount;
  if (myTimeLastMotorPacket == time(NULL) - 1)
    return myMotorPacCurrentCount;
  return 0;
}

MVREXPORT int MvrRobot::getSonarPacCount(void) const
{
  if (myTimeLastSonarPacket == time(NULL))
    return mySonarPacCount;
  if (myTimeLastSonarPacket == time(NULL) - 1)
    return mySonarPacCurrentCount;
  return 0;
}


MVREXPORT bool MvrRobot::processMotorPacket(MvrRobotPacket *packet)
{
  int x, y, th, qx, qy, qth; 
  double deltaX, deltaY, deltaTh;

  int numReadings;
  int sonarNum;
  int sonarRange;

  if (packet->getID() != 0x32 && packet->getID() != 0x33) 
    return false;

  // upkeep the counting variable
  if (myTimeLastMotorPacket != time(NULL)) 
  {
    myTimeLastMotorPacket = time(NULL);
    myMotorPacCount = myMotorPacCurrentCount;
    myMotorPacCurrentCount = 0;
  }
  myMotorPacCurrentCount++;

  x = (packet->bufToUByte2() & 0x7fff);
  y = (packet->bufToUByte2() & 0x7fff);
  th = packet->bufToByte2();

  if (myFakeFirstEncoderPose)
  {
    myLastX = x;
    myLastY = y;
    myLastTh = th;
    myFakeFirstEncoderPose = false;
  }

  if (myFirstEncoderPose)
  {
    qx = 0;
    qy = 0;
    qth = 0;
    myFirstEncoderPose = false;
    myRawEncoderPose.setPose(
	    myParams->getDistConvFactor() * x,
	    myParams->getDistConvFactor() * y, 
	    MvrMath::radToDeg(myParams->getAngleConvFactor() * (double)th));
    myEncoderPose = myRawEncoderPose;
    myEncoderTransform.setTransform(myEncoderPose, myGlobalPose);
  }
  else 
  {
    qx = x - myLastX;
    qy = y - myLastY;
    qth = th - myLastTh;
  }


  //MvrLog::log(MvrLog::Terse, "qx %d qy %d,  x %d y %d,  lastx %d lasty %d", qx, qy, x, y, myLastX, myLastY);  
  myLastX = x;
  myLastY = y;
  myLastTh = th;

  if (qx > 0x1000) 
    qx -= 0x8000;
  if (qx < -0x1000)
    qx += 0x8000;

  if (qy > 0x1000) 
    qy -= 0x8000;
  if (qy < -0x1000)
    qy += 0x8000;
  
  deltaX = myParams->getDistConvFactor() * (double)qx;
  deltaY = myParams->getDistConvFactor() * (double)qy;
  deltaTh = MvrMath::radToDeg(myParams->getAngleConvFactor() * (double)qth);
  //encoderTh = MvrMath::radToDeg(myParams->getAngleConvFactor() * (double)(th));


  // encoder stuff was here



  myLeftVel = myParams->getVelConvFactor() * packet->bufToByte2();
  myRightVel = myParams->getVelConvFactor() * packet->bufToByte2();
  myVel = (myLeftVel + myRightVel)/2.0;

  double batteryVoltage;
  batteryVoltage = packet->bufToUByte() * .1;
  if (!myIgnoreMicroControllerBatteryInfo)
  {
    myBatteryVoltage = batteryVoltage;
    myBatteryAverager.add(myBatteryVoltage);
  }

  myStallValue = packet->bufToByte2();
  
  //MvrLog::log("x %.1f y %.1f th %.1f vel %.1f voltage %.1f", myX, myY, myTh, 
  //myVel, myBatteryVoltage);
  if (!myKeepControlRaw) 
    myControl = MvrMath::fixAngle(MvrMath::radToDeg(
					 myParams->getAngleConvFactor() *
					 (packet->bufToByte2() - th)));
  else
    myControl = packet->bufToByte2();

  myFlags = packet->bufToUByte2();
  myCompass = 2*packet->bufToUByte();
  
  for (numReadings = packet->bufToByte(); numReadings > 0; numReadings--)
  {
    sonarNum = packet->bufToByte();
    sonarRange = MvrMath::roundInt(
	    (double)packet->bufToUByte2() * myParams->getRangeConvFactor());
    processNewSonar(sonarNum, sonarRange, packet->getTimeReceived());
  }
  
  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    myAnalogPortSelected = packet->bufToUByte2();
    myAnalog = packet->bufToByte();
    myDigIn = packet->bufToByte();
    myDigOut = packet->bufToByte();
  }

  double realBatteryVoltage;
  if (packet->getDataLength() - packet->getDataReadLength() > 0)
    realBatteryVoltage = packet->bufToUByte2() * .1;
  else
    realBatteryVoltage = myBatteryVoltage;
  if (!myIgnoreMicroControllerBatteryInfo)
  {
    myRealBatteryVoltage = realBatteryVoltage;
    myRealBatteryAverager.add(myRealBatteryVoltage);
  }


  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    if (!myOverriddenChargeState)
      myChargeState = (ChargeState) packet->bufToUByte();
    else
      packet->bufToUByte();
  }
  else if (!myOverriddenChargeState)
    myChargeState = CHARGING_UNKNOWN;

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
    myRotVel = (double)packet->bufToByte2() / 10.0;
  else
    myRotVel = MvrMath::radToDeg((myRightVel - myLeftVel) / 2.0 * 
				myParams->getDiffConvFactor());

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    myHasFaultFlags = true;
    myFaultFlags = packet->bufToUByte2();  
  }
  else
  {
    myHasFaultFlags = false;
    myFaultFlags = 0; //packet->bufToUByte2();  
  }

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    myLatVel = packet->bufToByte2();
  }

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    myTemperature = packet->bufToByte();
  }

  double stateOfCharge;
  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    stateOfCharge = packet->bufToByte();
    if (!myIgnoreMicroControllerBatteryInfo)
    {
      myStateOfCharge = stateOfCharge;
      if (!myHaveStateOfCharge && myStateOfCharge > 0)
	myHaveStateOfCharge = true;
      myStateOfChargeSetTime.setToNow();
    }
  }

  // holder for information we log if we're tracking packet received
  // and have timing info
  std::string movementReceivedTimingStr;

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    MvrTypes::UByte4 uCUSec = 0;
    MvrTypes::UByte4 lpcNowUSec = 0;
    MvrTypes::UByte4 lpcUSec = 0;
    MvrTime now;
    long long mSecSince = -999;
    MvrTime recvTime;

    uCUSec = packet->bufToUByte4();
    // make sure we get a good value
    if ((myPacketsReceivedTracking || myLogMovementReceived) && 
	myMTXTimeUSecCB != NULL && myMTXTimeUSecCB->invokeR(&lpcNowUSec))
    {
      mSecSince = packet->getTimeReceived().mSecSinceLL(now);
      lpcUSec = lpcNowUSec - (mSecSince + myOdometryDelay) * 1000;
      
      recvTime = packet->getTimeReceived();
      recvTime.addMSec(-myOdometryDelay);

      char buf[1024];
      sprintf(buf, " time: %6lld.%03lld.000 uCTime: %6u.%03u.%.03u",
	      recvTime.getSecLL(), recvTime.getMSecLL(),
	      uCUSec / 1000000, (uCUSec % 1000000) / 1000, uCUSec % 1000);
      movementReceivedTimingStr = buf;
    }

    if (myPacketsReceivedTracking)
      MvrLog::log(MvrLog::Normal, "MotorPacketTiming: commDiff %lld fpgaNowDiff %2d.%03d fpgaPacketDiff %2d.%03d bytes %d\nFPGA:uC  %6u.%03u.%.03u\nFPGA:lpcNow %6u.%03u.%.03u\nMvrTime:1stByte %6lld.%03lld\nMvrTime:now     %6lld.%.03lld\nFPGA:lpcPacket %6u.%03u.%03u", 
		 mSecSince, 
		 ((lpcNowUSec - uCUSec) % 1000000) / 1000, (lpcNowUSec - uCUSec) % 1000,
		 ((lpcUSec - uCUSec) % 1000000) / 1000, (lpcUSec - uCUSec) % 1000,		 
		 packet->getLength(),
		 
		 uCUSec / 1000000, (uCUSec % 1000000) / 1000, uCUSec % 1000,
		 lpcNowUSec / 1000000, (lpcNowUSec % 1000000) / 1000, lpcNowUSec % 1000,
		 packet->getTimeReceived().getSecLL(), 
		 packet->getTimeReceived().getMSecLL(),
		 now.getSecLL(), now.getMSecLL(),
		 lpcUSec / 1000000, (lpcUSec % 1000000) / 1000, lpcUSec % 1000);
    
  }
  else if (myLogMovementReceived)
  {
    MvrTime recvTime = packet->getTimeReceived();
    recvTime.addMSec(-myOdometryDelay);
    
    char buf[1024];
    sprintf(buf, " time: %6lld.%03lld.000",
	    recvTime.getSecLL(), recvTime.getMSecLL());
    movementReceivedTimingStr = buf;
  }

  if (packet->getDataLength() - packet->getDataReadLength() > 0)
  {
    myHasFlags3 = true;
    myFlags3 = packet->bufToUByte4();      
  }
  else
  {
    myHasFlags3 = false;
    myFlags3 = 0;
  }
  
  if(myLogSIPContents)
  {
    MvrLog::log(MvrLog::Normal, "SIP Contents:\n\tx=%d, y=%d, th=%d, lvel=%.2f, rvel=%.2f, battery=%.1f, stallval=0x%x, control=%d, compass=%d",
      x, y, th, myLeftVel, myRightVel, myBatteryVoltage, myStallValue, myControl, myCompass);
    MvrLog::log(MvrLog::Normal, "\tnumSonar=%d, gripstate=%d, anport=%d, analog=%d, digin=0x%x, digout=0x%x, batteryX10=%d, chargestage=%d rotvel=%.2f", 
      numReadings, (char)myAnalogPortSelected, (char)(myAnalogPortSelected>>8), myAnalog, myDigIn, myDigOut, myRealBatteryVoltage, (int)myChargeState, myRotVel);
    if(myHasFaultFlags)
      MvrLog::log(MvrLog::Normal, "\tfaultflags=0x%x", myFaultFlags);
    else
      MvrLog::log(MvrLog::Normal, "\tno faultflags");
    MvrLog::log(MvrLog::Normal, "\tlatvel=%.2f temperature=%d", myLatVel, myTemperature);
    if(myHaveStateOfCharge)
      MvrLog::log(MvrLog::Normal, "\tsoc=0x%x", myStateOfCharge);
    else
      MvrLog::log(MvrLog::Normal, "\tno soc");
  }

  /*
    Okay how this works is like so.  

    We keep around the raw encoder position, because we must use this
    to find differences between last position and this position.
    
    We find the difference in x and y positions (deltaX and deltaY)
    and keep these around for later use, but we also add these to our
    raw encoder readings for X and Y.  We also find the change in
    angle (deltaTh), which is used for inertial corrections, and added
    to the raw encoder heading to find which the current raw encoder
    heading.


    From here there are two paths:

    Path 1) Have a callback.  If we have a callback it means that we
    have an inertial nav device of some kind.  If this is the case,
    then we pass the callback the delta between last position and
    current position, along with the time of the current position,
    then the callback gives us back a new delta theta (deltaTh).  We
    then need to rotate the deltaX and deltaY into our corrected
    encoder space.  We do this by making a transform that takes the
    raw encoder heading and transforms it to what our new heading is
    (adding deltaTh to our current encoder th), and then applying that
    transform, taking the results as our new deltaX and deltaY.

    Path 2) We have no callback, we just use the heading that came
    back from the robot as our delta theta (deltaTh);

    From here the two paths unify again.  deltaX and deltaY are added
    to the encoder pose (this is the corrected encoder pose), and the
    encoder heading is set to the newTh.

    Note that this leaves a difference between rawEncoder heading and
    our heading, which is fine, BUT if you are sending heading
    commands to the robot you need to compenstate for the difference
    between these. 
    
    Note above that we return deltaTh instead of just heading so that
    we can turn inertial on and off without losing track of where
    we're at... since we're just adding in deltas from the heading it
    doesn't matter how we switch around the callback.
    
  **/

  myRawEncoderPose.setX(myRawEncoderPose.getX() + deltaX);
  myRawEncoderPose.setY(myRawEncoderPose.getY() + deltaY);
  myRawEncoderPose.setTh(myRawEncoderPose.getTh() + deltaTh);

  // check if there is a correction callback, if there is get the new
  // heading out of it instead of using the raw encoder heading
  if (myEncoderCorrectionCB != NULL)
  {
    MvrPoseWithTime deltaPose(deltaX, deltaY, deltaTh,
			     packet->getTimeReceived());
    deltaTh = myEncoderCorrectionCB->invokeR(deltaPose);   
    MvrTransform trans(MvrPose(0, 0, myRawEncoderPose.getTh()),
		      MvrPose(0, 0,
			     MvrMath::addAngle(myEncoderPose.getTh(), 
					      deltaTh)));

    MvrPose rotatedDelta = trans.doTransform(MvrPose(deltaX, deltaY, 0));

    deltaX = rotatedDelta.getX();
    deltaY = rotatedDelta.getY();
  }

  myEncoderPose.setTime(packet->getTimeReceived());
  myEncoderPose.setX(myEncoderPose.getX() + deltaX);
  myEncoderPose.setY(myEncoderPose.getY() + deltaY);
  myEncoderPose.setTh(MvrMath::addAngle(myEncoderPose.getTh(), deltaTh));

  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);

  double degreesTravelled = fabs(deltaTh);
  double distTravelled = sqrt(fabs(deltaX * deltaX + deltaY * deltaY));

  myOdometerDegrees += degreesTravelled;
  myOdometerDistance += distTravelled;

  myTripOdometerDegrees += degreesTravelled;
  myTripOdometerDistance += distTravelled;

  /*
    if (myLogMovementReceived)
    MvrLog::log(MvrLog::Normal, 
	       "Global (%5.0f %5.0f %5.0f) Encoder (%5.0f %5.0f %5.0f) Raw (%5.0f %5.0f %5.0f) Rawest (%5d %5d %5d) Delta (%5.0f %5.0f %5.0f) Conv %5.2f",
	       myGlobalPose.getX(), myGlobalPose.getY(),
	       myGlobalPose.getTh(),
	       myEncoderPose.getX(), myEncoderPose.getY(),
	       myEncoderPose.getTh(),
	       myRawEncoderPose.getX(), myRawEncoderPose.getY(), 
	       myRawEncoderPose.getTh(), x, y, th, deltaX, deltaY, deltaTh,
	       myParams->getDistConvFactor());	      
  */
  if (myLogMovementReceived && 
      (fabs(deltaX) > .0001 || fabs(deltaY) > .0001 || fabs(deltaTh) > .0001))
    MvrLog::log(MvrLog::Normal, 
	       "Global: %5.0f %5.0f %7.1f GlobalDelta: %5.0f %5.0f %7.1f uC: %5.0f %5.0f %7.1f uCDelta: %5.0f %5.0f %7.1f RawUC: %5d %5d %5d%s",
	       myGlobalPose.getX(), myGlobalPose.getY(),
	       myGlobalPose.getTh(),
	       myGlobalPose.getX() - myLastGlobalPose.getX(), 
	       myGlobalPose.getY() - myLastGlobalPose.getY(), 
	       MvrMath::subAngle(myGlobalPose.getTh(), myLastGlobalPose.getTh()),
	       myEncoderPose.getX(), myEncoderPose.getY(),
	       myEncoderPose.getTh(),
	       deltaX, deltaY, deltaTh, x, y, th, 
	       movementReceivedTimingStr.c_str());

  myLastGlobalPose = myGlobalPose;

  if (myLogMovementReceived && sqrt(deltaX*deltaX + deltaY*deltaY) > 1000)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot: Travelled over 1000 in a cycle, which is unlikely");
    MvrLog::logBacktrace(MvrLog::Normal);
  }

  if (myLogVelocitiesReceived)
  {
    if (!hasLatVel())
      MvrLog::log(MvrLog::Normal, 
		 "     TransVel: %4.0f RotVel: %4.0f dTh: %4.0f Th: %4.0f TransAcc: %4.0f RotAcc: %4.0f ddTh: %4.0f",
		 myVel, myRotVel, myLastHeading - getTh(), getTh(), 
		 myVel - myLastVel, myRotVel - myLastRotVel,
		 myLastDeltaHeading - (myLastHeading - getTh()));
    else
      MvrLog::log(MvrLog::Normal, 
		 "     TransVel: %4.0f RotVel: %4.0f dTh: %4.0f Th: %4.0f DTrans %4.0f DRot %4.0f ddth: %4.0f LatVel: %4.0f DLat: %4.0f",
		 myVel, myRotVel, myLastHeading - getTh(), getTh(), 
		 myVel - myLastVel, myRotVel - myLastRotVel,
		 myLastDeltaHeading - (myLastHeading - getTh()),
		 myLatVel, myLatVel - myLastLatVel);
  }
  myLastVel = myVel;
  myLastRotVel = myRotVel;
  myLastLatVel = myLatVel;
  myLastHeading = getTh();
  myLastDeltaHeading = myLastHeading - getTh();

  //MvrLog::log(MvrLog::Terse, "(%.0f %.0f) (%.0f %.0f)", deltaX, deltaY, myGlobalPose.getX(),	     myGlobalPose.getY());

  MvrTime packetTime = packet->getTimeReceived();
  /// used it doesn't have to account for the odometry delay
  packetTime.addMSec(-myOdometryDelay);

  //MvrLog::log(MvrLog::Normal, "Robot packet %lld mSec old", packetTime.mSecSince());
  
  myConnectionTimeoutMutex.lock();
  myLastOdometryReceivedTime = packetTime;
  myConnectionTimeoutMutex.unlock();

  myInterpolation.addReading(packetTime, myGlobalPose);
  myEncoderInterpolation.addReading(packetTime, myEncoderPose);

  return true;
}

MVREXPORT void MvrRobot::processNewSonar(char number, int range, 
				       MvrTime timeReceived)
{
  /**
     This function used to just create more sonar readings if it
     didn't have the sonar number that was given (only the case if
     that sonar didn't have an entry in the param file), this caused
     some silent bugs in other peoples code and so was
     removed... especially since we don't know where the sonar are at
     if they weren't in the parameter file anyways.
   **/
  std::map<int, MvrSensorReading *>::iterator it;
  MvrSensorReading *sonar;
  MvrTransform encoderTrans;
  MvrPose encoderPose;

  if ((it = mySonars.find(number)) != mySonars.end())
  {
    sonar = (*it).second;
    sonar->newData(range, getPose(), getEncoderPose(), getToGlobalTransform(), 
		   getCounter(), timeReceived); 
		 
    if (myTimeLastSonarPacket != time(NULL)) 
    {
      myTimeLastSonarPacket = time(NULL);
      mySonarPacCount = mySonarPacCurrentCount;
      mySonarPacCurrentCount = 0;
    }
    mySonarPacCurrentCount++;
  }
  else if (!myWarnedAboutExtraSonar)
  {
    MvrLog::log(MvrLog::Normal, "Robot gave back extra sonar reading!  Either the parameter file for the robot or the firmware needs updating.");
    myWarnedAboutExtraSonar = true;
  }
}



MVREXPORT bool MvrRobot::processEncoderPacket(MvrRobotPacket *packet)
{
  if (packet->getID() != 0x90)
    return false;
  myLeftEncoder = packet->bufToByte4();
  myRightEncoder = packet->bufToByte4();
  return true;
}

MVREXPORT bool MvrRobot::processIOPacket(MvrRobotPacket *packet)
{
  int i, num;

  if (packet->getID() != 0xf0)
    return false;

  myLastIOPacketReceivedTime = packet->getTimeReceived();

  // number of DigIn bytes
  num = packet->bufToUByte();
  for (i = 0; i < num; ++i)
    myIODigIn[i] = packet->bufToUByte();
  myIODigInSize = num;

  // number of DigOut bytes
  num = packet->bufToUByte();
  for (i = 0; i < num; ++i)
    myIODigOut[i] = packet->bufToUByte();
  myIODigOutSize = num;

  // number of A/D bytes
  num = packet->bufToUByte();
  for (i = 0; i < num; ++i)
    myIOAnalog[i] = packet->bufToUByte2();
  myIOAnalogSize = num;

  return true;
}

/**
   @param num the sonar number to check, should be between 0 and the number of
   sonar, the function won't fail if a bad number is given, will just return
   -1
   @return -1 if the sonar has never returned a reading, otherwise the sonar 
   range, which is the distance from the physical sonar disc to where the sonar
   bounced back
   @see getNumSonar
**/
MVREXPORT int MvrRobot::getSonarRange(int num) const
{
  std::map<int, MvrSensorReading *>::const_iterator it;
  
  if ((it = mySonars.find(num)) != mySonars.end())
    return (*it).second->getRange();
  else
    return -1;
}

/**
   @param num the sonar number to check, should be between 0 and the number of
   sonar, the function won't fail if a bad number is given, will just return
   false
   @return false if the sonar reading is old, or if there was no reading from
   that sonar, in the current SIP cycle.
   For best results, use this function in sync with the SIP cycle, for example,
   from a Sensor Interpretation Task Callback (see addSensorInterpTask).
**/

MVREXPORT bool MvrRobot::isSonarNew(int num) const
{
  std::map<int, MvrSensorReading *>::const_iterator it;
  
  if ((it = mySonars.find(num)) != mySonars.end())
    return (*it).second->isNew(getCounter());
  else
    return false;
}

/**
   @param num the sonar number to check, should be between 0 and the number of
   sonar, the function won't fail if a bad number is given, will just return
   false
   @return NULL if there is no sonar defined for the given number, otherwise
   it returns a pointer to an instance of the MvrSensorReading, note that this 
   class retains ownership, so the instance pointed to should not be deleted
   and no pointers to it should be stored.  Note that often there will be sonar
   defined but no readings for it (since the readings may be created by the 
   parameter reader), if there has never been a reading from the sonar then
   the range of that sonar will be -1 and its counterTaken value will be 0
**/
MVREXPORT MvrSensorReading *MvrRobot::getSonarReading(int num) const
{
  std::map<int, MvrSensorReading *>::const_iterator it;
  
  if ((it = mySonars.find(num)) != mySonars.end())
    return (*it).second;
  else
    return NULL;
}


/**
   @param command the command number to send
   @return whether the command could be sent or not
   @sa MvrCommands
**/
MVREXPORT bool MvrRobot::com(unsigned char command)
{
  if (myPacketsSentTracking)
    MvrLog::log(MvrLog::Normal, "Sent: com(%d)", command);
  return mySender.com(command);
}

/**
   @param command the command number to send
   @param argument the integer argument to send with the command
   @return whether the command could be sent or not
   @sa MvrCommands
**/
MVREXPORT bool MvrRobot::comInt(unsigned char command, short int argument)
{
  if (myPacketsSentTracking)
    MvrLog::log(MvrLog::Normal, "Sent: comInt(%d, %d)", command, argument);
  return mySender.comInt(command, argument);
}

/**
   @param command the command number to send
   @param high the high byte to send with the command
   @param low the low byte to send with the command
   @return whether the command could be sent or not
   @sa MvrCommands
**/
MVREXPORT bool MvrRobot::com2Bytes(unsigned char command, char high, char low)
{
  if (myPacketsSentTracking)
    MvrLog::log(MvrLog::Normal, "Sent: com2Bytes(%d, %d, %d)", command, 
	       high, low);
  return mySender.com2Bytes(command, high, low);
}

/**
   @param command the command number to send
   @param argument NULL-terminated string to get data from to send with the command; length to send with packet is determined by strlen
   @return whether the command could be sent or not
   @sa MvrCommands
**/
MVREXPORT bool MvrRobot::comStr(unsigned char command, const char *argument)
{
  if (myPacketsSentTracking)
    MvrLog::log(MvrLog::Normal, "Sent: comStr(%d, '%s')", command, 
	       argument);
  return mySender.comStr(command, argument);
}

/**
 * Sends a length-prefixed string command to the robot, copying 'size'
 * bytes of data from 'str' into the packet.
   @param command the command number to send
   @param str copy data to send from this character array 
   @param size length of the string to send; copy this many bytes from 'str'; use this value as the length prefix byte before the sent string. This length must be less than the maximum packet size of 200.
   @return whether the command could be sent or not
   @sa MvrCommands
**/
MVREXPORT bool MvrRobot::comStrN(unsigned char command, const char *str, 
			       int size)
{
  if (myPacketsSentTracking)
  {
    char strBuf[512];
    strncpy(strBuf, str, size);
    strBuf[size] = '\0';
    MvrLog::log(MvrLog::Normal, "Sent: comStrN(%d, '%s') (size %d)",
	       command, strBuf, size);
  }
  return mySender.comStrN(command, str, size);
}

MVREXPORT bool MvrRobot::comDataN(unsigned char command, const char* data, int size)
{
  if(myPacketsSentTracking)
  {
    MvrLog::log(MvrLog::Normal, "Sent: comDataN(%d, <data...>) (size %d)", command, size);
  }
  return mySender.comDataN(command, data, size);
}


MVREXPORT int MvrRobot::getClosestSonarRange(double startAngle, double endAngle) const
{
  int num;
  num = getClosestSonarNumber(startAngle, endAngle);
  if (num == -1)
    return -1;
  else 
    return getSonarRange(num);
}

MVREXPORT int MvrRobot::getClosestSonarNumber(double startAngle, double endAngle) const
{
  int i;
  MvrSensorReading *sonar;
  int closestReading;
  int closestSonar;
  bool noReadings = true;

  for (i = 0; i < getNumSonar(); i++) 
  {
    sonar = getSonarReading(i);
    if (sonar == NULL)
    {
      MvrLog::log(MvrLog::Terse, "Have an empty sonar at number %d, there should be %d sonar.", i, getNumSonar());
      continue;
    }
    if (MvrMath::angleBetween(sonar->getSensorTh(), startAngle, endAngle))
    {
      if (noReadings)
      {
	closestReading = sonar->getRange();
	closestSonar = i;
	noReadings = false;
      }
      else if (sonar->getRange() < closestReading)
      {
	closestReading = sonar->getRange();
	closestSonar = i;
      }
    }
  }

  if (noReadings)
    return -1;
  else
    return closestSonar;
}

/**
  @note It is not neccesary to call this method directly to add a laser (MvrLaser
  subclass or MvrSick object) if using MvrLaserConnector or MvrSimpleConnector, those
  classes automatically add the laser(s).  (But you may call this method with e.g.
  MvrSonarDevice, MvrIRs, MvrBumpers, etc.)
*/
MVREXPORT void MvrRobot::addRangeDevice(MvrRangeDevice *device)
{
  device->setRobot(this);
  myRangeDeviceList.push_front(device);
}

/**
   @param name remove the first device with this name
**/
MVREXPORT void MvrRobot::remRangeDevice(const char *name)
{
  std::list<MvrRangeDevice *>::iterator it;
  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    if (strcmp(name, (*it)->getName()) == 0)
    {
      myRangeDeviceList.erase(it);
      return;
    }
  }
}

/**
   @param device remove the first device with this pointer value
**/
MVREXPORT void MvrRobot::remRangeDevice(MvrRangeDevice *device)
{
  std::list<MvrRangeDevice *>::iterator it;
  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    if ((*it) == device)
    {
      myRangeDeviceList.erase(it);
      return;
    }
  }
}

/**
   @param name return the first device with this name
   @param ignoreCase true to ignore case, false to pay attention to it
   @return if found, a range device with the given name, if not found NULL
**/
MVREXPORT MvrRangeDevice *MvrRobot::findRangeDevice(const char *name, 
						 bool ignoreCase)
{
  std::list<MvrRangeDevice *>::iterator it;
  MvrRangeDevice *device;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    if ((ignoreCase && strcasecmp(name, device->getName()) == 0) || 
	(!ignoreCase && strcmp(name, device->getName()) == 0))
    {
      return device;
    }
  }
  return NULL;
}

/**
   @param name return the first device with this name
   @param ignoreCase true to ignore case, false to pay attention to it
   @return if found, a range device with the given name, if not found NULL
**/
MVREXPORT const MvrRangeDevice *MvrRobot::findRangeDevice(const char *name,
						       bool ignoreCase) const
{
  std::list<MvrRangeDevice *>::const_iterator it;
  MvrRangeDevice *device;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    if ((ignoreCase && strcasecmp(name, device->getName()) == 0) || 
	(!ignoreCase && strcmp(name, device->getName()) == 0))
    {
      return device;
    }
  }
  return NULL;
}

/** 
    This gets the list of range devices attached to this robot, do NOT
    manipulate this list directly.  If you want to manipulate use the 
    appropriate addRangeDevice, or remRangeDevice
    @return the list of range dvices attached to this robot
**/
MVREXPORT std::list<MvrRangeDevice *> *MvrRobot::getRangeDeviceList(void)
{
  return &myRangeDeviceList;
}

/**
   @param device the device to check for
**/
MVREXPORT bool MvrRobot::hasRangeDevice(MvrRangeDevice *device) const
{
  std::list<MvrRangeDevice *>::const_iterator it;
  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    if ((*it) == device)
      return true;
  }
  return false;
}

/**
 *  Find the closest reading from any range device's set of current readings
 *  within a polar region or "slice" defined by the given angle range.
 *  This function iterates through each registered range device (see 
 *  addRangeDevice()), calls MvrRangeDevice::lockDevice(), uses
 *  MvrRangeDevice::currentReadingPolar() to find a reading, then calls
 *  MvrRangeDevice::unlockDevice().
 *
 *  @copydoc MvrRangeDevice::currentReadingPolar()
 *  @param rangeDevice If not null, then a pointer to the MvrRangeDevice 
 *    that provided the returned reading is placed in this variable.
   @param useLocationDependentDevices If false, ignore sensor devices that are "location dependent". If true, include them in this check.
 *
**/

MVREXPORT double MvrRobot::checkRangeDevicesCurrentPolar(
	double startAngle, double endAngle, double *angle, 
	const MvrRangeDevice **rangeDevice,
	bool useLocationDependentDevices) const
{
  double closest = 32000;
  double closeAngle, tempDist, tempAngle;
  std::list<MvrRangeDevice *>::const_iterator it;
  MvrRangeDevice *device;
  bool foundOne = false;
  const MvrRangeDevice *closestRangeDevice = NULL;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    device->lockDevice();
    if (!useLocationDependentDevices && device->isLocationDependent())
    {
      device->unlockDevice();
      continue;
    }
    if (!foundOne || 
	(tempDist = device->currentReadingPolar(startAngle, endAngle,
						&tempAngle)) < closest)
    {
      if (!foundOne)
      {
	closest = device->currentReadingPolar(startAngle, endAngle, 
					      &closeAngle);
	closestRangeDevice = device;
      }
      else
      {
	closest = tempDist;
	closeAngle = tempAngle;
	closestRangeDevice = device;
      }
      foundOne = true;
    }
    device->unlockDevice();
  }
  if (!foundOne)
    return -1;
  if (angle != NULL)
    *angle = closeAngle;
  if (rangeDevice != NULL)
    *rangeDevice = closestRangeDevice;
  return closest;
  
}


/**
 *  Find the closest reading from any range device's set of cumulative readings
 *  within a polar region or "slice" defined by the given angle range.
 *  This function iterates through each registered range device (see 
 *  addRangeDevice()), calls MvrRangeDevice::lockDevice(), uses
 *  MvrRangeDevice::cumulativeReadingPolar() to find a reading, then calls
 *  MvrRangeDevice::unlockDevice().
 *
 *  @copydoc MvrRangeDevice::cumulativeReadingPolar()
 *  @param rangeDevice If not null, then a pointer to the MvrRangeDevice 
 *    that provided the returned reading is placed in this variable.
   @param useLocationDependentDevices If false, ignore sensor devices that are "location dependent". If true, include them in this check.
**/
MVREXPORT double MvrRobot::checkRangeDevicesCumulativePolar(
	double startAngle, double endAngle, double *angle, 
	const MvrRangeDevice **rangeDevice, 
	bool useLocationDependentDevices) const
{
  double closest = 32000;
  double closeAngle, tempDist, tempAngle;
  std::list<MvrRangeDevice *>::const_iterator it;
  MvrRangeDevice *device;
  bool foundOne = false;
  const MvrRangeDevice *closestRangeDevice = NULL;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    device->lockDevice();
    if (!useLocationDependentDevices && device->isLocationDependent())
    {
      device->unlockDevice();
      continue;
    }
    if (!foundOne || 
	(tempDist = device->cumulativeReadingPolar(startAngle, endAngle,
						 &tempAngle)) < closest)
    {
      if (!foundOne)
      {
	closest = device->cumulativeReadingPolar(startAngle, endAngle, 
					      &closeAngle);
	closestRangeDevice = device;
      }
      else
      {
	closest = tempDist;
	closeAngle = tempAngle;
	closestRangeDevice = device;
      }
      foundOne = true;
    }
    device->unlockDevice();
  }
  if (!foundOne)
    return -1;
  if (angle != NULL)
    *angle = closeAngle;
  if (rangeDevice != NULL)
    *rangeDevice = closestRangeDevice;
  return closest;
  
}

/**
   Gets the closest reading in a region defined by the two points of a 
   rectangle.
   This goes through all of the registered range devices and locks each,
   calls currentReadingBox on it, and then unlocks it.

   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param readingPos a pointer to a position in which to store the location of
   @param rangeDevice If not null, then a pointer to the MvrRangeDevice 
     that provided the returned reading is placed in this variable.
   the closest position
   @param useLocationDependentDevices If false, ignore sensor devices that are "location dependent". If true, include them in this check.
   @return If >= 0 then this is the distance to the closest
   reading. If < 0 then there were no readings in the given region
**/

MVREXPORT double MvrRobot::checkRangeDevicesCurrentBox(
	double x1, double y1, double x2, double y2, MvrPose *readingPos,
	const MvrRangeDevice **rangeDevice, 
	bool useLocationDependentDevices) const
{

  double closest = 32000;
  double tempDist;
  MvrPose closestPos, tempPos;
  std::list<MvrRangeDevice *>::const_iterator it;
  MvrRangeDevice *device;
  bool foundOne = false;
  const MvrRangeDevice *closestRangeDevice = NULL;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    device->lockDevice();
    if (!useLocationDependentDevices && device->isLocationDependent())
    {
      device->unlockDevice();
      continue;
    }
    if (!foundOne || 
	(tempDist = device->currentReadingBox(x1, y1, x2, y2, &tempPos)) < closest)
    {
      if (!foundOne)
      {
	closest = device->currentReadingBox(x1, y1, x2, y2, &closestPos);
	closestRangeDevice = device;
      }
      else
      {
	closest = tempDist;
	closestPos = tempPos;
	closestRangeDevice = device;
      }
      foundOne = true;
    }
    device->unlockDevice();
  }
  if (!foundOne)
    return -1;
  if (readingPos != NULL)
    *readingPos = closestPos;
  if (rangeDevice != NULL)
    *rangeDevice = closestRangeDevice;
  return closest;
}

/**
   Gets the closest reading in a region defined by the two points of a 
   rectangle.
   This goes through all of the registered range devices and locks each,
   calls cumulativeReadingBox() on it, and then unlocks it. If a reading was 
   found in the box, returns with results.

   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param readingPos If not NULL, a pointer to a position in which to store the location of
   the closest position
   @param rangeDevice If not NULL, a pointer in which to store a pointer to the range device
   that provided the closest reading in the box.
   @param useLocationDependentDevices If false, ignore sensor devices that are "location dependent". If true, include them in this check.
   @return If  >= 0 then this is the distance to the closest
   reading. If < 0 then there were no readings in the given region
**/

MVREXPORT double MvrRobot::checkRangeDevicesCumulativeBox(
	double x1, double y1, double x2, double y2, MvrPose *readingPos,
	const MvrRangeDevice **rangeDevice, 
	bool useLocationDependentDevices) const
{

  double closest = 32000;
  double tempDist;
  MvrPose closestPos, tempPos;
  std::list<MvrRangeDevice *>::const_iterator it;
  MvrRangeDevice *device;
  bool foundOne = false;
  const MvrRangeDevice *closestRangeDevice = NULL;

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); ++it)
  {
    device = (*it);
    device->lockDevice();
    if (!useLocationDependentDevices && device->isLocationDependent())
    {
      device->unlockDevice();
      continue;
    }
    if (!foundOne || 
	(tempDist = device->cumulativeReadingBox(x1, y1, x2, y2, &tempPos)) < closest)
    {
      if (!foundOne)
      {
	closest = device->cumulativeReadingBox(x1, y1, x2, y2, &closestPos);
	closestRangeDevice = device;
      }
      else
      {
	closest = tempDist;
	closestPos = tempPos;
	closestRangeDevice = device;
      }
      foundOne = true;
    }
    device->unlockDevice();
  }
  if (!foundOne)
    return -1;
  if (readingPos != NULL)
    *readingPos = closestPos;
  if (rangeDevice != NULL)
    *rangeDevice = closestRangeDevice;
  return closest;
}

/** 
 *
 * The robot-relative positions of the readings of attached range 
 * devices, plus sonar readings stored in this object, will also be updated.
 *
 * @note This simply changes our stored pose value, it does not cause the robot
 * to drive. Use setVel(), setRotVel(), move(), setHeading(), setDeltaHeading(),
 * or the actions system.
 *
    @param pose New pose to set (in absolute world coordinates)
    @param doCumulative whether to update the cumulative buffers of range devices 
**/
MVREXPORT void MvrRobot::moveTo(MvrPose pose, bool doCumulative)
{
  std::list<MvrRangeDevice *>::iterator it;
  MvrSensorReading *son;
  int i;

  // we need to get this one now because changing the encoder
  // transform and global pose will change the local transform
  MvrTransform localTransform;
  localTransform = getToLocalTransform();

  myEncoderTransform.setTransform(myEncoderPose, pose);
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); it++)
  {
    (*it)->lockDevice();
    (*it)->applyTransform(localTransform, doCumulative);
    (*it)->applyTransform(getToGlobalTransform(), doCumulative);
    (*it)->unlockDevice();
  }

  for (i = 0; i < getNumSonar(); i++)
  {
    son = getSonarReading(i);
    if (son != NULL)
    {
      son->applyTransform(localTransform);
      son->applyTransform(getToGlobalTransform());
    }
  }

  //MvrLog::log(MvrLog::Normal, "Robot moved to %.0f %.0f %.1f", getX(), getY(), getTh());
}

/** 
 * The robot-relative positions of the readings of attached range 
 * devices, plus sonar readings stored in this object, will also be updated.
 * This variant allows you to manually specify a pose to use as the robot's 
 * old pose when updating range device readings (rather than MvrRobot's 
 * currently stored pose).
 *
 * @note This simply changes our stored pose value, it does not cause the robot
 * to drive. Use setVel(), setRotVel(), move(), setHeading(), setDeltaHeading(),
 * or the actions system.
 *
    @param poseTo the new absolute real world position 
    @param poseFrom the original absolute real world position
    @param doCumulative whether to update the cumulative buffers of range devices
**/
MVREXPORT void MvrRobot::moveTo(MvrPose poseTo, MvrPose poseFrom,
			      bool doCumulative)
{
  std::list<MvrRangeDevice *>::iterator it;
  MvrSensorReading *son;
  int i;

  MvrPose result = myEncoderTransform.doInvTransform(poseFrom);

  // we need to get this one now because changing the encoder
  // transform and global pose will change the local transform
  MvrTransform localTransform;
  localTransform = getToLocalTransform();

  myEncoderTransform.setTransform(result, poseTo);
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();

  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); it++)
  {
    (*it)->lockDevice();
    (*it)->applyTransform(localTransform, doCumulative);
    (*it)->applyTransform(getToGlobalTransform(), doCumulative);
    (*it)->unlockDevice();
  }

  for (i = 0; i < getNumSonar(); i++)
  {
    son = getSonarReading(i);
    if (son != NULL)
    {
      son->applyTransform(localTransform);
      son->applyTransform(getToGlobalTransform());
    }
  }

  //MvrLog::log(MvrLog::Normal, "Robot moved to %.0f %.0f %.1f", getX(), getY(), getTh());
}

/**
 * This transform is applied to all odometric/encoder poses received. 
 * If you simply want to transform the robot's final reported pose (as returned
 * by getPose()) to match an external coordinate system, use moveTo() instead.
 * @sa moveTo()
   @param deadReconPos the dead recon position to transform from
   @param globalPos the real world global position to transform to
**/
MVREXPORT void MvrRobot::setEncoderTransform(MvrPose deadReconPos, 
				    MvrPose globalPos)
{
  myEncoderTransform.setTransform(deadReconPos, globalPos);
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();
}

/**
 * This transform is applied to all odometric/encoder poses received. 
 * If you simply want to transform the robot's final reported pose (as returned
 * by getPose()) to match an external coordinate system, use moveTo() instead.
 * @sa moveTo()
   @param transformPos the position to transform to
**/
MVREXPORT void MvrRobot::setEncoderTransform(MvrPose transformPos)
{
  myEncoderTransform.setTransform(transformPos);
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();
}

/*
 * This transform is applied to all odometric/encoder poses received. 
 * If you simply want to transform the robot's final reported pose (as returned
 * by getPose()) to match an external coordinate system, use moveTo() instead.
 * @sa moveTo()
   @param transformPos the position to transform to
*/
MVREXPORT void MvrRobot::setEncoderTransform(MvrTransform transform)
{
  myEncoderTransform = transform;
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();
}

/**
   @return the transform from encoder to global coords
**/
MVREXPORT MvrTransform MvrRobot::getEncoderTransform(void) const
{
  return myEncoderTransform;
}

/**
   @param pose the position to set the dead recon position to
**/
MVREXPORT void MvrRobot::setDeadReconPose(MvrPose pose)
{
  myEncoderPose.setPose(pose);
  myEncoderTransform.setTransform(myEncoderPose, myGlobalPose);
  myGlobalPose = myEncoderTransform.doTransform(myEncoderPose);
  mySetEncoderTransformCBList.invoke();
}


/** 
    @return an MvrTransform which can be used for transforming a position
    in local coordinates to one in global coordinates
**/
MVREXPORT MvrTransform MvrRobot::getToGlobalTransform(void) const
{
  MvrTransform trans;
  MvrPose origin(0, 0, 0);
  MvrPose pose = getPose();

  trans.setTransform(origin, pose);
  return trans;
}

/** 
    @return an MvrTransform which can be used for transforming a position
    in global coordinates to one in local coordinates
**/
MVREXPORT MvrTransform MvrRobot::getToLocalTransform(void) const
{
  MvrTransform trans;
  MvrPose origin(0, 0, 0);
  MvrPose pose = getPose();

  trans.setTransform(pose, origin);
  return trans;
}

/** 
    Applies a transform to the range devices and sonar... 
    this is mostly useful for
    translating to/from local/global coords, but may have other uses
    @param trans the transform to apply
    @param doCumulative whether to transform the cumulative buffers or not
**/    
MVREXPORT void MvrRobot::applyTransform(MvrTransform trans, bool doCumulative)
{
  std::list<MvrRangeDevice *>::iterator it;
  MvrSensorReading *son;
  int i;
  
  for (it = myRangeDeviceList.begin(); it != myRangeDeviceList.end(); it++)
  {
      (*it)->lockDevice();
      (*it)->applyTransform(trans, doCumulative);
      (*it)->unlockDevice();
  }

  for (i = 0; i < getNumSonar(); i++)
  {
    son = getSonarReading(i);
    if (son != NULL)
      son->applyTransform(trans);
  }
}

MVREXPORT const char *MvrRobot::getName(void) const
{
  return myName.c_str();
}

MVREXPORT void MvrRobot::setName(const char * name)
{
	std::list<MvrRobot *> *robotList;
    std::list<MvrRobot *>::iterator it;
    int i;
    char buf[1024];
 
 if (name != NULL)
  {
    myName = name;
  }
  else
  {

    

    robotList = Mvria::getRobotList();
    for (i = 1, it = robotList->begin(); it != robotList->end(); it++, i++)
    {
      if (this == (*it))
      {
	    if (i == 1)
			myName = "robot";
		else
		{
			sprintf(buf, "robot%d", i);
			myName = buf;
		}
	    return;
      }
    }
    sprintf(buf, "robot%lu", robotList->size());
    myName = buf;
   }
}

/**
   This sets the encoderCorrectionCB, this callback returns the robots
   change in heading, it takes in the change in heading, x, and y,
   between the previous and current readings.  
   
   @param functor an MvrRetFunctor1 created as an MvrRetFunctor1C, that
   will be the callback... call this function NULL to clear the
   callback @see getEncoderCorrectionCallback
**/

MVREXPORT void MvrRobot::setEncoderCorrectionCallback(
	MvrRetFunctor1<double, MvrPoseWithTime> *functor)
{
  myEncoderCorrectionCB = functor;
}
/**
   This gets the encoderCorrectionCB, see setEncoderCorrectionCallback
   for details.

   @return the callback, or NULL if there isn't one
**/
MVREXPORT MvrRetFunctor1<double, MvrPoseWithTime> * 
MvrRobot::getEncoderCorrectionCallback(void) const
{
  return myEncoderCorrectionCB;
}

/**
   The direct motion precedence time determines how long actions will be 
   ignored after a direct motion command is given.  If the direct motion
   precedence time is 0, then direct motion will take precedence over actions 
   until a clearDirectMotion command is issued.  This value defaults to 0.
   @param mSec the number of milliseconds direct movement should trump actions, 
   if a negative number is given, then the value will be 0
   @see setDirectMotionPrecedenceTime
   @see clearDirectMotion
**/
MVREXPORT void MvrRobot::setDirectMotionPrecedenceTime(int mSec)
{
  if (mSec < 0) 
    myDirectPrecedenceTime = 0;
  else
    myDirectPrecedenceTime = mSec;
}

/**
   The direct motion precedence time determines how long actions will be 
   ignored after a direct motion command is given.  If the direct motion
   precedence time is 0, then direct motion will take precedence over actions 
   until a clearDirectMotion command is issued.  This value defaults to 0.
   @return the number of milliseconds direct movement will trump actions
   @see setDirectMotionPrecedenceTime
   @see clearDirectMotion
**/
MVREXPORT unsigned int MvrRobot::getDirectMotionPrecedenceTime(void) const
{
  return myDirectPrecedenceTime;
}


/**
   This clears the direct motion commands so that actions will be allowed to 
   control the robot again.
   @see setDirectMotionPrecedenceTime
   @see getDirectMotionPrecedenceTime
**/
MVREXPORT void MvrRobot::clearDirectMotion(void)
{
  myTransType = TRANS_NONE;
  myLastTransType = TRANS_NONE;
  myRotType = ROT_NONE;
  myLastRotType = ROT_NONE;
  myLatType = LAT_NONE;
  myLastLatType = LAT_NONE;
  myLastActionTransVal = 0;
  myLastActionRotStopped = true;
  myLastActionRotHeading = false;
  myLastActionLatVal = 0;
}

/**
   This stops the state reflection task from sending any motion
   commands to the robot (it still receives data from the robot, and it
   will still send the PULSE command).  If you later call clearDirectMotion(),
   then state reflection will be re-enabled, but the velocity values are reset
   to 0.  Similarly, if you later use direct motion commands,
   state reflection for those commands will then become active and those
   commands will be sent each cycle; other commands will remain disabled.
   For example, if you call stopStateReflection(), then later use setTransVel(),
   then translation velocity commands will be re-enabled and used in state
   reflection, but rotational velocity will remain disabled until you
   call setRotVel(), or call clearDirectMotion().
 **/
MVREXPORT void MvrRobot::stopStateReflection(void)
{
  myTransType = TRANS_IGNORE;
  myLastTransType = TRANS_IGNORE;
  myRotType = ROT_IGNORE;
  myLastRotType = ROT_IGNORE;
  myLatType = LAT_IGNORE;
  myLastLatType = LAT_IGNORE;
}

/**
   Returns the state of direct motion commands: whether actions are allowed or not
   @see clearDirectMotion
**/
MVREXPORT bool MvrRobot::isDirectMotion(void) const
{
  if (myTransType ==  TRANS_NONE && myLastTransType == TRANS_NONE &&
      myRotType == ROT_NONE && myLastRotType == ROT_NONE && 
      (!hasLatVel() || 
       (hasLatVel() && myLatType == LAT_NONE && myLastLatType == LAT_NONE)))
    return false;
  else
    return true;
}


/**
   This command enables the motors on the robot, if it is connected.
**/
MVREXPORT void MvrRobot::enableMotors()
{
  comInt(MvrCommands::ENABLE, 1);
}

/**
   This command disables the motors on the robot, if it is connected.
**/
MVREXPORT void MvrRobot::disableMotors()
{
  comInt(MvrCommands::ENABLE, 0);
}

/**
   This command enables the sonars on the robot, if it is connected.
**/
MVREXPORT void MvrRobot::enableSonar()
{
  mySonarEnabled = true;
  myAutonomousDrivingSonarEnabled = false;
  comInt(MvrCommands::SONAR, 1);
  
  int ii;
  for (ii = 1; ii <= Mvria::getMaxNumSonarBoards(); ii++)
  {
    MvrSonarMTX *sonarMTX = findSonar(ii);
    
    if (sonarMTX == NULL) 
      continue;

    sonarMTX->turnOnTransducers();
  }
}

/**
   This command enables the sonars on the robot, if it is connected.
**/
MVREXPORT void MvrRobot::enableAutonomousDrivingSonar()
{
  mySonarEnabled = false;
  myAutonomousDrivingSonarEnabled = true;
  comInt(MvrCommands::SONAR, 1);
  
  int ii;
  for (ii = 1; ii <= Mvria::getMaxNumSonarBoards(); ii++)
  {
    MvrSonarMTX *sonarMTX = findSonar(ii);
    
    if (sonarMTX == NULL) 
      continue;

    sonarMTX->disableForAutonomousDriving();
  }
}

/**
   This command disables the sonars on the robot, if it is connected.
**/
MVREXPORT void MvrRobot::disableSonar()
{
  mySonarEnabled = false;
  myAutonomousDrivingSonarEnabled = false;
  comInt(MvrCommands::SONAR, 0);
  
  int ii;
  for (ii = 1; ii <= Mvria::getMaxNumSonarBoards(); ii++)
  {
    MvrSonarMTX *sonarMTX = findSonar(ii);
    
    if (sonarMTX == NULL) 
      continue;

    sonarMTX->turnOffTransducers();
  }
}


/**
   The state reflection refresh time is the number of milliseconds between 
   when the state reflector will refresh the robot, if the command hasn't 
   changed.  The default is 500 milliseconds.  If this number is less than 
   the cyle time, it'll simply happen every cycle.
   @param mSec the refresh time, in milliseconds, non-negative, if negative is 
   given, then the value will be 0
**/
MVREXPORT void MvrRobot::setStateReflectionRefreshTime(int mSec)
{
  if (mSec < 0)
    myStateReflectionRefreshTime = 0;
  else
    myStateReflectionRefreshTime = mSec;
}

/**
   The state reflection refresh time is the number of milliseconds between 
   when the state reflector will refresh the robot, if the command hasn't 
   changed.  The default is 500 milliseconds.  If this number is less than 
   the cyle time, it'll simply happen every cycle.
   @return the state reflection refresh time
**/
MVREXPORT int MvrRobot::getStateReflectionRefreshTime(void) const
{
  return myStateReflectionRefreshTime;
}

/**
   This will attach a key handler to a robot, by putting it into the
   robots sensor interp task list (a keyboards a sensor of users will,
   right?).  By default exitOnEscape is true, which will cause this
   function to add an escape key handler to the key handler, this will
   make the program exit when escape is pressed... if you don't like
   this you can pass exitOnEscape in as false.

   @param keyHandler the key handler to attach

   @param exitOnEscape whether to exit when escape is pressed or not
   
   @param useExitNotShutdown if true then Mvria::exit will be called
   instead of Mvria::shutdown if it tries to exit
**/
MVREXPORT void MvrRobot::attachKeyHandler(MvrKeyHandler *keyHandler, 
					bool exitOnEscape, 
					bool useExitNotShutdown)
{
  if (myKeyHandlerCB != NULL)
    delete myKeyHandlerCB;
  myKeyHandlerCB = new MvrFunctorC<MvrKeyHandler>(keyHandler, 
					    &MvrKeyHandler::checkKeys);
  addSensorInterpTask("Key Handler", 50, myKeyHandlerCB);

  myKeyHandler = keyHandler;
  myKeyHandlerUseExitNotShutdown = useExitNotShutdown;
  if (exitOnEscape)
    keyHandler->addKeyHandler(MvrKeyHandler::ESCAPE, &myKeyHandlerExitCB);
}

MVREXPORT MvrKeyHandler *MvrRobot::getKeyHandler(void) const
{
  return myKeyHandler;
}

MVREXPORT void MvrRobot::keyHandlerExit(void)
{
  MvrLog::log(MvrLog::Terse, "Escape was pressed, program is exiting.");
  // if we're using exit not the keyhandler then call Mvria::exit
  // instead of shutdown, this call never returns
  if (myKeyHandlerUseExitNotShutdown)
    Mvria::exit();
  stopRunning();
  unlock();
  Mvria::shutdown();
}

MVREXPORT void MvrRobot::setPacketsReceivedTracking(bool packetsReceivedTracking)
{
	if (packetsReceivedTracking)
      MvrLog::log(MvrLog::Normal, "MvrRobot: tracking packets received");
	else
      MvrLog::log(MvrLog::Normal, "MvrRobot: not tracking packets received");

  myPacketsReceivedTracking = packetsReceivedTracking;
	myReceiver.setTracking(myPacketsReceivedTracking);
	myReceiver.setTrackingLogName("MicroController");
  myPacketsReceivedTrackingCount = 0; 
  myPacketsReceivedTrackingStarted.setToNow(); 
}

MVREXPORT void MvrRobot::ariaExitCallback(void)
{
  mySyncLoop.stopRunIfNotConnected(false);
  disconnect();
}

/**
   @note only available on robots with an firwmare
   operating system (ARCOS or uARCS) thats at least newer than July of
   2005.  It'll just be CHARGING_UNKNOWN on everything else always.

   @note Some robots don't actively manage their own charge state (Pioneer 3, Amigo),
   instead this is done by the external charger hardware. ChargeState is most 
   revelant on uARCS systems (i.e. PatrolBot).
   If the firmware for the Pioneer based robots is newer and supports charge state,
   charge state will just always be CHARGING_NOT. If the firmware on these robots is
   older and does not support, it, this function will return CHARGING_UNKNOWN.
   (on these robots, you might be able to guess what the charger is doing based on 
   reported battery voltage, but this is not a direct measure.)
 **/
MVREXPORT MvrRobot::ChargeState MvrRobot::getChargeState(void) const
{
  return myChargeState;
}

/**
   
 **/
MVREXPORT void MvrRobot::setChargeState(MvrRobot::ChargeState chargeState) 
{
  myOverriddenChargeState = true;
  myChargeState = chargeState;
}

MVREXPORT bool MvrRobot::isChargerPowerGood(void) const
{
  if (myOverriddenIsChargerPowerGood)
    return myIsChargerPowerGood;
  else
    return (getFlags() & MvrUtil::BIT10);
}

MVREXPORT void MvrRobot::setIsChargerPowerGood(bool isChargerPowerGood) 
{
  myOverriddenIsChargerPowerGood = true;
  myIsChargerPowerGood = isChargerPowerGood;
}

MVREXPORT void MvrRobot::resetTripOdometer(void)
{
  myTripOdometerDistance = 0;
  myTripOdometerDegrees = 0;
  myTripOdometerStart.setToNow();
}

MVREXPORT void MvrRobot::setStateOfCharge(double stateOfCharge)
{
  myHaveStateOfCharge = true;
  myStateOfCharge = stateOfCharge;
  myStateOfChargeSetTime.setToNow();
}

MVREXPORT void MvrRobot::setIgnoreMicroControllerBatteryInfo(
	bool ignoreMicroControllerBatteryInfo)
{
  if (myIgnoreMicroControllerBatteryInfo != ignoreMicroControllerBatteryInfo)
  {
    if (ignoreMicroControllerBatteryInfo)
      MvrLog::log(MvrLog::Normal, "Ignoring battery info from the microcontroller");
    else
      MvrLog::log(MvrLog::Normal, "Not ignoring battery info from the microcontroller");
  }
  myIgnoreMicroControllerBatteryInfo = ignoreMicroControllerBatteryInfo;
}

MVREXPORT void MvrRobot::setBatteryInfo(double realBatteryVoltage, 
				      double normalizedBatteryVoltage,
				      bool haveStateOfCharge,
				      double stateOfCharge)
{
  myRealBatteryVoltage = realBatteryVoltage;
  myRealBatteryAverager.add(myRealBatteryVoltage);

  myBatteryVoltage = normalizedBatteryVoltage;
  myBatteryAverager.add(myBatteryVoltage);

  if (!myHaveStateOfCharge && haveStateOfCharge)
    myHaveStateOfCharge = true;
  if (haveStateOfCharge)
  {
    myStateOfCharge = stateOfCharge;
    myStateOfChargeSetTime.setToNow();
  }  
}


/*
  @note Do not call this method directly 
  if using MvrLaserConnector, it will automatically add laser(s).
  @internal
*/
MVREXPORT bool MvrRobot::addLaser(MvrLaser *laser, int laserNumber, 
				bool addAsRangeDevice)
{
  std::map<int, MvrLaser *>::iterator it;
  if (laser == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::addLaser: Tried to add NULL laser as laser number %d",
	       laserNumber);
    return false;
  }
  if ((it = myLaserMap.find(laserNumber)) != myLaserMap.end())
  {
    if ((*it).second == laser)
    {
      MvrLog::log(MvrLog::Verbose, 
		 "Tried to add laser %s as number %d (and as a range device) but already have that laser, doing nothing",
		 laser->getName(), laserNumber);    
      return true;
    }
    MvrLog::log(MvrLog::Normal, "MvrRobot::addLaser: Tried to add laser %s as laser number %d but there is already a laser of that number (called %s)",
	       laser->getName(), laserNumber,
	       (*it).second->getName());
    return false;
  }
  myLaserMap[laserNumber] = laser;
  if (addAsRangeDevice)
  {
    MvrLog::log(MvrLog::Verbose, 
	       "Added laser %s as number %d (and as a range device)",
	       laser->getName(), laserNumber);    
    remRangeDevice(laser);
    addRangeDevice(laser);
  }
  else
  {
    MvrLog::log(MvrLog::Verbose, 
	       "Added laser %s as number %d (but not a range device)",
	       laser->getName(), laserNumber);    
  }
  return true;
}



/*
    @internal
*/
MVREXPORT bool MvrRobot::remLaser(MvrLaser *laser, bool removeAsRangeDevice)
{
  if (laser == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLaser: Passed NULL laser to remove");
      return false;
  }
  std::map<int, MvrLaser *>::iterator it;
  for (it = myLaserMap.begin(); it != myLaserMap.end(); ++it)
  {
    if ((*it).second == laser)
    {
      if (removeAsRangeDevice)
      {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::remLaser: Removing laser %s (num %d) (and removing as range device)", 
		   laser->getName(), (*it).first);
	remRangeDevice((*it).second);
      }
      else 
      {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::remLaser: Removing laser %s (num %d) (will not remove as range device)", 
		   laser->getName(), (*it).first);  
      }
      myLaserMap.erase(it);
      return true;
    }
  }
  MvrLog::log(MvrLog::Normal, 
	     "MvrRobot::remLaser: Could not find laser %s to remove", 
	     laser->getName());
  return false;
}

/*
    @internal
*/
MVREXPORT bool MvrRobot::remLaser(int laserNumber, bool removeAsRangeDevice)
{
  std::map<int, MvrLaser *>::iterator it;
  if ((it = myLaserMap.find(laserNumber)) == myLaserMap.end())
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLaser: Could not find laser %d to remove", 
	       laserNumber);
    return false;
  
  }

  if (removeAsRangeDevice)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLaser: Removing laser %s (num %d) (and removing as range device)", 
	       (*it).second->getName(), (*it).first);  
    remRangeDevice((*it).second);
  }
  else
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLaser: Removing laser %s (num %d) (will not remove as range device)", 
	       (*it).second->getName(), (*it).first);  
  }
    
  myLaserMap.erase(it);
  return true;
}

/*
  @see MvrLaserConnector
*/
MVREXPORT const MvrLaser *MvrRobot::findLaser(int laserNumber) const
{
  std::map<int, MvrLaser *>::const_iterator it;
  if ((it = myLaserMap.find(laserNumber)) == myLaserMap.end())
    return NULL;
  else
    return (*it).second;
}

/* 
    @see MvrLaserConnector
*/
MVREXPORT MvrLaser *MvrRobot::findLaser(int laserNumber)
{
  if (myLaserMap.find(laserNumber) == myLaserMap.end())
    return NULL;
  else
    return myLaserMap[laserNumber];
}

/* 
    @see MvrLaserConnector
*/
MVREXPORT const std::map<int, MvrLaser *> *MvrRobot::getLaserMap(void) const
{
  return &myLaserMap;
}


/* 
    @see MvrLaserConnector
*/
MVREXPORT std::map<int, MvrLaser *> *MvrRobot::getLaserMap(void) 
{
  return &myLaserMap;
}

/* 
    @see MvrLaserConnector
*/
MVREXPORT bool MvrRobot::hasLaser(MvrLaser *device) const
{
  for(std::map<int, MvrLaser*>::const_iterator i = myLaserMap.begin();
        i != myLaserMap.end(); ++i)
  {
     if( (*i).second == device ) return true;
  }
  return false;
}


/* 
  @note Do not call this method directly 
  if using MvrBatteryConnector, it will automatically add battery(s).
  @internal
*/
MVREXPORT bool MvrRobot::addBattery(MvrBatteryMTX *battery, int batteryNumber)
{
  std::map<int, MvrBatteryMTX *>::iterator it;
  if (battery == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::addBattery: Tried to add NULL battery as battery number %d",
	       batteryNumber);
    return false;
  }
  if ((it = myBatteryMap.find(batteryNumber)) != myBatteryMap.end())
  {
    if ((*it).second == battery)
    {
      MvrLog::log(MvrLog::Verbose, 
		 "Tried to add battery %s as number %d but already have that battery, doing nothing",
		 battery->getName(), batteryNumber);    
      return true;
    }
    MvrLog::log(MvrLog::Normal, "MvrRobot::addBattery: Tried to add battery %s as battery number %d but there is already a battery of that number (called %s)",
	       battery->getName(), batteryNumber,
	       (*it).second->getName());
    return false;
  }
  myBatteryMap[batteryNumber] = battery;
  MvrLog::log(MvrLog::Verbose, 
	       "Added battery %s as number %d",
	       battery->getName(), batteryNumber);    
  return true;
}


/* 
    @internal
*/
MVREXPORT bool MvrRobot::remBattery(MvrBatteryMTX *battery)
{
  if (battery == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remBattery: Passed NULL battery to remove");
      return false;
  }
  std::map<int, MvrBatteryMTX *>::iterator it;
  for (it = myBatteryMap.begin(); it != myBatteryMap.end(); ++it)
  {
    if ((*it).second == battery)
    {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::remBattery: Removing battery %s (num %d)", 
		   battery->getName(), (*it).first);  
      myBatteryMap.erase(it);
      return true;
    }
  }
  MvrLog::log(MvrLog::Normal, 
	     "MvrRobot::remBattery: Could not find battery %s to remove", 
	     battery->getName());
  return false;
}

/* 
    @internal
*/
MVREXPORT bool MvrRobot::remBattery(int batteryNumber)
{
  std::map<int, MvrBatteryMTX *>::iterator it;
  if ((it = myBatteryMap.find(batteryNumber)) == myBatteryMap.end())
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remBattery: Could not find battery %d to remove", 
	       batteryNumber);
    return false;
  
  }

    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remBattery: Removing battery %s (num %d) ", 
	       (*it).second->getName(), (*it).first);  
    
  myBatteryMap.erase(it);
  return true;
}

/* 
  @see MvrBatteryConnector
*/
MVREXPORT const MvrBatteryMTX *MvrRobot::findBattery(int batteryNumber) const
{
  std::map<int, MvrBatteryMTX *>::const_iterator it;
  if ((it = myBatteryMap.find(batteryNumber)) == myBatteryMap.end())
    return NULL;
  else
    return (*it).second;
}

/* 
    @see MvrBatteryConnector
*/
MVREXPORT MvrBatteryMTX *MvrRobot::findBattery(int batteryNumber)
{
  if (myBatteryMap.find(batteryNumber) == myBatteryMap.end())
    return NULL;
  else
    return myBatteryMap[batteryNumber];
}

/* 
    @see MvrBatteryConnector
*/
MVREXPORT const std::map<int, MvrBatteryMTX *> *MvrRobot::getBatteryMap(void) const
{
  return &myBatteryMap;
}


/* 
    @see MvrBatteryConnector
*/
MVREXPORT std::map<int, MvrBatteryMTX *> *MvrRobot::getBatteryMap(void) 
{
  return &myBatteryMap;
}

/* 
    @see MvrBatteryConnector
*/
MVREXPORT bool MvrRobot::hasBattery(MvrBatteryMTX *device) const
{
  for(std::map<int, MvrBatteryMTX *>::const_iterator i = myBatteryMap.begin();
        i != myBatteryMap.end(); ++i)
  {
     if( (*i).second == device ) return true;
  }
  return false;
}


/* 
  @note Do not call this method directly 
  if using MvrLCDConnector, it will automatically add lcd(s).
  @internal
*/
MVREXPORT bool MvrRobot::addLCD(MvrLCDMTX *lcd, int lcdNumber)
{
  std::map<int, MvrLCDMTX *>::iterator it;
  if (lcd == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::addLCD: Tried to add NULL lcd as lcd number %d",
	       lcdNumber);
    return false;
  }
  if ((it = myLCDMap.find(lcdNumber)) != myLCDMap.end())
  {
    if ((*it).second == lcd)
    {
      MvrLog::log(MvrLog::Verbose, 
		 "Tried to add lcd %s as number %d but already have that lcd, doing nothing",
		 lcd->getName(), lcdNumber);    
      return true;
    }
    MvrLog::log(MvrLog::Normal, "MvrRobot::addLCD: Tried to add lcd %s as lcd number %d but there is already a lcd of that number (called %s)",
	       lcd->getName(), lcdNumber,
	       (*it).second->getName());
    return false;
  }
  myLCDMap[lcdNumber] = lcd;
  MvrLog::log(MvrLog::Verbose, 
	       "Added lcd %s as number %d",
	       lcd->getName(), lcdNumber);    
  return true;
}


/* 
    @internal
*/
MVREXPORT bool MvrRobot::remLCD(MvrLCDMTX *lcd)
{
  if (lcd == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLCD: Passed NULL lcd to remove");
      return false;
  }
  std::map<int, MvrLCDMTX *>::iterator it;
  for (it = myLCDMap.begin(); it != myLCDMap.end(); ++it)
  {
    if ((*it).second == lcd)
    {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::remLCD: Removing lcd %s (num %d)", 
		   lcd->getName(), (*it).first);  
      myLCDMap.erase(it);
      return true;
    }
  }
  MvrLog::log(MvrLog::Normal, 
	     "MvrRobot::remLCD: Could not find lcd %s to remove", 
	     lcd->getName());
  return false;
}

/* 
    @internal
*/
MVREXPORT bool MvrRobot::remLCD(int lcdNumber)
{
  std::map<int, MvrLCDMTX *>::iterator it;
  if ((it = myLCDMap.find(lcdNumber)) == myLCDMap.end())
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLCD: Could not find lcd %d to remove", 
	       lcdNumber);
    return false;
  
  }

    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remLCD: Removing lcd %s (num %d) ", 
	       (*it).second->getName(), (*it).first);  
    
  myLCDMap.erase(it);
  return true;
}

/* 
  @see MvrLCDConnector
*/
MVREXPORT const MvrLCDMTX *MvrRobot::findLCD(int lcdNumber) const
{
  std::map<int, MvrLCDMTX *>::const_iterator it;
  if ((it = myLCDMap.find(lcdNumber)) == myLCDMap.end())
    return NULL;
  else
    return (*it).second;
}

/* 
    @see MvrLCDConnector
*/
MVREXPORT MvrLCDMTX *MvrRobot::findLCD(int lcdNumber)
{
  if (myLCDMap.find(lcdNumber) == myLCDMap.end())
    return NULL;
  else
    return myLCDMap[lcdNumber];
}

/* 
    @see MvrLCDConnector
*/
MVREXPORT const std::map<int, MvrLCDMTX *> *MvrRobot::getLCDMap(void) const
{
  return &myLCDMap;
}


/* 
    @see MvrLCDConnector
*/
MVREXPORT std::map<int, MvrLCDMTX *> *MvrRobot::getLCDMap(void) 
{
  return &myLCDMap;
}

/* 
    @see MvrLCDConnector
*/
MVREXPORT bool MvrRobot::hasLCD(MvrLCDMTX *device) const
{
  for(std::map<int, MvrLCDMTX *>::const_iterator i = myLCDMap.begin();
        i != myLCDMap.end(); ++i)
  {
     if( (*i).second == device ) return true;
  }
  return false;
}

/* 
  @note Do not call this method directly 
  if using MvrBatteryConnector, it will automatically add battery(s).
  @internal
*/
MVREXPORT bool MvrRobot::addSonar(MvrSonarMTX *sonar, int sonarNumber)
{
  std::map<int, MvrSonarMTX *>::iterator it;
  if (sonar == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::addSonar: Tried to add NULL sonar board as sonar number %d",
	       sonarNumber);
    return false;
  }
  if ((it = mySonarMap.find(sonarNumber)) != mySonarMap.end())
  {
    if ((*it).second == sonar)
    {
      MvrLog::log(MvrLog::Verbose, 
		 "Tried to add sonar board %s as number %d but already have that sonar, doing nothing",
		 sonar->getName(), sonarNumber);    
      return true;
    }
    MvrLog::log(MvrLog::Normal, "MvrRobot::addSonar: Tried to add sonar board %s as sonar number %d but there is already a sonar of that number (called %s)",
	       sonar->getName(), sonarNumber,
	       (*it).second->getName());
    return false;
  }
  mySonarMap[sonarNumber] = sonar;
  MvrLog::log(MvrLog::Verbose, 
	       "Added sonar board %s as number %d",
	       sonar->getName(), sonarNumber);    
  return true;
}


/* 
    @internal
*/
MVREXPORT bool MvrRobot::remSonar(MvrSonarMTX *sonar)
{
  if (sonar == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remSonar: Passed NULL sonar board to remove");
      return false;
  }
  std::map<int, MvrSonarMTX *>::iterator it;
  for (it = mySonarMap.begin(); it != mySonarMap.end(); ++it)
  {
    if ((*it).second == sonar)
    {
	MvrLog::log(MvrLog::Normal, 
		   "MvrRobot::remSonar: Removing sonar board %s (num %d)", 
		   sonar->getName(), (*it).first);  
      mySonarMap.erase(it);
      return true;
    }
  }
  MvrLog::log(MvrLog::Normal, 
	     "MvrRobot::remSonar: Could not find sonar board %s to remove", 
	     sonar->getName());
  return false;
}

/* 
    @internal
*/
MVREXPORT bool MvrRobot::remSonar(int sonarNumber)
{
  std::map<int, MvrSonarMTX *>::iterator it;
  if ((it = mySonarMap.find(sonarNumber)) == mySonarMap.end())
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remSonar: Could not find sonar board %d to remove", 
	       sonarNumber);
    return false;
  
  }

    MvrLog::log(MvrLog::Normal, 
	       "MvrRobot::remSonar: Removing sonar board %s (num %d) ", 
	       (*it).second->getName(), (*it).first);  
    
  mySonarMap.erase(it);
  return true;
}

/* 
  @see MvrSonarConnector
*/
MVREXPORT const MvrSonarMTX *MvrRobot::findSonar(int sonarNumber) const
{
  std::map<int, MvrSonarMTX *>::const_iterator it;
  if ((it = mySonarMap.find(sonarNumber)) == mySonarMap.end())
    return NULL;
  else
    return (*it).second;
}

/* 
    @see MvrSonarConnector
*/
MVREXPORT MvrSonarMTX *MvrRobot::findSonar(int sonarNumber)
{
  if (mySonarMap.find(sonarNumber) == mySonarMap.end())
    return NULL;
  else
    return mySonarMap[sonarNumber];
}

/* 
    @see MvrSonarConnector
*/
MVREXPORT const std::map<int, MvrSonarMTX *> *MvrRobot::getSonarMap(void) const
{
  return &mySonarMap;
}


/* 
    @see MvrSonarConnector
*/
MVREXPORT std::map<int, MvrSonarMTX *> *MvrRobot::getSonarMap(void) 
{
  return &mySonarMap;
}

/* 
    @see MvrSonarConnector
*/
MVREXPORT bool MvrRobot::hasSonar(MvrSonarMTX *device) const
{
  for(std::map<int, MvrSonarMTX *>::const_iterator i = mySonarMap.begin();
        i != mySonarMap.end(); ++i)
  {
     if( (*i).second == device ) return true;
  }
  return false;
}



/**
   @param from the pose with time to apply the offset from... it applies the encoder mostion from the time on this to the to param
   
   @param to the time to find the offset to

   @param result the pose to put the result in

   @return This returns the same way that MvrInterpolation::getPose does...
@see MvrInterpolation::getPose
*/
MVREXPORT int MvrRobot::applyEncoderOffset(MvrPoseWithTime from, MvrTime to, 
					 MvrPose *result)
{

  MvrPose encoderPoseFrom;
  MvrPose globalPoseFrom;
  
  MvrPose encoderPoseTo;
  MvrPose globalPoseTo;

  int retFrom;
  int retTo;
  if ((retFrom = getEncoderPoseInterpPosition(from.getTime(), 
					     &encoderPoseFrom)) >= 0)
  {
    if ((retTo = getEncoderPoseInterpPosition(to, &encoderPoseTo)) >= 0)
    {
      globalPoseFrom = myEncoderTransform.doTransform(encoderPoseFrom);
      globalPoseTo = myEncoderTransform.doTransform(encoderPoseTo);
      MvrPose offset = globalPoseTo - globalPoseFrom;
      MvrPose retPose;
      retPose = from + offset;
      /*
      MvrLog::log(MvrLog::Normal, 
		 "%.0f %.0f %.1f + %.0f %.0f %.1f = %.0f %.0f %.1f (from %d to %d)", 
		 from.getX(), from.getY(), from.getTh(), 
		 offset.getX(), offset.getY(), offset.getTh(),
		 retPose.getX(), retPose.getY(), retPose.getTh(),
		 retFrom, retTo);
      */
      if (result != NULL)
	*result = retPose;
      return retTo;
    }
    else
    {
      MvrLog::log(MvrLog::Verbose, 
		 "MvrRobot::applyEncoderOffset: Can't find to offset, ret %d",
		 retTo);
      return retTo;
    }
  }
  else
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrRobot::applyEncoderOffset: Can't find from (%d ms ago) offset, ret %d",
	       retFrom, from.getTime().mSecSince());
    return retFrom;
  }

  return -3;
}

MVREXPORT void MvrRobot::internalIgnoreNextPacket(void)
{
  myIgnoreNextPacket = true;
  myPacketMutex.lock();
  myPacketList.clear();
  myPacketMutex.unlock();
}
