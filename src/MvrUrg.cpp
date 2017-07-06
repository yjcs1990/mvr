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
#include "MvrUrg.h"
#include "MvrRobot.h"
#include "MvrSerialConnection.h"
#include "mvriaInternal.h"

MVREXPORT MvrUrg::MvrUrg(int laserNumber, const char *name) :
  MvrLaser(laserNumber, name, 4095),
  mySensorInterpTask(this, &MvrUrg::sensorInterp),
  myMvrExitCB(this, &MvrUrg::disconnect)
{
  clear();
  myRawReadings = NULL;

  Mvria::addExitCallback(&myMvrExitCB, -10);

  laserSetName(getName());

  setParams();
  setSensorPosition(0, 0, 0);     

  laserAllowSetDegrees(-120, -135, 135, // default, min and max start degrees
		  120, -135, 135); // default, min and max end degrees

  laserAllowSetIncrement(1, 0, 135);
  
  std::list<std::string> baudChoices;
  baudChoices.push_back("019200");
  baudChoices.push_back("057600");
  baudChoices.push_back("115200");
  baudChoices.push_back("250000");
  baudChoices.push_back("500000");
  baudChoices.push_back("750000");
  laserAllowStartingBaudChoices("019200", baudChoices);

  laserAllowAutoBaudChoices("057600", baudChoices);


  setMinDistBetweenCurrent(0);
  setMaxDistToKeepCumulative(4000);
  setMinDistBetweenCumulative(200);
  setMaxSecondsToKeepCumulative(30);
  setMaxInsertDistCumulative(3000);

  setCumulativeCleanDist(75);
  setCumulativeCleanInterval(1000);
  setCumulativeCleanOffset(600);

  resetLastCumulativeCleanTime();

  setCurrentDrawingData(
	  new MvrDrawingData("polyDots", 
			    MvrColor(0, 0xaa, 0xaa), 
			    70,  // mm diameter of dots
			    77), true);
  setCumulativeDrawingData(
	  new MvrDrawingData("polyDots", 
			    MvrColor(0, 0x55, 0x55), 
			    100,  // mm diameter of dots
			    62), true);

  //myLogMore = false;
  myLogMore = true;
}

MVREXPORT MvrUrg::~MvrUrg()
{
  Mvria::remExitCallback(&myMvrExitCB);
  if (myRobot != NULL)
  {
    myRobot->remRangeDevice(this);
    myRobot->remLaser(this);
    myRobot->remSensorInterpTask(&mySensorInterpTask);
  }
  if (myRawReadings != NULL)
  {
    MvrUtil::deleteSet(myRawReadings->begin(), myRawReadings->end());
    myRawReadings->clear(); 
    delete myRawReadings;
    myRawReadings = NULL;
  }
  lockDevice();
  if (isConnected())
    disconnect();
  unlockDevice();
}

MVREXPORT void MvrUrg::laserSetName(const char *name)
{
  myName = name;
  
  myConnMutex.setLogNameVar("%s::myConnMutex", getName());
  myReadingMutex.setLogNameVar("%s::myReadingMutex", getName());
  myDataMutex.setLogNameVar("%s::myDataMutex", getName());
  myMvrExitCB.setNameVar("%s::exitCallback", getName());
  
  MvrLaser::laserSetName(getName());
}

MVREXPORT bool MvrUrg::setParams(
	double startingDegrees, double endingDegrees,
	double incrementDegrees, bool flipped)
{
  if (startingDegrees < -135.0 || startingDegrees > 135.0 || 
      endingDegrees < -135.0 || startingDegrees > 135.0 || 
      incrementDegrees < 0.0)
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s::setParams: Bad params (starting %g ending %g clusterCount %g)",
	       getName(), startingDegrees, endingDegrees, incrementDegrees);
    return false;
  }
  double startingStepRaw;
  double endingStepRaw;
  int startingStep;
  int endingStep;
  int clusterCount;
  
  startingStepRaw = -(startingDegrees - 135.0) / .3515625;
  endingStepRaw = -(endingDegrees - 135.0) / .3515625;
  startingStep = (int)ceil(MvrUtil::findMin(startingStepRaw, endingStepRaw));
  endingStep = (int)floor(MvrUtil::findMax(startingStepRaw, endingStepRaw));
  clusterCount = MvrMath::roundInt(incrementDegrees / .3515625);
  
  MvrLog::log(MvrLog::Verbose, "%s: starting deg %.1f raw %.1f step %d ending deg %.1f raw %.1f step %d, cluster deg %f count %d flipped %d",
	     getName(), startingDegrees, startingStepRaw, startingStep, endingDegrees, endingStepRaw, endingStep, incrementDegrees, clusterCount, flipped);
  
  return setParamsBySteps(startingStep, endingStep, clusterCount, flipped);
}

