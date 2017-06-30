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
#include "MvrLMS2xx.h"
#include "MvrRobot.h"
#include "MvrSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>

MVREXPORT MvrLMS2xx::ArLMS2xx(
	int laserNumber, const char *name, bool appendLaserNumberToName) :
  MvrLaser(laserNumber, name, 32000, false, appendLaserNumberToName),
  mySimPacketHandler(this, &ArLMS2xx::simPacketHandler),
  mySensorInterpCB(this, &ArLMS2xx::sensorInterpCallback),
  myLMS2xxPacketReceiver(0, true),
  myAriaExitCB(this, &ArLMS2xx::disconnect)
{
  std::string str;
  laserSetName(getName());
  laserSetDefaultTcpPort(8102);
  laserSetDefaultPortType("serial");

  Mvria::addExitCallback(&myAriaExitCB, -10);

  std::map<std::string, double> degreesChoices;
  degreesChoices["180"] = 180;
  degreesChoices["100"] = 100;
  laserAllowDegreesChoices("180", degreesChoices);

  std::map<std::string, double> incrementChoices;
  incrementChoices["one"] = 1;
  incrementChoices["half"] = .5;
  laserAllowIncrementChoices("one", incrementChoices);

  std::list<std::string> unitsChoices;
  unitsChoices.push_back("1mm");
  unitsChoices.push_back("1cm");
  unitsChoices.push_back("10cm");
  laserAllowUnitsChoices("1mm", unitsChoices);

  laserAllowSetPowerControlled(false);

  std::list<std::string> reflectorBitsChoices;
  reflectorBitsChoices.push_back("1ref");
  reflectorBitsChoices.push_back("2ref");
  reflectorBitsChoices.push_back("3ref");
  laserAllowReflectorBitsChoices("1ref", reflectorBitsChoices);

  std::list<std::string> baudChoices;
  baudChoices.push_back("9600");
  baudChoices.push_back("19200");
  baudChoices.push_back("38400");
  laserAllowStartingBaudChoices("9600", baudChoices);

  laserAllowAutoBaudChoices("38400", baudChoices);


  myAssembleReadings = new std::list<ArSensorReading *>;
  myCurrentReadings = new std::list<ArSensorReading *>;
  myRawReadings = myCurrentReadings;
  myIter = myAssembleReadings->begin();
  myConn = NULL;
  myRobot = NULL;
  myStartConnect = false;
  myRunningOnRobot = false;
  switchState(STATE_NONE);
  myProcessImmediately = false;
  myInterpolation = true;
  myUseSim = false;

  setMinDistBetweenCurrent(0);
  setMaxDistToKeepCumulative(6000);
  setMinDistBetweenCumulative(200);
  setMaxSecondsToKeepCumulative(30);
  setMaxInsertDistCumulative(3000);

  setCumulativeCleanDist(75);
  setCumulativeCleanInterval(1000);

  resetLastCumulativeCleanTime();

  setCurrentDrawingData(
	  new MvrDrawingData("polyDots", 
			    MvrColor(0, 0, 255), 
			    80,  // mm diameter of dots
			    75), // layer above sonar 
	  true);

  setCumulativeDrawingData(
	  new MvrDrawingData("polyDots", 
			    MvrColor(125, 125, 125), 
			    100, // mm diameter of dots
			    60), // layer below current range devices  
	  true);
}

MVREXPORT MvrLMS2xx::~MvrLMS2xx()
{
  Mvria::remExitCallback(&myAriaExitCB);
  if (myRobot != NULL)
  {
    myRobot->remRangeDevice(this);
    myRobot->remLaser(this);
    myRobot->remPacketHandler(&mySimPacketHandler);
    myRobot->remSensorInterpTask(&mySensorInterpCB);
  }
  lockDevice();
  if (isConnected())
  {
    disconnect();
  }
  unlockDevice();
}


MVREXPORT void MvrLMS2xx::laserSetName(const char *name)
{
  myName = name;

  myStateMutex.setLogNameVar("%s::myStateMutex", getName());
  myAriaExitCB.setNameVar("%s::exitCallback", getName());
  mySimPacketHandler.setNameVar("%s::simPacketHandler", getName());
  
  MvrLaser::laserSetName(getName());
}

MVREXPORT bool MvrLMS2xx::sickGetIsUsingSim(void)
{
  return myUseSim;
}

MVREXPORT void MvrLMS2xx::sickSetIsUsingSim(bool usingSim)
{
  myUseSim = usingSim;
}

MVREXPORT void MvrLMS2xx::setDeviceConnection(MvrDeviceConnection *conn)
{
  myConnMutex.lock();
  myLMS2xxPacketReceiver.setDeviceConnection(conn); 
  myConnMutex.unlock();
  MvrLaser::setDeviceConnection(conn);
}



MVREXPORT void MvrLMS2xx::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
  {
    myRobot->addPacketHandler(&mySimPacketHandler, MvrListPos::LAST);
    myRobot->addSensorInterpTask("sick", 90, &mySensorInterpCB);
  }
  MvrRangeDevice::setRobot(robot);
}


