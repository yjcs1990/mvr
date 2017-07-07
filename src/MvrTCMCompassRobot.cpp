#include "mvriaOSDef.h"
#include "MvrExport.h"
#include "MvrCommands.h"
#include "MvrRobot.h"
#include "MvrTCMCompassRobot.h"

MVREXPORT MvrTCMCompassRobot::MvrTCMCompassRobot(MvrRobot *robot) :
  myPacketHandlerCB(this, &MvrTCMCompassRobot::packetHandler)
{
  myRobot = robot;
  myPacketHandlerCB.setName("MvrTCMCompassRobot");
  if (myRobot != NULL)
    myRobot->addPacketHandler(&myPacketHandlerCB);
}

MVREXPORT MvrTCMCompassRobot::~MvrTCMCompassRobot()
{
  if (myRobot != NULL)
    myRobot->remPacketHandler(&myPacketHandlerCB);
}

bool MvrTCMCompassRobot::packetHandler(MvrRobotPacket *packet)
{
  if (packet->getID() != 0xC0)
    return false;
  
  myHeading = MvrMath::fixAngle(packet->bufToByte2() / 10.0);
  myPitch = MvrMath::fixAngle(packet->bufToByte2() / 10.0);
  myRoll = MvrMath::fixAngle(packet->bufToByte2() / 10.0);
  myXMag = packet->bufToByte2() / 100.0;  
  myYMag = packet->bufToByte2() / 100.0;
  myZMag = packet->bufToByte2() / 100.0;
  myTemperature = packet->bufToByte2() / 10.0;
  myError = packet->bufToByte2();
  myCalibrationH = packet->bufToByte();
  myCalibrationV = packet->bufToByte();
  myCalibrationM = packet->bufToByte2() / 100.0;

  myHaveHeading = 
    myHavePitch = 
    myHaveRoll = 
    myHaveXMag = 
    myHaveYMag = 
    myHaveZMag = 
    myHaveTemperature = 
    myHaveCalibrationH =
    myHaveCalibrationV =
    myHaveCalibrationM = true;

  incrementPacketCount();

  invokeHeadingDataCallbacks(myHeading);
  return true;
}

