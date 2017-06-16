/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSocket.cpp
 > Description  : For connecting to a device through a TCP network socket
 > Author       : Yu Jie
 > Create Time  : 2017年05月18日
 > Modify Time  : 2017年06月15日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrFunctor.h"
#include "MvrSocket.h"

#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

/// We're always initialized in Linux
bool MvrSocket::ourInitialized = true;

bool MvrSocket::init()
{
  return true;
}

void MvrSocket::shutdown()
{

}

MvrSocket::MvrSocket() : myType(Unknown),
                         myError(NoErr),
                         myErrorStr(),
                         myDoClose(true),
                         myFD(-1),
                         myNonBlocking(false),
                         mySin()
{
  internalInit();
}                

/**
   Constructs the socket and connects it to the given host.
   @param host hostname of the server to connect to
   @param port port number of the server to connect to
   @param type protocol type to use
*/         
MvrSocket::MvrSocket(const char *host, int port, Type type) :
            myType(type),
            myError(NoErr),
            myErrorStr(),
            myDoClose(true),
            myFD(-1),
            myNonBlocking(false),
            mySin()
{
  internalInit();
  connect(host, port, type);
}            

MvrSocket::MvrSocket(int port, bool doClose, Type type) :
            myType(type),
            myError(NoErr),
            myErrorStr(),
            myDoClose(doClose),
            myFD(-1),
            myNonBlocking(false),
            mySin()
{
  internalInit();
  connect(port, type);
}     

MvrSocket::~MvrSocket()
{
  close();
}       

bool MvrSocket::hostAddr(const char *host, struct in_addr &addr)
{
  struct hostent *hp;
  if (!(hp=gethostbyname(host)))
  {
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::hostAddr: gethostbyname failed");
    memset(&addr, 0, sizeof(in_addr));                           
  }
  else
  {
    bcopy(hp->h_addr, &addr, hp->h_length);
    return true;
  }
}

bool MvrSocket::addrHost(struct in_addr &addr, char *host)
{
  struct hostent *hp;

  hp = gethostbyaddr((char *)&addr.s_addr, sizeof(addr.s_addr), AF_INT);
  if (hp)
    strcpy(host, hp->h_name);
  else
    strcpy(host, inet_ntoa(addr));
  return true;
}

std::string MvrSocket::getHostName()
{
  char localhost[maxHostNameLen()];

  if (gethostname(localhost, sizeof(localhost)) == 1)
    return ("");
  else
    return localhost;
}

/** @return false and set error code and description string on failure  */
bool MvrSocket::connect(const char *host, int port, Type type, const char *openOnIP)
{
  char localhost[maxHostNameLen()];
  myError = noErr;
  myErrorStr.clear();
  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myError    = ConBadHost;
      myErrorStr = "Failure to locate host '";
      myErrorStr += localhost;
      myErrorStr += "'";
      MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: gethostname failed");
      return false;
    }
    host = localhost;
  }

  bzero(&mySin, sizeof(mySin));

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &useHost);

  if (!hostAddr(useHost, mySin.sin_addr))
    return false;
  setRawIPString();
  mySin.sin_family = AF_INET;
  mySin.sin_port   = hostToNetOrder(usePort);

  if ((type == TCP) && (myFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    myError    = NetFail;
    myErrorStr = "Failure to make TCP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: could not make tcp socket");
    return false;
  }
  else if ((type == UDP) && (myFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    myError    = NetFail;
    myErrorStr = "Failure to make UDP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: could not make dup socket");
    return false;    
  }

  MvrUtil::setFileCloseOnExec(myFD);

  if (openOnIP != NULL)
  {
    struct sockaddr_in outSin;
    if (!hostAddr(openOnIP, outSin.sin_addr))
    {
      myError    = NameLookup;
      myErrorStr = "Name lookup failed";
      MvrLog::log(MvrLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
      return false;       
    }
    outSin.sin_family = AF_INET;
    outSin.sin_port   = hostToNetOrder(0);
    if (bind(myFD, (struct sockaddr *)&outSin, sizeof(outSin)) < 0)
    {
      MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::connect: Failure to bind socket to port %d", 0);
      return false;
    }
  }
  myType = type;

  if (::connect(myFD, (struct sockaddr *)&mySin), sizeof(struct sockaddr_in) < 0)
  {
    myErrorStr = "Failure to connect socket";
    switch(errno)
    {
      case ECONNREFUSED:
        myError=ConRefused;
        myErrorStr+="; Connection refused";
        break;
      case ENETUNREACH:
        myError=ConNoRoute;
        myErrorStr+="; No route to host";
        break;
      default:
        myError=NetFail;
        break;
    }
    MvrLog::logErrorFromOS(MvrLog::Verbose, "MvrSocket::connect: could not connect");

    ::close(myFD);
    myFD = -1;
    return false;
  }
  return true;
}