/** @internal */
MVREXPORT bool MvrLMS2xx::simPacketHandler(MvrRobotPacket *packet)
{
  std::list<ArFunctor *>::iterator it;

  unsigned int totalNumReadings;
  unsigned int readingNumber;
  double atDeg;
  unsigned int i;
  MvrSensorReading *reading;
  std::list<ArSensorReading *>::iterator tempIt;
  unsigned int newReadings;
  int range;
  int refl = 0;
  MvrPose encoderPose;
  //std::list<double>::iterator ignoreIt;  
  bool ignore;
  
  if (packet->getID() != 0x60 && packet->getID() != 0x61)
    return false;

  bool isExtendedPacket = (packet->getID() == 0x61);
   
  // if we got here, its the right type of packet

  //printf("Got in a packet from the simulator\n");
  lockDevice();
  //printf("1\n");
  if (!myUseSim)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Got a packet from the simulator with laser information, but the laser is not being simulated, major trouble.", 
	       getName());
    unlockDevice();
    return true;
  }
  if(!isExtendedPacket)
  {
    // ignore the positional information
    packet->bufToByte2();
    packet->bufToByte2();
    packet->bufToByte2();
  }
  totalNumReadings = packet->bufToByte2(); // total for this reading
  readingNumber = packet->bufToByte2(); // which one we're on in this packet
  newReadings = packet->bufToUByte(); // how many are in this packet
  if (readingNumber == 0)
  {
    mySimPacketStart = myRobot->getPose();
    mySimPacketTrans = myRobot->getToGlobalTransform();
    mySimPacketEncoderTrans = myRobot->getEncoderTransform();
    mySimPacketCounter = myRobot->getCounter();
  }
  //printf("MvrLMS2xx::simPacketHandler: On reading number %d out of %d, new %d\n", readingNumber, totalNumReadings, newReadings);
  // if we have too many readings in our list of raw readings, pop the extras
  while (myAssembleReadings->size() > totalNumReadings)
  {
    MvrLog::log(MvrLog::Verbose, "MvrLMS2xx::simPacketHandler, too many readings, popping one.");
    tempIt = myAssembleReadings->begin();
    if (tempIt != myAssembleReadings->end())
      delete (*tempIt);
    myAssembleReadings->pop_front();
  }
  
  // If we don't have any sensor readings created at all, make 'em all now
  if (myAssembleReadings->size() == 0)
    for (i = 0; i < totalNumReadings; i++)
      myAssembleReadings->push_back(new MvrSensorReading);
  
  // Okay, we know where we're at, so get an iterator to the right spot, or 
  // make sure the one we keep around is in the right spot... if neither of
  // these trigger, then the iter should be in the right spot
  if ((readingNumber != myWhichReading + 1) || 
      totalNumReadings != myTotalNumReadings)
  {
    //printf("2\n");
    myWhichReading = readingNumber;
    myTotalNumReadings = totalNumReadings;
    for (i = 0, myIter = myAssembleReadings->begin(); i < readingNumber; i++)
    {
      tempIt = myIter;
      tempIt++;
      if (tempIt == myAssembleReadings->end() && (i + 1 != myTotalNumReadings))
	myAssembleReadings->push_back(new MvrSensorReading);
      myIter++;
    }
  }
  else
  {
    //printf("3\n");
    myWhichReading = readingNumber;
  }

  atDeg = (mySensorPose.getTh() - myOffsetAmount + 
	   readingNumber * myIncrementAmount);
  //printf("4\n");
  encoderPose = mySimPacketEncoderTrans.doInvTransform(mySimPacketStart);
  // while we have in the readings and have stuff left we can read 
  for (i = 0; 
       //	 (myWhichReading < myTotalNumReadings && 
       //	  packet->getReadLength() < packet->getLength() - 4);
       i < newReadings;
       i++, myWhichReading++, atDeg += myIncrementAmount)
  {
    reading = (*myIter);
    range = packet->bufToUByte2();
    if(isExtendedPacket)
    {
      refl = packet->bufToUByte();
      packet->bufToUByte(); // don't need this byte for anything yet
      packet->bufToUByte(); // don't need this byte for anything yet
    }
    ignore = false;

    /*
    for (ignoreIt = myIgnoreReadings.begin(); 
	 ignoreIt != myIgnoreReadings.end();
	 ignoreIt++)
    {
      //if (atDeg == 0)
      //printf("Ignoring %.0f\n", (*ignoreIt));
      if (MvrMath::fabs(MvrMath::subAngle(atDeg, *(ignoreIt))) < 1.0)
      {
	//printf("Ignoring %.0f\n", (*ignoreIt));
	ignore = true;
	break;
      }
    }
    //if (myMinRange != 0 && range < (int)myMinRange)
    //ignore = true;
    if (myMaxRange != 0 && range > (int)myMaxRange)
      ignore = true;
    */
    reading->resetSensorPosition(MvrMath::roundInt(mySensorPose.getX()),
				 MvrMath::roundInt(mySensorPose.getY()),
				 atDeg);
    //      printf("dist %d\n", dist);
    reading->newData(range, mySimPacketStart, 
		     encoderPose,
		     mySimPacketTrans,
		     mySimPacketCounter, packet->getTimeReceived(), ignore, refl);
    //printf("%d ", range);

    //addReading(reading->getX(), reading->getY());
    tempIt = myIter;
    tempIt++;
    if (tempIt == myAssembleReadings->end() && 
	myWhichReading + 1 != myTotalNumReadings)
    {
      myAssembleReadings->push_back(new MvrSensorReading);
    }
    myIter++;
  }
  
  // check if the sensor set is complete
  //printf("%d %d %d\n", newReadings, readingNumber, totalNumReadings);
  if (newReadings + readingNumber >= totalNumReadings)
  {
    // set MvrRangeDevice buffer
    myRawReadings = myAssembleReadings;
    // switch internal buffers
    myAssembleReadings = myCurrentReadings;
    myCurrentReadings = myRawReadings;
    // We have in all the readings, now sort 'em and update the current ones
    //filterReadings();
    laserProcessReadings();
    //printf("\n");
  }	
  
  unlockDevice();
  return true;
}


/** @internal */
MVREXPORT void MvrLMS2xx::switchState(State state)
{
  myStateMutex.lock();
  myState = state;
  myStateStart.setToNow();
  myStateMutex.unlock();
}

