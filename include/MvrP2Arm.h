/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrP2Arm.h
 > Description  : P2 Arm classes: MvrP2Arm for control and MvrP2ArmJoints for P2 Arm joint data
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRP2ARM_H
#define MVRP2ARM_H


#include "MvrRobot.h"
#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "mvriaOSDef.h"

/// P2 Arm joint info
class P2ArmJoint
{
public:

  MVRXPORT P2ArmJoint();
  MVRXPORT virtual ~P2ArmJoint();

  MvrTypes::UByte myPos;
  MvrTypes::UByte myVel;
  MvrTypes::UByte myHome;
  MvrTypes::UByte myMin;
  MvrTypes::UByte myCenter;
  MvrTypes::UByte myMax;
  MvrTypes::UByte myTicksPer90;
};

/// Arm Control class
class MvrP2Arm
{
public:

  /// General error conditions possible from most of the arm related functions
  typedef enum {
    SUCCESS, ///< Succeded
    ALREADY_INITED, ///< The class is already initialized
    NOT_INITED, ///< The class is not initialized
    ROBOT_NOT_SETUP, ///< The MvrRobot class is not setup properly
    NO_ARM_FOUND, ///< The arm can not be found
    COMM_FAILED, ///< Communications has failed
    COULD_NOT_OPEN_PORT, ///< Could not open the communications port
    COULD_NOT_SET_UP_PORT, ///< Could not setup the communications port
    ALREADY_CONNECTED, ///< Already connected to the arm
    NOT_CONNECTED, ///< Not connected with the arm, connect first
    INVALID_JOINT, ///< Invalid joint specified
    INVALID_POSITION ///< Invalid position specified
    } State;

  /// Type of arm packet identifiers. Used in MvrP2Arm::setPacketCB().
  typedef enum {
    StatusPacket, ///< The status packet type
    InfoPacket ///< The info packet type
  } PacketType;

  /// Type of status packets to request for. Used in MvrP2Arm::requestStatus()
  typedef enum
  {
    StatusOff=0, ///< Stop sending status packets
    StatusSingle=1, ///< Send a single status packets
    StatusContinuous=2 ///< Send continous packets. Once every 100ms.
  } StatusType;

  /// Bit for joint 1 in arm status byte
  MVRXPORT static const int ArmJoint1;
  /// Bit for joint 2 in arm status byte
  MVRXPORT static const int ArmJoint2;
  /// Bit for joint 3 in arm status byte
  MVRXPORT static const int ArmJoint3;
  /// Bit for joint 4 in arm status byte
  MVRXPORT static const int ArmJoint4;
  /// Bit for joint 5 in arm status byte
  MVRXPORT static const int ArmJoint5;
  /// Bit for joint 6 in arm status byte
  MVRXPORT static const int ArmJoint6;
  /// Bit for arm good state in arm status byte
  MVRXPORT static const int ArmGood;
  /// Bit for arm initialized in arm status byte
  MVRXPORT static const int ArmInited;
  /// Bit for arm powered on in arm status byte
  MVRXPORT static const int ArmPower;
  /// Bit for arm homing in arm status byte
  MVRXPORT static const int ArmHoming;
  /// Number of joints that the arm has
  MVRXPORT static int NumJoints;

  /// Constructor
  MVRXPORT MvrP2Arm();

  /// Destructor
  MVRXPORT virtual ~MvrP2Arm();

  /// Set the robot to use to talk to the arm
  MVRXPORT void setRobot(MvrRobot *robot) {myRobot=robot;}

  /// Init the arm class
  MVRXPORT virtual State init();

  /// Uninit the arm class
  MVRXPORT virtual State uninit();

  /// Power on the arm
  MVRXPORT virtual State powerOn(bool doWait=true);

  /// Power off the arm
  MVRXPORT virtual State powerOff();

  /// Request the arm info packet
  MVRXPORT virtual State requestInfo();

  /// Request the arm status packet
  MVRXPORT virtual State requestStatus(StatusType status);

  /// Request arm initialization
  MVRXPORT virtual State requestInit();

  /// Check to see if the arm is still connected
  MVRXPORT virtual State checkArm(bool waitForResponse=true);

  /// Home the arm
  MVRXPORT virtual State home(int joint=-1);

  /// Home the arm and power if off
  MVRXPORT virtual State park();

  /// Move a joint to a position in degrees
  MVRXPORT virtual State moveTo(int joint, float pos, unsigned char vel=0);

