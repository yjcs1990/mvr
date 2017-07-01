#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSocket.h"
#include "MvrLog.h"
#include <stdio.h>
#include <string.h>
#include "MvrFunctor.h"

/*
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
*/

bool MvrSocket::ourInitialized=false;


MVREXPORT MvrSocket::MvrSocket() :
  myType(Unknown),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
}

MVREXPORT MvrSocket::MvrSocket(const char *host, int port, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  connect(host, port, type);
}

MVREXPORT MvrSocket::MvrSocket(int port, bool doClose, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(doClose),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  open(port, type);
}

MVREXPORT MvrSocket::~MvrSocket()
{
  close();
}

/** @return false failure. */
MVREXPORT bool MvrSocket::init()
{
  WORD wVersionRequested;
  WSADATA wsaData;

//  if (!ourInitialized)
  //{
  wVersionRequested=MAKEWORD( 2, 2 );
  
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    ourInitialized=false;
    return(false);
  }
  
  ourInitialized=true;
  //}

  return(true);
}

MVREXPORT void MvrSocket::shutdown()
{
  if (ourInitialized)
  {
    MvrLog::log(MvrLog::Verbose, "MvrSocket::shutdown calling WSACleanup");
    WSACleanup();
    ourInitialized=false;
  }
}

/** @return false on failure */
MVREXPORT bool MvrSocket::hostAddr(const char *host, struct in_addr &addr)
{
  struct hostent *hp;

  if (!(hp=gethostbyname(host)))
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::hostAddr: gethostbyname failed");
    memset(&addr, 0, sizeof(in_addr));
    return(false);
  }
  else
  {
    memcpy(&addr, hp->h_addr, hp->h_length);
    return(true);
  }
}

