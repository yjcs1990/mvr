/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionBumpers.h
 > Description  : Action to deal with if the bumpers trigger
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONBUMPER_H
#define MVRACTIONBUMPER_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrActionBumpers : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionBumpers(const char *name="bumper",
            double backOffSpeed=100, int backOffTime=3000,
            int turnTime=3000, bool setMaximums=false);
  /// Destructor
  MVREXPORT virtual ~MvrActionBumpers();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  MVREXPORT double findDegreesToTurn(int bumpValue, int whichBumper);
  MVREXPORT virtual void activate(void);
protected:
  MvrActionDesired myDesired;
  bool mySetMaximums;
  double myBackOffSpeed;
  int myBackOffTime;
  int myTurnTime;
  //int myStopTime;
  bool myFiring;
  double mySpeed;
  double myHeading;
  int myBumpMask;
  MvrTime myStartBack;
};

#ifndef MVRACTIONBUMPER_H