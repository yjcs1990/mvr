#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaTypedefs.h"
#include "MvrFileDeviceConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

MVREXPORT MvrFileDeviceConnection::MvrFileDeviceConnection() :
  myInFD(-1), myOutFD(-1), myStatus(STATUS_NEVER_OPENED),
  myForceReadBufferSize(0), myReadByteDelay(0.0)
{
}

MVREXPORT MvrFileDeviceConnection::~MvrFileDeviceConnection()
{
  close();
}


/** 
 * @param infilename Input file name, or NULL to use stdin
 * @param outfilename Output file name, or NULL to use stdout
 * @param outflags Additional flags to pass to system open() function when opening
 * the output file in addition to O_WRONLY. For example, pass O_APPEND to append rather than rewrite the
 * file.
 */
MVREXPORT int MvrFileDeviceConnection::open(const char *infilename, const char *outfilename, int outflags)
{
  myStatus = STATUS_OPEN_FAILED;

  if(infilename)
  {
    myInFileName = infilename;
    myInFD = MvrUtil::open(infilename, O_RDONLY);
    if(myInFD == -1)
    {
      MvrLog::log(MvrLog::Terse, "MvrFileDeviceConnection: error opening input file \"%s\": %s", infilename, strerror(errno));
      return errno;
    }
  }
  else
  {
#ifdef WIN32
    myInFD = _fileno(stdin);
#else
    myInFD = STDIN_FILENO;
#endif
  }

  if(outfilename)
  {
    myOutFileName = outfilename;
    myOutFD = MvrUtil::open(outfilename, O_WRONLY|outflags);
    if(myInFD == -1)
    {
      MvrLog::log(MvrLog::Terse, "MvrFileDeviceConnection: error opening output file \"%s\": %s", outfilename, strerror(errno));
      return errno;
    }
  }
  else
  {
#ifdef WIN32
    myOutFD = _fileno(stdout);
#else
    myOutFD = STDOUT_FILENO;
#endif
  }

  myStatus = STATUS_OPEN;

  return 0;
}

MVREXPORT bool MvrFileDeviceConnection::close(void)
{
  MvrUtil::close(myInFD);
  MvrUtil::close(myOutFD);
  myStatus = STATUS_CLOSED_NORMALLY;
  return true;
}

MVREXPORT int MvrFileDeviceConnection::read(const char *data, unsigned int size, unsigned int msWait)
{
  unsigned int s = myForceReadBufferSize > 0 ? MvrUtil::findMinU(size, myForceReadBufferSize) : size;
#ifdef WIN32
  int r = _read(myInFD, (void*)data, s);
#else
  int r =  ::read(myInFD, (void*)data, s);
#endif
  if(myReadByteDelay > 0)
    MvrUtil::sleep(r * myReadByteDelay);
  // TODO add option for intermittent data by returning 0 until a timeout has passed.
  // TODO add option to delay full lines (up to \n\r or \n) in above behavior
  return r;
}

MVREXPORT int MvrFileDeviceConnection::write(const char *data, unsigned int size)
{
#ifdef WIN32
  return (int) _write(myOutFD, (void*)data, (size_t)size);
#else
  return (int) ::write(myOutFD, (void*)data, (size_t)size);
#endif
}


MVREXPORT bool MvrFileDeviceConnection::isTimeStamping(void)
{
  return false;
}

MVREXPORT MvrTime MvrFileDeviceConnection::getTimeRead(int index)
{
  MvrTime now;
  now.setToNow();
  return now;
}

MVREXPORT const char * MvrFileDeviceConnection::getOpenMessage(int err)
{
  return strerror(err);
}

