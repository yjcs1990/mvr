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
  MVEXPORT MvrSyncTask(const char *name, MvrFunctor * functor = NULL, 
                       MvrTaskState::State *state = NULL,
                       MvrSyncTask * parent = NULL);
  /// Destructor
  MVEXPORT virtual ~MvrSyncTask();

  /// Runs the node, which runs all children of this node as well
  MVEXPORT void run(void);
  /// Prints the node, which prints all the children of this node as well
  MVEXPORT void log(int depth = 0);

  /// Gets the state of the task
  MVEXPORT MvrTaskState::State getState(void);
  /// Sets the state of the task
  MVEXPORT void setState(MvrTaskState::State state);
    
  /// Finds the task in the instances list of children, by name
  MVEXPORT MvrSyncTask *findNonRecursive(const char *name);
  /// Finds the task in the instances list of children, by functor
  MVEXPORT MvrSyncTask *findNonRecursive(MvrFunctor *functor);

  /// Finds the task recursively down the tree by name
  MVEXPORT MvrSyncTask *find(const char *name); 
  /// Finds the task recursively down the tree by functor
  MVEXPORT MvrSyncTask *find(MvrFunctor *functor);

  /// Returns what this is running, if anything (recurses)
  MVEXPORT MvrSyncTask *getRunning(void);

  /// Adds a new branch to this instance
  MVEXPORT void addNewBranch(const char *nameOfNew, int position, 
			                       MvrTaskState::State *state = NULL);
  /// Adds a new leaf to this instance
  MVEXPORT void addNewLeaf(const char *nameOfNew, int position, 
                           MvrFunctor *functor, 
                           MvrTaskState::State *state = NULL);

  /// Gets the name of this task
  MVEXPORT std::string getName(void);

  /// Gets the functor this instance runs, if there is one
  MVEXPORT MvrFunctor *getFunctor(void);

  /// Sets the functor called to get the cycle warning time (should only be used from the robot)
  MVEXPORT void setWarningTimeCB(MvrRetFunctor<unsigned int> *functor);
  /// Gets the functor called to get the cycle warning time (should only be used from the robot)
  MVEXPORT MvrRetFunctor<unsigned int> *getWarningTimeCB(void);

  /// Sets the functor called to check if there should be a time warning this cycle (should only be used from the robot)
  MVEXPORT void setNoTimeWarningCB(MvrRetFunctor<bool> *functor);
  /// Gets the functor called to check if there should be a time warning this cycle (should only be used from the robot)
  MVEXPORT MvrRetFunctor<bool> *getNoTimeWarningCB(void);
  
  // removes this task from the map
  MVEXPORT void remove(MvrSyncTask * proc);

  // returns whether this node is deleting or not
  MVEXPORT bool isDeleting(void);
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