  /// Move a joint to a position in low level arm controller ticks
  MVRXPORT virtual State moveToTicks(int joint, unsigned char pos);

  /// Move a joint step degrees
  MVRXPORT virtual State moveStep(int joint, float pos, unsigned char vel=0);

  /// Move a joint step ticks
  MVRXPORT virtual State moveStepTicks(int joint, signed char pos);

  /// Set the joint to move at the given velocity
  MVRXPORT virtual State moveVel(int joint, int vel);

  /// Stop the arm
  MVRXPORT virtual State stop();

  /// Set the auto park timer value
  MVRXPORT virtual State setAutoParkTimer(int waitSecs);

  /// Set the gripper park timer value
  MVRXPORT virtual State setGripperParkTimer(int waitSecs);

  /// Set the arm stopped callback
  MVRXPORT virtual void setStoppedCB(MvrFunctor *func) {myStoppedCB=func;}

  /// set the arm packet callback
  MVRXPORT virtual void setPacketCB(MvrFunctor1<PacketType> *func)
    {myPacketCB=func;}

  /// Get the arm version
  MVRXPORT virtual std::string getArmVersion() {return(myVersion);}

  /// Get the joints position in degrees
  MVRXPORT virtual float getJointPos(int joint);

  /// Get the joints position in ticks
  MVRXPORT virtual unsigned char getJointPosTicks(int joint);

  /// Check to see if the arm is moving
  MVRXPORT virtual bool getMoving(int joint=-1);

  /// Check to see if the arm is powered
  MVRXPORT virtual bool isPowered();

  /// Check to see if the arm is communicating
  MVRXPORT virtual bool isGood();

  /// Get the two byts of status info from P2OS
  MVRXPORT virtual int getStatus() {return(myStatus);}

  /// Get when the last arm status packet came in
  MVRXPORT virtual MvrTime getLastStatusTime() {return(myLastStatusTime);}

  /// Get the robot that the arm is on
  MVRXPORT virtual MvrRobot * getRobot() {return(myRobot);}

  /// Get the joints data structure
  MVRXPORT virtual P2ArmJoint * getJoint(int joint);

  /// Converts degrees to low level arm controller ticks
  MVRXPORT virtual bool convertDegToTicks(int joint, float pos,
					  unsigned char *ticks);

  /// Converts low level arm controller ticks to degrees
  MVRXPORT virtual bool convertTicksToDeg(int joint, unsigned char pos, float *degrees);


protected:

  // AROS/P2OS parameters
  static const unsigned int ARMpac;
  static const unsigned int ARMINFOpac;
  static const unsigned char ComArmInfo;
  static const unsigned char ComArmStats;
  static const unsigned char ComArmInit;
  static const unsigned char ComArmCheckArm;
  static const unsigned char ComArmPower;
  static const unsigned char ComArmHome;
  static const unsigned char ComArmPark;
  static const unsigned char ComArmPos;
  static const unsigned char ComArmSpeed;
  static const unsigned char ComArmStop;
  static const unsigned char ComArmAutoPark;
  static const unsigned char ComArmGripperPark;

  bool comArmInfo();
  bool comArmStats(StatusType stats=StatusSingle);
  bool comArmInit();
  bool comArmCheckArm();
  bool comArmPower(bool on);
  bool comArmHome(unsigned char joint=0xff);
  bool comArmPark();
  bool comArmPos(unsigned char joint, unsigned char pos);
  bool comArmSpeed(unsigned char joint, unsigned char speed);
  bool comArmStop(unsigned char joint=0xff);
  bool comArmAutoPark(int waitSecs);
  bool comArmGripperPark(int waitSecs);

  bool armPacketHandler(MvrRobotPacket *packet);

  bool myInited;
  MvrRobot *myRobot;
  MvrTime myLastStatusTime;
  MvrTime myLastInfoTime;
  std::string myVersion;
  StatusType myStatusRequest;
  MvrTypes::UByte2 myLastStatus;
  MvrTypes::UByte2 myStatus;
  MvrSerialConnection myCon;
  MvrRetFunctorC<State, MvrP2Arm> myAriaUninitCB;
  MvrRetFunctor1C<bool, MvrP2Arm, MvrRobotPacket*> myArmPacketHandler;
  MvrFunctor1<PacketType> *myPacketCB;
  MvrFunctor *myStoppedCB;

  // We have 6 joints. Including the gripper. It's here so that we can
  // store its position even though its not really a joint.
  P2ArmJoint myJoints[6];
};

#endif  // MVRP2ARM_H