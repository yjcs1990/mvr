#ifndef MVRTCMCOMPASSROBOT_H
#define MVRTCMCOMPASSROBOT_H

#include "mvriaUtil.h"
#include "MvrCommands.h"
#include "MvrFunctor.h"
#include "MvrRobot.h"
#include "MvrTCM2.h"

/** Interface to a TCM 2/2.5/2.6 3-axis compass through the robot microcontroller.
 *  When a Pioneer robot is equipped with a TCM compass, it is typically connected
 *  to the robot microcontroller, which returns compass information in 
 *  compass data packets upon request (set compass mode/type to 2 or 3 in
 *  firmware configuration; mode 1 (data in SIP) is not supported by ARIA).  This class communicates
 *  with the robot microcontroller to configure the compass and recieve data
 *  from it. 
 *
*/
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


#endif 
