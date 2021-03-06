#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrDeviceConnection.h"
#include "MvrRobotPacketReceiver.h"
#include "MvrLogFileConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"


/**
   @param allocatePackets whether to allocate memory for the packets before
   returning them (true) or to just return a pointer to an internal 
   packet (false)... most everything should use false as this will help prevent
   many memory leaks or corruptions
   @param sync1 first byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
   @param sync2 second byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
*/
MVREXPORT MvrRobotPacketReceiver::MvrRobotPacketReceiver(bool allocatePackets,
						      unsigned char sync1,
						      unsigned char sync2) : 
  myPacket(sync1, sync2)
{
  myAllocatePackets = allocatePackets;
	myTracking = false;
	myTrackingLogName.clear();
  myDeviceConn = NULL;
  mySync1 = sync1;
  mySync2 = sync2;
  myPacketReceivedCallback = NULL;
}

/**
   @param deviceConnection the connection which the receiver will use
   @param allocatePackets whether to allocate memory for the packets before
   returning them (true) or to just return a pointer to an internal 
   packet (false)... most everything should use false as this will help prevent
   many memory leaks or corruptions
   @param sync1 first byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
   @param sync2 second byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
*/
MVREXPORT MvrRobotPacketReceiver::MvrRobotPacketReceiver(
	MvrDeviceConnection *deviceConnection, bool allocatePackets,
	unsigned char sync1, unsigned char sync2) :
  myPacket(sync1, sync2)
{
  myDeviceConn = deviceConnection;
	myTracking = false;
	myTrackingLogName.clear();
  myAllocatePackets = allocatePackets;
  mySync1 = sync1;
  mySync2 = sync2;
  myPacketReceivedCallback = NULL;
}

/**
   @param deviceConnection the connection which the receiver will use
   @param allocatePackets whether to allocate memory for the packets before
   returning them (true) or to just return a pointer to an internal 
   packet (false)... most everything should use false as this will help prevent
   many memory leaks or corruptions
   @param sync1 first byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
   @param sync2 second byte of the header this receiver will receive, this 
   should be left as the default in nearly all cases, ie don't mess with it
   @param tracking if true write log messages for packets received
   @param trackingLogName name to include for packets with tracking log messages
*/
MVREXPORT MvrRobotPacketReceiver::MvrRobotPacketReceiver(
	MvrDeviceConnection *deviceConnection, bool allocatePackets,
	unsigned char sync1, unsigned char sync2, bool tracking,
	const char *trackingLogName) :
  myPacket(sync1, sync2),
	myTracking(tracking),
	myTrackingLogName(trackingLogName)
{
  myDeviceConn = deviceConnection;
  myAllocatePackets = allocatePackets;
  mySync1 = sync1;
  mySync2 = sync2;
  myPacketReceivedCallback = NULL;
}

MVREXPORT MvrRobotPacketReceiver::~MvrRobotPacketReceiver() 
{
  
}

MVREXPORT void MvrRobotPacketReceiver::setDeviceConnection(
	MvrDeviceConnection *deviceConnection)
{
  myDeviceConn = deviceConnection;
}

MVREXPORT MvrDeviceConnection *MvrRobotPacketReceiver::getDeviceConnection(void)
{
  return myDeviceConn;
}

/**
    @param msWait how long to block for the start of a packet, nonblocking if 0
    @return NULL if there are no packets in alloted time, the device connection is closed, or other error. Otherwise a pointer
    to the packet received is returned. If allocatePackets is true than the caller 
    must delete the packet. If allocatePackets is false then the packet object
    will be reused in the next call; the caller must not store or use that packet object.
 */
