#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrActionIRs.h"
#include "MvrRobot.h"
#include "MvrCommands.h"

/**
   @param name name of the action
   @param backOffSpeed speed at which to back away (mm/sec)
   @param backOffTime number of msec to back up for (msec)
   @param turnTime number of msec to alow for turn (msec)
   @param setMaximums if true, set desired maximum speed limits to backOffSpeed when performing the action; otherwise use existing speed limits.
*/
MVREXPORT MvrActionIRs::MvrActionIRs(const char *name, 
					  double backOffSpeed,
					  int backOffTime, int turnTime,
					  bool setMaximums) :
  MvrAction(name, "Reacts to the IRs triggering")
{
  setNextArgument(MvrArg("back off speed", &myBackOffSpeed, 
			"Speed at which to back away (mm/sec)"));
  myBackOffSpeed = backOffSpeed;

  setNextArgument(MvrArg("back off time", &myBackOffTime,
			"Number of msec to back up for (msec)"));
  myBackOffTime = backOffTime;

  myStopTime = 1000;

  setNextArgument(MvrArg("turn time", &myTurnTime,
			"Number of msec to allow for turn (msec)"));
  myTurnTime = turnTime;

  setNextArgument(MvrArg("set maximums", &mySetMaximums,
			"Whether to set maximum vels or not (bool)"));
  mySetMaximums = setMaximums;
  
  myFiring = false; 
  mySpeed = 0.0;
  myHeading = 0.0;
}

MVREXPORT MvrActionIRs::~MvrActionIRs()
{

}

MVREXPORT void MvrActionIRs::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  const MvrRobotParams *params;
  params = myRobot->getRobotParams();
  myParams = *params;
  
  for(int i = 0; i < myParams.getNumIR(); i++)
    cycleCounters.push_back(1);
}

MVREXPORT MvrActionDesired *MvrActionIRs::fire(MvrActionDesired currentDesired)
{
  myDesired.reset();

  double angle = 0;
  int counter = 0;
  double turnRange = 135;

  MvrUtil::BITS bit;

  if(myFiring)
    {  
      if (myStartBack.mSecSince() < myBackOffTime)
	{
	  myDesired.setVel(mySpeed);
	  myDesired.setDeltaHeading(0);
	  return &myDesired;
	}
      else if (myStartBack.mSecSince() < myBackOffTime + myTurnTime &&
	     MvrMath::fabs(MvrMath::subAngle(myRobot->getTh(), myHeading)) > 3)
	{
	  myDesired.setVel(0);
	  myDesired.setHeading(myHeading);
	  return &myDesired;
	}  
      else if(stoppedSince.mSecSince() < myStopTime)
	{
	  myDesired.setVel(0);
	  myDesired.setDeltaHeading(0);
	  return &myDesired;
	}
    
      myFiring = false;
    }


  if(myParams.haveTableSensingIR())
    {
      for (int i = 0; i < myParams.getNumIR(); ++i)
	{
	  switch(i)
	    {
	    case 0:
	      bit = MvrUtil::BIT0;
	      break;
	    case 1:
	      bit = MvrUtil::BIT1;
	      break;
	    case 2:
	      bit = MvrUtil::BIT2;
	      break;
	    case 3:
	      bit = MvrUtil::BIT3;
	      break;
	    case 4:
	      bit = MvrUtil::BIT4;
	      break;
	    case 5:
	      bit = MvrUtil::BIT5;
	      break;
	    case 6:
	      bit = MvrUtil::BIT6;
	      break;
	    case 7:
	      bit = MvrUtil::BIT7;
	      break;
	    }
	  if(myParams.haveNewTableSensingIR() && myRobot->getIODigInSize() > 3)
	    {
	      if((myParams.getIRType(i) && !(myRobot->getIODigIn(3) & bit)) ||
		 (!myParams.getIRType(i) && (myRobot->getIODigIn(3) & bit)))
	      {
		if(cycleCounters[i] < myParams.getIRCycles(i))
		  {
		    cycleCounters[i] = cycleCounters[i] + 1;	      
		  }
		else
		  {
		    cycleCounters[i] = 1;
		   
		    MvrPose pose;
		    pose.setX(myParams.getIRX(i));
		    pose.setY(myParams.getIRY(i));
		    if(pose.getX() > 0)
		      {
			MvrPose center(0,0,0);
			angle += center.findAngleTo(pose);
			counter++;
		      }
		  }
	      }
	      else
		{
		  cycleCounters[i] = 1;
		}
	    }
	  else
	    {
	      if(!(myRobot->getDigIn() & bit))
	      {
		if(cycleCounters[i] < myParams.getIRCycles(i))
		  {
		    cycleCounters[i] = cycleCounters[i] + 1;	      
		  }
		else
		  {
		    cycleCounters[i] = 1;
		    
		    MvrPose pose;
		    pose.setX(myParams.getIRX(i));
		    pose.setY(myParams.getIRY(i));
		    if(pose.getX() > 0)
		      {
			MvrPose center(0,0,0);
			angle += center.findAngleTo(pose);
			counter++;
		      }
		  }
	      }
	      else
		{
		  cycleCounters[i] = 1;
		}
	      
	    }
	}
 
      if(counter > 0 && myRobot->getVel() > 50)
	{
	  angle = angle / (double) counter;
	  if(angle > (turnRange / 2))
	    angle = turnRange / 2;
	  else if(angle < -(turnRange / 2))
	    angle = -(turnRange / 2);
	  
	  if(angle < 0) angle = ((turnRange / 2) + angle) * -1;
	  else angle = ((turnRange / 2) - angle);
	
	  myHeading = MvrMath::addAngle(myRobot->getTh(), angle);
	  mySpeed = -myBackOffSpeed;
	  myStartBack.setToNow();
	  MvrLog::log(MvrLog::Normal, "MvrActionIRS: estopping");
	  myRobot->comInt(MvrCommands::ESTOP, 0);
	  myFiring = true;
	  
	  myDesired.setVel(mySpeed);
	  myDesired.setHeading(myHeading);
  
	}
      else if(counter > 0 && (myRobot->getVel() > -50 && myRobot->getVel() < 50))
	{
	  stoppedSince.setToNow();
	}
      else return NULL;
    }
  else return NULL;
  

  return &myDesired;
}  
