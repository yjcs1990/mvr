#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRobotPacketReaderThread.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include "MvrRobot.h"


MVREXPORT MvrRobotPacketReaderThread::MvrRobotPacketReaderThread() :
  MvrASyncTask(),
  myStopRunIfNotConnected(false),
  myRobot(0)
{
  setThreadName("MvrRobotPacketReader");
  myInRun = false;
}

MVREXPORT MvrRobotPacketReaderThread::~MvrRobotPacketReaderThread()
{
}

MVREXPORT void MvrRobotPacketReaderThread::setRobot(MvrRobot *robot)
{
  myRobot=robot;
}

MVREXPORT void MvrRobotPacketReaderThread::stopRunIfNotConnected(bool stopRun)
{
  myStopRunIfNotConnected = stopRun;
}

MVREXPORT void * MvrRobotPacketReaderThread::runThread(void *arg)
{
  threadStarted();

  if (!myRobot)
  {
    MvrLog::log(MvrLog::Terse, "MvrRobotPacketReaderThread::runThread: Trying to run the robot packet reader without a robot.");
    return(0);
  }

  // this skips the normal one, since the function will exit when the robot run stops
  myRobot->packetHandlerThreadedReader();
  
  /*
  while (myRunning)
  {
    myInRun = true;
    myRobot->packetHandlerThreadedReader();
    myInRun = false;
  }
  */

  threadFinished();
  return(0);
}

MVREXPORT const char *MvrRobotPacketReaderThread::getThreadActivity(void)
{
  if (myRunning)
    return "Unknown running";
  else
    return "Unknown"; 
}
