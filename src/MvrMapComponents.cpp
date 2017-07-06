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
#include "MvrMapComponents.h"

#include <algorithm>
#include <iterator>
#ifdef WIN32
#include <process.h>
#endif 
#include <ctype.h>

#include "MvrFileParser.h"
#include "MvrMapUtils.h"
#include "MvrMD5Calculator.h"

//#define ARDEBUG_MAP_COMPONENTS
#ifdef ARDEBUG_MAP_COMPONENTS
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

// ---------------------------------------------------------------------------- 
// MvrMapScan
// ---------------------------------------------------------------------------- 

const char *MvrMapScan::EOL_CHARS = "";

MVREXPORT MvrMapScan::MvrMapScan(const char *scanType) :

  myScanType(!MvrUtil::isStrEmpty(scanType) ? scanType : ""),
  myIsSummaryScan(isSummaryScanType(scanType)),
  myLogPrefix(),
  myKeywordPrefix(),
  myPointsKeyword(),
  myLinesKeyword(),
  myTimeChanged(),
  myDisplayString(!MvrUtil::isStrEmpty(scanType) ? scanType : ""),
  myNumPoints(0),
  myNumLines(0),
  myResolution(0),
  myMax(),
  myMin(),
  myLineMax(),
  myLineMin(),

  myIsSortedPoints(false),
  myIsSortedLines(false),

  myPoints(),
  myLines(),

  myMinPosCB(this, &MvrMapScan::handleMinPos),
  myMaxPosCB(this, &MvrMapScan::handleMaxPos),
  myIsSortedPointsCB(this, &MvrMapScan::handleIsSortedPoints),
  myNumPointsCB(this, &MvrMapScan::handleNumPoints),

  myLineMinPosCB(this, &MvrMapScan::handleLineMinPos),
  myLineMaxPosCB(this, &MvrMapScan::handleLineMaxPos),
  myIsSortedLinesCB(this, &MvrMapScan::handleIsSortedLines),
  myNumLinesCB(this, &MvrMapScan::handleNumLines),

  myResolutionCB(this, &MvrMapScan::handleResolution),
  myDisplayStringCB(this, &MvrMapScan::handleDisplayString),
  //myDataCB(this, &MvrMapScan::handleData),
  //myLinesCB(this, &MvrMapScan::handleLines),
  myPointCB(this, &MvrMapScan::handlePoint),
  myLineCB(this, &MvrMapScan::handleLine)
{

  if (isDefaultScanType(myScanType.c_str()) ||
      (MvrUtil::strcasecmp(myScanType.c_str(), "SickLaser") == 0)) {
    myKeywordPrefix = "";
    myPointsKeyword = "DATA";
    myLinesKeyword = "LINES";
  }
  else {
    myKeywordPrefix = myScanType;
    // TODO Any way to do an upper?
    myPointsKeyword = myScanType + "_DATA";
    myLinesKeyword = myScanType + "_LINES";
  }

  myLogPrefix = myScanType;
  if (!myLogPrefix.empty()) {
    myLogPrefix += " ";
  }

} // end constructor


MVREXPORT MvrMapScan::MvrMapScan(const MvrMapScan &other) :
  myScanType(other.myScanType),
  myIsSummaryScan(other.myIsSummaryScan),
  myLogPrefix(other.myLogPrefix),
  myKeywordPrefix(other.myKeywordPrefix),
  myPointsKeyword(other.myPointsKeyword),
  myLinesKeyword(other.myLinesKeyword),
  myTimeChanged(other.myTimeChanged),
  myDisplayString(other.myDisplayString),
  myNumPoints(0),  // Set later
  myNumLines(0),   // Set later
  myResolution(other.myResolution),
  myMax(other.myMax),
  myMin(other.myMin),
  myLineMax(other.myLineMax),
  myLineMin(other.myLineMin),
  myIsSortedPoints(other.myIsSortedPoints),
  myIsSortedLines(other.myIsSortedLines),
  myPoints(other.myPoints),
  myLines(other.myLines),

  // Not entirely sure what to do with these in a copy ctor situation...
  // but this seems safest
  myMinPosCB(this, &MvrMapScan::handleMinPos),
  myMaxPosCB(this, &MvrMapScan::handleMaxPos),
  myIsSortedPointsCB(this, &MvrMapScan::handleIsSortedPoints),
  myNumPointsCB(this, &MvrMapScan::handleNumPoints),

  myLineMinPosCB(this, &MvrMapScan::handleLineMinPos),
  myLineMaxPosCB(this, &MvrMapScan::handleLineMaxPos),
  myIsSortedLinesCB(this, &MvrMapScan::handleIsSortedLines),
  myNumLinesCB(this, &MvrMapScan::handleNumLines),

  myResolutionCB(this, &MvrMapScan::handleResolution),
  myDisplayStringCB(this, &MvrMapScan::handleDisplayString),
  //myDataCB(this, &MvrMapScan::handleData),
  //myLinesCB(this, &MvrMapScan::handleLines),
  myPointCB(this, &MvrMapScan::handlePoint),
  myLineCB(this, &MvrMapScan::handleLine)
{

  if (!myIsSummaryScan) {
    myNumLines = other.myLines.size();
  }
  else {
    myNumLines = other.myNumLines;
  }
  if (myNumLines != other.myNumLines) {
    MvrLog::log(MvrLog::Normal,
              "%sMvrMapScan copy constructor adjusted numLines from %i to %i",
              myLogPrefix.c_str(),
              other.myNumLines,
              myNumLines);
  }

  if (!myIsSummaryScan) {
    myNumPoints = other.myPoints.size();
  }
  else {
    myNumPoints = other.myNumPoints;
  }
  if (myNumPoints != other.myNumPoints) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan copy constructor adjusted numPoints from %i to %i",
               myLogPrefix.c_str(),
               other.myNumPoints,
               myNumPoints);
  }

} // end copy constructor


MVREXPORT MvrMapScan &MvrMapScan::operator=(const MvrMapScan &other) 
{
  if (&other != this) {
  
    myScanType = other.myScanType;
    myIsSummaryScan = other.myIsSummaryScan;

    myLogPrefix     = other.myLogPrefix;
    myKeywordPrefix = other.myKeywordPrefix;
    myPointsKeyword = other.myPointsKeyword;
    myLinesKeyword  = other.myLinesKeyword;

    myTimeChanged = other.myTimeChanged;
    myDisplayString = other.myDisplayString;

    //myNumPoints   = other.myNumPoints;
    //myNumLines = other.myNumLines;
    if (!myIsSummaryScan) {
      myNumLines = other.myLines.size();
    }
    else {
      myNumLines = other.myNumLines;
    }
    if (myNumLines != other.myNumLines) {
      MvrLog::log(MvrLog::Normal,
                "%sMvrMapScan operator= adjusted numLines from %i to %i",
                myLogPrefix.c_str(),
                other.myNumLines,
                myNumLines);
    }

    if (!myIsSummaryScan) {
      myNumPoints = other.myPoints.size();
    }
    else {
      myNumPoints = other.myNumPoints;
    }
    if (myNumPoints != other.myNumPoints) {
      MvrLog::log(MvrLog::Normal,
                "%sMvrMapScan operator= adjusted numPoints from %i to %i",
                myLogPrefix.c_str(),
                other.myNumPoints,
                myNumPoints);
    }

    myResolution = other.myResolution;
    myMax = other.myMax;
    myMin = other.myMin;
    myLineMax = other.myLineMax;
    myLineMin = other.myLineMin;
    myIsSortedPoints = other.myIsSortedPoints;
    myIsSortedLines = other.myIsSortedLines;
    myPoints = other.myPoints;
    myLines = other.myLines;
  }
  return *this;
}


MVREXPORT MvrMapScan::~MvrMapScan()
{
}

MVREXPORT bool MvrMapScan::addToFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }
  if (!addHandlerToFileParser(fileParser, "MinPos:", &myMinPosCB) ||
      !addHandlerToFileParser(fileParser, "MaxPos:", &myMaxPosCB) ||
      !addHandlerToFileParser(fileParser, "NumPoints:", &myNumPointsCB) ||
      !addHandlerToFileParser(fileParser, "PointsAreSorted:", &myIsSortedPointsCB) ||
      !addHandlerToFileParser(fileParser, "LineMinPos:", &myLineMinPosCB) ||
      !addHandlerToFileParser(fileParser, "LineMaxPos:", &myLineMaxPosCB) ||
      !addHandlerToFileParser(fileParser, "NumLines:", &myNumLinesCB) ||
      !addHandlerToFileParser(fileParser, "LinesAreSorted:", &myIsSortedLinesCB) ||
      !addHandlerToFileParser(fileParser, "Resolution:", &myResolutionCB) ||
      !addHandlerToFileParser(fileParser, "Display:", &myDisplayStringCB))
       {
    MvrLog::log(MvrLog::Terse, 
               "%sMvrMapScan::addToFileParser: could not add handlers",
               myLogPrefix.c_str());
    return false;
  }  
  MvrLog::log(MvrLog::Verbose,
             "%sMvrMapScan::addToFileParser() successfully added handlers",
             myLogPrefix.c_str());

  return true;

} // end method addToFileParser


MVREXPORT bool MvrMapScan::remFromFileParser(MvrFileParser *fileParser)
{
 if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myMinPosCB);
  fileParser->remHandler(&myMaxPosCB);
  fileParser->remHandler(&myNumPointsCB);
  fileParser->remHandler(&myIsSortedPointsCB);

  fileParser->remHandler(&myLineMinPosCB);
  fileParser->remHandler(&myLineMaxPosCB);
  fileParser->remHandler(&myNumLinesCB);
  fileParser->remHandler(&myIsSortedLinesCB);

  fileParser->remHandler(&myResolutionCB);
  fileParser->remHandler(&myDisplayStringCB);
 
  return true;

} // end method remFromFileParser


MVREXPORT bool MvrMapScan::addExtraToFileParser(MvrFileParser *fileParser,
                                              bool isAddLineHandler)
{
  if (fileParser == NULL) {
    return false;
  }
  if (isAddLineHandler) {
    if (!addHandlerToFileParser(fileParser, NULL, &myLineCB)) {
      return false;
    }
  }
  else {
    if (!addHandlerToFileParser(fileParser, NULL, &myPointCB)) {
      return false;
    }
  }
  return true;

} // end method addExtraToFileParser


MVREXPORT bool MvrMapScan::remExtraFromFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myLineCB);
  fileParser->remHandler(&myPointCB);

  return true;

} // end method remExtraFromFileParser



MVREXPORT MvrTime MvrMapScan::getTimeChanged() const
{
  return myTimeChanged;
}
  

MVREXPORT void MvrMapScan::clear()
{
  myTimeChanged.setToNow();
  myNumPoints = 0;
  myNumLines  = 0;
  myResolution = 0;
  myMax.setPose(0, 0);
  myMin.setPose(0, 0);
  myLineMax.setPose(0, 0);
  myLineMin.setPose(0, 0);
  myIsSortedPoints = false;
  myIsSortedLines = false;

  myPoints.clear();
  myLines.clear();

} // end method clear

MVREXPORT const char *MvrMapScan::getDisplayString(const char *scanType)
{
  return myDisplayString.c_str();
}

MVREXPORT std::vector<MvrPose> *MvrMapScan::getPoints(const char *scanType)
{
  return &myPoints;
}

MVREXPORT std::vector<MvrLineSegment> *MvrMapScan::getLines(const char *scanType)
{
  return &myLines;
}

MVREXPORT MvrPose MvrMapScan::getMinPose(const char *scanType)
{
  return myMin;
}

MVREXPORT MvrPose MvrMapScan::getMaxPose(const char *scanType)
{
  return myMax;
}

MVREXPORT int MvrMapScan::getNumPoints(const char *scanType)
{
  return myNumPoints;
}

MVREXPORT MvrPose MvrMapScan::getLineMinPose(const char *scanType)
{
  return myLineMin;
}

MVREXPORT MvrPose MvrMapScan::getLineMaxPose(const char *scanType)
{
  return myLineMax;
}

MVREXPORT int MvrMapScan::getNumLines(const char *scanType)
{ 
  return myNumLines;
}

MVREXPORT int MvrMapScan::getResolution(const char *scanType)
{
  return myResolution;
}

MVREXPORT bool MvrMapScan::isSortedPoints(const char *scanType) const
{
  return myIsSortedPoints;
}

MVREXPORT bool MvrMapScan::isSortedLines(const char *scanType) const
{
  return myIsSortedLines;
}


