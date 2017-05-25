/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRecurrentTask.h
 > Description  : Recurrent task (runs in its own thread)
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRECURRENTTASK_H
#define MVRRECURRENTTASK_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "MvrThread.h"
#include "MvrASyncTask.h"

class MvrRecurrentTask : public MvrASyncTask
{
public:
  /// Constructor
  MVREXPORT MvrRecurrentTask();
  /// Descructor
  MVREXPORT ~MvrRecurrentTask();	
  /// The main run loop
  /**
     Override this function and put your task here. 
  */
  virtual void task() = 0;
  /// Starts up on cycle of the recurrent task
  MVREXPORT void go();		
  /// Check if the task is running or not
  /**
     0 = running, 1 = finished normally, 2 = canceled
  */
  MVREXPORT int  done();	
  /// Cancel the task and reset for the next cycle
  MVREXPORT void reset();	// stops the current thread and restarts it
  MVREXPORT void kill();	        // kills the current thread

  MVREXPORT void *runThread(void *ptr); // main task loop

private:
  bool running;			// true if currently running
  bool go_req;			// run request
  bool killed;			// did we get killed by request?
};

#endif  // MVRRECURRENTTASK_H