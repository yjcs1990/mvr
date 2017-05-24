/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionInput.h
 > Description  : Action for taking input from outside to control the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONINPUT_H
#define MVRACTIONINPUT_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionInput : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionInput(const char *name = "Input");
  /// Destructor
  MVREXPORT virtual ~MvrActionInput();
  /// Set velocity (cancels deltaVel)
  MVREXPORT void setVel(double vel);
  /// Increment/decrement the heading from current
  MVREXPORT void deltaHeadingFromCurrent(double delta);
  /// Sets a rotational velocity
  MVREXPORT void setRotVel(double rotVel);
  /// Sets a heading
  MVREXPORT void setHeading(double heading);
  /// Clears it so its not using vel or heading
  MVREXPORT void clear(void);
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  enum RotRegime { NONE, ROTVEL, DELTAHEADING, SETHEADING };
  RotRegime myRotRegime;
  double myRotVal;
  bool myUsingVel;
  double myVelSet;
  MvrActionDesired myDesired;
};

#endif  // MVRACTIONINPUT_H