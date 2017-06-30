/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "MvrExport.h"
#include "ariaOSDef.h"
#include "MvrActionTriangleDriveTo.h"
#include "MvrRobot.h"
#include "MvrLaser.h"
#include "ariaInternal.h"

MVREXPORT MvrActionTriangleDriveTo::ArActionTriangleDriveTo(
	const char *name, double finalDistFromVertex,  
	double approachDistFromVertex, double speed,
	double closeDist, double acquireTurnSpeed) :
  MvrAction(name, "Finds a triangle and drives in front of it")
{
  myDataMutex.setLogName("MvrActionTriangleDriveTo::myDataMutex");
  setNextArgument(MvrArg("final dist from vertex", &myFinalDistFromVertex, 
			"Distance from vertex we want to be at (mm)"));
  myFinalDistFromVertex = finalDistFromVertex;

  setNextArgument(MvrArg("approach dist from vertex", &myApproachDistFromVertex,
			"Distance from vertex we'll go to before going to final (0 goes straight to final) (mm)"));
  myApproachDistFromVertex = approachDistFromVertex;

  setNextArgument(MvrArg("speed", &mySpeed, "speed to drive at (mm/sec)"));
  mySpeed = speed;

  setNextArgument(MvrArg("close dist", &myCloseDist, 
			"how close we have to get to our final point (mm)"));
  myCloseDist = closeDist;

  setNextArgument(MvrArg("acquire turn speed", &myAcquireTurnSpeed,
		"if we are aqcquiring the rot vel to turn at (deg/sec)"));
  myAcquireTurnSpeed = acquireTurnSpeed;

  myLaser = NULL;
  myLineFinder = NULL;
  myOwnLineFinder = false;
  myState = STATE_INACTIVE;
  mySaveData = false;
  myData = NULL;
  myVertexSeen = false;
  // set all our defaults
  setTriangleParams();
  setVertexUnseenStopMSecs();
  setAcquire();
  setIgnoreTriangleDist();
  myOriginalAngle = 1000;
  setMaxDistBetweenLinePoints();
  setMaxLateralDist();
  setMaxAngleMisalignment();
  myPrinting = false;
  myAdjustVertex = false;
  myGotoVertex = false;
  myLocalXOffset = 0;
  myLocalYOffset = 0;
  myThOffset = 0;
  myUseLegacyVertexOffset = false;
}

MVREXPORT void MvrActionTriangleDriveTo::setParameters(
	double finalDistFromVertex,  
	double approachDistFromVertex, double speed,
	double closeDist, double acquireTurnSpeed)
{
  myFinalDistFromVertex = finalDistFromVertex;
  myApproachDistFromVertex = approachDistFromVertex;
  mySpeed = speed;
  myCloseDist = closeDist;
  myAcquireTurnSpeed = acquireTurnSpeed;
}

MVREXPORT MvrActionTriangleDriveTo::~MvrActionTriangleDriveTo()
{
  if (myOwnLineFinder)
    delete myLineFinder;
}

MVREXPORT void MvrActionTriangleDriveTo::activate(void)
{
  if (myPrinting)
    MvrLog::log(MvrLog::Normal, "MvrActionTriangleDriveTo: Activating");
  MvrAction::activate();
  myVertexSeen = false;
  myVertexSeenLast.setToNow();
  myDataMutex.lock();
  if (mySaveData && myData != NULL)
    myData->setVertexSeen(false);
  myDataMutex.unlock();

  // in case we made things so early we didn't have sensors
  if (myLineFinder == NULL && myRobot != NULL)
    setRobot(myRobot);
  
  myOriginalAngle = myRobot->getTh();
  if (myLineFinder != NULL)
    myState = STATE_ACQUIRE;
  else 
    myState = STATE_FAILED;

}

MVREXPORT void MvrActionTriangleDriveTo::deactivate(void)
{
  MvrAction::deactivate();
  myState = STATE_INACTIVE;
}

MVREXPORT void MvrActionTriangleDriveTo::setRobot(MvrRobot *robot)
{
  MvrAction::setRobot(robot);
  if (myLineFinder == NULL && myRobot != NULL)
  {
    int ii;
    for (ii = 1; ii <= Mvria::getMaxNumLasers(); ii++)
    {
      if (myRobot->findLaser(ii) != NULL && 
	  myRobot->findLaser(ii)->isConnected())
      {
	myLaser = myRobot->findLaser(ii);
	myLineFinder = new MvrLineFinder(myLaser);
	myOwnLineFinder = true;
	break;
      }
    }
  }
}