MVREXPORT bool MvrUrg::setParamsBySteps(int startingStep, int endingStep, 
				      int clusterCount, bool flipped)
{
  if (startingStep < 0 || startingStep > 768 || 
      endingStep < 0 || endingStep > 768 || 
      startingStep > endingStep || 
      clusterCount < 1)
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s::setParamsBySteps: Bad params (starting %d ending %d clusterCount %d)",
	       getName(), startingStep, endingStep, clusterCount);
    return false;
  }
  
  myDataMutex.lock();
  myStartingStep = startingStep;
  myEndingStep = endingStep;
  myClusterCount = clusterCount;
  myFlipped = flipped;
  sprintf(myRequestString, "G%03d%03d%02d", myStartingStep, endingStep,
	  clusterCount);
  myClusterMiddleAngle = 0;
  if (myClusterCount > 1)
    myClusterMiddleAngle = myClusterCount * 0.3515625 / 2.0;

  if (myRawReadings != NULL)
  {
    MvrUtil::deleteSet(myRawReadings->begin(), myRawReadings->end());
    myRawReadings->clear();
    delete myRawReadings;
    myRawReadings = NULL;
  }

  myRawReadings = new std::list<MvrSensorReading *>;
  

  MvrSensorReading *reading;
  int onStep;
  double angle;

  for (onStep = myStartingStep; 
       onStep < myEndingStep; 
       onStep += myClusterCount)
  {
    /// FLIPPED
    if (!myFlipped)
      angle = MvrMath::subAngle(MvrMath::subAngle(135, onStep * 0.3515625),
			       myClusterMiddleAngle);
    else
      angle = MvrMath::addAngle(MvrMath::addAngle(-135, onStep * 0.3515625), 
			       myClusterMiddleAngle);
			       
    reading = new MvrSensorReading;
    reading->resetSensorPosition(MvrMath::roundInt(mySensorPose.getX()),
				 MvrMath::roundInt(mySensorPose.getY()),
				 MvrMath::addAngle(angle, 
						  mySensorPose.getTh()));
    myRawReadings->push_back(reading);
    //printf("%.1f ", reading->getSensorTh());
  }


  myDataMutex.unlock();
  return true;
}

void MvrUrg::clear(void)
{
  myIsConnected = false;
  myTryingToConnect = false;
  myStartConnect = false;

  myVendor = "";
  myProduct = "";
  myFirmwareVersion = "";
  myProtocolVersion = "";
  mySerialNumber = "";
  myStat = "";
}

MVREXPORT void MvrUrg::log(void)
{
  MvrLog::log(MvrLog::Normal, "URG %s:", getName());
  MvrLog::log(MvrLog::Normal, "Vendor information: %s", myVendor.c_str());
  MvrLog::log(MvrLog::Normal, "Product information: %s", myProduct.c_str());
  MvrLog::log(MvrLog::Normal, "Firmware version: %s", myFirmwareVersion.c_str());
  MvrLog::log(MvrLog::Normal, "Protocol Version: %s", myProtocolVersion.c_str());
  MvrLog::log(MvrLog::Normal, "Serial number: %s", mySerialNumber.c_str());
  MvrLog::log(MvrLog::Normal, "Stat: %s", myStat.c_str());
}

MVREXPORT void MvrUrg::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask("urg", 90, &mySensorInterpTask);
  MvrRangeDevice::setRobot(robot);
}

bool MvrUrg::writeLine(const char *str)
{
  if (myConn == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Attempt to write string to null connection", getName());
    return false;
  }

  bool ret = true;
  size_t len = strlen(str);

  myConnMutex.lock();
  if (myConn->write(str, len) < len || 
      myConn->write("\n", 1) < 1)
    ret = false;
  myConnMutex.unlock();
  return ret;
}

