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

  ///
protected:
};
#endif  // MVRROBOT_H