MVREXPORT void MvrMapScan::setPoints(const std::vector<MvrPose> *points,
                                   const char *scanType,
                                   bool isSorted,
                                   MvrMapChangeDetails *changeDetails)
{
  if (!myIsSortedPoints) {
	  std::sort(myPoints.begin(), myPoints.end());
    myIsSortedPoints = true;
  }

  const std::vector<MvrPose> *newPoints = points;
  std::vector<MvrPose> *pointsCopy = NULL;

  if (!isSorted && (points != NULL)) {
	  pointsCopy = new std::vector<MvrPose>(*points);
	  std::sort(pointsCopy->begin(), pointsCopy->end());
    newPoints = pointsCopy;
  }

  if (changeDetails != NULL) {
    
    if (newPoints != NULL) {
    
      MvrTime timeToDiff;

      set_difference(myPoints.begin(), myPoints.end(), 
                     newPoints->begin(), newPoints->end(),
                     std::inserter(*(changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::DELETIONS, scanType)), 
                              changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::DELETIONS, scanType)->begin()));
      set_difference(newPoints->begin(), newPoints->end(),
                     myPoints.begin(), myPoints.end(), 
                     std::inserter(*(changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::ADDITIONS, scanType)), 
                              changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::ADDITIONS, scanType)->begin()));

      MvrLog::log(MvrLog::Normal,
                 "%sMvrMapScan::setPoints() %i points were deleted, %i added",
                 myLogPrefix.c_str(),
                 changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::DELETIONS, scanType)->size(),
                 changeDetails->getChangedPoints
                                    (MvrMapChangeDetails::ADDITIONS, scanType)->size());

      long int elapsed = timeToDiff.mSecSince();

      MvrLog::log(MvrLog::Normal,
                "%sMvrMapScan::setPoints() took %i msecs to find changes in %i points for %s",
                myLogPrefix.c_str(),
                elapsed,
                myNumPoints,
                scanType);

    }
    else { // null points means none added and all deleted

      *(changeDetails->getChangedPoints(MvrMapChangeDetails::DELETIONS, scanType)) = myPoints;
    }
  } // end if track changes

  int origNumPoints = myNumPoints;
  MvrPose origMin = myMin;
  MvrPose origMax = myMax;

  myTimeChanged.setToNow();

  if ((newPoints != NULL) && (!newPoints->empty())) {

    MvrTime timeToCopy;

    double maxX = INT_MIN;
    double maxY = INT_MIN;
    double minX = INT_MAX;
    double minY = INT_MAX;

    for (std::vector<MvrPose>::const_iterator it = newPoints->begin(); 
         it != newPoints->end(); 
         it++)
    {
      const MvrPose &pose = (*it);

      if (pose.getX() > maxX)
        maxX = pose.getX();
      if (pose.getX() < minX)
        minX = pose.getX();

      if (pose.getY() > maxY)
        maxY = pose.getY();
      if (pose.getY() < minY)
        minY = pose.getY();
     
    } // end for each point

    myPoints = *newPoints;  
    if (myNumPoints != (int) myPoints.size()) {

      MvrLog::log(MvrLog::Normal,
                 "%sMvrMapScan::setPoints() point count changed from %i to %i",
                 myLogPrefix.c_str(),
                 myNumPoints,
                 myPoints.size());

      myNumPoints = myPoints.size();
    } 
    myMax.setPose(maxX, maxY);
    myMin.setPose(minX, minY);

    long int elapsed = timeToCopy.mSecSince();

    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::setPoints() took %i msecs to find min/max of %i points",
               myLogPrefix.c_str(),
               elapsed,
               myNumPoints);

  } // end if new points
  else { // no new points

    myPoints.clear();
    myMax.setX(INT_MIN);
    myMax.setY(INT_MIN);
    myMin.setX(INT_MAX);
    myMin.setY(INT_MAX);
    myNumPoints = 0;

  } // end else no new points


  if (changeDetails != NULL) {

    MvrMapFileLineSetWriter deletionWriter(changeDetails->getChangedSummaryLines
                                          (MvrMapChangeDetails::DELETIONS, scanType));
    MvrMapFileLineSetWriter additionWriter(changeDetails->getChangedSummaryLines
                                          (MvrMapChangeDetails::ADDITIONS, scanType));

    if (origNumPoints != myNumPoints) {
      MvrUtil::functorPrintf(&deletionWriter, "%sNumPoints: %d%s",
                            getKeywordPrefix(),
			                      origNumPoints, EOL_CHARS);
      MvrUtil::functorPrintf(&additionWriter, "%sNumPoints: %d%s", 
                            getKeywordPrefix(),
			                      myNumPoints, EOL_CHARS);
    }

    if (origMin != myMin) {
      if (origNumPoints != 0) {
        MvrUtil::functorPrintf(&deletionWriter, "%sMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origMin.getX(), origMin.getY(),  EOL_CHARS);
      }
      if (myNumPoints != 0) {
        MvrUtil::functorPrintf(&additionWriter, "%sMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myMin.getX(), myMin.getY(),  EOL_CHARS);
      }
    } // end if min changed
    if (origMax != myMax) {
      if (origNumPoints != 0) {
        MvrUtil::functorPrintf(&deletionWriter, "%sMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origMax.getX(), origMax.getY(),  EOL_CHARS);
      }
      if (myNumPoints != 0) {
        MvrUtil::functorPrintf(&additionWriter, "%sMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myMax.getX(), myMax.getY(),  EOL_CHARS);
      }
    } // end if min changed

  } // end if track changes

  if (pointsCopy != NULL) {
    delete pointsCopy;
  }

} // end method setPoints


MVREXPORT void MvrMapScan::setLines(const std::vector<MvrLineSegment> *lines,
                                  const char *scanType,
                                  bool isSorted,
                                  MvrMapChangeDetails *changeDetails)
{
  if (!myIsSortedLines) {
	  std::sort(myLines.begin(), myLines.end());
    myIsSortedLines = true;
  }
 
  const std::vector<MvrLineSegment> *newLines = lines;
  std::vector<MvrLineSegment> *linesCopy = NULL;

  if (!isSorted && (lines != NULL)) {
	  linesCopy = new std::vector<MvrLineSegment>(*lines);
	  std::sort(linesCopy->begin(), linesCopy->end());
    newLines = linesCopy;
  }


 if (changeDetails != NULL) {

    if (newLines != NULL) {
 
      set_difference(myLines.begin(), myLines.end(), 
                     newLines->begin(), newLines->end(),
                     std::inserter(*(changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::DELETIONS, scanType)), 
                              changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::DELETIONS, scanType)->begin()));
      set_difference(newLines->begin(), newLines->end(),
                     myLines.begin(), myLines.end(), 
                     std::inserter(*(changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::ADDITIONS, scanType)), 
                              changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::ADDITIONS, scanType)->begin()));

      MvrLog::log(MvrLog::Normal,
                 "%sMvrMapScan::setLines() %i lines were deleted, %i added",
                 myLogPrefix.c_str(),
                 changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::DELETIONS, scanType)->size(),
                 changeDetails->getChangedLineSegments
                                    (MvrMapChangeDetails::ADDITIONS, scanType)->size());

    }
    else { // null lines means none added and all deleted

      *(changeDetails->getChangedLineSegments(MvrMapChangeDetails::DELETIONS, scanType)) 
                  = myLines;
    }
  } // end if track changes
 

  int origNumLines = myNumLines;
  MvrPose origLineMin = myLineMin;
  MvrPose origLineMax = myLineMax;

  myTimeChanged.setToNow();

  if ((newLines != NULL) && (!newLines->empty())) {

    MvrTime timeToCopy;

    double maxX = INT_MIN;
    double maxY = INT_MIN;
    double minX = INT_MAX;
    double minY = INT_MAX;

    for (std::vector<MvrLineSegment>::const_iterator it = newLines->begin(); 
         it != newLines->end(); 
         it++)
    {
      const MvrLineSegment &line = (*it);

      if (line.getX1() > maxX)
        maxX = line.getX1();
      if (line.getX1() < minX)
        minX = line.getX1();

      if (line.getY1() > maxY)
        maxY = line.getY1();
      if (line.getY1() < minY)
        minY = line.getY1();
      
      if (line.getX2() > maxX)
        maxX = line.getX2();
      if (line.getX2() < minX)
        minX = line.getX2();

      if (line.getY2() > maxY)
        maxY = line.getY2();
      if (line.getY2() < minY)
        minY = line.getY2();
 
    } // end for each line

    myLines = *newLines;  
   
    if (myNumLines != (int) myLines.size()) {
      MvrLog::log(MvrLog::Normal,
                 "%sMvrMapScan::setLines() line count changed from %i to %i",
                 myLogPrefix.c_str(),
                 myNumLines,
                 myLines.size());
      myNumLines = myLines.size();
    }

    myLineMax.setPose(maxX, maxY);
    myLineMin.setPose(minX, minY);

    long int elapsed = timeToCopy.mSecSince();

    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::setLines() took %i msecs to find min/max of %i lines",
               myLogPrefix.c_str(),
               elapsed,
               myNumLines);

  } // end if new lines
  else { // no new lines

    myLines.clear();
    myLineMax.setX(INT_MIN);
    myLineMax.setY(INT_MIN);
    myLineMin.setX(INT_MAX);
    myLineMin.setY(INT_MAX);
    myNumLines = 0;

  } // end else no new lines


  if (changeDetails != NULL) {

    MvrMapFileLineSetWriter deletionWriter(changeDetails->getChangedSummaryLines
                                          (MvrMapChangeDetails::DELETIONS, scanType));
    MvrMapFileLineSetWriter additionWriter(changeDetails->getChangedSummaryLines
                                          (MvrMapChangeDetails::ADDITIONS, scanType));

    if (origNumLines != myNumLines) {
      MvrUtil::functorPrintf(&deletionWriter, "%sNumLines: %d%s", 
                            getKeywordPrefix(),
			                      origNumLines, EOL_CHARS);
      MvrUtil::functorPrintf(&additionWriter, "%sNumLines: %d%s", 
                            getKeywordPrefix(),
			                      myNumLines, EOL_CHARS);
    }

    if (origLineMin != myLineMin) {
      if (origNumLines != 0) {
        MvrUtil::functorPrintf(&deletionWriter, "%sLineMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origLineMin.getX(), origLineMin.getY(),  EOL_CHARS);
      }
      if (myNumLines != 0) {
        MvrUtil::functorPrintf(&additionWriter, "%sLineMinPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myLineMin.getX(), myLineMin.getY(),  EOL_CHARS);
      }
    } // end if min changed

    if (origLineMax != myLineMax) {
      if (origNumLines != 0) {
        MvrUtil::functorPrintf(&deletionWriter, "%sLineMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        origLineMax.getX(), origLineMax.getY(),  EOL_CHARS);
      }
      if (myNumLines != 0) {
        MvrUtil::functorPrintf(&additionWriter, "%sLineMaxPos: %.0f %.0f%s", 
                              getKeywordPrefix(),
			                        myLineMax.getX(), myLineMax.getY(),  EOL_CHARS);
      }
    } // end if max changed

  } // end if track changes

  if (linesCopy != NULL) {
    delete linesCopy;
  }

} // end method setLines


MVREXPORT void MvrMapScan::setResolution(int resolution,
                                       const char *scanType,
                                       MvrMapChangeDetails *changeDetails)
{


  if (myResolution == resolution) {
    return;
  }

  MvrMapFileLineSet origLines;

  if (changeDetails != NULL) {
    MvrMapFileLineSetWriter origWriter(changeDetails->getChangedSummaryLines
                                  (MvrMapChangeDetails::DELETIONS, scanType));

    MvrUtil::functorPrintf(&origWriter, "%sResolution: %d%s", myResolution, 
                          getKeywordPrefix(),
			                    EOL_CHARS);
  }
	myResolution = resolution;

  if (changeDetails != NULL) {
    MvrMapFileLineSetWriter newWriter(changeDetails->getChangedSummaryLines
                                  (MvrMapChangeDetails::ADDITIONS, scanType));

    MvrUtil::functorPrintf(&newWriter, "%sResolution: %d%s", myResolution, 
                          getKeywordPrefix(),
    			                EOL_CHARS);
  }
} // end method setResolution




MVREXPORT void MvrMapScan::writePointsToFunctor
		                         (MvrFunctor2<int, std::vector<MvrPose> *> *functor,
                              const char *scanType,
                              MvrFunctor1<const char *> *keywordFunctor)
{
 
  if (keywordFunctor != NULL) {
    MvrUtil::functorPrintf(keywordFunctor, "%s%s", 
                          getPointsKeyword(),
                          "");
  }
	functor->invoke(myNumPoints, &myPoints);

} // end method writePointsToFunctor


MVREXPORT void MvrMapScan::writeLinesToFunctor
	                           (MvrFunctor2<int, std::vector<MvrLineSegment> *> *functor,
                              const char *scanType,
                              MvrFunctor1<const char *> *keywordFunctor)
{
  if (keywordFunctor != NULL) {
    MvrUtil::functorPrintf(keywordFunctor, "%s%s", 
                          getLinesKeyword(),
                          "");
  }
	functor->invoke(myNumLines, &myLines);
} // end method writeLinesToFunctor


MVREXPORT void MvrMapScan::writeScanToFunctor
                             (MvrFunctor1<const char *> *functor, 
			                        const char *endOfLineChars,
                              const char *scanType)
{
  if (!myDisplayString.empty()) {

    MvrUtil::functorPrintf(functor, "%sDisplay: \"%s\"%s",
                          getKeywordPrefix(),
			                    myDisplayString.c_str(),  endOfLineChars);

  } // end if display specified

  if (myNumPoints != 0)
  {
    MvrUtil::functorPrintf(functor, "%sMinPos: %.0f %.0f%s",
                          getKeywordPrefix(),
			                    myMin.getX(), myMin.getY(),  endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sMaxPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
			                    myMax.getX(), myMax.getY(),  endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sNumPoints: %d%s", 
                          getKeywordPrefix(),
			                    myNumPoints, endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sPointsAreSorted: %s%s", 
                          getKeywordPrefix(),
                          (myIsSortedPoints ? "true" : "false"), endOfLineChars);

  }

  if (myResolution != -1) {
    MvrUtil::functorPrintf(functor, "%sResolution: %d%s", 
                          getKeywordPrefix(),
                          myResolution, endOfLineChars);
  }

  if (myNumLines != 0)
  {
    MvrUtil::functorPrintf(functor, "%sLineMinPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
                          myLineMin.getX(), myLineMin.getY(),  endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sLineMaxPos: %.0f %.0f%s", 
                          getKeywordPrefix(),
                          myLineMax.getX(), myLineMax.getY(),  endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sNumLines: %d%s", 
                          getKeywordPrefix(),
                          myNumLines, endOfLineChars);
    MvrUtil::functorPrintf(functor, "%sLinesAreSorted: %s%s", 
                          getKeywordPrefix(),
                          (myIsSortedLines ? "true" : "false"), endOfLineChars);
  }


} // end method writeScanToFunctor


MVREXPORT void MvrMapScan::writePointsToFunctor
                                (MvrFunctor1<const char *> *functor, 
			                           const char *endOfLineChars,
                                 const char *scanType)
{
  MvrUtil::functorPrintf(functor, "%s%s", 
                        getPointsKeyword(),
                        endOfLineChars);

  if (myPoints.empty()) {
    return;
  }

  bool isFastWrite = 
    ((strcmp(endOfLineChars, "\n") == 0) &&
     ((myMin.getX() > INT_MIN) && (myMin.getX() < INT_MAX)) && 
     ((myMin.getY() > INT_MIN) && (myMin.getY() < INT_MAX)) && 
     ((myMax.getX() > INT_MIN) && (myMax.getX() < INT_MAX)) && 
     ((myMax.getY() > INT_MIN) && (myMax.getY() < INT_MAX)));

  if (isFastWrite) {

    // Write the map data points in text format....
    char buf[10000];
    
    for (std::vector<MvrPose>::const_iterator pointIt = myPoints.begin(); 
         pointIt != myPoints.end();
         pointIt++)
    {
      // TODO Test the time of the long indicator...
      snprintf(buf, 10000, "%li %li\n", 
               (long int) (*pointIt).getX(), 
               (long int) (*pointIt).getY());
      functor->invoke(buf);
    } // end for each point
  } 
  else { // not fast write

    for (std::vector<MvrPose>::const_iterator pointIt = myPoints.begin(); 
         pointIt != myPoints.end();
         pointIt++)
    {
      MvrUtil::functorPrintf(functor, "%.0f %.0f%s", 
                                     (*pointIt).getX(), 
                                     (*pointIt).getY(), 
                                     endOfLineChars);

    }
  } // end else not fast write

} // end method writePointsToFunctor

MVREXPORT void MvrMapScan::writeLinesToFunctor
                                (MvrFunctor1<const char *> *functor, 
                                 const char *endOfLineChars,
                                 const char *scanType)
{
  writeLinesToFunctor(functor, myLines, endOfLineChars, scanType);

} // end method writeLinesToFunctor


MVREXPORT void MvrMapScan::writeLinesToFunctor
                                (MvrFunctor1<const char *> *functor, 
                                 const std::vector<MvrLineSegment> &lines,
                                 const char *endOfLineChars,
                                 const char *scanType)
{
  if (lines.empty()) {
    return;
  }
    
  MvrUtil::functorPrintf(functor, "%s%s", 
                        getLinesKeyword(), 
                        endOfLineChars);

  bool isFastWrite = 
    ((strcmp(endOfLineChars, "\n") == 0) &&
     ((myLineMin.getX() > INT_MIN) && (myLineMin.getX() < INT_MAX)) && 
     ((myLineMin.getY() > INT_MIN) && (myLineMin.getY() < INT_MAX)) && 
     ((myLineMax.getX() > INT_MIN) && (myLineMax.getX() < INT_MAX)) && 
     ((myLineMax.getY() > INT_MIN) && (myLineMax.getY() < INT_MAX)));

  if (isFastWrite) {

    // Write the map data points in text format....
    char buf[10000];

    for (std::vector<MvrLineSegment>::const_iterator lineIt = lines.begin(); 
      lineIt != lines.end();
      lineIt++)
    {
      snprintf(buf, 10000, "%li %li %li %li\n", 
               (long int) (*lineIt).getX1(), 
               (long int) (*lineIt).getY1(),
               (long int) (*lineIt).getX2(), 
               (long int) (*lineIt).getY2());
      functor->invoke(buf);
    }
  }
  else { // slow write

    for (std::vector<MvrLineSegment>::const_iterator lineIt = lines.begin(); 
         lineIt != lines.end();
         lineIt++)
    {
      MvrUtil::functorPrintf(functor, "%.0f %.0f %.0f %.0f%s", 
                                    (*lineIt).getX1(), 
                                    (*lineIt).getY1(),
                                    (*lineIt).getX2(), 
                                    (*lineIt).getY2(),
                                    endOfLineChars);
    } // end for each line
  } // end else slow write

} // end method writeLinesToFunctor


bool MvrMapScan::parseNumber(char *line, 
                            size_t lineLen, 
                            size_t *charCountOut,
                            int *numOut) const
{
  if (line == NULL) {
    return false;
  }

  bool isSuccess = true;
  size_t digitCount = 0;
  int num = 0;

  for (size_t i = 0; i < lineLen; i++) 
  {
    if ((isdigit(line[i])) || 
        ((i == 0) && (line[i] == '-'))) {
      digitCount++;
    }
    else {
      break;
    }
  } // end for each char in line

  // The less-than check should be okay since there should be a null-terminator
  if ((digitCount > 0) && (digitCount < (lineLen - 1))) {
    char origChar = line[digitCount];
    line[digitCount] = '\0';
  
    num = atoi(line);

    line[digitCount] = origChar;
  } 
  else { // no digits found
    isSuccess = false;
    digitCount = 0;
    num =  0;
  } // end else no digits found

  if (charCountOut != NULL) {
    *charCountOut = digitCount;
  }
  if (numOut != NULL) {
    *numOut = num;
  }

  return isSuccess;

} // end method parseNumber


bool MvrMapScan::parseWhitespace(char *line,
                                size_t lineLen,
                                size_t *charCountOut) const
{
  if (line == NULL) {
    return false;
  }

  bool isSuccess = true;
  size_t wsCount = 0;

  for (size_t i = 0; i < lineLen; i++) 
  {
    if (isspace(line[i]) && (line[i] != '\0')) { 
      wsCount++;
    }
    else {
      break;
    }
  } // end for each char in line

  // The less-than check should be okay since there should be a null-terminator
  if ((wsCount > 0) && (wsCount < (lineLen - 1))) {
  } 
  else { // no digits found
    isSuccess = false;
    wsCount = 0;
  } // end else no digits found

  if (charCountOut != NULL) {
    *charCountOut = wsCount;
  }

  return isSuccess;
  return false;

} // end method parseWhitespace


MVREXPORT bool MvrMapScan::readDataPoint( char *line)
{
  if (line == NULL) {
    return false;
  }
  
  int x = 0;
  int y = 0;

  bool isSuccess  = true;
  size_t lineLen  = strlen(line) + 1;
  int startIndex  = 0;
  size_t parsedCount = 0;
 
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readDataPoint error parsing x (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readDataPoint error parsing first whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readDataPoint error parsing y (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  loadDataPoint(x, y);

  return true;

} // end method readDataPoint


MVREXPORT bool MvrMapScan::readLineSegment( char *line)
{
  if (line == NULL) {
    return false;
  }
  
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;

  bool isSuccess  = true;
  size_t lineLen     = strlen(line) + 1;
  int startIndex  = 0;
  size_t parsedCount = 0;
 
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x1);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing x1 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing first whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y1);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing y1 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing second whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &x2);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing x2 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseWhitespace(&line[startIndex], lineLen, &parsedCount);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;
  
  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing third whitespace (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  isSuccess = parseNumber(&line[startIndex], lineLen, &parsedCount, &y2);
  
  startIndex += parsedCount;
  lineLen -= parsedCount;

  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrMapScan::readLineSegment error parsing y2 (startIndex = %i, lineLen = %i) in '%s'",
               myLogPrefix.c_str(), startIndex, lineLen, line);
    return false;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  loadLineSegment(x1, y1, x2, y2);

  return true;

} // end method readLineSegment


MVREXPORT void MvrMapScan::loadDataPoint(double x, double y)
{
  if (x > myMax.getX())
    myMax.setX(x);
  if (y > myMax.getY())
    myMax.setY(y);
  
  if (x < myMin.getX())
    myMin.setX(x);
  if (y < myMin.getY())
    myMin.setY(y);
  
  myPoints.push_back(MvrPose(x, y));
  
} // end method loadDataPoint


MVREXPORT void MvrMapScan::loadLineSegment(double x1, double y1, 
                                         double x2, double y2)
{
  if (x1 > myLineMax.getX())
    myLineMax.setX(x1);
  if (y1 > myLineMax.getY())
    myLineMax.setY(y1);
  
  if (x1 < myLineMin.getX())
    myLineMin.setX(x1);
  if (y1 < myLineMin.getY())
    myLineMin.setY(y1);

  if (x2 > myLineMax.getX())
    myLineMax.setX(x2);
  if (y2 > myLineMax.getY())
    myLineMax.setY(y2);
  
  if (x2 < myLineMin.getX())
    myLineMin.setX(x2);
  if (y2 < myLineMin.getY())
    myLineMin.setY(y2);
  
  myLines.push_back(MvrLineSegment(x1, y1, x2, y2));

} // end method loadLineSegment


MVREXPORT bool MvrMapScan::unite(MvrMapScan *other,
                               bool isIncludeDataPointsAndLines)
{
  if (other == NULL) {
    return false;
  }

  if ((myNumPoints > 0) || (myNumLines > 0)) {
    if (other->myTimeChanged.isAfter(myTimeChanged)) {
      myTimeChanged = other->myTimeChanged;
    }
    if (other->myResolution != -1) {
      // Not entirely sure whether it makes sense to use the largest 
      // resolution or the smallest...
      if (other->myResolution > myResolution) {
        myResolution = other->myResolution;
      }
    }
  }
  else {
    myTimeChanged = other->myTimeChanged;
    myResolution = other->myResolution;
  }

  if (myNumPoints > 0) {

    if (other->getMaxPose().getX() > myMax.getX()) {
      myMax.setX(other->getMaxPose().getX());
    }
    if (other->getMaxPose().getY() > myMax.getY()) {
      myMax.setY(other->getMaxPose().getY());
    }

    if (other->getMinPose().getX() < myMin.getX()) {
      myMin.setX(other->getMinPose().getX());
    }
    if (other->getMinPose().getY() < myMin.getY()) {
      myMin.setY(other->getMinPose().getY());
    }

    if (!other->isSortedPoints()) {
      myIsSortedPoints = false;
    }
  }
  else {
    myMax = other->getMaxPose();
    myMin = other->getMinPose();
  
    myIsSortedPoints = other->isSortedPoints();
  }
  myNumPoints += other->getNumPoints();

  if (myNumLines > 0) {

    if (other->getLineMaxPose().getX() > myLineMax.getX()) {
      myLineMax.setX(other->getLineMaxPose().getX());
    }
    if (other->getLineMaxPose().getY() > myLineMax.getY()) {
      myLineMax.setY(other->getLineMaxPose().getY());
    }

    if (other->getLineMinPose().getX() < myLineMin.getX()) {
      myLineMin.setX(other->getLineMinPose().getX());
    }
    if (other->getLineMinPose().getY() < myLineMin.getY()) {
      myLineMin.setY(other->getLineMinPose().getY());
    }
    if (!other->isSortedLines()) {
      myIsSortedLines = false;
    }
  }
  else {
    myLineMax = other->getLineMaxPose();
    myLineMin = other->getLineMinPose();
    
    myIsSortedLines = other->isSortedLines();
  }

  myNumLines += other->getNumLines();


  if (isIncludeDataPointsAndLines) {
   
    bool isPointsChanged = false;
    bool isLinesChanged = false;

    if (other->getPoints() != NULL) {
      myPoints.reserve(myNumPoints);
      for (std::vector<MvrPose>::iterator iter = other->getPoints()->begin();
           iter != other->getPoints()->end();
           iter++) {
        myPoints.push_back(*iter);
        isPointsChanged = true;
      } // end for each other point
    } // end if other scan has points

    if (myIsSortedPoints) {
	    std::sort(myPoints.begin(), myPoints.end());
    }
    
    if (other->getLines() != NULL) {
      myLines.reserve(myNumLines);
      for (std::vector<MvrLineSegment>::iterator iter = other->getLines()->begin();
           iter != other->getLines()->end();
           iter++) {
        myLines.push_back(*iter);
        isLinesChanged = true;
      } // end for each other line  
    } // end if other scan has lines 
    
    if (myIsSortedLines) {
	    std::sort(myLines.begin(), myLines.end());
    }

  } // end if include points and lines

  return true;

} // end method unite


bool MvrMapScan::handleMinPos(MvrArgumentBuilder *arg)
{
  return parsePose(arg, "MinPos:", &myMin);

} // end method handleMinPos


bool MvrMapScan::handleMaxPos(MvrArgumentBuilder *arg)
{
  return parsePose(arg, "MaxPos:", &myMax);
}

bool MvrMapScan::handleNumPoints(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {

    bool ok = false;
    int numPoints = arg->getArgInt(0, &ok);

    if (ok) {
      
      myNumPoints = numPoints;
      // myNumPoints = 0;

      MvrLog::log(MvrLog::Normal,
                 "%sMvrMapScan::handleNumPoints() set num points to %i",
                 myLogPrefix.c_str(),
                 numPoints);
  
      if (numPoints > 0) {
        // myPointText.reserve(myNumPoints);
        myPoints.reserve(numPoints);
      }
      else { // no points
        // Reset the min and max poses to have the same values that would result
        // from a call to setPoints with no points.  This is done primarily so 
        // that map change processing will behave correctly.
        myMax.setX(INT_MIN);
        myMax.setY(INT_MIN);
        myMin.setX(INT_MAX);
        myMin.setY(INT_MAX);
      } // end else no points

      return true;
    }
  } // end if enough args

  // If this is reached, then an error has occurred...  
  MvrLog::log(MvrLog::Terse, 
	           "%sMvrMapScan: '%sNumPoints:' bad argument, should be one integer (number of data points)",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  
  return false;

} // end method handleNumPoints


bool MvrMapScan::handleIsSortedPoints(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {
    bool ok = true;
    bool isSorted = arg->getArgBool(0, &ok);
    if (ok) {
      myIsSortedPoints = isSorted;
      return true;
    }
  } // end if correct arg count
    
  MvrLog::log(MvrLog::Terse, 
             "MvrMapScan: 'PointsAreSorted:' bad arguments, should be a boolean");
  return false;

} // end method handleIsSortedPoints


bool MvrMapScan::handleLineMinPos(MvrArgumentBuilder *arg)
{
  return parsePose(arg, "LineMinPos:", &myLineMin);
}

bool MvrMapScan::handleLineMaxPos(MvrArgumentBuilder *arg)
{
  return parsePose(arg, "LineMaxPos:", &myLineMax);
}


bool MvrMapScan::parsePose(MvrArgumentBuilder *arg,
                          const char *keyword,
                          MvrPose *poseOut) 
{
  if ((arg == NULL) || (keyword == NULL) || (poseOut == NULL)) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapScan::parsePose() invalid NULL parameters");
    return false;
  }

  if (arg->getArgc() == 2) {

    bool xOk = true;
    bool yOk = true;
    int x = arg->getArgInt(0, &xOk);
    int y = arg->getArgInt(1, &yOk);

    if (xOk && yOk) {
      poseOut->setPose(x, y);
      return true;
    }
  } // end if correct arg count

  MvrLog::log(MvrLog::Terse, 
 	           "%sMvrMapScan: '%s%s' bad arguments, should be two integers x y",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str(),
             keyword);

  return false;

} // end method parsePose


bool MvrMapScan::handleNumLines(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {

    bool ok = false;
    int numLines = arg->getArgInt(0, &ok);

    if (ok) {

      // Reset the myNumLines to 0, it is incremented as the actual lines are added
      myNumLines = numLines;

      if (numLines > 0) {
	      myLines.reserve(numLines);
      }
      else { // no lines
        // Reset the min and max poses to have the same values that would result
        // from a call to setLines with no lines.  This is done primarily so 
        // that map change processing will behave correctly.
        myLineMax.setX(INT_MIN);
        myLineMax.setY(INT_MIN);
        myLineMin.setX(INT_MAX);
        myLineMin.setY(INT_MAX);
      } // end else no lines

      return true;

    } // end if ok
  } // end if enough args

  // If this is reached, then an error has occurred...  
  MvrLog::log(MvrLog::Terse, 
	           "%sMvrMapScan: '%sNumLines:' bad argument, should be one integer (number of data points)",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  
  return false;
  
} // end method handleNumLines


bool MvrMapScan::handleIsSortedLines(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() >= 1) {
    bool ok = true;
    bool isSorted = arg->getArgBool(0, &ok);
    if (ok) {
      myIsSortedLines = isSorted;
      return true;
    }
  } // end if correct arg count
    
  MvrLog::log(MvrLog::Terse, 
 	           "%sMvrMapScan: '%sLinesAreSorted' bad arguments, should be a boolean",
             myLogPrefix.c_str(),
             myKeywordPrefix.c_str());
  return false;

} // end method handleIsSortedLines



bool MvrMapScan::handleResolution(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() == 1)
  {
    bool ok = true;
    int res = arg->getArgInt(0, &ok);
    if (ok) {
      myResolution = res;
      return true;
    }
  } // end if correct arg count

  MvrLog::log(MvrLog::Terse, 
	           "MvrMapScan: 'Resolution:' bad argument, should be one integer (resolution in mm)");
  return false;

} // end method handleResolution


