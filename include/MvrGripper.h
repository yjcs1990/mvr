/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrGripper.h
 > Description  : Contains gripper command numbers
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRGRIPPER_H
#define MVRGRIPPER_H

#include "mvriaTypedefs.h"
#include "MvrRobot.h"

class MvrGripperCommands
{
public:
  enum Commands {
    GRIP_OPEN = 1, ///< open the gripper paddles fully
    GRIP_CLOSE = 2, ///< close the gripper paddles all the way
    GRIP_STOP = 3, ///< stop the gripper paddles where they are
    LIFT_UP = 4, ///< raises the lift to the top of its range
    LIFT_DOWN = 5, ///< lowers the lift to the bottom of its range
    LIFT_STOP = 6, ///< stops the lift where it is
    GRIPPER_STORE = 7, /**< 
                          closes the paddles and raises the lift simultaneously, 
                          this is for storage not for grasping/carrying an object
                        */
    GRIPPER_DEPLOY = 8, /**< 
                        opens the paddles and lowers the lieft simultaneously,
                        this is for getting ready to grasp an object, not for
                        object drops
                          */
    GRIPPER_HALT = 15, ///< stops the gripper paddles and lift from moving
    GRIP_PRESSURE = 16, /**< 
                        sets the time delay in 20 msec increments after the
                        gripper paddles first grasp an object before they stop
                        moving, regulates grasp pressure
                          */
    LIFT_CMVRRY = 17 /**<
                      raises or lowers the lieft, the argument is the number
                      of 20 msec increments to raise or lower the lift,
                      poseitive arguments for raise, negative for lower
                    */
  };
};

/// Provides an interface to the Pioneer gripper device
///  @ingroup OptionalClasses
///  @ingroup DeviceClasses
class MvrGripper
{
public:
  /// Constructor
  MVREXPORT MvrGripper(MvrRobot *robot, int gripperType = QUERYTYPE);
  /// Destructor
  MVREXPORT virtual ~MvrGripper();
  /// Opens the gripper paddles
  MVREXPORT bool gripOpen(void);
  /// Closes the gripper paddles
  MVREXPORT bool gripClose(void);
  /// Stops the gripper paddles
  MVREXPORT bool gripStop(void);
  /// Raises the lift to the top
  MVREXPORT bool liftUp(void);
  /// Lowers the lift to the bottom
  MVREXPORT bool liftDown(void);
  /// Stops the lift
  MVREXPORT bool liftStop(void);
  /// Puts the gripper in a storage position
  MVREXPORT bool gripperStore(void);
  /// Puts the gripper in a deployed position, ready for use
  MVREXPORT bool gripperDeploy(void);
  /// Halts the lift and the gripper paddles
  MVREXPORT bool gripperHalt(void);
  /// Sets the amount of pressure the gripper applies
  MVREXPORT bool gripPressure(int mSecIntervals);
  /// Raises the lift by a given amount of time
  MVREXPORT bool liftCarry(int mSecIntervals);
  /// Returns true if the gripper paddles are moving
  MVREXPORT bool isGripMoving(void) const;
  /// Returns true if the lift is moving
  MVREXPORT bool isLiftMoving(void) const;
  /// Returns the state of the gripper paddles
  MVREXPORT int getGripState(void) const;
  /// Returns the state of each gripper paddle
  MVREXPORT int getPaddleState(void) const;
  /// Returns the state of the gripper's breakbeams
  MVREXPORT int getBreakBeamState(void) const;
  /// Returns the state of the lift
  MVREXPORT bool isLiftMaxed(void) const;
  /// Gets the type of the gripper
  MVREXPORT int getType(void) const;
  /// Sets the type of the gripper
  MVREXPORT void setType(int type);
  /// Gets the number of mSec since the last gripper packet
  MVREXPORT long getMSecSinceLastPacket(void) const;
  /// Gets the grasp time
  MVREXPORT int getGraspTime(void) const;
  /// logs the gripper state
  MVREXPORT void logState(void) const;
  /// Parses the gripper packet
  MVREXPORT bool packetHandler(MvrRobotPacket *packet);
  /// The handler for when the robot connects
  MVREXPORT void connectHandler(void);
  /// These are the types for the gripper
  enum Type {
    QUERYTYPE, ///< Finds out what type from the robot, default
    GENIO,     ///< Uses general IO
    USERIO,    ///< Uses the user IO
    GRIPPAC,   ///< Uses a packet requested from the robot
    NOGRIPPER  ///< There isn't a gripper
  }; 
protected:
  MvrRobot *myRobot;
  int myType;
  unsigned char myState;
  unsigned char mySwitches;
  unsigned char myGraspTime;
  MvrTime myLastDataTime;
  MvrFunctorC<MvrGripper> myConnectCB;
  MvrRetFunctor1C<bool, MvrGripper, MvrRobotPacket *> myPacketHandlerCB;
};

#endif  // MVRGRIPPER_H