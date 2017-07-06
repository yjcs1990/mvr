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
#include "MvrGPSConnector.h"
#include "MvrGPS.h"
#include "MvrDeviceConnection.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"

#include "MvrNovatelGPS.h"
#include "MvrTrimbleGPS.h"

#include <iostream>


// default values to use if no value is given in robot parameters or command
// line:
const int ARGPS_DEFAULT_SERIAL_BAUD = 9600;
const char* const ARGPS_DEFAULT_SERIAL_PORT = MvrUtil::COM2;
const int ARGPS_DEFAULT_REMOTE_TCP_PORT = 8103;

MVREXPORT MvrGPSConnector::MvrGPSConnector(MvrArgumentParser *argParser) :
  myDeviceCon(NULL),
  myArgParser(argParser),
  myParseArgsCallback(this, &MvrGPSConnector::parseArgs),
  myLogArgsCallback(this, &MvrGPSConnector::logOptions),
  myBaud(-1), 
  myPort(NULL),
  myTCPHost(NULL),
  myTCPPort(8103),
  myDeviceType(Invalid)
{
  myParseArgsCallback.setName("MvrGPSConnector");
  myLogArgsCallback.setName("MvrGPSConnector");
  Mvria::addParseArgsCB(&myParseArgsCallback);
  Mvria::addLogOptionsCB(&myLogArgsCallback);
}


MVREXPORT MvrGPSConnector::~MvrGPSConnector()
{
  if(myDeviceCon) delete myDeviceCon;
}

// Called by Mvria::parseArgs() to set parameter values from command line
// options, if present
MVREXPORT bool MvrGPSConnector::parseArgs() 
{
  if (!myArgParser) return false;
  if (!myArgParser->checkParameterArgumentString("-gpsPort", &myPort)) return false;
  if (!myArgParser->checkParameterArgumentInteger("-gpsBaud", &myBaud)) return false;
  if (!myArgParser->checkParameterArgumentString("-remoteGpsTcpHost", &myTCPHost)) return false;
  if (!myArgParser->checkParameterArgumentInteger("-remoteGpsTcpPort", &myTCPPort)) return false;
  char *deviceType = myArgParser->checkParameterArgument("-gpsType");
  if (deviceType) // if -gpsType was given
    myDeviceType = deviceTypeFromString(deviceType);
  return true;
}

MvrGPSConnector::GPSType MvrGPSConnector::deviceTypeFromString(const char *str)
{
  if (strcasecmp(str, "novatel") == 0)
  {
    return Novatel;
  }
  else if (strcasecmp(str, "trimble") == 0)
  {
    return Trimble;
  }
  else if (strcasecmp(str, "standard") == 0)
  {
    return Standard;
  }
  else if (strcasecmp(str, "novatelspan") == 0)
  {
    return NovatelSPAN;
  }
  else if (strcasecmp(str, "sim") == 0)
  {
    return Simulator;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "GPSConnector: Error: unrecognized GPS type.");
    return Invalid;
  }
}

void MvrGPSConnector::logOptions()
{
  MvrLog::log(MvrLog::Terse, "GPS options:"); 
  MvrLog::log(MvrLog::Terse, "-gpsType <standard|novatel|novatelspan|trimble|sim>\tSelect GPS device type (default: standard)");
  MvrLog::log(MvrLog::Terse, "-gpsPort <gpsSerialPort>\tUse the given serial port (default: %s)", ARGPS_DEFAULT_SERIAL_PORT);
  MvrLog::log(MvrLog::Terse, "-gpsBaud <gpsSerialBaudRate>\tUse the given serial Baud rate (default: %d)", ARGPS_DEFAULT_SERIAL_BAUD);
  MvrLog::log(MvrLog::Terse, "-remoteGpsTcpHost <host>\tUse a TCP connection instead of serial, and connect to remote host <host>");
  MvrLog::log(MvrLog::Terse, "-remoteGpsTcpPort <host>\tUse the given port number for TCP connection, if using TCP. (default %d)", ARGPS_DEFAULT_REMOTE_TCP_PORT);
}



