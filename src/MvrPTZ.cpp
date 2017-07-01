#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrPTZ.h"
#include "MvrRobot.h"
#include "MvrRobotPacket.h"
#include "MvrCommands.h"
#include "MvrDeviceConnection.h"

/**
   @param robot The robot this camera is attached to, can be NULL
**/
MVREXPORT MvrPTZ::MvrPTZ(MvrRobot *robot) :
  myRobot(robot),
  myConn(NULL),
  myConnectCB(this, &MvrPTZ::connectHandler),
  mySensorInterpCB(this, &MvrPTZ::sensorInterpHandler),
  myAuxPort(1),
  myAuxTxCmd(MvrCommands::TTY2),
  myAuxRxCmd(MvrCommands::GETAUX),
  myRobotPacketHandlerCB(this, &MvrPTZ::robotPacketHandler),
  myInverted(false),
  myMaxPosPan(90),
  myMaxNegPan(-90),
  myMaxPosTilt(90),
  myMaxNegTilt(0),
  myMaxZoom(100),
  myMinZoom(0)
{
  myRobotPacketHandlerCB.setName("MvrPTZ");
  if (myRobot != NULL)
  {
    myRobot->addConnectCB(&myConnectCB, MvrListPos::LAST);
    myRobot->addPacketHandler(&myRobotPacketHandlerCB, MvrListPos::FIRST);
  }
}

MVREXPORT MvrPTZ::~MvrPTZ()
{
  if (myRobot != NULL)
  {
    myRobot->remConnectCB(&myConnectCB);
    myRobot->remPacketHandler(&myRobotPacketHandlerCB);
    myRobot->remSensorInterpTask(&mySensorInterpCB);
  }
  
}

/**
   @param packet the packet to send
   @return true if the packet could be sent, false otherwise
**/
   
MVREXPORT bool MvrPTZ::sendPacket(MvrBasePacket *packet)
{
  packet->finalizePacket();
  if (myConn != NULL)
    return myConn->write(packet->getBuf(), packet->getLength());
  else if (myRobot != NULL)
    return myRobot->comStrN(myAuxTxCmd, packet->getBuf(), 
			    packet->getLength());
  else
    return false;
}

MVREXPORT bool MvrPTZ::robotPacketHandler(MvrRobotPacket *packet)
{
  //printf("%x\n", packet->getID());
  if ((packet->getID() == 0xb0 && myAuxPort == 1) ||
      (packet->getID() == 0xb8 && myAuxPort == 2) ||
	  (packet->getID() == 200 && myAuxPort == 3) 
     )
    return packetHandler(packet);
  else
    return false;
}

MVREXPORT void MvrPTZ::connectHandler(void)
{
  init();
}

MVREXPORT void MvrPTZ::sensorInterpHandler(void)
{
  MvrBasePacket *packet;
  while ((packet = readPacket()) != NULL)
    packetHandler(packet);
}

/**
   @param connection the device connection the camera is connected to,
   normally a serial port
   
   @param driveFromRobotLoop if this is true then a sensor interp
   callback wil be set and that callback will read packets and call
   the packet handler on them

   @return true if the serial port is opened or can be opened, false
   otherwise
**/
MVREXPORT bool MvrPTZ::setDeviceConnection(MvrDeviceConnection *connection,
					 bool driveFromRobotLoop)
{
  if (myRobot != NULL)
  {
    myRobot->remPacketHandler(&myRobotPacketHandlerCB);
    myRobot->remSensorInterpTask(&mySensorInterpCB);
  }
  myConn = connection;
  if (driveFromRobotLoop && myRobot != NULL && myConn != NULL)
    myRobot->addSensorInterpTask("ptz", 50, &mySensorInterpCB);
  if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
    return myConn->openSimple();
  else
    return true;
}

MVREXPORT MvrDeviceConnection *MvrPTZ::getDeviceConnection(void)
{
  return myConn;
}


/**
 @param auxPort Which auxilliary serial port on the robot's microcontroller that the device
 is connected to.  (The C166 controller only has one port; the H8 has two; the SH has three.)

 @return true if @a auxPort was valid (1, 2 or 3).  False otherwise. If @a auxPort was an invalid number, the previous setting will be retained.

**/
MVREXPORT bool MvrPTZ::setAuxPort(int auxPort)
{
  if (auxPort == 1)
  {
    myAuxTxCmd = MvrCommands::TTY2;
    myAuxRxCmd = MvrCommands::GETAUX;
    myAuxPort = 1;
    return true;
  }
  else if (auxPort == 2)
  {
    myAuxTxCmd = MvrCommands::TTY3;
    myAuxRxCmd = MvrCommands::GETAUX2;
    myAuxPort = 2;
    return true;
  }
  else if(auxPort == 3)
  {
    myAuxTxCmd = MvrCommands::TTY4;
    myAuxRxCmd = MvrCommands::GETAUX3;
    myAuxPort = 3;
    return true;
  }
  else
    return false;
}

