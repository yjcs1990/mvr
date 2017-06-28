/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotBatteryPacketReader.h
 > Description  : This class will read a config packet from the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年06月28日
***************************************************************************************************/
#include "mvriaOSDef.h"
#include "MvrExport.h"
#include "MvrRobot.h"
#include "MvrRobotPacket.h"
#include "MvrCommands.h"
#include "MvrRobotBatteryPacketReader.h"

MVREXPORT MvrRobotBatteryPacketReader::MvrRobotBatteryPacketReader(MvrRobot *robot) :
          myPacketHandlerCB(this, &MvrRobotBatteryPacketReader::packetHandler),
          myConnectCB(this, &MvrRobotBatteryPacketReader::connectCallback)
{
  myRobot = robot;
  myPacketArrived = false;
  myNumBatteries = 0;
  myNumBytesPerBattery = 0;
  myRequestedBatteryPackets = false;
  myPacketHandlerCB.setName("MvrRobotBatteryPacketReader");
  myConnectCB.setName("MvrRobotBatteryPacketReader");
  myRobot->addPacketHandler(&myPacketHandlerCB);
  myRobot->addConnectCB(&myConnectCB);
}          

MVREXPORT MvrRobotBatteryPacketReader::~MvrRobotBatteryPacketReader(void)
{
  myRobot->remPacketHandler(&myPacketHandlerCB);
  myRobot->remConnectCB(&myConnectCB);
}

MVREXPORT void MvrRobotBatteryPacketReader::connectCallback(void)
{
  requestSinglePacket();
}

MVREXPORT void MvrRobotBatteryPacketReader::requestSinglePacket(void)
{
  myRobot->comInt(MvrCommands::BATTERYINFO, 1);
  myRequestedBatteryPackets = false;
}

MVREXPORT void MvrRobotBatteryPacketReader::requestContinuousPackets(void)
{
  myRobot->comInt(MvrCommands::BATTERYINFO, 2);
  myRequestedBatteryPackets = true;
}

MVREXPORT void MvrRobotBatteryPacketReader::stopPackets(void)
{
  myRobot->comInt(MvrCommands::BATTERYINFO, 0);
  myRequestedBatteryPackets = false;
}

MVREXPORT bool MvrRobotBatteryPacketReader::haveRequestedPackets(void)
{
  return myRequestedBatteryPackets;
}

MVREXPORT bool MvrRobotBatteryPacketReader::packetHandler(MvrRobotPacket *packet)
{
  if (packet->getID() != 0xfa)
    return false;
  
  myFlags1.clear();
  myFlags2.clear();
  myFlags3.clear();
  myRelSOC.clear();
  myAbsSOC.clear();

  myNumBatteries = packet->bufToUByte();
  myNumBytesPerBattery = packet->bufToUByte();

  int battery;
  int byte;
  int val;

  for (battery = 1; battery <= myNumBatteries; battery++)
  {
    for (byte = 1; byte <= myNumBytesPerBattery; byte++)
    {
      val = packet->bufToUByte();
      if (byte == 1)
        myFlags1[battery] = val;
      else if (byte == 2)
        myFlags2[battery] = val;
      else if (byte == 3)
        myFlags3[battery] = val;
      else if (byte == 4)
        myRelSOC[battery] = val;
      else if (byte == 5)
        myAbsSOC[battery] = val;
    }
  }
  myPacketArrived = true;
  return true;
}