/** @return false on failure */
MVREXPORT bool MvrSocket::addrHost(struct in_addr &addr, char *host)
{
  struct hostent *hp;

  hp=gethostbyaddr((char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET);
  if (hp)
    strcpy(host, hp->h_name);
  else
    strcpy(host, inet_ntoa(addr));

  return(true);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::connect(const char *host, int port, Type type,
				const char *openOnIP)
{
  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  init();

  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myError=ConBadHost;
      myErrorStr="Failure to locate host '";
      myErrorStr+=localhost;
      myErrorStr+="'";
      MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: gethostname failed");
      return(false);
    }
    host=localhost;
  }

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  memset(&mySin, 0, sizeof(mySin));
  if ((mySin.sin_addr.s_addr = inet_addr(useHost)) == INADDR_NONE)
  {
    if (!hostAddr(host, mySin.sin_addr))
    {
      setRawIPString();
      myError = ConBadHost;
      myErrorStr = "Could not find the address of '";
      myErrorStr += host;
      myErrorStr += "'";
      return(false);
    }
  }

  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  // WSA_FLAG_OVERLAPPED allows concurrent calls to select, read and send on the same socket,
  // which could happen occasionally. If OVERLAPPED is not enabled in this situation, calls can
  // hang mysteriously.
  // This flag is also required for all non-blocking sockets on Windows NT 4.0 (according to MS
  // Knowlege Base article Q179942)
  if ((type == TCP) && ((myFD=WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make TCP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: could not make tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make UDP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: could not make udp socket");
    return(0);
  }

  myType = type;
  myHost = ((host != NULL) ? host : "");
  myPort = port;

  if (::connect(myFD, (struct sockaddr *)&mySin,
		sizeof(struct sockaddr_in)) < 0)
  {
    char buff[10];
    int err=WSAGetLastError();
    sprintf(buff, "%d", err);
    myErrorStr="Failure to connect socket";
    myErrorStr+=buff;
    switch (err)
    {
    case WSAEADDRNOTAVAIL:
      myError=ConBadHost;
      break;
    case WSAECONNREFUSED:
      myError=ConRefused;
      break;
    case WSAENETUNREACH:
      myError=ConNoRoute;
      break;
    default:
      myError=NetFail;
      break;
    }
    MvrLog::logErrorFromOS(MvrLog::Verbose, 
			  "MvrSocket::connect: Failure to connect");
    ::shutdown(myFD, SD_BOTH);
    closesocket(myFD);
    myFD = INVALID_SOCKET;
    return(0);
  }

  myLastStringReadTime.setToNow();
  return(1);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::open(int port, Type type, const char *openOnIP)
{
  int ret;
  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET))
  {
    //ret=WSAGetLastError();
    myError = NetFail;
    myErrorStr="Failure to make TCP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not create tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET))
  {
    myError = NetFail;
    myErrorStr="Failure to make UDP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not create udp socket");
    return(false);
  }

  setLinger(0);
  setReuseAddress();

  /* MPL this is useless withw hat I Took out below
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: gethostname failed");
    return(false);
  }
  */

  memset(&mySin, 0, sizeof(mySin));
  /* MPL took this out since it was just overriding it with the
     INADDR_ANY anyways and it could cause slowdowns if a machine wasn't
     configured so lookups are quick
  if (!hostAddr(localhost, mySin.sin_addr))
    return(false);
  */
  setRawIPString();
  if (openOnIP != NULL)
  {
    
    if (!hostAddr(openOnIP, mySin.sin_addr))
    {
      myError = NameLookup;
      myErrorStr = "Name lookup failed";
      MvrLog::log(MvrLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
      ::shutdown(myFD, SD_BOTH);
      myFD = INVALID_SOCKET;
      return(false); 
    }
    else
    {
      //printf("Opening on %s\n", openOnIP);
    }
  }
  else
  {
    mySin.sin_addr.s_addr=htonl(INADDR_ANY);
  }
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(port);

  myType=type;

  if ((ret=bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin))) < 0)
  {
    myErrorStr="Failure to bind socket to port ";
    sprintf(localhost, "%d", port);
    myErrorStr+=localhost;
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not bind");
    ::shutdown(myFD, SD_BOTH);
    myFD = INVALID_SOCKET;
    return(false);
  }

  if ((type == TCP) && (listen(myFD, 5) < 0))
  {
    myErrorStr="Failure to listen on socket";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not listen");
    ::shutdown(myFD, SD_BOTH);
    myFD = INVALID_SOCKET;
    return(false);
  }

  return(true);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::create(Type type)
{
  myError = NoErr;
  myErrorStr.clear();
  //if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET))
  if ((type == TCP) && ((myFD=WSASocket(AF_INET, SOCK_STREAM, 0,  NULL, 0, 0)) == INVALID_SOCKET))
  {
    myErrorStr="Failure to make TCP socket";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: could not create tcp socket");
    return(false);
  }
  //else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET))
  else if ((type == UDP) && ((myFD=WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0)) == INVALID_SOCKET))
  {
    myErrorStr="Failure to make UDP socket";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: could not create udp socket");
    return(false);
  }

  // KMC Reinserted
  myType = type;

                          

/*
  int zero = 0;
  if (setsockopt(myFD, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero)) != 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: setsockopt failed on SNDBUF");
    return(false);
  }

  if (setsockopt(myFD, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero)) != 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: setsockopt failed on RCVVBUF");
    return(false);
  }

  myType=type;
*/
  /*if (getSockName())
    return(true);
  else
    return(false);*/
  return(true);
}

