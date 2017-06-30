/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotJoyHandler.cpp
 > Description  : Interfaces to a joystick on the robot's microcontroller
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年06月22日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRobotJoyHandler.h"
#include "MvrRobot.h"
#include "MvrCommands.h"
#include "mvriaInternal.h"

MVREXPORT MvrRobotJoyHandler::MvrRobotJoyHandler(MvrRobot *robot) : 
          myHandleJoystickPacketCB(this, &MvrRobotJoyHandler::handleJoystickPacket),
          myConnectCB(this, &MvrRobotJoyHandler::connectCallback),
          myStopPacketsCB(this, &MvrRobotJoyHandler::stopPackets)
{
  myRobot = robot;

  myHandleJoystickPacketCB.setName("MvrRobotJoyHandler");
  myRobot->addConnectCB(&myConnectCB);
  myRobot->addPacketHandler(&myHandleJoystickPacketCB, MvrListPos::FIRST);
  if (myRobot->isConnected())
    connectCallback();

  myStarted.setToNow();
  myButton1 = 0;
  myButton2 = 0;
  myJoyX = 0;
  myJoyY = 0;

  myJoyXCenter = 512.0;
  myJoyYCenter = 512.0;

  myRawX = -1;
  myRawX = -1;
  myRawX = -1;

  myThrottle = 1;
  myGotData = false;
}

MVREXPORT MvrRobotJoyHandler::~MvrRobotJoyHandler()
{
  myRobot->remConnectCB(&myConnectCB);
  myRobot->remPacketHandler(&myHandleJoystickPacketCB);
  stopPackets();
  myRobot->remDisconnectNormallyCB(&myStopPacketsCB);
  Mvria::remExitCallback(&myStopPacketsCB);
}

MVREXPORT void MvrRobotJoyHandler::connectCallback(void)
{
  myRobot->addDisconnectNormallyCB(&myStopPacketsCB);
  Mvria::addExitCallback(&myStopPacketsCB);
  myRobot->comInt(MvrCommands::JOYINFO, 2);
}

void MvrRobotJoyHandler::stopPackets()
{
  myRobot->comInt(MvrCommands::JOYINFO, 0);
}

MVREXPORT bool MvrRobotJoyHandler::handleJoystickPacket(MvrRobotPacket *packet)
{

  if (packet->getID() != 0xF8)
    return false;
  
  myDataReceived.setToNow();

  if (packet->bufToUByte() != 0)
    myButton1 = true;
  else
    myButton1 = false;

  if (packet->bufToUByte() != 0)
    myButton2 = true;
  else
    myButton2  = false;

  myRawX = packet->bufToUByte2();
  myRawY = packet->bufToUByte2();
  myRawThrottle = packet->bufToUByte2();

  // these should vary between 1 and -1
  if (myJoyXCenter > 511.9 && myJoyXCenter < 512.1)
  {
    myJoyX = -((double)myRawX - 512.0) / 512.0;
  }
  else
  {
    if (myRawX > myJoyXCenter) 
    {
      myJoyX = -(myRawX - myJoyXCenter) / (double)(1024 - myJoyXCenter);
    }
    else if (myRawX < myJoyXCenter)
    {
      myJoyX = -(myRawX - myJoyXCenter) / (double)(myJoyXCenter);
    }
  }

  if (myJoyYCenter > 511.9 && myJoyYCenter < 512.1)
  {
    myJoyY = ((double)myRawY - 512.0) / 512.0;
  }
  else
  {
    if (myRawY > myJoyYCenter) 
    {
      myJoyY = (myRawY - myJoyYCenter) / (double)(1024 - myJoyYCenter);
    }
    else if (myRawY < myJoyYCenter)
    {
      myJoyY = (myRawY - myJoyYCenter) / (double)(myJoyYCenter);
    }
  }

  // these should vary between 1 and 0
  myThrottle = (double)myRawThrottle / 1024.0;

  
  if (!myGotData)
  {
    MvrLog::log(MvrLog::Verbose, "Received joystick information from the robot");
    myGotData = true;
  }
  return true;
}

MVREXPORT void MvrRobotJoyHandler::getDoubles(double *x, double *y, double *z)
{
  if (x != NULL)
    *x = myJoyX;
  if (y != NULL)
    *y = myJoyY;
  if (z != NULL)
    *z = myThrottle;
}

MVREXPORT void MvrRobotJoyHandler::addToConfig(MvrConfig *config, 
					     const char *section)
{
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("JoyXCenter", &myJoyXCenter,
		      "The X center", 0.0, 1024.0),
	  section, MvrPriority::NORMAL);

  config->addParam(
	  MvrConfigArg("JoyYCenter", &myJoyYCenter,
		      "The Y center", 0.0, 1024.0),
	  section, MvrPriority::NORMAL);
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::NORMAL);  
}