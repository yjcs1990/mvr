#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionMovementParametersDebugging.h"
#include "MvrConfig.h"
#include "MvrRobot.h"

/**
   @param name the name of this instance of the action
 **/
MVREXPORT MvrActionMovementParametersDebugging::MvrActionMovementParametersDebugging(
	const char *name) :
  MvrAction(name, "Sets all the max vel and accels/decels in a way appropriate for debugging")
{
  myEnabled = true;
  myEnableOnce = false;

  mySetMaxVel = false;
  myMaxVel = 2000;
  mySetMaxNegVel = false;
  myMaxNegVel = 1000;
  mySetTransAccel = false;
  myTransAccel = 500;
  mySetTransDecel = false;
  myTransDecel = 500;
  mySetMaxRotVel = false;
  myMaxRotVel = 250;
  mySetRotAccel = false;
  myRotAccel = 250;
  mySetRotDecel = false;
  myRotDecel = 250;
  mySetMaxLeftLatVel = false;
  myMaxLeftLatVel = 2000;
  mySetMaxRightLatVel = false;
  myMaxRightLatVel = 2000;
  mySetLatAccel = false;
  myLatAccel = 500;
  mySetLatDecel = false;
  myLatDecel = 500;
}

MVREXPORT MvrActionMovementParametersDebugging::~MvrActionMovementParametersDebugging()
{

}

MVREXPORT void MvrActionMovementParametersDebugging::addToConfig(
	MvrConfig *config, const char *section, const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetTransVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetMaxVel, 
		      "True to set the transVelMax"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "TransVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxVel, 
		      "Maximum forward translational velocity (0 means use default)"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetTransNegVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetMaxNegVel, 
		      "True to set the transNegVelMax"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "TransNegVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxNegVel, 
		      "Maximum backwards translational velocity"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetTransAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetTransAccel, 
		      "True to set the transAccel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "TransAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myTransAccel, 
		      "Translational acceleration"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetTransDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetTransDecel, 
		      "True to set the transDecel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "TransDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myTransDecel, 
		      "Translational deceleration"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetRotVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetMaxRotVel, 
		      "True to set the rotVelMax"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "RotVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxRotVel, 
		      "Maximum rotational velocity"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetRotAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetRotAccel, 
		      "True to set rotAccel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "RotAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myRotAccel, 
		      "Rotational acceleration"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetRotDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetRotDecel, 
		      "True to set rotDecel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "RotDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myRotDecel, 
		      "Rotational deceleration"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetLeftLatVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetMaxLeftLatVel, 
		      "True to set leftLatVelMax"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "LeftLatVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxLeftLatVel, 
		      "Maximum left lateral velocity"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "SetRightLatVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetMaxRightLatVel, 
		      "True to set rightLatVelMax"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "RightLatVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxRightLatVel, 
		      "Maximum right lateral velocity"),
	  section, MvrPriority::EXPERT);
  
  name = strPrefix;
  name += "SetLatAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetLatAccel, 
		      "True to set latAccel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "LatAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myLatAccel, 
		      "Lateral acceleration"),
	  section, MvrPriority::EXPERT);
  
  name = strPrefix;
  name += "SetLatDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &mySetLatDecel, 
		      "True to set latDecel"),
	  section, MvrPriority::EXPERT);

  name = strPrefix;
  name += "LatDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myLatDecel, 
		      "Lateral deceleration"),
	  section, MvrPriority::EXPERT);
  
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::EXPERT);


}

MVREXPORT MvrActionDesired *MvrActionMovementParametersDebugging::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  if (!myEnabled && !myEnableOnce)
    return NULL;
  myEnableOnce = false;

  if (mySetMaxVel)
    myDesired.setMaxVel(myMaxVel);

  if (mySetMaxNegVel)
    myDesired.setMaxNegVel(myMaxNegVel);

  if (mySetTransAccel)
    myDesired.setTransAccel(myTransAccel);

  if (mySetTransDecel)
    myDesired.setTransDecel(myTransDecel);

  if (mySetMaxRotVel)
    myDesired.setMaxRotVel(myMaxRotVel);

  if (mySetRotAccel)
    myDesired.setRotAccel(myRotAccel);

  if (mySetRotDecel)
    myDesired.setRotDecel(myRotDecel);

  if (mySetMaxLeftLatVel)
    myDesired.setMaxLeftLatVel(myMaxLeftLatVel);

  if (mySetMaxRightLatVel)
    myDesired.setMaxRightLatVel(myMaxRightLatVel);

  if (mySetLatAccel)
    myDesired.setLatAccel(myLatAccel);

  if (mySetLatDecel)
    myDesired.setLatDecel(myLatDecel);
  
  return &myDesired;
}