/**
   @internal
   @return 0 if its still trying to connect, 1 if it connected, 2 if it failed
**/
MVREXPORT int MvrLMS2xx::internalConnectHandler(void)
{
  MvrLMS2xxPacket *packet;
  MvrSerialConnection *conn;
  int value, autoBaud;

  switch (myState)
  {
  case STATE_INIT:
    if (myConn == NULL)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, have no deviceConnection.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
    {
      if ((conn = dynamic_cast<ArSerialConnection *>(myConn)) != NULL)
      {
	conn->setBaud(atoi(getStartingBaudChoice()));
      }
      if (!myConn->openSimple())
      {
	ArLog::log(MvrLog::Terse,
		   "%s: Failed to connect to laser, could not open port.",
		   getName());
	switchState(STATE_NONE);
	failedConnect();
	return 2;
      }
    }
    if (!getPowerControlled())
    {
      /*
      myPacket.empty();
      myPacket.uByteToBuf(0x20);
      myPacket.uByteToBuf(0x25);
      myPacket.finalizePacket();
      myConn->write(myPacket.getBuf(), myPacket.getLength());
      */
      switchState(STATE_CHANGE_BAUD);
      return internalConnectHandler();
    }
    MvrLog::log(MvrLog::Terse, "%s: waiting for laser to power on.",
	       getName());
    myPacket.empty();
    myPacket.uByteToBuf(0x10);
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      switchState(STATE_WAIT_FOR_POWER_ON);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, could not send init.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_WAIT_FOR_POWER_ON:
    while ((packet = myLMS2xxPacketReceiver.receivePacket()) != NULL)
    {
      if (packet->getID() == 0x90)
      {
	switchState(STATE_CHANGE_BAUD);
	return 0;
      }
    }
    if (myStateStart.secSince() > 65)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, no poweron received.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_CHANGE_BAUD:
    // don't autobaud if this isn't a serial device, just move on to the next stage
    if ((conn = dynamic_cast<ArSerialConnection *>(myConn)) == NULL)
    {
      switchState(STATE_CONFIGURE);
      return 0;
    }

    myPacket.empty();
    myPacket.byteToBuf(0x20);
    autoBaud = atoi(getAutoBaudChoice());
    if (autoBaud == 9600)
      myPacket.byteToBuf(0x42);
    else if (autoBaud == 19200)
      myPacket.byteToBuf(0x41);
    else if (autoBaud == 38400)
      myPacket.byteToBuf(0x40);
    else
    {
      MvrLog::log(MvrLog::Terse, "%s: Do not know how to autobaud to %d", 
		 getName(), autoBaud);
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      MvrUtil::sleep(20);
      conn->setBaud(autoBaud);
      switchState(STATE_CONFIGURE);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
	 "%s: Failed to connect to laser, could not send baud command.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_CONFIGURE:
    // wait a while for the baud to change
    if (myStateStart.mSecSince() < 300)
      return 0;
    myPacket.empty();
    myPacket.byteToBuf(0x3b);
    myPacket.uByte2ToBuf(abs(MvrMath::roundInt(myOffsetAmount * 2)));
    myPacket.uByte2ToBuf(abs(MvrMath::roundInt(myIncrementAmount * 100)));
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      switchState(STATE_WAIT_FOR_CONFIGURE_ACK);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, could not send configure command.", 
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_WAIT_FOR_CONFIGURE_ACK:
    while ((packet = myLMS2xxPacketReceiver.receivePacket()) != NULL)
    {
      if (packet->getID() == 0xbb)
      {
	value = packet->bufToByte();
	if (value == 0)
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not configure laser, failed connect.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
	else if (value == 1)
	{
	  // here
	  //switchState(STATE_START_READINGS);
	  switchState(STATE_INSTALL_MODE);
	  return 0;
	}
	else
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not configure laser, failed connect.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
      }
      else if (packet->getID() == 0xb0)
      {
	ArLog::log(MvrLog::Terse, 
		   "%s: extra data packet while waiting for configure ack", 
		   getName());
	myPacket.empty();
	myPacket.uByteToBuf(0x20);
	myPacket.uByteToBuf(0x25);
	myPacket.finalizePacket();
	if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
	{
	  switchState(STATE_CONFIGURE);
	  return 0;
	}
      }
      else
	ArLog::log(MvrLog::Terse, "%s: Got a 0x%x", getName(), packet->getID(),
		   getName());
    }
    if (myStateStart.mSecSince() > 10000)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, no configure acknowledgement received.", 
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_INSTALL_MODE:
    if (myStateStart.mSecSince() < 200)
      return 0;
    myPacket.empty();
    myPacket.byteToBuf(0x20);
    myPacket.byteToBuf(0x00);
    myPacket.strNToBuf("SICK_LMS", (int)strlen("SICK_LMS"));
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      switchState(STATE_WAIT_FOR_INSTALL_MODE_ACK);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
	 "%s: Failed to connect to laser, could not send start command.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_WAIT_FOR_INSTALL_MODE_ACK:
    while ((packet = myLMS2xxPacketReceiver.receivePacket()) != NULL)
    {
      if (packet->getID() == 0xa0)
      {
	value = packet->bufToByte();
	if (value == 0)
	{
	  //printf("Um, should set mode?\n");
	  switchState(STATE_SET_MODE);
	  return 0;
	}
	else if (value == 1)
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not start laser, incorrect password.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
	else if (value == 2)
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not start laser, LMI fault.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
	else
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not start laser, unknown problem.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
      }
      else if (packet->getID() == 0xb0)
      {
	
	ArLog::log(MvrLog::Terse, "%s: extra data packet",
		   getName());
	myPacket.empty();
	myPacket.uByteToBuf(0x20);
	myPacket.uByteToBuf(0x25);
	myPacket.finalizePacket();
	if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
	{
	  switchState(STATE_INSTALL_MODE);
	  return 0;
	}
      }
      else
	ArLog::log(MvrLog::Terse, "%s: bad packet 0x%x", packet->getID(),
		   getName());
    }
    if (myStateStart.mSecSince() > 10000)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, no install mode ack received.", getName());
      switchState(STATE_NONE);
      return 2;
    }
    break;
  case STATE_SET_MODE:
    if (myStateStart.mSecSince() < 200)
      return 0;
    myPacket.empty();
    // type of packet
    myPacket.byteToBuf(0x77);
    // blanking
    myPacket.uByte2ToBuf(0);
    // peak threshhold, stop threshold
    myPacket.uByte2ToBuf(70);
    // the old peak threshhold thats probably broken
    // myPacket.uByte2ToBuf(0);
    // fog correction
    myPacket.uByteToBuf(0);
    // measurement Mode (fun one) (we can switch this now)
    // next line was the previous permanent one
    // myPacket.uByteToBuf(6);
    // max range stuff was pulled cause its in checkParams now
    //int maxRange;
    //maxRange = 8;
    if (myNumReflectorBits == 1)
    {
      myPacket.uByteToBuf(5);
      //maxRange *= 4;
    }
    else if (myNumReflectorBits == 2)
    {
      myPacket.uByteToBuf(3);
      //maxRange *= 2;
    }
    else if (myNumReflectorBits == 3)
    {
      myPacket.uByteToBuf(1);
      //maxRange *= 1;
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "%s: Bits set to unknown value",
		 getName());
      myPacket.uByteToBuf(5);
      //maxRange *= 4;
    }
    // unit value (fun one), we can swithc this now
    // next line was the previous permanent one
    //myPacket.uByteToBuf(1);
    if (strcmp(getUnitsChoice(), "1mm") == 0)
    {
      //maxRange *= 1000;
      myPacket.uByteToBuf(1);
    }
    else if (strcmp(getUnitsChoice(), "10cm") == 0)
    {
      //maxRange = 150000;
      myPacket.uByteToBuf(2);
    }
    else if (strcmp(getUnitsChoice(), "1cm") == 0)
    {
      //maxRange *= 10000;
      myPacket.uByteToBuf(0);
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "%s: Units set to unknown value", getName());
      //maxRange *= 1000;
      myPacket.uByteToBuf(1);
    }
    //setMaxRange(maxRange);
    // temporary field set
    myPacket.uByteToBuf(0);
    // fields A & B as subtractive
    myPacket.uByteToBuf(0);
    // multiple evaluation
    myPacket.uByteToBuf(2);
    // restart
    myPacket.uByteToBuf(2);
    // restart time
    myPacket.uByteToBuf(0);
    // contour A as reference object size
    myPacket.uByteToBuf(0);
    // contour A positive range of tolerance
    myPacket.uByteToBuf(0);
    // contour A negative range of tolerance
    myPacket.uByteToBuf(0);
    // contour A starting angle
    myPacket.uByteToBuf(0);
    // contour A stopping angle
    myPacket.uByteToBuf(0);
    // contour B as reference object size
    myPacket.uByteToBuf(0);
    // contour B positive range of tolerance
    myPacket.uByteToBuf(0);
    // contour B negative range of tolerance
    myPacket.uByteToBuf(0);
    // contour B starting angle
    myPacket.uByteToBuf(0);
    // contour B stopping angle
    myPacket.uByteToBuf(0);
    // contour C as reference object size
    myPacket.uByteToBuf(0);
    // contour C positive range of tolerance
    myPacket.uByteToBuf(0);
    // contour C negative range of tolerance
    myPacket.uByteToBuf(0);
    // contour C starting angle
    myPacket.uByteToBuf(0);
    // contour C stopping angle
    myPacket.uByteToBuf(0);
    // pixel oriented evaluation
    myPacket.uByteToBuf(0);
    // mode for single meas value eval
    myPacket.uByteToBuf(0);
    // restart times for field b and field c
    myPacket.byte2ToBuf(0);
    // um, an extra one (sick quickstart manual says its 21 not 20 long)
    myPacket.uByteToBuf(0);
    
    myPacket.finalizePacket();
    //myPacket.log();
    //printf("Sending mode!\n");
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      //printf("Set mode!\n");
      switchState(STATE_WAIT_FOR_SET_MODE_ACK);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
		"%s: Failed to connect to laser, could not send set mode command.", 
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_WAIT_FOR_SET_MODE_ACK:
    while ((packet = myLMS2xxPacketReceiver.receivePacket()) != NULL)
    {
      if (packet->getID() == 0xF7)
      {
	//value = packet->bufToByte();
	//printf("YAY %d\n", value);
	//packet->log();
	switchState(STATE_START_READINGS);
	return 0;
      }
      else if (packet->getID() == 0xb0)
      {
	
	ArLog::log(MvrLog::Terse, "%s: extra data packet",
		   getName());
	myPacket.empty();
	myPacket.uByteToBuf(0x20);
	myPacket.uByteToBuf(0x25);
	myPacket.finalizePacket();
	if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
	{
	  switchState(STATE_INSTALL_MODE);
	  return 0;
	}
      }
      else if (packet->getID() == 0x92)
      {
	switchState(STATE_INSTALL_MODE);
	return 0;
      }
      else
	ArLog::log(MvrLog::Terse, "%s: Got a 0x%x", getName(), packet->getID());
    }
    if (myStateStart.mSecSince() > 14000)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, no set mode acknowledgement received.", 
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
    case STATE_START_READINGS: 
      if (myStateStart.mSecSince() < 200)
      return 0;
    myPacket.empty();
    myPacket.byteToBuf(0x20);
    myPacket.byteToBuf(0x24);
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      switchState(STATE_WAIT_FOR_START_ACK);
      return 0;
    }
    else
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, could not send start command.", 
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  case STATE_WAIT_FOR_START_ACK:
    while ((packet = myLMS2xxPacketReceiver.receivePacket()) != NULL)
    {
      if (packet->getID() == 0xa0)
      {
	value = packet->bufToByte();
	if (value == 0)
	{
	  MvrLog::log(MvrLog::Terse, "%s: Connected to the laser.",
		     getName());
	  switchState(STATE_CONNECTED);
	  madeConnection();
	  return 1;
	}
	else if (value == 1)
	{
	  MvrLog::log(MvrLog::Terse, 
	     "%s: Could not start laser laser, incorrect password.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
	else if (value == 2)
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not start laser laser, LMI fault.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
	else
	{
	  MvrLog::log(MvrLog::Terse, 
		     "%s: Could not start laser laser, unknown problem.",
		     getName());
	  switchState(STATE_NONE);
	  failedConnect();
	  return 2;
	}
      }
    }
    if (myStateStart.mSecSince() > 1000)
    {
      MvrLog::log(MvrLog::Terse,
		 "%s: Failed to connect to laser, no start acknowledgement received.",
		 getName());
      switchState(STATE_NONE);
      failedConnect();
      return 2;
    }
    break;
  default:
    MvrLog::log(MvrLog::Verbose, "%s: In bad connection state",
	       getName());
    break;
  }
  return 0;
}

