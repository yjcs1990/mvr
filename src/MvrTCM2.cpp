/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaInternal.h"
#include "mvriaUtil.h"
#include "MvrTCM2.h"
#include "MvrTCMCompassDirect.h"
#include "MvrTCMCompassRobot.h"

MVREXPORT MvrTCM2::MvrTCM2() :
  myHeading(0.0),
  myPitch(0.0),
  myRoll(0.0),
  myXMag(0.0),
  myYMag(0.0),
  myZMag(0.0),
  myTemperature(0.0),
  myError(0),
  myCalibrationH(0.0),
  myCalibrationV(0.0),
  myCalibrationM(0.0),
  myHaveHeading(false), 
  myHavePitch(false),
  myHaveRoll(false),
  myHaveXMag(false),
  myHaveYMag(false),
  myHaveZMag(false),
  myHaveTemperature(false),
  myHaveCalibrationH(false),
  myHaveCalibrationV(false),
  myHaveCalibrationM(false),
  myTimeLastPacket(0),
  myPacCurrentCount(0),
  myPacCount(0)
{
}

MVREXPORT bool MvrTCM2::connect() { return true; }
MVREXPORT bool MvrTCM2::blockingConnect(unsigned long) { return true; }

MVREXPORT MvrCompassConnector::MvrCompassConnector(MvrArgumentParser *argParser) :
  myArgParser(argParser),
  myParseArgsCallback(this, &MvrCompassConnector::parseArgs),
  myLogArgsCallback(this, &MvrCompassConnector::logOptions),
  myDeviceType(None),
  mySerialPort(ARTCM2_DEFAULT_SERIAL_PORT),
  mySerialTCMReadFunctor(NULL),
  myRobot(NULL)
{
  myParseArgsCallback.setName("MvrCompassConnector");
  myLogArgsCallback.setName("MvrCompassConnector");
  Mvria::addParseArgsCB(&myParseArgsCallback);
  Mvria::addLogOptionsCB(&myLogArgsCallback);
}

MVREXPORT MvrCompassConnector::~MvrCompassConnector() {
  if(mySerialTCMReadFunctor && myRobot) 
  {
    myRobot->lock();
    myRobot->remSensorInterpTask(mySerialTCMReadFunctor);
    delete mySerialTCMReadFunctor;
    myRobot->unlock();
  }
}

bool MvrCompassConnector::parseArgs()
{
  if(!myArgParser) return false;
  if(!myArgParser->checkParameterArgumentString("-compassPort", &mySerialPort)) return false;
  char *deviceType = myArgParser->checkParameterArgument("-compassType");
  if(deviceType)
  {
    if(strcasecmp(deviceType, "robot") == 0)
      myDeviceType = Robot;
    else if(strcasecmp(deviceType, "serialtcm") == 0)
      myDeviceType = SerialTCM;
    else if(strcasecmp(deviceType, "tcm") == 0)
      myDeviceType = SerialTCM;
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: unrecognized -compassType option: %s. Valid values are robot and serialTCM.", deviceType);
      return false;
    }
  }
  return true;
}

void MvrCompassConnector::logOptions()
{
  MvrLog::log(MvrLog::Terse, "Compass options:");
  MvrLog::log(MvrLog::Terse, "-compassType <robot|serialTCM>\tSelect compass device type (default: robot)");
  MvrLog::log(MvrLog::Terse, "-compassPort <port>\tSerial port for \"serialTCM\" type compass. (default: %s)", ARTCM2_DEFAULT_SERIAL_PORT);
}

MVREXPORT MvrTCM2 *MvrCompassConnector::create(MvrRobot *robot)
{
  if(myDeviceType == None)
  {
    if(robot && robot->getRobotParams())
    {
      const char *type = robot->getRobotParams()->getCompassType();
      if(type == NULL || strlen(type) == 0 || strcmp(type, "robot") == 0)
      {
        myDeviceType = Robot;
      }
      else if(strcmp(type, "serialTCM") == 0)
      {
        myDeviceType = SerialTCM;
        const char *port = robot->getRobotParams()->getCompassPort();
        if(port == NULL || strlen(port) == 0)
          mySerialPort = ARTCM2_DEFAULT_SERIAL_PORT;
        else
          mySerialPort = port;
      }
      else
      {
        MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: invalid compass type \"%s\" in robot parameters.", type);
        return NULL;
      }
    }
    else
    {
      myDeviceType = Robot;
    }
  }

  switch(myDeviceType)
  {
    case Robot:
      MvrLog::log(MvrLog::Verbose, "MvrCompassConnector: Using robot compass");
      return new MvrTCMCompassRobot(robot);
    case SerialTCM:
      {
        MvrLog::log(MvrLog::Verbose, "MvrCompassConnector: Using TCM2 compass on serial port %s", mySerialPort);
        MvrTCMCompassDirect *newDirectTCM = new MvrTCMCompassDirect(mySerialPort);
        mySerialTCMReadFunctor = new MvrRetFunctor1C<int, MvrTCMCompassDirect, unsigned int>(newDirectTCM, &MvrTCMCompassDirect::read, 1);
        robot->lock();
        robot->addSensorInterpTask("MvrTCMCompassDirect read", 200, mySerialTCMReadFunctor);
        myRobot = robot;
        robot->unlock();
        return newDirectTCM;
      }
    case None:
    default:
      // break out of switch and print warning there
      break;
  }
  MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: No compass type selected.");
  return NULL;
}

MVREXPORT bool MvrCompassConnector::connect(MvrTCM2 *compass) const
{
  return compass->blockingConnect();
}