bool MvrMapScan::handleDisplayString(MvrArgumentBuilder *arg)
{
  arg->compressQuoted();

  if (arg->getArgc() >= 1) {

    const char *displayArg = arg->getArg(0);
    int displayBufferLen = strlen(displayArg) + 1;
    char *displayBuffer = new char[displayBufferLen];
  
     if (MvrUtil::stripQuotes(displayBuffer, displayArg, displayBufferLen))
     {
        myDisplayString = displayBuffer;

        MvrLog::log(MvrLog::Normal, 
                  "%sMvrMapScan: '%sDisplay' setting display '%s'",
                  myLogPrefix.c_str(),
                  myKeywordPrefix.c_str(),
                  myDisplayString.c_str());
     }
     else {
      MvrLog::log(MvrLog::Terse, 
	                "%sMvrMapScan: '%sDisplay:' couldn't strip quotes from '%s'", 
                  myLogPrefix.c_str(),
                  myKeywordPrefix.c_str(),
	                displayArg);
     } // end if error stripping quotes

     delete [] displayBuffer;



    return true;
  }
  else {
    MvrLog::log(MvrLog::Terse, 
	            "%sMvrMapScan: '%sDisplay:' insufficient args '%s'", 
              myLogPrefix.c_str(),
              myKeywordPrefix.c_str(),
	            arg->getFullString());
    return false;
  
 }

} // end method handleDisplayString


bool MvrMapScan::handlePoint(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() == 2) {

    bool xOk = true;
    bool yOk = true;

    int x = arg->getArgInt(0, &xOk);
    int y = arg->getArgInt(1, &yOk);

    if (xOk && yOk) {
      loadDataPoint(x, y);
      return true;
    }
  } // end if correct arg count

  MvrLog::log(MvrLog::Terse, 
	           "MvrMapScan::handlePoint: map point wrong, should be x and y int coords (in mm) but is %s", 
             arg->getFullString());
    
  return false;

} // end method handlePoint


bool MvrMapScan::handleLine(MvrArgumentBuilder *arg)
{
 
  if (arg->getArgc() == 4) {

    bool x1Ok = true;
    bool y1Ok = true;
    bool x2Ok = true;
    bool y2Ok = true;

    int x1 = arg->getArgInt(0, &x1Ok);
    int y1 = arg->getArgInt(1, &y1Ok);
    int x2 = arg->getArgInt(2, &x2Ok);
    int y2 = arg->getArgInt(3, &y2Ok);

    if (x1Ok && y1Ok && x2Ok && y2Ok) {
      loadLineSegment(x1, y1, x2, y2);
      return true;
    }
  } // end if correct arg count

  MvrLog::log(MvrLog::Verbose, 
	            "MvrMapScan::handleLine: line wrong, should be 2 x, y points (in mm) but is %s", 
              arg->getFullString());
  return false;

} // end method handleLine

  
bool MvrMapScan::addHandlerToFileParser
                    (MvrFileParser *fileParser,
                     const char *keyword,
                     MvrRetFunctor1<bool, MvrArgumentBuilder *> *handler)
{
  if ((fileParser == NULL) || (handler == NULL)) {
    return false;
  }
  bool isAdded = false;
 
  if (keyword != NULL) {

    std::string fullKeyword = getKeywordPrefix();
    fullKeyword += keyword;

    isAdded = fileParser->addHandler(fullKeyword.c_str(),
                                     handler);
  }
  else {
    isAdded = fileParser->addHandler(NULL, handler);
  }

  return isAdded;

} // end method addHandlerToFileParser
  
  
MVREXPORT const char *MvrMapScan::getScanType() const
{
  return myScanType.c_str();
}

MVREXPORT const char *MvrMapScan::getPointsKeyword() const
{
  return myPointsKeyword.c_str();
}

MVREXPORT const char *MvrMapScan::getLinesKeyword() const
{
  return myLinesKeyword.c_str();
}
  
const char *MvrMapScan::getKeywordPrefix() const
{
  return myKeywordPrefix.c_str();
}


// ---------------------------------------------------------------------------- 
// MvrMapObjects
// ---------------------------------------------------------------------------- 


const char *MvrMapObjects::DEFAULT_KEYWORD = "Cairn:";


MVREXPORT MvrMapObjects::MvrMapObjects(const char *keyword) :
  myTimeChanged(),
  myIsSortedObjects(false),
  myKeyword((keyword != NULL) ? keyword : DEFAULT_KEYWORD),
  myMapObjects(),
  myMapObjectCB(this, &MvrMapObjects::handleMapObject)
{
}


MVREXPORT MvrMapObjects::MvrMapObjects(const MvrMapObjects &other) :
  myTimeChanged(other.myTimeChanged),
  myIsSortedObjects(other.myIsSortedObjects),
  myKeyword(other.myKeyword),
  myMapObjects(),
  myMapObjectCB(this, &MvrMapObjects::handleMapObject)
{
  for (std::list<MvrMapObject *>::const_iterator it = other.myMapObjects.begin(); 
       it != other.myMapObjects.end(); 
       it++)
  {
    myMapObjects.push_back(new MvrMapObject(*(*it)));
  }

} // end copy ctor


MVREXPORT MvrMapObjects &MvrMapObjects::operator=(const MvrMapObjects &other)
{
  if (&other != this) {

    MvrUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
    myMapObjects.clear();
  
    myTimeChanged = other.myTimeChanged;
    myIsSortedObjects = other.myIsSortedObjects;
  
    myKeyword = other.myKeyword;

    for (std::list<MvrMapObject *>::const_iterator it = other.myMapObjects.begin(); 
         it != other.myMapObjects.end(); 
         it++)
    {
      myMapObjects.push_back(new MvrMapObject(*(*it)));
    }
  }
  return *this;

} // end method operator=


MVREXPORT MvrMapObjects::~MvrMapObjects()
{
  MvrUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();
}


MVREXPORT bool MvrMapObjects::addToFileParser(MvrFileParser *fileParser) 
{
  if (fileParser == NULL) {
    return false;
  }

  // make sure we can add all our handlers
  if (!fileParser->addHandler(myKeyword.c_str(), &myMapObjectCB))
  {
    MvrLog::log(MvrLog::Terse, "MvrMapObjects::addToFileParser: could not add handlers");
    return false;
  }  

  return true;
  
} // end method addToFileParser


MVREXPORT bool MvrMapObjects::remFromFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }

  fileParser->remHandler(&myMapObjectCB);

  return true;

} // end method remFromFileParser

  
MVREXPORT MvrTime MvrMapObjects::getTimeChanged() const
{
  return myTimeChanged;
}


MVREXPORT void MvrMapObjects::clear() 
{
  myTimeChanged.setToNow();

  MvrUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();

} // end method clear


MVREXPORT MvrMapObject *MvrMapObjects::findFirstMapObject(const char *name, 
														                           const char *type,
                                                       bool isIncludeWithHeading)
{
  for (std::list<MvrMapObject *>::iterator objIt = getMapObjects()->begin(); 
       objIt != getMapObjects()->end(); 
       objIt++)
  {
    MvrMapObject* obj = (*objIt);
    if(obj == NULL)
      return NULL;
    // if we're searching any type or its the right type then check the name
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      if(name == NULL || strcasecmp(obj->getName(), name) == 0)
      {
        return obj;
      }
    }
  }

  // if we get down here we didn't find it
  return NULL;
} // end method findFirstMapObject


MVREXPORT MvrMapObject *MvrMapObjects::findMapObject(const char *name, 
				                                          const char *type,
                                                  bool isIncludeWithHeading)
{
  std::list<MvrMapObject *>::iterator objIt;
  MvrMapObject* obj = NULL;

  for (objIt = getMapObjects()->begin(); 
       objIt != getMapObjects()->end(); 
       objIt++)
  {
    obj = (*objIt);
    if(obj == NULL)
      return NULL;
    // if we're searching any type or its the right type then check the name
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      if(name == NULL || strcasecmp(obj->getName(), name) == 0)
      {
	      return obj;
      }
    }
  }

  // if we get down here we didn't find it
  return NULL;
} // end method findMapObject


/**
   When the map changes the pointers will no longer be valid... this
   doesn't lock the map while finding, so if you are using it from
   somewhere other than mapChanged you need to lock it... its probably
   easiest to just use it from mapChanged though.

  @return Gets a list of pointers to all the map objects of a given *
 type... if none match the list will be empty.

   @param type The type of object to try to find... NULL means find
   any type
   @param isIncludeWithHeading also match "WithHeading" versions of @a type
   ("<i>type</i>WithHeading")
 **/
MVREXPORT std::list<MvrMapObject *> MvrMapObjects::findMapObjectsOfType
                                                  (const char *type,
                                                   bool isIncludeWithHeading)
{
  std::list<MvrMapObject *> ret;

  for (std::list<MvrMapObject *>::iterator objIt = myMapObjects.begin(); 
       objIt != myMapObjects.end(); 
       objIt++)
  {
    MvrMapObject* obj = (*objIt);
    if (obj == NULL)
      continue;
    // if we're searching any type or its the right type then add it
    if (type == NULL || 
        (!isIncludeWithHeading && (strcasecmp(obj->getType(), type) == 0)) ||
        (isIncludeWithHeading && (strcasecmp(obj->getBaseType(), type) == 0)))
    {
      ret.push_back(obj);
    }
  }

  return ret;
} // end method findMapObjectsOfType

MVREXPORT std::list<MvrMapObject *> *MvrMapObjects::getMapObjects(void)
{
  // Think this should be done in getMapObjects....
  if (!myIsSortedObjects) {
    sortMapObjects(&myMapObjects);
    myIsSortedObjects = true;
  }
  return &myMapObjects;

} // end method getMapObjects


void MvrMapObjects::sortMapObjects(std::list<MvrMapObject *> *mapObjects)
{
  MvrMapObjectCompare compare;

  std::vector<MvrMapObject *> tempObjects;
  for (std::list<MvrMapObject *>::iterator iter1 = mapObjects->begin();
        iter1 != mapObjects->end();
        iter1++) {
    tempObjects.push_back(*iter1);    
  }
	std::sort(tempObjects.begin(), tempObjects.end(), compare);

  mapObjects->clear();
  for (std::vector<MvrMapObject *>::iterator iter2 = tempObjects.begin();
        iter2 != tempObjects.end();
        iter2++) {
    mapObjects->push_back(*iter2);    
  }

} // end method sortMapObjects


MVREXPORT void MvrMapObjects::setMapObjects(const std::list<MvrMapObject *> *mapObjects,
                                          bool isSortedObjects,
                                          MvrMapChangeDetails *changeDetails) 
{

  myTimeChanged.setToNow();

  // Think this should be done in getMapObjects....
  if (!myIsSortedObjects) {
    sortMapObjects(&myMapObjects);
    myIsSortedObjects = true;
  }

  const std::list<MvrMapObject *> *newMapObjects = mapObjects;
  std::list<MvrMapObject *> *mapObjectsCopy = NULL;
  
  if ((mapObjects != NULL) &&
      (!isSortedObjects || (mapObjects == &myMapObjects))) {
    mapObjectsCopy =  new std::list<MvrMapObject *>(*mapObjects);
	  sortMapObjects(mapObjectsCopy);
    newMapObjects = mapObjectsCopy;
  }
  

  MvrMapFileLineSet origLines;

  if (changeDetails != NULL) {
    createMultiSet(&origLines);
  }


  std::list<MvrMapObject *> origMapObjects = myMapObjects;
  //MvrUtil::deleteSet(myMapObjects.begin(), myMapObjects.end());
  myMapObjects.clear();
//  myMapObjectsChanged.setToNow();

  if (newMapObjects != NULL) {

    for (std::list<MvrMapObject *>::const_iterator it = newMapObjects->begin(); 
        it != newMapObjects->end(); 
        it++)
    {
      MvrMapObject *obj = *it;
      if (obj == NULL) {
        continue;
      }
      myMapObjects.push_back(new MvrMapObject(*obj));
      origMapObjects.remove(obj);
    }
  }

  if (changeDetails != NULL) {

    MvrMapFileLineSet newLines;
    createMultiSet(&newLines);

    bool isSuccess = MvrMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedObjectLines
                                          (MvrMapChangeDetails::DELETIONS),
                         changeDetails->getChangedObjectLines
                                          (MvrMapChangeDetails::ADDITIONS),
                         false);

    if (!isSuccess) {
      MvrLog::log(MvrLog::Normal,
                 "MvrMapObjects::setMapObjects() error calculating changes"); 
    }
  } // end if accumulate changes

  // Anything that is left in the original map objects list was not passed
  // in via the given map object list.  Delete all of the remaining objects.
  // The caller is responsible for deleting the objects in the given list.
  MvrUtil::deleteSet(origMapObjects.begin(), origMapObjects.end());

  if (mapObjectsCopy != NULL) {
    delete mapObjectsCopy;
  }

} // end method setMapObjects


MVREXPORT void MvrMapObjects::writeObjectListToFunctor(MvrFunctor1<const char *> *functor, 
		                                                 const char *endOfLineChars)
{
  // TODO: Ideally it would probably be nice to cache this string in the object...
  // Is this possible with the two different types of keywords?

  for (std::list<MvrMapObject*>::iterator mapObjectIt = myMapObjects.begin(); 
       mapObjectIt != myMapObjects.end(); 
       mapObjectIt++)
  {
    MvrMapObject *object = (*mapObjectIt);

    MvrUtil::functorPrintf(functor, 
	                        "%s %s%s",
                          myKeyword.c_str(),
                          object->toString(),
	                        endOfLineChars);
 }
} // end method writeObjectListToFunctor


bool MvrMapObjects::handleMapObject(MvrArgumentBuilder *arg)
{
  MvrMapObject *object = MvrMapObject::createMapObject(arg);

  if (object == NULL) {
    return false;
  }
  myMapObjects.push_back(object);
//  object->log(myKeyword.c_str());
  //arg->log();
  return true;

} // end method handleMapObject




void MvrMapObjects::createMultiSet(MvrMapFileLineSet *multiSet)
{
  MvrMapFileLineSetWriter origWriter(multiSet);

  writeObjectListToFunctor(&origWriter, "\n");

} // end method createMultiSet

void MvrMapObjects::logMultiSet(const char *prefix,
                               MvrMapFileLineSet *multiSet)
{
  if (prefix != NULL) {
    MvrLog::log(MvrLog::Normal,
               prefix);
  }
  if (multiSet == NULL) {
    MvrLog::log(MvrLog::Normal,
               "NULL");
    return;
  }

  for (MvrMapFileLineSet::iterator mIter = multiSet->begin();
       mIter != multiSet->end();
       mIter++) {
    MvrMapFileLineGroup &fileLine = *mIter;
    fileLine.log();
    /**
    MvrLog::log(MvrLog::Normal,
               "#%-3i : %s",
               fileLine.getLineNum(),
               fileLine.getLineText());
    **/
  }
} // end method logMultiSet


// ---------------------------------------------------------------------------- 
// MvrMapInfo
// ---------------------------------------------------------------------------- 



void MvrMapInfo::setDefaultInfoNames()
{
  myInfoTypeToNameMap[MAP_INFO] = "MapInfo:";
  myInfoTypeToNameMap[META_INFO] = "MetaInfo:";
  myInfoTypeToNameMap[TASK_INFO] = "TaskInfo:";
  myInfoTypeToNameMap[ROUTE_INFO] = "RouteInfo:";
  myInfoTypeToNameMap[SCHED_TASK_INFO] = "SchedTaskInfo:";
  myInfoTypeToNameMap[SCHED_INFO] = "SchedInfo:";
  myInfoTypeToNameMap[CAIRN_INFO] = "CairnInfo:";
  myInfoTypeToNameMap[CUSTOM_INFO] = "CustomInfo:";

} // end method setDefaultInfoNames



MvrMapInfo::MvrMapInfoData::MvrMapInfoData(MvrMapInfo *parent,
                                        const char *keyword,
                                        int type) :
  myParent(parent),
  myType(type),
  myKeyword((keyword!= NULL) ? keyword : ""),
  myInfo(),
  myInfoCB(NULL)
{
  if (myParent != NULL) {
    myInfoCB = new MvrRetFunctor1C<bool, MvrMapInfo, MvrArgumentBuilder *>
							                                (parent,
							                                 &MvrMapInfo::handleInfo,
								                               NULL);
  }

}

MvrMapInfo::MvrMapInfoData::MvrMapInfoData(MvrMapInfo *parent,
                                        const MvrMapInfoData &other) :
  myParent(parent),
  myType(other.myType),
  myKeyword(other.myKeyword),
  myInfo(),
  myInfoCB(NULL) // Don't copy callbacks
{
  if (myParent != NULL) {
    myInfoCB = new MvrRetFunctor1C<bool, MvrMapInfo, MvrArgumentBuilder *>
							                                (parent,
							                                 &MvrMapInfo::handleInfo,
								                               NULL);
  }
  for (std::list<MvrArgumentBuilder *>::const_iterator iter = other.myInfo.begin();
        iter != other.myInfo.end();
        iter++) {
    MvrArgumentBuilder *arg = *iter;
    if (arg == NULL) {
      continue;
    }
    myInfo.push_back(new MvrArgumentBuilder(*arg));
  }
} // end pseudo-copy-constructor


MvrMapInfo::MvrMapInfoData &MvrMapInfo::MvrMapInfoData::operator=(const MvrMapInfoData &other) 
{
  if (this != &other) {
    // Don't change parents
    myType = other.myType;
    myKeyword = other.myKeyword;

    MvrUtil::deleteSet(myInfo.begin(), myInfo.end());
    myInfo.clear();

    for (std::list<MvrArgumentBuilder *>::const_iterator iter = other.myInfo.begin();
         iter != other.myInfo.end();
         iter++) {
      MvrArgumentBuilder *arg = *iter;
      if (arg == NULL) {
        continue;
      }
      myInfo.push_back(new MvrArgumentBuilder(*arg));
    }
    // myInfoCB = other.myInfoCB;  // Don't copy callbacks
  }
  return *this;
}

MvrMapInfo::MvrMapInfoData::~MvrMapInfoData()
{
  MvrUtil::deleteSet(myInfo.begin(), myInfo.end());
  myInfo.clear();

  delete myInfoCB;
}




MVREXPORT MvrMapInfo::MvrMapInfo(const char **infoNameList,
                              size_t infoNameCount,
                              const char *keywordPrefix) :
  MvrMapInfoInterface(),
  myTimeChanged(),
  myNumInfos(0), 
  myPrefix((keywordPrefix != NULL) ? keywordPrefix : ""),
  myInfoTypeToNameMap(),
  myInfoNameToDataMap(),
  myKeywordToInfoNameMap()
{
  setDefaultInfoNames();

  if (infoNameList == NULL) {
   
    // TODO Someday the int types should disappear and this should just be a list of 
    // info names

    for (std::map<int, std::string>::iterator iter = myInfoTypeToNameMap.begin();
         iter != myInfoTypeToNameMap.end();
         iter++) {

      std::string dataName = myPrefix + iter->second.c_str();
     
      MvrMapInfoData *data = new MvrMapInfoData(this, dataName.c_str(), iter->first);
      myInfoNameToDataMap[iter->second] = data;
      myKeywordToInfoNameMap[dataName.c_str()] = iter->second;
      myNumInfos++;
    }
  }
  else { // info name list

    for (size_t i = 0; i < infoNameCount; i++) {
      const char *curName = infoNameList[i];
      if (MvrUtil::isStrEmpty(curName)) {
        continue;
      }
      std::string dataName = myPrefix + curName;

      MvrMapInfoData *data = new MvrMapInfoData(this, dataName.c_str());
      myInfoNameToDataMap[curName] = data;
      myKeywordToInfoNameMap[dataName.c_str()] = curName;
      myNumInfos++;
    }
  } // end else info name list

} // end ctor



MVREXPORT MvrMapInfo::MvrMapInfo(const MvrMapInfo &other) :
  MvrMapInfoInterface(),
  myTimeChanged(other.myTimeChanged),
  myNumInfos(other.myNumInfos), 
  myPrefix(other.myPrefix),
  myInfoTypeToNameMap(other.myInfoTypeToNameMap),
  myInfoNameToDataMap(),
  myKeywordToInfoNameMap(other.myKeywordToInfoNameMap)
{

  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::const_iterator iter = other.myInfoNameToDataMap.begin();
       iter != other.myInfoNameToDataMap.end();
       iter++) {
    const MvrMapInfoData *otherData = iter->second;
    if (otherData == NULL) {
      continue;
    }
    myInfoNameToDataMap[iter->first] = new MvrMapInfoData(this, *otherData);
  }
  
} // end copy ctor


