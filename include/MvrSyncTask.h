#ifndef MVRSYNCTASK_H
#define MVRSYNCTASK_H

#include <string>
#include <map>
#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "MvrTaskState.h"

/// Class used internally to manage the tasks that are called every cycle
/**
   This is used internally, no user should normally have to create one, but 
   serious developers may want to use the members.  Most users will be able to 
   add user tasks via the MvrRobot class.

   The way it works is that each instance is a node in a tree.  The only node 
   that should ever be created with a new is the top one.  The run and print
   functions both call the run/print on themselves, then on all of their 
   children, going from lowest numbered position to highest numbered, lower
   going first.  There are no hard limits to the position, it can be any 
   integer.  ARIA uses the convention of 0 to 100, when you add things of
   your own you should leave room to add in between.  Also you can add things
   with the same position, the only effect this has is that the first addition
   will show up first in the run or print.

   After the top one is created, every other task should be created with 
   either addNewBranch() or addNewLeaf().  Each node can either be a branch node 
   or a list node.  The list (a multimap) of branches/nodes is ordered
   by the position passed in to the add function.  addNewBranch() adds a new 
   branch node to the instance it is called on, with the given name and
   position.  addNewLeaf() adds a new leaf node to the instance it is called on,
   with the given name and position, and also with the MvrFunctor given, this 
   functor will be called when the leaf is run.  Either add creates the new 
   instance and puts it in the list of branches/nodes in the approriate spot.

   The tree takes care of all of its own memory management and list management,
   the "add" functions put into the list and creates the memory, conversely
   if you delete an MvrSyncTask (which is the correct way to get rid of one)
   it will remove itself from its parents list.

   If you want to add something to the tree the proper way to do it is to get
   the pointer to the root of the tree (ie with MvrRobot::getSyncProcRoot) and
   then to use find on the root to find the branch you want to travel down,
   then continue this until you find the node you want to add to.  Once there
   just call addNewBranch or addNewLeaf and you're done.

   The state of a task can be stored in the target of a given MvrTaskState::State pointer,
   or if NULL than MvrSyncTask will use its own member vmvriable.

  @internal
*/

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
  MVREXPORT void setWarningTimeCB(
	  MvrRetFunctor<unsigned int> *functor);
  /// Gets the functor called to get the cycle warning time (should only be used from the robot)
  MVREXPORT MvrRetFunctor<unsigned int> *getWarningTimeCB(void);

  /// Sets the functor called to check if there should be a time warning this cycle (should only be used from the robot)
  MVREXPORT void setNoTimeWarningCB(
	  MvrRetFunctor<bool> *functor);
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
  // vmvriables for introspection
  bool myRunning;
  // this is just a pointer to what we're invoking so we can know later
  MvrSyncTask *myInvokingOtherFunctor;
};



#endif