MVREXPORT MvrRobotPacket* MvrRobotPacketReceiver::receivePacket(unsigned int msWait)
{
  MvrRobotPacket *packet;
  unsigned char c;
  char buf[256];
  int count = 0;
  // state can be one of the STATE_ enums in the class
  int state = STATE_SYNC1;
  //unsigned int timeDone;
  //unsigned int curTime;
  long timeToRunFor;
  MvrTime timeDone;
  MvrTime lastDataRead;
  MvrTime packetReceived;
  int numRead;
  if (myAllocatePackets)
    packet = new MvrRobotPacket(mySync1, mySync2);
  else
    packet = &myPacket;

  if (packet == NULL || myDeviceConn == NULL || 
      myDeviceConn->getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    if (myTracking)
      MvrLog::log(MvrLog::Normal, "%s: receivePacket: connection not open", myTrackingLogName.c_str());
    myDeviceConn->debugEndPacket(false, -10);
    if (myAllocatePackets)
      delete packet;
    return NULL;
  }
  
  timeDone.setToNow();
  if (!timeDone.addMSec(msWait)) 
  {
    MvrLog::log(MvrLog::Normal,
               "MvrRobotPacketReceiver::receivePacket() error adding msecs (%i)",
               msWait);
  }

  // check for log file connection, return assembled packet
  if (dynamic_cast<MvrLogFileConnection *>(myDeviceConn))
  {
    packet->empty();
    packet->setLength(0);
    packetReceived = myDeviceConn->getTimeRead(0);
    packet->setTimeReceived(packetReceived);
    numRead = myDeviceConn->read(buf, 255, 0);
    if (numRead > 0)
    {
      packet->dataToBuf(buf, numRead);
      packet->resetRead();
      myDeviceConn->debugEndPacket(true, packet->getID());
      if (myPacketReceivedCallback != NULL)
				myPacketReceivedCallback->invoke(packet);

			// if tracking is on - log packet - also make sure
			// buffer length is in range
			
			if ((myTracking) && (packet->getLength() < 10000)) 
      {
  
				unsigned char *buf = (unsigned char *) packet->getBuf();

				char obuf[10000];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < packet->getLength(); i++) {
					sprintf (&obuf[j], "_%02x", buf[i]);
					j +=3;
				}

				MvrLog::log (MvrLog::Normal,
				            "Recv Packet: %s packet = %s", 
										myTrackingLogName.c_str(), obuf);


			}  // end tracking		

      return packet;
    }
    else
    {
      myDeviceConn->debugEndPacket(false, -20);
      if (myAllocatePackets)
	      delete packet;
     
      return NULL;
    }
  } // end if log file connection      
  

  do
    {
      timeToRunFor = timeDone.mSecTo();
      if (timeToRunFor < 0)
        timeToRunFor = 0;

      if (state == STATE_SYNC1)
	      myDeviceConn->debugStartPacket();

      if (myDeviceConn->read((char *)&c, 1, timeToRunFor) == 0) 
      {
        myDeviceConn->debugBytesRead(0);
          if (state == STATE_SYNC1)
            {
              myDeviceConn->debugEndPacket(false, -30);
              if (myAllocatePackets)
                delete packet; 
              if (myTracking)
                MvrLog::log(MvrLog::Normal, "%s: waiting for sync1 (got 0x%x).", myTrackingLogName.c_str(), c);
              return NULL;
            }
          else
            {
              //MvrUtil::sleep(1);
              continue;
            }
        }

      myDeviceConn->debugBytesRead(1);

      switch (state) {
      case STATE_SYNC1:


        if (c == mySync1) // move on, resetting packet
          {
            if (myTracking)
              MvrLog::log(MvrLog::Normal, "%s: got sync1 0x%x", myTrackingLogName.c_str(), c);
            state = STATE_SYNC2;
            packet->empty();
            packet->setLength(0);
            packet->uByteToBuf(c);
            packetReceived = myDeviceConn->getTimeRead(0);
            packet->setTimeReceived(packetReceived);
          }
        else
	      {
          if(myTracking) MvrLog::log(MvrLog::Normal, "%s: Not sync1 0x%x (expected 0x%x)\n", myTrackingLogName.c_str(), c, mySync1);
	      }
        break;
      case STATE_SYNC2:
 
       if (c == mySync2) // move on, adding this byte
          {
            if (myTracking)
              MvrLog::log(MvrLog::Normal, "%s: got sync2 0x%x", myTrackingLogName.c_str(), c);

            state = STATE_ACQUIRE_DATA;
            packet->uByteToBuf(c);
          }
        else // go back to beginning, packet hosed
          {
	          if(myTracking) MvrLog::log(MvrLog::Normal, "%s: Bad sync2 0x%x (expected 0x%x)\n", myTrackingLogName.c_str(), c, mySync2);
            state = STATE_SYNC1;
          }
        break;
      case STATE_ACQUIRE_DATA:

        // the character c is the count of the packets remianing at this point
        // so we'll just put it into the packet then get the rest of the data
        packet->uByteToBuf(c);
        // if c > 200 than there is a problem, spec says packet max size is 200
        count = 0;
	/** this case can't happen since c can't be over that so taking it out
        if (c > 255) 
          {
            MvrLog::log(MvrLog::Normal, "MvrRobotPacketReceiver::receivePacket: bad packet, more than 255 bytes");
            state = STATE_SYNC1;
            break;
          }
	*/
        // here we read until we get as much as we want, OR until
        // we go 100 ms without data... its arbitrary but it doesn't happen often
        // and it'll mean a bad packet anyways
        lastDataRead.setToNow();
        while (count < c)
          {
            numRead = myDeviceConn->read(buf + count, c - count, 1);
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
              myDeviceConn->debugEndPacket(false, -40);
              if (myAllocatePackets)
                delete packet;
              //printf("Bad time taken reading\n");
              return NULL;
            }
            count += numRead;
          }
        packet->dataToBuf(buf, c);
        if (packet->verifyCheckSum()) 
        {
	
          packet->resetRead();
	    /* put this in if you want to see the packets received
	       printf("Input ");
               packet->printHex();
	       */

	    // you can also do this next line if you only care about type
	    //printf("Input %x\n", packet->getID());
	    myDeviceConn->debugEndPacket(true, packet->getID());
	    if (myPacketReceivedCallback != NULL)
	      myPacketReceivedCallback->invoke(packet);

			// if tracking is on - log packet - also make sure
			// buffer length is in range

			if ((myTracking) && (packet->getLength() < 10000)) {
				unsigned char *buf = (unsigned char *) packet->getBuf();
		
				char obuf[10000];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < packet->getLength(); i++) {
					sprintf (&obuf[j], "_%02x", buf[i]);
					j= j+3;
				}

				MvrLog::log (MvrLog::Normal,
				            "Recv Packet: %s packet = %s", 
										myTrackingLogName.c_str(), obuf);


			}  // end tracking		

			return packet;
          }
        else 
          {
	    /* put this in if you want to see bad checksum packets 
               printf("Bad Input ");
               packet->printHex();
	       */
            MvrLog::log(MvrLog::Normal, 
                       "MvrRobotPacketReceiver::receivePacket: bad packet, bad checksum");
            state = STATE_SYNC1;
	    myDeviceConn->debugEndPacket(false, -50);
            break;
          }
        break;
      default:
        break;
      }
    } while (timeDone.mSecTo() >= 0 || state != STATE_SYNC1);

  myDeviceConn->debugEndPacket(false, -60);
  //printf("finished the loop...\n"); 
  if (myAllocatePackets)
    delete packet;
  return NULL;

}

MVREXPORT void MvrRobotPacketReceiver::setPacketReceivedCallback(
	MvrFunctor1<MvrRobotPacket *> *functor)
{
  myPacketReceivedCallback = functor;
}
