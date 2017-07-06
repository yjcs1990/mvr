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

#include "MvrMapUtils.h"

#include "mvriaUtil.h"
#include "MvrBasePacket.h"
#include "MvrMapComponents.h"
#include "MvrMD5Calculator.h"

#include <iterator>

//#define ARDEBUG_MAPUTILS
#ifdef ARDEBUG_MAPUTILS
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

// -----------------------------------------------------------------------------
// MvrMapId
// -----------------------------------------------------------------------------

MVREXPORT MvrMapId::MvrMapId() :
  mySourceName(),
  myFileName(),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(0),
  myTimestamp(-1)
{
}

MVREXPORT MvrMapId::MvrMapId(const char *sourceName,
						              const char *fileName,
						              const unsigned char *checksum,
                          size_t checksumLength,
						              long int size,
						              const time_t timestamp) :
  mySourceName((sourceName != NULL) ? sourceName : ""),
  myFileName((fileName != NULL) ? fileName : ""),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(size),
  myTimestamp(timestamp)
{
  if (checksumLength > 0) {
    setChecksum(checksum,
                checksumLength);
  }
}

MVREXPORT MvrMapId::MvrMapId(const MvrMapId &other) :
  mySourceName(other.mySourceName),
  myFileName(other.myFileName),
  myChecksum(NULL),
  myChecksumLength(0),
  myDisplayChecksum(NULL),
  myDisplayChecksumLength(0),
  mySize(other.mySize),
  myTimestamp(other.myTimestamp)
{
  if (other.myChecksumLength > 0) {
    setChecksum(other.myChecksum,
                other.myChecksumLength);
  }
}

MVREXPORT MvrMapId &MvrMapId::operator=(const MvrMapId &other)
{
  if (&other != this) {

    mySourceName = other.mySourceName;
    myFileName = other.myFileName;

    delete [] myChecksum;
    myChecksum = NULL;
    myChecksumLength = 0;

    delete [] myDisplayChecksum;
    myDisplayChecksum = NULL;
    myDisplayChecksumLength = 0;

    if (other.myChecksumLength > 0) {
      setChecksum(other.myChecksum,
                  other.myChecksumLength);
    }


    mySize = other.mySize;
    myTimestamp = other.myTimestamp;
  }
  return *this;
}

MVREXPORT MvrMapId::~MvrMapId()
{
  delete [] myChecksum;
  myChecksum = NULL;
  myChecksumLength = 0;

  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;
}

MVREXPORT bool MvrMapId::isNull() const
{
  // TODO Any need to check others?
  bool b = (MvrUtil::isStrEmpty(mySourceName.c_str()) &&
            MvrUtil::isStrEmpty(myFileName.c_str()));

  return b; 
}
  
MVREXPORT void MvrMapId::clear()
{
  mySourceName = "";
  myFileName = "";

  delete [] myChecksum;
  myChecksum = NULL;
  myChecksumLength = 0;

  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;

  mySize = 0;
  myTimestamp = -1;

} // end method clear


MVREXPORT const char *MvrMapId::getSourceName() const
{
  return mySourceName.c_str();
}

MVREXPORT const char *MvrMapId::getFileName() const
{
  return myFileName.c_str();
}

MVREXPORT const unsigned char *MvrMapId::getChecksum() const
{
  return myChecksum;
}
  
MVREXPORT size_t MvrMapId::getChecksumLength() const
{
  return myChecksumLength;
} 
  
MVREXPORT const char *MvrMapId::getDisplayChecksum() const
{
  if ((myDisplayChecksum == NULL) && (myChecksumLength > 0)) {

    myDisplayChecksumLength = MvrMD5Calculator::DISPLAY_LENGTH;
    myDisplayChecksum = new char[myDisplayChecksumLength];

    MvrMD5Calculator::toDisplay(myChecksum,
                               myChecksumLength,
                               myDisplayChecksum,
                               myDisplayChecksumLength);

  }

  return myDisplayChecksum;
} 

MVREXPORT long int MvrMapId::getSize() const
{
  return mySize;
}

MVREXPORT time_t MvrMapId::getTimestamp() const
{
  return myTimestamp;
}
    