MVREXPORT MvrMapInfo &MvrMapInfo::operator=(const MvrMapInfo &other)
{
  if (&other != this) {

    myTimeChanged = other.myTimeChanged;
    myNumInfos = other.myNumInfos;
    myPrefix = other.myPrefix;
    myInfoTypeToNameMap = other.myInfoTypeToNameMap;

    MvrUtil::deleteSetPairs(myInfoNameToDataMap.begin(), myInfoNameToDataMap.end());
    myInfoNameToDataMap.clear();

    for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::const_iterator iter = other.myInfoNameToDataMap.begin();
         iter != other.myInfoNameToDataMap.end();
         iter++) {
      const MvrMapInfoData *otherData = iter->second;
      if (otherData == NULL) {
        continue;
      }
      myInfoNameToDataMap[iter->first] = new MvrMapInfoData(this, *otherData);
    }

    myKeywordToInfoNameMap = other.myKeywordToInfoNameMap;

  } // end if not this
  
  return *this;

} // end operator=


MVREXPORT MvrMapInfo::~MvrMapInfo()
{
  MvrUtil::deleteSetPairs(myInfoNameToDataMap.begin(), myInfoNameToDataMap.end());
  myInfoNameToDataMap.clear();

} // end dtor


MVREXPORT bool MvrMapInfo::addToFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }

  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {

    MvrMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
	  if (!fileParser->addHandler(data->myKeyword.c_str(), 
									              data->myInfoCB)) {

 	    MvrLog::log(MvrLog::Terse, 
                 "MvrMapInfo::addToFileParser: could not add handler for %s",
                 data->myKeyword.c_str());
                  
      return false;
   } // end if error adding handlers                              
  } // end for each info

  return true;

} // end method addToFileParser


MVREXPORT bool MvrMapInfo::remFromFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }

  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {
    MvrMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
	  fileParser->remHandler(data->myInfoCB);
  }
  return true;

} // end method remFromFileParser


MVREXPORT MvrTime MvrMapInfo::getTimeChanged() const
{
  return myTimeChanged;
}

void MvrMapInfo::setChanged()
{
  myTimeChanged.setToNow();
}

MVREXPORT void MvrMapInfo::clear()
{
  myTimeChanged.setToNow();

  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
      iter != myInfoNameToDataMap.end();
      iter++) {
    MvrMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
    MvrUtil::deleteSet(data->myInfo.begin(), data->myInfo.end());
    data->myInfo.clear();

  } // end for each info

} // end method clear


MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapInfo::getInfo(int infoType)
{
  std::string infoName;
  std::map<int, std::string>::iterator iter1 = myInfoTypeToNameMap.find(infoType);
  if (iter1 != myInfoTypeToNameMap.end()) {
    infoName = iter1->second;
  }
  return getInfo(infoName.c_str());

}
MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapInfo::getInfo(const char *infoName)
{
  MvrMapInfoData *data = findData(infoName);

  if (data != NULL) {
    return &data->myInfo;
  }
  else {
    return NULL;
  }

} // end method getInfo


MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapInfo::getMapInfo(void)
{
  return getInfo(MAP_INFO_NAME);
}

MVREXPORT int MvrMapInfo::getInfoCount() const
{
  return myNumInfos;
}

MVREXPORT std::list<std::string> MvrMapInfo::getInfoNames() const
{
  std::list<std::string> infoNames;
  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::const_iterator iter =
            myInfoNameToDataMap.begin();
       iter != myInfoNameToDataMap.end();
       iter++) {

    infoNames.push_back(iter->first);
  }
  return infoNames;
}


  
void MvrMapInfo::createMultiSet(const char *infoName, 
                               MvrMapFileLineSet *multiSet,
                               MvrMapChangeDetails *changeDetails)
{
  MvrMapFileLineSetWriter origWriter(multiSet);

  MvrMapInfoData *data = findData(infoName);
  if (data == NULL) {
    return;
  }

  // TODO Will need to know position in order to propagate changes. ???
  for (std::list<MvrArgumentBuilder *>::iterator infoIt = data->myInfo.begin();
       infoIt != data->myInfo.end();
       infoIt++) {

    MvrArgumentBuilder *arg = *infoIt;
    if (arg == NULL) {
      continue;
    }
    bool isAddingChildren = false;
    if (changeDetails != NULL) {
      isAddingChildren = changeDetails->isChildArg(infoName, arg);
    }
    origWriter.setAddingChildren(isAddingChildren);

    MvrUtil::functorPrintf(&origWriter, "%s %s%s", 
                          data->myKeyword.c_str(),
                          (*infoIt)->getFullString(), 
                          "");  // TODO: What to do about endOfLineChars

  } // end for each info in list

} // end method createMultiSet


MVREXPORT MvrMapInfo::MvrMapInfoData *MvrMapInfo::findData(const char *infoName)
{
  if (MvrUtil::isStrEmpty(infoName)) {
    return NULL;
  }

  MvrMapInfoData *data = NULL;
  std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::iterator iter1 = 
                                            myInfoNameToDataMap.find(infoName);

  if (iter1 != myInfoNameToDataMap.end()) {
    data = iter1->second;
  }

  return data;

} // end method findData

MVREXPORT MvrMapInfo::MvrMapInfoData *MvrMapInfo::findDataByKeyword(const char *keyword)
{
  if (MvrUtil::isStrEmpty(keyword)) {
    return NULL;
  }
  std::map<std::string, std::string, MvrStrCaseCmpOp>::iterator iter = myKeywordToInfoNameMap.find(keyword);
  if (iter != myKeywordToInfoNameMap.end()) {
    return findData(iter->second.c_str());
  }
  return NULL;

} // end method findDataByKeyword

MVREXPORT bool MvrMapInfo::setInfo(int infoType,
						                     const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails)

{
  return setInfo(getInfoName(infoType), infoList, changeDetails);
}


MVREXPORT bool MvrMapInfo::setInfo(const char *infoName,
                                 const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails)

{
  MvrMapInfoData *data = findData(infoName);
  if (data == NULL) {
    return false;
  }

  
  MvrMapFileLineSet origLines;

  if (changeDetails != NULL) {
    createMultiSet(infoName, &origLines, changeDetails);
  }

  // Make sure that the original list is not being passed back into this method.
  // TODO Fix this
  if (&(data->myInfo) == infoList) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapInfo::setInfo() cannot set to original list");
    return false;
  }

  MvrUtil::deleteSet(data->myInfo.begin(), data->myInfo.end());
  data->myInfo.clear();

// TODO   myInfoChangedArray[infoType].setToNow();
  myTimeChanged.setToNow();

  if (infoList != NULL) {

    for (std::list<MvrArgumentBuilder *>::const_iterator it = infoList->begin(); 
        it != infoList->end(); 
        it++)
    {
      data->myInfo.push_back(new MvrArgumentBuilder(*(*it)));
    }
  }

  if (changeDetails != NULL) {

    MvrMapFileLineSet newLines;
    createMultiSet(infoName, &newLines, changeDetails);

    bool isSuccess = MvrMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedInfoLines
                                          (infoName,
                                           MvrMapChangeDetails::DELETIONS),
                         changeDetails->getChangedInfoLines
                                          (infoName,
                                           MvrMapChangeDetails::ADDITIONS));
    if (!isSuccess) {
      MvrLog::log(MvrLog::Normal,
                 "MvrMapInfo::setInfo() error calculating changes");
    }
  } // end if changeDetails

  return true;

} // end method setInfo


MVREXPORT bool MvrMapInfo::setMapInfo(const std::list<MvrArgumentBuilder *> *mapInfo,
                                    MvrMapChangeDetails *changeDetails)
{
  return setInfo(MAP_INFO_NAME, mapInfo, changeDetails);
}


MVREXPORT void MvrMapInfo::writeInfoToFunctor
				                     (MvrFunctor1<const char *> *functor, 
			                        const char *endOfLineChars)
{

  // TODO: Write the info list?

  for (std::map<std::string, MvrMapInfoData*, MvrStrCaseCmpOp>::iterator iter = myInfoNameToDataMap.begin();
       iter != myInfoNameToDataMap.end();
       iter++) {
    MvrMapInfoData *data = iter->second;
    if (data == NULL) {
      continue;
    }
    for (std::list<MvrArgumentBuilder *>::iterator infoIt = data->myInfo.begin();
         infoIt != data->myInfo.end();
         infoIt++) {

      MvrUtil::functorPrintf(functor, "%s %s%s", 
                            data->myKeyword.c_str(),
                            (*infoIt)->getFullString(), 
                            endOfLineChars);

      } // end for each info in list

  } // end for each info

} // end method writeInfoToFunctor


MVREXPORT const char *MvrMapInfo::getInfoName(int infoType)
{
  std::map<int, std::string>::iterator iter = myInfoTypeToNameMap.find(infoType);
  if (iter != myInfoTypeToNameMap.end()) {
    const std::string &name = iter->second;
    return name.c_str();
  }
  return NULL;

} // end method getInfoName


bool MvrMapInfo::handleInfo(MvrArgumentBuilder *arg)
{
  arg->compressQuoted();

  const char *keyword = arg->getExtraString();

  MvrMapInfoData *data = findDataByKeyword(keyword);

  if (data == NULL) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapInfo::handleInfo cannot process %s",
               keyword);
    return false;
  }
  MvrArgumentBuilder *infoBuilder = new MvrArgumentBuilder(*arg);
  data->myInfo.push_back(infoBuilder);

  return true;

} // end method handleInfo


// -----------------------------------------------------------------------------

const char *MvrMapSupplement::EOL_CHARS = "";

MVREXPORT MvrMapSupplement::MvrMapSupplement() :
  myTimeChanged(),
  myHasOriginLatLongAlt(false),
  myOriginLatLong(),
  myOriginAltitude(0),
  myOriginLatLongAltCB(this, &MvrMapSupplement::handleOriginLatLongAlt)
{
}

MVREXPORT MvrMapSupplement::MvrMapSupplement(const MvrMapSupplement &other) :
  myTimeChanged(other.myTimeChanged),
  myHasOriginLatLongAlt(other.myHasOriginLatLongAlt),
  myOriginLatLong(other.myOriginLatLong),
  myOriginAltitude(other.myOriginAltitude),

  myOriginLatLongAltCB(this, &MvrMapSupplement::handleOriginLatLongAlt)
{
}


MVREXPORT MvrMapSupplement &MvrMapSupplement::operator=(const MvrMapSupplement &other) 
{
  if (&other != this) {
    myTimeChanged = other.myTimeChanged;
    myHasOriginLatLongAlt = other.myHasOriginLatLongAlt;
    myOriginLatLong = other.myOriginLatLong;
    myOriginAltitude = other.myOriginAltitude;
  }
  return *this;
}


MVREXPORT MvrMapSupplement::~MvrMapSupplement()
{
}

MVREXPORT bool MvrMapSupplement::addToFileParser(MvrFileParser *fileParser)
{
  if (fileParser == NULL) {
    return false;
  }
  if (!fileParser->addHandler("OriginLatLongAlt:", &myOriginLatLongAltCB))
  {
    MvrLog::log(MvrLog::Terse, 
               "MvrMapSupplement::addToFileParser: could not add handlers");
    return false;
  }  
  MvrLog::log(MvrLog::Verbose,
             "MvrMapSupplement::addToFileParser() successfully added handlers");

  return true;

} // end method addToFileParser


MVREXPORT bool MvrMapSupplement::remFromFileParser(MvrFileParser *fileParser)
{
 if (fileParser == NULL) {
    return false;
  }
  fileParser->remHandler(&myOriginLatLongAltCB);
 
  return true;

} // end method remFromFileParser


MVREXPORT MvrTime MvrMapSupplement::getTimeChanged() const
{
  return myTimeChanged;
}
  

MVREXPORT void MvrMapSupplement::clear()
{
  myTimeChanged.setToNow();

  myHasOriginLatLongAlt = false;
  myOriginLatLong.setPose(0, 0);
  myOriginAltitude = 0;

} // end method clear


/// Gets if this map has an OriginLatLong or not
MVREXPORT bool MvrMapSupplement::hasOriginLatLongAlt()
{
  return myHasOriginLatLongAlt;
}

/// Returns the latitude/longitude origin of the map; valid only if hasOriginLatLongAlt returns true
MVREXPORT MvrPose MvrMapSupplement::getOriginLatLong()
{
  return myOriginLatLong;
}

/// Returns the altitude of the origin; valid only if hasOriginLatLongAlt returns true
MVREXPORT double MvrMapSupplement::getOriginAltitude()
{
  return myOriginAltitude;
}


MVREXPORT void MvrMapSupplement::setOriginLatLongAlt(bool hasOriginLatLongAlt,
                                             const MvrPose &originLatLong,
                                             double altitude,
                                             MvrMapChangeDetails *changeDetails)
{


  if ((myHasOriginLatLongAlt == hasOriginLatLongAlt) &&
      (myOriginLatLong == originLatLong) &&
      (fabs(myOriginAltitude - altitude) < MvrMath::epsilon())) {
    return;
  }

  MvrMapFileLineSet origLines;

  if (changeDetails != NULL) {
    MvrMapFileLineSetWriter origWriter(changeDetails->getChangedSupplementLines
                                  (MvrMapChangeDetails::DELETIONS));

    if (myHasOriginLatLongAlt) {
      MvrUtil::functorPrintf(&origWriter, "OriginLatLongAlt: %f %f %f%s", 
			                      myOriginLatLong.getX(), myOriginLatLong.getY(),
			                      myOriginAltitude, EOL_CHARS);
    }
    else {
      MvrUtil::functorPrintf(&origWriter, "OriginLatLongAlt:%s", 
			                      EOL_CHARS);
    }
  }
	myHasOriginLatLongAlt = hasOriginLatLongAlt;
  myOriginLatLong = originLatLong;
  myOriginAltitude = altitude;

  if (changeDetails != NULL) {
    MvrMapFileLineSetWriter newWriter(changeDetails->getChangedSupplementLines
                                  (MvrMapChangeDetails::ADDITIONS));

    if (myHasOriginLatLongAlt) {
      MvrUtil::functorPrintf(&newWriter, "OriginLatLongAlt: %f %f %f%s", 
			                      myOriginLatLong.getX(), myOriginLatLong.getY(),
			                      myOriginAltitude, EOL_CHARS);
    }
    else { 
      MvrUtil::functorPrintf(&newWriter, "OriginLatLongAlt:%s", 
			                      EOL_CHARS);

    }
  }
}


MVREXPORT void MvrMapSupplement::writeSupplementToFunctor
                              (MvrFunctor1<const char *> *functor, 
			                         const char *endOfLineChars)
{
 
  if (myHasOriginLatLongAlt) {
    MvrUtil::functorPrintf(functor, "OriginLatLongAlt: %f %f %f%s", 
                          myOriginLatLong.getX(), myOriginLatLong.getY(),
                          myOriginAltitude, endOfLineChars);
  }

} // end method writeSupplementToFunctor


bool MvrMapSupplement::handleOriginLatLongAlt(MvrArgumentBuilder *arg)
{
  if (arg->getArgc() >= 3) {
 
    bool xOk   = true;
    bool yOk   = true;
    bool altOk = true;

    double x = arg->getArgDouble(0, &xOk);
    double y = arg->getArgDouble(1, &yOk);
    double alt = arg->getArgDouble(2, &altOk);

    if (xOk && yOk && altOk) {
      myHasOriginLatLongAlt = true;
      myOriginLatLong.setPose(x, y);
      myOriginAltitude = alt;
      return true;    
    }
  }
  else if (arg->getArgc() == 0) {

    // This is to handle the special and unlikely change case above -- where 
    // the map originally has an origin lat/long, but then is changed to not have
    // one
    myHasOriginLatLongAlt = false;
    myOriginLatLong.setPose(0,0);
    myOriginAltitude = 0;
    return true;

  }

  MvrLog::log(MvrLog::Verbose, 
	           "MvrMap::handleOriginLatLongAlt: line wrong, should be x, y, altitude point (in lat long altitude) as doulbles but is %s", arg->getFullString());

  return false;

} // end method handleOriginLatLongAlt
  



// ---------------------------------------------------------------------------- 
// ---------------------------------------------------------------------------- 
// -----------------------------------------------------------------------------
// MvrMapSimple
// -----------------------------------------------------------------------------

// TODO: Should these constants be somewhere else?
/**
const char *MvrMapSimple::ourDefaultInactiveInfoNames[INFO_COUNT] =
{ 
  "_MapInfo:",
  "_MetaInfo:",
  "_TaskInfo:",
  "_RouteInfo:",
  "_SchedTaskInfo:",
  "_SchedInfo:",
  "_CairnInfo:",
  "_CustomInfo:"
};
**/

int MvrMapSimple::ourTempFileNumber = 0;

MvrMutex MvrMapSimple::ourTempFileNumberMutex;

MVREXPORT int MvrMapSimple::getNextFileNumber()
{
  ourTempFileNumberMutex.lock();
  ourTempFileNumber++;
  int ret = ourTempFileNumber;
  ourTempFileNumberMutex.unlock();
  
  return ret;

} // end method getNextFileNumber
  
MVREXPORT void MvrMapSimple::invokeCallbackList(std::list<MvrFunctor*> *cbList)
{
  if (cbList == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::invokeCallbackList failed because list is null");
    return;
  }
  for (std::list<MvrFunctor*>::iterator iter = cbList->begin();
       iter != cbList->end();
       iter++) {
    MvrFunctor *cb = *iter;
    if (cb == NULL) {
      continue;
    }
    cb->invoke();
  } 
} // end method invokeCallbackList
  
MVREXPORT void MvrMapSimple::addToCallbackList(MvrFunctor *functor,
                                             MvrListPos::Pos position,
                                             std::list<MvrFunctor*> *cbList)
{
  if (functor == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::addToCallbackList cannot add null functor");
    return;
  }
  if (cbList == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::addToCallbackList cannot add functor to null list");
    return;
  }

  switch (position) {
  case MvrListPos::FIRST:
    cbList->push_front(functor);
    break;
  case MvrListPos::LAST:
    cbList->push_back(functor);
    break;
  default:
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::addToCallbackList invalid position (%i)",
               position);
  } // end switch
} // end method addToCallbackList

MVREXPORT void MvrMapSimple::remFromCallbackList(MvrFunctor *functor,
                                               std::list<MvrFunctor*> *cbList)
{
  if (functor == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::remFromCallbackList cannot remove null functor");
    return;
  }
  if (cbList == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::addToCallbackList cannot remove functor to null list");
    return;
  }
  cbList->remove(functor);

} // end method remFromCallbackList

MVREXPORT MvrMapSimple::MvrMapSimple(const char *baseDirectory,
                                  const char *tempDirectory,
                                  const char *overrideMutexName)  :
  myMutex(),
  
  myMapCategoryList(),
  myMapCategory(),

  myChecksumCalculator(new MvrMD5Calculator()),

  myBaseDirectory((baseDirectory != NULL) ? baseDirectory : ""),
  myFileName(),
  myReadFileStat(),

  myPreWriteCBList(),  
  myPostWriteCBList(),

  myIsWriteToTempFile(tempDirectory != NULL),
  myTempDirectory((tempDirectory != NULL) ? tempDirectory : ""),

  myMapId(),

  myLoadingParser(NULL),

  myIgnoreEmptyFileName(false), // ignoreEmptyFileName),
  myIgnoreCase(false),

  myMapChangedHelper(new MvrMapChangedHelper()),

  myLoadingGotMapCategory(false),
  myLoadingDataStarted(false),
  myLoadingLinesAndDataStarted(false),

  myMapInfo(new MvrMapInfo()),
  myMapObjects(new MvrMapObjects()),
  myMapSupplement(new MvrMapSupplement()),

  myScanTypeList(),
  myTypeToScanMap(),
  mySummaryScan(NULL),
  
  myLoadingDataTag(),
  myLoadingScan(NULL),

  // Use special keywords for the inactive elements.
  myInactiveInfo(new MvrMapInfo(NULL, 0, "_")), 

  myInactiveObjects(new MvrMapObjects("_Cairn:")), 

  myChildObjects(new MvrMapObjects("ChildCairn:")), 

  myMapObjectNameToParamsMap(), 
  myRemainderList(),

  myTimeMapInfoChanged(),
  myTimeMapObjectsChanged(),
  myTimeMapScanChanged(),
  myTimeMapSupplementChanged(),

  myMapCategoryCB(this, &MvrMapSimple::handleMapCategory),
  mySourcesCB(this, &MvrMapSimple::handleSources),
  myDataIntroCB(this, &MvrMapSimple::handleDataIntro),
  myRemCB(this, &MvrMapSimple::handleRemainder),


  myIsQuiet(false),
  myIsReadInProgress(false),
  myIsCancelRead(false)

