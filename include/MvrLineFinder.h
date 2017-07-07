#ifndef MVRSICKLINEFINDER_H
#define MVRSICKLINEFINDER_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "mvriaUtil.h"
#include <vector>

class MvrLineFinderSegment;
class MvrConfig;

/** This class finds lines out of any range device with raw readings (lasers for instance)
 @ingroup OptionalClasses
 @ingroup UtilityClasses
*/
class MvrLineFinder
{
public:
  /// Constructor
  MVREXPORT MvrLineFinder(MvrRangeDevice *dev);
  /// Destructor
  MVREXPORT virtual ~MvrLineFinder();

#ifndef SWIG
  /// Finds the lines and returns a pointer to MvrLineFinder's map of them 
  /** @swigomit */
  MVREXPORT std::map<int, MvrLineFinderSegment *> *getLines(void);
  /// Finds the lines, but then returns a pointer to MvrLineFinder's map of the points that AREN'T in lines
  /** @swigomit */
  MVREXPORT std::map<int, MvrPose> *getNonLinePoints(void);
#endif
  /// Finds the lines, then copies @b pointers to them them into a new set
  MVREXPORT std::set<MvrLineFinderSegment*> getLinesAsSet();
  /// Finds the lines, and then copies the points that AREN'T in the lines into a new set
  MVREXPORT std::set<MvrPose> getNonLinePointsAsSet();

  /// Gets the robot pose at which the data from the range device (provided in
  /// constructor) was received
  MvrPose getLinesTakenPose(void) { return myPoseTaken; }
  /// Logs all the points and lines from the last getLines
  MVREXPORT void saveLast(void);
  /// Gets the lines, then prints them
  MVREXPORT void getLinesAndSaveThem(void);
  /// Whether to print verbose information about line decisions
  void setVerbose(bool verbose) { myPrinting = verbose; }
  /// Whether to print verbose information about line decisions
  bool getVerbose(void) { return myPrinting; }
  /// Sets some parameters for line creation
  void setLineCreationParams(int minLineLen = 40, int minLinePoints = 2)
    { myMakingMinLen = minLineLen; myMakingMinPoints = minLinePoints; }
  /// Sets some parameters for combining two possible lines into one line.
  void setLineCombiningParams(int angleTol = 30, int linesCloseEnough = 75) 
    { myCombiningAngleTol = angleTol; 
    myCombiningLinesCloseEnough = linesCloseEnough; }
  /// Filter out lines based on line length or number of points in the line.
  void setLineFilteringParams(int minPointsInLine = 3, int minLineLength = 75)
    { myFilteringMinPointsInLine = minPointsInLine; 
    myFilteringMinLineLength = minLineLength; }
  /// Don't let lines happen that have points not close to it
  void setLineValidParams(int maxDistFromLine = 30, 
			  int maxAveDistFromLine = 20)
    { myValidMaxDistFromLine = maxDistFromLine; 
    myValidMaxAveFromLine = maxAveDistFromLine; }

  /** Sets a maximum distance for points to be included in the same line. If
   points are greater than this distance apart, then they will not be
   considered in the same line.  If this value is 0, then there is no
   maximum-distance condition and points will be considered part of a line no
   matter how far apart.
  */
  void setMaxDistBetweenPoints(int maxDistBetweenPoints = 0)
    { myMaxDistBetweenPoints = maxDistBetweenPoints; }

  /// Add this MvrLineFinder's parameters to the given MvrConfig object.
  MVREXPORT void addToConfig(MvrConfig *config,
			    const char *section);
protected:
  // where the readings were taken
  MvrPose myPoseTaken;
  // our points
  std::map<int, MvrPose> *myPoints;
  std::map<int, MvrLineFinderSegment *> *myLines;
  std::map<int, MvrPose> *myNonLinePoints;
  // fills up the myPoints vmvriable from sick laser
  MVREXPORT void fillPointsFromLaser(void);
  // fills up the myLines vmvriable from the myPoints
  MVREXPORT void findLines(void);
  // cleans the lines and puts them into myLines 
  MVREXPORT bool combineLines();
  // takes two segments and sees if it can average them
  MVREXPORT MvrLineFinderSegment *averageSegments(MvrLineFinderSegment *line1, 
					  MvrLineFinderSegment *line2);
  // removes lines that don't have enough points added in
  MVREXPORT void filterLines();

  bool myFlippedFound;
  bool myFlipped;
  int myValidMaxDistFromLine;
  int myValidMaxAveFromLine;
  int myMakingMinLen;
  int myMakingMinPoints;
  int myCombiningAngleTol;
  int myCombiningLinesCloseEnough;
  int myFilteringMinPointsInLine;
  int myFilteringMinLineLength;
  int myMaxDistBetweenPoints;
  double mySinMultiplier;
  bool myPrinting;
  MvrRangeDevice *myRangeDevice;
};

/// Class for MvrLineFinder to hold more info than an MvrLineSegment
class MvrLineFinderSegment : public MvrLineSegment
{
public:
  MvrLineFinderSegment() {}
  MvrLineFinderSegment(double x1, double y1, double x2, double y2, 
		      int numPoints = 0, int startPoint = 0, int endPoint = 0)
    { newEndPoints(x1, y1, x2, y2, numPoints, startPoint, endPoint); }
  virtual ~MvrLineFinderSegment() {}
  void newEndPoints(double x1, double y1, double x2, double y2, 
		    int numPoints = 0, int startPoint = 0, int endPoint = 0)
    {
      MvrLineSegment::newEndPoints(x1, y1, x2, y2);
      myLineAngle = MvrMath::atan2(y2 - y1, x2 - x1);
      myLength = MvrMath::distanceBetween(x1, y1, x2, y2);
      myNumPoints = numPoints;
      myStartPoint = startPoint;
      myEndPoint = endPoint;
      myAveDistFromLine = 0;
    }
  double getLineAngle(void) { return myLineAngle; }
  double getLength(void) { return myLength; }
  int getNumPoints(void) { return myNumPoints; }
  int getStartPoint(void) { return myStartPoint; }
  int getEndPoint(void) { return myEndPoint; }
  void setAveDistFromLine(double aveDistFromLine) 
    { myAveDistFromLine = aveDistFromLine; }
  double getAveDistFromLine(void) { return myAveDistFromLine; }
protected:
  double myLineAngle;
  double myLength;
  int myNumPoints;
  int myStartPoint;
  int myEndPoint;
  double myAveDistFromLine;
};

#endif // ARSICKLINEFINDER_H
