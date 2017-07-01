#ifndef MVRROBOTPACKETREADER_H
#define MVRROBOTPACKETREADER_H


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
  MVREXPORT virtual void * runThread(void *arg);

  MVREXPORT virtual const char *getThreadActivity(void);


protected:
  bool myStopRunIfNotConnected;
  MvrRobot *myRobot;
  bool myInRun;
};


#endif // MVRSYNCLOOP_H
