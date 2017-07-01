#ifndef MVRFILECONNECTION_H
#define MVRFILECONNECTION_H

#include "MvrDeviceConnection.h"
#include <string>

#include "mvriaTypedefs.h"
#include "MvrSocket.h"

/// Reads/writes data to a plain file or character device. Used occasionally
/// by certain tests/debugging.  For real connections, use MvrSerialConnection or
/// MvrFileDeviceConnection instead.
class MvrFileDeviceConnection: public MvrDeviceConnection
{
 public:
  /// Constructor
  MVREXPORT MvrFileDeviceConnection();
  /// Destructor also closes connection
  MVREXPORT virtual ~MvrFileDeviceConnection();

  /// Opens a connection to the given host and port
  MVREXPORT int open(const char *infilename = NULL, const char *outfilename = NULL, int outflags = 0);
  bool openSimple() { return this->open() == 0; }
  MVREXPORT virtual bool close(void);
  MVREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0);
  MVREXPORT virtual int write(const char *data, unsigned int size);
  virtual int getStatus() { return myStatus; }
  MVREXPORT virtual const char *getOpenMessage(int err);
  MVREXPORT virtual MvrTime getTimeRead(int index);
  MVREXPORT virtual bool isTimeStamping(void);

  /// If >0 then only read at most this many bytes during read(), regardless of supplied size argument
  void setForceReadBufferSize(unsigned int s) { myForceReadBufferSize = s; }

  /// If >0 then add additional, artificial delay in read() by sleeping this many miliseconds per byte read in read().
  void setReadByteDelay(float d) { myReadByteDelay = d; }

protected:
  std::string myInFileName;
  std::string myOutFileName;
  int myInFD;
  int myOutFD;
  int myStatus;
  unsigned int myForceReadBufferSize;
  float myReadByteDelay;
};

#endif
