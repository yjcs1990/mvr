/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobot.h
 > Description  : Central class for communicating with and operating the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRROBOT_H
#define MVRROBOT_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacketSender.h"
#include "MvrRobotPacketReceiver.h"
#include "MvrFunctor.h"
#include "MvrSyncTask.h"
#include "MvrSensorReading.h"
#include "MvrMutex.h"
#include "MvrCondition.h"
#include "MvrSyncLoop.h"
#include "MvrRobotPacketReaderThread.h"
#include "MvrRobotParams.h"
#include "MvrActionDesired.h"
#include "MvrResolver.h"
#include "MvrTransform.h"
#include "MvrInterpolation.h"
#include "MvrKeyHandler.h"
#include <list>

class MvrAction;
class MvrRobotConfigPacketReader;
class MvrRobotBatteryPacketReader;
class MvrDeviceConnection;
class MvrRangeDevice;
class MvrRobotPacket;
class MvrPTZ;
class MvrLaser;
class MvrBatteryMTX;
class MvrSonarMTX;
class MvrLCDMTX;

/* 
    This is the most important class. It is used to communicate with
    the robot by sending commands and retrieving data (including
    wheel odometry, digital and analog inputs, sonar data, and more). 
    It is also used
    to provide access to objects for controlling attached accessories, 
    MvrRangeDevice objects, MvrAction objects, and others.  For details
    on usage, and how the task cycle and obot state synchronization works,
    see the @ref robot "MvrRobot section" and the
    @ref ClientCommands "Commands and Actions section" of the ARIA overview.

    @note In Windows you cannot make an MvrRobot object a global variable, 
    it will crash because the compiler initializes the constructors in
    the wrong order. You can, however, make a pointer to an MvrRobot and then
    allocate it with 'new' at program start.

    @see MvrRobotConnector

    @ingroup ImportantClasses
    @ingroup DeviceClasses
*/
class MvrRobot
{
public:
  typedef enum {
    WAIT_CONNECTED,   /// < The robot has connect
    WAIT_FAILED_CONN, /// < The robot failed to connect
    WAIT_RUN_EXIT,    /// < The run loop has exited
    WAIT_TIMEDOUT,    /// < The wait reached the timeout specified
    WAIT_INTR,        /// < The wait was interupted by a signal
    WAIT_FAIL         /// < The wait failed due to an error
  }WaitState;
  enum ChargeState {
    CHARGING_UNKNOWN=-1,
    CHARGING_NOT=0,
    CHARGING_BULK=1,
    CHARGING_OVERCHARGE=2,
    CHARGING_FLOAT=3,
    CHARGING_BALANCE=4
  };
  /// Constructor
  MVREXPORT MvrRobot(const char *name=NULL, bool ignored=true,
                     bool doSigHandle=true, bool normalInit=true,
                     bool addMvriaExitCallback=true);
  /// Destructor
  MVREXPORT ~MvrRobot();

  /// Starts the instance to do processing in this thread
  MVREXPORT void run(bool stopRunIfNotConnnected, bool runNonThreaded=false);
  /// Starts the instance to do processing in its own new thread
  MVREXPORT void runAsync(bool stopRunIfNotConnnected, bool runNonThreadedPacketReader=false);

  /// @copydoc run(bool, bool)
  /// @ingroup easy
  void run() { run(true); }

  /// @copydoc runAsync(bool, bool)
  /// @ingroup easy
  void runAsync() { runAsync(true); }

  /// Return whether the robot is currently running or not
  /// @ingroup easy
  MVREXPORT bool isRunning(void) const;

  /// Stops the robot from donig any more processing
  /// @ingroup easy
  MVREXPORT void stopRunnint(bool doDisconnect=true);