bool MvrUrg::readLine(char *buf, unsigned int size, 
		     unsigned int msWait)
{
  if (myConn == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Attempt to read line from null connection", getName());
    return false;
  }

  MvrTime started;
  started.setToNow();

  buf[0] = '\0';
  unsigned int onChar = 0;
  int ret;

  myConnMutex.lock();
  while ((msWait == 0 || started.mSecSince() < (int)msWait) && 
	 onChar < size)
  {
    if ((ret = myConn->read(&buf[onChar], 1, 0)) > 0)
    {
      if (buf[onChar] == '\n' || 
	  buf[onChar] == '\r')
      {
	//buf[onChar+1] = '\0';
	buf[onChar] = '\0';
	if (myLogMore)
	  MvrLog::log(MvrLog::Normal, "%s: '%s'", getName(), buf);
	myConnMutex.unlock();
	return true;
      }
      onChar++;
    }
    if (ret < 0)
    {
      MvrLog::log(MvrLog::Normal, "%s: bad ret", getName());
      myConnMutex.unlock();
      return false;
    }
    if (ret == 0)
      MvrUtil::sleep(1);
  }
  myConnMutex.unlock();
  return false;
}

bool MvrUrg::sendCommandAndRecvStatus(
	const char *command, const char *commandDesc, 
	char *buf, unsigned int size, unsigned int msWait)
{
  //bool ret;
  MvrTime start;

  // send the command
  if (!writeLine(command))
  {
    MvrLog::log(MvrLog::Normal, "%s: Could not send %s", getName(), 
	       commandDesc);
    return false;
  }

  // try and read the command back
  if (!readLine(buf, size, msWait))
  {
    MvrLog::log(MvrLog::Normal, "%s: Received no response to %s (in %d but really %d)", 
	       getName(), commandDesc, msWait, start.mSecSince());
    return false;
  }

  // make sure the command back matches
  if (strcasecmp(buf, command) != 0)
  {
    MvrLog::log(MvrLog::Normal, "%s: Received bad echo to %s (command %s got back %s)", 
	       getName(), commandDesc, command, buf);
    return false;
  }

  // get the status from that command back
  if (!readLine(buf, size, msWait))
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s: Did not receive status back from %s",
	       getName(), commandDesc);
    return false;
  }

  return true;
}

MVREXPORT bool MvrUrg::blockingConnect(void)
{
  if (!getRunning())
    runAsync();

  myConnMutex.lock();
  if (myConn == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Could not connect because there is no connection defined", 
	       getName());
    myConnMutex.unlock();
    failedToConnect();
    return false;
  }

  MvrSerialConnection *serConn = NULL;
  serConn = dynamic_cast<MvrSerialConnection *>(myConn);

  if (serConn != NULL)
    serConn->setBaud(atoi(getStartingBaudChoice()));

  if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN && 
      !myConn->openSimple())
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Could not connect because the connection was not open and could not open it", 
	       getName());
    myConnMutex.unlock();
    failedToConnect();
    return false;
  }
  myConnMutex.unlock();

  lockDevice();
  myTryingToConnect = true;
  unlockDevice();

  laserPullUnsetParamsFromRobot();
  laserCheckParams();
  
  setParams(getStartDegrees(), getEndDegrees(), getIncrement(), getFlipped());
  
  MvrUtil::sleep(100);

  bool connected = false;

  if (internalConnect())
    connected = true;

  if (connected)
  {
    lockDevice();
    myIsConnected = true;
    myTryingToConnect = false;
    unlockDevice();
    MvrLog::log(MvrLog::Normal, "%s: Connected to laser", getName());
    laserConnect();
    return true;
  }
  else
  {
    failedToConnect();
    return false;
  }
}


bool MvrUrg::internalConnect(void)

