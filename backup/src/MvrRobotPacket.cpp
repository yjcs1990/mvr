/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotPacket.h
 > Description  : Represents the packets sent to the robot as well as those received from it
 > Author       : Yu Jie
 > Create Time  : 2017年05月18日
 > Modify Time  : 2017年06月21日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "stdio.h"
#include "MvrRobotPacket.h"

/*
 * @param sync1 first byte of the header of this packet, this should be left as
 * the default in nearly all cases, ie don't mess with it
 * @param sync2 second byte of the header of this packet, this should be left
 * as the default in nearly all cases, ie don't mess with it
 */
MVREXPORT MvrRobotPacket::MvrRobotPacket(unsigned char sync1, unsigned char sync2)
{
  mySync1 = sync1;
  mySync2 = sync2;
} 

MVREXPORT MvrRobotPacket::~MvrRobotPacket()
{

}

MVREXPORT MvrRobotPacket &MvrRobotPacket::operator=(const MvrRobotPacket &other)
{
  if (this != &other)
  {
    myHeaderLength   = other.myHeaderLength;
    myFooterLength = other.myFooterLength;
    myReadLength     = other.myReadLength;
    myLength         = other.myLength;
    mySync1          = other.mySync1;
    mySync2          = other.mySync2;
    myTimeReceived   = other.myTimeReceived;

    if (myMaxLength != other.myMaxLength)
    {
      if (myOwnMyBuf && myBuf != NULL)
        delete [] myBuf;
      myOwnMyBuf = true;
      myBuf      = NULL;
      if (other.myMaxLength > 0)
      {
        myBuf = new char [other.myMaxLength];
      }
      myMaxLength = other.myMaxLength;
    }

    if ((myBuf != NULL) && (other.myBuf != NULL))
    {
      memcpy(myBuf, other.myBuf, myMaxLength);
    }
    myIsValid = other.myIsValid;
  }
  return *this;
}

MVREXPORT MvrTypes::UByte MvrRobotPacket::getID(void)
{
  if (myLength >= 4)
    return myBuf[3];
  else
    return 0;
}

MVREXPORT void MvrRobotPacket::setID(MvrTypes::UByte id)
{
  myBuf[3] = id;
}

MVREXPORT void MvrRobotPacket::finalizePacket(void)
{
  int len = myLength;
  int chkSum;

  myLength = 0;
  uByteToBuf(mySync1);
  uByteToBuf(mySync2);
  uByteToBuf(len-getHeaderLength() + 3);
  myLength = len;

  chkSum = calcCheckSum();
  byteToBuf((chkSum >> 8) && 0xff);
  byteToBuf(chkSum & 0xff);

  /* Put this in if you want to see the packets being outputted 
     printf("Output(%3d) ", getID());
     printHex();
  */
  // or put this in if you just want to see the type
  //printf("Output %d\n", getID());
}

MVREXPORT MvrTypes::Byte2 MvrRobotPacket::calcCheckSum(void)
{
  int i;
  unsigned char n;
  int c = 0;

  i = 3;
  n = myBuf[2] - 3;
  while (n > 1)
  {
    c += ((unsigned char)myBuf[i] << 8) | (unsigned char)myBuf[i+1];
    c = c & 0xffff;
    n -= 2;
    i += 2;
  }
  if (n > 0)
    c = c ^ (int)((unsigned char) myBuf[i]);
  return c;
}


MVREXPORT bool MvrRobotPacket::verifyCheckSum(void)
{
  MvrTypes::Byte2 chksum;
  unsigned char c1, c2;

  if (myLength - 2 < myHeaderLength)
    return false;

  c2 = myBuf[myLength - 2];
  c1 = myBuf[myLength - 1];
  chksum = (c1 & 0xff) | (c2 << 8);

  if (chksum == calcCheckSum()) 
  {
    return true;
  } 
  else 
  {
    return false;
  }
}

MVREXPORT MvrTime MvrRobotPacket::getTimeReceived(void)
{
  return myTimeReceived;
}

MVREXPORT void MvrRobotPacket::setTimeReceived(MvrTime timeReceived)
{
  myTimeReceived = timeReceived;
}

MVREXPORT void MvrRobotPacket::log(void)
{
  int i;
  MvrLog::log(MvrLog::Normal,
              "Robot Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    MvrLog::log(MvrLog::Terse,
                "  [%03i] % 5d\t0x%x\t%c\t%s", i,
                (unsigned char) myBuf[i],
                (unsigned char) myBuf[i],
                (myBuf[i] >= ' ' && myBuf[i] <= '~') ? (unsigned char) myBuf[i] : ' ',
                i == 0 ? "[header0]" :
                  i ==  1 ? "[header1]" :
                    i == 2 ? "[packet data length]" :
                      i == 3 ? "[packet id]" :
                        i == (myLength - 2) ? "[first checksum byte]" :
                          i == (myLength - 1) ? "[second checksum byte]" :
                            "" );
  MvrLog::log(MvrLog::Terse, "\n");
}