/** @return false and set error code and description string on failure  */
bool MvrSocket::open(int port, Type type, const char *openOnIP)
{
  int ret;
  char localhost[maxHostNameLen()];

  myError    = NoErr;
  myErrorStr.clear();
  if ((type == TCP) &&((myFD = socket(AF_INET, SOCK_STREAM, 0)) < 0))
  {
    myErrorStr = "Failure to make TCP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::open: could not create tcp socket");
    return false;                           
  }
  eles if ((type == UDP) &&((myFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0))
  {
    myErrorStr = "Failure to make UDP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::open: could not create udp socket");
    return false; 
  }

  MvrUtil::setFileCloseOnExec(myFD);

  setLinger(0);
  setReuseAddress();

  myType = type;

  bzero(&mySin, sizeof(mySin));

  if (openOnIP != NULL)
  {
    if (!hostAddr(openOnIP, mySin.sin_addr))
    {
      MvrLog::log(MvrLog::Normal,
                  "Couldn't find ip of %s to open on", openOnIP);
      myError    = NameLookup;
      myErrorStr = "Name look up failure";
      ::close(myFD);
      myFD       = -1;
      return false; 
    }
    else
    {
      // printf("Opening on %s\n", openOnIP);
    }
  }
  else
  {
    mySin.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(port);

  if ((ret=bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin))) < 0)
  {
    myError = NetFail;
    myErrorStr="Failure to bind socket to port ";
    sprintf(localhost, "%d", port);
    myErrorStr+=localhost;
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not bind");
    ::close(myFD);
    myFD = -1;
    return false;
  }

  if ((type == TCP) && (listen(myFD, 5) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to listen on socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::open: could not listen");
    ::close(myFD);
    myFD = -1;
    return false ;
  }

  myLastStringReadTime.setToNow();
  return true;
}

/** @return false and set error code and description string on failure  */
bool MvrSocket::create(Type type)
{
  myError = NoErr;
  myErrorStr.clear();
  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to make TCP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: could not create tcp socket");
    return false;
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to make UDP socket";
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSocket::create: could not create udp socket");
    return false;
  }

  MvrUtil::setFileCloseOnExec(myFD);

  myType=type;

  if (getSockName())
    return true;
  else
    return false;
}

/** @return false on error */
bool MvrSocket::findValidPort(int startPort, const char *openOnIP)
{
 
  for (int i=0; i+startPort < 65000; ++i)
  {
    bzero(&mySin, sizeof(mySin));
    setRawIPString();
    if (openOnIP != NULL)
    {
      
      if (!hostAddr(openOnIP, mySin.sin_addr))
      {
        MvrLog::log(MvrLog::Normal, "Couldn't find ip of %s to open udp on", openOnIP);
        return false; 
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
    mySin.sin_port=hostToNetOrder(startPort+i);

    if (bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin)) == 0)
      break;
  }
  return true;
}