{
  bool ret = true;
  char buf[1024];

  MvrSerialConnection *serConn = NULL;
  serConn = dynamic_cast<MvrSerialConnection *>(myConn);

  bool alreadyAtAutobaud = false;

  // empty the buffer...
  buf[0] = '\0';
  while (readLine(buf, sizeof(buf), 1));

  if (!(ret = sendCommandAndRecvStatus(
		"V", "version request", 
		buf, sizeof(buf), 10000)) || 
      strcasecmp(buf, "0") != 0)      
  {
    // if we didn't get it, try it at what the autobaud rate is
    if (serConn != NULL)
    {
      alreadyAtAutobaud = true;
      serConn->setBaud(atoi(getAutoBaudChoice()));
      MvrUtil::sleep(100);

      if (!(ret = sendCommandAndRecvStatus(
		"V", "version request after falling back to autobaudchoice", 
		buf, sizeof(buf), 10000)) || 
	  strcasecmp(buf, "0") != 0)      
      {
	if (ret && strcasecmp(buf, "0") != 0)      
	  MvrLog::log(MvrLog::Normal, 
		     "%s::blockingConnect: Bad status on version response after falling back to autobaudchoice", 
		     getName());
	return false;
      }
    }
    // if we don't have a serial port, then we can't change the baud,
    // so just fail
    else
    {
      if (ret && strcasecmp(buf, "0") != 0)      
	MvrLog::log(MvrLog::Normal, 
		   "%s::blockingConnect: Bad status on version response (%s)",
		   getName(), buf);
      return false;
    }
  }

  if (!alreadyAtAutobaud && serConn != NULL)
  {

    // empty the buffer from the last version request
    while (readLine(buf, sizeof(buf), 100));

    // now change the baud...
    sprintf(buf, "S%06d7654321", atoi(getAutoBaudChoice()));
    if (!writeLine(buf))
      return false;

    MvrUtil::sleep(100);

    //serConn->setBaud(115200);
    serConn->setBaud(atoi(getAutoBaudChoice()));
    // wait a second for the baud to change...
    MvrUtil::sleep(100);

    // empty the buffer from the baud change
    while (readLine(buf, sizeof(buf), 100));

    if (!(ret = sendCommandAndRecvStatus(
		  "V", "version request after switching to autobaudchoice", 
		  buf, sizeof(buf), 10000)) || 
	strcasecmp(buf, "0") != 0)      
    {
      if (ret && strcasecmp(buf, "0") != 0)      
	MvrLog::log(MvrLog::Normal, 
		   "%s::blockingConnect: Bad status on version response after switching to autobaudchoice", 
		   getName());
      return false;
    }

    MvrLog::log(MvrLog::Normal, "%s: Switched to %s baud rate",
	       getName(), getAutoBaudChoice());
  }

  while (readLine(buf, sizeof(buf), 10000))
  {
    /// MPL put this in instead of the following because of the
    /// behavior change of readline
    if (strlen(buf) == 0)
      break;

    if (strncasecmp(buf, "VEND:", strlen("VEND:")) == 0)
      myVendor = &buf[5];
    else if (strncasecmp(buf, "PROD:", strlen("PROD:")) == 0)
      myProduct = &buf[5];
    else if (strncasecmp(buf, "FIRM:", strlen("FIRM:")) == 0)
      myFirmwareVersion = &buf[5];
    else if (strncasecmp(buf, "PROT:", strlen("PROT:")) == 0)
      myProtocolVersion = &buf[5];
    else if (strncasecmp(buf, "SERI:", strlen("SERI:")) == 0)
      mySerialNumber = &buf[5];
    else if (strncasecmp(buf, "STAT:", strlen("STAT:")) == 0)
      myStat = &buf[5];
  }

  if (myVendor.empty() || myProduct.empty() || myFirmwareVersion.empty() || 
      myProtocolVersion.empty() || mySerialNumber.empty())
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s::blockingConnect: Missing information in version response",
	       getName());
    return false;
  }

  log();

  myLogMore = true;
  //  myLogMore = false;
  MvrUtil::sleep(100);
  
  
  printf("myRequestString %s\n", myRequestString);


  if (!(ret = sendCommandAndRecvStatus(
		myRequestString, "request distance reading", 
		buf, sizeof(buf), 10000)) || 
      strcasecmp(buf, "0") != 0)
  {
    if (ret && strcasecmp(buf, "0") != 0) 
      MvrLog::log(MvrLog::Normal, 
	 "%s::blockingConnect: Bad status on distance reading response (%c)",
		 getName(), buf[0]);
    return false;
  }
  
  myLogMore = false;

  MvrTime started;
  started.setToNow();
  while (started.secSince() < 10 && readLine(buf, sizeof(buf), 10000))
  {
    if (strlen(buf) == 0)
      return true;
  }

  MvrLog::log(MvrLog::Normal, "%s::blockingConnect: Did not get distance reading back",
	     getName(), buf[0]);
  return false;
}


MVREXPORT bool MvrUrg::asyncConnect(void)
{
  myStartConnect = true;
  if (!getRunning())
    runAsync();
  return true;
}

MVREXPORT bool MvrUrg::disconnect(void)
{
  if (!isConnected())
    return true;

  MvrLog::log(MvrLog::Normal, "%s: Disconnecting", getName());

  bool ret;

  char buf[2048];
  sprintf(buf, "S%06d7654321", atoi(getStartingBaudChoice()));
  ret = writeLine(buf);
  laserDisconnectNormally();
  return ret;

}

