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
#include "MvrStringInfoGroup.h"

/* 
   TODO the stringInt functions and such'll leak memory on a failed attempt 
*/

MVREXPORT MvrStringInfoGroup::MvrStringInfoGroup()
{
  myDataMutex.setLogName("MvrStringInfoGroup::myDataMutex");
}

MVREXPORT MvrStringInfoGroup::~MvrStringInfoGroup()
{

}

/**
  @note It is recommended that you avoid characters in a string name or value
  which may have special meaning in contexts like MvrConfig files, such as 
  '#', ';', tabs and newlines.
*/
MVREXPORT bool MvrStringInfoGroup::addString(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrFunctor2<char *, MvrTypes::UByte2> *functor)
{
  myDataMutex.lock();
  if (myAddedStrings.find(name) != myAddedStrings.end())
  {
    MvrLog::log(MvrLog::Normal, "MvrStringInfoGroups: Cannot add info '%s', duplicate", name);
    myDataMutex.unlock();
    return false;
  }

  std::list<MvrFunctor3<const char *, MvrTypes::UByte2,
  MvrFunctor2<char *, MvrTypes::UByte2> *> *>::iterator it;
  MvrLog::log(MvrLog::Verbose, "MvrStringInfoGroups: Adding info '%s'", name);
  myAddedStrings.insert(name);
  for (it = myAddStringCBList.begin(); it != myAddStringCBList.end(); it++)
  {
    (*it)->invoke(name, maxLength, functor);
  }
  MvrLog::log(MvrLog::Verbose, "MvrStringInfoGroups: Added info '%s'", name);
  myDataMutex.unlock();
  return true;
}

MVREXPORT bool MvrStringInfoGroup::addStringInt(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<int> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<int> *, 
	     const char *>(&MvrStringInfoHolderFunctions::intWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}

MVREXPORT bool MvrStringInfoGroup::addStringDouble(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<double> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<double> *, 
	     const char *>(&MvrStringInfoHolderFunctions::doubleWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}


MVREXPORT bool MvrStringInfoGroup::addStringBool(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<bool> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<bool> *, 
	     const char *>(&MvrStringInfoHolderFunctions::boolWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}

// for a functor returning a C string (const char*)
MVREXPORT bool MvrStringInfoGroup::addStringString(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<const char *> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<const char *> *, 
	     const char *>(&MvrStringInfoHolderFunctions::stringWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}

// for a functor returning a C++ string (std::string)
MVREXPORT bool MvrStringInfoGroup::addStringString(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<std::string> *functor)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor3<char *, MvrTypes::UByte2, MvrRetFunctor<std::string>*>(
         &MvrStringInfoHolderFunctions::cppStringWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor)
      )
  );
}

MVREXPORT bool MvrStringInfoGroup::addStringUnsignedLong(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<unsigned long> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<unsigned long> *, 
	     const char *>(&MvrStringInfoHolderFunctions::unsignedLongWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}

MVREXPORT bool MvrStringInfoGroup::addStringLong(
	const char *name, MvrTypes::UByte2 maxLength,
	MvrRetFunctor<long> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new MvrGlobalFunctor4<char *, MvrTypes::UByte2, 
	     MvrRetFunctor<long> *, 
	     const char *>(&MvrStringInfoHolderFunctions::longWrapper, 
			   (char *)NULL, (MvrTypes::UByte2) 0, 
			   functor, format)));
}


MVREXPORT void MvrStringInfoGroup::addAddStringCallback(
	MvrFunctor3<const char *, MvrTypes::UByte2,
	MvrFunctor2<char *, MvrTypes::UByte2> *> *functor,
	MvrListPos::Pos position)
{
  if (position == MvrListPos::FIRST)
    myAddStringCBList.push_front(functor);
  else if (position == MvrListPos::LAST)
    myAddStringCBList.push_back(functor);
  else
    MvrLog::log(MvrLog::Terse, 
	       "MvrStringInfoGroup::addAddStringCallback: Invalid position.");
}