/** @return false and set error code and description string on failure */
bool MvrSocket::connectTo(const char *host, int port)
{
  char localhost[maxHostNameLen()];
  myError = NoErr;
  myErrorStr.clear();
  if (myFD < 0)
    return false;
  
  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myErrorStr = "Failure to locale host '";
      myErrorStr += localhost;
      myErrorStr += "'";
      MvrLog::logErrorFromOS(MvrLog::Normal,
                             "MvrSocket::connectTo: gethostname failed");
    }
    host = localhost;
  }

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &useHost);

  bzero(&mySin, sizeof(mySin));
  if (!hostAddr(useHost, mySin.sin_addr))
    return false;
  setRawIPString;
  mySin.sin_family = AF_INET;
  mySin.sin_port   = hostToNetOrder(usePort);

  myLastStringReadTime.setToNow();
  return connectTo(&mySin);
}

/** @return false and set error code and description string on failure */
bool MvrSocket::connectTo(struct sockaddr_in *sin)
{
  myError = NoErr;
  myErrorStr.clear();
  if (::connect(myFD, (struct sockaddr *) sin, sizeof(struct sockaddr_in)) < 0)
  {
    myErrorStr = "Failure to connect socket";
    myError    = ConRefused;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::connectTo: connected failed.");
    return 0;
  }
  myLastStringReadTime.setToNow();
  return 1;
}

bool MvrSocket::close()
{
  if (myFD == -1)
    return true;
  MvrLog::log(MvrLog::Verbose, "Closing socket");
  if (myCloseFunctor != NULL)
    myCloseFunctor->invoke();
  if (myDoClose && ::close(myFD))
  {
    myFD = -1;
    return false;
  }
  else
  {
    myFD = -1;
    return true;
  }
}

/** @return false and set error code and description string on failure. */
bool MvrSocket::setLinger(int time)
{
  struct linger lin;
  myError = NoErr;
  myErrorStr.clear();

  if (time)
  {
    lin.l_onoff  = 1;
    lin.l_linger = time;
  }
  else
  {
    lin.l_onoff  = 0;
    lin.l_linger = time;
  }

  if (setsockopt(myFD, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin)) != 0)
  {
    myErrorStr = "Failure to setsockopt LINGER";
    myError    = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::setLinger: setsockopt failed");
    return false;
  } 
  else
    return true;
}

/** @return false and set error code and description string on failure. */
bool MvrSocket::setBroadcast()
{
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_BROADCAST, &lin, sizeof(lin)) != 0)
  {
    myErrorStr = "Failure to setsockopt BROADCAST";
    myError    = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::setBroadcast: setsockopt failed");
    return false;
  } 
  else
    return true;  
}

/*
 * @return false and set error code and description string on failure. 
 * @internal
 */
bool MvrSocket::setReuseAddress()
{
  int opt = 1;
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) != 0)
  {
    myErrorStr = "Failure to setsockopt REUSEADDR";
    myError    = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::setReuseAddress: setsockopt failed");
    return false;
  } 
  else
    return true;   
}

/** @return false and set error code and description string on failure.  */
bool MvrSocket::setNonBlock()
{
  myError = NoErr;
  myErrorStr.clear();
  if (fcntl(myFD, F_SETFL, O_NONBLOCK) != 0)
  {
    myErrorStr = "Failure to fcntl O_NONBLOCK";
    myError    = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::setNonBlock: fcntl failed");
    return false;
  } 
  else
  {
    myNonBlocking = true;
    return true;     
  }
}

/**
   Copy socket structures. Copy from one Socket to another will still have
   the first socket close the file descripter when it is destructed.
 @return false and set error code and description string on failure.  
*/
bool MvrSocket::copy(int fd, bool doclose)
{
  socklen_l len;

  myFD      = fd;
  myDoClose = doclose;
  myType    = unknown;

  len       = sizeof(struct sockaddr_in);
  if (getsockname(myFD, (struct sockaddr*)&mySin, &len))
  {
    myErrorStr = "Failure to getsockname on fd ";
    myError    = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal,
                           "MvrSocket::copy: getsockname failed");    
    return false;                           
  }
  else
    return true;
}


