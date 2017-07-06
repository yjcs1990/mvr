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
#include "mvriaOSDef.h"
#include "mvriaInternal.h"

#include "MvrMapInterface.h"


MVREXPORT const char *MvrMapInfoInterface::MAP_INFO_NAME        = "MapInfo:"; 
MVREXPORT const char *MvrMapInfoInterface::META_INFO_NAME       = "MetaInfo:";
MVREXPORT const char *MvrMapInfoInterface::TASK_INFO_NAME       = "TaskInfo:";   
MVREXPORT const char *MvrMapInfoInterface::ROUTE_INFO_NAME      = "RouteInfo:"; 
MVREXPORT const char *MvrMapInfoInterface::SCHED_TASK_INFO_NAME = "SchedTaskInfo:";
MVREXPORT const char *MvrMapInfoInterface::SCHED_INFO_NAME      = "SchedInfo:"; 
MVREXPORT const char *MvrMapInfoInterface::CAIRN_INFO_NAME      = "CairnInfo:";  
MVREXPORT const char *MvrMapInfoInterface::CUSTOM_INFO_NAME     = "CustomInfo:";

MVREXPORT const char *MvrMapInterface::MAP_CATEGORY_2D = "2D-Map";
MVREXPORT const char *MvrMapInterface::MAP_CATEGORY_2D_MULTI_SOURCES = "2D-Map-Ex";
MVREXPORT const char *MvrMapInterface::MAP_CATEGORY_2D_EXTENDED  = "2D-Map-Ex2";
MVREXPORT const char *MvrMapInterface::MAP_CATEGORY_2D_COMPOSITE = "2D-Map-Ex3";



MVREXPORT bool MvrMapScanInterface::isDefaultScanType(const char *scanType)
{
  bool b = false;
  if ((scanType != NULL) &&
      (MvrUtil::isStrEmpty(scanType))) {
    b = true;
  }
  return b;
}

MVREXPORT bool MvrMapScanInterface::isSummaryScanType(const char *scanType)
{
  bool b = scanType == NULL;
  return b;
}

// ----------------------------------------------------------------------------


/** 
 * Determines what system file path to use based on the contents of @a baseDirectory, @a fileName and
 * @a isIgnoreCase.  If @a fileName is not an absolute path and @a baseDirectory is not null and 
 * not empty, then it is combined with @a baseDirectory to form a full path.
 * An absolute path starts with the '/' or '\' character, or on Windows, with "X:\" where X is any
 * upper or lower case alphabetic character A-Z or a-z.  
 */
MVREXPORT std::string MvrMapInterface::createRealFileName(const char *baseDirectory,
                                                        const char *fileName,
                                                        bool isIgnoreCase)
{ 

  if (fileName == NULL) {
    return "";
  }
  std::string realFileName;
  
  // If there is no base directory or the filename part is an absolute path, use the filename directly without the base directory
  if ((fileName[0] == '/') || 
      (fileName[0] == '\\') ||
      (strlen(baseDirectory) == 0) ||
      (baseDirectory == NULL)
#ifdef WIN32
	  ||
	  ( fileName[1] == ':' && (fileName[2] == '\\' || fileName[2] == '/') && isalpha(fileName[0]) )
#endif
  )
  {
    realFileName = fileName;
  }
  else // non-empty base directory and fileName is not an absolute path
  {
    int totalLen = strlen(baseDirectory) + strlen(fileName) + 10;
    char *nameBuf = new char[totalLen];
    nameBuf[0] = '\0';

    snprintf(nameBuf, totalLen, baseDirectory);
    MvrUtil::appendSlash(nameBuf, totalLen);
    
    realFileName = nameBuf;
    realFileName += fileName;

    delete [] nameBuf;

  } // end else non empty base directory

  // this isn't needed in windows since it ignores case no matter what
#ifndef WIN32
  if (isIgnoreCase)
  {
    char directoryRaw[2048];
    directoryRaw[0] = '\0';
    char fileNamePart[2048];
    fileNamePart[0] = '\0';
    if (!MvrUtil::getDirectory(realFileName.c_str(), 
					                    directoryRaw, sizeof(directoryRaw)) ||
	      !MvrUtil::getFileName(realFileName.c_str(), 
			                       fileNamePart, sizeof(fileNamePart)))
    {
      MvrLog::log(MvrLog::Normal, 
		             "MvrMap: Problem with filename '%s'", 
		             realFileName.c_str());
      return "";
    }
    

    char directory[2048];
    //printf("DirectoryRaw %s\n", directoryRaw);
    if (strlen(directoryRaw) == 0 || strcmp(directoryRaw, ".") == 0)
    {
      strcpy(directory, ".");
    }
    else if (directoryRaw[0] == '/')
    {
      strcpy(directory, directoryRaw);
    }
    else if (!MvrUtil::matchCase(baseDirectory, 
				                        directoryRaw, 
                                directory, 
				                        sizeof(directory)))
    {
	    MvrLog::log(MvrLog::Normal, 
		             "MvrMap: Bad directory for '%s'", 
		              realFileName.c_str());
      return "";
    }

    char tmpDir[2048];
    tmpDir[0] = '\0';
    //sprintf(tmpDir, "%s", tmpDir, directory);
    strcpy(tmpDir, directory);
    MvrUtil::appendSlash(tmpDir, sizeof(tmpDir));
    char squashedFileName[2048];
    
    if (MvrUtil::matchCase(tmpDir, fileNamePart, 
			                    squashedFileName, 
			                    sizeof(squashedFileName)))
    {
      realFileName = tmpDir;
      realFileName += squashedFileName;
      //printf("squashed from %s %s\n", tmpDir, squashedFileName);
    }
    else
    {
      realFileName = tmpDir;
      realFileName += fileNamePart;
      //printf("unsquashed from %s %s\n", tmpDir, fileNamePart);
    }
    
    MvrLog::log(MvrLog::Verbose, 
	       "MvrMap: %s is %s",
	       fileName, realFileName.c_str());
  }
#endif

  return realFileName;

} // end method createRealFileName

MVREXPORT void MvrMapInterface::addMapChangedCB(MvrFunctor *functor, 
					      MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    addMapChangedCB(functor, 75);
  else if (position == MvrListPos::LAST)
    addMapChangedCB(functor, 25);
  else
    MvrLog::log(MvrLog::Terse, "MvrMapInterface::addPreMapChangedCB: Invalid position.");
}


MVREXPORT void MvrMapInterface::addPreMapChangedCB(MvrFunctor *functor, 
						 MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    addPreMapChangedCB(functor, 75);
  else if (position == MvrListPos::LAST)
    addPreMapChangedCB(functor, 25);
  else
    MvrLog::log(MvrLog::Terse, "MvrMapInterface::addPreMapChangedCB: Invalid position.");
}
