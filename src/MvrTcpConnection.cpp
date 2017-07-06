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
#include "MvrTcpConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"

MVREXPORT MvrTcpConnection::MvrTcpConnection()
{
  myStatus = STATUS_NEVER_OPENED;
  buildStrMap();
  myOwnSocket = true;
  mySocket = new MvrSocket();
  setPortType("tcp");
}

MVREXPORT MvrTcpConnection::~MvrTcpConnection()
{
  if (myOwnSocket)
    delete mySocket;
}

/**
   This will make the connection use this socket, its useful for doing
   funkier things with sockets but still being able to use a device
   connection.
   @param socket the socket to use
**/
MVREXPORT void MvrTcpConnection::setSocket(MvrSocket *socket)
{
  if (myOwnSocket)
  {
    delete mySocket;
    myOwnSocket = false;
  }
  mySocket = socket;
}

MVREXPORT MvrSocket *MvrTcpConnection::getSocket(void)
{
  return mySocket;
}

MVREXPORT void MvrTcpConnection::setStatus(int status)
{
  myStatus = status;
}

MVREXPORT void MvrTcpConnection::setPort(const char *host, int port)
{
  myPortNum = port;

  if (host == NULL)
    myHostName = "localhost";
  else
    myHostName = host;

  char portBuf[1024];
  sprintf(portBuf, "%d", port); 
  
  std::string portName;
  portName = myHostName;
  portName += ":";
  portName += portBuf;
  setPortName(portName.c_str());
}

MVREXPORT bool MvrTcpConnection::openSimple(void)
{
  if (internalOpen() == 0)
    return true;
  else
    return false;
}

/**
   @param host the hostname or IP address to connect to, if NULL (default) then localhost
   @param port the port to connect to. (the default port, 8101, is the TCP port
number used by the wireless bridge device and by the simulator for robot
connection)
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
MVREXPORT int MvrTcpConnection::open(const char *host, int port)
{
  setPort(host, port);
  return internalOpen();
}

MVREXPORT int MvrTcpConnection::internalOpen(void)
{
  mySocket->init();

  MvrLog::log(MvrLog::Verbose, "MvrTcpConnection::internalOpen: Connecting to %s %d", myHostName.c_str(), myPortNum);

  if (mySocket->connect(const_cast<char *>(myHostName.c_str()), myPortNum,
		       MvrSocket::TCP)) 
  {
    myStatus = STATUS_OPEN;
    mySocket->setNonBlock();
    mySocket->setNoDelay(true);
    return 0;
  }
  
  myStatus = STATUS_OPEN_FAILED;
  switch(mySocket->getError())
  {
  case MvrSocket::NetFail:
    return OPEN_NET_FAIL;
  case MvrSocket::ConBadHost:
    return OPEN_BAD_HOST;
  case MvrSocket::ConNoRoute:
    return OPEN_NO_ROUTE;
  case MvrSocket::ConRefused:
    return OPEN_CON_REFUSED;
  case MvrSocket::NoErr:
    MvrLog::log(MvrLog::Terse, "MvrTcpConnection::open: No error!\n");
  default:
    return -1;
  }

}

void MvrTcpConnection::buildStrMap(void)
{
  myStrMap[OPEN_NET_FAIL] = "Network failed.";
  myStrMap[OPEN_BAD_HOST] = "Could not find host.";
  myStrMap[OPEN_NO_ROUTE] = "No route to host.";
  myStrMap[OPEN_CON_REFUSED] = "Connection refused.";
}

MVREXPORT const char *MvrTcpConnection::getOpenMessage(int messageNumber)
{
  return myStrMap[messageNumber].c_str();
}

MVREXPORT bool MvrTcpConnection::close(void)
{
  myStatus = STATUS_CLOSED_NORMALLY;
  return mySocket->close();
}

MVREXPORT int MvrTcpConnection::read(const char *data, unsigned int size, 
				   unsigned int msWait)
{
  MvrTime timeDone;
  unsigned int bytesRead = 0;
  int n;

  if (getStatus() != STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrTcpConnection::read: Attempt to use port that is not open.");
    return -1;
  }


  int timeToWait;  
  timeDone.setToNow();
  if (!timeDone.addMSec(msWait)) {
    MvrLog::log(MvrLog::Normal,
               "MvrTcpConnection::read() error adding msecs (%i)",
               msWait);
  }

  do 
  {
    timeToWait = timeDone.mSecTo();
    if (timeToWait < 0)
      timeToWait = 0;
    // if the sockets empty don't read it, but pause some
    if (mySocket->getFD() < 0)
    {
      MvrLog::log(MvrLog::Terse, 
		 "MvrTcpConnection::read: Attempt to read port that already closed. (%d)", timeToWait);
      if (timeToWait > 0)
	MvrUtil::sleep(timeToWait);
      return -1;
    }

    n = mySocket->read(const_cast<char *>(data) + bytesRead, size - bytesRead,
		       timeToWait);
    /*if (n == -1) 
    {
      MvrLog::log("MvrTcpConnection::read: read failed.");
      return -1;
      } */
    //printf("%ld %d %d\n", timeDone.mSecTo(), n, size);
    if (n != -1)
      bytesRead += n;
    if (bytesRead >= size)
      return bytesRead;
  } while (timeDone.mSecTo() >= 0);

  return bytesRead;
}

MVREXPORT int MvrTcpConnection::write(const char *data, unsigned int size)
{
  int ret;

  if (getStatus() != STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrTcpConnection::write: Attempt to use port that is not open.");
    return -1;
  }
  if ((ret = mySocket->write(data, size)) != -1)
    return ret;

  MvrLog::log(MvrLog::Terse, "MvrTcpConnection::write: Write failed, closing connection.");
  close();
  return -1;
}


/** 
    @return the name of the host connected to
    @see getPort
*/
MVREXPORT std::string MvrTcpConnection::getHost(void)
{
  return myHostName;
}

/**
   @return the number of the port connected to
   @see getHost
*/
MVREXPORT int MvrTcpConnection::getPort(void)
{
  return myPortNum;
}

MVREXPORT int MvrTcpConnection::getStatus(void)
{
  return myStatus;
}

MVREXPORT bool MvrTcpConnection::isTimeStamping(void)
{
  return false;
}

MVREXPORT MvrTime MvrTcpConnection::getTimeRead(int index)
{
  MvrTime now;
  now.setToNow();
  return now;
}