/*
 * @return true if there are no errors, false if there are
 * errors... not that if you're in non-blocking mode and there is no
 * socket to connect that is NOT an error, you'll want to check the
 * getFD on the sock you pass in to see if it is actually a valid
 * socket.
 */
 bool MvrSocket::accept(MvrSocket *sock)
 {
   socklen_t len;
   // unsigned char *bytes;

   myError = NoErr;
   myErrorStr.clear();

   len = sizeof(struct sockaddr_in);
   sock->myFD=::accept(myFD, (struct sockaddr *)&(sock->mySin), &len);
   sock->myType = myType;
   sock->setRawIPString();


  // bytes = (unsigned char *)sock->inAddr();
  // sprintf(sock->myIPString, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 
  // bytes[3]);

  if ((sock->myFD < 0 && !myNonBlocking) || (sock->myFD < 0 && errno != EWOULDBLOCK && myNonBlocking))
  {
    myErrorStr = "Failed to accept on socket";
    myError    = ConRefused;
    MvrLog::logErrorFromOS(MvrLog::Terse, 
                           "MvrSocket::accept: accept failed");
    return false;                           
  }
  return true;
 }

 void MvrSocket::inToA(struct in_addr *addr, char *buff)
 {
   strcpy(buff, inet_ntoa(*addr));
 }

bool MvrSocket::getSockName()
{
  socklen_t size;
  myError = NoErr;
  myErrorStr.clear();
  if (myFD < 0)
  {
    myErrorStr="Trying to get socket name on an unopened socket";
    myError = NetFail;
    printf(myErrorStr.c_str());
    return false;
  }

  size=sizeof(mySin);
  if (getsockname(myFD, (struct sockaddr *)&mySin, &size) != 0)
  {
    myErrorStr="Error getting socket name";
    myError = NetFail;
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			                     "MvrSocket::getSockName: getSockName failed");
    return false;
  }

  return true;
}

unsigned int MvrSocket::hostToNetOrder(int i)
{
  return(htons(i));
}

unsigned int MvrSocket::netToHostOrder(int i)
{
  return(ntohs(i));
}

/** If this socket is a TCP socket, then set the TCP_NODELAY flag,
 *  to disable the use of the Nagle algorithm (which waits until enough
 *  data is ready to send to fill a TCP frame, rather then sending the
 *  packet immediately).
 *  @param flag true to turn on NoDelay, false to turn it off.
 *  @return true of the flag was successfully set, false if there was an 
 *    error or this socket is not a TCP socket.
 */
bool MvrSocket::setNoDelay(bool flag)
{
  if(myType != TCP) 
    return false;
  int f = flag?1:0;
  int r = setsockopt(myFD, IPPROTO_TCP, TCP_NODELAY, (char*)&f, sizeof(f));
  return (r != -1);
}

MVREXPORT const char *MvrSocket::toString(Type t)
{
  switch (t)
  {
    case TCP:
      return "TCP";
    case UDP:
      return "UDP";
    default:
      return "Unknown";
  }
  return "Unknown";
}

void MvrSocket::internalInit(void)
{
  myReadStringMutex.setLogName("MvrMutex::myReadStringMutex");
  myWriteStringMutex.setLogName("MvrMutex::myWriteStringMutex");
  myCloseFunctor    = NULL;
  myStringAutoEcho  = true;
  myStringEcho      = false;
  myStringPosLast   = 0;
  myStringPos       = 0;
  myStringGotComplete = false;
  myStringBufEmpty[0] = '\0';
  myStringGotEscapeChars = false;
  myStringHaveEchoed  = false;
  myLastStringReadTime.setToNow();
  myLogWriteStrings   = false;
  sprintf(myRawIPString, "none");
  myIPString        = "";
  myBadWrite        = false;
  myBadRead         = false;
  myStringIgnoreReturn = false;
  myStringWrongEndChars=false;
  myErrorTracking   = false;
  myFakeWrites      = false;
  resetTracking();
}

/// Normally, write() should be used instead. This is a wrapper around the sendto() system call.
MVREXPORT int MvrSocket::sendTo(const char *msg, int len)
{
  int ret;
  ret = ::sendto(myFD, (char*) msg, len, 0, (struct sockaddr*)&mySin, sizeof(mySin));
  if (ret > 0)
  {
    mySends++;
    myBytesSent += ret;
  }
  return ret;
}

