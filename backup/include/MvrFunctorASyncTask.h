/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFunctorASyncTask.h
 > Description  : This is like MvrASyncTask, but instead of runThread it uses a functor to run
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRFUNCTORASYNCTASK_H
#define MVRFUNCTORASYNCTASK_H

#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"
#include "MvrFunctor.h"

class MvrFunctorASyncTask : public MvrASyncTask
{
public:
  /// Constructor
  MVREXPORT MvrFunctorASyncTask(MvrRetFunctor1<void *, void *> *functor);
  /// Destructor
  MVREXPORT virtual ~MvrFunctorASyncTask();
  /// Our reimplementation of runThread
  MVREXPORT virtual void *runThread(void *arg);
protected:
  MvrRetFunctor1<void *, void *> *myFunc;
};

#endif  // MVRFUNCTORASYNCTASK_H