/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLogFileConnection.cpp
 > Description  : For connecting through a specially formatted log file
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月05日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLogFileConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"

#include <ctype.h>

MVREXPORT MvrLogFileConnection::MvrLogFileConnection()
{
  myStatus   = STATUS_NEVER_OPENED;
  myLogFile  = NULL;
  myFD       = NULL;
  stopAfter  = 1;
  strcpy(myName, "random");
  strcpy(myType, "amigo");
  strcpy(mySubtype, "amigo");
}

MVREXPORT MvrLogFileConnection::~MvrLogFileConnection()
{
  if (myFD != NULL)
    fclose(myFD);
}

MVREXPORT void MvrLogFileConnection::setLogFile(const char *fileName)
{
  if (fileName == NULL)
    myLogFile = "robot.log";
  else
    myLogFile = fileName;
}

MVREXPORT bool MvrLogFileConnection::openSimple(void)
{
  if (internalOpen() == 0)
    return true;
  else
    return false;
}

/**
   @param fname the file to connect to, if NULL (default) then robot.log
   @return 0 for success, otherwise one of the open enums
*/
MVREXPORT int MvrLogFileConnection::open(const char *fileName)
{
  setLogFile(fileName);
  return internalOpen();
}

MVREXPORT int MvrLogFileConnection::internalOpen(void)
{
  havePose = false;
  myFD     = MvrUtil::fopen(myLogFile, "r");
  if (myFD == NULL)
  {
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_FILE_NOT_FOUND;
  }

  char buf[100];
  if (fgets(buf, 100, myFD) != NULL)
  {
    if (strncmp(buf, "// Saphira log file", 19) != 0)
    {
      myStatus = STATUS_OPEN_FAILED;
      fclose(myFD);
      myFD = NULL;
      return OPEN_NOT_A_LOG_FILE;
    }
  }
  else
  {
    myStatus = STATUS_OPEN_FAILED;
    fclose(myFD);
    myFD = NULL;
    return OPEN_NOT_A_LOG_FILE;
  }

  if (fgets(buf, 100, myFD) != NULL)
  {
    if (strncmp(buf, "// Robot position", 17) == 0)
    {
      int x, y, th;
      fgets(buf, 100, myFD);
      sscanf(buf, "%d %d %d" ,&x, &y, &th);
      myPose.setX(x);
      myPose.setY(y);
      myPose.setTh(th);
      havePose = true;
    }
    if (strncmp(buf, "// Robot name", 13) == 0)
    {
      fgets(buf, 100, myFD);
      sscanf(buf, "%s %s %s", myName, myType, mySubtype);
    }
  }
  myStatus = STATUS_OPEN;
  return 0;
}

void MvrLogFileConnection::buildStrMap(void)
{
  myStrMap[OPEN_FILE_NOT_FOUND] = "File not found.";
  myStrMap[OPEN_NOT_A_LOG_FILE] = "File is not a log file.";
}

MVREXPORT const char *MvrLogFileConnection::getOpenMessage(int messageNumber)
{
  return myStrMap[messageNumber].c_str();
}

MVREXPORT bool MvrLogFileConnection::close(void)
{
  myStatus = STATUS_CLOSED_NORMALLY;
  if (myFD != NULL)
    fclose(myFD);
  myFD = NULL;
  return true;
}

MVREXPORT int MvrLogFileConnection::read(const char *data, unsigned int size, unsigned int msWait)
{
  MvrTime timeDone;
  unsigned int bytesRead = 0;
  int n;

  if (getStatus() != STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrLogFileConnection::read: Attempt to use port that is not open.");
    return -1;
  }
  
  timeDone.setToNow();
  if (!timeDone.addMSec(msWait)) {
    MvrLog::log(MvrLog::Normal,
               "MvrLogFileConnection::read() error adding msecs (%i)",
               msWait);
  }

  if (stopAfter-- <= 0)
    {
      stopAfter= 1;
      return 0;
    }

  if (myFD != NULL)
    {
      char line[1000];
      if (fgets(line, 1000, myFD) == NULL) // done with file, close
        {
          close();
          return -1;
        }
      // parse the line
      int i=0;
      n = 0;
      while (line[i] != 0)
        {
          if (isdigit(line[i]))
            {
              if (isdigit(line[i+1]))
                {
                  if (isdigit(line[i+2]))
                    {
                      const_cast<char *>(data)[n++] = 
                        100 * (line[i]-'0') + 10*(line[i+1]-'0') + line[i+2]-'0';
                      i++;
                    }
                  else
                      const_cast<char *>(data)[n++] = 10*(line[i]-'0') + line[i+1]-'0';
                  i++;
                }
              else
                const_cast<char *>(data)[n++] = line[i]-'0';
            }
          i++;
        }
    }

#if 0
  if (n > 0)                    // add in checksum
    {
      int i;
      unsigned char nn;
      int c = 0;

      i = 3;
      nn = data[2] - 2;
      while (nn > 1) 
        {
          c += ((unsigned char)data[i]<<8) | (unsigned char)data[i+1];
          c = c & 0xffff;
          nn -= 2;
          i += 2;
        }
      if (nn > 0) 
        c = c ^ (int)((unsigned char) data[i]);

      const_cast<char *>(data)[n++] = (c << 8) & 0xff;
      const_cast<char *>(data)[n++] = c & 0xff;
    }
#endif

  bytesRead = n;
  return bytesRead;
}

MVREXPORT int MvrLogFileConnection::write(const char *data, unsigned int size)
{
  return size;
}

/*
 * @return the name of the log file
 */
MVREXPORT const char *MvrLogFileConnection::getLogFile(void)
{
  return myLogFile;
}

MVREXPORT int MvrLogFileConnection::getStatus(void)
{
  return myStatus;
}

MVREXPORT bool MvrLogFileConnection::isTimeStamping(void)
{
  return false;
}

MVREXPORT MvrTime MvrLogFileConnection::getTimeRead(int index)
{
  MvrTime now;
  now.setToNow();
  return now;
}