MVREXPORT bool MvrMapId::isSameFile(const MvrMapId &other) const
{
  if (MvrUtil::strcasecmp(myFileName, other.myFileName) != 0) {
    return false;
  }
  if (mySize != other.mySize) {
    return false;
  }

  // If both timestamps are specified, then they must be identical...
  // KMC 8/29/13 Is this true?  Running CS and sim, had a scenario
  // where everything was the same except the timestamps. Presumably
  // sim wrote file even though nothing had changed. I think that 
  // the file name, size and checksum comparison is probably sufficient 
  // for this purpose.
  /***
  if ((myTimestamp != other.myTimestamp) &&
      (isValidTimestamp()) &&
      (other.isValidTimestamp())) {
    return false;
  }
  ***/
  if (myChecksumLength != other.myChecksumLength) {
    return false;
  }
  if ((myChecksum != NULL) && (other.myChecksum != NULL)) {
    return (memcmp(myChecksum, other.myChecksum, myChecksumLength) == 0);
  }
  return true; // ??
}


MVREXPORT bool MvrMapId::isVersionOfSameFile(const MvrMapId &other) const
{
  if ((MvrUtil::strcasecmp(mySourceName, other.mySourceName) == 0) && 
      (MvrUtil::strcasecmp(myFileName, other.myFileName) == 0)) {
    return true;
  }
  return false;

} // end method isVersionOfSameFile
  
MVREXPORT bool MvrMapId::isValidTimestamp() const
{
  bool b = ((myTimestamp != -1) &&
            (myTimestamp != 0));
  return b;
}


MVREXPORT void MvrMapId::setSourceName(const char *sourceName)
{
  if (sourceName != NULL) {
    mySourceName = sourceName;
  }
  else {
    mySourceName = "";
  }
}

MVREXPORT void MvrMapId::setFileName(const char *fileName)
{
  if (fileName != NULL) {
    myFileName = fileName;
  }
  else {
    myFileName = "";
  }
}

MVREXPORT void MvrMapId::setChecksum(const unsigned char *checksum,
                                   size_t checksumLen)
{
  if (checksumLen < 0) {
    checksumLen = 0;
  }
  if (checksumLen != myChecksumLength) {
    delete [] myChecksum;
    myChecksum = NULL;
    myChecksumLength = 0;
  }
  if (checksumLen > 0) {
    myChecksumLength = checksumLen;
    myChecksum = new unsigned char[myChecksumLength];
    memcpy(myChecksum, checksum, myChecksumLength);
  }
  // Clear this so that it is calculated if necessary....
  delete [] myDisplayChecksum;
  myDisplayChecksum = NULL;
  myDisplayChecksumLength = 0;



}

MVREXPORT void MvrMapId::setSize(long int size)
{
  mySize = size;
}

MVREXPORT void MvrMapId::setTimestamp(const time_t &timestamp)
{
  IFDEBUG(MvrLog::log(MvrLog::Normal,
                     "MvrMapId::setTimestamp() time = %i", 
                     timestamp));

  myTimestamp = timestamp;
}

/// Determines whether two IDs are equal.
MVREXPORT bool operator==(const MvrMapId & id1, const MvrMapId & id2)
{  
  // The mirror opposite of operator!=

  // Filename is compared last just because it takes longer
  if (id1.mySize != id2.mySize) {
    return false;
  }
  // A null timestamp  (-1) can be "equal" to any other timestamp
  if ((id1.isValidTimestamp()) &&
      (id2.isValidTimestamp()) &&
      (id1.myTimestamp != id2.myTimestamp)) {
    return false;
  }
  if (id1.myChecksumLength != id2.myChecksumLength) {
    return false;
  }

  // TODO: Compare only if sources are not null (like timestamps)?
  if (MvrUtil::strcasecmp(id1.mySourceName, id2.mySourceName) != 0) {
    return false;
  }
  if (MvrUtil::strcasecmp(id1.myFileName, id2.myFileName) != 0) {
    return false;
  }
  if ((id1.myChecksum != NULL) && (id2.myChecksum != NULL)) {
    if (memcmp(id1.myChecksum, id2.myChecksum, id1.myChecksumLength) != 0) {
      return false;
    }
  }
  else if (id1.myChecksum != id2.myChecksum) {
    // The above says that if one of them is null, then both of them
    // must be null.
    return false;
  }

  return true;

} // end method operator==

/// Determines whether two IDs are not equal.
MVREXPORT bool operator!=(const MvrMapId & id1, const MvrMapId & id2)
{
  // The mirror opposite of operator==
  
  // Filename is compared last just because it takes longer
  if (id1.mySize != id2.mySize) {
    return true;
  }
  // A null timestamp  (-1) can be "equal" to any other timestamp
  if ((id1.isValidTimestamp()) &&
      (id2.isValidTimestamp()) &&
      (id1.myTimestamp != id2.myTimestamp)) {
    return true;
  }
  if (id1.myChecksumLength != id2.myChecksumLength) {
    return true;
  }

  // TODO: Compare only if sources are not null (like timestamps)?
  if (MvrUtil::strcasecmp(id1.mySourceName, id2.mySourceName) != 0) {
    return true;
  }
  if (MvrUtil::strcasecmp(id1.myFileName, id2.myFileName) != 0) {
    return true;
  }
  if ((id1.myChecksum != NULL) && (id2.myChecksum != NULL)) {
    if (memcmp(id1.myChecksum, id2.myChecksum, id1.myChecksumLength) != 0) {
      return true;
    }
  }
  else if (id1.myChecksum != id2.myChecksum) {
    // The above says that if one of them is null, then both of them
    // must be null.
    return true;
  }

  return false;

} // end method operator!=
  

