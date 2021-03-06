#include "mvriaOSDef.h"
#include "MvrExport.h"
#include "MvrConfig.h"
#include "MvrRobotConfig.h"
#include "MvrRobot.h"
#include "MvrAnalogGyro.h"
#include "mvriaInternal.h"

MVREXPORT MvrRobotConfig::MvrRobotConfig(MvrRobot *robot) : 
  myConnectCB(this, &MvrRobotConfig::connectCallback),
  myProcessFileCB(this, &MvrRobotConfig::processFile)
{
  myRobot = robot;

  myAddedMovementParams = false;
  myTransVelMax = 0;
  myTransAccel = 0;
  myTransDecel = 0;
  myRotVelMax = 0;
  myRotAccel= 0;
  myRotDecel = 0;

  mySavedOriginalMovementParameters = false;
  myOriginalTransVelMax = 0;
  myOriginalTransAccel = 0;
  myOriginalTransDecel = 0;
  myOriginalRotVelMax = 0;
  myOriginalRotAccel= 0;
  myOriginalRotDecel = 0;


  myAnalogGyro = NULL;
  myUseGyro = true;
  myAddedGyro = false;

  myConnectCB.setName("MvrRobotConfig");
  myProcessFileCB.setName("MvrRobotConfig");

  myRobot->addConnectCB(&myConnectCB, MvrListPos::FIRST);
  Mvria::getConfig()->addProcessFileCB(&myProcessFileCB, 98);

  if (myRobot->isConnected())
    connectCallback();
}

MVREXPORT MvrRobotConfig::~MvrRobotConfig()
{
}

MVREXPORT bool MvrRobotConfig::processFile(void)
{
    if (myTransVelMax != 0)
    myRobot->setTransVelMax(myTransVelMax);
  else
    myRobot->setTransVelMax(myOriginalTransVelMax);

  if (myTransAccel != 0)
    myRobot->setTransAccel(myTransAccel);
  else
    myRobot->setTransAccel(myOriginalTransAccel);

  if (myTransDecel != 0)
    myRobot->setTransDecel(myTransDecel);
  else
    myRobot->setTransDecel(myOriginalTransDecel);

  if (myRotVelMax != 0)
    myRobot->setRotVelMax(myRotVelMax);
  else
    myRobot->setRotVelMax(myOriginalRotVelMax);

  if (myRotAccel!= 0)
    myRobot->setRotAccel(myRotAccel);
  else
    myRobot->setRotAccel(myOriginalRotAccel);

  if (myRotDecel != 0)
    myRobot->setRotDecel(myRotDecel);
  else
    myRobot->setRotDecel(myOriginalRotDecel);

  if (myAnalogGyro != NULL && myAddedGyro)
  {
    if (myUseGyro && !myAnalogGyro->isActive())
    {
      MvrLog::log(MvrLog::Normal, "Gyro(analog) activated");
      myAnalogGyro->activate();
    }
    else if (!myUseGyro && myAnalogGyro->isActive())
    {
      MvrLog::log(MvrLog::Normal, "Gyro(analog) deactivated");
      myAnalogGyro->deactivate();
    }
  }
  
  return true;
}

MVREXPORT void MvrRobotConfig::addAnalogGyro(MvrAnalogGyro *gyro)
{
  myAnalogGyro = gyro;
  if (myRobot->isConnected())
    connectCallback();
}

MVREXPORT void MvrRobotConfig::connectCallback(void)
{
  std::string section;
  section = "Robot config";

  if (!mySavedOriginalMovementParameters)
  {
    mySavedOriginalMovementParameters = true;
    myOriginalTransVelMax = MvrMath::roundInt(myRobot->getTransVelMax());
    myOriginalTransAccel = MvrMath::roundInt(myRobot->getTransAccel());
    myOriginalTransDecel = MvrMath::roundInt(myRobot->getTransDecel());
    myOriginalRotVelMax = MvrMath::roundInt(myRobot->getRotVelMax());
    myOriginalRotAccel = MvrMath::roundInt(myRobot->getRotAccel());
    myOriginalRotDecel = MvrMath::roundInt(myRobot->getRotDecel());
  }

  if (!myAddedMovementParams)
  {
    myAddedMovementParams = true;
    Mvria::getConfig()->addParam(
	    MvrConfigArg("TransVelMax", &myTransVelMax, 
			"maximum translational speed (mm/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransVel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
    Mvria::getConfig()->addParam(
	    MvrConfigArg("TransAccel", &myTransAccel, 
			"translational acceleration (mm/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransAccel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
    Mvria::getConfig()->addParam(
	    MvrConfigArg("TransDecel", &myTransDecel, 
			"translational deceleration (mm/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransDecel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
    Mvria::getConfig()->addParam(
	    MvrConfigArg("RotVelMax", &myRotVelMax, 
			"maximum rotational speed (deg/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotVel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
    Mvria::getConfig()->addParam(
	    MvrConfigArg("RotAccel", &myRotAccel, 
			"rotational acceleration (deg/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotAccel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
    Mvria::getConfig()->addParam(
	    MvrConfigArg("RotDecel", &myRotDecel, 
			"rotational deceleration (deg/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotDecel()), 
	    section.c_str(), MvrPriority::TRIVIAL);
  }

  if (myAnalogGyro != NULL && !myAddedGyro && 
      myAnalogGyro->haveGottenData())
  {
    myAddedGyro = true;
    Mvria::getConfig()->addParam(
	    MvrConfigArg("UseGyro", &myUseGyro, 
			"True to use the gyro, false not to"),
	    section.c_str(), MvrPriority::TRIVIAL);

  }

}