{
  if (overrideMutexName == NULL) {
    myMutex.setLogName("MvrMapSimple::myMutex");
  }
  else {
    myMutex.setLogName(overrideMutexName);
    //myMutex.setLog(true);
  }

  MvrUtil::appendSlash(myTempDirectory);
  MvrUtil::fixSlashes(myTempDirectory);


  myMapCategoryList.push_back(MAP_CATEGORY_2D);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_MULTI_SOURCES);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_EXTENDED);
  myMapCategoryList.push_back(MAP_CATEGORY_2D_COMPOSITE);

  myMapCategory = MAP_CATEGORY_2D;

  // Create the default scan for the sick laser.  
  MvrMapScan *mapScan = new MvrMapScan(ARMAP_DEFAULT_SCAN_TYPE);
  // TODO This needs to be a constant!!
  myScanTypeList.push_back(ARMAP_DEFAULT_SCAN_TYPE);
  myTypeToScanMap[ARMAP_DEFAULT_SCAN_TYPE] = mapScan;

  /***
  std::list<std::string> inactiveInfoNames = createInactiveInfoNames
                                                (myMapInfo->getInfoNames());
  myInactiveInfo->resetInfoNames(inactiveInfoNames);
  ***/

  reset();
}  

MVREXPORT MvrMapSimple::MvrMapSimple(const MvrMapSimple &other) :
  myMutex(),
  myMapCategoryList(other.myMapCategoryList),
  myMapCategory(other.myMapCategory),
  myChecksumCalculator((other.myChecksumCalculator != NULL) ? 
                      new MvrMD5Calculator() :
                      NULL),

  myBaseDirectory(other.myBaseDirectory),
  myFileName(other.myFileName),
  myReadFileStat(other.myReadFileStat),

  myPreWriteCBList(),  // Do not copy the callbacks...
  myPostWriteCBList(), 
  myIsWriteToTempFile(other.myIsWriteToTempFile),
  myTempDirectory(other.myTempDirectory),

  myMapId(other.myMapId),
  myLoadingParser(NULL), // TODO


  //myConfigParam(other.myConfigParam),
  myIgnoreEmptyFileName(other.myIgnoreEmptyFileName),
  myIgnoreCase(other.myIgnoreCase),

  myMapChangedHelper(new MvrMapChangedHelper()), // Do not want to copy the other one

  // things for our config
  //myConfigProcessedBefore(other.myConfigProcessedBefore),
  //myConfigMapName(),

  myLoadingGotMapCategory(other.myLoadingGotMapCategory), 
  myLoadingDataStarted(other.myLoadingDataStarted), 
  myLoadingLinesAndDataStarted(other.myLoadingLinesAndDataStarted), 

  myMapInfo(new MvrMapInfo(*other.myMapInfo)),
  myMapObjects(new MvrMapObjects(*other.myMapObjects)),
  myMapSupplement(new MvrMapSupplement(*other.myMapSupplement)),
  myScanTypeList(other.myScanTypeList),
  myTypeToScanMap(),
  mySummaryScan(NULL),

  // TODO Need to set this 
  myLoadingDataTag(),
  myLoadingScan(NULL),

  myInactiveInfo(new MvrMapInfo(*other.myInactiveInfo)),
  myInactiveObjects(new MvrMapObjects(*other.myInactiveObjects)),

  myChildObjects(new MvrMapObjects(*other.myChildObjects)),

  myMapObjectNameToParamsMap(), // since this is a cache, ok not to copy
  myRemainderList(),

  myTimeMapInfoChanged(other.myTimeMapInfoChanged),  // TODO Or now??
  myTimeMapObjectsChanged(other.myTimeMapObjectsChanged), 
  myTimeMapScanChanged(other.myTimeMapScanChanged), 
  myTimeMapSupplementChanged(other.myTimeMapSupplementChanged),

  // callbacks
  myMapCategoryCB(this, &MvrMapSimple::handleMapCategory),
  mySourcesCB(this, &MvrMapSimple::handleSources),
  myDataIntroCB(this, &MvrMapSimple::handleDataIntro),
  myRemCB(this, &MvrMapSimple::handleRemainder),

  myIsQuiet(false),
  myIsReadInProgress(false),
  myIsCancelRead(false)
{
  myMapId.log("MvrMapSimple::copy_ctor");

  myMutex.setLogName("MvrMapSimple::myMutex");
  

  for (MvrTypeToScanMap::const_iterator iter = 
            other.myTypeToScanMap.begin();
       iter != other.myTypeToScanMap.end();
       iter++) {
    myTypeToScanMap[iter->first] = new MvrMapScan(*(iter->second));
  }

  if (other.mySummaryScan != NULL) {
    mySummaryScan = new MvrMapScan(*other.mySummaryScan);
  }  // end if other has summary

 //strncpy(myConfigMapName, other.myConfigMapName, MAX_MAP_NAME_LENGTH);
 // myConfigMapName[MAX_MAP_NAME_LENGTH - 1] = '\0';

  reset();

} // end copy ctor


MVREXPORT MvrMapSimple &MvrMapSimple::operator=(const MvrMapSimple &other)
{
  if (&other != this) {

    // TODO: Change this to figure out deltas if desired...
    // (or add another method that would assign with delta compute)

    lock();

    myMapCategoryList = other.myMapCategoryList;
    myMapCategory     = other.myMapCategory;

    delete myChecksumCalculator;
    myChecksumCalculator = NULL;
    if (other.myChecksumCalculator != NULL) {
      myChecksumCalculator = new MvrMD5Calculator();
    }

    myBaseDirectory     = other.myBaseDirectory;
    myFileName          = other.myFileName;
    myReadFileStat      = other.myReadFileStat;
  
    // myPreWriteCBList(),  // Do not overwrite the callbacks...?
    // myPostWriteCBList(), 

    myIsWriteToTempFile = other.myIsWriteToTempFile;
    myTempDirectory     = other.myTempDirectory;

    myMapId = other.myMapId;

    myMapId.log("MvrMapSimple::operator=");
    //myLoadingParser = NULL; // TODO

    //myConfigParam = other.myConfigParam;
    myIgnoreEmptyFileName = other.myIgnoreEmptyFileName;
    myIgnoreCase = other.myIgnoreCase;

    // Do not copy the map changed helper

    // things for our config
    //myConfigProcessedBefore = other.myConfigProcessedBefore;
    //strncpy(myConfigMapName, other.myConfigMapName, MAX_MAP_NAME_LENGTH);
    //myConfigMapName[MAX_MAP_NAME_LENGTH - 1] = '\0';

    myLoadingGotMapCategory = other.myLoadingGotMapCategory; 
    myLoadingDataStarted = other.myLoadingDataStarted; 
    myLoadingLinesAndDataStarted = other.myLoadingLinesAndDataStarted; 

    *myMapInfo = *other.myMapInfo;
    *myMapObjects = *other.myMapObjects;
    *myMapSupplement = *other.myMapSupplement;

    delete mySummaryScan;
    mySummaryScan = NULL;

    myScanTypeList = other.myScanTypeList;
    
    MvrUtil::deleteSetPairs(myTypeToScanMap.begin(), myTypeToScanMap.end());
    myTypeToScanMap.clear();

    for (MvrTypeToScanMap::const_iterator iter = 
              other.myTypeToScanMap.begin();
         iter != other.myTypeToScanMap.end();
         iter++) {
      myTypeToScanMap[iter->first] = new MvrMapScan(*(iter->second));
    }
    if (other.mySummaryScan != NULL) {
      mySummaryScan = new MvrMapScan(*other.mySummaryScan);
    }  // end if other has summary

    myLoadingDataTag = other.myLoadingDataTag;
    myLoadingScan = NULL;
    if (other.myLoadingScan != NULL) {
      MvrTypeToScanMap::iterator scanIter = 
        myTypeToScanMap.find(other.myLoadingScan->getScanType());
      if (scanIter != myTypeToScanMap.end()) {
        myLoadingScan = scanIter->second;
      }
    }

    *myInactiveInfo = *other.myInactiveInfo;
    *myInactiveObjects = *other.myInactiveObjects;
 
    *myChildObjects = *other.myChildObjects;

    // Since the myMapObjectNameToParamsMap is a cache, there's no
    // real need to copy the other one
    MvrUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                           myMapObjectNameToParamsMap.end());
    myMapObjectNameToParamsMap.clear();

    MvrUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
    myRemainderList.clear();
    for (std::list<MvrArgumentBuilder *>::const_iterator remIter = other.myRemainderList.begin();
         remIter != other.myRemainderList.end();
         remIter++) {
      MvrArgumentBuilder *arg = *remIter;
      if (arg == NULL) {
        continue; // Should never happen
      }
      myRemainderList.push_back(new MvrArgumentBuilder(*arg));
    }

    // The various time flags represent the last time that mapChanged()
    // was invoked.  Since it has not yet been run for this instance of
    // the map, DO NOT update flags. (Doing so may prevent the updated 
    // map from being downloaded to client MobileEyes apps.)
    //
    // myTimeMapInfoChanged = other.myTimeMapInfoChanged; 
    // myTimeMapObjectsChanged = other.myTimeMapObjectsChanged; 
    // myTimeMapScanChanged = other.myTimeMapScanChanged; 
    // myTimeMapSupplementChanged = other.myTimeMapSupplementChanged;
    
    myIsQuiet = other.myIsQuiet; 
    myIsReadInProgress = other.myIsReadInProgress;
    myIsCancelRead = other.myIsCancelRead;

    // Primarily to get the new base directory into the file parser
    reset(); 

    // Think its best if the caller invokes mapChanged as necessary.
    // mapChanged();

    unlock();

  }

  return *this;

} // end operator=


MVREXPORT MvrMapSimple::~MvrMapSimple(void)
{ 

  if (myIsReadInProgress) {

    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::dtor() map file is being read");
    myIsCancelRead = true;
    if (myLoadingParser != NULL) {
      myLoadingParser->cancelParsing();
    }

    // Wait a little while to see if the file read can be cancelled
    for (int i = 0; ((i < 20) && (myIsReadInProgress)); i++) {
      MvrUtil::sleep(5);
    }
    if (myIsReadInProgress) {
      MvrLog::log(MvrLog::Normal,
                 "MvrMapSimple::dtor() map file is still being read");
    }

  } // end if read in progress

  delete myChecksumCalculator;
  myChecksumCalculator = NULL;

  delete myLoadingParser;
  myLoadingParser = NULL;

  delete myMapChangedHelper;
  myMapChangedHelper = NULL;

  delete myMapInfo;
  // const, so don't myMapInfo = NULL;

  delete myMapObjects;
  // const, so don't myMapObjects = NULL;

  delete myMapSupplement;
  // const, so don't myMapSupplement = NULL;
 
  myScanTypeList.clear();
  MvrUtil::deleteSetPairs(myTypeToScanMap.begin(),
                         myTypeToScanMap.end());
  myTypeToScanMap.clear();


  delete mySummaryScan;
  mySummaryScan = NULL;

  // This is a reference to one of the scans deleted above, so just
  // clear the pointer.
  myLoadingScan = NULL;

  delete myInactiveInfo;
  // const, so don't myInactiveInfo = NULL;

  delete myInactiveObjects;
  // const, so don't myInactiveObjects = NULL;

  delete myChildObjects;
  // const, so don't myChildObjects = NULL;

  MvrUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();

  MvrUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();
        
} // end dtor 
  

MVREXPORT MvrMapInterface *MvrMapSimple::clone()
{
  return new MvrMapSimple(*this);
}

MVREXPORT bool MvrMapSimple::set(MvrMapInterface *other)
{
  if (other == NULL) {
    return false;
  }
  if (getInfoCount() != other->getInfoCount()) {
    return false;
  }

  lock();

  myBaseDirectory = ((other->getBaseDirectory() != NULL) ?
                          other->getBaseDirectory() : "");

  myFileName = ((other->getFileName() != NULL) ?
                          other->getFileName() : "");

  myReadFileStat = other->getReadFileStat();

  myIsWriteToTempFile = (other->getTempDirectory() != NULL);
  myTempDirectory   = ((other->getTempDirectory() != NULL) ?
                                   other->getTempDirectory() : "");

  other->getMapId(&myMapId);

  myMapId.log("MvrMapSimple::set");
  
  myLoadingLinesAndDataStarted = 
                          other->isLoadingLinesAndDataStarted();
  myLoadingDataStarted = 
                          other->isLoadingDataStarted();


  std::list<std::string> otherInfoNames = other->getInfoNames();
  for (std::list<std::string>::const_iterator infoIter = 
                                                  otherInfoNames.begin();
       infoIter != otherInfoNames.end();
       infoIter++) {
    const char *infoName = (*infoIter).c_str();
    setInfo(infoName, other->getInfo(infoName));
  }
  // int infoCount = other->getInfoCount();
  //for (int i = 0; i < infoCount; i++) {
  //  setInfo(i, other->getInfo(i));
  //} // end for each info type
 
  setMapObjects(other->getMapObjects());

  createScans((other->getScanTypes()));


  for (std::list<std::string>::iterator tIter1 = myScanTypeList.begin();
       tIter1 != myScanTypeList.end();
       tIter1++) {
    const char *scanType = (*tIter1).c_str();

    setPoints(other->getPoints(scanType), scanType, other->isSortedPoints());
    setLines(other->getLines(scanType), scanType, other->isSortedLines());
    setResolution(other->getResolution(scanType), scanType);

  } // end for each scan type

  setOriginLatLongAlt(other->hasOriginLatLongAlt(), 
                      other->getOriginLatLong(),
                      other->getOriginAltitude());


  // Not sure about the implications of doing this...
  for (std::list<std::string>::const_iterator infoIter2 = 
                                                  otherInfoNames.begin();
       infoIter2 != otherInfoNames.end();
       infoIter2++) {
    const char *infoName = (*infoIter2).c_str();
    setInactiveInfo(infoName, other->getInactiveInfo()->getInfo(infoName));
  }

  setInactiveObjects(other->getInactiveObjects()->getMapObjects());

  setChildObjects(other->getChildObjects()->getMapObjects());

  updateSummaryScan();
        
  // Since the myMapObjectNameToParamsMap is a cache, there's no
  // real need to copy the other one
  MvrUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();


  MvrUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();

  std::list<MvrArgumentBuilder *> *otherRemainderList = other->getRemainder();

  if (otherRemainderList != NULL) {
    for (std::list<MvrArgumentBuilder *>::const_iterator remIter = otherRemainderList->begin();
          remIter != otherRemainderList->end();
          remIter++) {
      MvrArgumentBuilder *arg = *remIter;
      if (arg == NULL) {
        continue; // Should never happen
      }
      myRemainderList.push_back(new MvrArgumentBuilder(*arg));
    } // end for each remainder
  } // end if remainder list not null

  updateMapCategory();

  reset();

  unlock();

  return true;

} // end method set 



MVREXPORT void MvrMapSimple::clear()
{
  lock();  // ???

  myFileName = "";
  myMapId = MvrMapId();
  myMapId.log("MvrMapSimple::clear");

  myMapInfo->clear();
  myMapObjects->clear();
  myMapSupplement->clear();
 
  for (MvrTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    MvrMapScan *scan = iter->second;
    if (scan != NULL) {
      scan->clear();
    }
  } // end for each scan type

  myInactiveInfo->clear();
  myInactiveObjects->clear();
  myChildObjects->clear();

  MvrUtil::deleteSetPairs(myMapObjectNameToParamsMap.begin(),
                         myMapObjectNameToParamsMap.end());
  myMapObjectNameToParamsMap.clear();

  MvrUtil::deleteSet(myRemainderList.begin(), myRemainderList.end());
  myRemainderList.clear();

  reset(); 

  mapChanged(); //???

  unlock();

} // end method clear
  
MVREXPORT std::list<std::string> MvrMapSimple::getScanTypes() const
{
  return myScanTypeList;
}

MVREXPORT bool MvrMapSimple::setScanTypes(const std::list<std::string> &scanTypeList)
{
  return createScans(scanTypeList);

} // end method setScanTypes


MVREXPORT struct stat MvrMapSimple::getReadFileStat() const
{
  return myReadFileStat;
}

MVREXPORT void MvrMapSimple::reset()
{

  myLoadingGotMapCategory = false; 
  myLoadingDataStarted = false; 
  myLoadingLinesAndDataStarted = false; 

  /// HERE ///

  if (myLoadingParser != NULL) {
    delete myLoadingParser;
    myLoadingParser = NULL;
  }
  myLoadingParser = new MvrFileParser("./",  // base directory
                                     true); // precompress quotes


  myLoadingParser->setBaseDirectory(myBaseDirectory.c_str());

  // The map file cannot contain any comments.
  myLoadingParser->clearCommentDelimiters();

  myLoadingParser->setQuiet(myIsQuiet);

/// END HERE ////

  for (std::list<std::string>::iterator iter = myMapCategoryList.begin();
       iter != myMapCategoryList.end();
       iter++) {

    if (!myLoadingParser->addHandler((*iter).c_str(), &myMapCategoryCB)) {
      MvrLog::log(MvrLog::Terse, 
                 "MvrMapSimple::reset() could not add map category handler for %s",
                 (*iter).c_str());
    }
  } // end for each map category

} // end method reset
  
  
MVREXPORT bool MvrMapSimple::refresh()
{
  MvrLog::log(MvrLog::Terse, 
             "MvrMapSimple::refresh() not implemented");
  return true;
}

MVREXPORT void MvrMapSimple::updateMapFileInfo(const char *realFileName)
{
  stat(realFileName, &myReadFileStat);

  if (myChecksumCalculator != NULL) {

    myMapId = MvrMapId(myMapId.getSourceName(),
                      myMapId.getFileName(),
                      myChecksumCalculator->getDigest(),
                      MvrMD5Calculator::DIGEST_LENGTH,
                      myReadFileStat.st_size,
                      myReadFileStat.st_mtime);

    // TODO Not entirely sure whether we want to register the entire path name,
    // or just the file name...?

  } 
  else { // checksums turned off

    myMapId = MvrMapId(myMapId.getSourceName(),
                      myMapId.getFileName(),
                      NULL,
                      0,
                      myReadFileStat.st_size,
                      myReadFileStat.st_mtime);

  } // end else checksums turned off
    
  myMapId.log("MvrMapSimple::updateMapFileInfo");

  // TODO MvrMapRegistry::getIt()->registerMap(realFileName, myMapId);

} // end method updateMapFileInfo

MVREXPORT const char *MvrMapSimple::getMapCategory()
{
  if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {

    if (myScanTypeList.size() == 1) {

      const char *scanType = myScanTypeList.front().c_str();

      // Override the scan type only if it is the sick laser.  Other sensors
      // must have their scan type specified in the file.
      if (isDefaultScanType(scanType) || 
         (strcasecmp(scanType, "SickLaser") == 0)) {
        myMapCategory = MAP_CATEGORY_2D;
      }
    }
  }
  return myMapCategory.c_str();

} // end method getMapCategory


MVREXPORT void MvrMapSimple::updateMapCategory(const char *updatedInfoName)
{
  // The isDowngradeCategory flag indicates whether the map category can 
  // be downgraded (e.g. from 2D-Map-Ex2 to 2D-Map) when the map doesn't 
  // contain advanced features.  This would occur if the advanced feature 
  // was added on the server and was later removed.
  //
  // The default behavior of the server deactivates the removed items, 
  // saving them for later use.  If the map downgrades the map category,
  // big problems can occur if an old copy of the editor is used to 
  // modify the downgraded map.  If the server later re-activates the 
  // items, the map will most likely be corrupted.
  //
  // Hence the flag is set to false. 

  bool isDowngradeCategory = false;

  // This is the "top-most" map category.. If it's already been set, then there's
  // nothing to do
  if (!isDowngradeCategory &&
      strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_COMPOSITE) == 0) {
    return;
  }

  // If a GroupType MapInfo has been defined, then it must be the composite 
  // category.  
  if ((updatedInfoName == NULL) || 
      (strcasecmp(updatedInfoName, MAP_INFO_NAME) == 0)) {

    if (mapInfoContains("GroupType")) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_COMPOSITE) != 0) {
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                   MAP_CATEGORY_2D_COMPOSITE,
                   myMapCategory.c_str(),
                   "GroupType");
        myMapCategory = MAP_CATEGORY_2D_COMPOSITE;
      }
      return;
    } // end if map info contains group type

  } // end if updated info name not specified or is map info


  // TODO If a parent / child map has been defined, then it must be the composite
  // category


  // If any CairnInfo or CustomInfo have been set, then it must be the extended
  // category
  const char *extendedInfoNames[2];
  extendedInfoNames[0] = CUSTOM_INFO_NAME;
  extendedInfoNames[1] = CAIRN_INFO_NAME;

  for (int i = 0; i < 2; i++) {
    if ((updatedInfoName != NULL) &&
        (strcasecmp(updatedInfoName, extendedInfoNames[i]) != 0)) {
      continue;
    }
        
    if ((getInfo(extendedInfoNames[i]) != NULL) && 
        (!getInfo(extendedInfoNames[i])->empty())) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_EXTENDED) != 0) {
        MvrLog::log(MvrLog::Normal,
                    "MvrMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                    MAP_CATEGORY_2D_EXTENDED,
                    myMapCategory.c_str(),
                    extendedInfoNames[i]);
        myMapCategory = MAP_CATEGORY_2D_EXTENDED;
      }
      return;
    }
  } // end for each extended info name


  // Similarly, if any MapInfo's contain an ArgDesc, then it must be the extended
  // category.  (It seems alright to check this because there shouldn't be 
  // thousands of map info lines...)
  if ((updatedInfoName == NULL) || 
      (strcasecmp(updatedInfoName, MAP_INFO_NAME) == 0)) {

    if (mapInfoContains("ArgDesc")) {
      if ((strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_COMPOSITE) != 0) &&
          (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_EXTENDED) != 0)) {
        MvrLog::log(MvrLog::Normal,
                  "MvrMapSimple::updateMapCategory() changing category to %s from %s because %s found",
                  MAP_CATEGORY_2D_EXTENDED,
                  myMapCategory.c_str(),
                  "ArgDesc");
        myMapCategory = MAP_CATEGORY_2D_EXTENDED;
      }
      return;
    }

  } // end if updated info name not specified or is map info


  // Otherwise, if there is more than one scan type, it must be the multi-source
  // category
  if (!isDowngradeCategory &&
      strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {
    return;
  }

  std::list<std::string> scanTypeList = getScanTypes();

  if (scanTypeList.size() > 1) {

    if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) != 0) {
      MvrLog::log(MvrLog::Normal,
                  "MvrMapSimple::updateMapCategory() changing category to %s from %s because %i scan types",
                  MAP_CATEGORY_2D_MULTI_SOURCES,
                  myMapCategory.c_str(),
                  getScanTypes().size());
      myMapCategory = MAP_CATEGORY_2D_MULTI_SOURCES;
    }
    return;

  } // end if more than one scan type


  if (scanTypeList.size() == 1) {

    const char *scanType = scanTypeList.front().c_str();

    if (!isDefaultScanType(scanType) && 
        (strcasecmp(scanType, "SickLaser") != 0)) {
      if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D_MULTI_SOURCES) != 0) {
        MvrLog::log(MvrLog::Normal,
                    "MvrMapSimple::updateMapCategory() changing category to %s from %s because scan type is %s",
                    MAP_CATEGORY_2D_MULTI_SOURCES,
                    myMapCategory.c_str(),
                    scanType);
        myMapCategory = MAP_CATEGORY_2D_MULTI_SOURCES;
      }
      return;
    }
  }
  

  if (strcasecmp(myMapCategory.c_str(), MAP_CATEGORY_2D) != 0) {
    MvrLog::log(MvrLog::Normal,
                "MvrMapSimple::updateMapCategory() changing category to %s from %s because no special cases found",
                MAP_CATEGORY_2D,
                myMapCategory.c_str());
    myMapCategory = MAP_CATEGORY_2D;
  }

  return;

} // end method updateMapCategory


