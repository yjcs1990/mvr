#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRatioInputKeydrive.h"
#include "MvrRobot.h"
#include "mvriaInternal.h"
#include "MvrKeyHandler.h"

MVREXPORT MvrRatioInputKeydrive::MvrRatioInputKeydrive(MvrRobot *robot, 
						    MvrActionRatioInput *input,
						    int priority,
						    double velIncrement) :
  myUpCB(this, &MvrRatioInputKeydrive::up),
  myDownCB(this, &MvrRatioInputKeydrive::down),
  myLeftCB(this, &MvrRatioInputKeydrive::left),
  myRightCB(this, &MvrRatioInputKeydrive::right),
  myZCB(this, &MvrRatioInputKeydrive::z),
  myXCB(this, &MvrRatioInputKeydrive::x),
  mySpaceCB(this, &MvrRatioInputKeydrive::space),
  myFireCB(this, &MvrRatioInputKeydrive::fireCallback),
  myActivateCB(this, &MvrRatioInputKeydrive::activate),
  myDeactivateCB(this, &MvrRatioInputKeydrive::deactivate)
{
  myRobot = robot;
  myInput = input;
  myInput->addFireCallback(priority, &myFireCB);
  myInput->addActivateCallback(&myActivateCB);
  myInput->addDeactivateCallback(&myDeactivateCB);
  myFireCB.setName("Keydrive");
  myVelIncrement = velIncrement;
  myLatVelIncrement = velIncrement;
  myHaveKeys = false;
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  myThrottle = 100;
  myPrinting = false;
}

MVREXPORT MvrRatioInputKeydrive::~MvrRatioInputKeydrive()
{
  myInput->remFireCallback(&myFireCB);
  myInput->remActivateCallback(&myActivateCB);
}


MVREXPORT void MvrRatioInputKeydrive::takeKeys(void)
{
  myHaveKeys = true;
  MvrKeyHandler *keyHandler;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrRatioInputKeydrive::takeKeys: There is no key handler, keydrive will not work.");
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
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->addKeyHandler('z', &myZCB))
      MvrLog::log(MvrLog::Terse,  
		 "The key handler already has a key for z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('Z', &myZCB))
      MvrLog::log(MvrLog::Terse,  
		 "The key handler already has a key for Z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('x', &myXCB))
      MvrLog::log(MvrLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('X', &myXCB))
      MvrLog::log(MvrLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
  }
}

MVREXPORT void MvrRatioInputKeydrive::giveUpKeys(void)
{
  MvrKeyHandler *keyHandler;
  myHaveKeys = false;
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrRatioInputKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    MvrLog::log(MvrLog::Terse, "MvrRatioInputKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    MvrLog::log(MvrLog::Terse, "MvrRatioInputKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrRatioInputKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrRatioInputKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    MvrLog::log(MvrLog::Terse,  
	       "MvrRatioInputKeydrive: The key handler didn't have a key for space, something is wrong.");
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->remKeyHandler(&myZCB))
      MvrLog::log(MvrLog::Terse,  
		 "MvrRatioInputKeydrive: The key handler didn't have a key for z, something is wrong.");
    if (!keyHandler->remKeyHandler(&myXCB))
      MvrLog::log(MvrLog::Terse,  
		 "MvrRatioInputKeydrive: The key handler didn't have a key for x, something is wrong.");
  }

}


MVREXPORT void MvrRatioInputKeydrive::up(void)
{
  if (myPrinting)
    printf("up\n");
  myTransRatio += myVelIncrement;
  if (myTransRatio > 100)
    myTransRatio = 100;
}

MVREXPORT void MvrRatioInputKeydrive::down(void)
{
  if (myPrinting)
    printf("down\n");
  myTransRatio -= myVelIncrement;
  if (myTransRatio < -100)
    myTransRatio = -100;
}

MVREXPORT void MvrRatioInputKeydrive::left(void)
{
  if (myPrinting)
    printf("left\n");
  myRotRatio = 100;
}

MVREXPORT void MvrRatioInputKeydrive::right(void)
{
  if (myPrinting)
    printf("right\n");
  myRotRatio = -100;
}

MVREXPORT void MvrRatioInputKeydrive::space(void)
{
  if (myPrinting)
    printf("stop\n");
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
}

MVREXPORT void MvrRatioInputKeydrive::z(void)
{
  if (myPrinting)
    printf("up\n");
  myLatRatio += myLatVelIncrement;
  if (myLatRatio > 100)
    myLatRatio = 100;
}

MVREXPORT void MvrRatioInputKeydrive::x(void)
{
  if (myPrinting)
    printf("down\n");
  myLatRatio -= myLatVelIncrement;
  if (myLatRatio < -100)
    myLatRatio = -100;
}

MVREXPORT void MvrRatioInputKeydrive::activate(void)
{
  // set things so we'll stop
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  if (myHaveKeys)
    takeKeys();
}

MVREXPORT void MvrRatioInputKeydrive::deactivate(void)
{
  if (myHaveKeys)
    giveUpKeys();
}

MVREXPORT void MvrRatioInputKeydrive::fireCallback(void)
{
  // set what we want to do
  myInput->setTransRatio(myTransRatio);
  myInput->setRotRatio(myRotRatio);
  myInput->setLatRatio(myLatRatio);
  myInput->setThrottleRatio(myThrottle);

  // reset us to going straight (if they're holding the key we'll keep turning)
  myRotRatio = 0;

  if (myHaveKeys)
    return;
  MvrKeyHandler *keyHandler;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Mvria::getKeyHandler()) == NULL)
  {
    keyHandler = new MvrKeyHandler;
    Mvria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}
