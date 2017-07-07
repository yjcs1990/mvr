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