MVREXPORT bool MvrMapSimple::mapInfoContains(const char *arg0Text) 
{
  if (MvrUtil::isStrEmpty(arg0Text)) {
    return false;
  }

  std::list<MvrArgumentBuilder*> *mapInfoList = getInfo(MAP_INFO_NAME);
  if (mapInfoList != NULL) {

    for (std::list<MvrArgumentBuilder*>::const_iterator iter = mapInfoList->begin();
        iter != mapInfoList->end();
        iter++) {
      
      MvrArgumentBuilder *arg = *iter;
      if ((arg == NULL) || (arg->getArgc() < 1) || (arg->getArg(0) == NULL)) {
        continue;
      }
      if (strcasecmp(arg->getArg(0), arg0Text) == 0) {
        return true;
      } // end if arg desc found
      
    } // end for each map info line
  } // end if non-NULL map info

  return false;

} // end method mapInfoContains


MVREXPORT void MvrMapSimple::addPreWriteFileCB(MvrFunctor *functor,
                                             MvrListPos::Pos position)
{
  addToCallbackList(functor, position, &myPreWriteCBList);

} // end method addPreWriteFileCB

MVREXPORT void MvrMapSimple::remPreWriteFileCB(MvrFunctor *functor)
{
  remFromCallbackList(functor, &myPreWriteCBList);

} // end method remPreWriteFileCB

MVREXPORT void MvrMapSimple::addPostWriteFileCB(MvrFunctor *functor,
                                              MvrListPos::Pos position)
{
  addToCallbackList(functor, position, &myPostWriteCBList);

} // end method addPostWriteFileCB

MVREXPORT void MvrMapSimple::remPostWriteFileCB(MvrFunctor *functor)
{
  remFromCallbackList(functor, &myPostWriteCBList);

} // end method remPostWriteFileCB

MVREXPORT bool MvrMapSimple::readFile(const char *fileName, 
			                              char *errorBuffer, 
                                    size_t errorBufferLen,
                                    unsigned char *md5DigestBuffer,
                                    size_t md5DigestBufferLen)
{

  if (MvrUtil::isStrEmpty(fileName)) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::readFile() cannot read empty file name");
    return false;
  }

  IFDEBUG(
  MvrLog::log(MvrLog::Normal, 
             "MvrMapSimple::readFile() reading %s",
             fileName);
  );

  lock();
  myIsReadInProgress = true;

  if (myMapInfo != NULL) {
    myMapInfo->clear();
  }
  if (myMapObjects != NULL) {
    myMapObjects->clear();
  }
  if (myMapSupplement != NULL) {
    myMapSupplement->clear();
  }
  for (MvrTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    MvrMapScan *scan = iter->second;
    if (scan != NULL) {
      scan->clear();
    }
  } // end for each scan type

  if (myInactiveInfo != NULL) {
    myInactiveInfo->clear();
  }
  if (myInactiveObjects != NULL) {
    myInactiveObjects->clear();
  }
  if (myChildObjects != NULL) {
    myChildObjects->clear();
  }

  reset();

  // stat(fileName, &myReadFileStat);
  FILE *file = NULL;

  char line[10000];

  std::string realFileName = createRealFileName(fileName);

  MvrLog::log(MvrLog::Normal, 
             "Opening map file %s, given %s", 
             realFileName.c_str(), fileName);


  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.
  if ((file = MvrUtil::fopen(realFileName.c_str(), "rb")) == NULL)
  {
    MvrLog::log(MvrLog::Terse, "Cannot open file '%s'", realFileName.c_str());
    // TODO This used to put the config param name into the error buffer
    if (errorBuffer != NULL) {
      snprintf(errorBuffer, errorBufferLen, 
               "Map invalid: cannot open file '%s'",
               fileName);
    }
    myIsReadInProgress = false;
    unlock();
    return false;
  }

  MvrFunctor1<const char *> *parseFunctor = NULL;

  MvrTime parseTime;
  parseTime.setToNow();

  if (myChecksumCalculator != NULL) {
      
    myChecksumCalculator->reset();

    parseFunctor = myChecksumCalculator->getFunctor();
    myLoadingParser->setPreParseFunctor(parseFunctor);
  }
  else {
    myLoadingParser->setPreParseFunctor(NULL);
  }

  bool isSuccess = true;

  char *localErrorBuffer = NULL;
  size_t localErrorBufferLen = 0;

  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    localErrorBufferLen = errorBufferLen;
    localErrorBuffer = new char[localErrorBufferLen];
    localErrorBuffer[0] = '\0';
  }

  if (// TODO !reset() || 
      !myLoadingParser->parseFile(file, line, 10000, 
                                  false,  // Do not continue on error
                                  localErrorBuffer, localErrorBufferLen))
  {
    // If the error buffer is specified, then just make absolutely sure
    // that it is null terminated
    if ((localErrorBuffer != NULL) && (localErrorBufferLen > 0)) {
      localErrorBuffer[localErrorBufferLen - 1] = '\0';
    }

    if (myLoadingDataTag.empty()) 
    // if (!myLoadingDataStarted && !myLoadingLinesAndDataStarted) 
    {
      // TODO reset();
      if (errorBuffer != NULL) {

    // TODO This used to put the config param name into the error buffer
        snprintf(errorBuffer, errorBufferLen, 
                 "Map invalid: '%s' not a valid map (%s)",
                 fileName, localErrorBuffer);
        errorBuffer[errorBufferLen - 1] = '\0';
      }
      //unlock();
      MvrLog::log(MvrLog::Terse, "Could not load map file '%s'", fileName);

      isSuccess = false;
    }
  } // end if parse success

  delete [] localErrorBuffer;

  if (!myLoadingGotMapCategory)
  {
    // TODO reset();
    if (errorBuffer != NULL) {
     // TODO This used to put the config param name into the error buffer
     snprintf(errorBuffer, errorBufferLen, 
               "Map invalid, '%s' was not a map file",
               fileName);
    }

    //unlock();
    MvrLog::log(MvrLog::Terse, 
               "Could not load map file '%s' it was not a recognized map format",              fileName);
    isSuccess = false;
  }

  bool isLineDataTag = false; // TODO 
  bool isEndOfFile = false;
  
  myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                          &isLineDataTag);

  isSuccess = (myLoadingScan != NULL);

  while (isSuccess && !isEndOfFile && !myIsCancelRead) {
    
    bool isDataTagFound = false;

    while ((fgets(line, sizeof(line), file) != NULL) && !myIsCancelRead) 
    {
      if (parseFunctor != NULL) {
        parseFunctor->invoke(line);
      }

      if (isDataTag(line)) // strncasecmp(line, "DATA", strlen("DATA")) == 0)
      {
        //myLoadingDataTag = line;
        isDataTagFound = true;
        break;
      }
      if (isLineDataTag && !readLineSegment(line))
      {
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::readFile() error reading line data '%s'",
                   line);
        continue;
      }
      else if (!isLineDataTag && !readDataPoint(line))
      {
        continue;
      }
    } // end while more lines to read

    if (isDataTagFound) {
    
      myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                              &isLineDataTag);
      isSuccess = (myLoadingScan != NULL);

      if (myLoadingScan != NULL) {

        MvrLog::log(MvrLog::Verbose,
                   "MvrMapSimple::readFile() found scan type %s for data tag %s (is line = %i)",
                   myLoadingScan->getScanType(),
                   myLoadingDataTag.c_str(),
                   isLineDataTag);
      }
      else {
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::readFile() cannot find scan for data tag %s (is line = %i)",
                   myLoadingDataTag.c_str(),
                   isLineDataTag);
      }
    }
    else { // else must be end of file

      isEndOfFile = true;
      MvrLog::log(MvrLog::Verbose,
                 "MvrMapSimple::readFile() end of file found");

    } // end else end of file

  }  // end while no error and not end of file


  updateSummaryScan();


  int elapsed = parseTime.mSecSince();

  MvrLog::log(MvrLog::Normal, 
             "MvrMapSimple::readFile() %s took %i msecs to read map of %i points",
             realFileName.c_str(),
             elapsed,
             getNumPoints(ARMAP_SUMMARY_SCAN_TYPE));	

  fclose(file);

  if (!myIsCancelRead) {
    updateMapFileInfo(realFileName.c_str());

    //stat(realFileName.c_str(), &myReadFileStat);

    if (myChecksumCalculator != NULL) {
      
      if (md5DigestBuffer != NULL) {
        memset(md5DigestBuffer, 0, md5DigestBufferLen);
        memcpy(md5DigestBuffer, myChecksumCalculator->getDigest(), 
              MvrUtil::findMin(md5DigestBufferLen, MvrMD5Calculator::DIGEST_LENGTH));
      }

      myLoadingParser->setPreParseFunctor(NULL);
    }

    if (isSuccess) {
      // move the stuff over from reading to new
      myFileName = fileName;
    
      MvrLog::log(myMapChangedHelper->getMapChangedLogLevel(), 
                "MvrMapSimple:: Calling mapChanged()");	
      mapChanged();
      MvrLog::log(myMapChangedHelper->getMapChangedLogLevel(), 
                "MvrMapSimple:: Finished mapChanged()");

    }
  } // end if not cancelling

  myIsReadInProgress = false;

  unlock();
  return isSuccess;

} // end method readFile


MVREXPORT bool MvrMapSimple::isDataTag(const char *line) 
{
  // Pre: Line is not null
  MvrDataTagToScanTypeMap::iterator typeIter =  
            myDataTagToScanTypeMap.find(line);

  if (typeIter != myDataTagToScanTypeMap.end()) {
    myLoadingDataTag = typeIter->first;
    return true;
  }
  return false;

} // end method isDataTag


MVREXPORT MvrMapScan *MvrMapSimple::findScanWithDataKeyword
                                     (const char *loadingDataTag,
                                      bool *isLineDataTagOut)
{
  //MvrLog::log(MvrLog::Normal,
  //           "MvrMapSimple::findScanWithDataKeyword() looking for scan with tag %s",
  //           loadingDataTag);

  if (MvrUtil::isStrEmpty(loadingDataTag)) {
    return NULL;
  }
  MvrDataTagToScanTypeMap::iterator typeIter =  
            myDataTagToScanTypeMap.find(loadingDataTag);
  if (typeIter == myDataTagToScanTypeMap.end()) {
    return NULL;
  }

  MvrTypeToScanMap::iterator scanIter =
            myTypeToScanMap.find(typeIter->second);

  if (scanIter == myTypeToScanMap.end()) {
    return NULL;
  }

  MvrMapScan *mapScan = scanIter->second;
  if (mapScan == NULL) {
    return NULL;
  }

  bool isLineDataTag = (MvrUtil::strcasecmp(loadingDataTag,
                                           mapScan->getLinesKeyword()) == 0);
 
  //MvrLog::log(MvrLog::Normal,
  //           "MvrMapSimple::findScanWithDataKeyword() found scan for tag %s, isLineData = %i",
  //           loadingDataTag,
  //           isLineDataTag);

  
  if (isLineDataTag) {
    myLoadingLinesAndDataStarted = true;
  }
  else {
    myLoadingDataStarted = true;
  }

  if (isLineDataTagOut != NULL) {
    *isLineDataTagOut = isLineDataTag;
  }

  return mapScan;
  
} // end method findScanWithDataKeyword


MVREXPORT bool MvrMapSimple::writeFile(const char *fileName, 
                                     bool internalCall,
                                     unsigned char *md5DigestBuffer,
                                     size_t md5DigestBufferLen,
                                     time_t fileTimestamp)
{ 
  FILE *file = NULL;
  if (!internalCall)
    lock();

  // Calling updateMapCategory here just in case the file is being 
  // written because the associated info has changed (as in the case
  // of AramMapInfoMinder).
  updateMapCategory();

  invokeCallbackList(&myPreWriteCBList);

	std::string realFileName = createRealFileName(fileName);
	std::string writeFileName;

  if (myIsWriteToTempFile) {

    char tempFileName[3200];
    int tempFileNameLen = 3200;

    tempFileName[0] = '\0';

    int fileNumber = getNextFileNumber();

// Hoping that this is highly temporary...
#ifdef WIN32
    snprintf(tempFileName, tempFileNameLen,
             "%sMvrMap.%d.%d", 
             myTempDirectory.c_str(), _getpid(), fileNumber);
#else // linux
    snprintf(tempFileName, tempFileNameLen,
             "%sMvrMap.%d.%d", 
             myTempDirectory.c_str(), getpid(), fileNumber);
#endif  // end else linux

    MvrLog::log(MvrLog::Normal,
               "Writing map %s to temp file %s\n",
               fileName,
               tempFileName);

    writeFileName = tempFileName;

  }
  else { // write to actual file

    writeFileName = realFileName;

  } // end else write to actual file

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.

  if ((file = MvrUtil::fopen(writeFileName.c_str(), "wb")) == NULL)
  {
    bool isFileSuccess = false;

    MvrLog::log(MvrLog::Terse, 
               "MvrMap: Cannot open file '%s' for writing",
	             writeFileName.c_str());
  
    invokeCallbackList(&myPostWriteCBList);

    if (!internalCall)
      unlock();

    return false;

  } // end if error opening file for writing


  MvrTime writeTime;
  writeTime.setToNow();

  MvrFunctor1<const char *> *writeFunctor = NULL;

  MvrGlobalFunctor2<const char *, FILE *> functor(&MvrUtil::writeToFile, "", file);

  
  if (myChecksumCalculator != NULL) { 
    MvrLog::log(MvrLog::Normal, 
               "MvrMapSimple::writeFile() recalculating checksum");

    myChecksumCalculator->reset();

    // Note that this is reset to NULL below before it leaves the scope
    // of this method.
    myChecksumCalculator->setSecondFunctor(&functor);
    writeFunctor = myChecksumCalculator->getFunctor();
  }
  else {
    writeFunctor = &functor;
  }

  writeToFunctor(writeFunctor, "\n");
    
  int elapsed = writeTime.mSecSince();

  MvrLog::log(MvrLog::Normal, 
             "MvrMapSimple::writeFile() took %i msecs to write map of %i points",
             elapsed,
             getNumPoints());	


  fclose(file);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (myIsWriteToTempFile) {

    char systemBuf[6400];
    int  systemBufLen = 6400;

#ifndef WIN32
    const char *moveCmdName = "mv -f";
#else
    const char *moveCmdName = "move";
#endif

    int printLen = snprintf(systemBuf, systemBufLen,
                            "%s \"%s\" \"%s\"", 
                            moveCmdName, 
                            writeFileName.c_str(), 
                            realFileName.c_str());
    systemBuf[systemBufLen - 1] = '\0';

    int ret = -1;

    if ((printLen >= 0) && (printLen < systemBufLen)) {

      ret = system(systemBuf);

    } // end if success creating command
  
    if (ret != 0) {

      MvrLog::log(MvrLog::Terse, 
                 "Error saving map file %s.  Temp file cannot be moved. (%s)", 
                 fileName,
                 systemBuf);
      if (!internalCall)
        unlock();

      return false;

    } // end if error moving file

  } // end if write to temp file

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // The file info needs to be set to the real file name (not the possible temp one)

  if (fileTimestamp != -1) {

    MvrLog::log(MvrLog::Normal, 
               "MvrMapSimple::writeFile() setting time of %s",
               realFileName.c_str());	
    bool isTimeChanged = MvrUtil::changeFileTimestamp(realFileName.c_str(), 
                                                     fileTimestamp);

  }

// KMC TODO Wouldn't it make sense to change the myFileName member?
  myFileName = fileName;



  updateMapFileInfo(realFileName.c_str());

  // Reset the file statistics to reflect the newly written file.	
	stat(realFileName.c_str(), &myReadFileStat);

  if (myChecksumCalculator != NULL) {

    if (md5DigestBuffer != NULL) {
      if (md5DigestBufferLen != MvrMD5Calculator::DIGEST_LENGTH) {
        // log warning
      }
      memset(md5DigestBuffer, 0, md5DigestBufferLen);
      memcpy(md5DigestBuffer, myChecksumCalculator->getDigest(), 
            MvrUtil::findMin(md5DigestBufferLen, MvrMD5Calculator::DIGEST_LENGTH));
    }

    // Reset to NULL before the functor leaves the scope of this method.
    myChecksumCalculator->setSecondFunctor(NULL);

  } // end if checksum calculated
    
  invokeCallbackList(&myPostWriteCBList);

  MvrLog::log(MvrLog::Normal, "Saved map file %s", fileName);
  if (!internalCall)
    unlock();


  return true;

} // end method writeFile


MVREXPORT bool MvrMapSimple::calculateChecksum(unsigned char *md5DigestBuffer,
                                             size_t md5DigestBufferLen)
{

  if ((md5DigestBuffer == NULL) || 
      (md5DigestBufferLen < MvrMD5Calculator::DIGEST_LENGTH)) {
    return false;
  }

  lock();
  
  bool isLocalCalculator = false;
  MvrMD5Calculator *calculator = myChecksumCalculator;
  if (calculator == NULL) {
    isLocalCalculator = true;
    calculator = new MvrMD5Calculator();
  }

  memset(md5DigestBuffer, 0, md5DigestBufferLen);

  calculator->reset();
  writeToFunctor(calculator->getFunctor(), "\n");

  memcpy(md5DigestBuffer, calculator->getDigest(), 
         MvrMD5Calculator::DIGEST_LENGTH);

  if (isLocalCalculator) {
    delete calculator;
  }

  unlock();

  return true;

} // end method calculateChecksum


MVREXPORT const char *MvrMapSimple::getBaseDirectory(void) const
{ 
  return myBaseDirectory.c_str();

} // end method getBaseDirectory


MVREXPORT const char *MvrMapSimple::getFileName(void) const 
{
  return myFileName.c_str();

} // end method getFileName


void MvrMapSimple::setIgnoreEmptyFileName(bool ignore)
{ 
  myIgnoreEmptyFileName = ignore;

} // end method setIgnoreEmptyFileName


bool MvrMapSimple::getIgnoreEmptyFileName(void)
{ 
  return myIgnoreEmptyFileName;

} // end method getIgnoreEmptyFileName


void MvrMapSimple::setIgnoreCase(bool ignoreCase)
{ 
  myIgnoreCase = ignoreCase;

} // end method setIgnoreCase


bool MvrMapSimple::getIgnoreCase(void) 
{ 
  return myIgnoreCase;

} // end method getIgnoreCase


MVREXPORT void MvrMapSimple::setBaseDirectory(const char *baseDirectory)
{ 
  if (baseDirectory != NULL) {
    myBaseDirectory = baseDirectory;
  }
  else {
    myBaseDirectory = "";
  }

} // end method setBaseDirectory

MVREXPORT const char *MvrMapSimple::getTempDirectory(void) const
{
  if (myIsWriteToTempFile) {
    return myTempDirectory.c_str();
  }
  else {
    return NULL;
  }
}

MVREXPORT void MvrMapSimple::setTempDirectory(const char *tempDirectory)
{
  if (tempDirectory != NULL) {
    myIsWriteToTempFile = true;
    myTempDirectory = tempDirectory;
  }
  else {
    myIsWriteToTempFile = false;
    myTempDirectory = "";
  }
}


MVREXPORT void MvrMapSimple::setSourceFileName(const char *sourceName,
                                             const char *fileName,
                                             bool isInternalCall)
{
  if (!isInternalCall) {
    lock();
  }
  myMapId.setSourceName(sourceName);

  std::string realFileName = ((fileName != NULL) ? fileName : "");

  if (!myBaseDirectory.empty()) {
    if (realFileName.find(myBaseDirectory) == 0) {
      size_t dirLen = myBaseDirectory.length();
      if ((myBaseDirectory[dirLen - 1] == '/') || 
          (myBaseDirectory[dirLen - 1] == '\\')) {
        realFileName = realFileName.substr(dirLen, 
                                           realFileName.length() - dirLen);
      }
      else {
        realFileName = realFileName.substr(dirLen + 1, 
                                           realFileName.length() - (dirLen + 1));
      }

      MvrLog::log(MvrLog::Normal,
                 "MvrMapSimple::setSourceFileName(%s, %s) stripped base dir = %s",
                 sourceName,
                 fileName,
                 realFileName.c_str());
    }

  }

  myMapId.setFileName(realFileName.c_str());
  if (!isInternalCall) {
    unlock();
  }

} // end method setSourceFileName


