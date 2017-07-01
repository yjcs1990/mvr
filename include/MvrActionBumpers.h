#ifndef MVRACTIONBUMPERS_H
#define MVRACTIONBUMPERS_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

/// Action to deal with if the bumpers trigger
/**
   This class basically responds to the bumpers the robot has, what
   the activity things the robot has is decided by the param file.  If
   the robot is going forwards and bumps into something with the front
   bumpers, it will back up and turn.  If the robot is going backwards
   and bumps into something with the rear bumpers then the robot will
   move forward and turn.  

  @ingroup ActionClasses
*/

class MvrActionBumpers : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionBumpers(const char *name = "bumpers", 
			   double backOffSpeed = 100, int backOffTime = 3000,
			   int turnTime = 3000, bool setMaximums = false);
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
  //MvrTime myStoppedSince;
};

#endif // MVRACTIONBUMPERS
