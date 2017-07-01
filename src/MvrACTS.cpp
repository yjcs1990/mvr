#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrACTS.h"


MVREXPORT MvrACTS_1_2::MvrACTS_1_2() :
  mySensorTaskCB(this, &MvrACTS_1_2::actsHandler)
{
  myRobot = NULL;
  myBlobsBad = true;
  myInverted = false;
}

MVREXPORT MvrACTS_1_2::~MvrACTS_1_2()
{

}

/** 
    Opens the port to the ACTS server
    @param robot the robot to attach this to, which puts a sensorInterp on the
    robot so that MvrACTS will always have fresh data from ACTS... giving a 
    NULL value is perfectly acceptable, in this case MvrACTS will not do any
    processing or requesting and you'll have to use receiveBlobInfo and 
    requestPacket (or just call actsHandler)
    @param port the port the ACTS server is running on, default of 5001
    @param host the host the ACTS server is running on, default is localhost
    (ie this machine)
    @return true if the connection was established, false otherwise
*/
MVREXPORT bool MvrACTS_1_2::openPort(MvrRobot *robot, const char *host, int port)
{
  int ret;
  std::string str;
  if ((ret = myConn.open(host, port)) != 0)
  {
    str = myConn.getOpenMessage(ret);
    MvrLog::log(MvrLog::Terse, "MvrACTS_1_2: Could not connect to ACTS running on %s:%d (%s)", host, port, str.c_str()); 
    return false;

  }
  myRequested = false;
  setRobot(robot);
  return true;
}

/**
   Closes the port to the ACTS server
   @return true if the connection was closed properly, false otherwise
*/
MVREXPORT bool MvrACTS_1_2::closePort(void)
{
  return myConn.close();
}

/**
   Requests a packet from the ACTS server, specifically it sends the 
   request to the acts server over its connection
   @return true if the command was sent succesfully, false otherwise
*/
MVREXPORT bool MvrACTS_1_2::requestPacket(void)
{
  const char c = '0';
  if (myConn.getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrACTS_1_2::requestPacket: No connection to ACTS.\n");
    return false;
  }
  return myConn.write(&c, 1);
}

/**
   Sends a command to the ACTS server requesting that ACTS quit
   @return true if the request was sent succesfully, false otherwise
*/
MVREXPORT bool MvrACTS_1_2::requestQuit(void)
{
  const char c = '1';
  if (myConn.getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrACTS_1_2::requestQuit: No connection to ACTS.\n");
    return false;
  }
  return myConn.write(&c, 1);
}

/**
   Checks the connection to the ACTS server for data, if data is there it 
   fills in the blob information, otherwise just returns false
   @return true if there was new data and the data could be read succesfully
*/
MVREXPORT bool MvrACTS_1_2::receiveBlobInfo(void)
{
  int i;
  char *data;
  int numBlobs = 0;

  myBlobsBad = true;
  if (myConn.getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    MvrLog::log(MvrLog::Verbose,
	       "MvrACTS_1_2::receiveBlobInfo: No connection to ACTS.\n");
    return false;
  }

  if (!myConn.read(myData, NUM_CHANNELS*4, 20))
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrACTS_1_2::receiveBlobInfo: Couldn't get the blob stats.\n");
    return false;
  }
  data = myData;
  for (i = 0; i < NUM_CHANNELS; i++)
  {
    myBlobIndex[i] = (*(data++))-1;
    myBlobIndex[i] = myBlobIndex[i] << 6;
    myBlobIndex[i] |= (*(data++))-1;
    
    myBlobNum[i] = (*(data++))-1;
    myBlobNum[i] = myBlobNum[i] << 6;
    myBlobNum[i] |= (*(data++))-1;
    numBlobs += myBlobNum[i];
  }
  if (numBlobs == 0)
    return true;
  if (!myConn.read(myData, numBlobs * ACTS_BLOB_DATA_SIZE, 10))
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrACTS_1_2::receiveBlobInfo: Couldn't read blob data.\n");
    return false;
  }
  myBlobsBad = false;
  return true;
}

