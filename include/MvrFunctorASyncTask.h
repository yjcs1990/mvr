#ifndef MVRFUNCTORASYNCTASK_H
#define MVRFUNCTORASYNCTASK_H

#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"
#include "MvrFunctor.h"

/// This is like MvrASyncTask, but instead of runThread it uses a functor to run
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

#endif
