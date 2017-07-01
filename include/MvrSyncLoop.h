#ifndef MVRSYNCLOOP_H
#define MVRSYNCLOOP_H


#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"
#include "MvrSyncTask.h"


class MvrRobot;


class MvrSyncLoop : public MvrASyncTask
{
public:

  MVREXPORT MvrSyncLoop();
  MVREXPORT virtual ~MvrSyncLoop();

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