/**
   Sends the commands to the sim to start up the connection

   @return true if the commands were sent, false otherwise
**/
MVREXPORT bool MvrLMS2xx::internalConnectSim(void)
{
  lockDevice();
  double offset = myOffsetAmount;
  double increment = myIncrementAmount;
  unlockDevice();

  myRobot->lock();
  // return true if we could send all the commands
  if (myRobot->comInt(36, -ArMath::roundInt(offset)) &&   // Start angle
      myRobot->comInt(37, MvrMath::roundInt(offset)) &&    // End angle
      myRobot->comInt(38, MvrMath::roundInt(increment * 100.0)) && // increment
      myRobot->comInt(35, 2)) // Enable sending data, with extended info 
    ///@todo only choose extended info if reflector bits desired, also shorten range.
  {
    myRobot->unlock();
    switchState(STATE_CONNECTED);
    madeConnection();
    MvrLog::log(MvrLog::Terse, "%s: Connected to simulated laser.",
	       getName());
    return true;
  }
  else
  {
    switchState(STATE_NONE);
    failedConnect();
    MvrLog::log(MvrLog::Terse, 
	       "%s: Failed to connect to simulated laser.",
	       getName());
    return false;
  }
}

/** @internal */
MVREXPORT void MvrLMS2xx::dropConnection(void)
{
  std::list<ArFunctor *>::iterator it;  

  if (myState != STATE_CONNECTED)
    return;

  myCurrentBuffer.reset();
  myCumulativeBuffer.reset();
  MvrLog::log(MvrLog::Terse, 
	     "%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(), 
	     myLastReading.mSecSince()/1000.0, 
	     getConnectionTimeoutSeconds());
  switchState(STATE_NONE);

  if (myConn != NULL)
    myConn->close();

  laserDisconnectOnError();
}

