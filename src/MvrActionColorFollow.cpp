#include "MvrExport.h"

#include "mvriaOSDef.h"
#include "MvrActionColorFollow.h"
#include "MvrResolver.h"
#include "MvrRobot.h"

// Constructor: Initialize the color follow action
MVREXPORT MvrActionColorFollow::MvrActionColorFollow(const char *name, MvrACTS_1_2 *acts, 
						  MvrPTZ *camera, double speed, 
						  int width, int height) :
    MvrAction(name, "Follows the largest blob of color.")
{
  myActs = acts;
  myAcquire = true;
  killMovement = true;
  myCamera = camera;
  myChannel = 0;
  myState = NO_TARGET;
  myMove = STOPPED;
  myLocation = CENTER;
  setChannel(1);
  myMaxTime = 2000;
  myWidth = width;
  myHeight = height;
  mySpeed = speed;
}

// Destructor
MVREXPORT MvrActionColorFollow::~MvrActionColorFollow(void) {}


// The color follow action
MVREXPORT MvrActionDesired *MvrActionColorFollow::fire(MvrActionDesired currentDesired)
{
  MvrACTSBlob blob;
  MvrACTSBlob largestBlob;

  bool haveBlob = false;

  int numberOfBlobs;
  int blobArea = 20;

  double xRel, yRel;

  // Reset the desired action
  myDesired.reset();

  // If we are in fact connected to ACTS...
  if(myActs != NULL)
    {
      numberOfBlobs = myActs->getNumBlobs(myChannel);
      
      // If there are blobs to be seen, set the time to now
      if(numberOfBlobs != 0)
	{
	  // Find the largest blob
	  for(int i = 0; i < numberOfBlobs; i++)
	    {
	      myActs->getBlob(myChannel, i + 1, &blob);
	      if(blob.getArea() > blobArea)
		{
		  haveBlob = true;
		  blobArea = blob.getArea();
		  largestBlob = blob;
		}
	    }
	  
	  myLastSeen.setToNow();
	}
    
      // Decide the state of myState: do we have a target or not
      if (myLastSeen.mSecSince() > myMaxTime)
	{
	  myState = NO_TARGET;
	}
      else
	{
	  myState = TARGET;
	}
      
      // If we have a target start following
      if(myState == TARGET) myMove = FOLLOWING;
      // If not and we are allowed to acquire one, start searching
      else if(myState == NO_TARGET && myAcquire)
	{
	  myMove = ACQUIRING;
	}
      // Otherwise stay still
      else myMove = STOPPED;
    }

  // We are not connected to acts, dont do anything
  else
    {
      myState = NO_TARGET;
      myMove = STOPPED;
    }

  // If we are following a blob of color
  if(myMove == FOLLOWING)
    {
      if(haveBlob)
	{ 
	  // Determine where the largest blob's center of gravity
	  // is relative to the center of the camera
	  xRel = (double)(largestBlob.getXCG() - myWidth/2.0)  / (double)myWidth;
	  yRel = (double)(largestBlob.getYCG() - myHeight/2.0) / (double)myHeight;
	  
	  // Tilt the camera toward the blob
	  if(!(MvrMath::fabs(yRel) < .20) && myCamera != NULL)
	    {
	      if (-yRel > 0)
		myCamera->tiltRel(1);
	      else
		myCamera->tiltRel(-1);
	    }

	  // If we can move, move after that blob
	  if(!killMovement)
	    {
	      // Set the heading and velocity for the robot
	      if (MvrMath::fabs(xRel) < .10)
		{
		  myLocation = CENTER;
		  myDesired.setDeltaHeading(0);
		  myDesired.setVel(mySpeed);
		  return &myDesired;
		}
	      else
		{
		  if (MvrMath::fabs(-xRel * 10) <= 10)
		    {
		      if(xRel < 0) myLocation = RIGHT;
		      if(xRel > 0) myLocation = LEFT;
		      myDesired.setDeltaHeading(-xRel * 10);
		    }
		  else if (-xRel > 0)
		    {
		      myLocation = RIGHT;
		      myDesired.setDeltaHeading(10);
		    }
		  else
		    {
		      myLocation = LEFT;
		      myDesired.setDeltaHeading(-10);
		    }
		  
		  myDesired.setVel(mySpeed);
		  return &myDesired;    
		}
	    }
	}
      // If we do not have a blob in sight, yet are still
      // supposed to be following one... 
      else if(!killMovement)
	{
	  // head toward where the blob was last seen
	  if(myLocation == LEFT)
	    {
	      myDesired.setDeltaHeading(-10);
	      myDesired.setVel(0);
	    }
	  if(myLocation == RIGHT)
	    {
	      myDesired.setDeltaHeading(10);
	      myDesired.setVel(0);
	    }
	  if(myLocation == CENTER)
	    {
	      myDesired.setDeltaHeading(0);
	      myDesired.setVel(mySpeed);
	    }
	
	  return &myDesired; 
	}
    }
  // If we are acquiring, keep turning around in a circle
  else if(myMove == ACQUIRING)
    {
      if(myCamera != NULL) myCamera->tilt(0);

      if (!killMovement)
	{
	  myDesired.setVel(0);
	  myDesired.setDeltaHeading(8);
	  return &myDesired;
	}
    }
  
  // If we dont meet one of the above conditions, just sit tight
  myDesired.setVel(0);
  myDesired.setDeltaHeading(0);
  return &myDesired;
}

// Set the channel that the blob info will be obtained from
MVREXPORT bool MvrActionColorFollow::setChannel(int channel)
{
  // Make sure that the requested channel is in range
  if (channel >= 1 && channel <= MvrACTS_1_2::NUM_CHANNELS)
  {
    myChannel = channel;
    return true;
  }
  else
    return false;
}

// Set the camera that we are controlling
MVREXPORT void MvrActionColorFollow::setCamera(MvrPTZ *camera)
{
  myCamera = camera;
}

// Toggle whether or not we should attempt to acquire a
// target or just stay still
MVREXPORT void MvrActionColorFollow::setAcquire(bool acquire)
{
  myAcquire = acquire;
}

// Kill movement
MVREXPORT void MvrActionColorFollow::stopMovement(void)
{
  killMovement = true;
}

// Start moving
MVREXPORT void MvrActionColorFollow::startMovement(void)
{
  killMovement = false;
}

// Return the channel that we are looking on
MVREXPORT int MvrActionColorFollow::getChannel(void)
{
  return myChannel;
}

// Return if we are actively trying to find a blob
// if one cannot be seen
MVREXPORT bool MvrActionColorFollow::getAcquire(void)
{
  return myAcquire;
}

// Return whether we are supposed to be moving or not
MVREXPORT bool MvrActionColorFollow::getMovement(void)
{
  return !killMovement;
}

// Return whether we can see a target or not
MVREXPORT bool MvrActionColorFollow::getBlob(void)
{
  if(myState == TARGET) return true;
  else return false;
}
