#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRVisionPTZ.h"
#include "MvrRobot.h"
#include "MvrCommands.h"

MVREXPORT MvrRVisionPacket::MvrRVisionPacket(MvrTypes::UByte2 bufferSize) :
  MvrBasePacket(bufferSize)
{

}

MVREXPORT MvrRVisionPacket::~MvrRVisionPacket()
{

}

MVREXPORT void MvrRVisionPacket::uByteToBuf(MvrTypes::UByte val)
{
  if (myLength + 1 > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrRVisionPacket::uByteToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val;
  ++myLength;
}

MVREXPORT void MvrRVisionPacket::byte2ToBuf(MvrTypes::Byte2 val)
{
  if ((myLength + 4) > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrRVisionPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = (val & 0xf000) >> 12;
  ++myLength;
  myBuf[myLength] = (val & 0x0f00) >> 8;
  ++myLength;
  myBuf[myLength] = (val & 0x00f0) >> 4;
  ++myLength;
  myBuf[myLength] = (val & 0x000f) >> 0;
  ++myLength;
}

/**
   This function is my concession to not rebuilding a packet from scratch
   for every command, basicaly this is to not lose all speed over just using
   a character array.  This is used by the default rvision commands, unless
   you have a deep understanding of how the packets are working and what
   the packet structure looks like you should not play with this function, 
   it also isn't worth it unless you'll be sending commands frequently.
   @param val the Byte2 to put into the packet
   @param pose the position in the packets array to put the value
*/
MVREXPORT void MvrRVisionPacket::byte2ToBufAtPos(MvrTypes::Byte2 val,
					    MvrTypes::UByte2 pose)
{
  MvrTypes::Byte2 prevLength = myLength;

  if ((pose + 4) > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrRVisionPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myLength = pose;
  byte2ToBuf(val);
  myLength = prevLength;
}


MVREXPORT MvrRVisionPTZ::MvrRVisionPTZ(MvrRobot *robot) :
  MvrPTZ(NULL),
  myPacket(255), 
  myZoomPacket(9),
  mySerialPort(MvrUtil::COM4)
{
  //myRobot = robot;
  initializePackets();
  
  // these ticks were derived emperically.  Ticks / real_degrees
  myDegToTilt = 2880 / 180;
  myDegToPan =  960 / 60;

  myTiltOffsetInDegrees = TILT_OFFSET_IN_DEGREES; 
  myPanOffsetInDegrees = PAN_OFFSET_IN_DEGREES; 

  myConn = NULL;

  MvrPTZ::setLimits(MAX_PAN, MIN_PAN, MAX_TILT, MIN_TILT, MAX_ZOOM, MIN_ZOOM);
    /*
  MVREXPORT virtual double getMaxPosPan(void) const { return MAX_PAN; }
  MVREXPORT virtual double getMaxNegPan(void) const { return MIN_PAN; }
  MVREXPORT virtual double getMaxPosTilt(void) const { return MAX_TILT; }
  MVREXPORT virtual double getMaxNegTilt(void) const { return MIN_TILT; }
  MVREXPORT virtual int getMaxZoom(void) const { return MAX_ZOOM; }
  MVREXPORT virtual int getMinZoom(void) const { return MIN_ZOOM; }
  */

}

MVREXPORT MvrRVisionPTZ::~MvrRVisionPTZ()
{
}

void MvrRVisionPTZ::initializePackets(void)
{
  myZoomPacket.empty();
  myZoomPacket.uByteToBuf(0x81);
  myZoomPacket.uByteToBuf(0x01);
  myZoomPacket.uByteToBuf(0x04);
  myZoomPacket.uByteToBuf(0x47);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0xff);

  myPanTiltPacket.empty();
  myPanTiltPacket.uByteToBuf(0x81);
  myPanTiltPacket.uByteToBuf(0x01);
  myPanTiltPacket.uByteToBuf(0x06);
  myPanTiltPacket.uByteToBuf(0x02);
  myPanTiltPacket.uByteToBuf(0x18);
  myPanTiltPacket.uByteToBuf(0x14);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0xff);
}


MVREXPORT bool MvrRVisionPTZ::init(void)
{
  // send command to power on camera on seekur
  if(myRobot)
  {
	  MvrLog::log(MvrLog::Normal, "MvrRVisionPTZ: turning camera power on ...");
	  myRobot->com2Bytes(116, 12, 1);
  }
  
  myConn = getDeviceConnection();
  if(!myConn)
  {
     MvrLog::log(MvrLog::Normal, "MvrRVisionPTZ: new connection to camera on %s...", mySerialPort);
     MvrSerialConnection *ser = new MvrSerialConnection();
     if(ser->open(mySerialPort) != 0)
     {
	MvrLog::log(MvrLog::Terse, "MvrRVisionPTZ: error opening %s for camera PTZ control, initialization failed.", mySerialPort);
        myConn = NULL;
        return false;
     }
     myConn = ser;
  }
     
  myPacket.empty();
  myPacket.uByteToBuf(0x88);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x00);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0xff);
  myPacket.uByteToBuf(0x88);
  myPacket.uByteToBuf(0x30);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0xff);

  if (!sendPacket(&myPacket))
  {
    MvrLog::log(MvrLog::Terse, "MvrRVisionPTZ: Error sending initialization packet to RVision camera!");
    return false;
  }
  if (!panTilt(0, 0))
    return false;
  if (!zoom(0))
    return false;
  return true;
}

