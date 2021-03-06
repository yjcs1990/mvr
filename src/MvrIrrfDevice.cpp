#include "MvrExport.h"

#include "mvriaOSDef.h"
#include "MvrIrrfDevice.h"
#include "MvrCommands.h"

MVREXPORT MvrIrrfDevice::MvrIrrfDevice(size_t currentBufferSize,
                             size_t cumulativeBufferSize, const char *name) :
  MvrRangeDevice(currentBufferSize, cumulativeBufferSize, name, 5000),
  myPacketHandler(this, &MvrIrrfDevice::packetHandler)
{
  int i;
  myRobot = NULL;
  // These numbers haven't been proven, yet.  It will take experimentation to maximize the best results
  myCumulativeMaxRange = 10000;
  myMaxRange = 5000;
  myFilterFarDist = 7000;
  myFilterNearDist = 50;

  myPacketHandler.setName("MvrIrrfDevice");
  // The 91 readings start at -81 degrees, and move in 1.8 degree steps
  myRawReadings = new std::list<MvrSensorReading *>;
  for(i=0;i<91;i++)
    myRawReadings->push_back(new MvrSensorReading(0, 0, (1.8*i - 81)));
}

MVREXPORT MvrIrrfDevice::~MvrIrrfDevice()
{
  if (myRobot != NULL)
  {
    myRobot->remPacketHandler(&myPacketHandler);
    myRobot->remRangeDevice(this);
  }
}

MVREXPORT void MvrIrrfDevice::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addPacketHandler(&myPacketHandler, MvrListPos::LAST);
}

MVREXPORT void MvrIrrfDevice::processReadings(void)
{
  int i;
  double rx, ry, nx, ny, dx, dy, dist;
  MvrSensorReading *reading;
  std::list<MvrSensorReading *>::iterator rawIt;
  std::list<MvrPoseWithTime *> *readingList;
  std::list<MvrPoseWithTime *>::iterator readIt;
  lockDevice();

  rx = myRobot->getX();
  ry = myRobot->getY();

  i=0;
  for (rawIt = myRawReadings->begin();rawIt != myRawReadings->end();rawIt++)
  {
    reading = (*rawIt);
    nx = reading->getX();
    ny = reading->getY();
    dx = nx - rx;
    dy = nx - ry;
    dist = (dx*dx) + (dy*dy);
    if (!reading->isNew(myRobot->getCounter()))
      continue;

    if (dist < (myMaxRange * myMaxRange))
      myCurrentBuffer.addReading(nx, ny);

    if (dist < (myCumulativeMaxRange * myCumulativeMaxRange))
    {
      myCumulativeBuffer.beginInvalidationSweep();
      readingList = myCumulativeBuffer.getBuffer();

      if (readingList != NULL)
      {
        for (readIt = readingList->begin(); 
	     readIt != readingList->end();
	     readIt++)
        {
          dx = (*readIt)->getX() - nx;
          dy = (*readIt)->getY() - ny;
          if ((dx*dx + dy*dy) < (myFilterNearDist * myFilterNearDist))
            myCumulativeBuffer.invalidateReading(readIt);
        }
      }
      myCumulativeBuffer.endInvalidationSweep();
      myCumulativeBuffer.addReading(nx, ny);
    }
  }
 
  readingList = myCumulativeBuffer.getBuffer();

  rx = myRobot->getX();
  ry = myRobot->getY();

  myCumulativeBuffer.beginInvalidationSweep();
  if (readingList != NULL)
  {
    for (readIt = readingList->begin(); readIt != readingList->end();readIt++)
    {
      dx = (*readIt)->getX() - rx;
      dy = (*readIt)->getY() - ry;
      if ((dx*dx + dy*dy) > (myFilterFarDist * myFilterFarDist))
        myCumulativeBuffer.invalidateReading(readIt);
    }
  }
  myCumulativeBuffer.endInvalidationSweep();

  unlockDevice();
}

/**
  This is the packet handler for the PB9 data, which is sent via the micro
  controller, to the client.  This will read the data from the packets,
  and then call processReadings to filter add the data to the current and
  cumulative buffers.
*/
MVREXPORT bool MvrIrrfDevice::packetHandler(MvrRobotPacket *packet)
{
  int portNum, i, dist, packetCounter;
  double conv;
  MvrTransform packetTrans;
  std::list<MvrSensorReading *>::iterator it;
  MvrSensorReading *reading;
  MvrPose pose;
  MvrTransform encoderTrans;
  MvrPose encoderPose;

  pose = myRobot->getPose();
  conv = 2.88;

  packetTrans.setTransform(pose);
  packetCounter = myRobot->getCounter();

  if (packet->getID() != 0x10)
    return false;

  // Which Aux port the IRRF is connected to
  portNum = packet->bufToByte2();
  encoderTrans = myRobot->getEncoderTransform();
  encoderPose = encoderTrans.doInvTransform(pose);

  i = 0;
  for (i=0, it = myRawReadings->begin();it != myRawReadings->end();it++, i++)
  {
    reading = (*it);
    dist = (int) ((packet->bufToUByte2()) / conv);
    reading->newData(dist, pose, encoderPose, packetTrans, packetCounter, packet->getTimeReceived());
  }

  myLastReading.setToNow();

  processReadings();

  return true;
}
