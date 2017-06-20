/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFunctorASyncTask.cpp
 > Description  : This is like MvrASyncTask, but instead of runThread it uses a functor to run
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrFunctorASyncTask.h"

MVREXPORT MvrFunctorASyncTask::MvrFunctorASyncTask(MvrRetFunctor1<void *, void *> *functor)
{
  setThreadName(functor->getName());
  myFunc = functor;
}

MVREXPORT MvrFunctorASyncTask::~MvrFunctorASyncTask()
{

}

MVREXPORT void *MvrFunctorASyncTask::runThread(void *arg)
{
  threadStarted();
  void *ret = myFunc->invokeR(arg);
  threadFinished();
  return ret;
}