/**
   @return the number of blobs on the channel, or -1 if the channel is invalid
*/
MVREXPORT int MvrACTS_1_2::getNumBlobs(int channel)
{

  if (channel >= 1 && channel <= NUM_CHANNELS)
  {
    --channel;
    return myBlobNum[channel];
  }
  else
    return -1;
}

int MvrACTS_1_2::getData(char *rawData)
{
  int temp;
  temp = (*(rawData++)) - 1;
  temp = temp << 6;
  temp |= (*(rawData++)) - 1;
  
  return temp;
}

/**
   Gets the blobNumber from the channel given, fills the information for 
   that blob into the given blob structure.
   @param channel the channel to get the blob from
   @param blobNumber the number of the blob to get from the given channel
   @param blob the blob instance to fill in with the data about the requested
   blob
   @return true if the blob instance could be filled in from the 
*/
MVREXPORT bool MvrACTS_1_2::getBlob(int channel, int blobNumber, MvrACTSBlob *blob)
{
  char * blobInfo;
  int i;
  int temp;

  if (myBlobsBad)
  {
    MvrLog::log(MvrLog::Verbose, 
	       "MvrACTS_1_2::getBlob: There is no valid blob data.\n");
    return false;
  }

  if (channel <= 0 || channel > NUM_CHANNELS) 
  {
    MvrLog::log(MvrLog::Normal,
	       "MvrACTS_1_2::getBlob: Channel %d out of range 1 to %d\n", 
	       channel, NUM_CHANNELS);
    return false;
  }
  --channel;
  
  if (blobNumber <= 0 || blobNumber > myBlobNum[channel])
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrACTS_1_2::getBlob: Blob number %d out of range 1 to %d", 
	       blobNumber,  myBlobNum[channel]);
    return false;
  }
  --blobNumber;

  blobInfo = myData + (myBlobIndex[channel]+blobNumber) * ACTS_BLOB_DATA_SIZE;
  
  temp = 0;
  for (i = 0; i < 4; i++)
  {
    temp = temp << 6;
    temp |= (*(blobInfo++)) - 1;
  }
  blob->setArea(temp);
  
  blob->setXCG(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setYCG(invertY(getData(blobInfo)));
  blobInfo += 2;

  blob->setLeft(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setRight(invertX(getData(blobInfo)));
  blobInfo += 2;

  blob->setTop(invertY(getData(blobInfo)));
  blobInfo += 2;

  blob->setBottom(invertY(getData(blobInfo)));
  blobInfo += 2;

  return true;
}

int MvrACTS_1_2::invertX(int before)
{
  if (myInverted)
    return myWidth - before;
  else
    return before;
}

int MvrACTS_1_2::invertY(int before)
{
  if (myInverted)
    return myHeight - before;
  else
    return before;
}

/**
   This inverts the image, but since ACTS doesn't tell this driver the
   height or width, you need to provide both of those for the image,
   default is 160x120.
   @param width the width of the images acts is grabbing (pixels)
   @param height the height of the images acts is grabbing (pixels)
**/
MVREXPORT void MvrACTS_1_2::invert(int width, int height)
{
  myInverted = true;
  myWidth = true;
  myHeight = true;
}

MVREXPORT bool MvrACTS_1_2::isConnected(void)
{
  if (myConn.getStatus() != MvrDeviceConnection::STATUS_OPEN)
    return false;
  else
    return true;
}

MVREXPORT MvrRobot *MvrACTS_1_2::getRobot(void)
{
  return myRobot;
}
MVREXPORT void MvrACTS_1_2::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL) 
  {
    myRobot->remSensorInterpTask(&mySensorTaskCB);
    myRobot->addSensorInterpTask("acts Handler", 50, &mySensorTaskCB);
  }
}

MVREXPORT void MvrACTS_1_2::actsHandler(void)
{
  if (!myRequested || receiveBlobInfo())
  {
    myRequested = true;
    requestPacket();
  }

}
