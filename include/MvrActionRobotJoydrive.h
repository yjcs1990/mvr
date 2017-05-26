/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionRobotJoydrive.h
 > Description  : This action will use the joystick for input to drive the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月14日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONROBOTJOYDRIVE_H
#define MVRACTIONROBOTJOYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrRobotPacket;

class MvrActionRobotJoydrive : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionRobotJoydrive(const char * name = "robotJoyDrive", 
				                            bool requireDeadmanPushed = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionRobotJoydrive();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  MVREXPORT virtual void setRobot(NvrRobot *robot);
protected:
  MVREXPORT bool handleJoystickPacket(MvrRobotPacket *packet);
  MVREXPORT void connectCallback(void);
  // whether we require the deadman to be pushed to drive
  bool myRequireDeadmanPushed;

  bool myDeadZoneLast;
  int myButton1;
  int myButton2;
  int myJoyX;
  int myJoyY;
  int myThrottle;
  MvrTime myPacketReceivedTime;
  // action desired
  MvrActionDesired myDesired;
  MvrRetFunctor1C<bool, MvrActionRobotJoydrive, 
      MvrRobotPacket *> myHandleJoystickPacketCB;
  MvrFunctorC<MvrActionRobotJoydrive> myConnectCB;
};
#endif  // MVRACTIONROBOTJOYDRIVE_H