MVREXPORT void MvrActionTriangleDriveTo::setLineFinder(MvrLineFinder *lineFinder)
{
  if (myLineFinder != NULL && myOwnLineFinder)
    delete myLineFinder;

  myLineFinder = lineFinder;
  myOwnLineFinder = false;
}

MVREXPORT MvrPose MvrActionTriangleDriveTo::findPoseFromVertex(
	double distFromVertex)
{
  MvrPose ret;
  MvrPose vertex;
  vertex = myRobot->getEncoderTransform().doTransform(myVertex);

  ret.setX((vertex.getX() + MvrMath::cos(vertex.getTh()) * distFromVertex));
  ret.setY((vertex.getY() + MvrMath::sin(vertex.getTh()) * distFromVertex));
  return ret;
}

/**
   This finds our vertex point and angle from vertex.  

   @internal
   
   @param initial if we're finding the initial vertex then we look for
   more in front of us than on the side and closer, if we're not
   finding the initial one we look more towards where we found it last
   time

   @param goStraight we're just driving straight in to the vertex in the second
   stage.
**/
MVREXPORT void MvrActionTriangleDriveTo::findTriangle(bool initial, 
						    bool goStraight)
{
  if (myGotLinesCounter != myRobot->getCounter())
  {
    MvrTime now;
    now.setToNow();
    myLineFinder->setMaxDistBetweenPoints(myMaxDistBetweenLinePoints);
    myLines = myLineFinder->getLines();
    //printf("took %d\n", now.mSecSince());
  }
  myGotLinesCounter = myRobot->getCounter();
  //  myLineFinder->saveLast();
  
  int start;
  int len = myLines->size();
  FILE *corners = NULL;
  /*if ((corners = MvrUtil::fopen("corners", "w+")) == NULL)
  {
    printf("Couldn't open corners file\n");
  }
  */
  // length of line 1
  double line1Dist;
  // length of line 2
  double line2Dist;
  // distance from the end of line1 to the beginning of line2
  double distLine1ToLine2;
  // difference in angle between line1 and line2
  double angleBetween;
  // difference between the angleBetween and the angleBetween we want
  double angleDelta;
  /// difference between line1 and how long it should be
  double line1Delta;
  /// difference between line2 and how long it should be
  double line2Delta;
  // the score for the line evaluated
  double lineScore;
  // our position of this vertex
  MvrPose vertex;
  // the score for our good line
  double goodLineScore = 0.0;
  // my pose for the good vertex
  MvrPose goodVertex;
  MvrLineSegment vertexLine;
  MvrPose adjustedVertex;

  MvrPose lastVertex;
  bool printing = false;

  lastVertex = myRobot->getEncoderTransform().doTransform(myVertex);

  for (start = 0; start < len; start++)
  {
    if (start + 1 < len)
    {
      line1Dist = (*myLines)[start]->getEndPoint1().findDistanceTo(
	      (*myLines)[start]->getEndPoint2());
      line2Dist = (*myLines)[start+1]->getEndPoint1().findDistanceTo(
	      (*myLines)[start+1]->getEndPoint2());
      distLine1ToLine2 = (*myLines)[start]->getEndPoint2().findDistanceTo(
	      (*myLines)[start+1]->getEndPoint1());
      angleBetween = MvrMath::subAngle(180, 
			   MvrMath::subAngle((*myLines)[start]->getLineAngle(),
					   (*myLines)[start+1]->getLineAngle()));

      if (myAngleBetween != 0)
	angleDelta = MvrMath::fabs(MvrMath::subAngle(
					  angleBetween, myAngleBetween));
      else
	angleDelta = 0;

      if (myLine1Length != 0)
	line1Delta = MvrMath::fabs(line1Dist - myLine1Length);
      else
	line1Delta = 0;

      if (myLine2Length != 0)
	line2Delta = MvrMath::fabs(line2Dist - myLine2Length);
      else
	line2Delta = 0;


      if (printing)
	ArLog::log(MvrLog::Normal, "dl1l2 %5.0f l1d %5.0f l2d %5.0f ab %5.0f thdt %5.0f l1dt %5.0f l2dt %5.0f", 
		   distLine1ToLine2,
		   line1Dist,
		   line2Dist,
		   angleBetween,
		   angleDelta,
		   line1Delta, line2Delta);

      // if any of these is true the line is just too bad and we should bail
      if (line1Delta > 125 || line2Delta > 125 ||
	  angleDelta > 15 || distLine1ToLine2 > 100)
	continue;


      MvrPose intersection;


      MvrLine *line1Line = new MvrLine(*(*myLines)[start]->getLine());
      MvrLine *line2Line = new MvrLine(*(*myLines)[start+1]->getLine());
      if (!line1Line->intersects(line2Line, &intersection))
      {
	ArLog::log(MvrLog::Terse, "MvrActionTriangeDriveTo: couldn't find intersection of lines (shouldn't happen)");
	return;
      }
      delete line1Line;
      delete line2Line;


      
      /*
	printf("corner at polar %5.0f %5.0f pose (%.0f %.0f) angleDelta %.0f %.0f\n", 
	myFinder->getLinesTakenPose().findAngleTo(intersection),
	myFinder->getLinesTakenPose().findDistanceTo(intersection),
	intersection.getX(), intersection.getY(),
	angleDelta,
	ArMath::fabs((line1Delta + line2Delta) / 2));
      */
      if (corners != NULL)
	fprintf(corners, "%.0f %.0f\n", intersection.getX(), 
		intersection.getY());
      
      vertex.setPose(intersection);
      /*
      vertex.setTh(MvrMath::subAngle(MvrMath::atan2((*myLines)[start]->getY1() - 
						  (*myLines)[start+1]->getY2(),
						  (*myLines)[start]->getX1() - 
					  (*myLines)[start+1]->getX2()),
				    90));;
      */
      // if we don't care about the angle or it's a non-inverted
      // triangle use the old way
      if (myAngleBetween > -.1)
	vertex.setTh(MvrMath::addAngle((*myLines)[start]->getLineAngle(),
				      angleBetween / 2));
      // if it's an inverted triangle flip the angle so that things
      // work right
      else
	vertex.setTh(MvrMath::addAngle(180, 
				      MvrMath::addAngle((*myLines)[start]->getLineAngle(),
						       angleBetween / 2)));

      vertexLine.newEndPoints(vertex.getX(), vertex.getY(),
		      vertex.getX() + MvrMath::cos(vertex.getTh()) * 20000,
		       vertex.getY() + MvrMath::sin(vertex.getTh()) * 20000);

      if (myMaxLateralDist > 0)
      {
	ArLineSegment robotLine;
	
	robotLine.newEndPoints(
		myRobot->getX(), myRobot->getY(),
		myRobot->getX() + MvrMath::cos(myRobot->getTh()) * 20000,
		myRobot->getY() + MvrMath::sin(myRobot->getTh()) * 20000);

	if (robotLine.getDistToLine(vertex) > myMaxLateralDist)
	{
	  if (printing)
	    MvrLog::log(MvrLog::Normal, 
		   "Robot off possible vertex line by %.0f, ignoring it", 
		       robotLine.getDistToLine(vertex));
	  continue;
	}
      }

      if (myMaxAngleMisalignment > 0)
      {
	//if (fabs(MvrMath::subAngle(MvrMath::subAngle(myRobot->getTh(), 180), 
	//vertex.getTh())) > myMaxAngleMisalignment)
	if (fabs(MvrMath::subAngle(MvrMath::subAngle(myOriginalAngle, 180), 
				  vertex.getTh())) > myMaxAngleMisalignment)
	{
	  if (printing)
	    MvrLog::log(MvrLog::Normal, 
	   "Robot misaligned from possible vertex line by %.0f (original %.0f robot %.0f line %.0f), ignoring it", 
		       fabs(MvrMath::subAngle(
				    MvrMath::subAngle(myOriginalAngle, 180), 
				    vertex.getTh())),
		       myOriginalAngle, myRobot->getTh(), vertex.getTh());
	  continue;
	}
      }

      if (myAdjustVertex) 
      {
	ArPose end1 = (*myLines)[start]->getEndPoint1();
	ArPose end2 = (*myLines)[start+1]->getEndPoint2();
	ArPose vertexLocal = vertex;
	end1 = myRobot->getToLocalTransform().doTransform(end1);
	end2 = myRobot->getToLocalTransform().doTransform(end2);
	vertexLocal = myRobot->getToLocalTransform().doTransform(vertexLocal);

	ArPose closest;
	/* old way that checked too large an area
	myLaser->currentReadingBox(0, end1.getY(),
				   MvrUtil::findMax(end1.getX(), end2.getX()),
				   end2.getY(),
				   &closest);
	*/

	// new way that looks 100 mm closer to the robot than the
	// vertex this may not be quite right because there's a chance
	// the vertex won't be closest to the robot but that will be
	// at an angle deep enough we probably won't see the other
	// side the of the triangle so we should be okay
	myLaser->currentReadingBox(vertexLocal.getX() - 100, end1.getY(),
				   MvrUtil::findMax(end1.getX(), end2.getX()),
				   end2.getY(),
				   &closest);
	closest = myRobot->getToGlobalTransform().doTransform(closest);
	//printf("%g %g %g %g close %g %g\n", end1.getX(), end1.getY(), 
	//end2.getX(), end2.getY(), closest.getX(), closest.getY());
	
	adjustedVertex.setPose(
		closest.getX(), closest.getY(),
		ArMath::addAngle((*myLines)[start]->getLineAngle(),
				 angleBetween / 2));

      }	


      //printf("One that may be good enough\n");
      // if we got here we should calculate the score for the line
      // first we calculate the score based on it matching
      lineScore = 0;
      lineScore += 10 - 1 * (line1Delta / 25);
      //printf("1 %.0f\n", lineScore);
      lineScore += 10 - 1 * (line2Delta / 25);
      //printf("2 %.0f\n", lineScore);
      lineScore += 10 - 1 * (angleDelta / 2);
      //printf("3 %.0f\n", lineScore);
      // now we calculate based on its position 
      
      // if we're in our initial one we mostly want to make sure its
      // in front of us
      if (initial)
      {
	if (printing)
	  printf("init %.0f (%.3f) %.0f\n", 
		 MvrMath::subAngle(myRobot->getTh(),
				  myRobot->getPose().findAngleTo(vertex)),
		 90 - MvrMath::fabs(myRobot->findDeltaHeadingTo(vertex)) / 90,
		 vertexLine.getDistToLine(myRobot->getPose()));
	
	//ArMath::subAngle(myRobot->getTh(),
	//myRobot->getPose().findAngleTo(vertex)))) / 90,

	// weight it more heavily for the lines in front of us
	lineScore *= .5 + .5 * (30 - 
				ArMath::fabs(
					ArMath::subAngle(myRobot->getTh(),
			       myRobot->getPose().findAngleTo(vertex)))) / 30;
	if (printing)
	  printf("angle %.0f\n", lineScore);
	lineScore *= .5 + .5 * (1500 - 
				vertexLine.getDistToLine(myRobot->getPose())) / 1500;
	if (printing)
	  printf("disttoline %.0f\n", lineScore);

	// weight it more heavily if the vertex points towards along
	// the same line as the line from the robot to the vertex
	lineScore *= .5 + .5 * (30 - fabs(MvrMath::subAngle(
						  vertex.getTh(), vertex.findAngleTo(myRobot->getPose()))));
	
	if (printing)
	  printf("anglefrompointing %.0f (%.0f %.0f)\n", lineScore,
		 vertex.getTh(), vertex.findAngleTo(myRobot->getPose()));

      }
      // for not the initial one weight them more heavily if they're
      // close to us
      else
      {
	double dist = lastVertex.findDistanceTo(vertex);
	if (dist < 100)
	  lineScore *= 1;
	if (dist < 200)
	  lineScore *= .8;
	else if (dist < 400)
	  lineScore *= .5;
	else if (dist < 600)
	  lineScore *= .2;
	else if (dist < 800)
	  lineScore *= .1;
	else
	  lineScore *= 0;

	double angleFromLast = fabs(MvrMath::subAngle(lastVertex.getTh(), 
						vertex.getTh()));
	//printf("AngleFromLast %.0f %.0f %.0f\n", angleFromLast, lastVertex.getTh(), vertex.getTh());
	if (angleFromLast < 5)
	  lineScore *= 1;
	else if (angleFromLast < 10)
	  lineScore *= .75;
	else if (angleFromLast < 20)
	  lineScore *= .5;
	else if (angleFromLast < 30)
	  lineScore *= .25;
	else 
	  lineScore *= 0;

	if (goStraight)
	{
	  double angle = fabs(myRobot->findDeltaHeadingTo(vertex));
	  if (angle < 2)
	    lineScore *= 1;
	  else if (angle < 5)
	    lineScore *= .5;
	  else if (angle < 10)
	    lineScore *= .1;
	  else
	    lineScore *= 0;
	}
      }
      if (printing)
	printf("linescore %.0f\n", lineScore);
      // if the match is too bad just bail
      if (lineScore < 5)
      {
	continue;
      }
      // otherwise see if its better then our previous one, if so set
      // our actual vertex to it
      if (goodLineScore < 1 || lineScore > goodLineScore)
      {
	if (printing)
	  printf("#### %.0f %.0f %.0f at %.0f %.0f\n", 
		 vertex.getX(), vertex.getY(), vertex.getTh(),
		 myRobot->getPose().findAngleTo(vertex),
		 myRobot->getPose().findDistanceTo(vertex));
	goodLineScore = lineScore;
	ArPose usedVertex;
	if (myAdjustVertex)
	  usedVertex = adjustedVertex;
	else
	  usedVertex = vertex;
	
	if (myLocalXOffset != 0 || myLocalYOffset != 0 || 
	    fabs(myThOffset) > .00001)
	{
	  MvrPose before = usedVertex;
	    //ArLog::log(MvrLog::Normal, 
	    //"@@ Before %.0f %.0f %.0f (%d %d %.1f)",
	    //usedVertex.getX(), usedVertex.getY(), usedVertex.getTh(),
	    //myLocalXOffset, myLocalYOffset, myThOffset);
	  // old wrong code
	  if (myUseLegacyVertexOffset)
	  {
	    MvrLog::log(MvrLog::Normal, "Legacy vertex mode...");
	    usedVertex.setX(usedVertex.getX() + 
			    myLocalXOffset * MvrMath::cos(usedVertex.getTh()) + 
			    myLocalYOffset * MvrMath::sin(usedVertex.getTh()));
	    usedVertex.setY(usedVertex.getY() - 
			    myLocalXOffset * MvrMath::sin(usedVertex.getTh()) -
			    myLocalYOffset * MvrMath::cos(usedVertex.getTh()));
	    usedVertex.setTh(MvrMath::addAngle(vertex.getTh(), myThOffset));
	    //ArLog::log(MvrLog::Normal, "@@ After %.0f %.0f %.0f (%.0f angle from before to after)",
	    //usedVertex.getX(), usedVertex.getY(), usedVertex.getTh(),
	    //before.findAngleTo(usedVertex));
	  }
	  else
	  {
	    MvrLog::log(MvrLog::Normal, "New vertex mode...");
	    // new code that uses a transform and does it right
	    // make a transform so that our existing vertex becomes the origin
	    MvrTransform localTrans(before, MvrPose(0, 0));
	    // then do an inverse transform to pull the local offset out of 
	    MvrPose transformed = localTrans.doInvTransform(MvrPose(myLocalXOffset, myLocalYOffset));
	    usedVertex.setX(transformed.getX());
	    usedVertex.setY(transformed.getY());
	    usedVertex.setTh(MvrMath::addAngle(vertex.getTh(), myThOffset));
	    //ArLog::log(MvrLog::Normal, "@@ Transformed %.0f %.0f %.0f (%.0f angle from before to after)",
	    //transformed.getX(), transformed.getY(), transformed.getTh(),
	    //before.findAngleTo(transformed));
	  }	  
	}
			  
	myVertex = myRobot->getEncoderTransform().doInvTransform(usedVertex);

	myVertexSeen = true;
	myVertexSeenLast.setToNow();
	myDataMutex.lock();
	if (mySaveData && myData != NULL)
	{
	  myData->setVertexSeenThisCycle(true);
	  myData->setVertexSeen(true);
	}
	myDataMutex.unlock();
      }
      continue;

    }
  }

  if (corners != NULL)
    fclose(corners);
  myDataMutex.lock();
  if (mySaveData && myData != NULL)
    myData->setLinesAndVertex(
	    myLines, myRobot->getEncoderTransform().doTransform(myVertex));
  myDataMutex.unlock();
}

