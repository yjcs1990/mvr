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
#include "ariaOSDef.h"
#include "MvrTCMCompassDirect.h"
#include "MvrDeviceConnection.h"
#include "MvrSerialConnection.h"
#include "ariaUtil.h"

//#define DEBUG_ARTCMCOMPASSDIRECT 1
//#define DEBUG_ARTCMCOMPASSDIRECT_STATS 1

#if defined(DEBUG_ARTCMCOMPASSDIRECT) || defined(DEBUG_ARTCMCOMPASSDIRECT_STATS)
void ArTCMCompassDirect_printTransUnprintable( const char *data, int size){  for(int i = 0; i < size; ++i)  {    if(data[i] < ' ' || data[i] > '~')    {      printf("[0x%X]", data[i] & 0xff);    }    else    {      putchar(data[i]);    }  }}
#endif

MVREXPORT ArTCMCompassDirect::ArTCMCompassDirect(MvrDeviceConnection *devCon) :
  myDeviceConnection(devCon), myCreatedOwnDeviceConnection(false),
  myNMEAParser("MvrTCMCompassDirect"),
  myHCHDMHandler(this, &ArTCMCompassDirect::handleHCHDM)
{
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}

MVREXPORT ArTCMCompassDirect::ArTCMCompassDirect(const char *serialPortName) :
  myCreatedOwnDeviceConnection(true),
  myNMEAParser("MvrTCMCompassDirect"),
  myHCHDMHandler(this, &ArTCMCompassDirect::handleHCHDM)
{
  ArSerialConnection *newSerialCon = new ArSerialConnection();
  newSerialCon->setPort(serialPortName);
  newSerialCon->setBaud(9600);
  myDeviceConnection = newSerialCon;
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}
  

MVREXPORT ArTCMCompassDirect::~MvrTCMCompassDirect() {
  if(myCreatedOwnDeviceConnection && myDeviceConnection)
    delete myDeviceConnection;
}

bool ArTCMCompassDirect::sendTCMCommand(const char *fmt, ...)
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != ArDeviceConnection::STATUS_OPEN) return false;
  va_list args;
  va_start(args, fmt);
  char buf[32];
  vsnprintf(buf, sizeof(buf)-1, fmt, args);
  buf[sizeof(buf)-1] = 0;
  return myDeviceConnection->write(buf, strlen(buf));
}

MVREXPORT bool ArTCMCompassDirect::blockingConnect(unsigned long connectTimeout)
{
  ArTime start;
  start.setToNow();
  if(!connect()) return false;
  ArLog::log(MvrLog::Normal, "MvrTCMCompassDirect: Opened connection, waiting for initial data...");
  while((unsigned long)start.mSecSince() <= connectTimeout)
  {
    if(read(0) > 0)
      return true;
    ArUtil::sleep(100);
  }
  ArLog::log(MvrLog::Terse, "MvrTCMCompassDirect: Error: No response from compass after %dms.", connectTimeout);
  return false;
}


MVREXPORT bool ArTCMCompassDirect::connect()
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != ArDeviceConnection::STATUS_OPEN)
  {
    if(!myDeviceConnection->openSimple()) return false;
  }

  if(!sendTCMCommand("h\r"))
    return false;


  // sp= is sampling/send rate, maximum of 8hz. sdo is output format (t for cm
  // protocol or n for nmea).
  if(!sendTCMCommand("h\rsp=8\rsn=m\rsdo=n\rgo\r"))  
    return false;

#ifdef DEBUG_ARTCMCOMPASSDIRECT
  char buf[640];
  memset(buf, 0, 640);
  myDeviceConnection->read(buf, 640, 2000);
  printf("Compass responded to init commands with: %s\n", buf);
#endif

  return true;
}



MVREXPORT void ArTCMCompassDirect::commandUserCalibration()
{
  sendTCMCommand("cc\rmpcal=e\rgo\r");
}

MVREXPORT void ArTCMCompassDirect::commandStopCalibration()
{
  sendTCMCommand("h\rmpcal=d\rautocal=d\r");
}

MVREXPORT void ArTCMCompassDirect::commandContinuousPackets()
{
  sendTCMCommand("go\r");
}

MVREXPORT void ArTCMCompassDirect::commandOff()
{
  sendTCMCommand("h\r");
}

MVREXPORT void ArTCMCompassDirect::commandOnePacket()
{
  sendTCMCommand("c?\r");
}

MVREXPORT void ArTCMCompassDirect::commandAutoCalibration()
{
  sendTCMCommand("h\rcc\rautocal=e\r");
}

MVREXPORT int ArTCMCompassDirect::read(unsigned int msWait)
{
  return myNMEAParser.parse(myDeviceConnection);
}

void ArTCMCompassDirect::handleHCHDM(MvrNMEAParser::Message m)
{
  myHeading = ArMath::fixAngle(atof((*m.message)[1].c_str()));
#ifdef DEBUG_ARTCMCOMPASSDIRECT 
  printf("XXX ArTCMCompassDirect: recieved HCHDM message with compass heading %f.\n", myHeading);
#endif
  myHaveHeading = true;
  incrementPacketCount();
  invokeHeadingDataCallbacks(myHeading);
}

