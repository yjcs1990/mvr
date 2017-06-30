/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotPacketReaderThread.h
 > Description  : 
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRROBOTPACKETREADERTHREAD_H
#define MVRROBOTPACKETREADERTHREAD_H

#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"

class MvrRobot;

class MvrRobotPacketReaderThread : public MvrASyncTask
{
public:
  MVREXPORT MvrRobotPacketReaderThread();
  MVREXPORT virtual ~MvrRobotPacketReaderThread();

  MVREXPORT void setRobot(MvrRobot *robot);

  MVREXPORT void stopRunIfNotConnected(bool stopRun);
  MVREXPORT virtual void *runThread(void *arg);

  MVREXPORT virtual const char *getThreadActivity(void);
protected:
  bool myStopRunIfNotConnected;
  MvrRobot *myRobot;
  bool myInRun;
};
#endif  // MVRROBOTPACKETREADERTHREAD_H