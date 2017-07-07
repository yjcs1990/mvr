#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionLimiterRot.h"
#include "MvrRobot.h"
#include "MvrCommands.h"
#include "mvriaInternal.h"
#include "MvrRobotConfigPacketReader.h"
#include "MvrRangeDevice.h"

/**
   @param name name of the action
*/
MVREXPORT MvrActionLimiterRot::MvrActionLimiterRot(
	const char *name) :
  MvrAction(name,
      "Applies a limit to rotation if there are sensor readings in the radius")
{
  setParameters();

  myUseLocationDependentDevices = true;
}

MVREXPORT MvrActionLimiterRot::~MvrActionLimiterRot()
{

}

/**
   @param checkRadius True to check the radius and apply this speed, false not to
   @param inRadiusSpeed the speed to go at if something is in the radius
   
*/
MVREXPORT void MvrActionLimiterRot::setParameters(
	bool checkRadius, double inRadiusSpeed)
{
  myCheckRadius = checkRadius;
  myInRadiusSpeed = inRadiusSpeed;
}

MVREXPORT void MvrActionLimiterRot::addToConfig(MvrConfig *config, 
						       const char *section, 
						       const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::NORMAL);  

  name = strPrefix;
  name += "CheckRadius";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myCheckRadius, 
		      "True to check the robot's radius and apply InRadiusSpeed, false not to"), 
	  section, MvrPriority::NORMAL);

  name = strPrefix;
  name += "InRadiusSpeed";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myInRadiusSpeed, 
		      "Maximum speed to allow if CheckRadius is true and there are sensor readings in the robot's radius, 0 or less means allow no rotation (deg/sec)"), 
	  section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::NORMAL);
}

MVREXPORT MvrActionDesired *
MvrActionLimiterRot::fire(MvrActionDesired currentDesired)
{
  bool printing = false;


  if (!myCheckRadius)
  {
    if (printing)
      printf("Nothing\n");
    return NULL;
  }

  double leftDist;
  const MvrRangeDevice *leftRangeDevice = NULL;
  double rightDist;
  const MvrRangeDevice *rightRangeDevice = NULL;

  double dist;
  //const MvrRangeDevice *rangeDevice = NULL;

  //MvrLog::LogLevel verboseLogLevel = MvrLog::Verbose;
  //MvrLog::LogLevel verboseLogLevel = MvrLog::Verbose;
  //if (printing)
  //  verboseLogLevel = MvrLog::Normal;


  leftDist = myRobot->checkRangeDevicesCurrentPolar(
	  0, 179.999, NULL, &leftRangeDevice, myUseLocationDependentDevices);

  rightDist = myRobot->checkRangeDevicesCurrentPolar(
	  -179.999, 0, NULL, &rightRangeDevice, myUseLocationDependentDevices);
  
  if (leftDist > 0 && rightDist < 0)
  {
    dist = leftDist;
    //rangeDevice = leftRangeDevice;
  }
  else if (rightDist > 0 && leftDist < 0)
  {
    dist = rightDist;
    //rangeDevice = rightRangeDevice;
  }
  else if (leftDist > 0 && rightDist > 0)
  {
    if (leftDist < rightDist)
    {
      dist = leftDist;
      //rangeDevice = leftRangeDevice;
    }
    else 
    {
      dist = rightDist;
      //rangeDevice = rightRangeDevice;
    }
  }
  else
  {
    dist = -1;
    //rangeDevice = NULL;
  }

  if (printing)
    printf("left %.0f right %.0f dist %.0f robotRadius %.0f\n", 
	   leftDist, rightDist, dist, myRobot->getRobotRadius());

  if (dist > 0 && dist < myRobot->getRobotRadius())
  {
    if (myInRadiusSpeed <= 0)
    {
      if (printing)
	printf("rot of zero\n");
      myDesired.setMaxRotVel(0);
    }
    else
    {
      if (printing)
	printf("rot of %.0f\n", myInRadiusSpeed);
      myDesired.setMaxRotVel(myInRadiusSpeed);
    }
    return &myDesired;
  }
  else
  {
    if (printing)
      printf("Nothing at the end\n");
    return NULL;
  }
}



