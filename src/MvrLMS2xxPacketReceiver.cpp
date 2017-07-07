#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLMS2xxPacketReceiver.h"
#include "MvrLog.h"
#include "mvriaUtil.h"


/*
   allocatePackets: whether to allocate memory for the packets before
   returning them (true) or to just return a pointer to an internal 
   packet (false)... most everything should use false as this will help prevent
   many memory leaks or corruptions
*/
MVREXPORT MvrLMS2xxPacketReceiver::MvrLMS2xxPacketReceiver(
	unsigned char receivingAddress, bool allocatePackets,
	bool useBase0Address) 
{
  myAllocatePackets = allocatePackets;
  myReceivingAddress = receivingAddress;
  myDeviceConn = NULL;
  myUseBase0Address = useBase0Address;
}

/*
   allocatePackets: whether to allocate memory for the packets before
   returning them (true) or to just return a pointer to an internal 
   packet (false)... most everything should use false as this will help prevent
   many memory leaks or corruptions
*/
MVREXPORT MvrLMS2xxPacketReceiver::MvrLMS2xxPacketReceiver(
	MvrDeviceConnection *deviceConnection, 
	unsigned char receivingAddress, bool allocatePackets,
	bool useBase0Address)
{
  myDeviceConn = deviceConnection;
  myAllocatePackets = allocatePackets;
  myReceivingAddress = receivingAddress;
  myUseBase0Address = useBase0Address;
}

MVREXPORT MvrLMS2xxPacketReceiver::~MvrLMS2xxPacketReceiver() 
{
  
}

MVREXPORT void MvrLMS2xxPacketReceiver::setDeviceConnection(
	MvrDeviceConnection *deviceConnection)
{
  myDeviceConn = deviceConnection;
}

MVREXPORT MvrDeviceConnection *MvrLMS2xxPacketReceiver::getDeviceConnection(void)
{
  return myDeviceConn;
}

