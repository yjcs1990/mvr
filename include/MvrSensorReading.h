/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSensorReading.h
 > Description  : Used to convert and store data from  and/or about a range sensor
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRSENSORREADING_H
#define MVRSENSORREADING_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrTransform.h"

class MvrSensorReading
{
public:
  /// Constructor, the three args are the physical location of the sensor
  MVREXPORT MvrSensorReading(double xPos=0.0, double yPos=0.0, double thPos=0.0);
  /// Copy constructor
  MVREXPORT MvrSensorReading(const MvrSensorReading &reading);
  /// Assignment operator
  MVREXPORT MvrSensorReading &operator=(const MvrSensorReading &reading);
  /// Destructor
  MVREXPORT virtual ~MvrSensorReading();

  /// Get the range from sensor of the reading
  /*
   * @return the distance to the reading from the sensor itself
   */
  unsigned int getRange(void) const { return myRange; }

  // Given the counter from the robot, it returns whether the reading is new
  /*
   * @param counter the counter from the robot at the current time
   * @return true if the reading was taken on the current loop
   * @see getCounter
  */
  bool isNew(unsigned int counter) const { return counter == myCounterTaken; }
  /// Gets the X location of the sensor reading
  double getX(void) const { return myReading.getX(); }
  /// Gets the Y location of the sensor reading
  double getY(void) const { return myReading.getY(); }
  /// Gets the position of the reading 
  /// @return the position of the reading (ie where the sonar pinged back)
  MvrPose getPose(void) const { return myReading; }

  /// Gets the X location of the sensor reading in local coords
  double getLocalX(void) const { return myLocalReading.getX(); }
  /// Gets the Y location of the sensor reading
  double getLocalY(void) const { return myLocalReading.getY(); }
  /// Gets the position of the reading 
  /// @return the position of the reading (ie the obstacle where the sonar pinged back)
  MvrPose getLocalPose(void) const { return myLocalReading; }

  /* Gets the pose of the robot at which the reading was taken 
   * @sa getEncoderPoseTaken()
   * @sa getTimeTaken()
   * @sa MvrRobot::getPose()
  */
  MvrPose getPoseTaken(void) const { return myReadingTaken; }

  /* Gets the robot's encoder pose the reading was taken at
   * @sa getPoseTaken()
   * @sa MvrRobot::getEncoderPose()
  */
  MvrPose getEncoderPoseTaken(void) const { return myEncoderPoseTaken; }

  /*
   * Gets the X location of the sonar on the robot
   * @sa getSensorPosition()
  */
  double getSensorX(void) const { return mySensorPos.getX(); }
  /* Gets the Y location of the sensor on the robot
   *   @sa getsensorPosition()
   */
  double getSensorY(void) const { return mySensorPos.getY(); }
  /* Gets the heading of the sensor on the robot
   * @sa getsensorPosition()
   */
  double getSensorTh(void) const { return mySensorPos.getTh(); }
  
  /// Gets whether this reading should be ignore or not. e.g. the sensor
  /// encountered an error or did not actually detect anything.
  bool getIgnoreThisReading(void) const { return myIgnoreThisReading; }

  /// Gets the extra int with this reading
  /**
     Some range devices provide extra device-dependent information
     with each reading.  What that means depends on the range device,
     if a range device doesn't give the meaning in its constructor
     description then it has no meaning at all.

     Note that for all laser like devices this should be a value
     between 0 - 255 which is the measure of reflectance.  It should
     be 0 if that device doesn't measure reflectance (the default).
   **/
  int getExtraInt(void) const { return myExtraInt; }


  /// Gets the sensor's position on the robot
  /* 
   * @return the position of the sensor on the robot
  */
  MvrPose getSensorPosition(void) const { return mySensorPos; }

  /// Gets the cosine component of the heading of the sensor reading
  double getSensorDX(void) const { return mySensorCos; }
  /// Gets the sine component of the heading of the sensor reading
  double getSensorDY(void) const { return mySensorSin; }

