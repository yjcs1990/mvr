/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSyncLoop.cpp
 > Description  : 
 > Author       : Yu Jie
 > Create Time  : 2017年05月18日
 > Modify Time  : 2017年06月19日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrLog.h"
#include "MvrRobot.h"
#include "MvrSyncLoop.h"

MVREXPORT MvrSyncLoop::MvrSyncLoop() :
          MvrASyncTask(),
          myStopRunIfNotConnected(false),
          myRobot(0)
{
  setThreadName("MvrRobotSyncLoop");
  myInRun = false;
}          

MVREXPORT MvrSyncLoop::~MvrSyncLoop()
{

}

MVREXPORT void MvrSyncLoop::setRobot(MvrRobot *robot)
{
  myRobot = robot;
}

MVREXPORT void MvrSyncLoop::stopRunIfNotConnected(bool stopRun)
{
  myStopRunIfNotConnected = stopRun;
}

MVREXPORT void *MvrSyncLoop::runThread(void *arg)
{
  threadStarted();

  long timeToSleep;
  MvrTime loopEndTime;
  std::list<MvrFunctor *> *runList;
  std::list<MvrFunctor *>::iterator iter;
  MvrTime lastLoop;
  bool firstLoop = true;
  bool warned = false;

  if (!myRobot)
  {
    MvrLog::log(MvrLog::Terse,
                "MvrSyncLoop::runThread: Trying to run the syncchronous loop without a robot.");
    return 0;
  }
  if (!myRobot->getSyncTaskRoot())
  {
    MvrLog::log(MvrLog::Terse,
                "MvrSyncLoop::runThread: Cannot run the syncchronous loop without a task tree.");
    return 0;    
  }

  while(myRunning)
  {
    myRobot->lock();
    if (!firstLoop && !warned && !myRobot->getNoTimeWarningThisCycle() && 
        myRobot->getCycleWarningTime() != 0 && 
        myRobot->getCycleWarningTime() > 0 && 
        lastLoop.mSecSince() > (signed int) myRobot->getCycleWarningTime())
    {
      MvrLog::log(MvrLog::Normal,
                  "Warning: MvrRobot cycle took too long because the loop was waiting for lock.");
      MvrLog::log(MvrLog::Normal,
                  "\tThe cycle took %u ms, (%u ms normal %u ms warning)",
                  lastLoop.mSecSince(), myRobot->getCycleTime(),
                  myRobot->getCycleWarningTime());
    }
    myRobot->setNoTimeWarningThisCycle(false);
    firstLoop = false;
    warned    = false;
    lastLoop.setToNow();

    loopEndTime.setToNow();
    if (!loopEndTime.addMSec(myRobot->getCycleTime())){
      MvrLog::log(MvrLog::Normal,
                  "MvrSyncLoop::runThread() error adding msces (%i)",
                  myRobot->getCycleTime());
    }
    myRobot->incCounter();
    myRobot->unlock();

    // note that all the stuff beyond here should maybe have a lock
    // but it should be okay because its just getting data
    myInRun = true;
    myRobot->getSyncTaskRoot()->run();
    myInRun = false;
    if (myStopRunIfNotConnected && !myRobot->isConnected())
    {
      if (myRunning)
        MvrLog::log(MvrLog::Normal, "Exiting robot run because of lost connection.");
      break;
    }
    timeToSleep = loopEndTime.mSecTo();

    if (myRobot->isCycleChained() && myRobot->isConnected())
      timeToSleep = 0;
    
    if (!myRobot->getNoTimeWarningThisCycle() &&
        myRobot->getCycleWarningTime() != 0 && 
        myRobot->getCycleWarningTime() > 0 && 
        lastLoop.mSecSince() > (signed int) myRobot->getCycleWarningTime())   
    {
      MvrLog::log(MvrLog::Normal,
                  "Warning: MvrRobot sync tasks too long at %u ms, (%u ms normal %u ms warning)",
                  lastLoop.mSecSince(), myRobot->getCycleTime(),
                  myRobot->getCycleWarningTime());
      warned = true;
    } 

    if (timeToSleep > 0)
      MvrUtil::sleep(timeToSleep);
  }
  myRobot->lock();
  myRobot->wakeAllRunExitWaitingThreads();
  myRobot->unlock();

  myRobot->lock();
  runList = myRobot->getRunExitListCopy();
  myRobot->unlock();
  for (iter = runList->begin(); iter != runList->end(); ++iter)
    (*iter)->invoke();
  delete runList;

  threadFinished();
  return 0;        
}

MVREXPORT const char *MvrSyncLoop::getThreadActivity(void)
{
  if (myRunning)
  {
    MvrSyncTask *syncTask;
    syncTask = myRobot->getSyncTaskRoot()->getRunning();
    if (syncTask != NULL)
      return syncTask->getName().c_str();
    else
      return "Unknown running";
  }
  else
    return "Unknown";
}