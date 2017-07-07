#ifndef MVRTCPCONNECTION_H
#define MVRTCPCONNECTION_H

#include "MvrDeviceConnection.h"
#include <string>

#include "mvriaTypedefs.h"
#include "MvrSocket.h"

/// For connecting to a device through a TCP network socket
/// @ingroup UtilityClasses
class MvrTcpConnection: public MvrDeviceConnection
{
 public:
  /// Constructor
  MVREXPORT MvrTcpConnection();
  /// Destructor also closes connection
  MVREXPORT virtual ~MvrTcpConnection();

  /// Opens a connection to the given host and port
  MVREXPORT int open(const char * host = NULL, int port = 8101);

  MVREXPORT void setPort(const char *host = NULL, int port = 8101);
  MVREXPORT virtual bool openSimple(void);  
  MVREXPORT virtual int getStatus(void);
  MVREXPORT virtual bool close(void);
  MVREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0);
  MVREXPORT virtual int write(const char *data, unsigned int size);
  MVREXPORT virtual const char * getOpenMessage(int messageNumber);
  MVREXPORT virtual MvrTime getTimeRead(int index);
  MVREXPORT virtual bool isTimeStamping(void);

  /// Gets the name of the host connected to
  MVREXPORT std::string getHost(void);
  /// Gets the number of the port connected to
  MVREXPORT int getPort(void);

  /// Internal function used by open and openSimple
  MVREXPORT int internalOpen(void);

  /// Sets the tcp connection to use this socket instead of its own
  MVREXPORT void setSocket(MvrSocket *socket);
  /// Gets the socket this tcp connection is using
  MVREXPORT MvrSocket *getSocket(void);
  /// Sets the status of the device, ONLY use this if you're playing
  /// with setSocket and know what you're doing
  MVREXPORT void setStatus(int status);

  enum Open { 
      OPEN_NET_FAIL = 1, ///< Some critical part of the network isn't working
      OPEN_BAD_HOST, ///< Could not find the host
      OPEN_NO_ROUTE, ///< Know where the host is, but can't get to it
      OPEN_CON_REFUSED ///< Got to the host but it didn't allow a connection
  };



protected:
  void buildStrMap(void);
  
  MvrStrMap myStrMap;
  bool myOwnSocket;
  MvrSocket *mySocket;
  int myStatus;
  
  std::string myHostName;
  int myPortNum;
};

#endif //MVRTCPCONNECTION_H