void MvrUrg::sensorInterp(void)
{
  MvrTime readingRequested;
  std::string reading;
  myReadingMutex.lock();
  if (myReading.empty())
  {
    myReadingMutex.unlock();
    return;
  }

  readingRequested = myReadingRequested;
  reading = myReading;
  myReading = "";
  myReadingMutex.unlock();

  MvrTime time = readingRequested;
  MvrPose pose;
  int ret;
  int retEncoder;
  MvrPose encoderPose;

  //time.addMSec(-13);
  if (myRobot == NULL || !myRobot->isConnected())
  {
    pose.setPose(0, 0, 0);
    encoderPose.setPose(0, 0, 0);
  } 
  else if ((ret = myRobot->getPoseInterpPosition(time, &pose)) < 0 ||
	   (retEncoder = 
	    myRobot->getEncoderPoseInterpPosition(time, &encoderPose)) < 0)
  {
    MvrLog::log(MvrLog::Normal, "%s: reading too old to process", getName());
    return;
  }

  MvrTransform transform;
  transform.setTransform(pose);

  unsigned int counter = 0; 
  if (myRobot != NULL)
    counter = myRobot->getCounter();

  lockDevice();
  myDataMutex.lock();

  //double angle;
  int i;
  int len = reading.size();

  int range;
  int big; 
  int little;
  //int onStep;

  std::list<MvrSensorReading *>::reverse_iterator it;
  MvrSensorReading *sReading;

  bool ignore;
  for (it = myRawReadings->rbegin(), i = 0; 
       it != myRawReadings->rend() && i < len - 1; 
       it++, i += 2)
  {
    ignore = false;
    big = reading[i] - 0x30;
    little = reading[i+1] - 0x30;
    range = (big << 6 | little);
    if (range < 20)
    {
      /* Well that didn't work...
      // if the range is 1 to 5 that means it has low intensity, which
      // could be black or maybe too far... try ignoring it and see if
      // it helps with too much clearing
      if (range >= 1 || range <= 5)
	ignore = true;
      */
      range = 4096;
    }
    sReading = (*it);
    sReading->newData(range, pose, encoderPose, transform, counter, 
		      time, ignore, 0);
  }

  myDataMutex.unlock();

  laserProcessReadings();
  unlockDevice();
}

MVREXPORT void * MvrUrg::runThread(void *arg)
{
  while (getRunning())
  {
    lockDevice();
    if (myStartConnect)
    {
      myStartConnect = false;
      myTryingToConnect = true;
      unlockDevice();

      blockingConnect();

      lockDevice();
      myTryingToConnect = false;
      unlockDevice();
      continue;
    }
    unlockDevice();
    
    if (!myIsConnected)
    {
      MvrUtil::sleep(100);
      continue;
    }

    // if we have a robot but it isn't running yet then don't have a
    // connection failure
    if (laserCheckLostConnection())
    {
      MvrLog::log(MvrLog::Terse, 
		 "%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(), 
		 myLastReading.mSecSince()/1000.0, 
		 getConnectionTimeoutSeconds());
      myIsConnected = false;
      laserDisconnectOnError();
      continue;
    }

    internalGetReading();

    MvrUtil::sleep(1);

  }
  return NULL;
}

bool MvrUrg::internalGetReading(void)
{
  MvrTime readingRequested;
  std::string reading;
  char buf[1024];

  reading = "";
  readingRequested.setToNow();
  if (!writeLine(myRequestString))
  {
    MvrLog::log(MvrLog::Terse, "Could not send request distance reading to urg");
    return false;
  }
  
  if (!readLine(buf, sizeof(buf), 10000) || 
      strcasecmp(buf, myRequestString) != 0)
  {
    MvrLog::log(MvrLog::Normal, "%s: Did not get distance reading response",
	       getName());
    return false;
  }
  
  if (!readLine(buf, sizeof(buf), 10000) || 
      strcasecmp(buf, "0") != 0)		  
  {
    MvrLog::log(MvrLog::Normal, 
	       "%s: Bad status on distance reading response (%c)",
	       getName(), buf[0]);
    return false;
  }
  
  while (readLine(buf, sizeof(buf), 10000))
  {
    if (strlen(buf) == 0)
    {
      myReadingMutex.lock();
      myReadingRequested = readingRequested;
      myReading = reading;
      myReadingMutex.unlock();	
      if (myRobot == NULL)
	sensorInterp();
      return true;
    }
    else
    {
      reading += buf;
    }
  }

  return false;
}

void MvrUrg::failedToConnect(void)
{
  lockDevice();
  myTryingToConnect = true;
  unlockDevice();
  laserFailedConnect();
}

bool MvrUrg::laserCheckParams(void)
{
  laserSetAbsoluteMaxRange(4095);
  return true;
}
