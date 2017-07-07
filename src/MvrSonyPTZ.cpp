#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSonyPTZ.h"
#include "MvrRobot.h"
#include "MvrCommands.h"

MVREXPORT MvrSonyPacket::MvrSonyPacket(MvrTypes::UByte2 bufferSize) :
  MvrBasePacket(bufferSize)
{
  
}

MVREXPORT MvrSonyPacket::~MvrSonyPacket()
{

}

MVREXPORT void MvrSonyPacket::uByteToBuf(MvrTypes::UByte val)
{
  if (myLength + 1 > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrSonyPacket::uByteToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val;
  ++myLength;
}

MVREXPORT void MvrSonyPacket::byte2ToBuf(MvrTypes::Byte2 val)
{
  if ((myLength + 4) > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrSonyPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
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
   a character array.  This is used by the default sony commands, unless
   you have a deep understanding of how the packets are working and what
   the packet structure looks like you should not play with this function, 
   it also isn't worth it unless you'll be sending commands frequently.
   @param val the Byte2 to put into the packet
   @param pose the position in the packets array to put the value
*/
MVREXPORT void MvrSonyPacket::byte2ToBufAtPos(MvrTypes::Byte2 val,
					    MvrTypes::UByte2 pose)
{
  MvrTypes::Byte2 prevLength = myLength;

  if ((pose + 4) > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrSonyPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myLength = pose;
  byte2ToBuf(val);
  myLength = prevLength;
}


MVREXPORT MvrSonyPTZ::MvrSonyPTZ(MvrRobot *robot) :
  MvrPTZ(robot),
  myPacket(255), 
  myZoomPacket(9)
{
  myRobot = robot;
  initializePackets();

  setLimits(90, -90, 30, -30, 1024, 0);
    /*
  MVREXPORT virtual double getMaxPosPan(void) const { return 90; }
  MVREXPORT virtual double getMaxNegPan(void) const { return -90; }
  MVREXPORT virtual double getMaxPosTilt(void) const { return 30; }
  MVREXPORT virtual double getMaxNegTilt(void) const { return -30; }
  MVREXPORT virtual int getMaxZoom(void) const { return 1024; }
  MVREXPORT virtual int getMinZoom(void) const { return 0; }
  */
  
  myDegToTilt = 0x12c / ((double) getMaxTilt() /*MAX_TILT*/ );
  myDegToPan = 0x370 / ((double) getMaxPan() /*MAX_PAN*/ );
}

MVREXPORT MvrSonyPTZ::~MvrSonyPTZ()
{
}

void MvrSonyPTZ::initializePackets(void)
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


MVREXPORT bool MvrSonyPTZ::init(void)
{
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
    return false;
  if (!panTilt(0, 0))
    return false;
  if (!zoom(0))
    return false;
  return true;
}

MVREXPORT bool MvrSonyPTZ::backLightingOn(void)
{
  myPacket.empty();
  myPacket.uByteToBuf(0x81);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x04);
  myPacket.uByteToBuf(0x33);
  myPacket.uByteToBuf(0x02);
  myPacket.uByteToBuf(0xff);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrSonyPTZ::backLightingOff(void)
{
  myPacket.empty();
  myPacket.uByteToBuf(0x81);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x04);
  myPacket.uByteToBuf(0x33);
  myPacket.uByteToBuf(0x03);
  myPacket.uByteToBuf(0xff);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrSonyPTZ::panTilt_i(double degreesPan, double degreesTilt)
{
  if (degreesPan > getMaxPan())
    degreesPan = getMaxPan();
  if (degreesPan < getMinPan())
    degreesPan = getMinPan();
  myPan = degreesPan;

  if (degreesTilt > getMaxTilt())
    degreesTilt = getMaxTilt();
  if (degreesTilt < getMinTilt())
    degreesTilt = getMinTilt();
  myTilt = degreesTilt;

  myPanTiltPacket.byte2ToBufAtPos(MvrMath::roundInt(myPan * myDegToPan), 6);
  myPanTiltPacket.byte2ToBufAtPos(MvrMath::roundInt(myTilt * myDegToTilt), 10);
  return sendPacket(&myPanTiltPacket);
}

MVREXPORT bool MvrSonyPTZ::panTiltRel_i(double degreesPan, double degreesTilt)
{
  return panTilt(myPan + degreesPan, myTilt + degreesTilt);
}

MVREXPORT bool MvrSonyPTZ::pan_i(double degrees)
{
  return panTilt(degrees, myTilt);
}

MVREXPORT bool MvrSonyPTZ::panRel_i(double degrees)
{
  return panTiltRel(degrees, 0);
}

MVREXPORT bool MvrSonyPTZ::tilt_i(double degrees)
{
  return panTilt(myPan, degrees);
}

MVREXPORT bool MvrSonyPTZ::tiltRel_i(double degrees)
{
  return panTiltRel(0, degrees);
}

MVREXPORT bool MvrSonyPTZ::zoom(int zoomValue)
{
  if (zoomValue > getMaxZoom())
    zoomValue = getMaxZoom();
  if (zoomValue < getMinZoom())
    zoomValue = getMinZoom();
  myZoom = zoomValue;
    
  myZoomPacket.byte2ToBufAtPos(MvrMath::roundInt(myZoom), 4);
  return sendPacket(&myZoomPacket);
}

MVREXPORT bool MvrSonyPTZ::zoomRel(int zoomValue)
{
  return zoom(myZoom + zoomValue);
}


/*
MVREXPORT bool MvrSonyPTZ::packetHandler(MvrRobotPacket *packet)
{
  if (packet->getID() != 0xE0)
    return false;

  return true;
}
*/

MvrPTZConnector::GlobalPTZCreateFunc MvrSonyPTZ::ourCreateFunc(&MvrSonyPTZ::create);

MvrPTZ* MvrSonyPTZ::create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot)
{
  return new MvrSonyPTZ(robot);
}

void MvrSonyPTZ::registerPTZType()
{
  MvrPTZConnector::registerPTZType("sony", &ourCreateFunc);
}
