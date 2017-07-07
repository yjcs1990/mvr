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

/// Central class for communicating with and operating the robot
/** 
    This is the most important class. It is used to communicate with
    the robot by sending commands and retrieving data (including
    wheel odometry, digital and analog inputs, sonar data, and more). 
    It is also used
    to provide access to objects for controlling attached accessories, 
    MvrRangeDevice objects, MvrAction objects, and others.  For details
    on usage, and how the task cycle and obot state synchronization works,
    see the @ref robot "MvrRobot section" and the
    @ref ClientCommands "Commands and Actions section" of the ARIA overview.

    @note In Windows you cannot make an MvrRobot object a global vmvriable, 
    it will crash because the compiler initializes the constructors in
    the wrong order. You can, however, make a pointer to an MvrRobot and then
    allocate it with 'new' at program start.

    @see MvrRobotConnector

    @ingroup ImportantClasses
   @ingroup DeviceClasses
**/
class MvrRobot
{
public:

  typedef enum {
    WAIT_CONNECTED, ///< The robot has connected
    WAIT_FAILED_CONN, ///< The robot failed to connect
    WAIT_RUN_EXIT, ///< The run loop has exited
    WAIT_TIMEDOUT, ///< The wait reached the timeout specified
    WAIT_INTR, ///< The wait was interupted by a signal
    WAIT_FAIL ///< The wait failed due to an error
  } WaitState;

  enum ChargeState {
    CHARGING_UNKNOWN = -1,
    CHARGING_NOT = 0,
    CHARGING_BULK = 1,
    CHARGING_OVERCHARGE = 2,
    CHARGING_FLOAT = 3,
    CHARGING_BALANCE = 4
  };
  /// Constructor
  MVREXPORT MvrRobot(const char * name = NULL, bool ignored = true,
		   bool doSigHandle=true, 
		   bool normalInit = true, bool addMvrExitCallback = true);
		   
  /// Destructor
  MVREXPORT ~MvrRobot();

  /// Starts the instance to do processing in this thread
  MVREXPORT void run(bool stopRunIfNotConnected, 
		    bool runNonThreaded = false);
  /// Starts the instance to do processing in its own new thread
  MVREXPORT void runAsync(bool stopRunIfNotConnected, 
			 bool runNonThreadedPacketReader = false);

  /// @copydoc run(bool, bool)
  /// @ingroup easy
  void run() { run(true); }

  /// @copydoc runAsync(bool, bool)
  /// @ingroup easy
  void runAsync() { runAsync(true); }

  /// Returns whether the robot is currently running or not
  /// @ingroup easy
  MVREXPORT bool isRunning(void) const;

  /// Stops the robot from doing any more processing
  /// @ingroup easy
  MVREXPORT void stopRunning(bool doDisconnect=true); 

