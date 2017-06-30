/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "MvrExport.h"
#include "ariaOSDef.h"
#include "MvrGripper.h"
#include "MvrCommands.h"

/**
   @param robot The robot this gripper is attached to
   @param gripperType How to communicate with the gripper: see MvrGripper::Type. 
     The default, QUERYTYPE, will work with most robot configurations with 
     a recent firmware version.
*/
MVREXPORT MvrGripper::ArGripper(MvrRobot *robot, int gripperType) :
  myConnectCB(this, &ArGripper::connectHandler),
  myPacketHandlerCB(this, &ArGripper::packetHandler)
{
  myRobot = robot;
  myType = gripperType;
  if (myRobot != NULL) 
  {
    myRobot->addPacketHandler(&myPacketHandlerCB, MvrListPos::FIRST);
    myRobot->addConnectCB(&myConnectCB, MvrListPos::LAST);
    if (myRobot->isConnected() && (myType == GRIPPAC || myType == QUERYTYPE))
      myRobot->comInt(MvrCommands::GRIPPERPACREQUEST, 2);
  }
  myLastDataTime.setToNow();
}

MVREXPORT MvrGripper::~MvrGripper()
{
}

MVREXPORT void MvrGripper::connectHandler(void)
{
  if (myRobot != NULL && (myType == GRIPPAC || myType == QUERYTYPE))
    myRobot->comInt(MvrCommands::GRIPPERPACREQUEST, 2);
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripOpen(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIP_OPEN);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripClose(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIP_CLOSE);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripStop(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIP_STOP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::liftUp(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::LIFT_UP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::liftDown(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::LIFT_DOWN);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::liftStop(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::LIFT_STOP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripperStore(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIPPER_STORE);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripperDeploy(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIPPER_DEPLOY);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripperHalt(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(MvrCommands::GRIPPER, 
			   MvrGripperCommands::GRIPPER_HALT);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::gripPressure(int mSecIntervals)
{
  if (myRobot == NULL)
    return false;
  
  if (myRobot->comInt(MvrCommands::GRIPPER, MvrGripperCommands::GRIP_PRESSURE) &&
      myRobot->comInt(MvrCommands::GRIPPERVAL, mSecIntervals))
    return true;
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
MVREXPORT bool MvrGripper::liftCarry(int mSecIntervals)
{
  if (myRobot == NULL)
    return false;
  
  if (myRobot->comInt(MvrCommands::GRIPPER, MvrGripperCommands::LIFT_CARRY) &&
      myRobot->comInt(MvrCommands::GRIPPERVAL, mSecIntervals))
    return true;
  else
    return false;
}

/**
   @return true if the gripper paddles are moving
*/
MVREXPORT bool MvrGripper::isGripMoving(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO || myType == USERIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else
      d = mySwitches;

    if (myType == USERIO && (d & MvrUtil::BIT2)) // moving
      return true;
    else if (myType != USERIO && (d & MvrUtil::BIT7)) // moving
      return true;
    else // not moving
      return false;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrGripper::isGripMoving: Gripper type unknown.");
    return false;
  }
}

/**
   @return true if the lift is moving
*/
MVREXPORT bool MvrGripper::isLiftMoving(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO || myType == USERIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else
      d = mySwitches;

    if (d & MvrUtil::BIT6) // moving
      return true;
    else // not moving
      return false;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrGripper::isLiftMoving: Gripper type unknown.");
    return false;
  }
    
}

/**
   @return 0 if no gripper paddles are triggered, 1 if the left paddle
   is triggered, 2 if the right paddle is triggered, 3 if both are
   triggered
**/
MVREXPORT int MvrGripper::getPaddleState(void) const
{
  int d;
  int ret = 0;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if (!(d & MvrUtil::BIT4))
      ret += 1;
    if (!(d & MvrUtil::BIT5))
      ret += 2;
    return ret;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrGripper::getPaddleState: Gripper type unknown.");
    return 0;
  }
}
/**
   @return 0 if gripper paddles between open and closed, 1 if gripper paddles 
   are open, 2 if gripper paddles are closed
*/
MVREXPORT int MvrGripper::getGripState(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if (!(d & MvrUtil::BIT4) && !(d & MvrUtil::BIT5)) // both
      return 2;
    else if (!(d & MvrUtil::BIT0)) // inner
      return 1;
    else // between
      return 0;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrGripper::getGripState: Gripper type unknown.");
    return 0;
  }

}

/**
   @return 0 if no breakbeams broken, 1 if inner breakbeam broken, 2 if 
   outter breakbeam broken, 3 if both breakbeams broken
*/
MVREXPORT int MvrGripper::getBreakBeamState(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if ((d & MvrUtil::BIT2) && (d & MvrUtil::BIT3)) // both
      return 3;
    else if (d & MvrUtil::BIT3) // inner
      return 1;
    else if (d & MvrUtil::BIT2) // outter
      return 2;
    else // neither
      return 0;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrGripper::getBreakBeamState: Gripper type unknown.");
    return 0;
  }
}

/**
   @return false if lift is between up and down, true is either all the 
   way up or down
*/
MVREXPORT bool MvrGripper::isLiftMaxed(void) const
{
  int d = 0;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;
    if (!(d & MvrUtil::BIT1))
      return true;
    else
      return false;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrGripper::getLiftState: Gripper type unknown.");
    return false;
  }
}
  

