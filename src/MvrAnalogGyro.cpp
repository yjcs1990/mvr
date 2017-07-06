#include "mvriaOSDef.h"
#include "MvrCommands.h"
#include "MvrExport.h"
#include "MvrAnalogGyro.h"
#include "MvrRobot.h"
#include "MvrRobotConfigPacketReader.h"

/**
**/
MVREXPORT MvrAnalogGyro::MvrAnalogGyro(MvrRobot *robot) : 
  myHandleGyroPacketCB(this, &MvrAnalogGyro::handleGyroPacket),
  myEncoderCorrectCB(this, &MvrAnalogGyro::encoderCorrect),
  myStabilizingCB(this, &MvrAnalogGyro::stabilizingCallback),
  myUserTaskCB(this, &MvrAnalogGyro::userTaskCallback)
{
  myRobot = robot;
  myHandleGyroPacketCB.setName("MvrAnalogGyro");
  myRobot->addStabilizingCB(&myStabilizingCB);
  // this scaling factor now comes from the parameter file
  myScalingFactor = 0;
  myHeading = 0;
  myReadingThisCycle = false;

  myAverageTotal = 0;
  myAverageCount = 0;
  myLastAverageTaken.setSec(0);
  myLastAverageTaken.setMSec(0);
  myLastAverage = 0;
  // nominal values
  myGyroSigma = .01;
  myInertialVarianceModel = 0.001;
  myRotVarianceModel = .25; // deg2/deg
  myTransVarianceModel = 4.0; // deg2/meter
  myAccumulatedDelta = 0;
  myIsActive = true;
  myHaveGottenData = false;
  myLogAnomalies = false;
  myGyroType = GYRO_NONE;
  myHasGyroOnlyMode = false;
  myHasNoData = true;
  myGyroWorking = true;
  if (myRobot->isConnected())
    stabilizingCallback();
  myUserTaskCB.setName("MvrAnalogGyro");
  myRobot->addUserTask("MvrAnalogGyro", 50, &myUserTaskCB);
  
}

MVREXPORT MvrAnalogGyro::~MvrAnalogGyro()
{
  myRobot->comInt(MvrCommands::GYRO, 0);
  myRobot->remPacketHandler(&myHandleGyroPacketCB);
  myRobot->remStabilizingCB(&myStabilizingCB);
  myRobot->setEncoderCorrectionCallback(NULL);
}

MVREXPORT void MvrAnalogGyro::stabilizingCallback(void)
{
  if (myRobot->getOrigRobotConfig() != NULL &&
      myRobot->getOrigRobotConfig()->getHasGyro() == 1)
  {
    myGyroType = GYRO_ANALOG_COMPUTER;
    myHasGyroOnlyMode = false;
    myHasNoData = false;
    myHaveGottenData = false;
    // moved these two here from above
    myRobot->setEncoderCorrectionCallback(&myEncoderCorrectCB);
    myRobot->addPacketHandler(&myHandleGyroPacketCB);

    myScalingFactor = myRobot->getRobotParams()->getGyroScaler();  
    
    if (!myRobot->isConnected())
      MvrLog::log(MvrLog::Normal, "Stabilizing gyro");
    if (!myRobot->isConnected() && myRobot->getStabilizingTime() < 3000)
      myRobot->setStabilizingTime(3000);
    myRobot->comInt(MvrCommands::GYRO, 1);
  }
  else if (myRobot->getOrigRobotConfig() != NULL &&
	   myRobot->getOrigRobotConfig()->getGyroType() >= 2)
  {
    myGyroType = GYRO_ANALOG_CONTROLLER;
    myHasGyroOnlyMode = true;
    myHasNoData = true;
    myHaveGottenData = true;
    if (!myRobot->isConnected())
      MvrLog::log(MvrLog::Normal, "Stabilizing microcontroller gyro");
    myRobot->setStabilizingTime(2000);
    // only set this if it isn't already set (since this now comes in
    // the config packet so its variable per connection/robot)
    if (myRobot->getOdometryDelay() == 0)
      myRobot->setOdometryDelay(25);
  }
}