MVREXPORT bool MvrMapSimple::getMapId(MvrMapId *mapIdOut,
                                    bool isInternalCall)
{
  if (mapIdOut != NULL) {
    if (!isInternalCall) {
      lock();
    }
    *mapIdOut = myMapId;
    if (!isInternalCall) {
      unlock();
    }
    return true;
  }
  else {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::getMapId() null map ID param");
  }
  return false;

} // end method getMapId
  
  
MVREXPORT MvrArgumentBuilder *MvrMapSimple::findMapObjectParams
                                           (const char *mapObjectName)
{
  if (MvrUtil::isStrEmpty(mapObjectName)) {
    return NULL;
  }

  MvrArgumentBuilder *params = NULL;

  std::map<std::string, MvrArgumentBuilder *, MvrStrCaseCmpOp>::iterator iter =
     myMapObjectNameToParamsMap.find(mapObjectName);

  if (iter == myMapObjectNameToParamsMap.end()) {
    
    std::list<MvrArgumentBuilder*> *cairnInfoList = getInfo(CAIRN_INFO_NAME);

    if (cairnInfoList != NULL) {

      MvrArgumentBuilder *paramInfo = NULL;
      std::list<MvrArgumentBuilder *>::iterator infoIter = 
                                          findMapObjectParamInfo(mapObjectName,
                                                                 *cairnInfoList);
      if (infoIter != cairnInfoList->end()) {
        paramInfo = *infoIter;
      }
      if (paramInfo != NULL) {

        params = new MvrArgumentBuilder(*paramInfo);
        params->compressQuoted(true);

        if (params->getArgc() >= 2) {
          params->removeArg(0);       // Remove the "Params" field
          params->removeArg(0, true); // Remove the map object name
        }
        myMapObjectNameToParamsMap[mapObjectName] = params;
      }

    } // end for each cairn info
  } // end if 
  // if we already have one, juset set that
  else
  {
    params = (*iter).second;
  }

  return params;

} // end method findMapObjectParams

/**
  MvrMapObject *obj = myMap->findMapObject(objName);
  const MvrArgumentBuilder *args = myMap->findMapObjectParams(objName);

  bool ok = true;
  int someNum = 0;

  if ((args != NULL) && (args->getArgc() > 0)) {
    
    someNum = args->getArgInt(0, &ok);
  }

  if (ok) {

    MvrArgumentBuilder newArgs;
    char buf[128]
    snprintf(buf, sizeof(buf), "%i", someNum + 1);
    newArgs.add(buf);

    // Note that the newArgs will be copied, and that the original args 
    // pointer (above) will be invalidated by the following call.
    myMap->setMapObjectParams(objName, &newArgs);
    args = NULL;
  }

 
  mapObjectList.remove(obj);
  myMap->setMapObjectParams(obj->getName(), NULL);
  myMap->setMapObjects(mapObjectList);
  delete obj; 
  
**/

MVREXPORT bool MvrMapSimple::setMapObjectParams(const char *mapObjectName,
                                              MvrArgumentBuilder *params,
                                              MvrMapChangeDetails *changeDetails)
{
  if (MvrUtil::isStrEmpty(mapObjectName)) {
    return false;
  }
  
  std::list<MvrArgumentBuilder*> *cairnInfoList = getInfo(CAIRN_INFO_NAME);
  if (cairnInfoList == NULL) {
    return false;
  }
  
  MvrMapFileLineSet origLines;

  if (changeDetails != NULL) {
    myMapInfo->createMultiSet(CAIRN_INFO_NAME, &origLines, changeDetails);
  }

  std::map<std::string, MvrArgumentBuilder *, MvrStrCaseCmpOp>::iterator iter =
     myMapObjectNameToParamsMap.find(mapObjectName);

  if (iter != myMapObjectNameToParamsMap.end()) {
    MvrArgumentBuilder *oldParams = iter->second;
    myMapObjectNameToParamsMap.erase(iter);
    delete oldParams;
  }
  
  if (params != NULL) {
    myMapObjectNameToParamsMap[mapObjectName] = new MvrArgumentBuilder(*params);
  }


  std::list<MvrArgumentBuilder *>::iterator infoIter = 
                                     findMapObjectParamInfo
                                        (mapObjectName,
                                         *cairnInfoList);
  if (infoIter != cairnInfoList->end()) {
    MvrArgumentBuilder *oldInfo = *infoIter;
    cairnInfoList->erase(infoIter);
    delete oldInfo;
  }
 
  if (params != NULL) {
 
    // Any need to make sure that mapObjectName is not started with quotes
    // already?
    std::string quotedMapObjectName = "\"";
    quotedMapObjectName += mapObjectName;
    quotedMapObjectName += "\"";

    MvrArgumentBuilder *newInfo = new MvrArgumentBuilder();
    newInfo->add("Params");
    newInfo->add(quotedMapObjectName.c_str());
    newInfo->add(params->getFullString());

    // This little alphabetization of the CairnInfo list is done just to 
    // make the MvrMapChangeDetails processing better. (i.e. It's in sync
    // with the editor.)
    bool isInserted = false;
    for (infoIter = cairnInfoList->begin(); 
         infoIter != cairnInfoList->end(); 
         infoIter++) {
      MvrArgumentBuilder *curArg = *infoIter;
      
      if ((curArg == NULL) || (curArg->getArgc() < 2)) {
        continue;
      }
      
      if (MvrUtil::strcasequotecmp(mapObjectName, curArg->getArg(1)) <= 0) {
        cairnInfoList->insert(infoIter, newInfo);
        isInserted = true;
        break;
      }
    } // end for each info item

    if (!isInserted) {
      cairnInfoList->push_back(newInfo);
    }
    
  } // end if new params
  
  myMapInfo->setChanged();
  
  if (changeDetails != NULL) {

    MvrMapFileLineSet newLines;
    myMapInfo->createMultiSet(CAIRN_INFO_NAME, &newLines, changeDetails);

    bool isSuccess = MvrMapFileLineSet::calculateChanges
                        (origLines,
                         newLines,
                         changeDetails->getChangedInfoLines
                                          (CAIRN_INFO_NAME,
                                           MvrMapChangeDetails::DELETIONS),
                         changeDetails->getChangedInfoLines
                                          (CAIRN_INFO_NAME,
                                           MvrMapChangeDetails::ADDITIONS));
    if (!isSuccess) {
      MvrLog::log(MvrLog::Normal,
                 "MvrMapInfo::setInfo() error calculating changes");
    }
  } // end if changeDetails

  return true;

} // end method setMapObjectParams


std::list<MvrArgumentBuilder *>::iterator MvrMapSimple::findMapObjectParamInfo
             (const char *mapObjectName,
              std::list<MvrArgumentBuilder*> &cairnInfoList)
{
  // If the map object has parameters, then it must have a name.
  if (MvrUtil::isStrEmpty(mapObjectName)) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::findMapObjectParamInfo() cannot find empty map object name");
    return cairnInfoList.end();
  }

  for (std::list<MvrArgumentBuilder*>::iterator iter = cairnInfoList.begin();
       iter != cairnInfoList.end();
       iter++) {

    MvrArgumentBuilder *arg = *iter;
    // The first arg in the CairnInfo line is the type of info (e.g. Params)
    // and the second arg name of the map object...
    if ((arg == NULL) || 
        (arg->getArgc() < 2) || 
        (MvrUtil::isStrEmpty(arg->getArg(1)))) {
      MvrLog::log(MvrLog::Normal,
                 "AramMapInfoMinder::findObjectParams() skipping: %s",
                 ((arg != NULL) ? arg->getFullString(): "NULL"));
      continue;
    }

    if (MvrUtil::strcasequotecmp(arg->getArg(1), mapObjectName) == 0) {
      return iter;
    }
  } // end for each cairn info list item

  // Not found
  return cairnInfoList.end();

} // end method findMapObjectParamInfo
  


MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapSimple::getRemainder()
{
  return &myRemainderList;
}

MVREXPORT void MvrMapSimple::setQuiet(bool isQuiet)
{ 
  myIsQuiet = isQuiet;

} // end method setQuiet
	

MVREXPORT void MvrMapSimple::mapChanged(void)
{ 
  MvrTime maxScanTimeChanged = findMaxMapScanTimeChanged();
//  MvrLog::log(level, "MvrMap: Calling mapChanged callbacks");
  if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()) ||
      !myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()) ||
      !myTimeMapSupplementChanged.isAt(myMapSupplement->getTimeChanged()) ||
      !myTimeMapScanChanged.isAt(maxScanTimeChanged)) {
    
    MvrLog::log(myMapChangedHelper->getMapChangedLogLevel(),
	       "MvrMapSimple::mapChanged() msecs-objects: %i, msecs-points: %i, msecs-mapInfo: %i msecs-supplement: %i",
	       myTimeMapObjectsChanged.isAt(myMapObjects->getTimeChanged()),
	       !myTimeMapScanChanged.isAt(maxScanTimeChanged),
	       !myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged()),
	       !myTimeMapSupplementChanged.isAt(
		       myMapSupplement->getTimeChanged()));
		

    // Since setInfo is not necessarily called, call updateMapCategory just to 
    // make sure that the category is correctly set based on the contents of the 
    // map file.
    if (!myTimeMapInfoChanged.isAt(myMapInfo->getTimeChanged())) {
      updateMapCategory();
    }
    
    if (!myTimeMapScanChanged.isAt(maxScanTimeChanged)) {
      updateSummaryScan();
    } // end if scan was changed
    
    myMapChangedHelper->invokeMapChangedCallbacks();
    
    MvrLog::log(myMapChangedHelper->getMapChangedLogLevel(),
	       "MvrMapSimple: Done calling mapChanged callbacks");
  }
  else { // nothing changed
    
    MvrLog::log(MvrLog::Verbose,
	       "MvrMapSimple::mapChanged(): Map was not changed");
    
  } // end else nothing changed
  
  myTimeMapObjectsChanged = myMapObjects->getTimeChanged();
  myTimeMapSupplementChanged = myMapSupplement->getTimeChanged();
  myTimeMapInfoChanged    = myMapInfo->getTimeChanged();
  myTimeMapScanChanged    = findMaxMapScanTimeChanged(); 

} // end method mapChanged

      
MVREXPORT void MvrMapSimple::updateSummaryScan()
{ 
  if (mySummaryScan != NULL) {
    
    mySummaryScan->clear();

    for (MvrTypeToScanMap::iterator iter = myTypeToScanMap.begin();
         iter != myTypeToScanMap.end();
         iter++) {
      mySummaryScan->unite(iter->second); 
    }
  } // end if summary
}


MVREXPORT MvrTime MvrMapSimple::findMaxMapScanTimeChanged()
{
  MvrTime maxMapScanTimeChanged;
  bool isFirst = true;

  for (MvrTypeToScanMap::iterator iter = 
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    MvrMapScan *scan = iter->second;
    if (scan != NULL) {
      if (isFirst || 
          (scan->getTimeChanged().isAfter(maxMapScanTimeChanged))) {
        isFirst = false;
        maxMapScanTimeChanged = scan->getTimeChanged();
      }
    }
  } // end for each scan
 
  return maxMapScanTimeChanged;


} // end method findMaxMapScanTimeChanged


MVREXPORT void MvrMapSimple::addMapChangedCB(MvrFunctor *functor, 
					   int position)
{ 
  myMapChangedHelper->addMapChangedCB(functor, position);

} // end method addMapChangedCB


MVREXPORT void MvrMapSimple::remMapChangedCB(MvrFunctor *functor)
{ 
  myMapChangedHelper->remMapChangedCB(functor);

} // end method remMapChangedCB


MVREXPORT void MvrMapSimple::addPreMapChangedCB(MvrFunctor *functor,
                                              int position)
{ 
  myMapChangedHelper->addPreMapChangedCB(functor, position);
} // end method addPreMapChangedCB


MVREXPORT void MvrMapSimple::remPreMapChangedCB(MvrFunctor *functor)
{ 
  myMapChangedHelper->remPreMapChangedCB(functor);

} // end method remPreMapChangedCB


MVREXPORT void MvrMapSimple::setMapChangedLogLevel(MvrLog::LogLevel level)
{ 
  myMapChangedHelper->setMapChangedLogLevel(level);

} // end method setMapChangedLogLevel

MVREXPORT MvrLog::LogLevel MvrMapSimple::getMapChangedLogLevel(void)
{ 
  return myMapChangedHelper->getMapChangedLogLevel();

} // end method getMapChangedLogLevel


MVREXPORT int MvrMapSimple::lock()
{ 
  return myMutex.lock();

} // end method lock

MVREXPORT int MvrMapSimple::tryLock()
{ 
  return myMutex.tryLock();

} // end method tryLock

MVREXPORT int MvrMapSimple::unlock()
{ 
  return myMutex.unlock();

} // end method unlock

// ---------------------------------------------------------------------------
// MvrMapInfoInterface
// ---------------------------------------------------------------------------

MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapSimple::getInfo(const char *infoName)
{ 
  return myMapInfo->getInfo(infoName);

} // end method getInfo

MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapSimple::getInfo(int infoType)
{ 
  return myMapInfo->getInfo(infoType);

} // end method getInfo

MVREXPORT std::list<MvrArgumentBuilder *> *MvrMapSimple::getMapInfo(void)
{ 
  return myMapInfo->getInfo(MvrMapInfo::MAP_INFO_NAME);

} // end method getMapInfo

MVREXPORT int MvrMapSimple::getInfoCount() const 
{
  return myMapInfo->getInfoCount();
}

MVREXPORT std::list<std::string> MvrMapSimple::getInfoNames() const
{
  return myMapInfo->getInfoNames();
}

MVREXPORT bool MvrMapSimple::setInfo(const char *infoName,
						                       const std::list<MvrArgumentBuilder *> *infoList,
                                   MvrMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(infoName, infoList, changeDetails);

  // updateMapCategory(infoName);

  return b;

} // end method setInfo

MVREXPORT bool MvrMapSimple::setInfo(int infoType,
						                       const std::list<MvrArgumentBuilder *> *infoList,
                                   MvrMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(infoType, infoList, changeDetails);
  
  // updateMapCategory(NULL);

  return b;

} // end method setInfo

MVREXPORT bool MvrMapSimple::setMapInfo(const std::list<MvrArgumentBuilder *> *mapInfo,
                                      MvrMapChangeDetails *changeDetails)
{ 
  bool b = myMapInfo->setInfo(MvrMapInfo::MAP_INFO_NAME, mapInfo, changeDetails);

  // updateMapCategory(MvrMapInfo::MAP_INFO_NAME);

  return b;

} // end method setMapInfo


MVREXPORT void MvrMapSimple::writeInfoToFunctor
				(MvrFunctor1<const char *> *functor, 
			        const char *endOfLineChars)
{ 
  return myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

} // end method writeInfoToFunctor


MVREXPORT const char *MvrMapSimple::getInfoName(int infoType)
{ 
  return myMapInfo->getInfoName(infoType);

} // end method getInfoName

// ---------------------------------------------------------------------------
// MvrMapObjectsInterface
// ---------------------------------------------------------------------------

MVREXPORT MvrMapObject *MvrMapSimple::findFirstMapObject(const char *name, 
                                                      const char *type,
                                                      bool isIncludeWithHeading)
{ 
  return myMapObjects->findFirstMapObject(name, type, isIncludeWithHeading);

} // end method findFirstMapObject


MVREXPORT MvrMapObject *MvrMapSimple::findMapObject(const char *name, 
				                                         const char *type,
                                                 bool isIncludeWithHeading)
{ 
  return myMapObjects->findMapObject(name, type, isIncludeWithHeading);

} // end method findMapObject

MVREXPORT std::list<MvrMapObject *> MvrMapSimple::findMapObjectsOfType
                                                (const char *type,
                                                 bool isIncludeWithHeading)
{
  return myMapObjects->findMapObjectsOfType(type, isIncludeWithHeading);
}


MVREXPORT std::list<MvrMapObject *> *MvrMapSimple::getMapObjects(void)
{ 
  return myMapObjects->getMapObjects();

} // end method getMapObjects


MVREXPORT void MvrMapSimple::setMapObjects
                             (const std::list<MvrMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              MvrMapChangeDetails *changeDetails)
{
  
  myMapObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);
 
} // end method setMapObjects


MVREXPORT void MvrMapSimple::writeObjectsToFunctor(MvrFunctor1<const char *> *functor, 
			                                           const char *endOfLineChars,
                                                 bool isOverrideAsSingleScan,
                                                 const char *maxCategory)
{
  std::string category = getMapCategory();
  if (maxCategory != NULL) {
    if (strcasecmp(maxCategory, MAP_CATEGORY_2D_COMPOSITE) == 0) {
      category = MAP_CATEGORY_2D_COMPOSITE;
    }
    else if (strcasecmp(maxCategory, MAP_CATEGORY_2D_EXTENDED) == 0) {
      category = MAP_CATEGORY_2D_EXTENDED;
    } 
    else if (strcasecmp(maxCategory, MAP_CATEGORY_2D_MULTI_SOURCES) == 0) {
      category = MAP_CATEGORY_2D_MULTI_SOURCES;
    }
    else {
      category = MAP_CATEGORY_2D;
      isOverrideAsSingleScan = true;
    }
  }
  else if (isOverrideAsSingleScan) {
    category = MAP_CATEGORY_2D;
  }

  MvrUtil::functorPrintf(functor, "%s%s", 
                        category.c_str(),
                        endOfLineChars);

  if (!isOverrideAsSingleScan) {
    writeScanTypesToFunctor(functor, endOfLineChars);
  }

  if (!isOverrideAsSingleScan) {
                        
    for (std::list<std::string>::iterator iter = myScanTypeList.begin();
         iter != myScanTypeList.end();
         iter++) {
      const char *scanType = (*iter).c_str();
      MvrMapScanInterface *mapScan = getScan(scanType);
      if (mapScan != NULL) {
        mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
      }
    } // end for each scan type
  }
  else { // else send single scan

    MvrMapScanInterface *mapScan = getScan(ARMAP_SUMMARY_SCAN_TYPE);
    if (mapScan != NULL) {
      mapScan->writeScanToFunctor(functor, 
                                  endOfLineChars, 
                                  ARMAP_SUMMARY_SCAN_TYPE);
    }
  } // end else just send single scan 

  myMapSupplement->writeSupplementToFunctor(functor, endOfLineChars);

  myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

} // end method writeObjectsToFunctor


MVREXPORT void MvrMapSimple::writeObjectListToFunctor(MvrFunctor1<const char *> *functor, 
			                                              const char *endOfLineChars)
{ 
  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

} // end method writeObjectListToFunctor




// ---------------------------------------------------------------------------
// MvrMapScanInterface
// ---------------------------------------------------------------------------

MVREXPORT const char *MvrMapSimple::getDisplayString(const char *scanType)
{
  if (isSummaryScanType(scanType)) {
    // TODO Could return a special "Summary" string instead...
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::getDisplayString() summary display is not supported");
    return "";
  }

  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getDisplayString(scanType);
  }
  return "";

} // end method getDisplayString


MVREXPORT std::vector<MvrPose> *MvrMapSimple::getPoints(const char *scanType)
{
  if (isSummaryScanType(scanType)) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::getPoints() summary of points is not supported");
    return NULL;
  }

  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getPoints(scanType);
  }
  return NULL;

} // end method getPoints


MVREXPORT std::vector<MvrLineSegment> *MvrMapSimple::getLines(const char *scanType)
{ 
  if (isSummaryScanType(scanType)) {
    MvrLog::log(MvrLog::Terse,
               "MvrMapSimple::getLines() summary of lines is not supported");
    return NULL;
  }
  
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLines(scanType);
  }
  return NULL;

} // end method getLines


MVREXPORT MvrPose MvrMapSimple::getMinPose(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getMinPose(scanType);
  }
  return MvrPose();

} // end method getMinPose

MVREXPORT MvrPose MvrMapSimple::getMaxPose(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getMaxPose(scanType);
  }
  return MvrPose();

} // end method getMaxPose

MVREXPORT int MvrMapSimple::getNumPoints(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getNumPoints(scanType);
  }
  return 0;

} // end method getNumPoints

MVREXPORT MvrPose MvrMapSimple::getLineMinPose(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLineMinPose(scanType);
  }
  return MvrPose();

} // end method getLineMinPose

MVREXPORT MvrPose MvrMapSimple::getLineMaxPose(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getLineMaxPose(scanType);
  }
  return MvrPose();

} // end method getLineMaxPose

MVREXPORT int MvrMapSimple::getNumLines(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getNumLines(scanType);
  }
  return 0;

} // end method getNumLines

MVREXPORT int MvrMapSimple::getResolution(const char *scanType)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->getResolution(scanType);
  }
  return 0;

} // end method getResolution



MVREXPORT bool MvrMapSimple::isSortedPoints(const char *scanType) const
{
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->isSortedPoints(scanType);
  }
  return false;
}

MVREXPORT bool MvrMapSimple::isSortedLines(const char *scanType) const
{
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    return mapScan->isSortedLines(scanType);
  }
  return false;
}

