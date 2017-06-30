/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotPacketSender.cpp
 > Description  : Given a device connection which sends commands through it to the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrDeviceConnection.h"
#include "MvrRobotPacketSender.h"

/*
 * Use setDeviceConnection() to set the device connection before use.
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 */
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(unsigned char sync1, unsigned char sync2) :
          myPacket(sync1, sync2)
{
  myDeviceConn = NULL;
  myTracking   = false;
  myTrackingLogName.clear();
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
} 
                                                

/*
 * @param deviceConnection device connection to send packets to
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
*/
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(MvrDeviceConnection *deviceConnection, 
                                                     unsigned char sync1, unsigned char sync2) :
          myPacket(sync1, sync2)
{
  myDeviceConn = deviceConnection;
  myTracking   = false;
  myTrackingLogName.clear();
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
}          

/*
 * @param deviceConnection device connection to send packets to
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param tracking if true write packet-tracking log messages for each packet sent.
 * @param trackingLogName name (packet type) to include in packet-tracking log messages 
 */
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(MvrDeviceConnection *deviceConnection,
                                                     unsigned char sync1, unsigned char sync2,
                                                     bool tracking, const char *trackingLogName) :
          myPacket(sync1, sync2),
          myTracking(tracking),
          myTrackingLogName(trackingLogName)
{
  myDeviceConn = deviceConnection;
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
}                             

MVREXPORT MvrRobotPacketSender::~MvrRobotPacketSender()
{

}

MVREXPORT void MvrRobotPacketSender::setDeviceConnection(MvrDeviceConnection *deviceConnection)
{
  myDeviceConn = deviceConnection;
}

MVREXPORT MvrDeviceConnection *MvrRobotPacketSender::getDeviceConnection(void)
{
  return myDeviceConn;
}

bool MvrRobotPacketSender::connValid(void)
{
  return (myDeviceConn != NULL && myDeviceConn->getStatus() == MvrDeviceConnection::STATUS_OPEN);
}

/*
 *  @param command the command number to send
 *  @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::com(unsigned char command)
{
  if (!connValid())
    return false;
  bool ret;

  mySendingMutex.lock();
  
  myPacket.empty();
  myPacket.setID(command);

  myPacket.finalizePacket();

  if (myTracking)
    myPacket.log();
  
  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

/*
 * @param command the command number to send
 * @param argument the integer argument to send with the command
 * @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::comInt(unsigned char command, short int argument)
{
  if (!connValid())
    return false;
  
  bool ret = true;

  mySendingMutex.lock();
  
  myPacket.empty();
  myPacket.setID(command);

  if (argument >=0)
  {
    myPacket.uByteToBuf(INTARG);
  }
  else
  {
    myPacket.uByteToBuf(NINTARG);
    argument = -argument;
  }
  myPacket.uByte2ToBuf(argument);

  myPacket.finalizePacket();

  if (myTracking)
    myPacket.log();
  
  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);
  
  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

/*
 * @param command the command number to send
 * @param high the high byte to send with the command
 * @param low the low byte to send with the command
 * @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::com2Bytes(unsigned char command, char high, char low)
{
  return comInt(command, ((high & 0xff) << 8 ) + (low & 0xff));
}

/*
 * Sends a length-prefixed string command.
 * @param command the command number to send
 * @param argument NULL-terminated string to send with the command
 * @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::comStr(unsigned char command, const char *argument)
{
  size_t size;
  if (!connValid())
    return false;
  size = strlen(argument);
  if (size > 199)   /// 200-1 byte for length
    return false;
  
  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();

  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);
  myPacket.uByteToBuf((MvrTypes::UByte)size);
  myPacket.strToBuf(argument);

  myPacket.finalizePacket();
  
  if (myTracking)
    myPacket.log();
  
  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);
  
  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

/*
 * Sends a packet containing the given command, and a length-prefixed string 
 * containing the specified number of bytes copied from the given source string.
 * @param command the command number to send
 * @param str the character array containing data to send with the command
 * @param size number of bytes from the array to send; prefix the string with a byte containing this value as well. this size must be less than the maximum packet size of 200
 * @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::comStrN(unsigned char command, const char *str, int size)
{
  if (!connValid())
    return false;
  if (size > 199)   /// 200-1 byte for length
    return false;
  
  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();

  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);
  myPacket.uByteToBuf(size);
  myPacket.strNToBuf(str, size);

  myPacket.finalizePacket();
  
  if (myTracking)
    myPacket.log();
  
  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);
  
  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

/*
 * Sends an MvrRobotPacket
 * @param packet MvrRobotPacket
 * @return whether the command could be sent or not
 */
MVREXPORT bool MvrRobotPacketSender::sendPacket(MvrRobotPacket *packet)
{
  if (!connValid())
    return false;
  
  bool ret = true;

  mySendingMutex.lock();

  packet->finalizePacket();

  // if tracking is on - log packet - also make sure buffer length is in range
  if ((myTracking) && (packet->getLength() < 10000))
  {
    unsigned char *buf = (unsigned char *) packet->getBuf();

    char obuf[10000];
    obuf[0] = '\0';
    int j = 0;
    for (int i=0; i < packet->getLength(); i++)
    {
      sprintf (&obuf[j], "_%02x", buf[i]);
			j= j+3;
    }
    MvrLog::log(MvrLog::Normal,
                "Send Packet: %s packet = %s",
                myTrackingLogName.c_str(), obuf);
  }

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);
  
  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

MVREXPORT bool MvrRobotPacketSender::comDataN(unsigned char command, const char *data, int size)
{
  if (!connValid())
    return false;
  if (size > 200)
    return false;
  
  bool ret = true;
  
  mySendingMutex.lock();

  myPacket.empty();
  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);
  myPacket.strNToBuf(data, size);
  myPacket.finalizePacket();

  if (myTracking)
    myPacket.log();

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);
  
  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  return ret;
}

MVREXPORT void MvrRobotPacketSender::setPacketSentCallback(MvrFunctor1<MvrRobotPacket *> *functor)
{
  myPacketSentCallback = functor;
}