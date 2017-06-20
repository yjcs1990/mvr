/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSensorReading.cpp
 > Description  : Used to convert and store data from  and/or about a range sensor
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrSensorReading.h"

/*
 * @param xPos the x position of the sensor on the robot (mm)
 * @param yPos the y position of the sensor on the robot (mm)
 * @param thPos the heading of the sensor on the robot (deg)
 */
MvrSensorReading::MvrSensorReading(double xPos, double yPos, double thPos)
{
  myRange = 5000;
  myCounterTaken = 0;
  myReading.setPose(-1, -1);
  myReadingTaken.setPose(-1, -1, -1);
  resetSensorPosition(xPos, yPos, thPos);
  myExtraInt = 0;
  myAdjusted = false;
}

MVREXPORT MvrSensorReading::MvrSensorReading(const MvrSensorReading &reading)
{
  myCounterTaken      = reading.myCounterTaken;
  myReading           = reading.myReading;
  myLocalReading      = reading.myLocalReading;
  myReadingTaken      = reading.myReadingTaken;
  mySensorPos         = reading.mySensorPos;
  mySensorCos         = reading.mySensorCos;
  mySensorSin         = reading.mySensorSin;
  myDistToCenter      = reading.myDistToCenter;
  myAngleToCenter     = reading.myAngleToCenter;
  myRange             = reading.myRange;
  myTimeTaken         = reading.myTimeTaken;
  myIgnoreThisReading = reading.myIgnoreThisReading;
  myExtraInt          = reading.myExtraInt;
  myadjusted          = reading.myadjusted;
}

MVREXPORT MvrSensorReading &MvrSensorReading::operator=(const MvrSensorReading &reading)
{
  if (this != &reading)
  {
    myCounterTaken      = reading.myCounterTaken;
    myReading           = reading.myReading;
    myLocalReading      = reading.myLocalReading;
    myReadingTaken      = reading.myReadingTaken;
    mySensorPos         = reading.mySensorPos;
    mySensorCos         = reading.mySensorCos;
    mySensorSin         = reading.mySensorSin;
    myDistToCenter      = reading.myDistToCenter;
    myAngleToCenter     = reading.myAngleToCenter;
    myRange             = reading.myRange;
    myTimeTaken         = reading.myTimeTaken;
    myIgnoreThisReading = reading.myIgnoreThisReading;
    myExtraInt          = reading.myExtraInt;
    myadjusted          = reading.myadjusted;
  }
  return *this;
}

MvrSensorReading::~MvrSensorReading()
{

}

/*
 * @param range the distance from the sensor to the sensor return (mm)
 * @param robotPose the robot's pose when the reading was taken
 * @param encoderPose the robot's encoder pose when the reading was taken
 * @param trans the transform from local coords to global coords
 * @param counter the counter from the robot when the sensor reading was taken
 * @param timeTaken the time the reading was taken
 * @param ignoreThisReading if this reading should be ignored or not
 * @param extraInt extra laser device-specific value associated with this
 */
MVREXPORT void MvrSensorReading::newData(int range, MvrPose robotPose, MvrPose encoderPose,
                                         MvrTransform trans, unsigned int counter, 
                                         MvrTime timeTaken, bool ignoreThisReading,
                                         int extraInt)
{
  // TODO caculate the x and y position of the sensor
  double rx, ry;
  myRange = range;
  myCounterTaken      = counter;
  myReadingTaken      = robotPose;
  myEncoderPoseTaken  = encoderPose;
  rx = getSensorX() + myRange * mySensorCos;
  ry = getSensorY() + myRange * mySensorSin;
  myLocalReading.setPose(rx, ry);
  myReading           = trans.doTransform(myLocalReading);
  myTimeTaken         = timeTaken;
  myIgnoreThisReading = ignoreThisReading;
  myExtraInt          = extraInt;
  myAdjusted          = false;
}        

/*
 * @param sx the coords of the sensor return relative to sensor (mm)
 * @param sy the coords of the sensor return relative to sensor (mm)
 * @param robotPose the robot's pose when the reading was taken
 * @param encoderPose the robot's encoder pose when the reading was taken
 * @param trans transform of reading from local to global position
 * @param counter the counter from the robot when the sensor reading was taken
 * @param timeTaken the time the reading was taken
 * @param ignoreThisReading if this reading should be ignored or not
 * @param extraInt extra laser device-specific value associated with this
 * reading
 */
MVREXPORT void MvrSensorReading::newData(int sx, int sy, MvrPose robotPose,
                                         MvrPose encoderPose, MvrTransform trans, 
                                         unsigned int counter, MvrTime timeTaken,
                                         bool ignoreThisReading, int extraInt)
{
  // TODO caculate the x and y position of the sensor
  double rx, ry;
  myRange = (int)sqrt((double)(sx*sx + sy*sy));  
  myCounterTaken      = counter;
  myReadingTaken      = robotPose;
  myEncoderPoseTaken  = encoderPose;
  rx = getSensorX() + sx;
  ry = getSensorY() + sy;
  myLocalReading.setPose(rx, ry);
  myReading           = trans.doTransform(myLocalReading);
  myTimeTaken         = timeTaken;
  myIgnoreThisReading = ignoreThisReading;
  myExtraInt          = extraInt;
  myAdjusted          = false;
}                                         

/*
 * @param xPos the x position of the sensor on the robot (mm)
 * @param yPos the y position of the sensor on the robot (mm)
 * @param thPos the heading of the sensor on the robot (deg)
 * @param forceComputation recompute position even if new position is the same as current
 */
MVREXPORT void MvrSensorReading::resetSensorPosition(double xPos, double yPos, double thPos, bool forceComputation)
{
  // if its the same position and we're not forcing, just bail
  if (!forceComputation && fabs(thPos - mySensorPos.getTh()) < .00001 &&
      xPos == mySensorPos.getX() && yPos == mySensorPos.getY())
    return;
  
  mySensorPos.setPose(xPos, yPos, thPos);
  myDistToCenter  = sqrt(xPos*xPos + yPos*yPos);
  myAngleToCenter = MvrMath::atan2(xPos, yPos);
  mySensorCos     = MvrMath::cos(thPos);
  mySensorSin     = MvrMath::sin(thPos);
}

/*
 *  @param trans the transform to apply to the reading and where the reading was taken
 */
MVREXPORT void MvrSensorReading::applyTransform(MvrTransform trans)
{
  myReading      = trans.doTransform(myReading);
  myReadingTaken = trans.doTransform(myReadingTaken);
}

/*
 * @param trans the transform to apply to the encoder pose taken
 */
MVREXPORT void MvrSensorReading::applyEncoderTransform(MvrTransform trans)
{
  myEncoderPoseTaken = trans.doTransform(myEncoderPoseTaken);
}