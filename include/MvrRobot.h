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
    ArRangeDevice objects, ArAction objects, and others.  For details
    on usage, and how the task cycle and obot state synchronization works,
    see the @ref robot "ArRobot section" and the
    @ref ClientCommands "Commands and Actions section" of the ARIA overview.

    @note In Windows you cannot make an ArRobot object a global variable, 
    it will crash because the compiler initializes the constructors in
    the wrong order. You can, however, make a pointer to an ArRobot and then
    allocate it with 'new' at program start.

    @see ArRobotConnector

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
  { return hasRearBumpers() && (((myStallValue & 0xff) & ~ArUtil::BIT0) !=0 ); }

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




  

protected:
  MvrPose myGlobalPose;

protected:
};
#endif  // MVRROBOT_H