/** @return false on failure */
MVREXPORT bool MvrSocket::findValidPort(int startPort, const char *openOnIP)
{

  /*
  char localhost[MAXGETHOSTSTRUCT];
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::findValidPort: gethostname failed");
    return(false);
  }
  */
  for (int i=0; i+startPort < 65000; ++i)
  {
    memset(&mySin, 0, sizeof(mySin));
    /*
    if (!hostAddr(localhost, mySin.sin_addr))
      return(false);
    */
    setRawIPString();
    if (openOnIP != NULL)
    {
      if (!hostAddr(openOnIP, mySin.sin_addr))
      {
        MvrLog::log(MvrLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
        return(false); 
      }
      else
      {
	//printf("Opening on %s\n", openOnIP);
      }
    }
    else
    {
      mySin.sin_addr.s_addr=htonl(INADDR_ANY);
    }

    mySin.sin_family=AF_INET;
    //mySin.sin_addr.s_addr=htonl(INADDR_ANY);
    mySin.sin_port=hostToNetOrder(startPort+i);

    if (bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin)) == 0)
      break;
  }

  return(true);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::connectTo(const char *host, int port)
{

  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  if (myFD == INVALID_SOCKET)
    return(false);

  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myErrorStr="Failure to locate host '";
      myErrorStr+=localhost;
      myErrorStr+="'";
      myError = ConBadHost;
      MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connectTo: gethostname failed");
      return(false);
    }
    host=localhost;
  }


  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  //myHost = ((host != NULL) ? host : "");
  //myPort = port;
  myHost = useHost;
  myPort = usePort;

  memset(&mySin, 0, sizeof(mySin));
  if (!hostAddr(useHost, mySin.sin_addr))
  {
    myError = ConBadHost;
    return(false);
  }
  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  myLastStringReadTime.setToNow();
  return(connectTo(&mySin));
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::connectTo(struct sockaddr_in *sin)
{
  myError = NoErr;
  myErrorStr.clear();
  if (::connect(myFD, (struct sockaddr *)sin,
		sizeof(struct sockaddr_in)) < 0)
  {
    myErrorStr="Failure to connect socket";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connectTo: gethostname failed");
    return(0);
  }

  myLastStringReadTime.setToNow();
  return(1);
}
  


