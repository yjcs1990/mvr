/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFileDeviceConnection.h
 > Description  : Reads/writes data to a plain file or character device.
 > Author       : Yu Jie
 > Create Time  : 2017年06月13日
 > Modify Time  : 2017年06月13日
***************************************************************************************************/

#ifndef MVRFILEDEVICECONNECTION_H
#define MVRFILEDEVICECONNECTION_H

#include "MvrDeviceConnection.h"
#include "mvriaTypedefs.h"
#include "MvrSocket.h"

#include <string>

/// Used occasionally by certain tests/debugging.  
/// For real connections, use MvrSerialConnection or MvrFileDeviceConnection.
class MvrFileDeviceConnection : public MvrDeviceConnection
{
public:
  /// Constructor
  MVREXPORT MvrFileDeviceConnection();
  /// Destructor closes connection
  MVREXPORT virtual ~MvrFileDeviceConnection();

  /// Opens a connection to the given host and port
  MVREXPORT int open(const char *inFileName = NULL, const char *outFileName = NULL, int outFlags = 0);
  bool openSimple() { return this->open() == 0; }
  MVREXPORT virtual bool close(void);
  MVREXPORT virtual int read(const char *data, unsigned int size, unsigned int msWait = 0);
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
#endif  // MVRFILEDEVICECONNECTION_H