/** @internal */
MVREXPORT void MvrLMS2xx::failedConnect(void)
{
  std::list<ArFunctor *>::iterator it;  
  
  switchState(STATE_NONE);

  if (myConn != NULL)
    myConn->close();

  laserFailedConnect();
}

/** @internal */
MVREXPORT void MvrLMS2xx::madeConnection(void)
{
  myLastReading.setToNow();

  laserConnect();
}

/**
   Disconnects from the laser.  You should lockDevice the laser before
   calling this function.  Also if you are using the simulator it will
   lock the robot so it can send the command to the simulator, so you
   should make sure the robot is unlocked.  
   
   @return true if it could disconnect from the laser cleanly
**/
MVREXPORT bool MvrLMS2xx::disconnect(void)
{
  std::list<ArFunctor *>::iterator it;  
  bool ret;
  MvrSerialConnection *conn;

  myStateMutex.lock();
  if (myState == STATE_NONE)
  {
    myStateMutex.unlock();
    return true;
  }
    
  if (myState != STATE_CONNECTED)
  {
    lockDevice();
    myConnMutex.lock();
    myState = STATE_NONE;
    ret = myConn->close();
    myConnMutex.unlock();
    unlockDevice();
    MvrLog::log(MvrLog::Terse, "%s: Disconnecting from laser that was not fully connected to...  this may cause problems later.", getName());
    myStateMutex.unlock();
    return ret;
  }

  myCurrentBuffer.reset();
  myCumulativeBuffer.reset();
  MvrLog::log(MvrLog::Terse, "%s: Disconnecting from laser.", getName());
  myState = STATE_NONE;
  myStateMutex.unlock();


  if (myUseSim)
  {
    if (myRobot == NULL)
    {
      laserDisconnectNormally();
      return false;
    }
    // locks are recursive now, so don't have to worry about this
    //if (!doNotLockRobotForSim)
    myRobot->lock();
    ret = myRobot->comInt(35, 2); // 2=extendend info request
    ///@todo only choose extended info if reflector bits desired, also shorten range.
    //if (!doNotLockRobotForSim)
    myRobot->unlock();
    laserDisconnectNormally();
    return ret;
  }
  else
  {
    myConnMutex.lock();
    while (myLMS2xxPacketReceiver.receivePacket() != NULL);
    myPacket.empty();
    myPacket.uByteToBuf(0x20);
    myPacket.uByteToBuf(0x25);
    myPacket.finalizePacket();
    ret = myConn->write(myPacket.getBuf(), myPacket.getLength());
    // put the thing back to 9600 baud
    MvrUtil::sleep(1000);
    myPacket.empty();
    myPacket.byteToBuf(0x20);
    myPacket.byteToBuf(0x42);
    myPacket.finalizePacket();
    if (myConn->write(myPacket.getBuf(), myPacket.getLength()))
    {
      MvrUtil::sleep(20);
      if ((conn = dynamic_cast<ArSerialConnection *>(myConn)))
	  conn->setBaud(9600);
    } else
      ret = false;
    ret = ret && myConn->close();
    myConnMutex.unlock();
    MvrUtil::sleep(300);
    laserDisconnectNormally();
    return ret;
  }
}

bool MvrLMS2xx::finishParams(void)
{
  if (!getRunning())
    runAsync();

  if (!laserPullUnsetParamsFromRobot())
  {
    MvrLog::log(MvrLog::Normal, "%s: Couldn't pull params from robot",
	       getName());
    return false;
  }

  return laserCheckParams();
}

