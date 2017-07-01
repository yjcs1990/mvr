#ifndef MVRACTIONCOLORFOLLOW_H
#define MVRACTIONCOLORFOLLOW_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"
#include "MvrAction.h"
#include "MvrACTS.h"
#include "MvrPTZ.h"

/// MvrActionColorFollow is an action that moves the robot toward the
/// largest ACTS blob that appears in it's current field of view.
///  @ingroup ActionClasses
class MvrActionColorFollow : public MvrAction
{
  
public:
  // Constructor
  MVREXPORT MvrActionColorFollow(const char *name, 
			       MvrACTS_1_2 *acts,
			       MvrPTZ *camera,
			       double speed = 200, 
			       int width = 160, 
			       int height = 120);
  
  // Destructor
  MVREXPORT virtual ~MvrActionColorFollow(void);
  
  // The action
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);

  // Set the ACTS channel that we want to get blob info out of
  MVREXPORT bool setChannel(int channel);

  // Set the camera that we will be controlling
  MVREXPORT void setCamera(MvrPTZ *camera);

  // Toggle whether we should try to acquire a blob
  // if one cannot be seen
  MVREXPORT void setAcquire(bool acquire);

  // Stop moving alltogether
  MVREXPORT void stopMovement(void);
  
  // Start moving
  MVREXPORT void startMovement(void);

  // Return the channel that we are looking for blobs on
  MVREXPORT int getChannel();
  
  // Return whether or not we are trying to acquire a blob
  // if we cannot see one
  MVREXPORT bool getAcquire();
  
  // Return whether or not we are moving
  MVREXPORT bool getMovement();

  // Return whether or not we can see a target
  MVREXPORT bool getBlob();

  // The state of the action
  enum TargetState 
  {
    NO_TARGET,      // There is no target in view
    TARGET          // There is a target in view
  };

  // The state of movement
  enum MoveState
  {
    FOLLOWING,     // Following a blob
    ACQUIRING,     // Searching for a blob
    STOPPED        // Sitting still
  };

  // The last seen location of the blob
  enum LocationState
  {
    LEFT,           // The blob is on the left side of the screen
    RIGHT,          // The blob is on the right side of the screen
    CENTER          // The blob is relatively close to the center
  };
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
protected:
  MvrActionDesired myDesired;
  MvrACTS_1_2 *myActs;
  MvrPTZ *myCamera;
  MvrTime myLastSeen;
  TargetState myState;
  MoveState myMove;
  LocationState myLocation;
  bool myAcquire;
  bool killMovement;
  int myChannel;
  int myMaxTime;
  int myHeight;
  int myWidth;
  double mySpeed;
};


#endif // MVRACTIONCOLORFOLLOW_H