MVREXPORT void MvrMapId::log(const char *prefix) const
{
  time_t idTime = getTimestamp();

  char timeBuf[500];

  struct tm *idTm = NULL;
  
  if (idTime != -1) {
    idTm = localtime(&idTime);
  }
  if (idTm != NULL) {
    strftime(timeBuf, sizeof(timeBuf), "%c", idTm);
  }
  else {
    snprintf(timeBuf, sizeof(timeBuf), "NULL");
  }

  MvrLog::log(MvrLog::Normal,
             "%s%smap %s %s%s checksum = \"%s\" size = %i  time = %s (%i)",
             ((prefix != NULL) ? prefix : ""),
             ((prefix != NULL) ? " " : ""),
             getFileName(),
             (!MvrUtil::isStrEmpty(getSourceName()) ? "source " : ""),
             (!MvrUtil::isStrEmpty(getSourceName()) ? getSourceName() : ""),
             getDisplayChecksum(),
             getSize(),
             timeBuf,
             idTime);
  
}
  
MVREXPORT bool MvrMapId::fromPacket(MvrBasePacket *packetIn,
                                  MvrMapId *mapIdOut)
{
  if ((packetIn == NULL) || (mapIdOut == NULL)) {
    return false;
  }

  char sourceBuffer[512];
	packetIn->bufToStr(sourceBuffer, sizeof(sourceBuffer));
	
  char fileNameBuffer[512];
	packetIn->bufToStr(fileNameBuffer, sizeof(fileNameBuffer));

  MvrUtil::fixSlashes(fileNameBuffer, sizeof(fileNameBuffer));

  size_t checksumLength = packetIn->bufToUByte4();
 
  unsigned char *checksum = NULL;
  if (checksumLength > 0) {
    checksum = new unsigned char[checksumLength];
    packetIn->bufToData(checksum, checksumLength);
  }

  size_t fileSize = packetIn->bufToUByte4();
  time_t fileTime = packetIn->bufToByte4();

  IFDEBUG(MvrLog::log(MvrLog::Normal,
                     "MvrMapId::fromPacket() time = %i", 
                     fileTime));

  *mapIdOut = MvrMapId(sourceBuffer,
                      fileNameBuffer,
                      checksum,
                      checksumLength,
                      fileSize,
                      fileTime);

  IFDEBUG(mapIdOut->log("MvrMapId::fromPacket()"));

  delete [] checksum;

  return true;

} // end method fromPacket


MVREXPORT bool MvrMapId::toPacket(const MvrMapId &mapId,
                                MvrBasePacket *packetOut)
{
  
  IFDEBUG(mapId.log("MvrMapId::toPacket()"));
 
  if (packetOut == NULL) {
    return false;
  }

  if (!MvrUtil::isStrEmpty(mapId.getSourceName())) {
    packetOut->strToBuf(mapId.getSourceName());  
  }
  else {
    packetOut->strToBuf("");
  }

  if (!MvrUtil::isStrEmpty(mapId.getFileName())) {
    packetOut->strToBuf(mapId.getFileName());
  }
  else {
    packetOut->strToBuf("");
  }

  packetOut->uByte4ToBuf(mapId.getChecksumLength());
  if (mapId.getChecksumLength() > 0) {
    packetOut->dataToBuf(mapId.getChecksum(), mapId.getChecksumLength()); 
  }
  packetOut->uByte4ToBuf(mapId.getSize());
  packetOut->byte4ToBuf(mapId.getTimestamp());
  
  IFDEBUG(MvrLog::log(MvrLog::Normal,
                     "MvrMapId::toPacket() time = %i", 
                     mapId.getTimestamp()));
  
  return true;

} // end method toPacket
  
  
  