MVREXPORT bool MvrLMS2xx::laserCheckParams(void)
{
  myOffsetAmount = getDegreesChoiceDouble() / 2.0;
  if (getFlipped())
    myOffsetAmount *= -1;

  myIncrementAmount = getIncrementChoiceDouble();
  if (getFlipped())
    myIncrementAmount *= -1;    

  if (strcasecmp(getReflectorBitsChoice(), "1ref") == 0)
    myNumReflectorBits = 1;
  else if (strcasecmp(getReflectorBitsChoice(), "2ref") == 0)
    myNumReflectorBits = 2;
  else if (strcasecmp(getReflectorBitsChoice(), "3ref") == 0)
    myNumReflectorBits = 3;
  else
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s: Bad reflectorBits choice %s, choices are %s", 
	       getName(), getReflectorBitsChoice(), 
	       getReflectorBitsChoicesString());
    return false;
  }

  if (strcmp(getIncrementChoice(), "half") == 0)
    myInterlaced = true; 
  else if (strcmp(getIncrementChoice(), "one") == 0)
    myInterlaced = false;
  else
  {
    MvrLog::log(MvrLog::Normal, "%s: Bad increment choice %s, choices are %s", 
	       getName(), getIncrementChoice(), getIncrementChoicesString());
    return false;
  }


  int maxRange;
  maxRange = 8;
  if (myNumReflectorBits == 1)
    maxRange *= 4;
  else if (myNumReflectorBits == 2)
    maxRange *= 2;
  else if (myNumReflectorBits == 3)
    myPacket.uByteToBuf(1);
  else
  {
    MvrLog::log(MvrLog::Terse, "%s: Bits set to unknown value", getName());
    maxRange *= 4;
  }
  
  if (strcmp(getUnitsChoice(), "1mm") == 0)
    maxRange *= 1000;
  else if (strcmp(getUnitsChoice(), "10cm") == 0)
    maxRange = 150000;
  else if (strcmp(getUnitsChoice(), "1cm") == 0)
    maxRange *= 10000;
  else
  {
    MvrLog::log(MvrLog::Terse, "%s: Units set to unknown value", getName());
    maxRange *= 1000;
  }

  laserSetAbsoluteMaxRange(maxRange);
  return true;
}

/**
   Locks this class (using lockDevice()), tries to make a connection, then
   unlocks.  If connecting to the simulator, 
   then it will commands to the simulator instead of connecting
   over the configured serial port.  

   @note If you have previously locked
   the laser with lockDevice(), then you must unlock with unlockDevice() 
   before calling this function.

   @note Since the simulated laser uses the robot connection instead
   of a separate, new connection, this MvrLMS2xx object @b must have been
   added to the robot using MvrRobot::addRangeDevice(), and the robot
   connection @b must be connected and running (e.g. in a background thread
   via MvrRobot::runAsync()) for blockingConnect() to be able to successfully
   connect to the simulator.

   @return true if a connection was successfully made, false otherwise
**/
 
MVREXPORT bool MvrLMS2xx::blockingConnect(void)
{
  int ret;

  if (!finishParams())
    return false;

  // if we're using the sim
  if (myUseSim)
  {
    return internalConnectSim();
  }
  // if we're talking to a real laser
  else 
  {
    if (myConn == NULL)
    {
      MvrLog::log(MvrLog::Terse, 
		 "%s: Invalid device connection, cannot connect.",
		 getName());
      return false; // Nobody ever set the device connection.
    }
    lockDevice();
    myConnMutex.lock();
    switchState(STATE_INIT);
    unlockDevice();
    while (getRunningWithLock() && (ret = internalConnectHandler()) == 0)
      MvrUtil::sleep(100);
    myConnMutex.unlock();
    if (ret == 1)
      return true;
    else 
      return false;
  }
  return false;
}

/**
  This does not lock the laser, but you should lock the
  laser before you try to connect.  Also note that if you are
  connecting to the simulator the laser MUST be unlocked so that this can
  lock the laser and send the commands to the sim.  To be connected
  successfully, either the useSim must be set from configure (and the
  laser must be connected to a simulator, or this will return true but
  connection will fail), the device must have been run() or runAsync(), or
  runOnRobot() used.

  @return true if a connection will be able to be tried, false
  otherwise 

  @see configure(), MvrRangeDeviceThreaded::run(),
  MvrRangeDeviceThreaded::runAsync(), runOnRobot()
**/
MVREXPORT bool MvrLMS2xx::asyncConnect(void)
{
  if (myState == STATE_CONNECTED)
  {
    MvrLog::log(MvrLog::Terse, "%s: already connected to laser.",
	       getName());
    return false;
  }

  if (!finishParams())
    return false;

  if (!myUseSim && myConn == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Invalid device connection, cannot connect.",
	       getName());
    return false; // Nobody ever set the device connection.
  }
  
  myStartConnect = true;
  return true;
}

/**
   This alternate method of operation sets up a sensor interpretation task 
   on the robot object, instead of in a background thread.
   Note that the device must have been added to
   the robot already so that the device has a pointer to the robot.
   You should lock the robot and lockDevice() this laser before calling this if 
   other things are running already.
**/
MVREXPORT bool MvrLMS2xx::internalRunOnRobot(void)
{
  if (myRobot == NULL)
    return false;
  else
  {  
    myRunningOnRobot = true;
    if (getRunning())
      stopRunning();
    return true;
  }
}