  /// Sets the connection this instance uses
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *connection);
  /// Gets the connection this instance uses
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void) const;
  
  /// Questions whether the robot is connected or not
  /** 
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

  /// Clears what direct motion commands have been given, so actions work
  MVREXPORT void clearDirectMotion(void);
  /// Returns true if direct motion commands are blocking actions
  MVREXPORT bool isDirectMotion(void) const;

  /// Sets the state reflection to be inactive (until motion or clearDirectMotion)
  /// @see clearDirectMotion
  MVREXPORT void stopStateReflection(void);
  

  /// Enables the motors on the robot
  /// @ingroup easy
  MVREXPORT void enableMotors();
  /// Disables the motors on the robot
  /// @ingroup easy
  MVREXPORT void disableMotors();

  /// Enables the sonar on the robot
  /// @ingroup easy
  MVREXPORT void enableSonar();
  /// Enables some of the sonar on the robot (the ones for autonomous driving)
  MVREXPORT void enableAutonomousDrivingSonar();
  /// Disables the sonar on the robot
  /// @ingroup easy
  MVREXPORT void disableSonar();

  /// Stops the robot
  /// @see clearDirectMotion
  /// @ingroup easy
  MVREXPORT void stop(void);
  /// Sets the velocity
  /// @see clearDirectMotion
  /// @ingroup easy
  MVREXPORT void setVel(double velocity);
  /// Sets differential movement if implemented by robot
  MVREXPORT void setVel2(double leftVelocity, double rightVelocity);
  /// Move the given distance forward/backwards
  /// @ingroup easy
  MVREXPORT void move(double distance);
  /// Sees if the robot is done moving the previously given move
  /// @ingroup easy
  MVREXPORT bool isMoveDone(double delta = 0.0);
  /// Sets the difference required for being done with a move
  void setMoveDoneDist(double dist) { myMoveDoneDist = dist; }
  /// Gets the difference required for being done with a move
  double getMoveDoneDist(void) { return myMoveDoneDist; }
  /// Sets the heading
  /// @ingroup easy
  MVREXPORT void setHeading(double heading);
  /// Sets the rotational velocity
  /// @ingroup easy
  MVREXPORT void setRotVel(double velocity);
  /// Sets the delta heading
  /// @ingroup easy
  MVREXPORT void setDeltaHeading(double deltaHeading);
  /// Sees if the robot is done changing to the previously given setHeading
  /// @ingroup easy
  MVREXPORT bool isHeadingDone(double delta = 0.0) const;
  /// sets the difference required for being done with a heading change (e.g. used in isHeadingDone())
  void setHeadingDoneDiff(double degrees) 
    { myHeadingDoneDiff = degrees; }
  /// Gets the difference required for being done with a heading change (e.g. used in isHeadingDone())
  double getHeadingDoneDiff(void) const { return myHeadingDoneDiff; }
  /// Sets the lateral velocity
  /// @see clearDirectMotion
  MVREXPORT void setLatVel(double latVelocity);


  /// sees if we're stopped
  /// @ingroup easy
  MVREXPORT bool isStopped(double stoppedVel = 0.0, double stoppedRotVel = 0.0,
			  double stoppedLatVel = 0.0);

  /// Sets the vels required to be stopped
  MVREXPORT void setStoppedVels(double stoppedVel, double stoppedRotVel,
			       double stoppedLatVel);

  /// Sets the length of time a direct motion command will take precedence 
  /// over actions, in milliseconds
  MVREXPORT void setDirectMotionPrecedenceTime(int mSec);

  /// Gets the length of time a direct motion command will take precedence 
  /// over actions, in milliseconds
  MVREXPORT unsigned int getDirectMotionPrecedenceTime(void) const;

  /// Sends a command to the robot with no arguments
  MVREXPORT bool com(unsigned char command);
  /// Sends a command to the robot with an int for argument
  MVREXPORT bool comInt(unsigned char command, short int argument);
  /// Sends a command to the robot with two bytes for argument
  MVREXPORT bool com2Bytes(unsigned char command, char high, char low);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStr(unsigned char command, const char *argument);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStrN(unsigned char command, const char *str, int size);
  /// Sends a command containing exactly the data in the given buffer as argument
  MVREXPORT bool comDataN(unsigned char command, const char *data, int size);

  /// Returns the robot's name that is set in its onboard firmware configuration
  const char * getRobotName(void) const { return myRobotName.c_str();}
  /// Returns the type of the robot we are currently connected to
  const char * getRobotType(void) const { return myRobotType.c_str();}
  /// Returns the subtype of the robot we are currently connected to
  const char * getRobotSubType(void) const 
    { return myRobotSubType.c_str(); }

  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransVel(void) const 
    { return myAbsoluteMaxTransVel; }
  /// Sets the robot's absolute maximum translational velocity
  MVREXPORT bool setAbsoluteMaxTransVel(double maxVel);

  /// Gets the robot's absolute maximum translational velocity
  double getAbsoluteMaxTransNegVel(void) const 
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
  double getAbsoluteMaxRotVel(void) const 
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
  double getAbsoluteMaxLatVel(void) const 
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
  /// Sets the robot's absolute maximum lateral deceleration
  MVREXPORT bool setAbsoluteMaxLatDecel(double maxDecel);


  // Accessors

  /** @brief Get the current stored global position of the robot.
   *
   *  This position is updated by data reported by the robot as it
   *  moves, and may also be changed by other program components,
   *  such as a localization process (see moveTo()).
   *
   *  This position is also referred to as the robot's 
   *  "odometry" or "odometric" pose, since the robot
   *  uses its odometry data to determine this pose; but it
   *  may also incorporate additional data sources such as 
   *  an onboard gyro.  The term "odometric pose" also 
   *  distinguishes this position by the fact that its
   *  coordinate system may be arbitrary, and seperate
   *  from any external coordinate system.
   *
   *  @sa getEncoderPose()
   *  @sa moveTo()
   *  @ingroup easy
   *
   */
  MvrPose getPose(void) const { return myGlobalPose; }
  /// Gets the global X position of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getX(void) const { return  myGlobalPose.getX(); }
  /// Gets the global Y position of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getY(void) const { return myGlobalPose.getY(); }
  /// Gets the global angular position ("theta") of the robot
  /** @sa getPose() 
      @ingroup easy
  */
  double getTh(void) const { return myGlobalPose.getTh(); }
  /// Gets the distance to a point from the robot's current position
  /// @ingroup easy
  double findDistanceTo(const MvrPose pose) 
    { return myGlobalPose.findDistanceTo(pose); }
  /// Gets the angle to a point from the robot's current position and orientation
  /// @ingroup easy
  double findAngleTo(const MvrPose pose) 
    { return myGlobalPose.findAngleTo(pose); }
  /// Gets the difference between the angle to a point from the robot's current heading
  /// @ingroup easy
  double findDeltaHeadingTo(const MvrPose pose) 
    { return MvrMath::subAngle(myGlobalPose.findAngleTo(pose),
			      myGlobalPose.getTh()); }


  /// Gets the current translational velocity of the robot
  /// @ingroup easy
  double getVel(void) const { return myVel; }
  /// Gets the current rotational velocity of the robot
  /**
     Note that with new firmware versions (ARCOS as of April 2006 or so)
     this is the velocity reported by the robot.  With older firmware
     this number is calculated using the difference between the robot's reported wheel
     velocities multiplied by diffConvFactor from the .p (robot parameter) files.
      @ingroup easy
  **/
  double getRotVel(void) const { return myRotVel; }
  /// Gets the current lateral velocity of the robot
  /**
     Note that this will only be valid if hasLatVel() returns true
     @since 2.6.0
   **/
  double getLatVel(void) const { return myLatVel; }
  /** Sees if the robot supports lateral velocities (e.g. Seekur(TM))
      @since 2.6.0
  */
  bool hasLatVel(void) const { return myParams->hasLatVel(); }
  /// Gets the robot radius (in mm)
  /// @ingroup easy
  double getRobotRadius(void) const { return myParams->getRobotRadius(); }
  /// Gets the robot width (in mm)
  /// @ingroup easy
  double getRobotWidth(void) const { return myParams->getRobotWidth(); }
  /// Gets the robot length (in mm)
  /// @ingroup easy
  double getRobotLength(void) const { return myParams->getRobotLength(); }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthFront(void) const { return myRobotLengthFront; }
  /// Gets the robot length to the front (in mm)
  double getRobotLengthRear(void) const { return myRobotLengthRear; }
  /// Gets distance from center of robot to first vertex of octagon approximating the shape of robot. (mm) 
  double getRobotDiagonal(void) const { return myParams->getRobotDiagonal(); }
  /// Gets the battery voltage of the robot (normalized to 12 volt system)
  /**
     This value is averaged over a number of readings, use getBatteryVoltageNow() to
     get the value most recently received.  (Access the number of readings
     used in the running average with getBatteryVoltageAverageOfNum() and
     setBatteryVoltageAverageOfNum().)

     This is a value normalized to 12 volts, if you want what the
     actual voltage of the robot is use getRealBatteryVoltage().

     @sa getRealBatteryVoltage()
     @sa getBatteryVoltageNow()
   **/
  double getBatteryVoltage(void) const {return myBatteryAverager.getAverage();}
  /// Gets the instaneous battery voltage
  /**
     This is a value normalized to 12 volts, if you want what the
     actual voltage of the robot is use getRealBatteryVoltage().
     
     @sa getBatteryVoltage()
     @sa getRealBatteryVoltage()
   **/
  double getBatteryVoltageNow(void) const { return myBatteryVoltage; }
  /// Gets the real battery voltage of the robot 
  /**
     This value is averaged over a number of readings, use getRealBatteryVoltageNow() to
     get the value most recently received.  (Access the number of readings
     used in the running average with getRealBatteryVoltageAverageOfNum() and
     setRealBatteryVoltageAverageOfNum().)

     This is whatever the actual voltage of the robot is, if you want
     a value normalized to common 12 volts for all robot types use getBatteryVoltage(). 

     If the robot doesn't support a "real" battery voltage, then
     this method will just return the normal battery voltage (normalized to 12
     volt scale). (Most older robots that don't support a real battery voltage
     have 12 volts batteries anyway.)
   **/
  double getRealBatteryVoltage(void) const 
    { return myRealBatteryAverager.getAverage(); }
  /// Gets the instaneous battery voltage
  /**
     This is whatever the actual voltage of the robot is, if you want
     a value normalized to a common 12 volts for all robot types use getBatteryVoltage().  If the
     robot doesn't support this number the voltage will be less than 0
     and you should use getBatteryVoltageNow().
  **/
  double getRealBatteryVoltageNow(void) const { return myRealBatteryVoltage; }
  /// Gets if the state of charge value is in use
  bool haveStateOfCharge(void) const { return myHaveStateOfCharge; }
  /// @copydoc haveStateOfCharge()
  bool hasStateOfCharge() const { return haveStateOfCharge(); }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfCharge(void) const 
    { if (!myHaveStateOfCharge) return 0; else return myStateOfCharge; }
  /// Gets the last time the state of charge was set
  MvrTime getStateOfChargeSetTime(void) const { return myStateOfChargeSetTime; }
  /// Gets the state of charge that is considered low
  double getStateOfChargeLow(void) const 
    { return myStateOfChargeLow; }
  /// Gets the state of charge (percent of charge, as number between 0 and 100)
  double getStateOfChargeShutdown(void) const 
    { return myStateOfChargeShutdown; }
  /// Gets the velocity of the left side of the robot in mm/s (use of getVel() and getRotVel() is preferred instead)
  double getLeftVel(void) const { return myLeftVel; }
  /// Gets the velocity of the right side of the robot in mm/s (use of getVel() and getRotVel() is preferred instead)
  double getRightVel(void) const { return myRightVel; }
  /// Gets the 2 bytes of stall and bumper flags from the robot 
  /** See robot operations manual SIP packet documentation for details.
   *  @sa isLeftMotorStalled()
   *  @sa isRightMotorStalled()
   *  @sa isFrontBumperTriggered()
   *  @sa isRearBumperTriggered()
   *  @sa MvrBumpers
   */
  int getStallValue(void) const { return myStallValue; }
  /// Returns true if the left motor is stalled
  bool isLeftMotorStalled(void) const 
    { return (myStallValue & 0xff) & MvrUtil::BIT0; }
  /// Returns true if the left motor is stalled
  bool isRightMotorStalled(void) const
    { return ((myStallValue & 0xff00) >> 8) & MvrUtil::BIT0; }
  /// Returns true if the front bumper is triggered.   @sa MvrBumpers
  bool isFrontBumperTriggered(void) const
  { return hasFrontBumpers() && ((((myStallValue & 0xff00) >> 8) & ~MvrUtil::BIT0) != 0); }
  /// Returns true if the rear bumper is triggered. @sa MvrBumpers
  bool isRearBumperTriggered(void) const
  { return hasRearBumpers() && (((myStallValue & 0xff) & ~MvrUtil::BIT0) !=0 ); }

  /// Gets the legacy control heading
  /**
    Gets the control heading as an offset from the current heading.
    Only used in older Pioneer 2 and P2 robots with PSOS, P2OS, AROS.
    @see getTh
  */
  double getControl(void) const { return myControl; }
  /// Sets whether to keep the control value raw or not
  void setKeepControlRaw(bool keepControlRaw) 
    { myKeepControlRaw = keepControlRaw; }
  /// Gets whether the control value is kept raw 
  bool getKeepControlRaw(void) 
    { return myKeepControlRaw; }
  /// Gets the flags values
  int getFlags(void) const { return myFlags; }
  /// Gets the fault flags values
  int getFaultFlags(void) const { return myFaultFlags; }
  /// Gets whether or not we're getting the fault flags values
  bool hasFaultFlags(void) const { return myHasFaultFlags; }
  /// Gets the flags3 values
  int getFlags3(void) const { return myFlags3; }
  /// Gets whether or not we're getting the fault flags values
  bool hasFlags3(void) const { return myHasFlags3; }
  /// returns true if the motors are enabled
  bool areMotorsEnabled(void) const { return (myFlags & MvrUtil::BIT0); }
  /// returns true if the sonars are enabled
  /** This used to just check the low level firmware values, but
   * that's now done by areSonarsEnabledLegacy.
   */ 
  bool areSonarsEnabled(void) const 
    { return mySonarEnabled; }
  /// returns true if the sonars are enabled for autonomous driving 
  bool areAutonomousDrivingSonarsEnabled(void) const 
    { return myAutonomousDrivingSonarEnabled; }
  /// returns true if the sonars are enabled on legacy platforms (by checking the low level stuff)
  bool areSonarsEnabledLegacy(void) const { 
    return (myFlags & 
	    (MvrUtil::BIT1 | MvrUtil::BIT2 | MvrUtil::BIT3 | MvrUtil::BIT4)); }
  /// returns true if the estop is pressed (or unrelieved)
  bool isEStopPressed(void) const { return (myFlags & MvrUtil::BIT5); }

  /// Returns true if the E-Stop button is pressed
  bool getEstop(void) { return isEStopPressed(); }

  /// Gets the compass heading from the robot
  double getCompass(void) const { return myCompass; }
  /// Gets which analog port is selected
  int getAnalogPortSelected(void) const { return myAnalogPortSelected; }
  /// Gets the analog value
  unsigned char getAnalog(void) const { return myAnalog; }
  /// Gets the byte representing digital input status
  unsigned char getDigIn(void) const { return myDigIn; }
  /// Gets the byte representing digital output status
  unsigned char getDigOut(void) const { return myDigOut; }
  /// Gets the charge state of the robot (see long docs)
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
  /// Gets the number of bytes in the digital output IO buffer
  int getIODigOutSize(void) const { return myIODigOutSize; }

  /// Gets the n'th byte from the analog input data from the IO packet
  MVREXPORT int getIOAnalog(int num) const;
  /// Gets the n'th byte from the analog input data from the IO packet
  MVREXPORT double getIOAnalogVoltage(int num) const;
  /// Gets the n'th byte from the digital input data from the IO packet
  MVREXPORT unsigned char getIODigIn(int num) const;
  /// Gets the n'th byte from the digital output data from the IO packet
  MVREXPORT unsigned char getIODigOut(int num) const;

  /// Gets whether the robot has table sensing IR or not (see params in docs)
  bool hasTableSensingIR(void) const { return myParams->haveTableSensingIR(); }
  /// Returns true if the left table sensing IR is triggered
  MVREXPORT bool isLeftTableSensingIRTriggered(void) const;
  /// Returns true if the right table sensing IR is triggered
  MVREXPORT bool isRightTableSensingIRTriggered(void) const;
  /// Returns true if the left break beam IR is triggered
  MVREXPORT bool isLeftBreakBeamTriggered(void) const;
  /// Returns true if the right break beam IR is triggered
  MVREXPORT bool isRightBreakBeamTriggered(void) const;
  /// Returns the time received of the last IO packet
  MvrTime getIOPacketTime(void) const { return myLastIOPacketReceivedTime; }

  /// Gets whether the robot has front bumpers (see ARCOS parameters in the robot manual)
  bool hasFrontBumpers(void) const { return myParams->haveFrontBumpers(); }
  /// Get the number of the front bumper switches
  unsigned int getNumFrontBumpers(void) const 
    { return myParams->numFrontBumpers(); }
  /// Gets whether the robot has rear bumpers (see ARCOS parameters in the robot manual)
  bool hasRearBumpers(void) const { return myParams->haveRearBumpers(); }
  /// Gets the number of  rear bumper switches
  unsigned int getNumRearBumpers(void) const 
    { return myParams->numRearBumpers(); }

  /** @brief Get the position of the robot according to the last robot SIP,
   * possibly with gyro correction if installed and enabled, but without any
   * transformations applied.
   * @sa getPose()
   * @sa getRawEncoderPose()
   */
  MvrPose getEncoderPose(void) const { return myEncoderPose; }
  /// Gets the encoder X position of the robot
  /** @sa getEncoderPose() */
  double getEncoderX(void) const { return  myEncoderPose.getX(); }
  /// Gets the encoder Y position of the robot
  /** @sa getEncoderPose() */
  double getEncoderY(void) const { return myEncoderPose.getY(); }
  /// Gets the encoder  angular position ("theta") of the robot
  /** @sa getEncoderPose() */
  double getEncoderTh(void) const { return myEncoderPose.getTh(); }

  /// Gets if the robot is trying to move or not 
  /**
     "Trying" to move means that some action or other command has
     requested motion, but another action or factor has cancelled
     that request. 
   
     This is so that if the robot is trying to move, but is prevented
     (mainly by an action) there'll still be some indication that the
     robot is trying to move (e.g. we can prevent the sonar from automatically being
     turned off). Note that this flag
     doesn't have anything to do with if the robot is really moving or
     not, to check that, check the current velocities.
     @sa forceTryingToMove() to force this state on
   **/
  bool isTryingToMove(void) { return myTryingToMove; }

  /// Manually sets the flag that says the robot is trying to move for one cycle
  /**
     This is so that things that might move the robot at any time
     can can make the robot look
     like it is otherwise in the TryingToMove state, even if no motion
     is currently being requested.  For example, MvrNetworking's teleoperation
     mode forces TryingToMove at all times while active, not just when
     requesting motion.  This method must be called in each task cycle,
     it is reset at the end of the task cycle (in state reflection stage)
     to its natural (non-forced) value.

     @sa isTryingToMove()
   **/
  void forceTryingToMove(void) { myTryingToMove = true; }
  

  /// Gets the number of motor packets received in the last second
  MVREXPORT int getMotorPacCount(void) const;
  /// Gets the number of sonar returns received in the last second
  MVREXPORT int getSonarPacCount(void) const;

  /// Gets the range of the last sonar reading for the given sonar
  /// @ingroup easy
  MVREXPORT int getSonarRange(int num) const;
  /// Find out if the given sonar reading was newly refreshed by the last incoming SIP received.
  MVREXPORT bool isSonarNew(int num) const;
  /// Find the number of sonar sensors (that the robot has yet returned values for)
  /// @ingroup easy
  int getNumSonar(void) const { return myNumSonar; }
  /// Returns the sonar reading for the given sonar
  /// @ingroup easy
  MVREXPORT MvrSensorReading *getSonarReading(int num) const;
  /// Returns the closest of the current sonar reading in the given range
  /// @ingroup easy
  MVREXPORT int getClosestSonarRange(double startAngle, double endAngle) const;
  /// Returns the number of the sonar that has the closest current reading in the given range
  /// @ingroup easy
  MVREXPORT int getClosestSonarNumber(double startAngle, double endAngle) const;

  /// Gets the robots name in ARIAs list
  MVREXPORT const char *getName(void) const;
  /// Sets the robots name in ARIAs list
  MVREXPORT void setName(const char *name);

  /// Change stored pose (i.e. the value returned by getPose())
  MVREXPORT void moveTo(MvrPose pose, bool doCumulative = true);
  /// Change stored pose (i.e. the value returned by getPose()) 
  MVREXPORT void moveTo(MvrPose to, MvrPose from, bool doCumulative = true);
  /// Manually sets the current percentage that the robot is charged (argument is percentage, as a number between 0 and 100)
  MVREXPORT void setStateOfCharge(double stateOfCharge);
  /// Sets the state of charge (percentage) that is considered to be low
  void setStateOfChargeLow(double stateOfChargeLow) 
    { myStateOfChargeLow = stateOfChargeLow; }
  /// Sets the state of charge that will cause a shutdown
  void setStateOfChargeShutdown(double stateOfChargeShutdown) 
    { myStateOfChargeShutdown = stateOfChargeShutdown; }
  /// Sets the charge state (for use with setting the state of charge)
  MVREXPORT void setChargeState(MvrRobot::ChargeState chargeState);
  /// Sets if we're on the charger (for use with setting the state of charge)
  MVREXPORT void setIsChargerPowerGood(bool onCharger);

  /// Sets things so that the battery info from the microcontroller
  /// will be ignored
  MVREXPORT void setIgnoreMicroControllerBatteryInfo(
	  bool ignoreMicroControllerBatteryInfo);
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
  
  /// Gets the temperature of the robot, -128 if not available, -127 to 127 otherwise
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
  MVREXPORT void setEncoderTransform(MvrPose deadReconPos,
				    MvrPose globalPos);

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
  void setMTXTimeUSecCB(MvrRetFunctor1<bool, 
					       MvrTypes::UByte4 *> *functor)
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
  double getTripOdometerDegrees(void) 
    { return myTripOdometerDegrees; }

  /// This gets the time since the "Trip Odometer" was reset (sec)
  /// @see getTripOdometerDistance()
  double getTripOdometerTime(void) 
    { return myTripOdometerStart.secSince(); }

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
  double getOdometerDistance(void) 
    { return myOdometerDistance; }
  /// @see getOdometerDistance
  double getOdometerDistanceMeters(void)
    { return myOdometerDistance/1000.0; }

  /// This gets the total cumulative number of degrees the robot has turned (deg)
  /// This is a virtual odometer (by analogy with a car odometer) that measures the total linear distance the
  /// robot has travelled since ARIA connected, approximated by adding linear
  /// distance between current pose and previous pose in each MvrRobot cycle.
  /// @note This value is not related to the
  /// robot's actual odometry sensor or wheel encoders. For position based on that
  /// plus possible additional correction, see getPose(). For raw encoder count
  /// data, see requestEncoderPackets() instead. 
  double getOdometerDegrees(void) 
    { return myOdometerDegrees; }

  /// This gets the time since the robot started (sec)
  double getOdometerTime(void) 
    { return myOdometerStart.secSince(); }
  /// This gets the time since the robot started (mins)
  double getOdometerTimeMinutes()
    { return myOdometerStart.secSince() / 60.0; }
  


  /// Adds a rangeDevice to the robot's list of them, and set the MvrRangeDevice
  /// object's robot pointer to this MvrRobot object.
  MVREXPORT void addRangeDevice(MvrRangeDevice *device);
  /// Remove a range device from the robot's list, by name
  MVREXPORT void remRangeDevice(const char *name);
  /// Remove a range device from the robot's list, by instance
  MVREXPORT void remRangeDevice(MvrRangeDevice *device);

  /// Finds a rangeDevice in the robot's list
  MVREXPORT const MvrRangeDevice *findRangeDevice(const char *name, 
						bool ignoreCase = false) const;

  /// Finds a rangeDevice in the robot's list
  MVREXPORT MvrRangeDevice *findRangeDevice(const char *name, 
					  bool ignoreCase = false);

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



  /// Adds a sonar to the robot's map of them
  /// @internal (MvrSonarConnector/MvrRobotConnector will automatically and MTX
  /// sonar interfaces upon connection)
  MVREXPORT bool addSonar(MvrSonarMTX *sonar, int sonarNumber);
  /// Remove a sonar from the robot's list, by instance
  /// @internal
  MVREXPORT bool remSonar(MvrSonarMTX *sonar);
  /// Remove a sonar from the robot's list, by number
  /// @internal
  MVREXPORT bool remSonar(int sonarNumber);

  /// Finds a sonar in the robot's list (@a sonarNumber indices start at 1)
  /// (MTX robots only)
  MVREXPORT const MvrSonarMTX *findSonar(int sonarNumber) const;

  /// Finds a sonar in the robot's list (@a sonarNumber indices start at 1)
  /// (MTX robots only)
  MVREXPORT MvrSonarMTX *findSonar(int sonarNumber);

  /// Gets the sonar list
  /// (MTX robots only)
  MVREXPORT const std::map<int, MvrSonarMTX *> *getSonarMap(void) const;

  /// Gets the sonar list
  /// (MTX robots only)
  MVREXPORT std::map<int, MvrSonarMTX *> *getSonarMap(void);

  /// Finds whether a particular sonar is attached to this robot or not
  /// (MTX robots only)
  MVREXPORT bool hasSonar(MvrSonarMTX *device) const;

  /// Associates an MvrPTZ object with this robot (see getPTZ())
  void setPTZ(MvrPTZ *ptz) { myPtz = ptz; }
  /// Get PTZ interface associated with this robot.
  MvrPTZ *getPTZ(void) { return myPtz; }

  /// Sets the number of milliseconds between state reflection refreshes
  /// if the state has not changed 
  MVREXPORT void setStateReflectionRefreshTime(int msec);

  /// Gets the number of milliseconds between state reflection refreshes
  /// if the state has not changed 
  MVREXPORT int getStateReflectionRefreshTime(void) const;

  /// Adds a packet handler to the list of packet handlers
  MVREXPORT void addPacketHandler(
	  MvrRetFunctor1<bool, MvrRobotPacket *> *functor, 
	  MvrListPos::Pos position = MvrListPos::LAST);
  
  /// Removes a packet handler from the list of packet handlers
  MVREXPORT void remPacketHandler(
	  MvrRetFunctor1<bool, MvrRobotPacket *> *functor);

  /// Adds a connect callback
  MVREXPORT void addConnectCB(MvrFunctor *functor, 
			     MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a connect callback
  MVREXPORT void remConnectCB(MvrFunctor *functor);

  /// Adds a callback for when a connection to the robot is failed
  MVREXPORT void addFailedConnectCB(MvrFunctor *functor, 
				   MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback for when a connection to the robot is failed
  MVREXPORT void remFailedConnectCB(MvrFunctor *functor);

  /// Adds a callback for when disconnect is called while connected
  MVREXPORT void addDisconnectNormallyCB(MvrFunctor *functor, 
				MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback for when disconnect is called while connected
  MVREXPORT void remDisconnectNormallyCB(MvrFunctor *functor);
  
  /// Adds a callback for when disconnection happens because of an error
  MVREXPORT void addDisconnectOnErrorCB(MvrFunctor *functor, 
				   MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback for when disconnection happens because of an error
  MVREXPORT void remDisconnectOnErrorCB(MvrFunctor *functor);

  /// Adds a callback for when the run loop exits for what ever reason
  MVREXPORT void addRunExitCB(MvrFunctor *functor, 
			     MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback for when the run loop exits for what ever reason
  MVREXPORT void remRunExitCB(MvrFunctor *functor);

  /// Suspend calling thread until the MvrRobot is connected
  /// @ingroup easy
  MVREXPORT WaitState waitForConnect(unsigned int msecs=0);
  /// Suspend calling thread until the MvrRobot is connected or fails to connect
  MVREXPORT WaitState waitForConnectOrConnFail(unsigned int msecs=0);
  /// Suspend calling thread until the MvrRobot run loop has exited
  /// @ingroup easy
  MVREXPORT WaitState waitForRunExit(unsigned int msecs=0);

  /// Wake up all threads waiting on this robot
  MVREXPORT void wakeAllWaitingThreads();
  /// Wake up all threads waiting for connection
  MVREXPORT void wakeAllConnWaitingThreads();
  /// Wake up all threads waiting for connection or connection failure
  MVREXPORT void wakeAllConnOrFailWaitingThreads();
  /// Wake up all threads waiting for the run loop to exit
  MVREXPORT void wakeAllRunExitWaitingThreads();

  /// Adds a user task to the list of synchronous taskes
  MVREXPORT bool addUserTask(const char *name, int position, 
			       MvrFunctor *functor,
			       MvrTaskState::State *state = NULL);


  /// Removes a user task from the list of synchronous taskes by name
  MVREXPORT void remUserTask(const char *name);
  /// Removes a user task from the list of synchronous taskes by functor
  MVREXPORT void remUserTask(MvrFunctor *functor);

  /// Finds a user task by name
  MVREXPORT MvrSyncTask *findUserTask(const char *name);
  /// Finds a user task by functor
  MVREXPORT MvrSyncTask *findUserTask(MvrFunctor *functor);
  
  /// Logs the list of user tasks, strictly for your viewing pleasure
  MVREXPORT void logUserTasks(void) const;
  /// Logs the list of all tasks, strictly for your viewing pleasure
  MVREXPORT void logAllTasks(void) const;

  /// Adds a sensor interpretation task. These are called during the MvrRobot
  /// task synchronous cycle after robot data has been received (from the SIP
  /// and other robot packets).
  MVREXPORT bool addSensorInterpTask(const char *name, int position, 
				       MvrFunctor *functor,
	       			       MvrTaskState::State *state = NULL);


  /// Removes a sensor interp tasks by name
  MVREXPORT void remSensorInterpTask(const char *name);
  /// Removes a sensor interp tasks by functor
  MVREXPORT void remSensorInterpTask(MvrFunctor *functor);

  /// Finds a task by name
  MVREXPORT MvrSyncTask *findTask(const char *name);
  /// Finds a task by functor
  MVREXPORT MvrSyncTask *findTask(MvrFunctor *functor);

  /// Adds an action to the list with the given priority 
  /// @ingroup easy
  MVREXPORT bool addAction(MvrAction *action, int priority);
  /// Removes an action from the list, by pointer
  /// @ingroup easy
  MVREXPORT bool remAction(MvrAction *action);
  /// Removes an action from the list, by name
  /// @ingroup easy
  MVREXPORT bool remAction(const char *actionName);
  /// Returns the first (highest priority) action with the given name (or NULL)
  MVREXPORT MvrAction *findAction(const char *actionName);
  /// Returns the map of actions... don't do this unless you really
  /// know what you're doing
  MVREXPORT MvrResolver::ActionMap *getActionMap(void);
  /// Deactivates all the actions
  MVREXPORT void deactivateActions(void);

  /// Logs out the actions and their priorities
  /// @ingroup easy
  MVREXPORT void logActions(bool logDeactivated = false) const;

  /// Gets the resolver the robot is using
  MVREXPORT MvrResolver *getResolver(void);

  /// Sets the resolver the robot is using
  MVREXPORT void setResolver(MvrResolver *resolver);

  /// Sets the encoderCorrectionCallback
  MVREXPORT void setEncoderCorrectionCallback(
	  MvrRetFunctor1<double, MvrPoseWithTime> *functor);
  /// Gets the encoderCorrectionCallback
  MVREXPORT MvrRetFunctor1<double, MvrPoseWithTime> *
          getEncoderCorrectionCallback(void) const;
  
  /// Gets the time the connection to the robot was made
  MvrTime getConnectionOpenedTime(void) const 
    { return myConnectionOpenedTime; }

  // set up some of the internals of how the MvrRobot class works
  /// Sets the number of ms between cycles
  MVREXPORT void setCycleTime(unsigned int ms);
  /// Gets the number of ms between cycles
  MVREXPORT unsigned int getCycleTime(void) const;
  /// Sets the number of ms between cycles to warn over
  MVREXPORT void setCycleWarningTime(unsigned int ms);
  /// Gets the number of ms between cycles to warn over
  MVREXPORT unsigned int getCycleWarningTime(void) const;
  /// Gets the number of ms between cycles to warn over
  MVREXPORT unsigned int getCycleWarningTime(void);
  /// Sets the multiplier for how many cycles MvrRobot waits when connecting
  MVREXPORT void setConnectionCycleMultiplier(unsigned int multiplier);
  /// Gets the multiplier for how many cycles MvrRobot waits when connecting
  MVREXPORT unsigned int getConnectionCycleMultiplier(void) const;
  
  /// Sets whether to chain the robot cycle to when we get in SIP packets
  void setCycleChained(bool cycleChained) { myCycleChained = cycleChained; }
  /// Gets whether we chain the robot cycle to when we get in SIP packets
  bool isCycleChained(void) const { return myCycleChained; }
  /// Sets the time without a response until connection assumed lost (threadsafe)
  MVREXPORT void setConnectionTimeoutTime(int mSecs);
  /// Gets the time without a response until connection assumed lost (threadsafe)
  MVREXPORT int getConnectionTimeoutTime(void);
  /// Gets the time the last packet was received (threadsafe)
  MVREXPORT MvrTime getLastPacketTime(void);
  /// Gets the time the last odometry was received (threadsafe)
  MVREXPORT MvrTime getLastOdometryTime(void);

  /// Sets the number of packets back in time the MvrInterpolation goes
  void setPoseInterpNumReadings(size_t numReadings) 
    { myInterpolation.setNumberOfReadings(numReadings); }

  /// Sets the number of packets back in time the position interpol goes
  size_t getPoseInterpNumReadings(void) const
    { return myInterpolation.getNumberOfReadings(); }
  
  
  /// Gets the position the robot was at at the given timestamp
  /** @see MvrInterpolation::getPose 
   */
  int getPoseInterpPosition(MvrTime timeStamp, MvrPose *position,
				     MvrPoseWithTime *mostRecent = NULL)
    { return myInterpolation.getPose(timeStamp, position, mostRecent); }

  /// Gets the pose interpolation object, this should only really used internally
  MvrInterpolation *getPoseInterpolation(void)
    { return &myInterpolation; }

  /// Gets the callback that will call getPoseInterpPosition
  MvrRetFunctor3<int, MvrTime, MvrPose *, MvrPoseWithTime *> *
  getPoseInterpPositionCallback(void)
  { 
    return &myPoseInterpPositionCB; 
  }

  /// Sets the number of packets back in time the MvrInterpolation goes for encoder readings
  void setEncoderPoseInterpNumReadings(size_t numReadings) 
    { myEncoderInterpolation.setNumberOfReadings(numReadings); }

  /// Sets the number of packets back in time the encoder position interpolation goes
  size_t getEncoderPoseInterpNumReadings(void) const
    { return myEncoderInterpolation.getNumberOfReadings(); }

  /// Gets the encoder position the robot was at at the given timestamp
  /** @see MvrInterpolation::getPose 
   */
  int getEncoderPoseInterpPosition(MvrTime timeStamp, MvrPose *position, 
					    MvrPoseWithTime *mostRecent = NULL)
    { return myEncoderInterpolation.getPose(timeStamp, position, mostRecent); }

  MvrInterpolation *getEncoderPoseInterpolation(void)
    { return &myEncoderInterpolation; }

  /// Gets the callback that will call getPoseInterpPosition
  MvrRetFunctor3<int, MvrTime, MvrPose *, MvrPoseWithTime *> *
  getEncoderPoseInterpPositionCallback(void)
  { 
    return &myEncoderPoseInterpPositionCB; 
  }

  /// Applies the encoder motion from the passed in MvrPoseWithTime to the passed in MvrTime
  MVREXPORT int applyEncoderOffset(MvrPoseWithTime from, MvrTime to, 
				  MvrPose *result);

  /// Gets the Counter for the time through the loop
  unsigned int getCounter(void) const { return myCounter; }

  /// Gets the parameters the robot is using
  MVREXPORT const MvrRobotParams *getRobotParams(void) const;

  /// Sets if a config packet is requried or not... 
  /** By default it is not required, since some ancient robots have no
      config packets... some of the modern software may die hideously
      if there is no config packet, so you should set this to true if
      you're using modern robots (basically anything other than a
      pioneer 1)
  **/
  void setRequireConfigPacket(bool requireConfigPacket)
    { myRequireConfigPacket = requireConfigPacket; }

  /// Gets the original robot config packet information
  MVREXPORT const MvrRobotConfigPacketReader *getOrigRobotConfig(void) const;

  /// Gets the battery packet reader
  MvrRobotBatteryPacketReader *getBatteryPacketReader(void) 
    { return myBatteryPacketReader; }

  /// Sets the maximum translational velocity
  MVREXPORT void setTransVelMax(double vel);
  /// Sets the maximum translational velocity
  MVREXPORT void setTransNegVelMax(double negVel);
  /// Sets the translational acceleration
  MVREXPORT void setTransAccel(double acc);
  /// Sets the translational acceleration
  MVREXPORT void setTransDecel(double decel);
  /// Sets the maximum rotational velocity
  MVREXPORT void setRotVelMax(double vel);
  /// Sets the rotational acceleration
  MVREXPORT void setRotAccel(double acc);
  /// Sets the rotational acceleration
  MVREXPORT void setRotDecel(double decel);
  /// Sets the maximum lateral velocity
  MVREXPORT void setLatVelMax(double vel);
  /// Sets the lateral acceleration
  MVREXPORT void setLatAccel(double acc);
  /// Sets the lateral acceleration
  MVREXPORT void setLatDecel(double decel);

  /// If the robot has settable maximum velocities
  bool hasSettableVelMaxes(void) const 
    { return myParams->hasSettableVelMaxes(); }
  /// Gets the maximum translational velocity
  MVREXPORT double getTransVelMax(void) const;
  /// Gets the maximum translational velocity
  MVREXPORT double getTransNegVelMax(void) const;
  /// Gets the maximum rotational velocity
  MVREXPORT double getRotVelMax(void) const;
  /// If the robot has settable accels and decels
  bool hasSettableAccsDecs(void)
      const { return myParams->hasSettableAccsDecs(); }
  /// Gets the translational acceleration
  MVREXPORT double getTransAccel(void) const;
  /// Gets the translational acceleration
  MVREXPORT double getTransDecel(void) const;
  /// Gets the rotational acceleration
  MVREXPORT double getRotAccel(void) const;
  /// Gets the rotational acceleration
  MVREXPORT double getRotDecel(void) const;

  /// Gets the maximum lateral velocity
  MVREXPORT double getLatVelMax(void) const;
  /// Gets the lateral acceleration
  MVREXPORT double getLatAccel(void) const;
  /// Gets the lateral acceleration
  MVREXPORT double getLatDecel(void) const;

  /// Loads a parameter file (replacing all other params)
  MVREXPORT bool loadParamFile(const char *file);
  /// Sets the robot to use a passed in set of params (passes ownership)
  MVREXPORT void setRobotParams(MvrRobotParams *params);

  /// Attachs a key handler
  MVREXPORT void attachKeyHandler(MvrKeyHandler *keyHandler,
				 bool exitOnEscape = true,
				 bool useExitNotShutdown = true);
  /// Gets the key handler attached to this robot
  MVREXPORT MvrKeyHandler *getKeyHandler(void) const;

  /// Lock the robot instance
  int lock() {return(myMutex.lock());}
  /// Try to lock the robot instance without blocking
  int tryLock() {return(myMutex.tryLock());}
  /// Unlock the robot instance
  int unlock() {return(myMutex.unlock());}
  /// Turn on verbose locking of robot mutex
  void setMutexLogging(bool v) { myMutex.setLog(v); }
  /// Set robot lock warning time (see MvrMutex::setLockWarningTime())
  void setMutexLockWarningTime(double sec) { myMutex.setLockWarningTime(sec); }
  /// Set robot lock-unlock warning time (see MvrMutex::setUnlockWarningTime())
  void setMutexUnlockWarningTime(double sec) { myMutex.setUnlockWarningTime(sec); }

  /// This tells us if we're in the preconnection state
  bool isStabilizing(void) { return myIsStabilizing; }

  /// How long we should stabilize for in ms (0 disables stabilizing)
  MVREXPORT void setStabilizingTime(int mSecs);

  /// How long we stabilize for in ms (0 means no stabilizng)
  MVREXPORT int getStabilizingTime(void) const;


  /// Adds a callback called when the robot starts stabilizing before declaring connection
  MVREXPORT void addStabilizingCB(MvrFunctor *functor, 
			     MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes stabilizing callback
  MVREXPORT void remStabilizingCB(MvrFunctor *functor);
  
  /// This gets the root of the syncronous task tree, only serious 
  /// developers should use it
  MVREXPORT MvrSyncTask *getSyncTaskRoot(void);

  /// This function loops once...  only serious developers should use it
  MVREXPORT void loopOnce(void);

  /// Sets the delay in the odometry readings
  /**
     Note that this doesn't cause a delay, its informational so that
     the delay can be adjusted for and causes nothing to happen in
     this class.
  **/
  void setOdometryDelay(int msec) { myOdometryDelay = msec; }

  /// Gets the delay in odometry readings
  /**
     This gets the odometry delay, not that this is just information
     about what the delay is it doesn't cause anything to happen in
     this class.
  **/
  int getOdometryDelay(void) { return myOdometryDelay; }

  /// Gets if we're logging all the movement commands sent down
  bool getLogMovementSent(void) { return myLogMovementSent; }
  /// Sets if we're logging all the movement commands sent down
  void setLogMovementSent(bool logMovementSent)
    { myLogMovementSent = logMovementSent; }

  /// Gets if we're logging all the positions received from the robot
  bool getLogMovementReceived(void) { return myLogMovementReceived; }
  /// Sets if we're logging all the positions received from the robot
  void setLogMovementReceived(bool logMovementReceived)
    { myLogMovementReceived = logMovementReceived; }

  /// Gets if we're logging all the velocities (and heading) received
  bool getLogVelocitiesReceived(void) 
    { return myLogVelocitiesReceived; }
  /// Sets if we're logging all the velocities (and heading) received
  void setLogVelocitiesReceived(bool logVelocitiesReceived) 
    { myLogVelocitiesReceived = logVelocitiesReceived; }

  /// Gets if we're logging all the packets received (just times and types)
  bool getPacketsReceivedTracking(void)
    { return myPacketsReceivedTracking; }
  /// Sets if we're logging all the packets received (just times and types)
  MVREXPORT void setPacketsReceivedTracking(bool packetsReceivedTracking);

  /// Gets if we're logging the contents of the standard SIP (motors packet)
  bool getLogSIPContents() { return myLogSIPContents; }
  /// Set if we're logging the contents of the standard SIP (motor packets)
  void setLogSIPContents(bool logSIP)  { myLogSIPContents = logSIP; }

  /// Gets if we're logging all the packets sent and their payload
  bool getPacketsSentTracking(void)
    { return myPacketsSentTracking; }
  /// Sets if we're logging all the packets sent and their payloads
  void setPacketsSentTracking(bool packetsSentTracking)
		{ 
		if (packetsSentTracking)
			MvrLog::log(MvrLog::Normal, "MvrRobot: tracking packets sent");
		else
			MvrLog::log(MvrLog::Normal, "MvrRobot: not tracking packets sent");
		myPacketsSentTracking = packetsSentTracking; 
		}

  /// Gets if we're logging all the actions as they happen
  bool getLogActions(void) { return myLogActions; }
  /// Sets if we're logging all the actions as they happen
  void setLogActions(bool logActions)
  { myLogActions = logActions; }

  /// Sets the flag that controls if the robot will switch baud rates
  void setDoNotSwitchBaud(bool doNotSwitchBaud)
    { myDoNotSwitchBaud = true; } 

  /// Gets the flag that controls if the robot won't switch baud rates
  bool getDoNotSwitchBaud(void) { return myDoNotSwitchBaud; }
  
  /// This is only for use by syncLoop
  /// @internal
  void incCounter(void) { myCounter++; }

  /// Packet Handler, internal (calls old or new as appropriate)
  /// @internal
  MVREXPORT void packetHandler(void);
  /// Action Handler, internal
  /// @internal
  MVREXPORT void actionHandler(void);
  /// State Reflector, internal
  /// @internal
  MVREXPORT void stateReflector(void);
  /// Robot locker, internal
  /// @internal
  MVREXPORT void robotLocker(void);
  /// Robot unlocker, internal
  /// @internal
  MVREXPORT void robotUnlocker(void);

  /// Packet handler, internal, for use in the syncloop when there's no threading
  /// @internal
  MVREXPORT void packetHandlerNonThreaded(void);
  /// Packet handler, internal, for use in the syncloop to handle the
  /// actual packets
  /// @internal
  MVREXPORT void packetHandlerThreadedProcessor(void);
  /// Packet handler, internal, for using from a thread to actually
  /// read all the packets
  /// @internal
  MVREXPORT void packetHandlerThreadedReader(void);

  /// For the key handler, escape calls this to exit, internal
  MVREXPORT void keyHandlerExit(void);

  /// Processes a motor packet, internal 
  /// @internal 
  MVREXPORT bool processMotorPacket(MvrRobotPacket *packet);
  /// Processes a new sonar reading, internal
  /// @internal
  MVREXPORT void processNewSonar(char number, int range, MvrTime timeReceived);
  /// Processes a new encoder packet, internal
  /// @internal
  MVREXPORT bool processEncoderPacket(MvrRobotPacket *packet);
  /// Processes a new IO packet, internal
  /// @internal
  MVREXPORT bool processIOPacket(MvrRobotPacket *packet);
  
  /// Internal function, shouldn't be used
  /// @internal
  MVREXPORT void init(void);

  /// Internal function, shouldn't be used, sets up the default sync list
  /// @internal
  MVREXPORT void setUpSyncList(void);
  /// Internal function, shouldn't be used, sets up the default packet handlers
  /// @internal
  MVREXPORT void setUpPacketHandlers(void);

  MvrRetFunctor1C<bool, MvrRobot, MvrRobotPacket *> myMotorPacketCB;
  MvrRetFunctor1C<bool, MvrRobot, MvrRobotPacket *> myEncoderPacketCB;
  MvrRetFunctor1C<bool, MvrRobot, MvrRobotPacket *> myIOPacketCB;
  MvrFunctorC<MvrRobot> myPacketHandlerCB;
  MvrFunctorC<MvrRobot> myActionHandlerCB;
  MvrFunctorC<MvrRobot> myStateReflectorCB;
  MvrFunctorC<MvrRobot> myRobotLockerCB;
  MvrFunctorC<MvrRobot> myRobotUnlockerCB;
  MvrFunctorC<MvrRobot> myKeyHandlerExitCB;
  MvrFunctorC<MvrKeyHandler> *myKeyHandlerCB;

  // These four are internal... only people monkeying deeply should mess
  // with them, so they aren't documented... these process the cblists
  // and such
  // Internal function, shouldn't be used, does a single run of connecting
  MVREXPORT int asyncConnectHandler(bool tryHarderToConnect);

  /// Internal function, shouldn't be used, drops the conn because of error
  /// @internal
  MVREXPORT void dropConnection(const char *technicalReason = "Lost connection to the microcontroller because of undefined error.",
			       const char *userReason = NULL);
  /// Internal function that gets the reason the connection dropped
  /// @internal
  const char *getDropConnectionReason(void) { return myDropConnectionReason.c_str(); }
  /// Internal function that gets the reason the connection dropped
  /// @internal
  const char *getDropConnectionUserReason(void) { return myDropConnectionUserReason.c_str(); }
  /// Internal function, shouldn't be used, denotes the conn failed
  /// @internal
  MVREXPORT void failedConnect(void);
  /// Internal function, shouldn't be used, does the initial conn stuff
  /// @internal
  MVREXPORT bool madeConnection(bool resetConnectionTime = true);
  /// Internal function, shouldn't be used, calls the preconnected stuff
  /// @internal
  MVREXPORT void startStabilization(void);
  /// Internal function, shouldn't be used, does the after conn stuff
  /// @internal
  MVREXPORT void finishedConnection(void);
  /// Internal function, shouldn't be used, cancels the connection quietly
  /// @internal
  MVREXPORT void cancelConnection(void);

  /// Internal function, takes a packet and passes it to the packet handlers,
  /// returns true if handled, false otherwise
  /// @internal
  MVREXPORT bool handlePacket(MvrRobotPacket *packet);

  MVREXPORT std::list<MvrFunctor *> * getRunExitListCopy();
  // Internal function, processes a parameter file
  MVREXPORT void processParamFile(void);

  /** @brief Get the position of the robot according to the last robot SIP only,
   *  with no correction by the gyro, other devices or software proceses.
   *
   * @note For the most accurate pose, use getPose() or getEncoderPose(); 
   * only use this method if you must have raw encoder pose with no correction.
   * @sa getPose()
   * @sa getEncoderPose()
   */
  MvrPose getRawEncoderPose(void) const { return myRawEncoderPose; }

  /// Internal function for sync loop and sync task to see if we should warn this cycle or not
  bool getNoTimeWarningThisCycle(void)  
    { return myNoTimeWarningThisCycle; }
  /// Internal function for sync loop and sync task to say if we should warn this cycle or not
  void setNoTimeWarningThisCycle(bool noTimeWarningThisCycle)  
    { myNoTimeWarningThisCycle = noTimeWarningThisCycle; }
  // callbacks for warning time and if we should warn now to pass to sync tasks
  MvrRetFunctorC<unsigned int, MvrRobot> myGetCycleWarningTimeCB;
  MvrRetFunctorC<bool, MvrRobot> myGetNoTimeWarningThisCycleCB;
  /// internal function called when Mvria::exit is called
  /// @internal
  MVREXPORT void mvriaExitCallback(void);
  // internal call that will let the robot connect even if it can't find params
  void setConnectWithNoParams(bool connectWithNoParams) 
    { myConnectWithNoParams = connectWithNoParams; }
  // Internal call that will get the thread the robot is running in
  MvrThread::ThreadType getOSThread(void) { return mySyncLoop.getOSThread(); }
  // Internal call that will get the async connect state
  int getAsyncConnectState(void) { return myAsyncConnectState; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  /// @internal
  MvrRobotPacketSender *getPacketSender(void)
    { return &mySender; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  /// @internal
  MvrRobotPacketReceiver *getPacketReceiver(void)
    { return &myReceiver; }

  // Gets a pointer to the robot parameters in an internal way so they can be modified (only for internal use)
  MVREXPORT MvrRobotParams *getRobotParamsInternal(void);

  // Sets if we've received the first encoder pose for very internal usage
  void internalSetFakeFirstEncoderPose(bool fakeFirstEncoderPose)
    { myFakeFirstEncoderPose = fakeFirstEncoderPose; }
  // Sets a flag to ignore the next packet handled, for very internal usage
  MVREXPORT void internalIgnoreNextPacket(void);
protected:
  enum RotDesired { 
    ROT_NONE,
    ROT_IGNORE,
    ROT_HEADING,
    ROT_VEL
  };
  enum TransDesired {
    TRANS_NONE,
    TRANS_IGNORE,
    TRANS_VEL,
    TRANS_VEL2,
    TRANS_DIST,
    TRANS_DIST_NEW
  };
  enum LatDesired {
    LAT_NONE,
    LAT_IGNORE,
    LAT_VEL
  };
  void reset(void);
 

  // the config the robot had at connection
  MvrRobotConfigPacketReader *myOrigRobotConfig;
  // the reader for the battery packet
  MvrRobotBatteryPacketReader *myBatteryPacketReader;
  // the values we'll maintain for the different motion parameters
  double myRotVelMax;
  double myRotAccel;
  double myRotDecel;
  double myTransVelMax;
  double myTransNegVelMax;
  double myTransAccel;
  double myTransDecel;
  double myLatVelMax;
  double myLatAccel;
  double myLatDecel;

  MvrPTZ *myPtz;
  bool myNoTimeWarningThisCycle;

  long int myLeftEncoder;
  long int myRightEncoder;
  bool myFirstEncoderPose;
  bool myFakeFirstEncoderPose;
  MvrPoseWithTime myRawEncoderPose;

  MvrTransform myEncoderTransform;
  MvrCallbackList mySetEncoderTransformCBList;

  MvrRetFunctor1<bool, MvrTypes::UByte4 *> *myMTXTimeUSecCB;

  bool myIgnoreNextPacket;

  bool myLogMovementSent;
  bool myLogMovementReceived;
  bool myPacketsReceivedTracking;
  bool myLogActions;
  bool myLogVelocitiesReceived;
  bool myLogSIPContents;

  double myLastVel;
  double myLastRotVel;
  double myLastHeading;
  double myLastDeltaHeading;
  double myLastLatVel;

  bool myTryingToMove;

  long myPacketsReceivedTrackingCount;
  MvrTime myPacketsReceivedTrackingStarted;
  bool myPacketsSentTracking;
  MvrMutex myMutex;
  MvrSyncTask *mySyncTaskRoot;
  std::list<MvrRetFunctor1<bool, MvrRobotPacket *> *> myPacketHandlerList;

  MvrSyncLoop mySyncLoop;
  MvrRobotPacketReaderThread myPacketReader;

  // the data items for reading packets in one thread and processing them in another
  MvrMutex myPacketMutex;
  std::list<MvrRobotPacket *> myPacketList;
  MvrCondition myPacketReceivedCondition;
  bool myRunningNonThreaded;


  std::list<MvrFunctor *> myStabilizingCBList;
  std::list<MvrFunctor *> myConnectCBList;
  std::list<MvrFunctor *> myFailedConnectCBList;
  std::list<MvrFunctor *> myDisconnectNormallyCBList;
  std::list<MvrFunctor *> myDisconnectOnErrorCBList;
  std::list<MvrFunctor *> myRunExitCBList;

  MvrRetFunctor1<double, MvrPoseWithTime> *myEncoderCorrectionCB;
  std::list<MvrRangeDevice *> myRangeDeviceList;
  std::map<int, MvrLaser *> myLaserMap;

  std::map<int, MvrBatteryMTX *> myBatteryMap;

  std::map<int, MvrLCDMTX *> myLCDMap;

  std::map<int, MvrSonarMTX *> mySonarMap;

  MvrCondition myConnectCond;
  MvrCondition myConnOrFailCond;
  MvrCondition myRunExitCond;

  MvrResolver::ActionMap myActions;
  bool myOwnTheResolver;
  MvrResolver *myResolver;

  std::map<int, MvrSensorReading *> mySonars;
  int myNumSonar;
  
  unsigned int myCounter;
  bool myIsConnected;
  bool myIsStabilizing;
  bool myRequireConfigPacket;

  bool mySonarEnabled;
  bool myAutonomousDrivingSonarEnabled;

  bool myBlockingConnectRun;
  bool myAsyncConnectFlag;
  int myAsyncConnectState;
  int myAsyncConnectNoPacketCount;
  int myAsyncConnectTimesTried;
  MvrTime myAsyncStartedConnection;
  int myAsyncConnectStartBaud;
  MvrTime myAsyncConnectStartedChangeBaud;
  bool myAsyncConnectSentChangeBaud;
  MvrTime myStartedStabilizing;

  MvrTime myConnectionOpenedTime;

  int myStabilizingTime;

  bool mySentPulse;

  double myTransVal;
  double myTransVal2;
  int myLastTransVal;
  int myLastTransVal2;
  TransDesired myTransType;
  TransDesired myLastTransType;
  MvrTime myTransSetTime;
  MvrTime myLastTransSent;
  int myLastActionTransVal;
  bool myActionTransSet;
  MvrPose myTransDistStart;
  double myMoveDoneDist;

  double myRotVal;
  int myLastRotVal;
  RotDesired myRotType;
  RotDesired myLastRotType;
  MvrTime myRotSetTime;
  MvrTime myLastRotSent;
  int myLastActionRotVal;
  bool myLastActionRotHeading;
  bool myLastActionRotStopped;
  bool myActionRotSet;
  double myHeadingDoneDiff;

  double myLatVal;
  int myLastLatVal;
  LatDesired myLatType;
  LatDesired myLastLatType;
  MvrTime myLatSetTime;
  MvrTime myLastLatSent;
  int myLastActionLatVal;
  bool myActionLatSet;

  double myStoppedVel;
  double myStoppedRotVel;
  double myStoppedLatVel;

  double myLastSentTransVelMax;
  double myLastSentTransAccel;
  double myLastSentTransDecel;
  double myLastSentRotVelMax;
  double myLastSentRotVelPosMax;
  double myLastSentRotVelNegMax;
  double myLastSentRotAccel;
  double myLastSentRotDecel;
  double myLastSentLatVelMax;
  double myLastSentLatAccel;
  double myLastSentLatDecel;

  MvrTime myLastPulseSent;

  int myDirectPrecedenceTime;
  
  int myStateReflectionRefreshTime;

  MvrActionDesired myActionDesired;

  std::string myName;
  std::string myRobotName;
  std::string myRobotType;
  std::string myRobotSubType;

  double myAbsoluteMaxTransVel;
  double myAbsoluteMaxTransNegVel;
  double myAbsoluteMaxTransAccel;
  double myAbsoluteMaxTransDecel;
  double myAbsoluteMaxRotVel;
  double myAbsoluteMaxRotAccel;
  double myAbsoluteMaxRotDecel;
  double myAbsoluteMaxLatVel;
  double myAbsoluteMaxLatAccel;
  double myAbsoluteMaxLatDecel;

  MvrDeviceConnection *myConn;

  MvrRobotPacketSender mySender;
  MvrRobotPacketReceiver myReceiver;

  std::string myDropConnectionReason;
  std::string myDropConnectionUserReason;

  MvrRobotParams *myParams;
  double myRobotLengthFront;
  double myRobotLengthRear;

  MvrInterpolation myInterpolation;
  MvrInterpolation myEncoderInterpolation;

  MvrKeyHandler *myKeyHandler;
  bool myKeyHandlerUseExitNotShutdown;

  bool myConnectWithNoParams;
  bool myWarnedAboutExtraSonar;

  // vmvriables for tracking the data stream
  time_t myTimeLastMotorPacket;
  int myMotorPacCurrentCount;
  int myMotorPacCount;
  time_t myTimeLastSonarPacket;
  int mySonarPacCurrentCount;
  int mySonarPacCount;
  unsigned int myCycleTime;
  unsigned int myCycleWarningTime;
  unsigned int myConnectionCycleMultiplier;
  bool myCycleChained;
  MvrTime myLastPacketReceivedTime;
  MvrTime myLastOdometryReceivedTime;
  int myTimeoutTime;
  MvrMutex myConnectionTimeoutMutex;

  bool myRequestedIOPackets;
  bool myRequestedEncoderPackets;

  // all the state reflecing vmvriables
  MvrPoseWithTime myEncoderPose;
  MvrTime myEncoderPoseTaken;
  MvrPose myGlobalPose;
  MvrPose myLastGlobalPose;
  // um, this myEncoderGlobalTrans doesn't do anything
  MvrTransform myEncoderGlobalTrans;
  double myLeftVel;
  double myRightVel;
  bool myIgnoreMicroControllerBatteryInfo;
  double myBatteryVoltage;
  MvrRunningAverage myBatteryAverager;
  double myRealBatteryVoltage;
  MvrRunningAverage myRealBatteryAverager;
  bool myHaveStateOfCharge;
  double myStateOfCharge;
  MvrTime myStateOfChargeSetTime;
  double myStateOfChargeLow;
  double myStateOfChargeShutdown;

  int myStallValue;
  double myControl;
  bool myKeepControlRaw;
  int myFlags;
  int myFaultFlags;
  bool myHasFaultFlags;
  int myFlags3;
  bool myHasFlags3;
  double myCompass;
  int myAnalogPortSelected;
  unsigned char myAnalog;
  unsigned char myDigIn;
  unsigned char myDigOut;
  int myIOAnalog[128];
  unsigned char myIODigIn[255];
  unsigned char myIODigOut[255];
  int myIOAnalogSize;
  int myIODigInSize;
  int myIODigOutSize;
  MvrTime myLastIOPacketReceivedTime;
  double myVel;
  double myRotVel;
  double myLatVel;
  int myLastX;
  int myLastY;
  int myLastTh;
  ChargeState myChargeState;
  bool myOverriddenChargeState;
  bool myIsChargerPowerGood;
  bool myOverriddenIsChargerPowerGood;
  
  char myTemperature;

  double myTripOdometerDistance;
  double myTripOdometerDegrees;
  MvrTime myTripOdometerStart;

  double myOdometerDistance;
  double myOdometerDegrees;
  MvrTime myOdometerStart;
  
  int myOdometryDelay;

  bool myDoNotSwitchBaud;
  
  bool myAddedMvrExitCB;
  MvrFunctorC<MvrRobot> myMvrExitCB;

  MvrRetFunctor3C<int, MvrRobot, MvrTime, MvrPose *, 
		 MvrPoseWithTime *> myPoseInterpPositionCB;
  MvrRetFunctor3C<int, MvrRobot, MvrTime, MvrPose *, 
		 MvrPoseWithTime *> myEncoderPoseInterpPositionCB;
};


#endif // ARROBOT_H
