#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionKeydrive.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"
#include "MvrKeyHandler.h"

MVREXPORT MvrActionKeydrive::MvrActionKeydrive(const char *name,
					    double transVelMax,
					    double turnAmountMax,
					    double velIncrement,
					    double turnIncrement)
  :
  MvrAction(name, "This action reads the keyboard arrow keys and sets the translational and rotational velocities based on this."),
  myUpCB(this, &MvrActionKeydrive::up),
  myDownCB(this, &MvrActionKeydrive::down),
  myLeftCB(this, &MvrActionKeydrive::left),
  myRightCB(this, &MvrActionKeydrive::right),
  mySpaceCB(this, &MvrActionKeydrive::space)
{
  setNextArgument(MvrArg("trans vel max", &myTransVelMax, "The maximum speed to go (mm/sec)"));
  myTransVelMax = transVelMax;

  setNextArgument(MvrArg("turn amount max", &myTurnAmountMax, "The maximum amount to turn (deg/cycle)"));
  myTurnAmountMax = turnAmountMax;

  setNextArgument(MvrArg("vel increment per keypress", &myVelIncrement, "The amount to increment velocity by per keypress (mm/sec)"));
  myVelIncrement = velIncrement;
  
  setNextArgument(MvrArg("turn increment per keypress", &myVelIncrement, "The amount to turn by per keypress (deg)"));
  myTurnIncrement = turnIncrement;

  myDesiredSpeed = 0;
  myDeltaVel = 0;
  myTurnAmount = 0;
  mySpeedReset = true;
}

MVREXPORT MvrActionKeydrive::~MvrActionKeydrive()
{

}

MVREXPORT void MvrActionKeydrive::setRobot(MvrRobot *robot)
{
  MvrKeyHandler *keyHandler;
  myRobot = robot;
  if (robot == NULL)
    return;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    keyHandler = new MvrKeyHandler;
    Mvria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}

MVREXPORT void MvrActionKeydrive::takeKeys(void)
{
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrActionKeydrive::takeKeys: There is no key handler, keydrive will not work.");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->addKeyHandler(MvrKeyHandler::UP, &myUpCB))
    MvrLog::log(MvrLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::DOWN, &myDownCB))
    MvrLog::log(MvrLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::LEFT, &myLeftCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::RIGHT, &myRightCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for right, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(MvrKeyHandler::SPACE, &mySpaceCB))
    MvrLog::log(MvrLog::Terse,  
	       "The key handler already has a key for space, keydrive will not work correctly.");
}

MVREXPORT void MvrActionKeydrive::giveUpKeys(void)
{
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrActionKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    MvrLog::log(MvrLog::Terse, "MvrActionKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    MvrLog::log(MvrLog::Terse, "MvrActionKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrActionKeydrive: The key handler didn't have a key for space, something is wrong.");
}

MVREXPORT void MvrActionKeydrive::setSpeeds(double transVelMax, 
					  double turnAmountMax)
{
  myTransVelMax = transVelMax;
  myTurnAmountMax = turnAmountMax;
}

MVREXPORT void MvrActionKeydrive::setIncrements(double velIncrement, 
					      double turnIncrement)
{
  myVelIncrement = velIncrement;
  myTurnIncrement = turnIncrement;
}

MVREXPORT void MvrActionKeydrive::up(void)
{
  myDeltaVel += myVelIncrement;
}

MVREXPORT void MvrActionKeydrive::down(void)
{
  myDeltaVel -= myVelIncrement;
}

MVREXPORT void MvrActionKeydrive::left(void)
{
  myTurnAmount += myTurnIncrement;
  if (myTurnAmount > myTurnAmountMax)
    myTurnAmount = myTurnAmountMax;
}

MVREXPORT void MvrActionKeydrive::right(void)
{
  myTurnAmount -= myTurnIncrement;
  if (myTurnAmount < -myTurnAmountMax)
    myTurnAmount = -myTurnAmountMax;
}

MVREXPORT void MvrActionKeydrive::space(void)
{
  mySpeedReset = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

MVREXPORT void MvrActionKeydrive::activate(void)
{
  if (!myIsActive)
    takeKeys();
  myIsActive = true;
}

MVREXPORT void MvrActionKeydrive::deactivate(void)
{
  if (myIsActive)
    giveUpKeys();
  myIsActive = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

MVREXPORT MvrActionDesired *MvrActionKeydrive::fire(MvrActionDesired currentDesired)
{
  myDesired.reset();

  // if we don't have any strength left
  if (fabs(currentDesired.getVelStrength() - 1.0) < .0000000000001)
  {
    mySpeedReset = true;
  }

  // if our speed was reset, set our desired to how fast we're going now
  if (mySpeedReset && myDesiredSpeed > 0 && myDesiredSpeed > myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  if (mySpeedReset && myDesiredSpeed < 0 && myDesiredSpeed < myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  mySpeedReset = false;

  if (currentDesired.getMaxVelStrength() && 
      myDesiredSpeed > currentDesired.getMaxVel())
    myDesiredSpeed = currentDesired.getMaxVel();

  if (currentDesired.getMaxNegVelStrength() && 
      myDesiredSpeed < currentDesired.getMaxNegVel())
    myDesiredSpeed = currentDesired.getMaxNegVel();

  myDesiredSpeed += myDeltaVel;
  if (myDesiredSpeed > myTransVelMax)
    myDesiredSpeed = myTransVelMax;
  if (myDesiredSpeed < -myTransVelMax)
    myDesiredSpeed = -myTransVelMax;

  myDesired.setVel(myDesiredSpeed);
  myDeltaVel = 0;
  
  myDesired.setDeltaHeading(myTurnAmount);
  myTurnAmount = 0;

  
  return &myDesired;
}