/** @internal */
MVREXPORT void MvrLMS2xx::processPacket(MvrLMS2xxPacket *packet, MvrPose pose,
				    MvrPose encoderPose,
				    unsigned int counter,
				    bool deinterlace,
				    MvrPose deinterlaceDelta)
{
  std::list<ArFunctor *>::iterator it;  
  unsigned int rawValue;
  unsigned int value;
  unsigned int reflector = 0;
  unsigned int numReadings;
  unsigned int i;
  double atDeg;
  unsigned int onReading;
  MvrSensorReading *reading;
  int dist;
  std::list<ArSensorReading *>::iterator tempIt;
  int multiplier;
  MvrTransform transform;
  //std::list<double>::iterator ignoreIt;  
  bool ignore;

  MvrTime arTime;
  arTime = packet->getTimeReceived();
  if (!arTime.addMSec(-13)) {
    MvrLog::log(MvrLog::Normal,
               "MvrLMS2xx::processPacket() error adding msecs (-13)");
  }

  MvrTime deinterlaceTime;
  deinterlaceTime = packet->getTimeReceived();
  if (!deinterlaceTime.addMSec(-27)) {
    MvrLog::log(MvrLog::Normal,
               "MvrLMS2xx::processPacket() error adding msecs (-27)");
  } 
  //if (packet->getID() != 0xb0)
  //printf("Got in packet of type 0x%x\n", packet->getID());
  if (packet->getID() == 0xb0)
  {
    value = packet->bufToUByte2();
    numReadings = value & 0x3ff;
    //printf("numreadings %d\n", numReadings);
    if (!(value & MvrUtil::BIT14) && !(value & MvrUtil::BIT15))
      multiplier = 10;
    else if ((value & MvrUtil::BIT14) && !(value & MvrUtil::BIT15))
      multiplier = 1;
    else if (!(value & MvrUtil::BIT14) && (value & MvrUtil::BIT15))
      multiplier = 100;
    else
    {
      MvrLog::log(MvrLog::Terse, 
		 "%s::processPacket: bad distance configuration in packet",
		 getName());
      multiplier = 0;
    }
    //printf("%ld ms after last reading.\n", myLastReading.mSecSince());
    /*printf("Reading number %d, complete %d, unit: %d %d:\n", numReadings,
      !(bool)(value & MvrUtil::BIT13), (bool)(value & MvrUtil::BIT14),
      (bool)(value & MvrUtil::BIT15));*/
    while (myAssembleReadings->size() > numReadings)
    {
      MvrLog::log(MvrLog::Verbose, "%s::processPacket, too many readings, popping one.",
		 getName());
      tempIt = myAssembleReadings->begin();
      if (tempIt != myAssembleReadings->end())
	delete (*tempIt);
      myAssembleReadings->pop_front();
    }
    
    // If we don't have any sensor readings created at all, make 'em all 
    if (myAssembleReadings->size() == 0)
      for (i = 0; i < numReadings; i++)
	myAssembleReadings->push_back(new MvrSensorReading);

    transform.setTransform(pose);
    //deinterlaceDelta = transform.doInvTransform(deinterlacePose);
    // printf("usePose2 %d, th1 %.0f th2 %.0f\n",  usePose2, pose.getTh(), pose2.getTh());
    for (atDeg = mySensorPose.getTh() - myOffsetAmount, onReading = 0,
	 myIter = myAssembleReadings->begin();
	 (onReading < numReadings && 
	  packet->getReadLength() < packet->getLength() - 4);
	 myWhichReading++, atDeg += myIncrementAmount, myIter++, onReading++)
    {
      reading = (*myIter);
      //reading->resetSensorPosition(0, 0, 0);

      //value = packet->bufToUByte2() & 0x1fff;
      //dist = (value & 0x1fff) * multiplier ;

      rawValue = packet->bufToUByte2();
      if (myNumReflectorBits == 1)
      {
	dist = (rawValue & 0x7fff) * multiplier;
	reflector = ((rawValue & 0x8000) >> 15) << 2;
      }
      else if (myNumReflectorBits == 2)
      {
	dist = (rawValue & 0x3fff) * multiplier;
	reflector = ((rawValue & 0xc000) >> 14) << 1 ;
      }
      else if (myNumReflectorBits == 3)
      {
	dist = (rawValue & 0x1fff) * multiplier;
	reflector = ((rawValue & 0xe000) >> 13);
      }
      // just trap for if we don't know what it is, this shouldn't
      // happen though
      else
      {
	dist = (rawValue & 0x7fff) * multiplier;
	reflector = 0;
      }
      // there are 3 reflector bits (its already been normalized above
      // to that range) so now we need to shift it another 5 so we get
      // 0-255.
      reflector = reflector << 5;

      ignore = false;
      /*
      for (ignoreIt = myIgnoreReadings.begin(); 
	   ignoreIt != myIgnoreReadings.end();
	   ignoreIt++)
      {
	if (MvrMath::fabs(MvrMath::subAngle(atDeg, *(ignoreIt))) < 1.0)
	{
	  ignore = true;
	  break;
	}
      }
      //if (myMinRange != 0 && dist < (int)myMinRange)
      //ignore = true;
      if (myMaxRange != 0 && dist > (int)myMaxRange)
	ignore = true;
      */
      if (deinterlace && (onReading % 2) == 0)
      {
	reading->resetSensorPosition(
	       MvrMath::roundInt(mySensorPose.getX() + deinterlaceDelta.getX()),
	       MvrMath::roundInt(mySensorPose.getY() + deinterlaceDelta.getY()),
	       MvrMath::addAngle(atDeg, deinterlaceDelta.getTh()));
	reading->newData(dist, pose, encoderPose, transform, counter, 
			 deinterlaceTime, ignore, reflector);
      }
      else
      {
	reading->resetSensorPosition(MvrMath::roundInt(mySensorPose.getX()),
				     MvrMath::roundInt(mySensorPose.getY()),
				     atDeg); 
	reading->newData(dist, pose, encoderPose, transform, counter, 
			 arTime, ignore, reflector);
      }
      /*
      reading->newData(onReading, 0, 0, 0,
		       MvrTransform(), counter, 
		       packet->getTimeReceived());
      */
      tempIt = myIter;
      tempIt++;
      if (tempIt == myAssembleReadings->end() && 
	  onReading + 1 != numReadings)
      {
	myAssembleReadings->push_back(new MvrSensorReading);
      }
    }
    // set MvrRangeDevice buffer, switch internal buffers
    myRawReadings = myAssembleReadings;
    //printf("Readings? 0x%x\n", myRawReadings);
    myAssembleReadings = myCurrentReadings;
    myCurrentReadings = myRawReadings;
    //printf("\n");
    myLastReading.setToNow();
    //filterReadings();
    laserProcessReadings();
  }
}