// Create an MvrGPS object. If some options were obtained from command-line
// parameters by parseArgs(), use those, otherwise get values from robot
// parameters (the .p file) if we have a valid robot with valid parameters.
MVREXPORT MvrGPS* MvrGPSConnector::createGPS(MvrRobot *robot)
{
  // If we have a robot with parameters (i.e. have connected and read the .p
  // file), use those values unless already set by parseArgs() from command-line 
  if(robot && robot->getRobotParams())
  {
    if(myPort == NULL) {
      myPort = robot->getRobotParams()->getGPSPort();
      if(strcmp(myPort, "COM1") == 0)
        myPort = MvrUtil::COM1;
      if(strcmp(myPort, "COM2") == 0)
        myPort = MvrUtil::COM2;
      if(strcmp(myPort, "COM3") == 0)
        myPort = MvrUtil::COM3;
      if(strcmp(myPort, "COM4") == 0)
        myPort = MvrUtil::COM4;
    }
    if(myBaud == -1) {
      myBaud = robot->getRobotParams()->getGPSBaud();
    }
    if(myDeviceType == Invalid) {
      myDeviceType = deviceTypeFromString(robot->getRobotParams()->getGPSType());
    }
  }
  else
  {
    if(myPort == NULL) myPort = ARGPS_DEFAULT_SERIAL_PORT;
    if(myBaud == -1) myBaud = ARGPS_DEFAULT_SERIAL_BAUD;
    if(myDeviceType == Invalid) myDeviceType = Standard;
  }

  // If simulator, create simulated GPS and return
  if(robot && strcmp(robot->getRobotName(), "MobileSim") == 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using simulated GPS");
    myDeviceType = Simulator;
    return new MvrSimulatedGPS(robot);
  }

  // Create gps and connect to serial port or tcp port for device data stream:
  MvrGPS* newGPS = NULL;
  switch (myDeviceType)
  {
    case Novatel:
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using Novatel GPS");
      newGPS = new MvrNovatelGPS;
      break;
    case Trimble:
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using Trimble GPS");
      newGPS = new MvrTrimbleGPS;
      break;
    case NovatelSPAN:
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using Novatel SPAN GPS");
      newGPS = new MvrNovatelSPAN;
      break;
    case Simulator:
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using simulated GPS");
      newGPS = new MvrSimulatedGPS(robot);
      break;
    default:
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Using standard NMEA GPS");
      newGPS = new MvrGPS;
      break;
  }

  if(myDeviceType != Simulator)
  {
    if (myTCPHost == NULL)
    {
      // Setup serial connection
      MvrSerialConnection *serialCon = new MvrSerialConnection;
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Connecting to GPS on port %s at %d baud...", myPort, myBaud);
      if (!serialCon->setBaud(myBaud)) { delete serialCon; return NULL; }
      if (serialCon->open(myPort) != 0) {
        MvrLog::log(MvrLog::Terse, "MvrGPSConnector: Error: could not open GPS serial port %s.", myPort);
        delete serialCon;
        return NULL;
      }
      newGPS->setDeviceConnection(serialCon);
      myDeviceCon = serialCon;
    }
    else
    {
      // Setup TCP connection
      MvrTcpConnection *tcpCon = new MvrTcpConnection;
      MvrLog::log(MvrLog::Normal, "MvrGPSConnector: Opening TCP connection to %s:%d...", myTCPHost, myTCPPort);
      int openState = tcpCon->open(myTCPHost, myTCPPort);
      if (openState != 0) {
        MvrLog::log(MvrLog::Terse, "MvrGPSConnector: Error: could not open TCP connection to %s port %d: %s", tcpCon->getOpenMessage(openState));
        delete tcpCon;
        return NULL;
      }
      newGPS->setDeviceConnection(tcpCon);
      myDeviceCon = tcpCon;
    }
  }

  return newGPS;
}