MVREXPORT bool MvrMapId::create(const char *mapFileName,
                              MvrMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    MvrLog::log(MvrLog::Normal,
               "Cannot create null map ID");
    return false;
  }
  if (MvrUtil::isStrEmpty(mapFileName)) {
    MvrLog::log(MvrLog::Verbose,
               "Returning null map ID for null file name");
    mapIdOut->clear();
    return true;
  }

  struct stat mapFileStat;
  if (stat(mapFileName, &mapFileStat) != 0)
  {
    MvrLog::log(MvrLog::Normal, 
               "Map file %s not not found", mapFileName);
    mapIdOut->clear();
    return false;
  }

  unsigned char buffer[MvrMD5Calculator::DIGEST_LENGTH];
  bool isSuccess = MvrMD5Calculator::calculateChecksum(mapFileName,
                                                      buffer,
                                                      sizeof(buffer));

  if (!isSuccess) {
    MvrLog::log(MvrLog::Normal,
               "Error calculating checksum for map file %s",
               mapFileName);
    mapIdOut->clear();
    return false;
  }

  mapIdOut->setFileName(mapFileName);
	mapIdOut->setChecksum(buffer, sizeof(buffer));
	mapIdOut->setSize(mapFileStat.st_size);
	mapIdOut->setTimestamp(mapFileStat.st_mtime);

  return true;

} // end method create


// -----------------------------------------------------------------------------
// MvrMapFileLineSet
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// MvrMapFileLineGroup
// -----------------------------------------------------------------------------


void MvrMapFileLineGroup::log() 
{
  MvrLog::log(MvrLog::Normal,
             "#%-3i : %s",
             myParentLine.getLineNum(),
             myParentLine.getLineText());
  for (std::vector<MvrMapFileLine>::iterator iter = myChildLines.begin();
       iter != myChildLines.end();
       iter++) {
    MvrMapFileLine &fileLine = *iter;
    MvrLog::log(MvrLog::Normal,
                "     #%-3i : %s",
                fileLine.getLineNum(),
                fileLine.getLineText());
  }
} // end method log



MVREXPORT void MvrMapFileLineSet::log(const char *prefix)
{
  if (prefix != NULL) {
    MvrLog::log(MvrLog::Normal,
               prefix);
  }
 
  int i = 0;
  MvrMapFileLineSet::iterator mIter = begin();

  for (;((mIter != end()) && (i < 100));
       mIter++, i++) {
    MvrMapFileLineGroup &group = *mIter;
    group.log();
  }
 
  if (mIter != end()) {
    MvrLog::log(MvrLog::Normal,
               "..... (cont.)");
    MvrLog::log(MvrLog::Normal,
               "Size = %i", size());

  }
} // end method log


MVREXPORT MvrMapFileLineSet::iterator MvrMapFileLineSet::find(const MvrMapFileLine &groupParent) {
  for (iterator iter = begin(); iter != end(); iter++) {
    MvrMapFileLineGroup &group = *iter;
    if ((group.getParentLine()->getLineNum() == groupParent.getLineNum()) &&
        true) {

        if (strcmp(group.getParentLine()->getLineText(),
                   groupParent.getLineText()) != 0) {
          MvrLog::log(MvrLog::Normal,
                     "Line #i text does not match:",
                     group.getParentLine()->getLineNum());
          MvrLog::log(MvrLog::Normal,
                      "\"%s\"",
                      group.getParentLine()->getLineText());
          MvrLog::log(MvrLog::Normal,
                      "\"%s\"",
                      groupParent.getLineText());

        }
        //(strcmp(group.getParentLine()->getLineText(),
        //        groupParent.getLineText()) == 0)) {
                  MvrLog::log(MvrLog::Normal,
                              "Found #%i : %s",
                              groupParent.getLineNum(),
                              groupParent.getLineText());
      return iter;
    }
  }
  return end();
}