/** @internal */
MVREXPORT void MvrLMS2xx::runOnce(bool lockRobot)
{
  MvrLMS2xxPacket *packet;
  unsigned int counter;
  int ret;
  MvrTime time;
  MvrTime time2;
  MvrPose pose;
  MvrPose pose2;
  MvrPose encoderPose;

  if (myProcessImmediately && myRobot != NULL)
  {
    if (lockRobot)
      myRobot->lock();
    pose = myRobot->getPose();
    counter = myRobot->getCounter();
    if (lockRobot)
      myRobot->unlock();
  }

  lockDevice();
  if (myState == STATE_CONNECTED && laserCheckLostConnection())
  {
    dropConnection();
    unlockDevice();
    return;
  }
  if (myUseSim)
  {
    unlockDevice();
    return;
  }
  if (myState == STATE_CONNECTED)
  {
    unlockDevice();
    myConnMutex.lock();
    packet = myLMS2xxPacketReceiver.receivePacket();
    myConnMutex.unlock();
    lockDevice();
    // if we're attached to a robot and have a packet
    if (myRobot != NULL && packet != NULL && !myProcessImmediately)
    {
      myPackets.push_back(packet);
    }
    else if (myRobot != NULL && packet != NULL && myProcessImmediately)
    {
      unlockDevice();
      if (lockRobot && myInterpolation)
	myRobot->lock();
      // try to get the interpolated position, if we can't, just use
      // the robot's pose
      if (myInterpolation && (ret = myRobot->getPoseInterpPosition(
	      packet->getTimeReceived(), &pose)) < 0)
	pose = myRobot->getPose();
      // try to get the interpolated encoder position, if we can't,
      // just fake it from the robot's pose and the encoder transform
      if (myInterpolation && (ret = myRobot->getEncoderPoseInterpPosition(
	      packet->getTimeReceived(), &encoderPose)) < 0)
	encoderPose = myRobot->getEncoderTransform().doInvTransform(pose);
      if (lockRobot && myInterpolation)
	myRobot->unlock();
      lockDevice();
      processPacket(packet, pose, encoderPose, counter, false, MvrPose());
    }
    else if (packet != NULL) // if there's no robot
    {
      processPacket(packet, pose, encoderPose, 0, false, MvrPose());
      delete packet;
    }
  }
  unlockDevice();
  return;
}

/** @internal */
MVREXPORT void MvrLMS2xx::sensorInterpCallback(void)
{
  std::list<ArLMS2xxPacket *>::iterator it;
  std::list<ArLMS2xxPacket *> processed;
  MvrLMS2xxPacket *packet;
  MvrTime time;
  MvrPose pose;
  int ret = -999;
  int retEncoder = -999;
  MvrPose encoderPose;
  MvrPose deinterlaceEncoderPose;
  bool deinterlace;
  MvrTime deinterlaceTime;
  MvrPose deinterlaceDelta;
  
  if (myRunningOnRobot)
    runOnce(false);

  lockDevice();

  if (myInterlaced)
    adjustRawReadings(true);
  else
    adjustRawReadings(false);

  for (it = myPackets.begin(); it != myPackets.end(); it++)
  {
    packet = (*it);
    time = packet->getTimeReceived();
    if (!time.addMSec(-13)) {
      MvrLog::log(MvrLog::Normal,
                 "MvrLMS2xx::sensorInterpCallback() error adding msecs (-13)");
    }
    if ((ret = myRobot->getPoseInterpPosition(time, &pose)) >= 0 &&
	(retEncoder = 
	 myRobot->getEncoderPoseInterpPosition(time, &encoderPose)) >= 0)
    {
      deinterlaceTime = packet->getTimeReceived();
      if (!deinterlaceTime.addMSec(-27)) {
        MvrLog::log(MvrLog::Normal,
                   "MvrLMS2xx::sensorInterpCallback() error adding msecs (-27)");
      } 
      if (myInterlaced && 
	  (myRobot->getEncoderPoseInterpPosition(
		  deinterlaceTime, &deinterlaceEncoderPose)) >= 0)
	deinterlace = true;
      else
	deinterlace = false;

      MvrTransform deltaTransform;
      deltaTransform.setTransform(encoderPose);
      deinterlaceDelta = deltaTransform.doInvTransform(deinterlaceEncoderPose);

      processPacket(packet, pose, encoderPose, myRobot->getCounter(),
		    deinterlace, deinterlaceDelta);
      processed.push_back(packet);
    }
    /// MPL changing this since it seems -1 is left out of everything
    //else if (ret < -1 || retEncoder < -1)
    else if (ret < 0 || retEncoder < 0)
    {
      if (myRobot->isConnected())
      {
	ArLog::log(MvrLog::Normal, 
		   "%s::processPacket: too old to process",
		   getName());
      }
      else
      {
	processPacket(packet, pose, encoderPose, myRobot->getCounter(), false,
		      MvrPose());
      }
      processed.push_back(packet);
    }
    else 
    {
      //ArLog::log(MvrLog::Terse, "%s::processPacket: error %d from interpolation\n", getName(), ret);
      //printf("$$$ ret = %d\n", ret);
    }
  }
  while ((it = processed.begin()) != processed.end())
  {
    packet = (*it);
    myPackets.remove(packet);
    processed.pop_front();
    delete packet;
  }
  unlockDevice();
}

/** @internal */
MVREXPORT void *ArLMS2xx::runThread(void *arg)
{
  while (getRunningWithLock())
  {
    lockDevice();
    if (myStartConnect)
    {
      myStartConnect = false;
      switchState(STATE_INIT);
      if (myUseSim)
      {
	unlockDevice();
	internalConnectSim();
      }
      else 
      {
	unlockDevice();
	while (getRunningWithLock())
	{
	  lockDevice();
	  myConnMutex.lock();
	  if (internalConnectHandler() != 0)
	  {
	    myConnMutex.unlock();
	    unlockDevice();
	    break;
	  }
	  myConnMutex.unlock();
	  unlockDevice();
	  MvrUtil::sleep(1);
	}
      }
    } else
      unlockDevice();
    runOnce(true);
    MvrUtil::sleep(1);
  }
  lockDevice();
  if (isConnected())
  {
    disconnect();
  }
  unlockDevice();

  return NULL;
}

