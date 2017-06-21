/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotPacketReaderThread.h
 > Description  : 
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月21日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include "MvrRobot.h"
#include "MvrRobotPacketReaderThread.h"

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
  myRobot = robot;
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
    MvrLog::log(MvrLog::Normal,
                "MvrRobotPacketReaderThread::runThread: Trying to run the robot packet reader without a robot.")
    return 0;                
  }
  // This skips the normal one, since the function will exit when the robot run stops
  myPacket->packetHandlerThreadedReader();

  /*
  while (myRunning)
  {
    myInRun = true;
    myRobot->packetHandlerThreadedReader();
    myInRun = false;
  }
  */
  threadFinished();
  return 0;
}

MVREXPORT const char *MvrRobotPacketReaderThread::getThreadActivity(void)
{
  if (myRunning)
    return "Unknown running";
  else
    return "Unknown";
}