MVREXPORT bool MvrRVisionPTZ::panTilt_i(double degreesPan, double degreesTilt)
{
  if (degreesPan > MAX_PAN)
    degreesPan = MAX_PAN;
  if (degreesPan < MIN_PAN)
    degreesPan = MIN_PAN;
  myPan = degreesPan;

  if (degreesTilt > MAX_TILT)
    degreesTilt = MAX_TILT;
  if (degreesTilt < MIN_TILT)
    degreesTilt = MIN_TILT;
  myTilt = degreesTilt;

  myPanTiltPacket.byte2ToBufAtPos(MvrMath::roundInt((myPan+myPanOffsetInDegrees) * myDegToPan), 6);
  myPanTiltPacket.byte2ToBufAtPos(MvrMath::roundInt((myTilt+myTiltOffsetInDegrees) * myDegToTilt), 10);
  return sendPacket(&myPanTiltPacket);
}

MVREXPORT bool MvrRVisionPTZ::panTiltRel_i(double degreesPan, double degreesTilt)
{
  return panTilt_i(myPan + degreesPan, myTilt + degreesTilt);
}

MVREXPORT bool MvrRVisionPTZ::pan_i(double degrees)
{
  return panTilt_i(degrees, myTilt);
}

MVREXPORT bool MvrRVisionPTZ::panRel_i(double degrees)
{
  return panTiltRel_i(degrees, 0);
}

MVREXPORT bool MvrRVisionPTZ::tilt_i(double degrees)
{
  return panTilt_i(myPan, degrees);
}

MVREXPORT bool MvrRVisionPTZ::tiltRel_i(double degrees)
{
  return panTiltRel_i(0, degrees);
}

MVREXPORT bool MvrRVisionPTZ::zoom(int zoomValue)
{
  //printf("MvrRVision::zoom(%d)\n", zoomValue);
  if (zoomValue > MAX_ZOOM)
    zoomValue = MAX_ZOOM;
  if (zoomValue < MIN_ZOOM)
    zoomValue = MIN_ZOOM;
  myZoom = zoomValue;
    
  myZoomPacket.byte2ToBufAtPos(MvrMath::roundInt(myZoom), 4);
  return sendPacket(&myZoomPacket);
}

MVREXPORT bool MvrRVisionPTZ::zoomRel(int zoomValue)
{
  return zoom(myZoom + zoomValue);
}

/*
MVREXPORT bool MvrRVisionPTZ::packetHandler(MvrRobotPacket *packet)
{
  if (packet->getID() != 0xE0)
    return false;

  return true;
}
*/

#define MVRRVISION_MAX_RESPONSE_BYTES 16
//MVREXPORT bool MvrRVisionPTZ::packetHandler(MvrBasePacket *packet)
MvrBasePacket * MvrRVisionPTZ::readPacket(void)
{
  unsigned char data[MVRRVISION_MAX_RESPONSE_BYTES];
  unsigned char byte;
  int num;
  memset(data, 0, MVRRVISION_MAX_RESPONSE_BYTES);
  for (num=0; num <= MVRRVISION_MAX_RESPONSE_BYTES+1; num++) {
    if (myConn->read((char *) &byte, 1,1) <= 0 ||
	num == MVRRVISION_MAX_RESPONSE_BYTES+1) {
      return NULL;
    }
    else if (byte == 0x90) {
      data[0] = byte;
      //printf("MvrRVisionPTZ::packetHandler:  got 0x%x, expecting packet\n", byte);
      break;
    }
    else {
      //printf("MvrRVisionPTZ::packetHandler:  got 0x%x, skipping\n", byte);
    }
  }
  // we got the header
  for (num=1; num <= MVRRVISION_MAX_RESPONSE_BYTES; num++) {
    if (myConn->read((char *) &byte, 1, 1) <= 0) {
      // there are no more bytes, so check the last byte for the footer
      if (data[num-1] != 0xFF) {
	//printf("MvrRVisionPTZ::packetHandler: should have gotten 0xFF, got 0x%x\n", data[num-1]);
	return NULL;
      }
      else {
	break;
      }
    }
    else {
      // add the byte to the array
      data[num] = byte;
    }
  }
  // print the data for now
  //printf("MvrRVisionPTZ::packetHandler: got packet!\n");
  //for (int i=0; i <= num; i++) {
  //printf("\t[%d]: 0x%x\n", i, data[i]);
  //}
  return NULL;
}

MvrPTZConnector::GlobalPTZCreateFunc MvrRVisionPTZ::ourCreateFunc(&MvrRVisionPTZ::create);

MvrPTZ* MvrRVisionPTZ::create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot)
{
  MvrRVisionPTZ *ptz = new MvrRVisionPTZ(robot);
  if(params.serialPort != "" && params.serialPort != "none")
	  ptz->setPort(params.serialPort.c_str());
  return ptz;
}

void MvrRVisionPTZ::registerPTZType()
{
  MvrPTZConnector::registerPTZType("rvision", &ourCreateFunc);
}