  /*
   * Gets the X locaiton of the robot when the reading was received
   * @sa getPoseTaken()
   */
  double getXTaken(void) const { return myReadingTaken.getX(); }
  /*
   * Gets the Y location of the robot when the reading was received
   * @sa getPoseTaken()
   */
  double getYTaken(void) const { return myReadingTaken.getY(); }
  /* Gets the th (heading) of the robot when the reading was received
   *   @sa getPoseTaken()
   */
  double getThTaken(void) const { return myReadingTaken.getTh(); }

  /// Gets the counter from when the reading arrived
  /*
   * @return the counter from the robot when the sonar reading was taken
   * @see isNew
   */
  unsigned int getCounterTaken(void) const { return myCounterTaken; }

  MvrTime getTimeTaken(void) const { return myTimeTaken; }
  
  /**
    Update data. 
    @param range Sensed distance. Will be projected to a global X,Y position based on the sensor position and @a robotPose
    @param robotPose Robot position in global coordinates space when the sensor data was received.
    @param encoderPose Robot encoder-only position in global coordinate space when the sensor data was received.
    @param trans Transform reading position from robot-local coordinate system.
    For example, pass result of MvrRobot::getToGlobalTransform() transform to robot's global
    coordinate system.
    @param counter an incrementing counter used to check for updated data (by this class
    and other classes using MvrSensorReading objects)
    @param timeTaken System time when this measurement was taken or received.
    @param ignoreThisReading Set the "ignore" flag for this reading. Data is stored but applications (e.g. navigation) may use this flag to ignore some sensor readings based on robot or user configuration.
    @param extraInt extra device-specific data. @see getExtraInt()
*/
  MVREXPORT void newData(int range, MvrPose robotPose, MvrPose encoderPose,
                         MvrTransform trans, unsigned int counter, 
                         MvrTime timeTaken, bool ignoreThisReading = false,
                         int extraInt = 0);

  /**
    @copydoc newData(int, MvrPose, MvrPose, MvrTransform, unsigned int, MvrTime, bool, int)
  */
  MVREXPORT void newData(int sx, int sy, MvrPose robotPose,
                         MvrPose encoderPose,
                         MvrTransform trans, 
                         unsigned int counter,
                         MvrTime timeTaken,
                         bool ignoreThisReading = false,
                         int extraInt = 0);

  /// Resets the sensors idea of its physical location on the robot
  MVREXPORT void resetSensorPosition(double xPos, double yPos, double thPos,
				                             bool forceComputation = false);

  /// Sets that we should ignore this reading
  MVREXPORT void setIgnoreThisReading(bool ignoreThisReading) 
    { myIgnoreThisReading = ignoreThisReading; }

  /// Sets the extra int
  MVREXPORT void setExtraInt(int extraInt) { myExtraInt = extraInt; }


  /// Applies a transform to the reading position, and where it was taken
  /// @internal
  MVREXPORT void applyTransform(MvrTransform trans);
  /// Applies a transform to the encoder pose taken
  /// @internal
  MVREXPORT void applyEncoderTransform(MvrTransform trans);
  /// Whether a transform to this reading's position was applied (An adjustment
  /// transform due to robot position and motion, etc. is normally initiated
  /// automatically by the range device class which is providing this sensor
  /// reading.)
  MVREXPORT bool getAdjusted(void) { return myAdjusted; }
  /// Applies a transform to the reading position, and where it was taken
  /// @internal
  MVREXPORT void setAdjusted(bool adjusted) { myAdjusted = adjusted; }
protected:
  unsigned int myCounterTaken;
  MvrPose myReading;
  MvrPose myLocalReading;  
  MvrPose myReadingTaken;
  MvrPose myEncoderPoseTaken;
  MvrPose mySensorPos;
  double mySensorCos, mySensorSin;
  double myDistToCenter;
  double myAngleToCenter;
  int myRange;
  MvrTime myTimeTaken;
  bool myIgnoreThisReading;
  int myExtraInt;
  bool myAdjusted;
};

#endif  // MVRSENSORRECORDING_H