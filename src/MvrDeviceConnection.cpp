#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrDeviceConnection.h"

bool MvrDeviceConnection::ourStrMapInited = false;
MvrStrMap MvrDeviceConnection::ourStrMap;
bool MvrDeviceConnection::ourDCDebugShouldLog = false;
MvrTime MvrDeviceConnection::ourDCDebugFirstTime;

/**
   Subclasses of this connection type should call setDCPortType in
   their constructor, then setDCPortName in their openPort.
   
   Classes that use a device connection should call setDCDeviceName
   with the device the connection is attached too (usually in
   setDeviceConnection on the device)...

   Things that read the port should call debugStartPacket whenever
   they are starting reading a packet...  Then debugBytesRead with the
   byte count whenever they read or fail a read (0 if a read is
   failed)...  Then debugEndPacket with a boolean for if the packet
   was a good one or a bad one and an integer that's postive for the
   type of packet (if successful) or negative to express why the read
   failed (if not successful).  For the 'why the read failed' the
   values should be negative, and should all be unique (anywhere a
   read can return), preferably with a gap between the values, so that
   if more debugging is needed there are values in between the ones
   already there.  Generally this information isn't used or computed,
   unless the global member MvrDeviceConnection::debugShouldLog is
   called to turn it on.
**/
MVREXPORT MvrDeviceConnection::MvrDeviceConnection()
{
  if (!ourStrMapInited)
  {
    ourStrMapInited = true;
    buildStrMap();
  }

  myDCPortName = "Unknown port name";
  myDCPortType = "Unknown port type";
  myDCDeviceName = "Unknown device type";

  myDCDebugPacketStarted = false;
  myDCDebugBytesRead = 0;
  myDCDebugTimesRead = 0;
  myDCDebugNumGoodPackets = 0;
  myDCDebugNumBadPackets = 0;
}

MVREXPORT MvrDeviceConnection::~MvrDeviceConnection()
{
  close();
}


void MvrDeviceConnection::buildStrMap(void)
{
  ourStrMap[STATUS_NEVER_OPENED] = "never opened";
  ourStrMap[STATUS_OPEN] = "open";
  ourStrMap[STATUS_OPEN_FAILED] = "open failed";
  ourStrMap[STATUS_CLOSED_NORMALLY] = "closed";
  ourStrMap[STATUS_CLOSED_ERROR] = "closed on error";
}

MVREXPORT const char * MvrDeviceConnection::getStatusMessage(int messageNumber) const
{
  MvrStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}

MVREXPORT void MvrDeviceConnection::setPortName(const char *portName)
{
  if (portName != NULL)
    myDCPortName = portName;
  else
    myDCPortName = "Unknown port name";
}

MVREXPORT const char *MvrDeviceConnection::getPortName(void) const
{
  return myDCPortName.c_str();
}

MVREXPORT void MvrDeviceConnection::setPortType(const char *portType)
{
  if (portType != NULL)
    myDCPortType = portType;
  else
    myDCPortType = "Unknown port type";
}

MVREXPORT const char *MvrDeviceConnection::getPortType(void) const
{
  return myDCPortType.c_str();
}

MVREXPORT void MvrDeviceConnection::setDeviceName(const char *deviceName)
{
  if (deviceName != NULL)
    myDCDeviceName = deviceName;
  else
    myDCDeviceName = "Unknown device name";
}

MVREXPORT const char *MvrDeviceConnection::getDeviceName(void) const
{
  return myDCDeviceName.c_str();
}

MVREXPORT void MvrDeviceConnection::debugStartPacket(void)
{
  if (!ourDCDebugShouldLog)
    return;

  myDCDebugStartTime.setToNow();
  myDCDebugPacketStarted = true;
  myDCDebugBytesRead = 0;
  myDCDebugTimesRead = 0;
}

MVREXPORT void MvrDeviceConnection::debugBytesRead(int bytesRead)
{
  if (!ourDCDebugShouldLog || !myDCDebugPacketStarted)
    return;

  if (bytesRead > 0)
  {
    if (myDCDebugBytesRead == 0)
      myDCDebugFirstByteTime.setToNow();
    myDCDebugLastByteTime.setToNow();
    myDCDebugBytesRead += bytesRead;
  }

  myDCDebugTimesRead++;
}

MVREXPORT void MvrDeviceConnection::debugEndPacket(bool goodPacket, int type)
{
  if (!ourDCDebugShouldLog || !myDCDebugPacketStarted)
    return;

  if (myDCDebugBytesRead == 0)
  {
    myDCDebugFirstByteTime.setToNow();
    myDCDebugLastByteTime.setToNow();
  }
  
  if (goodPacket)
    myDCDebugNumGoodPackets++;
  else
    myDCDebugNumBadPackets++;

  long long firstSince = ourDCDebugFirstTime.mSecSinceLL(myDCDebugStartTime);

  MvrLog::log(MvrLog::Normal, 
	     "DevCon %s %s %s started %lld.%03lld firstByte %lld lastByte %lld bytesRead %d timesRead %d good %d numGood %lld numBad %lld type %d 0x%x",
	     myDCPortType.c_str(), myDCPortName.c_str(), myDCDeviceName.c_str(),
	     firstSince / 1000, firstSince % 1000,
	     myDCDebugStartTime.mSecSinceLL(myDCDebugFirstByteTime),
	     myDCDebugStartTime.mSecSinceLL(myDCDebugLastByteTime),
	     myDCDebugBytesRead, myDCDebugTimesRead, goodPacket,
	     myDCDebugNumGoodPackets, myDCDebugNumBadPackets, 
	     type, type);
  
  myDCDebugPacketStarted = false;
}

MVREXPORT bool MvrDeviceConnection::debugShouldLog(bool shouldLog)
{
  ourDCDebugShouldLog = shouldLog;
  return true;
}
