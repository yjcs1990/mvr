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