MVREXPORT bool MvrMapFileLineSet::calculateChanges(MvrMapFileLineSet &origLines,
                                     MvrMapFileLineSet &newLines,
                                     MvrMapFileLineSet *deletedLinesOut,
                                     MvrMapFileLineSet *addedLinesOut,
                                     bool isCheckChildren)
{
  if ((deletedLinesOut == NULL) || (addedLinesOut == NULL)) {
    return false;
  }
  MvrMapFileLineGroupCompare compare;
  MvrMapFileLineGroupLineNumCompare compareLineNums;

	std::sort(origLines.begin(), origLines.end(), compare);
	std::sort(newLines.begin(), newLines.end(), compare);

  set_difference(origLines.begin(), origLines.end(), 
                 newLines.begin(), newLines.end(),
                 std::inserter(*deletedLinesOut, 
                          deletedLinesOut->begin()), 
                 compare);

  set_difference(newLines.begin(), newLines.end(),
                 origLines.begin(), origLines.end(), 
                 std::inserter(*addedLinesOut, 
                          addedLinesOut->begin()),
                 compare);

  if (isCheckChildren) {

    MvrMapFileLineSet unchangedOrigLines;
    MvrMapFileLineSet unchangedNewLines;

    set_difference(origLines.begin(), origLines.end(), 
                   deletedLinesOut->begin(), deletedLinesOut->end(),
                   std::inserter(unchangedOrigLines, 
                            unchangedOrigLines.begin()), 
                   compare);

    set_difference(newLines.begin(), newLines.end(), 
                   addedLinesOut->begin(), addedLinesOut->end(),
                   std::inserter(unchangedNewLines, 
                            unchangedNewLines.begin()), 
                   compare);

    MvrMapFileLineCompare compareLine;

    for (MvrMapFileLineSet::iterator iterO = unchangedOrigLines.begin(),
                                 iterN = unchangedNewLines.begin();
         ( (iterO != unchangedOrigLines.end()) && 
           (iterN != unchangedNewLines.end()) );
         iterO++, iterN++) {

       MvrMapFileLineGroup &origGroup = *iterO;
       MvrMapFileLineGroup &newGroup = *iterN;

	     std::sort(origGroup.getChildLines()->begin(), 
                 origGroup.getChildLines()->end(), 
                 compareLine);
	     std::sort(newGroup.getChildLines()->begin(), 
                 newGroup.getChildLines()->end(), 
                 compareLine);

       MvrMapFileLineSet tempDeletedLines;
       MvrMapFileLineSet tempAddedLines;
     
       set_difference(origGroup.getChildLines()->begin(), 
                      origGroup.getChildLines()->end(), 
                      newGroup.getChildLines()->begin(), 
                      newGroup.getChildLines()->end(),
                      std::inserter(tempDeletedLines, 
                               tempDeletedLines.begin()), 
                      compareLine);

       set_difference(newGroup.getChildLines()->begin(), 
                      newGroup.getChildLines()->end(),
                      origGroup.getChildLines()->begin(), 
                      origGroup.getChildLines()->end(), 
                      std::inserter(tempAddedLines, 
                              tempAddedLines.begin()),
                      compareLine);

        // TODO: Right now just sending the entire group -- but someday
        // we may just want to send the lines that have changed within
        // the group (plus the group heading).
        if (!tempDeletedLines.empty() || !tempAddedLines.empty()) {

          deletedLinesOut->push_back(origGroup);
          addedLinesOut->push_back(newGroup);

        } // end if child changes

    } // end for each unchanged line

  } // end if check children

	std::sort(deletedLinesOut->begin(), deletedLinesOut->end(), compareLineNums);
	std::sort(addedLinesOut->begin(), addedLinesOut->end(), compareLineNums);

  return true;

} // end method calculateChanges


// -----------------------------------------------------------------------------
// MvrMapChangeDetails
// -----------------------------------------------------------------------------