MVREXPORT bool MvrAnalogGyro::handleGyroPacket(MvrRobotPacket *packet)
{
  int numReadings;
  int i;
  double reading;
  int temperature;
  double rate;
  MvrTime now;

  if (packet->getID() != 0x98)
    return false;

  numReadings = packet->bufToByte();
  if (numReadings > 0)
    myHaveGottenData = true;
  //packet->log();
  //printf("%d readings %d bytes ", numReadings, packet->getLength() - packet->getReadLength());
  for (i = 0; i < numReadings; i++)
  {
    reading = packet->bufToByte2();
    temperature = packet->bufToUByte();

    // if we haven't moved, check our average, to see if we've moved,
    // we see if the average is within .5% of the average and
    // if the velocity is less then 1 mm / sec and 
    // if the rotational velocity is less then 1 deg / sec
    //printf("%d %g %g %g %g\n", myAverageCount, myAverageTotal / myAverageCount, reading, myRobot->getVel(), myRobot->getRotVel());
    if ((myAverageCount == 0 || fabs(myAverageTotal / myAverageCount - reading) < myAverageTotal / myAverageCount * .005) &&
	fabs(myRobot->getVel()) < 1 && 
	fabs(myRobot->getRotVel()) < 1)
    {
      if (myAverageStarted.mSecSince() > 1000 && myAverageCount > 25)
      {
	//printf("new average\n");
	myLastAverage = myAverageTotal / myAverageCount;
	myLastAverageTaken.setToNow();
	myAverageTotal = 0;
	myAverageCount = 0;
	myAverageStarted.setToNow();
      }
      myAverageTotal += reading;
      myAverageCount++;
    }
    else
    {
      myAverageTotal = 0;
      myAverageCount = 0;
      myAverageStarted.setToNow();
    }
    
    if (myLastAverage == 0)
      continue;
    reading -= myLastAverage;
    rate = ((reading * 5.0 / 1023)) * 300 / 2.5 * myScalingFactor;
    rate *= -1;

    myTemperature = temperature;
    //printf("reading %10f rate %10f diff %10f temp %d, ave %g\n", reading, rate, rate * .025, temperature, myLastAverage);

    // if we're not moving and the reading is small disregard it
    if ((fabs(myRobot->getVel()) < 2 && fabs(myRobot->getRotVel()) < 2) &&
	MvrMath::fabs(reading) < 2)      
    {
      rate = 0;
    }
    myHeading += rate * .025;
    //printf("rate %6.3f, reading %6.3f heading %6.3f\n", rate, reading, myHeading);

    myHeading = MvrMath::fixAngle(myHeading);

    if (myTimeLastPacket != time(NULL)) 
    {
      myTimeLastPacket = time(NULL);
      myPacCount = myPacCurrentCount;
      myPacCurrentCount = 0;
    }
    myPacCurrentCount++;
    myReadingThisCycle = true;
    //printf("(%3d %3d)", reading, temperature);
  }
  //printf("\n");
  return true;
}