MVREXPORT MvrActionDesired *ArActionTriangleDriveTo::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();
  double dist;
  double angle;
  double vel;
  MvrPose approach;
  MvrPose final;
  MvrPose vertex;
  
  myDataMutex.lock();
  if (myData != NULL)
  {
    delete myData;
    myData = NULL;
  }
  if (mySaveData)
    myData = new Data;
  if (mySaveData && myData != NULL)
  {
    myData->setVertexSeenThisCycle(false);
    myData->setVertexSeen(myVertexSeen);
  }
  myDataMutex.unlock();

  vertex = myRobot->getEncoderTransform().doTransform(myVertex);

  switch (myState)
  {
  case STATE_INACTIVE:
    return NULL;
  case STATE_SUCCEEDED:
  case STATE_FAILED:
    myDesired.setVel(0);
    myDesired.setRotVel(0);
    return &myDesired;
  case STATE_ACQUIRE:
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Acquire");
    findTriangle(true);
    if (!myVertexSeen && 
	myVertexSeenLast.mSecSince() > myVertexUnseenStopMSecs)
    {
      if (myAcquire)
      {
	myState = STATE_SEARCHING;
	return fire(currentDesired);
      }
      else
      {
	if (myPrinting)
	  MvrLog::log(MvrLog::Normal, "Failed");
	myState = STATE_FAILED;
	return fire(currentDesired);
      }
    } 
    if (myVertexSeen)
    {
      approach = findPoseFromVertex(myApproachDistFromVertex);
      final = findPoseFromVertex(myFinalDistFromVertex);
      if (mySaveData)
      {
	myDataMutex.lock();
	if (myData != NULL)
	{
	  myData->setApproach(approach);
	  myData->setFinal(final);
	}
	myDataMutex.unlock();
      }
      // if we aren't approaching or if its behind us go straight to
      // final
      if (myPrinting)
	ArLog::log(MvrLog::Normal, "%.0f", MvrMath::fabs(myRobot->findDeltaHeadingTo(approach)));
      if (myApproachDistFromVertex <= 0 || 
	  MvrMath::fabs(myRobot->findDeltaHeadingTo(approach)) > 90 ||
	  (MvrMath::fabs(myRobot->findDeltaHeadingTo(approach)) > 45 &&  
	   myRobot->findDistanceTo(final) < myRobot->findDistanceTo(approach)))

      {
	if (myGotoVertex)
	{
	  if (myPrinting)
	    MvrLog::log(MvrLog::Normal, "Going to vertex");
	  myState = STATE_GOTO_VERTEX;
	}
	else
	{
	  if (myPrinting)
	    MvrLog::log(MvrLog::Normal, "Going to final");
	  myState = STATE_GOTO_FINAL;
	}
	return fire(currentDesired);
      }
      // otherwise we go to our approach
      else
      {
	myState = STATE_GOTO_APPROACH;
	return fire(currentDesired);
      }
    }
    myDesired.setVel(0);
    myDesired.setRotVel(0);
    return &myDesired;
  case STATE_SEARCHING:
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Searching");
    myVertexSeen = false;
    myDataMutex.lock();
    if (mySaveData && myData != NULL)
      myData->setVertexSeen(false);
    myDataMutex.unlock();

    findTriangle(true);
    if (myVertexSeen)
    {
      myState = STATE_ACQUIRE;
      return fire(currentDesired);
    }
    myDesired.setVel(0);
    myDesired.setRotVel(myAcquireTurnSpeed);
    return &myDesired;
    break;
  case STATE_GOTO_APPROACH:
    findTriangle(false);
    if (!myVertexSeen && 
	myVertexSeenLast.mSecSince() > myVertexUnseenStopMSecs)
    {
      if (myAcquire)
      {
	myState = STATE_SEARCHING;
	return fire(currentDesired);
      }
      else
      {
	ArLog::log(MvrLog::Normal, "MvrActionTriangleDriveTo: Failed");
	myState = STATE_FAILED;
	return fire(currentDesired);
      }
    } 
    approach = findPoseFromVertex(myApproachDistFromVertex);
    if (mySaveData)
    {
      myDataMutex.lock();
      if (myData != NULL)
	myData->setApproach(approach);
      myDataMutex.unlock();
    }
    dist = myRobot->getPose().findDistanceTo(approach);
    angle = myRobot->getPose().findAngleTo(approach);
    if (dist < myCloseDist || 
	(dist < myCloseDist * 2 && 
	 MvrMath::fabs(myRobot->findDeltaHeadingTo(approach)) > 30))
    {
      if (myPrinting)
	ArLog::log(MvrLog::Normal, "Goto approach there");
      myState = STATE_ALIGN_APPROACH;
      return fire(currentDesired);
    }
    myDesired.setHeading(angle);
    vel = sqrt(dist * 400 * 2);
    vel *= (180 - MvrMath::fabs(myRobot->findDeltaHeadingTo(approach))) / 180;
    if (vel < 0)
      vel = 0;
    if (vel > mySpeed)
      vel = mySpeed;
    myDesired.setVel(vel);
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Goto approach speed %.0f dist %.0f angle %.0f", vel, dist, angle);
    return &myDesired;
  case STATE_ALIGN_APPROACH:
    angle = myRobot->getPose().findAngleTo(vertex);
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Align approach %.0f %.0f", myRobot->getTh(),
		 angle);
    if (MvrMath::fabs(MvrMath::subAngle(myRobot->getTh(), angle)) < 2 &&
	ArMath::fabs(myRobot->getVel()) < 5)
    {
      //ArLog::log(MvrLog::Normal, "finaldist %.0f", MvrMath::fabs(myFinalDistFromVertex));
      if (myGotoVertex)
      {
	if (myPrinting)
	  MvrLog::log(MvrLog::Normal, "Going to vertex");
	myState = STATE_GOTO_VERTEX;
      }
      else
      {
	if (myPrinting)
	  MvrLog::log(MvrLog::Normal, "Going to final");
	myState = STATE_GOTO_FINAL;
      }
      return fire(currentDesired);
    }
    myDesired.setHeading(angle);
    myDesired.setVel(0);
    return &myDesired;
  case STATE_GOTO_VERTEX:
    final = findPoseFromVertex(0);
    if (myUseIgnoreInGoto && 
	(myRobot->findDistanceTo(final) > 
	 myFinalDistFromVertex + myIgnoreTriangleDist))
      findTriangle(false, true);

    if (!myVertexSeen && 
	myVertexSeenLast.mSecSince() > myVertexUnseenStopMSecs)
    {
      MvrLog::log(MvrLog::Normal, "ActionTriangle: Failed");
      myState = STATE_FAILED;
      return fire(currentDesired);
    } 
    final = findPoseFromVertex(0);
    if (mySaveData)
    {
      myDataMutex.lock();
      if (myData != NULL)
	myData->setFinal(final);
      myDataMutex.unlock();
    }
    dist = myRobot->findDistanceTo(final);
    dist -= myFinalDistFromVertex;
    angle = myRobot->findDeltaHeadingTo(final);
    if (MvrMath::fabs(angle) > 10)
      {
	ArLog::log(MvrLog::Normal, "ActionTriangle: FAILING because trying to turn %.0f degrees to something %.0f away that we saw %ld ms ago ", angle, dist, myVertexSeenLast.mSecSince());
	myState = STATE_FAILED;
	return fire(currentDesired);
      }
    else
      {
	myDesired.setDeltaHeading(angle);
      }
    if (dist > 0)
      vel = sqrt(dist * 100 * 2);
    else
      vel = 0;
    if (vel < 0)
      vel = 0;
    if (vel > mySpeed)
      vel = mySpeed;
    myDesired.setVel(vel);
    if (dist <= 0 && fabs(myRobot->getVel()) < 5)
    {
      MvrLog::log(MvrLog::Normal, "MvrActionTriangleDriveTo: Succeeded (vertex) %g", dist);
      myState = STATE_SUCCEEDED;
      return fire(currentDesired);
    }
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Goto vertex speed %.0f dist %.0f angle %.0f %ld ago",
		 vel, dist, myRobot->findDeltaHeadingTo(final),
		 myVertexSeenLast.mSecSince());
    return &myDesired;

  case STATE_GOTO_FINAL:
    // see if we are close enough we just keep the same reading,
    // otherwise we get the new reading

    final = findPoseFromVertex(myFinalDistFromVertex);

    //if (myRobot->findDistanceTo(final) > 250)
    if (myRobot->findDistanceTo(final) > myIgnoreTriangleDist)
      findTriangle(false);

    if (!myVertexSeen && 
	myVertexSeenLast.mSecSince() > myVertexUnseenStopMSecs)
    {
      if (myAcquire)
      {
	myState = STATE_SEARCHING;
	return fire(currentDesired);
      }
      else
      {
	ArLog::log(MvrLog::Normal, "MvrActionTriangleDriveTo: Failed");
	myState = STATE_FAILED;
	return fire(currentDesired);
      }
    } 
    final = findPoseFromVertex(myFinalDistFromVertex);
    if (mySaveData)
    {
      myDataMutex.lock();
      if (myData != NULL)
	myData->setFinal(final);
      myDataMutex.unlock();
    }
    dist = myRobot->getPose().findDistanceTo(final);
    angle = myRobot->getPose().findAngleTo(final);
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, 
		 "final %.0f away at %.0f vertex %.0f away %ld ago", 
		 dist, myRobot->findDeltaHeadingTo(final),
		 myRobot->findDistanceTo(vertex), 
		 myVertexSeenLast.mSecSince());

    if ((dist < 5) || 
	(myRobot->getVel() > 0 && dist < myCloseDist && 
	 MvrMath::fabs(myRobot->findDeltaHeadingTo(final)) > 20) ||
	(myRobot->getVel() < 0 && dist < myCloseDist && 
	 MvrMath::fabs(myRobot->findDeltaHeadingTo(final)) < 160) ||
	(MvrMath::fabs(myRobot->getVel()) < 5 && dist < myCloseDist))
    {
      if (myPrinting)
	ArLog::log(MvrLog::Normal, "Goto final there");
      myState = STATE_ALIGN_FINAL;
      return fire(currentDesired);
    }

    if (MvrMath::fabs(MvrMath::subAngle(myRobot->getTh(),
				      angle)) < 90)
    {
      myDesired.setHeading(angle);
      vel = sqrt(dist * 100 * 2);
      vel *= (45 - MvrMath::fabs(myRobot->findDeltaHeadingTo(final))) / 45;
      if (vel < 0)
	vel = 0;
      if (vel > mySpeed)
	vel = mySpeed;
      myDesired.setVel(vel);
      if (myPrinting)
	ArLog::log(MvrLog::Normal, "Goto final speed %.0f dist %.0f angle %.0f", vel, dist, myRobot->findDeltaHeadingTo(final));

      return &myDesired;
    }
    else
    {
      myDesired.setHeading(
	      MvrMath::subAngle(myRobot->getPose().findAngleTo(final), 180));
      vel = -1 * sqrt(dist * 100 * 2);
      vel *= (45 - MvrMath::fabs(MvrMath::subAngle(180,
				 myRobot->findDeltaHeadingTo(final)))) / 45;
      if (vel > 0)
	vel = 0;
      if (vel < -mySpeed)
	vel = -mySpeed;
      myDesired.setVel(vel);
      if (myPrinting)
	ArLog::log(MvrLog::Normal, "Goto final (backing) speed %.0f dist %.0f angle %.0f (turning to %.0f)", vel, dist, angle, 
		   MvrMath::subAngle(180, myRobot->findDeltaHeadingTo(final)));
      return &myDesired;
    }
  case STATE_ALIGN_FINAL:
    angle = myRobot->getPose().findAngleTo(vertex);
    if (MvrMath::fabs(MvrMath::subAngle(myRobot->getTh(), angle)) < 2 &&
	ArMath::fabs(myRobot->getVel()) < 5)
    {
      MvrLog::log(MvrLog::Normal, "MvrActionTriangleDriveTo: Succeeded");
      myState = STATE_SUCCEEDED;
      return fire(currentDesired);
    }
    if (myPrinting)
      MvrLog::log(MvrLog::Normal, "Align final");
    myDesired.setHeading(angle);
    myDesired.setVel(0);
    return &myDesired;
  }

  findTriangle(true);
  return NULL;

}

ArActionTriangleDriveTo::Data *ArActionTriangleDriveTo::getData(void)
{
  Data *data = NULL;
  myDataMutex.lock();
  data = myData;
  myData = NULL;
  myDataMutex.unlock();
  return data;
}