void MvrMapChangeDetails::createChildArgMap()
{
  myInfoNameToMapOfChildArgsMap["MapInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["TaskInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["Task"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["GoalTask"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["MacroTask"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goto"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goalBefore"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_goalAfter"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_everyBefore"] = true;
  myInfoNameToMapOfChildArgsMap["RouteInfo:"]["_everyAfter"] = true;

  myInfoNameToMapOfChildArgsMap["SchedTaskInfo:"]["ArgDesc"] = true;

  myInfoNameToMapOfChildArgsMap["SchedInfo:"]["Route"] = true;
  myInfoNameToMapOfChildArgsMap["SchedInfo:"]["SchedTask"] = true;

} // end method createChildArgMap

MvrMapChangeDetails::MvrMapScanChangeDetails::MvrMapScanChangeDetails() :
  myChangedPoints(),
  myChangedLineSegments(),
  myChangedSummaryLines()
{
} // end constructor

MvrMapChangeDetails::MvrMapScanChangeDetails::~MvrMapScanChangeDetails()
{
  // TODO

} // end destructor

MVREXPORT MvrMapChangeDetails::MvrMapChangeDetails() :
  myMutex(),
  myOrigMapId(),
  myNewMapId(),
  myInfoNameToMapOfChildArgsMap(),
  myScanTypeList(),
  myScanTypeToChangesMap(),
  myNullScanTypeChanges(),
  myChangedSupplementLines(),
  myChangedObjectLines(),
  myInfoToChangeMaps()
{
  myMutex.setLogName("MvrMapChangeDetails");

  createChildArgMap();

} // end ctor

MVREXPORT MvrMapChangeDetails::MvrMapChangeDetails
                                (const MvrMapChangeDetails &other) :
  myMutex(),
  myOrigMapId(other.myOrigMapId),
  myNewMapId(other.myNewMapId),
  myInfoNameToMapOfChildArgsMap(),
  myScanTypeList(other.myScanTypeList),
  myScanTypeToChangesMap(),
  myNullScanTypeChanges(),
  myChangedSupplementLines(),
  myChangedObjectLines(),
  myInfoToChangeMaps()
{
  myMutex.setLogName("MvrMapChangeDetails");

  createChildArgMap();

  for (std::map<std::string, MvrMapScanChangeDetails*>::const_iterator iter =
                                 other.myScanTypeToChangesMap.begin();
       iter != other.myScanTypeToChangesMap.end();
       iter++) {
    MvrMapScanChangeDetails *otherScan = iter->second;
    if (otherScan == NULL) {
      continue;
    }
    myScanTypeToChangesMap[iter->first] = new MvrMapScanChangeDetails(*otherScan);
  }

  for (int i = 0; i < CHANGE_TYPE_COUNT; i++) {
    myChangedSupplementLines[i] = other.myChangedSupplementLines[i];
    myChangedObjectLines[i] = other.myChangedObjectLines[i];
    myInfoToChangeMaps[i] = other.myInfoToChangeMaps[i];
  }
  
} // end copy ctor

MVREXPORT MvrMapChangeDetails &MvrMapChangeDetails::operator=
                                  (const MvrMapChangeDetails &other)
{
  if (this != &other) {

    myOrigMapId = other.myOrigMapId;
    myNewMapId  = other.myNewMapId;
    myScanTypeList = other.myScanTypeList;

    MvrUtil::deleteSetPairs(myScanTypeToChangesMap.begin(),
                           myScanTypeToChangesMap.end());
    myScanTypeToChangesMap.clear();

    for (std::map<std::string, MvrMapScanChangeDetails*>::const_iterator iter =
                                  other.myScanTypeToChangesMap.begin();
        iter != other.myScanTypeToChangesMap.end();
        iter++) {
      MvrMapScanChangeDetails *otherScan = iter->second;
      if (otherScan == NULL) {
        continue;
      }
      myScanTypeToChangesMap[iter->first] = new MvrMapScanChangeDetails(*otherScan);
    }

    for (int i = 0; i < CHANGE_TYPE_COUNT; i++) {
      myChangedSupplementLines[i] = other.myChangedSupplementLines[i];
      myChangedObjectLines[i] = other.myChangedObjectLines[i];
      myInfoToChangeMaps[i] = other.myInfoToChangeMaps[i];
    }
  
  }
  return *this;

} // end operator=


MVREXPORT MvrMapChangeDetails::~MvrMapChangeDetails()
{
  MvrUtil::deleteSetPairs(myScanTypeToChangesMap.begin(),
                         myScanTypeToChangesMap.end());
}


MVREXPORT bool MvrMapChangeDetails::getOrigMapId(MvrMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    return false;
  }
  *mapIdOut = myOrigMapId;
  return true;
}

MVREXPORT bool MvrMapChangeDetails::getNewMapId(MvrMapId *mapIdOut)
{
  if (mapIdOut == NULL) {
    return false;
  }
  *mapIdOut = myNewMapId;
  return true;
}

MVREXPORT void MvrMapChangeDetails::setOrigMapId(const MvrMapId &mapId)
{
  myOrigMapId = mapId;
  myOrigMapId.log("MvrMapChangeDetails::setOrigMapId");
}

MVREXPORT void MvrMapChangeDetails::setNewMapId(const MvrMapId &mapId)
{
  myNewMapId = mapId;
  myNewMapId.log("MvrMapChangeDetails::setNewMapId");
}


MVREXPORT std::list<std::string> *MvrMapChangeDetails::getScanTypes() 
{
  return &myScanTypeList;
}

MVREXPORT std::vector<MvrPose> *MvrMapChangeDetails::getChangedPoints
                                                    (MapLineChangeType change,
                                                     const char *scanType) 
{
  MvrMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedPoints[change];
  //return &myChangedPoints[change];
}

MVREXPORT std::vector<MvrLineSegment> *MvrMapChangeDetails::getChangedLineSegments
                                                           (MapLineChangeType change,
                                                            const char *scanType) 
{
  MvrMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedLineSegments[change];
  //return &myChangedLineSegments[change];
}

MVREXPORT MvrMapFileLineSet *MvrMapChangeDetails::getChangedSummaryLines
                                               (MapLineChangeType change,
                                                const char *scanType) 
{
  MvrMapScanChangeDetails *scanChange = getScanChangeDetails(scanType);
  return &scanChange->myChangedSummaryLines[change];
  //return &myChangedSummaryLines[change];
}

MVREXPORT MvrMapFileLineSet *MvrMapChangeDetails::getChangedSupplementLines
                                              (MapLineChangeType change) 
{
  return &myChangedSupplementLines[change];
}

MVREXPORT MvrMapFileLineSet *MvrMapChangeDetails::getChangedObjectLines
                                              (MapLineChangeType change) 
{
  return &myChangedObjectLines[change];
}

MVREXPORT MvrMapFileLineSet *MvrMapChangeDetails::getChangedInfoLines
                                                (const char *infoName,
                                                 MapLineChangeType change) 
{
  if (MvrUtil::isStrEmpty(infoName)) {
    MvrLog::log(MvrLog::Normal, "MvrMapChangeDetails::getChangedInfoLines() null info name");
    return NULL;
  }

  std::map<std::string, MvrMapFileLineSet>::iterator iter = myInfoToChangeMaps[change].find(infoName);
  if (iter == myInfoToChangeMaps[change].end()) {
    myInfoToChangeMaps[change][infoName] = MvrMapFileLineSet();
    iter = myInfoToChangeMaps[change].find(infoName);
  }
  return &(iter->second);
}



MVREXPORT bool MvrMapChangeDetails::isChildArg(const char *infoName,
                                             MvrArgumentBuilder *arg) const 
{
  if ((arg == NULL) || 
      (arg->getArgc() < 1)) {
    return false;
  }
  const char *argText = arg->getArg(0);

  return isChildArg(infoName, argText);
}


MVREXPORT bool MvrMapChangeDetails::isChildArg(const char *infoName,
                                             const char *argText) const
{
  if (MvrUtil::isStrEmpty(infoName) || MvrUtil::isStrEmpty(argText)) {
    return false;
  }
  std::map<std::string, std::map<std::string, bool> >::const_iterator iter1 =
                              myInfoNameToMapOfChildArgsMap.find(infoName);
  if (iter1 == myInfoNameToMapOfChildArgsMap.end()) {
    return false;
  }

  std::map<std::string, bool>::const_iterator iter2 = iter1->second.find(argText);
  if (iter2 != iter1->second.end()) {
    return iter2->second;
  }

  return false;

} // end method isChildArg

MvrMapChangeDetails::MvrMapScanChangeDetails *MvrMapChangeDetails::getScanChangeDetails
                                                      (const char *scanType)
{
  MvrMapScanChangeDetails *scanChanges = NULL;

  if (scanType != NULL) {
      
    std::map<std::string, MvrMapScanChangeDetails*>::iterator iter = 
                              myScanTypeToChangesMap.find(scanType);
    if (iter != myScanTypeToChangesMap.end()) {
      scanChanges = iter->second;
    }
    else {
/**
      MvrLog::log(MvrLog::Normal,
        "MvrMapChangeDetails::getScanChangeDetails() adding details for scan type %s",
        scanType);
**/

      if (MvrUtil::isStrEmpty(scanType)) {
        MvrLog::log(MvrLog::Verbose,
                   "MvrMapChangeDetails::getScanChangeDetails() adding empty scan type%s",
                   scanType);
      }
      
      scanChanges = new MvrMapScanChangeDetails();
      myScanTypeToChangesMap[scanType] = scanChanges;

      myScanTypeList.push_back(scanType);
    }

  } // end if scanType not null

  if (scanChanges == NULL) {
    scanChanges = &myNullScanTypeChanges;
  }
  return scanChanges;
} // end method getScanChangeDetails


MVREXPORT std::list<std::string> MvrMapChangeDetails::findChangedInfoNames() const 
{
  std::list<std::string> changedInfoNames;
  std::map<std::string, bool> infoNameToBoolMap;

  for (int change = 0; change < CHANGE_TYPE_COUNT; change++) {
    for (std::map<std::string, MvrMapFileLineSet>::const_iterator iter = myInfoToChangeMaps[change].begin();
          iter != myInfoToChangeMaps[change].end();
          iter++) {
      const MvrMapFileLineSet &fileLineSet = iter->second;
      if (!fileLineSet.empty()) {
        infoNameToBoolMap[iter->first] = true;
      }
    }
  }  // end for each change type

  for (std::map<std::string, bool>::const_iterator iter2 = infoNameToBoolMap.begin();
        iter2 != infoNameToBoolMap.end();
        iter2++) {
    changedInfoNames.push_back(iter2->first);
  } // end for each info type

  return changedInfoNames;

} // end method getChangedInfoTypes


MVREXPORT void MvrMapChangeDetails::log()
{

  MvrLog::log(MvrLog::Normal,
             "");

  for (int t = 0; t < CHANGE_TYPE_COUNT; t++) {

    MapLineChangeType change = (MapLineChangeType) t;

    switch (t) {
    case DELETIONS:
      MvrLog::log(MvrLog::Normal,
                "---- DELETED MAP LINES --------------");
      break;
    case ADDITIONS:
      MvrLog::log(MvrLog::Normal,
                "---- ADDED MAP LINES ----------------");
      break;

    default:
      return;
    }

    for (std::list<std::string>::iterator iter2 = myScanTypeList.begin();
         iter2 != myScanTypeList.end();
         iter2++) {

      const char *scanType = (*iter2).c_str();

      MvrLog::log(MvrLog::Normal,
                "%s Point Count:  %i",
                scanType,
                getChangedPoints(change, scanType)->size());
      MvrLog::log(MvrLog::Normal,
                "%s Line Segment Count:  %i",
                scanType,
                getChangedLineSegments(change, scanType)->size());


      MvrLog::log(MvrLog::Normal,
                "");

      MvrMapFileLineSet *changedSummaryLines = getChangedSummaryLines(change,
                                                                  scanType);
      if (!changedSummaryLines->empty()) {
        std::string scanTypeSummary = scanType;
        scanTypeSummary += " ";
        scanTypeSummary += "Summary Lines";

        changedSummaryLines->log(scanTypeSummary.c_str());
      }
    } // end for each scan type

    if (!myChangedSupplementLines[t].empty()) {
      myChangedSupplementLines[t].log("Map Supplement Lines");
    }

    if (!myChangedObjectLines[t].empty()) {
      myChangedObjectLines[t].log("Map Object Lines");
    }

    for (std::map<std::string, MvrMapFileLineSet>::iterator iter = myInfoToChangeMaps[t].begin();
         iter != myInfoToChangeMaps[t].end();
         iter++) {
      MvrMapFileLineSet &fileLineSet = iter->second;
      if (!fileLineSet.empty()) {
        fileLineSet.log("      ");
      }
    }
  } // end for each change type 
} // end method log

MVREXPORT void MvrMapChangeDetails::lock()
{
  myMutex.lock();
}

MVREXPORT void MvrMapChangeDetails::unlock()
{
  myMutex.unlock();
}

// ------------------------------------------------------------------------------
// MvrMapChangedHelper
// ------------------------------------------------------------------------------

MVREXPORT MvrMapChangedHelper::MvrMapChangedHelper() :
  myMapChangedLogLevel(MvrLog::Verbose),
  myMapChangedCBList(),
  myPreMapChangedCBList()
{
  myMapChangedCBList.setName("MapChangedHelper");
  myPreMapChangedCBList.setName("PreMapChangedHelper");
}

MVREXPORT MvrMapChangedHelper::~MvrMapChangedHelper()
{
}

MVREXPORT void MvrMapChangedHelper::invokeMapChangedCallbacks(void)
{
  MvrLog::LogLevel level = myMapChangedLogLevel;

  myPreMapChangedCBList.setLogLevel(level);
  myPreMapChangedCBList.invoke();
  
  myMapChangedCBList.setLogLevel(level);
  myMapChangedCBList.invoke();

} // end method invokeMapChangedCallbacks


MVREXPORT void MvrMapChangedHelper::addMapChangedCB(MvrFunctor *functor, 
						  int position)
{
  myMapChangedCBList.addCallback(functor, position);
} // end method addMapChangedCB

MVREXPORT void MvrMapChangedHelper::remMapChangedCB(MvrFunctor *functor)
{
  myMapChangedCBList.remCallback(functor);

} // end method remMapChangedCB


MVREXPORT void MvrMapChangedHelper::addPreMapChangedCB(MvrFunctor *functor,
                                                     int position)
{
  myPreMapChangedCBList.addCallback(functor, position);
} // end method addPreMapChangedCB


MVREXPORT void MvrMapChangedHelper::remPreMapChangedCB(MvrFunctor *functor)
{
  myPreMapChangedCBList.remCallback(functor);

} // end method remPreMapChangedCB


MVREXPORT void MvrMapChangedHelper::setMapChangedLogLevel(MvrLog::LogLevel level)
{
  myMapChangedLogLevel = level;

} // end method setMapChangedLogLevel


MVREXPORT MvrLog::LogLevel MvrMapChangedHelper::getMapChangedLogLevel(void)
{
  return myMapChangedLogLevel;

} // end method getMapChangedLogLevel