MVREXPORT double MvrAnalogGyro::encoderCorrect(MvrPoseWithTime deltaPose)
{
  MvrPose ret;

  // variables
  double inertialVariance;
  double encoderVariance;

  double robotDeltaTh;
  double inertialDeltaTh;
  double deltaTh;

  /*
  MvrPose lastPose;
  double lastTh;
  MvrPose thisPose;
  double thisTh;
  */
  MvrPoseWithTime zero(0.0, 0.0, 0.0);


  // if we didn't get a reading this take what we got at face value
  if (!myReadingThisCycle)
  {
    //MvrLog::log(MvrLog::Verbose, "MvrAnalogGyro: no inertial reading, using encoder");
    myAccumulatedDelta += deltaPose.getTh();
    //printf("adding %f\n", myAccumulatedDelta);
    return deltaPose.getTh();
  }

  //added this fix
  robotDeltaTh = MvrMath::fixAngle(myAccumulatedDelta + deltaPose.getTh());
  //printf("using %f %f %f\n", robotDeltaTh, myAccumulatedDelta, deltaPose.getTh());

  inertialVariance = (myGyroSigma * myGyroSigma * 10);
  // was: deltaPose.getTime().mSecSince(myLastAsked)/10.0);


  //printf("@ %10f %10f %10f %10f\n", multiplier, MvrMath::subAngle(thisTh, lastTh), thisTh, lastTh);
  inertialDeltaTh = MvrMath::subAngle(myHeading, myLastHeading);

  inertialVariance += fabs(inertialDeltaTh) * myInertialVarianceModel;
  encoderVariance = (fabs(deltaPose.getTh()) * myRotVarianceModel +
		     deltaPose.findDistanceTo(zero) * myTransVarianceModel);
  
  
  if (myLogAnomalies)
  {
    if (fabs(inertialDeltaTh) < 1 && fabs(robotDeltaTh) < 1)
    {

    }
    else if (fabs(robotDeltaTh) < 1 && fabs(inertialDeltaTh) > 2)
      MvrLog::log(MvrLog::Normal, "MvrAnalogGyro::anomaly: Gyro (%.1f) moved but encoder (%.1f) didn't, using gyro", inertialDeltaTh, robotDeltaTh);
    else if ((inertialDeltaTh < -1 && robotDeltaTh > 1) ||
	     (robotDeltaTh < -1 && inertialDeltaTh > 1))
      MvrLog::log(MvrLog::Normal, "MvrAnalogGyro::anomaly: gyro (%.1f) moved opposite of robot (%.1f)", inertialDeltaTh, robotDeltaTh);
    else if (fabs(robotDeltaTh) < fabs(inertialDeltaTh * .66666))
      MvrLog::log(MvrLog::Normal, "MvrAnalogGyro::anomaly: robot (%.1f) moved less than gyro (%.1f)", robotDeltaTh, inertialDeltaTh);
    else if (fabs(inertialDeltaTh) < fabs(robotDeltaTh * .66666))
      MvrLog::log(MvrLog::Normal, "MvrAnalogGyro::anomaly: gyro (%.1f) moved less than robot (%.1f)", inertialDeltaTh, robotDeltaTh);
  }


  //don't divide by 0, or close to it
  if (fabs(inertialVariance + encoderVariance) < .00000001)
    deltaTh = 0;
  // if we get no encoder readings, but we get gyro readings, just
  // believe the gyro
  else if (fabs(robotDeltaTh) < 1 && fabs(inertialDeltaTh) > 2)
    deltaTh = MvrMath::fixAngle(inertialDeltaTh);
  else
    deltaTh = MvrMath::fixAngle(
	    (robotDeltaTh * 
	     (inertialVariance / (inertialVariance + encoderVariance))) +
	    (inertialDeltaTh *
	     (encoderVariance / (inertialVariance + encoderVariance))));

  // now we need to compensate for the readings we got when we didn't
  // have gyro readings
  deltaTh -= myAccumulatedDelta;
  myReadingThisCycle = false;
  myLastHeading = myHeading;
  //printf("%6.3f %6.3f %6.3f\n", deltaTh, inertialDeltaTh, robotDeltaTh);

  myAccumulatedDelta = 0;
  
  if (myIsActive)
    return deltaTh;
  else
    return deltaPose.getTh();
}


MVREXPORT void MvrAnalogGyro::activate(void)
{ 
  if (!myIsActive || myIsGyroOnlyActive)
    MvrLog::log(MvrLog::Normal, "Activating gyro"); 
  myIsActive = true; 
  myIsGyroOnlyActive = false;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(MvrCommands::GYRO, 1);
}

MVREXPORT void MvrAnalogGyro::deactivate(void)
{ 
  if (myIsActive || myIsGyroOnlyActive)
    MvrLog::log(MvrLog::Normal, "Dectivating gyro"); 
  myIsActive = false; 
  myIsGyroOnlyActive = false;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(MvrCommands::GYRO, 0);
}

MVREXPORT void MvrAnalogGyro::activateGyroOnly(void)
{ 
  if (!myHasGyroOnlyMode)
  {
    MvrLog::log(MvrLog::Normal, 
	       "An attempt was made (and rejected) to set gyro only mode on a gyro that cannot do that mode"); 
    return;
  }
  if (!myIsActive || !myIsGyroOnlyActive)
    MvrLog::log(MvrLog::Normal, "Activating gyro only"); 
  myIsActive = false; 
  myIsGyroOnlyActive = true;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(MvrCommands::GYRO, 2);
}

MVREXPORT void MvrAnalogGyro::userTaskCallback(void)
{
  if ((myRobot->getFaultFlags() & MvrUtil::BIT4) && myGyroWorking)
  {
    MvrLog::log(MvrLog::Normal, "MvrAnalogGyro: Gyro failed");
    myGyroWorking = false;
  }

  if (!(myRobot->getFaultFlags() & MvrUtil::BIT4) && !myGyroWorking)
  {
    MvrLog::log(MvrLog::Normal, "MvrAnalogGyro: Gyro recovered");
    myGyroWorking = true;
  }
}