/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::close()
{
  myError = NoErr;
  myErrorStr.clear();

  if (myFD != INVALID_SOCKET) {
    if (!myHost.empty() && (myPort != -1)) {
      MvrLog::log(MvrLog::Verbose, "Closing %s socket %s:%i (0x%p)",
                 toString(myType),
                 myHost.c_str(),
                 myPort,
                 myFD);
    }
    else {
      MvrLog::log(MvrLog::Verbose, "Closing %s socket (0x%p)",
                 toString(myType),
                 myFD);
    }
  }

  if (myCloseFunctor != NULL)
    myCloseFunctor->invoke();

  if (myDoClose && (myFD != INVALID_SOCKET))
  {
    int shutdownRet = ::shutdown(myFD, SD_BOTH);
    if (shutdownRet != 0) {
      int error = WSAGetLastError();
      MvrLog::log(MvrLog::Normal, "Shutdown %s socket (0x%p) returns %i (error = %i)",
                 toString(myType),
                 myFD,
                 shutdownRet, 
                 error);
    }

    // ?? Should we do this only if shutdown return successful?
    // ?? In theory, I think that we're supposed to wait for the shutdown to complete.
    // ?? Could this be causing the intermittent hang?

    int closeRet = closesocket(myFD);
    if (closeRet  != 0) {
      int error = WSAGetLastError();
      MvrLog::log(MvrLog::Normal, "Close %s socket (0x%p) returns %i (error = %i)",
                 toString(myType),
                 myHost.c_str(),
                 closeRet,
                 error);
    }

    myFD = INVALID_SOCKET;

    if (!myHost.empty() && (myPort != -1)) {
      MvrLog::log(MvrLog::Verbose, "%s socket %s:%i closed",
                 toString(myType),
                 myHost.c_str(),
                 myPort);
    }
    else {
      MvrLog::log(MvrLog::Verbose, "%s socket closed",
                 toString(myType));
    }

    return(true);
  }

  MvrLog::log(MvrLog::Verbose, "Close %s socket requires no action",
              toString(myType));


  return(false);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::setLinger(int time)
{
  struct linger lin;
  myError = NoErr;
  myErrorStr.clear();

  if (time)
  {
    lin.l_onoff=1;
    lin.l_linger=time;
  }
  else
  {
    lin.l_onoff=0;
    lin.l_linger=time;
  }

  if (setsockopt(myFD, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin)) != 0)
  {
    myErrorStr="Failure to setsockopt LINGER";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::setLinger: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure */
MVREXPORT bool MvrSocket::setBroadcast()
{
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_BROADCAST, NULL, 0) != 0)
  {
    myError = NetFail;
    myErrorStr="Failure to setsockopt BROADCAST";
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::setBroadcast: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure 
    @internal
    @note MvrSocket always sets the reuse-address option in open(), so calling this function is normally unneccesary.
     (This apparently needs to be done after the socket is created before
     the socket is bound.)
*/
MVREXPORT bool MvrSocket::setReuseAddress()
{
  int opt=1;
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_REUSEADDR,
		 (char*)&opt, sizeof(opt)) != 0)
  {
    myErrorStr="Failure to setsockopt REUSEADDR";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::setReuseAddress: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure  */
MVREXPORT bool MvrSocket::setNonBlock()
{
  u_long arg=1;
  myError = NoErr;
  myErrorStr.clear();
  if (ioctlsocket(myFD, FIONBIO, &arg) != 0)
  {
    myErrorStr="Failure to fcntl O_NONBLOCK";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::setNonBlock: fcntl failed");
    return(false);
  }
  else
  {
    myNonBlocking = true;
    return(true);
  }
}

/** @return false and set error code and description string on failure  */
MVREXPORT bool MvrSocket::copy(int fd, bool doclose)
{
  int len;

  myFD=fd;
  myDoClose=doclose;
  myError = NoErr;
  myErrorStr.clear();

  len=sizeof(struct sockaddr_in);
  if (getsockname(myFD, (struct sockaddr*)&mySin, &len))
  {
    myErrorStr="Failed to getsockname on fd ";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::copy: getsockname failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure  */
MVREXPORT bool MvrSocket::accept(MvrSocket *sock)
{
  int len;
  //unsigned char *bytes;
  myError = NoErr;
  myErrorStr.clear(); 
  len=sizeof(struct sockaddr_in);
  sock->myFD=::accept(myFD, (struct sockaddr*)&(sock->mySin), &len);
  sock->myType=myType;
  sock->setRawIPString();
  /*
  bytes = (unsigned char *)sock->inAddr();
  sprintf(sock->myIPString, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 
	  bytes[3]);
  */
  if ((sock->myFD < 0 && !myNonBlocking) || 
      (sock->myFD < 0 && WSAGetLastError() != WSAEWOULDBLOCK && myNonBlocking))
  {
    myErrorStr="Failed to accept on socket";
    myError = ConRefused;
    MvrLog::logErrorFromOS(MvrLog::Terse, 
			  "MvrSocket::accept: accept failed");
    return(false);
  }

  return(true);
}

MVREXPORT void MvrSocket::inToA(struct in_addr *addr, char *buff)
{
  strcpy(buff, inet_ntoa(*addr));
}

/** @return false and set error code and description string on failure  */
MVREXPORT bool MvrSocket::getSockName()
{
  int size;

  myError = NoErr;
  myErrorStr.clear();
  if (myFD == INVALID_SOCKET)
  {
    myErrorStr="Trying to get socket name on an unopened socket";
    myError = NetFail;
    printf(myErrorStr.c_str());
    return(false);
  }

  size=sizeof(mySin);
  if (getsockname(myFD, (struct sockaddr *)&mySin, &size) != 0)
  {
    myErrorStr="Error getting socket name";
    switch (WSAGetLastError())
    {
    case WSAEINVAL:
      myErrorStr+=": inval";
      break;
    case WSANOTINITIALISED:
      myErrorStr+=": not init";
      break;
    }
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSocket::getSockName: getSockName failed");
    return(false);
  }

  return(true);
}

MVREXPORT unsigned int MvrSocket::hostToNetOrder(int i)
{
  return(htons(i));
}

MVREXPORT unsigned int MvrSocket::netToHostOrder(int i)
{
  return(ntohs(i));
}

MVREXPORT std::string MvrSocket::getHostName()
{
  char localhost[MAXGETHOSTSTRUCT];

  if (gethostname(localhost, sizeof(localhost)) == 1)
    return("");
  else
    return(localhost);
}

/** If this socket is a TCP socket, then set the TCP_NODELAY flag to 1,
 *  to disable the use of the Nagle algorithm (which waits until enough
 *  data is ready to send to fill a TCP frame, rather then sending the
 *  packet immediately).
 *  @return true of the flag was successfully set, false if there was an 
 *    error or this socket is not a TCP socket.
 */
MVREXPORT bool MvrSocket::setNoDelay(bool flag)
{
  if(myType != TCP) return false;
  int f = flag?1:0;
  int r = setsockopt(myFD, IPPROTO_TCP, TCP_NODELAY, (char*)&f, sizeof(f));
  return (r != -1);
}