MVREXPORT void MvrMapSimple::setPoints(const std::vector<MvrPose> *points,
                                     const char *scanType,
                                     bool isSorted,
                                     MvrMapChangeDetails *changeDetails)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setPoints(points, 
                       scanType,
                       isSorted, 
                       changeDetails);
  }

} // end method setPoints

MVREXPORT void MvrMapSimple::setLines(const std::vector<MvrLineSegment> *lines,
                                    const char *scanType,
                                    bool isSorted,
                                    MvrMapChangeDetails *changeDetails)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setLines(lines, 
                      scanType,
                      isSorted, 
                      changeDetails);
  }

} // end method setLines

MVREXPORT void MvrMapSimple::setResolution(int resolution,
                                         const char *scanType,
                                         MvrMapChangeDetails *changeDetails)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->setResolution(resolution, 
                           scanType,
                           changeDetails);
  }
} // end method setResolution




MVREXPORT void MvrMapSimple::writeScanToFunctor(MvrFunctor1<const char *> *functor, 
			                                        const char *endOfLineChars,
                                              const char *scanType)
{
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
  }

} // end method writeScanToFunctor


MVREXPORT void MvrMapSimple::writePointsToFunctor
		(MvrFunctor2<int, std::vector<MvrPose> *> *functor,
     const char *scanType,
     MvrFunctor1<const char *> *keywordFunctor)
{
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writePointsToFunctor(functor, scanType, keywordFunctor);
  }

} // end method writePointsToFunctor

MVREXPORT void MvrMapSimple::writeLinesToFunctor
	(MvrFunctor2<int, std::vector<MvrLineSegment> *> *functor,
   const char *scanType,
   MvrFunctor1<const char *> *keywordFunctor)
{ 
  MvrMapScanInterface *mapScan = getScan(scanType);
  if (mapScan != NULL) {
    mapScan->writeLinesToFunctor(functor, scanType, keywordFunctor);
  }

} // end method writeLinesToFunctor


MVREXPORT bool MvrMapSimple::readDataPoint( char *line)
{
  // TODO Locking?
  if (myLoadingScan != NULL) {
    return myLoadingScan->readDataPoint(line);
  }
  return false;

} // end method readDataPoint

MVREXPORT bool MvrMapSimple::readLineSegment( char *line)
{
  if (myLoadingScan != NULL) {
    return myLoadingScan->readLineSegment(line);
  }
  else {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::readLineSegment() NULL loading scan for '%s'",
               line);
  }
  return false;

} // end method readLineSegment


MVREXPORT void MvrMapSimple::loadDataPoint(double x, double y)
{
  if (myLoadingScan != NULL) {
    myLoadingScan->loadDataPoint(x, y);
  }

} // end method loadDataPoint


MVREXPORT void MvrMapSimple::loadLineSegment(double x1, double y1, double x2, double y2)
{
  if (myLoadingScan != NULL) {
    myLoadingScan->loadLineSegment(x1, y1, x2, y2);
  }
} // end method loadLineSegment


MVREXPORT bool MvrMapSimple::addToFileParser(MvrFileParser *fileParser)
{
  if (myTypeToScanMap.empty()) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::addToFileParser() error: no scans in map");
    return false;
  }
  bool isAdded = true;

  for (MvrTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end(); iter++) {
    MvrMapScan *mapScan = iter->second;
    if (mapScan != NULL) {
      isAdded = mapScan->addToFileParser(fileParser) && isAdded;
    }
  }
  return isAdded; 
}

MVREXPORT bool MvrMapSimple::remFromFileParser(MvrFileParser *fileParser)
{
  if (myTypeToScanMap.empty()) {
    return false;
  }
  bool isRemoved = true;

  for (MvrTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end(); iter++) {
    MvrMapScan *mapScan = iter->second;
    if (mapScan != NULL) {
      isRemoved = mapScan->remFromFileParser(fileParser) && isRemoved;
    }
  }
  return isRemoved; 
}


MVREXPORT void MvrMapSimple::writeScanTypesToFunctor
                                (MvrFunctor1<const char *> *functor, 
			                           const char *endOfLineChars)
{
  bool hasSourceList = false;
  if (!myScanTypeList.empty()) {
    hasSourceList = !MvrUtil::isStrEmpty(myScanTypeList.front().c_str());
  }
  if (hasSourceList) {
  
    std::string sourceString = "Sources:";

    //MvrUtil::functorPrintf(functor, "Sources:%s", "");
    
    for (std::list<std::string>::iterator iter = myScanTypeList.begin(); 
         iter != myScanTypeList.end(); 
         iter++) {

      const char *scanType = (*iter).c_str();
      sourceString += " ";
      sourceString += scanType;

     // MvrUtil::functorPrintf(functor, " %s%s", scanType, "");
      
    } // end for each scan type
    
    MvrUtil::functorPrintf(functor, "%s%s", sourceString.c_str(), endOfLineChars);

  } // end if source list

} // end method writeScanTypesToFunctor


MVREXPORT bool MvrMapSimple::hasOriginLatLongAlt()
{ 
  return myMapSupplement->hasOriginLatLongAlt();

} // end method hasOriginLatLongAlt

MVREXPORT MvrPose MvrMapSimple::getOriginLatLong()
{ 
  return myMapSupplement->getOriginLatLong();

} // end method getOriginLatLong

MVREXPORT double MvrMapSimple::getOriginAltitude()
{ 
  return myMapSupplement->getOriginAltitude();

} // end method getOriginAltitude

MVREXPORT void MvrMapSimple::setOriginLatLongAlt
                                  (bool hasOriginLatLong,
                                   const MvrPose &originLatLong,
                                   double originAltitude,
                                   MvrMapChangeDetails *changeDetails)
{
  myMapSupplement->setOriginLatLongAlt(hasOriginLatLong, 
                                       originLatLong, 
                                       originAltitude, 
                                       changeDetails);
} // end method setOriginLatLongAlt

MVREXPORT void MvrMapSimple::writeSupplementToFunctor(MvrFunctor1<const char *> *functor, 
			                                              const char *endOfLineChars)
{
  myMapSupplement->writeSupplementToFunctor(functor, 
                                            endOfLineChars);


} // end method writeSupplementToFunctor



// ---------------------------------------------------------------------------

MVREXPORT void MvrMapSimple::writeToFunctor(MvrFunctor1<const char *> *functor, 
			                                    const char *endOfLineChars)
{ 
  // Write the header information and Cairn objects...
  MvrUtil::functorPrintf(functor, "%s%s", 
                        getMapCategory(),
                        endOfLineChars);

  std::list<std::string>::iterator iter = myScanTypeList.end();
  
  writeScanTypesToFunctor(functor, endOfLineChars);

  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    MvrMapScan *mapScan = getScan(scanType);

    if (mapScan != NULL) {
      mapScan->writeScanToFunctor(functor, endOfLineChars, scanType);
    }
  }

  myMapSupplement->writeSupplementToFunctor(functor, endOfLineChars);

  myMapInfo->writeInfoToFunctor(functor, endOfLineChars);

  myMapObjects->writeObjectListToFunctor(functor, endOfLineChars);

  myInactiveInfo->writeInfoToFunctor(functor, endOfLineChars);

  myInactiveObjects->writeObjectListToFunctor(functor, endOfLineChars);

  myChildObjects->writeObjectListToFunctor(functor, endOfLineChars);

  // Write out any unrecognized (remainder) lines -- just to try to prevent them
  // from being accidentally lost

  for (std::list<MvrArgumentBuilder*>::const_iterator remIter = myRemainderList.begin();
       remIter != myRemainderList.end();
       remIter++) {
    MvrArgumentBuilder *remArg = *remIter;
    if (remArg == NULL) {
      continue;
    }
    MvrUtil::functorPrintf(functor, "%s%s", 
                          remArg->getFullString(),
                          endOfLineChars);

  } // end for each remainder line


  // Write the lines...
  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    MvrMapScan *mapScan = getScan(scanType);
    
    if (mapScan != NULL) {
      mapScan->writeLinesToFunctor(functor, endOfLineChars, scanType);
    }
  }

  // Write the points...
  for (iter = myScanTypeList.begin(); iter != myScanTypeList.end(); iter++) {

    const char *scanType = (*iter).c_str();
    MvrMapScan *mapScan = getScan(scanType);
    
    if (mapScan != NULL) {
      mapScan->writePointsToFunctor(functor, endOfLineChars, scanType);
    }
  } 

} // end method writeToFunctor


MVREXPORT MvrMapInfoInterface *MvrMapSimple::getInactiveInfo()
{
  return myInactiveInfo;
}

MVREXPORT MvrMapObjectsInterface *MvrMapSimple::getInactiveObjects()
{
  return myInactiveObjects;
}

MVREXPORT MvrMapObjectsInterface *MvrMapSimple::getChildObjects()
{
  return myChildObjects;
}

MVREXPORT bool MvrMapSimple::parseLine(char *line)
{ 
  return myLoadingParser->parseLine(line);

} // end method parseLine

MVREXPORT void MvrMapSimple::parsingComplete(void)
{ 
  lock();
  mapChanged();
  unlock();

} // end method parsingComplete


MVREXPORT bool MvrMapSimple::isLoadingDataStarted()
{ 
  return myLoadingDataStarted;

} // end method  isLoadingDataStarted


MVREXPORT bool MvrMapSimple::isLoadingLinesAndDataStarted()
{ 
  return myLoadingLinesAndDataStarted;

} // end method isLoadingLinesAndDataStarted
            

std::string MvrMapSimple::createRealFileName(const char *fileName)
{ 
  return MvrMapInterface::createRealFileName(myBaseDirectory.c_str(),
                                            fileName,
                                            myIgnoreCase);

} // end method createRealFileName

bool MvrMapSimple::handleMapCategory(MvrArgumentBuilder *arg)
{ 
  MvrLog::log(MvrLog::Verbose, 
             "MvrMapSimple::handleMapCategory() read category %s",
             arg->getExtraString());


  if (!addScansToParser() || 
      !myLoadingParser->addHandler("Sources:", &mySourcesCB) ||
      !myMapInfo->addToFileParser(myLoadingParser) ||
      !myMapSupplement->addToFileParser(myLoadingParser) ||
      !myMapObjects->addToFileParser(myLoadingParser) ||
      !myInactiveInfo->addToFileParser(myLoadingParser) ||
      !myInactiveObjects->addToFileParser(myLoadingParser) ||
      !myChildObjects->addToFileParser(myLoadingParser) ||
      // Add a handler for unrecognized lines...
      !myLoadingParser->addHandler(NULL, &myRemCB)) 
  {
    MvrLog::log(MvrLog::Terse, 
               "MvrMapSimple::handleMapCategory: could not add handlers");
    return false;
  }  
  
  // If all of the parsers were successfully added, then remove the map 
  // category handlers and return
 
  myMapCategory = "";
  for (std::list<std::string>::iterator iter = myMapCategoryList.begin();
       iter != myMapCategoryList.end();
       iter++) {
    if (strncasecmp(arg->getExtraString(), 
                    (*iter).c_str(),
                    (*iter).length()) == 0) {
      myMapCategory = *iter;
    }
  
    myLoadingParser->remHandler((*iter).c_str());

  } // end for each category

  if (myMapCategory.empty()) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::handleMapCategory() error finding category for %s",
               arg->getExtraString());
    arg->getExtraString();
  }
  
  myLoadingGotMapCategory = true;

  return true;

} // end method handleMapCategory
  
  
bool MvrMapSimple::handleSources(MvrArgumentBuilder *arg)
{
  
  std::list<std::string> scanTypeList;

  for (size_t i = 0; i < arg->getArgc(); i++) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::handleSources() source #%i = %s",
               i, arg->getArg(i));
    scanTypeList.push_back(arg->getArg(i));
  }

  if (scanTypeList.empty()) {
    MvrLog::log(MvrLog::Terse,
              "MvrMapSimple::handleSources() at least one source must be specified");
    return false;
  }

  remScansFromParser(true);

  createScans(scanTypeList);

  addScansToParser();

  return true;

} // end method handleSources


bool MvrMapSimple::createScans(const std::list<std::string> &scanTypeList)
{
  if (scanTypeList.empty()) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::createScans() scan type list must be non-empty");

    return false;
  }

  bool isListValid = true;

  // Perform some simple validations on the scan type list...
  if (scanTypeList.size() > 1) {

    std::map<std::string, bool, MvrStrCaseCmpOp> typeToExistsMap;

    // Make sure that none of the scan types are empty
    for (std::list<std::string>::const_iterator iter1 = scanTypeList.begin();
        iter1 != scanTypeList.end();
        iter1++) {
      const char *scanType = (*iter1).c_str();
      if (MvrUtil::isStrEmpty(scanType)) {
        isListValid = false;
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::createScans() empty scan name is valid only when there is one scan type");
        break;
      }
    
      // Make sure that there are no duplicates...
      std::map<std::string, bool, MvrStrCaseCmpOp>::iterator tIter = 
                                                typeToExistsMap.find(scanType);
      if (tIter != typeToExistsMap.end()) {
        isListValid = false;
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::createScans() duplicate scan names are not allowed (%s)",
                   scanType);
        break;
      }

      typeToExistsMap[scanType] = true;

    } // end for each scan type
  } // end if more than one entry

  if (!isListValid) {

    MvrLog::log(MvrLog::Terse,
                "MvrMapSimple error setting up map for multiple scan types");
    return false;

  }

  
  delete mySummaryScan;
  mySummaryScan = NULL;

  myScanTypeList.clear();
  MvrUtil::deleteSetPairs(myTypeToScanMap.begin(), myTypeToScanMap.end());
  myTypeToScanMap.clear();

  MvrMapScan *mapScan = NULL;

  for (std::list<std::string>::const_iterator iter = scanTypeList.begin();
       iter != scanTypeList.end();
       iter++) {
    std::string scanType = *iter;

    mapScan = new MvrMapScan(scanType.c_str());
  
    myScanTypeList.push_back(scanType);
    myTypeToScanMap[scanType] = mapScan;

  } // end for each scan type

  if (myScanTypeList.size() > 1) {
    mySummaryScan = new MvrMapScan(ARMAP_SUMMARY_SCAN_TYPE);
  }
  return true;

} // end method createScans


bool MvrMapSimple::addScansToParser()
{
  if (myLoadingParser == NULL) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::addScansToParser() error, loading parser is null");
    return false;
  }
  if (myTypeToScanMap.empty()) {
    MvrLog::log(MvrLog::Normal,
               "MvrMapSimple::addScansToParser() error, no maps scans");
    return false;
  }

  bool isLoaded = true;
  
  for (MvrTypeToScanMap::iterator iter = myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {

    MvrMapScan *mapScan = iter->second;
    if (mapScan == NULL) {
      continue;
    }
    if (!mapScan->addToFileParser(myLoadingParser)) {
      MvrLog::log(MvrLog::Normal,
                 "MvrMapSimple::addScansToParser() error, could not add scan for %s",
                 iter->first.c_str());
      isLoaded = false;
      continue;
    }

    MvrLog::log(MvrLog::Verbose,
               "MvrMapSimple::addScansToParser() adding for type %s points keyword %s lines keyword %s",
               mapScan->getScanType(),
               mapScan->getPointsKeyword(),
               mapScan->getLinesKeyword());
    
    if (!MvrUtil::isStrEmpty(mapScan->getPointsKeyword())) {
      
      myDataTagToScanTypeMap[mapScan->getPointsKeyword()] = iter->first;
      if (!myLoadingParser->addHandler(mapScan->getPointsKeyword(), 
                                       &myDataIntroCB)) {
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::addScansToParser() error, could not handler for %s",
                   mapScan->getPointsKeyword());
        isLoaded = false;
      }
    }
    if (!MvrUtil::isStrEmpty(mapScan->getLinesKeyword())) {
      myDataTagToScanTypeMap[mapScan->getLinesKeyword()] = iter->first;
      if (!myLoadingParser->addHandler(mapScan->getLinesKeyword(), 
                                       &myDataIntroCB)) {
        MvrLog::log(MvrLog::Normal,
                   "MvrMapSimple::addScansToParser() error, could not handler for %s",
                   mapScan->getLinesKeyword());
        isLoaded = false;
      }
    }
  } // end for each scan

  return isLoaded;

} // end method addScansToParser


bool MvrMapSimple::remScansFromParser(bool isRemovePointsAndLinesKeywords)
{
  if (myLoadingParser == NULL) {
    return false;
  }
  if (myTypeToScanMap.empty()) {
    return false;
  }
  bool isRemoved = true;

  for (MvrTypeToScanMap::iterator iter =
          myTypeToScanMap.begin();
       iter != myTypeToScanMap.end();
       iter++) {
    MvrMapScan *mapScan = iter->second;
    if (mapScan == NULL) {
      continue;
    }
    if (!mapScan->remFromFileParser(myLoadingParser)) {
      isRemoved = false;
      continue;
    }
   
    if (isRemovePointsAndLinesKeywords) {
      if (!MvrUtil::isStrEmpty(mapScan->getPointsKeyword())) {
        if (!myLoadingParser->remHandler(mapScan->getPointsKeyword(), 
                                        &myDataIntroCB)) {
          isRemoved = false;
        }
      }
      if (!MvrUtil::isStrEmpty(mapScan->getLinesKeyword())) {
        if (!myLoadingParser->remHandler(mapScan->getLinesKeyword(), 
                                        &myDataIntroCB)) {
          isRemoved = false;
        }
      }
    } // end if remove points and lines keywords
  } // end for each scan

  return isRemoved;

} // end method remScansFromParser


bool MvrMapSimple::handleDataIntro(MvrArgumentBuilder *arg)
{
  remScansFromParser(false);

  myMapSupplement->remFromFileParser(myLoadingParser);
  myMapInfo->remFromFileParser(myLoadingParser);
  myMapObjects->remFromFileParser(myLoadingParser);

  myInactiveInfo->remFromFileParser(myLoadingParser);
  myInactiveObjects->remFromFileParser(myLoadingParser);
  myChildObjects->remFromFileParser(myLoadingParser);

  myLoadingParser->remHandler("Sources:");

  // Remove the remainder handler
  myLoadingParser->remHandler((const char *)NULL);

  // All of the info types have been read by now... If there is
  // an extended one, then update the map's category.
  updateMapCategory();
  
  MvrLog::log(MvrLog::Verbose,
             "MvrMapSimple::handleDataIntro %s",
             arg->getExtraString());

  // The "extra string" contains the keyword - in all lowercase 
  if (arg->getExtraString() != NULL) {
    myLoadingDataTag = arg->getExtraString();
  }
  else {
    myLoadingDataTag = "";
  }
  
  // Need to set the myLoadingScan so that calls from ArQ to loadDataPoint
  // and loadDataLine are processed correctly.  In addition, the findScan...
  // method sets the myLoadingData / myLoadingLinesAndData attributes --
  // which are also needed by ArQ
  bool isLineDataTag = false;

  if (myLoadingScan != NULL) {
    myLoadingScan->remExtraFromFileParser(myLoadingParser);
  }

  myLoadingScan = findScanWithDataKeyword(myLoadingDataTag.c_str(),
                                          &isLineDataTag);

  if (myLoadingScan != NULL) {
    myLoadingScan->addExtraToFileParser(myLoadingParser, isLineDataTag);
  }

  return false;

} // end method handleDataIntro


bool MvrMapSimple::handleRemainder(MvrArgumentBuilder *arg)
{
  if (arg != NULL) {
    myRemainderList.push_back(new MvrArgumentBuilder(*arg));
  }
  return true;

} // end method handleRemainder


bool MvrMapSimple::setInactiveInfo(const char *infoName,
						                               const std::list<MvrArgumentBuilder *> *infoList,
                                           MvrMapChangeDetails *changeDetails)
{ 
  return myInactiveInfo->setInfo(infoName, infoList, changeDetails);

} // end method setInactiveInfo

void MvrMapSimple::setInactiveObjects
                             (const std::list<MvrMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              MvrMapChangeDetails *changeDetails)
{ 
  myInactiveObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);

} // end method setInactiveObjects
 

void MvrMapSimple::setChildObjects
                             (const std::list<MvrMapObject *> *mapObjects,
                              bool isSortedObjects, 
                              MvrMapChangeDetails *changeDetails)
{ 
  myChildObjects->setMapObjects(mapObjects, isSortedObjects, changeDetails);

} // end method setChildObjects



MVREXPORT MvrMapScan *MvrMapSimple::getScan(const char *scanType) const
{
  // The summary scan type is a special designation that allows the user
  // to return the total number of points in the map, the bounding box of
  // all the scans, etc.  If there are multiple scan types in the map, then
  // the mySummaryScan member is created.  Otherwise, the summary is the
  // same as the single scan.
  if (isSummaryScanType(scanType)) {
    if (mySummaryScan != NULL) {
      return mySummaryScan;
    }
    else {
      scanType = ARMAP_DEFAULT_SCAN_TYPE;
    }
  } // end if summary scan type

  MvrTypeToScanMap::const_iterator iter = myTypeToScanMap.find(scanType);
  
  // If the specified scan type was not found, then see if the special 
  // ARMAP_DEFAULT_SCAN_TYPE was specified.  If so, this is equivalent to the
  // first scan in the scan list.
  if ((iter == myTypeToScanMap.end()) &&
      (isDefaultScanType(scanType)) &&
      (!myScanTypeList.empty())) {
    scanType = myScanTypeList.front().c_str();
    iter = myTypeToScanMap.find(scanType);
  }

  if (iter != myTypeToScanMap.end()) {
    return iter->second;
  }
 
  return NULL;

} // end method getScan




