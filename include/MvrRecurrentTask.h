/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
// MvrRecurrentTask.h -- Recurrent async task interface class
#ifndef ARRECURASYNCTASK_H
#define ARRECURASYNCTASK_H


#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "MvrThread.h"
#include "MvrASyncTask.h"

/// Recurrent task (runs in its own thread)
/**
   The MvrRecurrentTask is a task that runs in its own thread.  Recurrent
   tasks are asynchronous tasks that complete in a finite amount of time,
   and need to be reinvoked recurrently.  A typical example is Saphira's
   localization task: it runs for a few hundred milliseconds, localizes the
   robot, and returns.  Then the cycle starts over.
   The user simply needs to derive their own class
   from MvrRecurrentTask and define the task() function.  This is the user
   code that will be called to execute the task body.
   Then, create an object of the class, and call the go() function to 
   start the task.  The status of the task can be checked with the
   done() function, which returns 0 if running, 1 if completed, and 2 if
   killed.  
   go() can be called whenever the task is done to restart it.  To stop the
   task in midstream, call reset().  
   kill() kills off the thread, shouldn't be used unless exiting the 
   async task permanently

   @ingroup UtilityClasses
*/
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


#endif // ARRECURASYNCTASK_H
