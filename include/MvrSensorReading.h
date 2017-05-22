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
  MVREXPORT MvrSensorReading &operator(const MvrSensorReading);
  /// Destructor
  MVREXPORT virtual ~MvrSensorReading();

  /// Get the range from sensor of the reading
  /*
   * @return the distance to the reading from the sensor itself
   */
  unsigned int getRange(void) const { return myRange; }

  // Given the counter from the robot, it returns whether the reading is new
  
protected:
};

#endif  // MVRSENSORRECORDING_H