/**
    @param msWait how long to block for the start of a packet, nonblocking if 0
    @return NULL if there are no packets in alloted time, otherwise a pointer
    to the packet received, if allocatePackets is true than the place that 
    called this function owns the packet and should delete the packet when 
    done... if allocatePackets is false then nothing must store a pointer to
    this packet, the packet must be used and done with by the time this 
    method is called again
*/
MVREXPORT MvrLMS2xxPacket *MvrLMS2xxPacketReceiver::receivePacket(
	unsigned int msWait)
{
  MvrLMS2xxPacket *packet;
  unsigned char c;
  char buf[2048];
  long count = 0;
  // state can be one of the STATE_ enums in the class
  int state = STATE_START;
  //unsigned int timeDone;
  //unsigned int curTime;
  long timeToRunFor;
  long packetLength;
  MvrTime timeDone;
  MvrTime lastDataRead;
  MvrTime packetReceived;
  int numRead;


  if (myDeviceConn == NULL || 
      myDeviceConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    myDeviceConn->debugEndPacket(false, -10);
    return NULL;
  }
  
  timeDone.setToNow();
  if (!timeDone.addMSec(msWait)) {
    MvrLog::log(MvrLog::Normal,
               "MvrLMS2xxPacketReceiver::receivePacket() error adding msecs (%i)",
               msWait);
  }
  do
  {
    timeToRunFor = timeDone.mSecTo();
    if (timeToRunFor < 0)
      timeToRunFor = 0;

    if (state == STATE_START)
      myDeviceConn->debugStartPacket();

    if (myDeviceConn->read((char *)&c, 1, timeToRunFor) == 0) 
    {
      myDeviceConn->debugBytesRead(0);
      if (state == STATE_START)
      {
	myDeviceConn->debugEndPacket(false, -20);
	return NULL;
      }
      else
      {
	//MvrUtil::sleep(1);
	continue;
      }
    }

    myDeviceConn->debugBytesRead(1);

    //printf("%x\n", c);
    switch (state) {
    case STATE_START:
      if (c == 0x02) // move on, resetting packet
      {
	//printf("###############\n");
	state = STATE_ADDR;
	myPacket.empty();
	myPacket.setLength(0);
	myPacket.uByteToBuf(c);
	packetReceived = myDeviceConn->getTimeRead(0);
	myPacket.setTimeReceived(packetReceived);
      }
      /*else
      {
	//printf(" BAD\n");
	}*/
      break;
    case STATE_ADDR:
      // if this is correct move on, adding this byte... this is taken
      // out in favor of a more inclusive approach, if someone ever
      // wnats to drive multiple robots off of one serial port just
      // put this back in, or I don't know, punt
      //if (c == ((unsigned char)0x80 + myReceivingAddress)) 
      if (!myUseBase0Address && c >= 0x80 && c <= 0x84)
      {
	state = STATE_START_COUNT;
	myPacket.uByteToBuf(c);
      }
      // c will always be >= 0 since its unsigned
      else if (myUseBase0Address && c <= 0x4)
      {
	state = STATE_START_COUNT;
	myPacket.uByteToBuf(c);
      }
      else // go back to beginning, packet hosed
      {
	MvrLog::log(MvrLog::Terse, 
		   "MvrLMS2xxPacketReceiver::receivePacket: wrong address (0x%x instead of 0x%x)", c, (unsigned) 0x80 + myReceivingAddress);
	state = STATE_START;
      }
      break;
    case STATE_START_COUNT:
      packetLength = c;
      myPacket.uByteToBuf(c);
      state = STATE_ACQUIRE_DATA;
      break;
    case STATE_ACQUIRE_DATA:
      // the character c is high ordre byte of the packet length count 
      // so we'll just build the length of the packet then get the 
      //rest of the data
      myPacket.uByteToBuf(c);
      packetLength = packetLength | (c << 8);
      count = 0;
      // make sure the length isn't longer than the maximum packet length...
      // getting some wierd 25k or 44k long packets (um, no)
      if (packetLength > ((long)myPacket.getMaxLength() -
			  (long)myPacket.getHeaderLength()))
      {
	MvrLog::log(MvrLog::Normal, 
	   "MvrLMS2xxPacketReceiver::receivePacket: packet too long, it is %d long while the maximum is %d.", packetLength, myPacket.getMaxLength());
	state = STATE_START;
	//myPacket.log();
	break;
      }
      // here we read until we get as much as we want, OR until
      // we go 100 ms without data... its arbitrary but it doesn't happen often
      // and it'll mean a bad packet anyways
      lastDataRead.setToNow();
      while (count < packetLength + 2)
      {
	numRead = myDeviceConn->read(buf + count, packetLength + 2- count, 1);
	if (numRead > 0)
	{	
	  myDeviceConn->debugBytesRead(numRead);
	  lastDataRead.setToNow();
	}
	else
	{
	  myDeviceConn->debugBytesRead(0);
	}
	if (lastDataRead.mSecTo() < -100)
	{
	  myDeviceConn->debugEndPacket(false, -30);
	  return NULL;
	}
	count += numRead;
      }
      myPacket.dataToBuf(buf, packetLength + 2);
      if (myPacket.verifyCRC()) 
      {
	myPacket.resetRead();
	myDeviceConn->debugEndPacket(true, myPacket.getID());
	//printf("Received ");
	//myPacket.log();
	if (myAllocatePackets)
	{
	  packet = new MvrLMS2xxPacket;
	  packet->duplicatePacket(&myPacket);
	  return packet;
	}
	else
	  return &myPacket;
      }
      else 
      {
	MvrLog::log(MvrLog::Normal, 
	   "MvrLMS2xxPacketReceiver::receivePacket: bad packet, bad checksum");
	state = STATE_START;
	//myPacket.log();
	break;
      }
      break;
    default:
      break;
    }
  } while (timeDone.mSecTo() >= 0 || state != STATE_START);

  myDeviceConn->debugEndPacket(false, -40);
  //printf("finished the loop...\n");
  return NULL;

}


