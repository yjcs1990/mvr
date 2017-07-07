#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrSyncTask.h"
#include "MvrLog.h"

/**
   New should never be called to create an MvrSyncTask except to create the 
   root node.  Read the detailed documentation of the class for details.
*/
MVREXPORT MvrSyncTask::MvrSyncTask(const char *name, MvrFunctor *functor,
				MvrTaskState::State *state, MvrSyncTask *parent)
{
  myName = name;
  myStatePointer = state;
  myFunctor = functor;
  myParent = parent;
  myIsDeleting = false;
  setState(MvrTaskState::INIT);
  if (myParent != NULL)
  {
    setWarningTimeCB(parent->getWarningTimeCB());
    setNoTimeWarningCB(parent->getNoTimeWarningCB());
  }
  else
  {
    setWarningTimeCB(NULL);
    setNoTimeWarningCB(NULL);
  }
}

/**
   If you delete the task it deletes everything in its list, so to 
   delete the whole tree just delete the top one... also note that if you
   delete a node, it will remove itself from its parents list.
*/
MVREXPORT MvrSyncTask::~MvrSyncTask()
{
  myIsDeleting = true;
  if (myParent != NULL && !myParent->isDeleting())
    myParent->remove(this);
  
  MvrUtil::deleteSetPairs(myMultiMap.begin(), myMultiMap.end());  
  myMultiMap.clear();
}

MVREXPORT MvrTaskState::State MvrSyncTask::getState(void)
{
  if (myStatePointer != NULL)
    return *myStatePointer;
  else
    return myState;
}

MVREXPORT void MvrSyncTask::setState(MvrTaskState::State state)
{
  if (myStatePointer != NULL)
    *myStatePointer = state;
  else
    myState = state;
}

MVREXPORT std::string MvrSyncTask::getName(void)
{
  return myName;
}


/**
   Finds a node below (or at) this level in the tree with the given functor
   @param functor The task functor pointer to search for. Must not be NULL.
   @return The task, if found.  If not found, NULL.
*/
MVREXPORT MvrSyncTask *MvrSyncTask::find(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  std::multimap<int, MvrSyncTask *>::iterator it;
  
  if (myFunctor == functor)
    return this;

  for (it = myMultiMap.begin(); it != myMultiMap.end(); ++it)
  {
    proc = (*it).second;
    if (proc->find(functor) != NULL)
      return proc;
  }
  return NULL;
  
}

/**
   Finds a node below (or at) this level in the tree with the given name
   @param name The name of the child we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
MVREXPORT MvrSyncTask *MvrSyncTask::find(const char *name)
{
  MvrSyncTask *proc;
  std::multimap<int, MvrSyncTask *>::iterator it;
  
  if (strcmp(myName.c_str(), name) == 0)
    return this;

  for (it = myMultiMap.begin(); it != myMultiMap.end(); ++it)
  {
    proc = (*it).second;
    if (proc->find(name) != NULL)
      return proc;
  }
  return NULL;
  
}

/**
   Finds a child of this node with the given name
   @param name The name of the child we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
MVREXPORT MvrSyncTask *MvrSyncTask::findNonRecursive(const char * name)
{
  MvrSyncTask *proc;
  std::multimap<int, MvrSyncTask *>::iterator it;
  
  for (it = myMultiMap.begin(); it != myMultiMap.end(); ++it)
  {
    proc = (*it).second;
    if (strcmp(proc->getName().c_str(), name) == 0)  
      return proc;
  }
  return NULL;
}

/**
   Finds a child of this node with the given functor
   @param functor the functor we are interested in finding
   @return The task, if found.  If not found, NULL.
*/
MVREXPORT MvrSyncTask *MvrSyncTask::findNonRecursive(MvrFunctor *functor)
{
  MvrSyncTask *proc;
  std::multimap<int, MvrSyncTask *>::iterator it;
  
  for (it = myMultiMap.begin(); it != myMultiMap.end(); ++it)
  {
    proc = (*it).second;
    if (proc->getFunctor() == functor)
      return proc;
  }
  return NULL;
}

/**
   Creates a new task with the given name and puts the task into its 
   own iternal list at the given position.  
   @param nameOfNew Name to give to the new task.
   @param position place in list to put the branch, things are run/printed in 
   the order of highest number to lowest number, no limit on numbers (other 
   than that it is an int).  ARIA uses 0 to 100 just as a convention.
   @param state Pointer to external vmvriable to store task state in, or NULL to use a new
   internal vmvriable instead.
*/
MVREXPORT void MvrSyncTask::addNewBranch(const char *nameOfNew, int position,
				       MvrTaskState::State *state)
{
  MvrSyncTask *proc = new MvrSyncTask(nameOfNew, NULL, state, this);
  myMultiMap.insert(std::pair<int, MvrSyncTask *>(position, proc));
}

/**
   Creates a new task with the given name and puts the task into its 
   own iternal list at the given position.  Sets the nodes functor so that
   it will call the functor when run is called.
   @param nameOfNew Name to give to the new task.
   @param position place in list to put the branch, things are run/printed in 
   the order of highest number to lowest number, no limit on numbers (other 
   than that it is an int).  ARIA uses 0 to 100 just as a convention.
   @param functor MvrFunctor which contains the functor to invoke when run is 
   called.
   @param state Pointer to external vmvriable to store task state in, or NULL to
   use an internal vmvriable instead.
*/
MVREXPORT void MvrSyncTask::addNewLeaf(const char *nameOfNew, int position, 
				     MvrFunctor *functor, 
				     MvrTaskState::State *state)
{
  MvrSyncTask *proc = new MvrSyncTask(nameOfNew, functor, state, this);
  myMultiMap.insert(std::pair<int, MvrSyncTask *>(position, proc));
}

