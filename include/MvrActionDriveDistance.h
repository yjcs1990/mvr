#ifndef MVRACTIONDRIVEDISTANCE_H
#define MVRACTIONDRIVEDISTANCE_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrAction.h"

/// This action drives the robot specific distances

/**
   This action naively drives a fixed distance. The action stops the
   robot when it has travelled the appropriate distance. It
   travels at 'speed' mm/sec.

   You can give it a distance with setDistance(), cancel its movement
   with cancelDistance(), and see if it got there with
   haveAchievedDistance().

   You can tell it to go backwards by calling setDistance with a
   negative value.

   This doesn't avoid obstacles or anything, you could add have an
   limiting MvrAction at a higher priority to try to do this (so you
   don't smash things). (For truly intelligent navigation, see
   the ARNL or SONARNL software libraries.)
  @ingroup ActionClasses
**/


class MvrActionDriveDistance : public MvrAction
{
public:
  MVREXPORT MvrActionDriveDistance(const char *name = "driveDistance", 
				double speed = 400, double deceleration = 200);
  MVREXPORT virtual ~MvrActionDriveDistance();

  /// Sees if the goal has been achieved
  MVREXPORT bool haveAchievedDistance(void);
  /// Cancels the goal the robot has
  MVREXPORT void cancelDistance(void);
  /// Sets a new goal and sets the action to go there
  MVREXPORT void setDistance(double distance, bool useEncoders = true);
  /// Gets whether we're using the encoder position or the normal position
  bool usingEncoders(void) { return myUseEncoders; }
  /// Sets the speed the action will travel at (mm/sec)
  void setSpeed(double speed = 400) { mySpeed = speed; }
  /// Gets the speed the action will travel at (mm/sec)
  double getSpeed(void) { return mySpeed; }
  /// Sets the deceleration the action will use (mm/sec/sec)
  void setDeceleration(double deceleration = 200) 
    { myDeceleration = deceleration; }
  /// Gets the deceleration the action will use (mm/sec/sec)
  double getDeceleration(void) { return myDeceleration; }
  /// Sets if we're printing or not
  void setPrinting(bool printing) { myPrinting = printing; }
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  double myDistance;
  bool myUseEncoders;
  double mySpeed;
  double myDeceleration;
  MvrActionDesired myDesired;
  bool myPrinting;
  double myLastVel;

  double myDistTravelled;
  MvrPose myLastPose;
  
  enum State
  {
    STATE_NO_DISTANCE, 
    STATE_ACHIEVED_DISTANCE,
    STATE_GOING_DISTANCE
  };
  State myState;
};

#endif // MVRACTIONDRIVE