MVREXPORT void MvrGripper::logState(void) const
{
  char paddleBuf[128];
  char liftBuf[128];
  char breakBeamBuf[128];
  char buf[1024];
  int state;

  if (myType == NOGRIPPER)
  {
    MvrLog::log(MvrLog::Terse, "There is no gripper.");
    return;
  } 
  if (myType == QUERYTYPE)
  {
    MvrLog::log(MvrLog::Terse, "Querying gripper type.");
    return;
  }
  
  if (isLiftMaxed())
    sprintf(liftBuf, "maxed");
  else
    sprintf(liftBuf, "between");

  if (isLiftMoving())
    strcat(liftBuf, "_moving");

  state = getGripState();
  if (state == 1)
    sprintf(paddleBuf, "open");
  else if (state == 2)
    sprintf(paddleBuf, "closed");
  else
    sprintf(paddleBuf, "between");

  if (isGripMoving())
    strcat(paddleBuf, "_moving");

  state = getBreakBeamState();
  if (state == 0)
    sprintf(breakBeamBuf, "none");
  else if (state == 1)
    sprintf(breakBeamBuf, "inner");
  else if (state == 2)
    sprintf(breakBeamBuf, "outter");
  else if (state == 3)
    sprintf(breakBeamBuf, "both");
  
  sprintf(buf, "Lift: %15s  Grip: %15s  BreakBeam: %10s", liftBuf, paddleBuf,
	  breakBeamBuf);
  if (myType == GRIPPAC)
    sprintf(buf, "%s TimeSince: %ld", buf, getMSecSinceLastPacket());
  MvrLog::log(MvrLog::Terse, buf);
  
}

MVREXPORT bool MvrGripper::packetHandler(MvrRobotPacket *packet)
{
  int type;
  
  if (packet->getID() != 0xE0)
    return false;

  myLastDataTime.setToNow();
  type = packet->bufToUByte();  
  mySwitches = packet->bufToUByte();
  myGraspTime = packet->bufToUByte();

  if (myType == QUERYTYPE)
  {
    if (type == 2)
    {
      MvrLog::log(MvrLog::Normal, 
		 "Gripper:  querried, using General IO.");
      myType = GENIO;
    }
    else if (type == 1)
    {
      MvrLog::log(MvrLog::Normal, 
		 "Gripper:  querried, using User IO.");
      myType = USERIO;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, 
		 "Gripper:  querried, the robot has no gripper.");
      myType = NOGRIPPER;
    }
    if (myRobot != NULL)
      myRobot->comInt(MvrCommands::GRIPPERPACREQUEST, 0);
    return true;
  }
  if (myRobot != NULL && myType != GRIPPAC)
  {
    MvrLog::log(MvrLog::Verbose, 
	       "Gripper: got another gripper packet after stop requested.");
    myRobot->comInt(MvrCommands::GRIPPERPACREQUEST, 0);
  }
  return true;
}

/**
   @return the gripper type
   @see Type
*/
MVREXPORT int MvrGripper::getType(void) const
{
  return myType;
}

/**
   @param type the type of gripper to set it to
*/
MVREXPORT void MvrGripper::setType(int type)
{
  myType = type;
  if (myRobot != NULL && (myType == GRIPPAC || myType == QUERYTYPE))
    myRobot->comInt(MvrCommands::GRIPPERPACREQUEST, 2);
}

/**
   @return the number of milliseconds since the last packet
*/
MVREXPORT long MvrGripper::getMSecSinceLastPacket(void) const
{
  return myLastDataTime.mSecSince();
}

/**
   If you are using this as anything other than GRIPPAC and you want to
   find out the grasp time again, just do a setType with QUERYTYPE and 
   it will query the robot again and get the grasp time from the robot.
   @return the number of 20 MSec intervals the gripper will continue grasping 
   for after both paddles are triggered
*/
MVREXPORT int MvrGripper::getGraspTime(void) const
{
  return myGraspTime;
}