MVREXPORT void MvrSyncTask::remove(MvrSyncTask *proc)
{
  std::multimap<int, MvrSyncTask *>::iterator it;
  
  for (it = myMultiMap.begin(); it != myMultiMap.end(); it++)
  {
    if ((*it).second == proc)
    {
      myMultiMap.erase(it);
      return;
    }
  }
}

MVREXPORT bool MvrSyncTask::isDeleting(void)
{
  return myIsDeleting;
}

MVREXPORT MvrFunctor *MvrSyncTask::getFunctor(void)
{
  return myFunctor;
}

/**
   If this node is a leaf it calls the functor for the node, if it is
   a branch it goes through all of the children in the order of
   highest position to lowest position and calls run on them.
**/
MVREXPORT void MvrSyncTask::run(void)
{
  myRunning = true;

  std::multimap<int, MvrSyncTask *>::reverse_iterator it;
  MvrTaskState::State state;
  MvrTime runTime;
  int took;  

  state = getState();
  switch (state) 
  {
  case MvrTaskState::SUSPEND:
  case MvrTaskState::SUCCESS:
  case MvrTaskState::FAILURE:
    // The task isn't running so just return  
    myRunning = true;
    return;
  case MvrTaskState::INIT:
  case MvrTaskState::RESUME:
  case MvrTaskState::ACTIVE:
  default:
    break;
  }
  
  runTime.setToNow();
  if (myFunctor != NULL)
    myFunctor->invoke();
  
  if (myNoTimeWarningCB != NULL && !myNoTimeWarningCB->invokeR() && 
      myFunctor != NULL && myWarningTimeCB != NULL &&
      myWarningTimeCB->invokeR() > 0 && 
      (took = runTime.mSecSince()) > (signed int)myWarningTimeCB->invokeR())
    MvrLog::log(MvrLog::Normal, 
	       "Warning: Task '%s' took %d ms to run (longer than the %d warning time)",
	       myName.c_str(), took, (signed int)myWarningTimeCB->invokeR());
  
  
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
  {
    myInvokingOtherFunctor = (*it).second;
    myInvokingOtherFunctor->run();
  }
  myInvokingOtherFunctor = NULL;
}

/**
   This sets a functor which will be called to find the time on the
   task such that if it takes longer than this number of ms to run a
   warning message will be issued, sets this on the children too.
**/
MVREXPORT void MvrSyncTask::setWarningTimeCB(MvrRetFunctor<unsigned int> *functor)
{
  std::multimap<int, MvrSyncTask *>::reverse_iterator it;
  myWarningTimeCB = functor;
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->setWarningTimeCB(functor);
}

/**
   This gets a functor which will be called to find the time on the
   task such that if it takes longer than this number of ms to run a
   warning message will be issued, sets this on the children too.
**/
MVREXPORT MvrRetFunctor<unsigned int> *MvrSyncTask::getWarningTimeCB(void)
{
  return myWarningTimeCB;
}

/**
   This sets a functor which will be called to see if we should warn
   this time through or not.
**/
MVREXPORT void MvrSyncTask::setNoTimeWarningCB(MvrRetFunctor<bool> *functor)
{
  std::multimap<int, MvrSyncTask *>::reverse_iterator it;
  myNoTimeWarningCB = functor;
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->setNoTimeWarningCB(functor);
}

/**
   This sets a functor which will be called to see if we should warn
   this time through or not.
**/
MVREXPORT MvrRetFunctor<bool> *MvrSyncTask::getNoTimeWarningCB(void)
{
  return myNoTimeWarningCB;
}


/**
   Prints the node... the defaulted depth parameter controls how far over to 
   print the data (how many tabs)... it recurses down all its children.
*/
MVREXPORT void MvrSyncTask::log(int depth)
{
  int i;
  std::multimap<int, MvrSyncTask *>::reverse_iterator it;
  std::string str = "";
  MvrTaskState::State state;
  
  for (i = 0; i < depth; i++)
    str += "\t";
  str += myName.c_str();
  str += " (";
  state = getState();
  switch (state) 
  {
  case MvrTaskState::INIT:
    str += "INIT, running)";
    break;
  case MvrTaskState::RESUME:
    str += "RESUME, running)";
    break;
  case MvrTaskState::ACTIVE:
    str += "ACTIVE, running)";
    break;
  case MvrTaskState::SUSPEND:
    str += "SUSPEND, NOT running)";
    break;
  case MvrTaskState::SUCCESS:
    str += "SUCCESS, NOT running)";
    break;
  case MvrTaskState::FAILURE:
    str += "FAILURE, NOT running)";
    break;
  default:
    str += state;
    str += ", running)";
    break;
  }
  MvrLog::log(MvrLog::Terse, const_cast<char *>(str.c_str()));
  for (it = myMultiMap.rbegin(); it != myMultiMap.rend(); it++)
    (*it).second->log(depth + 1);
  
}


/// Returns what this is running, if anything (recurses)
MVREXPORT MvrSyncTask *MvrSyncTask::getRunning(void)
{
  if (!myRunning)
    return NULL;
  else if (myInvokingOtherFunctor != NULL)
    return myInvokingOtherFunctor->getRunning();
  else if (myFunctor)
    return this;
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSyncTask::getRunning: Tried to get running, but apparently nothing was running");
    return NULL;
  }
}
