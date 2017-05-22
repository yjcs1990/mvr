/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSyncTask.h
 > Description  : Class used internally to manage the tasks that are called every cycle
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/

#ifndef MVRSYNCTASK_H
#define MVRSYNCTASK_H

#include <string>
#include <map>
#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "MvrTaskState.h"

///@internal

class MvrSyncTask
{
public:
  /// Constructor, shouldn't ever do a new on anything besides the root node
  MVREXPORT MvrSyncTask(const char *name, MvrFunctor * functor = NULL, 
                       MvrTaskState::State *state = NULL,
                       MvrSyncTask * parent = NULL);
  /// Destructor
  MVREXPORT virtual ~MvrSyncTask();

  /// Runs the node, which runs all children of this node as well
  MVREXPORT void run(void);
  /// Prints the node, which prints all the children of this node as well
  MVREXPORT void log(int depth = 0);

  /// Gets the state of the task
  MVREXPORT MvrTaskState::State getState(void);
  /// Sets the state of the task
  MVREXPORT void setState(MvrTaskState::State state);
    
  /// Finds the task in the instances list of children, by name
  MVREXPORT MvrSyncTask *findNonRecursive(const char *name);
  /// Finds the task in the instances list of children, by functor
  MVREXPORT MvrSyncTask *findNonRecursive(MvrFunctor *functor);

  /// Finds the task recursively down the tree by name
  MVREXPORT MvrSyncTask *find(const char *name); 
  /// Finds the task recursively down the tree by functor
  MVREXPORT MvrSyncTask *find(MvrFunctor *functor);

  /// Returns what this is running, if anything (recurses)
  MVREXPORT MvrSyncTask *getRunning(void);

  /// Adds a new branch to this instance
  MVREXPORT void addNewBranch(const char *nameOfNew, int position, 
			                       MvrTaskState::State *state = NULL);
  /// Adds a new leaf to this instance
  MVREXPORT void addNewLeaf(const char *nameOfNew, int position, 
                           MvrFunctor *functor, 
                           MvrTaskState::State *state = NULL);

  /// Gets the name of this task
  MVREXPORT std::string getName(void);

  /// Gets the functor this instance runs, if there is one
  MVREXPORT MvrFunctor *getFunctor(void);

  /// Sets the functor called to get the cycle warning time (should only be used from the robot)
  MVREXPORT void setWarningTimeCB(MvrRetFunctor<unsigned int> *functor);
  /// Gets the functor called to get the cycle warning time (should only be used from the robot)
  MVREXPORT MvrRetFunctor<unsigned int> *getWarningTimeCB(void);

  /// Sets the functor called to check if there should be a time warning this cycle (should only be used from the robot)
  MVREXPORT void setNoTimeWarningCB(MvrRetFunctor<bool> *functor);
  /// Gets the functor called to check if there should be a time warning this cycle (should only be used from the robot)
  MVREXPORT MvrRetFunctor<bool> *getNoTimeWarningCB(void);
  
  // removes this task from the map
  MVREXPORT void remove(MvrSyncTask * proc);

  // returns whether this node is deleting or not
  MVREXPORT bool isDeleting(void);
protected:
  std::multimap<int, MvrSyncTask *> myMultiMap;
  MvrTaskState::State *myStatePointer;
  MvrTaskState::State myState;
  MvrFunctor *myFunctor;
  std::string myName;
  MvrSyncTask *myParent;
  bool myIsDeleting;
  MvrRetFunctor<unsigned int> *myWarningTimeCB;
  MvrRetFunctor<bool> *myNoTimeWarningCB;
  // variables for introspection
  bool myRunning;
  // this is just a pointer to what we're invoking so we can know later
  MvrSyncTask *myInvokingOtherFunctor;
};


#endif  // MVRSYNCTASK_H