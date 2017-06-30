/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLogFileConnection.h
 > Description  : For connecting through a specially formatted log file
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#ifndef MVRLOGFILECONNECTION_H
#define MVRLOGFILECONNECTION_H

#include "MvrDeviceConnection.h"
#include <string>
#include <stdio.h>

#include "mvriaTypedefs.h"

class MvrLogFileConnection: public MvrDeviceConnection
{
 public:
  /// Constructor
  MVREXPORT MvrLogFileConnection();
  /// Destructor also closes connection
  MVREXPORT virtual ~MvrLogFileConnection();

  /// Opens a connection to the given host and port
  MVREXPORT int open(const char * fname = NULL);

  MVREXPORT void setLogFile(const char *fname = NULL);
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
  MVREXPORT const char *getLogFile(void);

  /// Internal function used by open and openSimple
  MVREXPORT int internalOpen(void);

  enum Open { 
      OPEN_FILE_NOT_FOUND = 1,  ///< Can't find the file
      OPEN_NOT_A_LOG_FILE       ///< Doesn't look like a log file
  };

  // robot parameters
  MvrPose myPose;     
  bool havePose;
  char myName[100];
  char myType[20];
  char mySubtype[20];

protected:
  void buildStrMap(void);
  MvrStrMap myStrMap;

  int myStatus;
  int stopAfter;                // temp here for returning one packet at a time
  
  const char *myLogFile;
  FILE *myFD;                   // file descriptor

};

#endif  // MVRLOGFILECONNECTION_H