/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTCMCompassRobot.h
 > Description  : Interface to a TCM 2/2.5/2.6 3-axis compass through the robot microcontroller.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/

#ifndef MVRTCMCOMPASSROBOT_H
#define MVRTCMCOMPASSROBOT_H

#include "mvriaUtil.h"
#include "MvrCommands.h"
#include "MvrFunctor.h"
#include "MvrRobot.h"
#include "MvrTCM2.h"

/// Interface to a TCM  3-axis compass through the robot microcontroller.

class MvrTCMCompassRobot : public virtual MvrTCM2
{
public:

  MVREXPORT MvrTCMCompassRobot(MvrRobot *robot);
  MVREXPORT virtual ~MvrTCMCompassRobot();

  virtual void commandOff(void) { myRobot->comInt(MvrCommands::TCM2, 0); }
  virtual void commandJustCompass(void) { myRobot->comInt(MvrCommands::TCM2, 1); }
  virtual void commandOnePacket(void) { myRobot->comInt(MvrCommands::TCM2, 2); }
  virtual void commandContinuousPackets(void) { myRobot->comInt(MvrCommands::TCM2, 3); }
  virtual void commandUserCalibration(void) { myRobot->comInt(MvrCommands::TCM2, 4); }
  virtual void commandAutoCalibration(void) { myRobot->comInt(MvrCommands::TCM2, 5); }
  virtual void commandStopCalibration(void) { myRobot->comInt(MvrCommands::TCM2, 6); }
  virtual void commandSoftReset(void) { myRobot->comInt(MvrCommands::TCM2, 7); }

private:  
  MvrRobot *myRobot;
  MvrRetFunctor1C<bool, MvrTCMCompassRobot, MvrRobotPacket*> myPacketHandlerCB;
  bool packetHandler(MvrRobotPacket *packet);
};
#endif // MVRTCMCOMPASSROBOT_H