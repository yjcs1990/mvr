#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrMode.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"

MvrMode *MvrMode::ourActiveMode = NULL;
MvrGlobalFunctor *MvrMode::ourHelpCB = NULL;
std::list<MvrMode *> MvrMode::ourModes;

/**
   @param robot the robot we're attaching to
   
   @param name the name of this mode

   @param key the primary key to switch to this mode on... it can be
   '\\0' if you don't want to use this

   @param key2 an alternative key to switch to this mode on... it can be
   '\\0' if you don't want a second alternative key
**/
MVREXPORT MvrMode::MvrMode(MvrRobot *robot, const char *name, char key, 
			char key2) :
  myActivateCB(this, &MvrMode::activate),
  myDeactivateCB(this, &MvrMode::deactivate),
  myUserTaskCB(this, &MvrMode::userTask)
{
  MvrKeyHandler *keyHandler;
  myName = name;
  myRobot = robot;
  myKey = key;
  myKey2 = key2;
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    keyHandler = new MvrKeyHandler;
    Mvria::setKeyHandler(keyHandler);
    if (myRobot != NULL)
      myRobot->attachKeyHandler(keyHandler);
    else
      MvrLog::log(MvrLog::Terse, "MvrMode: No robot to attach a keyHandler to, keyHandling won't work... either make your own keyHandler and drive it yourself, make a keyhandler and attach it to a robot, or give this a robot to attach to.");
  }  
  if (ourHelpCB == NULL)
  {
    ourHelpCB = new MvrGlobalFunctor(&MvrMode::baseHelp);
    if (!keyHandler->addKeyHandler('h', ourHelpCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for 'h', MvrMode will not be invoked on an 'h' keypress.");
    if (!keyHandler->addKeyHandler('H', ourHelpCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for 'H', MvrMode will not be invoked on an 'H' keypress.");
    if (!keyHandler->addKeyHandler('?', ourHelpCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for '?', MvrMode will not be invoked on an '?' keypress.");
    if (!keyHandler->addKeyHandler('/', ourHelpCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for '/', MvrMode will not be invoked on an '/' keypress.");

  }

  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (myKey != '\0')
    if (!keyHandler->addKeyHandler(myKey, &myActivateCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for '%c', MvrMode will not work correctly.", myKey);
  if (myKey2 != '\0')
    if (!keyHandler->addKeyHandler(myKey2, &myActivateCB))
      MvrLog::log(MvrLog::Terse, "The key handler already has a key for '%c', MvrMode will not work correctly.", myKey2);

  // toss this mode into our list of modes
  ourModes.push_front(this);
}

MVREXPORT MvrMode::~MvrMode()
{
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) != NULL)
  {
    if (myKey != '\0')
      keyHandler->remKeyHandler(myKey);
    if (myKey2 != '\0')
      keyHandler->remKeyHandler(myKey2);
  }
  if (myRobot != NULL)
    myRobot->remUserTask(&myUserTaskCB);
}

/** 
   Inheriting modes must first call this to get their user task called
   and to deactivate the active mode.... if it returns false then the
   inheriting class must return, as it means that his mode is already
   active
**/
MVREXPORT bool MvrMode::baseActivate(void)
{
  if (ourActiveMode == this)
    return false;
  myRobot->deactivateActions();
  if (myRobot != NULL)
  {
    myRobot->addUserTask(myName.c_str(), 50, &myUserTaskCB);
  }
  if (ourActiveMode != NULL)
    ourActiveMode->deactivate();
  ourActiveMode = this;
  if (myRobot != NULL)
  {
    myRobot->stop();
    myRobot->clearDirectMotion();
  }
  
  baseHelp();
  return true;
}

/**
   This gets called when the mode is deactivated, it removes the user
   task from the robot
**/
MVREXPORT bool MvrMode::baseDeactivate(void)
{
  if (myRobot != NULL)
    myRobot->remUserTask(&myUserTaskCB);
  if (ourActiveMode == this)
  {
    ourActiveMode = NULL;
    return true;
  }
  return false;
}

MVREXPORT const char *MvrMode::getName(void)
{
  return myName.c_str();
}

MVREXPORT char MvrMode::getKey(void)
{
  return myKey;
}

MVREXPORT char MvrMode::getKey2(void)
{
  return myKey2;
}

MVREXPORT void MvrMode::baseHelp(void)
{
  std::list<MvrMode *>::iterator it;
  MvrLog::log(MvrLog::Terse, "\n\nYou can do these actions with these keys:\n");
  MvrLog::log(MvrLog::Terse, "quit: escape");
  MvrLog::log(MvrLog::Terse, "help: 'h' or 'H' or '?' or '/'");
  MvrLog::log(MvrLog::Terse, "\nYou can switch to other modes with these keys:");
  for (it = ourModes.begin(); it != ourModes.end(); ++it)
  {
    MvrLog::log(MvrLog::Terse, "%30s mode: '%c' or '%c'", (*it)->getName(), 
	       (*it)->getKey(), (*it)->getKey2());
  }
  if (ourActiveMode == NULL)
    MvrLog::log(MvrLog::Terse, "You are in no mode currently.");
  else
  {
    MvrLog::log(MvrLog::Terse, "You are in '%s' mode currently.\n",
	       ourActiveMode->getName());
    ourActiveMode->help();
  }
}

MVREXPORT void MvrMode::addKeyHandler(int keyToHandle, MvrFunctor *functor)
{
  MvrKeyHandler *keyHandler;
  std::string charStr;

  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse,"MvrMode '%s'::keyHandler: There should already be a key handler, but there isn't... mode won't work right.", getName());
    return;
  }

  if (!keyHandler->addKeyHandler(keyToHandle, functor))
  {
    bool specialKey = true;
    switch (keyToHandle) {
    case MvrKeyHandler::UP:
      charStr = "Up";
      break;
    case MvrKeyHandler::DOWN:
      charStr = "Down";
      break;
    case MvrKeyHandler::LEFT:
      charStr = "Left";
      break;
    case MvrKeyHandler::RIGHT:
      charStr = "Right";
      break;
    case MvrKeyHandler::ESCAPE:
      charStr = "Escape";
      break;
    case MvrKeyHandler::F1:
      charStr = "F1";
      break;
    case MvrKeyHandler::F2:
      charStr = "F2";
      break;
    case MvrKeyHandler::F3:
      charStr = "F3";
      break;
    case MvrKeyHandler::F4:
      charStr = "F4";
      break;
    case MvrKeyHandler::SPACE:
      charStr = "Space";
      break;
    case MvrKeyHandler::TAB:
      charStr = "Tab";
      break;
    case MvrKeyHandler::ENTER:
      charStr = "Enter";
      break;
    case MvrKeyHandler::BACKSPACE:
      charStr = "Backspace";
      break;
    default:
      charStr = (char)keyToHandle;
      specialKey = false;
      break;
    }
    if (specialKey || (keyToHandle >= '!' && keyToHandle <= '~'))
      MvrLog::log(MvrLog::Terse,  
		 "MvrMode '%s': The key handler has a duplicate key for '%s' so the mode may not work right.", getName(), charStr.c_str());
    else
      MvrLog::log(MvrLog::Terse,  
		 "MvrMode '%s': The key handler has a duplicate key for number %d so the mode may not work right.", getName(), keyToHandle);
  }
  
}

MVREXPORT void MvrMode::remKeyHandler(MvrFunctor *functor)
{
  MvrKeyHandler *keyHandler;
  std::string charStr;

  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse,"MvrMode '%s'::keyHandler: There should already be a key handler, but there isn't... mode won't work right.", getName());
    return;
  }
  if (!keyHandler->remKeyHandler(functor))
    MvrLog::log(MvrLog::Terse,  
	       "MvrMode '%s': The key handler already didn't have the given functor so the mode may not be working right.", getName());
}
  