  /// Sets the connection this instance uses
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *connection);
  /// Gets the connection this instance uses
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void) const;

  /// Questions whether the robot is connected or not
  /*
   @return true if connected to a robot, false if not
   @ingroup easy
  */
  bool isConnected(void) const { return myIsConnected; }
  /// Connects to a robot, not returning until connection made or failed
  MVREXPORT bool blockingConnect(void);
  /// Connects to a robot, from the robots own thread
  MVREXPORT bool asyncConnect(void);
  /// Disconnects from a robot
  MVREXPORT bool disconnect(void);

  /// Clears what direct motion commands have been given ,so actions work
  MVREXPORT void clearDirectMotion(void);
  /// Returns true if direct motion commands are blocking actions
  MVREXPORT bool isDerectMotion(void) const;

  /// Sets the state reflection to be inactive (until motion or clearDirectMotion)
  /// @see clearDirectMotion
  MVREXPORT void stopStateReflection(void);

  /// Enable the motors on the robot
  /// @ingroup easy
  MVREXPORT void enableMotors(void);
  /// Disable the motors on the robot
  MVREXPORT void disableMotors(void);

  /// Enables the sonar on the robot
  MVREXPORT void enableSonar(void);
  /// Enables some of the sonar on the robot(the ones for automous driving)
  MVREXPORT void enableAutonomousDrivingSonar(void);
  /// Disable the sonar on the robot
  MVREXPORT void disableSonar(void);

  /// Stops the robot
  /// @see clearDirectMotion
  MVREXPORT void stop(void);
  /// Sets the velocity
  MVREXPORT void setVel(double velocity);
  /// Sets differential movement if implemented by robot
  MVREXPORT void setVel2(double leftVelocity, double rightVelocity);
  /// Move the givebn distance forward/backwards
  /// @ingroup easy
  MVREXPORT void move(double distance);
  /// Sees if the robot is done moving the previously given more
  MVREXPORT bool isMoveDone(double delta=0.0);
  /// Sets the difference required for being done with a move
  void setMoveDoneDist(double dist) { myMoveDoneDist = dist; }
  /// Gets the difference required for being done with a move
  double getMoveDoneDist(void) { return myMoveDoneDist; }
  /// Sets the heading
  MVREXPORT void setHeading(double heading);
  /// Sets the rotational velocity
  MVREXPORT void setRotVel(double velocity);
  /// Sets the delta heading
  MVREXPORT void setDeltaHeading(double deltaHeading);
  /// Sees if the robot is done changing to the previously given setHeading
  MVREXPORT bool isHeadingDone(double delta=0.0) const;
  /// Sets the difference required for being done with a heading change(e.g. used in isHeadingDone())
  void setHeadingDoneDiff(double degree)
   { myHeadingDoneDiff=degrees; }
  /// Gets the difference required for being done with a heaing change(e.g. used in isHeadingDone())
  double getHeadingDoneDiff(void) const { return myHeadingDoneDiff; }
  /// Sets the lateral velocity
  MVREXPORT void setLatVel(double latVelocity);

  /// sees if we stopped
  MVREXPORT bool isStopped(double stoppedVel=0.0, double stoppedRotVel=0.0, double stoppedLatVel=0.0);

  /// Sets the vels required to be stopped
  MVREXPORT void setStoppedVel(double stoppedVel, double stoppedRotVel, double stoppedLatVel);

  /// Sets the length of time a direct motion command will take precedence
  /// over actions, in milliseconds
  MVREXPORT void setDirectMotionPrecedenceTime(int mSec);

  /// Gets the length of time a direct motion command will take precedence
  /// over actions, in milliseconds
  MVREXPORT unsigned int getDirectMotionPrecedenceTime(void) const;

  /// Sends a command to the robot with no argument
  MVREXPORT bool com(unsigned char command);
  /// Sends a command to the robot with an int for argument
  MVREXPORT bool comInt(unsigned char command, short int argument);
  /// Sends a command to the robot with two bytes for argument
  MVREXPORT bool com2Bytes(unsigned char command, char high, char low);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStr(unsigned char command, const char *argument);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStrN(unsigned char command, const char *std, int  size);
  /// Sends a command containing exactly the data in the given buffer as argument
  MVREXPORT bool comDataN(unsigned char command, const char *data, int size);

  /// Returns the robot's name that is set in its onboard firmware configuration
  const char *getRobotName(void) const { return myRobotName.c_str(); }
  /// Returns the type of the robot we are currently connected to 
  const char *getRobotType(void) const { return myRobotType.c_str(); }
  /// Returns the subtype of the robot we are currently connected to 
  const char *getRobotsubType(void) const { return myRobotsubType.c_str(); }

  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransVel(void) const;
   { return myAbsoluteMaxTransVel; }
  /// Sets the robot's absolute maximum translational velocity
  MVREXPORT bool setAbsoluteMaxTransVel(double maxVel);
  
  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransNegVel(void) const;
   { return myAbsoluteMaxTransNegVel; }
  /// Sets the robot's absolute maximum translational velocity
  MVREXPORT bool setAbsoluteMaxTransNegVel(double maxVel);

  /// Gets the robot's absolute maximum translational acceleration
  double getAbsoluteMaxTransAccel(void) const
   { return myAbsoluteMaxTransAccel; }
  /// Sets the robot's absolute maximum translational acceleration
  MVREXPORT bool setAbsoluteMaxTransAccel(double maxAccel);

  /// Gets the robot's absolute maximum translational deceleration
  double getAbsoluteMaxTransDecel(void) const
   { return myAbsoluteMaxTransDecel; }
  /// Sets the robot's absolute maximum translational deceleration
  MVREXPORT bool setAbsoluteMaxTransDecel(double maxDecel);

  /// Gets the robot's absolute maximum rotational velocity
  double getAbsoluteMaxRotVel(void) const;
   { return myAbsoluteMaxRotVel; }
  /// Sets the robot's absolute maximum rotational velocity
  MVREXPORT bool setAbsoluteMaxRotVel(double maxVel);

  /// Gets the robot's absolute maximum rotational acceleration
  double getAbsoluteMaxRotAccel(void) const
   { return myAbsoluteMaxRotAccel; }
  /// Sets the robot's absolute maximum rotational acceleration
  MVREXPORT bool setAbsoluteMaxRotAccel(double maxAccel);

  /// Gets the robot's absolute maximum rotational deceleration
  double getAbsoluteMaxRotDecel(void) const
   { return myAbsoluteMaxRotDecel; }
  /// Sets the robot's absolute maximum rotational deceleration
  MVREXPORT bool setAbsoluteMaxRotDecel(double maxDecel);

  /// Gets the robot's absolute maximum lateral velocity
  double getAbsoluteMaxLatVel(void) const;
   { return myAbsoluteMaxLatVel; }
  /// Sets the robot's absolute maximum lateral velocity
  MVREXPORT bool setAbsoluteMaxLatVel(double maxVel);

  /// Gets the robot's absolute maximum lateral acceleration
  double getAbsoluteMaxLatAccel(void) const
   { return myAbsoluteMaxLatAccel; }
  /// Sets the robot's absolute maximum lateral acceleration
  MVREXPORT bool setAbsoluteMaxLatAccel(double maxAccel);

  /// Gets the robot's absolute maximum lateral deceleration
  double getAbsoluteMaxLatDecel(void) const
   { return myAbsoluteMaxLatDecel; }
  /// Sets the robot's absolute maximum rotational deceleration
  MVREXPORT bool setAbsoluteMaxLatDecel(double maxDecel);

  /// Accessors
  /*
   * @brief Get the current stored global position of the robot
   * This position is updated by data reported by the robot as it 
   * moves, and may also be changed by other program components,
   * such as localization process (see moveTo())
   *
   * This position is also referred to as the robot's
   * "odometry" or "odometric" pose, since the robot
   * uses its odometry data to determine this pose; but it
   * may also incorporate additional data sources such as
   * an onboard gyro. The term "odometric pose" also
   * distinguishes this position by the fact that its
   * coordinate system may be arbitrary, and seperate
   * from any external coordinate system.
   *
   * @see getEncoderPose()
   * @see moveTo()
   * @ingroup easy
  */
  MvrPose getPose(void) const { return myGlobalPose; }
  /// Gets the global X position of the robot
  double getX(void) const { return myGlobalPose.getX(); }
  /// Gets the global Y position of the robot
  double getY(void) const { return myGlobalPose.getY(); }
  /// Gets the global angular position ("theta") of the robot
  double getTh(void) const { return myGlobalPose.getTh(); }
  /// Gets the distance to a point from the robot's current position
  double findDistanceTo(const MvrPose pose)
  { return myGlobalPose.findDistanceTo(pose); }
  /// Gets the angle to a point from the robot's current positoin and orientation
  double findAngleTo(const MvrPose pose)
  { return myGlobalPose.findAngleTo(pose); }
  /// Gets the difference between the angle to a point from the robot's current heading
  double findDeltaHeadingTo(const MvrPose pose)
  { return MvrMath::subAngle(myGlobalPose.findAngleTo(pose), myGlobalPose.getTh()); }

  /// Gets the current translational velocity of the robot
  double getVel(void) const { return myVel; }
  /// Gets the current rotational velocity of the robot
  double getRotVel(void) const { return myRotVel; }
  /// Gets the current lateral velocity of the robot
  double getLatVel(void) const { return myLatVel; }
  /// Sees if the robot supports lateral velocities
  bool hasLatVel(void) const { return myParam->hasLatVel(); }
  /// Gets the robot radius (in mm)
  double getRobotRadius(void) const { return myParams->getRobotRadius(); }
  /// Gets the robot width (in mm)
  double getRotbotWidth(void) const { return myParams->getRobotWidth(); }
  /// Gets the robot length (in mm)
  double getRobotLength(void) const { return myParams->getRobotLength(); }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthFront(void) const { return myRobotLengthFront; }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthRear(void) const { return myRobotLengthRear; }
  /// Gets distance from center of robot to frist vertex of octagon approximating the shape of robot (mm)
  double getRobotDiagonal(void) const { return myParams->getRobotDiagonal();}
  /// Gets the battery voltage of the robot (normalized to 12 volt system)
  double getBatteryVoltage(void) const { return myBatteryAverager.getAverag(); }\
  /// Gets the instanceous battery voltage
  double getBatteryVoltageNow(void) const { return myBatteryVoltage; }
  /// Gets the real battery voltage of the robot
  double getRealBatteryVoltage(void) const
   { return myRealBatteryAverager.getAverage(); }
  /// Gets the instaneous battery voltage
  double getRealBatteryVoltageNow(void) const { return myRealBatteryVoltage; }
  /// Gets if the state of charge value is in use
  bool haveStateOfCharge(void) const { return myHaveStateOfCharge; }
  //// @copydoc haveStateOfCharge
  bool hasStateOfCharge(void) const { return haveStateOfCharge(); }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfCharge(void) const
   { if (!myHaveStateOfCharge) return 0; else return myHaveStateOfCharge; }
  /// Gets the last time the state of charge was set
  MvrTime getStateOfChargeSetTime(void) const { return myStateOfChargeSetTime; }
  /// Gets the state of charge that is considered low
  double getStateOfChargeLow(void) const
    { return myStateOfChargeLow; }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfChargeShutdown(void) const
    { return myStateOfChargeShutdown; }
  /// Gets the velocity of the left side of the robot in mm/s (use of getVel() and )getRotVel() is preferred instead)
  double getLeftVel(void) const { return myLeftVel; }
  /// Gets the velocity of the right side of the robot in mm/s (use of getVel() and )getRotVel() is preferred instead)
  double getRightVel(void) const { return myRightVel; }
  /// Gets the 2 bytes of stall and bumper flags from the robot
  /*
   * See robot operations manual SIP packet documentation for details
   * @see isLeftMotorStalled()
   * @see isRightMotorStalled()
   * @see isFrontBumperTriggered()
   * @see MvrBumpers()
   */
  int getStallValue(void) const { return myStallValue; }
  //// Returns true if the left motor is stalled
  bool isLeftMotorStalled(void) cosnt
  { return (myStallValue & 0xff) & MvrUtil::BIT0; }
  //// Returns true if the right motor is stalled
  bool isRightMotorStalled(void) cosnt
  { return ((myStallValue & 0xff00) >> 8) & MvrUtil::BIT0; }
  /// Returns true if the front bumper is triggered
  /// @see MvrBumpers
  bool isFrontBumperTriggered(void) const
  { return hasFrontBumpers() && ((((myStallValue & 0xff00) >> 8) & ~MvrUtil::BIT0) != 0); }
  /// Returns true if the rear bumper is triggered.
  /// @see MvrBumpers
  bool isRearBumperTriggered(void) const
  { return hasRearBumpers() && (((myStallValue & 0xff) & ~MvrUtil::BIT0) !=0 ); }

  /// Gets the legacy control heading
  /*
   * Gets the control heading as an offset from the current heading
   * @see getTh
  */
  double getControl(void) const { return myControl; }
  /// Sets whether to keep the control value raw or not
  void setKeepControlRaw(bool keepControlRaw)
  { myKeepControlRaw = kepControlRaw; }
  /// Gets whetehr the control value is kept raw
  bool getKeepControlRaw(void)
  { return myKeepControlRaw; }
  /// Gets the flags value
  int getFlags(void) const { return myFlags; }
  /// Gets the fault flag values
  int getFaultFlags(void) const { return myFaultFlags; }
  /// Gets whethter or not we're getting the fault flags value
  bool hasFaultFlags(void) const { return myHasFaultFlags; }
  /// Gets the flags values
  int getFlags3(void) const { return myFlags3; }
  /// Gets whether or not we're getting the fault flags values
  bool hasFlags3(void) const { return myHasFlags3; }
  /// returns true if the motors are enable
  bool areMotorsEnabled(void) const { return (myFlags & MvrUtil:BIT0); }
  /// returns true if the sonars are enable
  /// This used to just check the low level firmware value, but
  /// that's now done by areSonarsEnabledLegacy;
  bool areSonarsEnabled(void) const { return mySonarEnabled; }
  /// returns true if the sonars are enabled for autonomous driving
  bool areAutonomousDrivingSonarEnabled(void) const
  { return myAutonomousDrivingSonarEnabled; }
  /// returns true if the sonars are enabled on legacy platforms
  bool areSonarsEnabledLegacy(void) const 
  {
    return (myFlags & (MvrUtil::BIT1 | MvrUtil::BIT2 | MvrUtil::BIT3 | MvrUtil::BIT4));
  }
  /// returns true if the estop is pressed (or unrelieved)
  bool isEStopPressed(void) const { return (myFlags & MvrUtil::BIT5); }
  /// Returns true if the E-Stop button is pressed
  bool getEstop(void) {return isEStopPressed(); }

  // Gets the compass heading from the robot
  double getCompass(void) const { return myCompass; }
  /// Gets which analog port is selected
  int getAnalogPortSelected(void) const { return myAnalogPortSelected; }
  /// Gets the analog value
  unsigned char getAnalog(void) const { return myAnalog; }
  /// Gets the byte representing digital input status
  unsigned char getDigIn(void) const { return myDigIn; }
  /// Gets the byte representing digital output status
  unsigned char getDigOut(void) const { return myDigOut; }
  /// Gets the charge state of the robot 
  MVREXPORT ChargeState getChargeState(void) const;

  /// Gets the name of the charge state
  const char *getChargeStateName() const
  {
    switch(getChargeState())
    {
      case CHARGING_NOT:
        return "not charging";
      case CHARGING_BULK:
        return "bulk";
      case CHARGING_OVERCHARGE:
        return "overcharge";
      case CHARGING_FLOAT:
        return "float";
      case CHARGING_BALANCE:
        return "balance";
      case CHARGING_UNKNOWN:
      default:
        return "unknown";
    }
    return "unknown";
  }
  /// Gets if the robot is on a charger
  MVREXPORT bool isChargerPowerGood(void) const;

  /// Gets the number of bytes in the analog IO buffer
  int getIOAnalogSize(void) const { return myIOAnalogSize; }
  /// Gets the number of bytes in the digital input IO buffer
  int getIODigInSize(void) const { return myIODigInSize; }
  /// Gets the bumber of bytes in the digital output IO buffer
  int getIODigOutSize(void) const { return myIODigOutSize; }

  /// Gets the n'th byte from the analog input data from the IO packet
  MVREXPORT int getIOAnalog(int num) const;
  /// Gets the n'th byte from the analog input data from the IO packet
  MVREXPORT double getIOAnalogVoltage(int num) const;
  /// Gets the n'th byte from the digital input data from the IO packet
  MVREXPORT unsigned char getIODigIn(int num) const;
  /// Gets the n'th byte from the digital output data from the IO packet
  MVREXPORT unsigned char getIODigOut(int num) const;
  
  /// Gets whether the robot has table sensing IR or not
  bool hasTableSensingIR(void) const { return myParams->haveTableSensingIR(); }
  /// Returns true if the left table sensing IR is triggered
  MVREXPORT bool isLeftTableSensingIRTriggered(void) const;
  /// Returns true if the right table sensing IR is triggered
  MVREXPORT bool isRightTableSensingIRTriggered(void) const;
  /// Returns true if the left break beam IR is triggered
  MVREXPORT bool isLeftBreakBeamTriggered(void) const;
  /// Returns true if the right break beam IR is triggered
  MVREXPORT bool isRightBreakBeamTriggered(void) const;
  /// Returns the time received on the last IO packet
  MvrTime getIOPacketTime(void) const { return myLastIOPacketReceivedTime; }

  /// Gets whether the robot has front bumpers
  bool hasFrontBumpers(void) const { return myParams->haveFrontBumpers(); }
  /// Gets the number of the fron bumper switches
  unsigned int getNumFrontBumpers(void) const
  { return myParams->numFrontBumpers(); }
  /// Gets whether the robot has rear bumpers
  bool hasRearBumpers(void) const { return myParams->haveRearBumpers(); }
  /// Gets the number of rear bumper switch
  unsigned int getNumRearBumper(void) const
  { return myParams->numRearBumpers(); }
  
  /// @brief Get the position of the robot according to the last robot SIP,
  /// possibly with gyro correction if installed and enabled, but without any
  /// transformations applied
  /// @see getPose()
  /// @see getRawEncoderPose()
  MvrPose getEncoderPose(void) const { return myEncoderPose; }
  /// Gets the encoder X position of the robot
  /// @see getEncoderPose()
  double getEncoderX(void) const { return myEncoderPose.getX(); }
  /// Gets the encoder Y position of the robot
  /// @see getEncoderPose()
  double getEncoderY(void) const { return myEncoderPose.getY(); }
  /// Gets the encoder angular position (theta) of the robot
  /// @see getEncoderPose()
  double getEncoderTh(void) const { return myEncoderPose.getTh(); }

  /// Gets if the robot is trying to move or not
  bool isTryingToMove(void) { return myTryingToMove; }
  /// Manually sets the flag that says the robot is trying to move for a cycle
  void forceTryingToMove(void) { myTryingToMove = true; }

  /// Gets the number of motor packets received in the last second
  MVREXPORT int getMotorPacCount(void) const;
  /// Gets the number of sonar returns received in the last second
  MVREXPORT int getSonarPacCount(void) const;

  /// Gets the range of the last sonar reading for the given sonar
  MVREXPORT int getSonarRange(int num) const;
  /// Find out if the given sonar reading was newly refreshed by the last incoming SIP
  MVREXPORT bool isSonarNew(int num) const;
  /// Find the number of sonar sensors(that the robot has yet returned values for)
  int getNumSonar(void) const { return myNumSonar; }
  /// Returns the sonar reading for the given sonar
  MVREXPORT MvrSensorReading *getSonarReading(int num) const;
  /// Returns the closest of the current sonar reading in the given range
  MVREXPORT int getClosestSonarRange(double startAngle, double endAngle) const;
  /// Returns the number of the sonar that the closest current reading in the given angle
  MVREXPORT int getClosestSonarNumber(double startAngle, double endAngle) const;

  /// Gets the robots name
  MVREXPORT const char *getName(void) const;
  /// Sets the rbots name
  MVREXPORT void setName(const char *name);
  
  /// Change stored pose (i.e. the value returned by getPose())
  MVREXPORT void moveTo(MvrPose pose, bool doCumulative=true);
  /// Change stored pose (i.e. the value returned by getPose())
  MVREXPORT void moveTo(MvrPose to, MvrPose from, bool doCumulative=true);
  /// Manually sets the current percentage that the robot is charged
  MVREXPORT void setStateOfCharge(double stateOfCharge);
  /// Sets the state of charge (percentage) that is considered to be low
  void setStateOfChargeLow(double stateOfChargeLow)
  { myStateOfChargeLow = stateOfChargeLow; }
  /// Sets the charge state (for use with setting the state of charge)
  MVREXPORT void setChargeState(MvrRobot::ChargeState chargeState);
  /// Sets if we're on the charger (for use with setting the state of charge)
  MVREXPORT void setIsChargerPowerGood(bool onCharger);

  /// Sets things so that the battery info from the microcontroller
  /// will be ignored
  MVREXPORT void setIgnoreMicroControllerBatteryInfo(bool ignoreMicroControllerBatteryInfo);
  /// Sets the battery info
  MVREXPORT void setBatteryInfo(double realBatteryVoltage, 
                                double normalizedBatteryVoltage,
                                bool haveStateOfCharge,
                                double stateOfCharge);
  /// Gets the number of readings the battery voltage is the average of
  size_t getBatteryVoltageAverageOfNum(void) 
  { return myBatteryAverager.getNumToAverage(); }
  /// Sets the number of readings the battery voltage is the average of (default 20)
  void setBatteryVoltageAverageOfNum(size_t numToAverage)
  { myBatteryAverager.setNumToAverage(numToAverage); }

  /// Gets the number of readings the battery voltage is the average of
  size_t getRealBatteryVoltageAverageOfNum(void) 
  { return myRealBatteryAverager.getNumToAverage(); }

  /// Sets the number of readings the real battery voltage is the average of (default 20)
  void setRealBatteryVoltageAverageOfNum(size_t numToAverage)
  { myRealBatteryAverager.setNumToAverage(numToAverage); }

  /// Returns true if we have a temperature, false otherwise
  bool hasTemperature(void)
  { if (myTemperature != -128) return true; else return false; }

  /// Gets the temperature of the robot, -128 if not availabe, -127 to 127 otherwise
  int getTemperature(void) const { return myTemperature; }

  /// Starts a continuous stream of encoder packets
  MVREXPORT void requestEncoderPackets(void);

  /// Starts a continuous stream of IO packets
  MVREXPORT void requestIOPackets(void);

  /// Stops a continuous stream of encoder packets
  MVREXPORT void stopEncoderPackets(void);

  /// Stops a continuous stream of IO packets
  MVREXPORT void stopIOPackets(void);

  /// Sees if we've explicitly requested encoder packets
  MVREXPORT bool haveRequestedEncoderPackets(void);

  /// Sees if we've explicitly requested IO packets
  MVREXPORT bool haveRequestedIOPackets(void);

  /// Gets packet data from the left encoder
  MVREXPORT long int getLeftEncoder(void);

  /// Gets packet data from the right encoder
  MVREXPORT long int getRightEncoder(void);

  /// Changes the transform
  MVREXPORT void setEncoderTransform(MvrPose deadReconPos, MvrPose globalPos);

  /// Changes the transform directly
  MVREXPORT void setEncoderTransform(MvrPose transformPos);

  /// Changes the transform directly
  MVREXPORT void setEncoderTransform(MvrTransform transform);

  /// Sets the encoder pose, for internal use
  void setEncoderPose(MvrPose encoderPose) 
    { myEncoderPose = encoderPose; }

  /// Sets the raw encoder pose, for internal use
  void setRawEncoderPose(MvrPose rawEncoderPose) 
    { myRawEncoderPose = rawEncoderPose; }
  
  /// Adds a callback for when the encoder transform is changed
  void addSetEncoderTransformCB(MvrFunctor *functor, int position = 50)
    { mySetEncoderTransformCBList.addCallback(functor, position); }

  /// Removes a callback for when the encoder transform is changed
  void remSetEncoderTransformCB(MvrFunctor *functor)
    { mySetEncoderTransformCBList.remCallback(functor); }

  /// Adds a callback that'll be used to see the time on the computer side (for an MTX)
  void setMTXTimeUSecCB(MvrRetFunctor1<bool, MvrTypes::UByte4 *> *functor)
    { myMTXTimeUSecCB = functor; }

  /// Gets the encoder transform
  MVREXPORT MvrTransform getEncoderTransform(void) const;

  /// This gets the transform from local coords to global coords
  MVREXPORT MvrTransform getToGlobalTransform(void) const;

  /// This gets the transform for going from global coords to local coords
  MVREXPORT MvrTransform getToLocalTransform(void) const;

  /// This applies a transform to all the robot range devices and to the sonar
  MVREXPORT void applyTransform(MvrTransform trans, bool doCumulative = true);

  /// Sets the dead recon position of the robot
  MVREXPORT void setDeadReconPose(MvrPose pose);
  /// This gets the distance the robot has travelled since the last time resetTripOdometer() was called (mm)
  /// This is a virtual odometer that measures the total linear distance the
  /// robot has travelled since first connected, or resetTripOdometer() was
  /// called, approximated by adding linear
  /// distance between current pose and previous pose in each MvrRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  MVREXPORT double getTripOdometerDistance(void) 
    { return myTripOdometerDistance; }

  /// This gets the number of degrees the robot has turned since the last time resetTripOdometer() was called (deg)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since first connected, or resetTripOdometer() was last
  /// called, approximated by adding linear
  /// distance between current pose and previous pose in each MvrRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  /// @see getTripOdometerDistance()
  double getTripOdometerDegrees(void) { return myTripOdometerDegrees; }

  /// This gets the time since the "Trip Odometer" was reset (sec)
  /// @see getTripOdometerDistance()
  double getTripOdometerTime(void) { return myTripOdometerStart.secSince(); }

  /// Resets the "Trip Odometer"
  /// @see getTripOdometerDistance()
  MVREXPORT void resetTripOdometer(void);

  /// This gets the total cumulative distance the robot has travelled (mm)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since ARIA connected, approximated by adding linear
  /// distance between current pose and previous pose in each MvrRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  double getOdometerDistance(void) { return myOdometerDistance; }
  /// @see getOdometerDistance
  double getOdometerDistanceMeters(void) { return myOdometerDistance/1000.0; }

  /// This gets the total cumulative number of degrees the robot has turned (deg)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since ARIA connected, approximated by adding linear
  /// distance between current pose and previous pose in each MvrRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  double getOdometerDegrees(void) { return myOdometerDegrees; }

  /// This gets the time since the robot started (sec)
  double getOdometerTime(void) { return myOdometerStart.secSince(); }
  /// This gets the time since the robot started (mins)
  double getOdometerTimeMinutes() { return myOdometerStart.secSince() / 60.0; }
  
  /// Adds a rangeDevice to the robot's list of them, and set the MvrRangeDevice
  /// object's robot pointer to this MvrRobot object.
  MVREXPORT void addRangeDevice(MvrRangeDevice *device);
  /// Remove a range device from the robot's list, by name
  MVREXPORT void remRangeDevice(const char *name);
  /// Remove a range device from the robot's list, by instance
  MVREXPORT void remRangeDevice(MvrRangeDevice *device);

  /// Finds a rangeDevice in the robot's list
  MVREXPORT const MvrRangeDevice *findRangeDevice(const char *name, bool ignoreCase = false) const;

  /// Finds a rangeDevice in the robot's list
  MVREXPORT MvrRangeDevice *findRangeDevice(const char *name, bool ignoreCase = false);

  /// Gets the range device list
  MVREXPORT std::list<MvrRangeDevice *> *getRangeDeviceList(void);

  /// Finds whether a particular range device is attached to this robot or not
  MVREXPORT bool hasRangeDevice(MvrRangeDevice *device) const;

  /// Goes through all the range devices and checks them
  MVREXPORT double checkRangeDevicesCurrentPolar(
	  double startAngle, double endAngle, double *angle = NULL,
	  const MvrRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  /// Goes through all the range devices and checks them
  MVREXPORT double checkRangeDevicesCumulativePolar(
	  double startAngle, double endAngle, double *angle = NULL,
	  const MvrRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;
    
  // Goes through all the range devices and checks them
  MVREXPORT double checkRangeDevicesCurrentBox(
	  double x1, double y1, double x2, double y2,
	  MvrPose *readingPos = NULL,
	  const MvrRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  // Goes through all the range devices and checks them
  MVREXPORT double checkRangeDevicesCumulativeBox(
	  double x1, double y1, double x2, double y2,
	  MvrPose *readingPos = NULL, 
	  const MvrRangeDevice **rangeDevice = NULL,
	  bool useLocationDependentDevices = true) const;

  /// Adds a laser to the robot's map of them
  MVREXPORT bool addLaser(MvrLaser *laser, int laserNumber, 
			                    bool addAsRangeDevice = true);
  /// Remove a range device from the robot's list, by instance
  MVREXPORT bool remLaser(MvrLaser *laser, bool removeAsRangeDevice = true);
  /// Remove a range device from the robot's list, by number
  MVREXPORT bool remLaser(int laserNumber, bool removeAsRangeDevice = true);

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  MVREXPORT const MvrLaser *findLaser(int laserNumber) const;

  /// Finds a laser in the robot's list (@a laserNumber indices start at 1)
  MVREXPORT MvrLaser *findLaser(int laserNumber);

  /// Gets the range device list
  MVREXPORT const std::map<int, MvrLaser *> *getLaserMap(void) const;

  /// Gets the range device list
  MVREXPORT std::map<int, MvrLaser *> *getLaserMap(void);

  /// Finds whether a particular range device is attached to this robot or not
  MVREXPORT bool hasLaser(MvrLaser *device) const;

  size_t getNumLasers() { return myLaserMap.size(); }

  /// Adds a battery to the robot's map of them
  /// @internal
  /// (MvrBatteryConnector/MvrRobotConnector will automatically add MvrBatteryMTX
  /// object(s) if connected.)
  MVREXPORT bool addBattery(MvrBatteryMTX *battery, int batteryNumber);
  /// Remove a battery from the robot's list, by instance
  /// (MTX robots only)
  /// Primarily for ARIA internal use only.
  MVREXPORT bool remBattery(MvrBatteryMTX *battery);
  /// Remove a battery from the robot's list, by number
  /// (MTX robots only)
  /// Primarily for ARIA internal use only.
  MVREXPORT bool remBattery(int batteryNumber);

  /// Finds a battery in the robot's list (@a batteryNumber indices start at 1)
  /// (MTX robots only)
  MVREXPORT const MvrBatteryMTX *findBattery(int batteryNumber) const;

  /// Finds a battery in the robot's list (@a batteryNumber indices start at 1)
  /// (MTX robots only)
  MVREXPORT MvrBatteryMTX *findBattery(int batteryNumber);

  /// Gets the battery list
  MVREXPORT const std::map<int, MvrBatteryMTX *> *getBatteryMap(void) const;

  /// Gets the battery list
  MVREXPORT std::map<int, MvrBatteryMTX *> *getBatteryMap(void);

  /// Finds whether a particular battery is attached to this robot or not
  MVREXPORT bool hasBattery(MvrBatteryMTX *device) const;

  /// Adds a lcd to the robot's map of them
  /// @internal
  /// (MvrLCDConnector/MvrRobotConnector will automatically add LCD interfaces if
  /// connected.)
  MVREXPORT bool addLCD(MvrLCDMTX *lcd, int lcdNumber);
  /// Remove a lcd from the robot's list, by instance
  /// @internal
  /// (MvrLCDConnector/MvrRobotConnector will automatically add LCD interfaces if
  /// connected.)
  MVREXPORT bool remLCD(MvrLCDMTX *lcd);
  /// Remove a lcd from the robot's list, by number
  /// @internal
  /// (MvrLCDConnector/MvrRobotConnector will automatically add LCD interfaces if
  /// connected.)
  MVREXPORT bool remLCD(int lcdNumber);

  /// Finds a lcd in the robot's list (@a lcdNumber indices start at 1)
  /// (MTX robots only) 
  MVREXPORT const MvrLCDMTX *findLCD(int lcdNumber = 1) const;

  /// Finds a lcd in the robot's list (@a lcdNumber indices start at 1)
  /// (MTX robots only) 
  MVREXPORT MvrLCDMTX *findLCD(int lcdNumber = 1);

  /// Gets the lcd list
  /// (MTX robots only) 
  MVREXPORT const std::map<int, MvrLCDMTX *> *getLCDMap(void) const;

  /// Gets the lcd list
  /// (MTX robots only) 
  MVREXPORT std::map<int, MvrLCDMTX *> *getLCDMap(void);

  /// Finds whether a particular lcd is attached to this robot or not
  /// (MTX robots only) 
  MVREXPORT bool hasLCD(MvrLCDMTX *device) const;
protected:  
  MvrPose myGlobalPose;

protected:
};
#endif  // MVRROBOT_H