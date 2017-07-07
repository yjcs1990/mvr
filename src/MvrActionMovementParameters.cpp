#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionMovementParameters.h"
#include "MvrConfig.h"
#include "MvrRobot.h"

/**
   @param name the name of this instance of the action

   @param overrideFaster if this is set to true and a fast max vel is
   set and our max vel is lower we'll set the vel with all our
   strength (since hopefully that faster setting was set with minimum
   strength)

   @param addLatVelIfAvailable If true, include the LatVel parameters in the
   MvrConfig if the robot supports lateral motion.
 **/
MVREXPORT MvrActionMovementParameters::MvrActionMovementParameters(
	const char *name,
	bool overrideFaster,
	bool addLatVelIfAvailable) : 
  MvrAction(name, "Sets all the max vel and accels/decels")
{
  myOverrideFaster = overrideFaster;
  myAddLatVelIfAvailable = addLatVelIfAvailable;
  myEnabled = true;
  myEnableOnce = false;
  setParameters();
}

MVREXPORT MvrActionMovementParameters::~MvrActionMovementParameters()
{

}

MVREXPORT void MvrActionMovementParameters::setParameters(double maxVel, 
							double maxNegVel,
							double transAccel,
							double transDecel,
							double rotVelMax, 
							double rotAccel,
							double rotDecel,
							double latVelMax, 
							double latAccel,
							double latDecel)
{
  myMaxVel = maxVel;
  myMaxNegVel = maxNegVel;
  myTransAccel = transAccel;
  myTransDecel = transDecel;
  myMaxRotVel = rotVelMax;
  myRotAccel = rotAccel;
  myRotDecel = rotDecel;
  myMaxLatVel = latVelMax;
  myLatAccel = latAccel;
  myLatDecel = latDecel;
}

MVREXPORT void MvrActionMovementParameters::addToConfig(MvrConfig *config, 
						      const char *section, 
						      const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::DETAILED);
  name = strPrefix;
  name += "TransVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxVel, 
		      "Maximum forward translational velocity (0 means use default)", 
		      0),
		      //myRobot->getAbsoluteMaxTransVel()),
	  section, MvrPriority::DETAILED);


  name = strPrefix;
  name += "TransNegVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxNegVel, 
		      "Maximum backwards translational velocity (0 means use default)", 
		      0),
	  //myRobot->getAbsoluteMaxTransVel()),
	  section, MvrPriority::DETAILED);

  name = strPrefix;
  name += "TransAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myTransAccel, 
		      "Translational acceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxTransAccel()),		      
	  section, MvrPriority::DETAILED);

  name = strPrefix;
  name += "TransDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myTransDecel, 
		      "Translational deceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxTransDecel()),		      
	  section, MvrPriority::DETAILED);

  name = strPrefix;
  name += "RotVelMax";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myMaxRotVel, 
		      "Maximum rotational velocity (0 means use default)", 
		      0), // myRobot->getAbsoluteMaxRotVel()),
	  section, MvrPriority::DETAILED);

  name = strPrefix;
  name += "RotAccel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myRotAccel, 
		      "Rotational acceleration (0 means use default)", 0),
	  //myRobot->getAbsoluteMaxRotAccel()),
	  section, MvrPriority::DETAILED);

  name = strPrefix;
  name += "RotDecel";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myRotDecel, 
		      "Rotational deceleration (0 means use default)", 0),
		      //myRobot->getAbsoluteMaxRotDecel()),
	  section, MvrPriority::DETAILED);

  if (myAddLatVelIfAvailable && myRobot != NULL && myRobot->hasLatVel())
  {
    name = strPrefix;
    name += "LatVelMax";
    config->addParam(
	    MvrConfigArg(name.c_str(), &myMaxLatVel, 
			"Maximum lateral velocity (0 means use default)", 
			0), // myRobot->getAbsoluteMaxLatVel()),
	    section, MvrPriority::DETAILED);
    
    name = strPrefix;
    name += "LatAccel";
    config->addParam(
	    MvrConfigArg(name.c_str(), &myLatAccel, 
			"Lateral acceleration (0 means use default)", 0),
	    //myRobot->getAbsoluteMaxLatAccel()),
	    section, MvrPriority::DETAILED);
    
    name = strPrefix;
    name += "LatDecel";
    config->addParam(
	    MvrConfigArg(name.c_str(), &myLatDecel, 
			"Lateral deceleration (0 means use default)", 0),
	    //myRobot->getAbsoluteMaxLatDecel()),
	    section, MvrPriority::DETAILED);
  }

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section, MvrPriority::DETAILED);


}

MVREXPORT MvrActionDesired *MvrActionMovementParameters::fire(
	MvrActionDesired currentDesired)
{
  myDesired.reset();

  if (!myEnabled && !myEnableOnce)
    return NULL;
  myEnableOnce = false;

  if (fabs(myMaxVel) >= 1)
    myDesired.setMaxVel(myMaxVel);

  if (fabs(myMaxNegVel) >= 1)
    myDesired.setMaxNegVel(-myMaxNegVel);

  if (fabs(myTransAccel) >= 1)
    myDesired.setTransAccel(myTransAccel);

  if (fabs(myTransDecel) >= 1)
    myDesired.setTransDecel(myTransDecel);

  if (fabs(myMaxRotVel) >= 1)
    myDesired.setMaxRotVel(myMaxRotVel);

  if (fabs(myRotAccel) >= 1)
    myDesired.setRotAccel(myRotAccel);

  if (fabs(myRotDecel) >= 1)
    myDesired.setRotDecel(myRotDecel);

  if (fabs(myMaxLatVel) >= 1)
  {
    myDesired.setMaxLeftLatVel(myMaxLatVel);
    myDesired.setMaxRightLatVel(myMaxLatVel);
  }

  if (fabs(myLatAccel) >= 1)
    myDesired.setLatAccel(myLatAccel);

  if (fabs(myLatDecel) >= 1)
    myDesired.setLatDecel(myLatDecel);
  
  return &myDesired;
}