/// Normally, write() should be used instead. This is a wrapper around the sendto() system call.
MVREXPORT int MvrSocket::sendTo(const char *msg, int len, struct sockaddr_in *sin)
{
  int ret;
  ret = ::sendto(myFD, (char*) msg, len, 0, (struct sockaddr*)sin, sizeof(struct sockaddr_in));

  if (ret > 0)
  {
    mySends++;
    myBytesSent += ret;
  }
  return ret;  
}

/// Normally, read() should be used instead. This is a wrapper around the recvfrom() system call.
MVREXPORT int MvrSocket::recvFrom(void *msg, int len, sockaddr_in *sin)
{
#ifdef WIN32
  int i=sizeof(sockaddr_in);
#else
  socklen_t i = sizeof(sockaddr_in);
#endif
  int ret;
  ret = ::recvfrom(myFD, (char *)msg, len, 0, (struct sockaddr*)sin, &i);
  if (ret > 0)
  {
    myRecvs++;
    myBytesRecvd += ret;
  }
  return ret;
}

/*
 * @param buff buffer to write from
 * @param len how many bytes to write
 * @return number of bytes written
*/
MVREXPORT int MvrSocket::write(const void *buff, size_t len)
{
  // this is for when we're faking MvrNetworking commands over the text server
  if (myFakeWrites)
    return len;
  
  if (myFD < 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrSocket::write: called after socket closed.");
    return 0;
  }

  struct timeval tval;
  fd_set fdSet;
  tval.tv_sec  = 0;
  tval.tv_usec = 0;
  FD_ZERO(&fdSet);
  FD_SET(myFD, &fdSet);

#ifdef WIN32
  if (select(0, NULL, &fdSet, NULL, &tval) <= 0) // fd count is ignored on windows (fd_set is an array)
#else
  if (select(myFD+1, NULL, &fdSet, NULL, &tval) <= 0)
#endif 
    return 0;
  
  int ret;
#ifdef WIN32
  ret = ::send(myFD, (char*)buff, len, 0);
#else
  ret = ::write(myFd, (char *)buff, len);
#endif 

  if (ret > 0)
  {
    mySends++;
    myBytesSent += mySends;
  }

  if (myErrorTracking && ret < 0)
  {
    if (myNonBlocking)
    {
#ifdef WIN32
      if (WSAGetLastError() != WSAEWOULDBLOCK)
        myBadWrite = true;
#endif

#ifndef WIN32
      if (errno != EAGAIN)
        myBadWrite = true;
#endif        
    }
    else
      myBadWrite = false;
  }
  return ret;
}

/**
   @param buff buffer to read into
   @param len how many bytes to read
   @param msWait if 0, don't block, if > 0 wait this long for data
   @return number of bytes read
*/
MVREXPORT int MvrSocket::read(void *buff, size_t len, unsigned int msWait)
{
  if (myFD < 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrSocket::read: called after socket closed.");
    return 0;
  }

  int ret;
  if (myWait != 0)
  {
    struct timeval tval;
    fd_set fdSet;
    tval.tv_sec  = msWait / 1000;
    tval.tv_usec = (msWait %1000) * 1000;
    FD_ZERO(&fdSet);
    FD_SET(myFD, &fdSet);
#ifdef WIN32
    if (select(0, &fdSet, NULL,, NULL, &tval) <= 0) 
      return 0;
#else
    if (select(myFD+1, &fdSet,NULL, NULL, &tval) <= 0)
      return 0;  
#endif        
  }
  ret = ::recv(myFD, (char*) buff, len, 0);
  if (ret > 0)
  {
    myRecvs++;
    myBytesRecvd += ret;
  }
  if (myErrorTracking && ret < 0)
  {
    if (myNonBlocking)
    {
#ifdef WIN32
      if (WSAGetLastError() != WSAEWOULDBLOCK)
	      myBadRead = true;
#endif
#ifndef WIN32
      if (errno != EAGAIN)
	      myBadRead = true;
#endif      
    }
    else
      myBadRead = false;
  }
  return ret;
}

