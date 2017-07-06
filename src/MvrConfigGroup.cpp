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
#include "MvrConfigGroup.h"
#include "MvrArgumentBuilder.h"
#include "MvrLog.h"

MVREXPORT MvrConfigGroup::MvrConfigGroup(const char *baseDirectory)
{
  if (baseDirectory != NULL)
    myBaseDirectory = baseDirectory;
  else
    myBaseDirectory = "";
}

MVREXPORT MvrConfigGroup::~MvrConfigGroup(void)
{
  
}

MVREXPORT void MvrConfigGroup::addConfig(MvrConfig *config)
{
  myConfigs.push_back(config);
}

MVREXPORT void MvrConfigGroup::remConfig(MvrConfig *config)
{
  myConfigs.remove(config);
}

MVREXPORT bool MvrConfigGroup::parseFile(const char *fileName, 
				       bool continueOnError)
{
  std::list<MvrConfig *>::iterator it;
  bool ret = true;

  myLastFile = fileName;
  // go through all the configs and set the base directory (we don't
  // do it when we're parsing just so that whether it suceeds or fails
  // its the same behavior in this base directory regard)
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    (*it)->setBaseDirectory(myBaseDirectory.c_str());
  }
  // now we go through and parse files... if we get an error we stop
  // if we're supposed to
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    if (!(*it)->parseFile(fileName, continueOnError))
    {
      // if we are continuing on errors we still want to tell them we
      // had an error
      ret = false;
      // if we aren't continuing on error then just return
      if (!continueOnError)
	return false;
    }
  }
  return ret;
}

MVREXPORT bool MvrConfigGroup::reloadFile(bool continueOnError)
{
  return parseFile(myLastFile.c_str(), continueOnError);
}

MVREXPORT bool MvrConfigGroup::writeFile(const char *fileName)
{
  std::set<std::string> alreadyWritten;
  std::list<MvrConfig *>::iterator it;
  bool ret = true;
  bool append = false;
  
  // go through all the configs and set the base directory (we don't
  // do it when we're parsing just so that whether it suceeds or fails
  // its the same behavior in this base directory regard)
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    (*it)->setBaseDirectory(myBaseDirectory.c_str());
  }
  // now we go through and parse files... if we get an error we stop
  // if we're supposed to
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    MvrLog::log(MvrLog::Verbose, "Writing config file");
    if (!(*it)->writeFile(fileName, append, &alreadyWritten))
    {
      // if we are continuing on errors we still want to tell them we
      // had an error
      ret = false;
    }
    append = true;
  }
  return ret;
  
}

MVREXPORT void MvrConfigGroup::setBaseDirectory(const char *baseDirectory)
{
  myBaseDirectory = baseDirectory;
}

MVREXPORT const char *MvrConfigGroup::getBaseDirectory(void) const
{
  return myBaseDirectory.c_str();
}
