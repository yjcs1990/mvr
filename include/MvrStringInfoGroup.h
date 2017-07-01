#ifndef MVRSTRINGINFOGROUP_H
#define MVRSTRINGINFOGROUP_H

#include "mvriaUtil.h"
#include "MvrMutex.h"
#include <string>
#include <set>
#include <list>

/**
   This class takes callbacks from different classes that want this
   string information and then lets you just add the information here
   instead of to each individual class.

  @ingroup OptionalClasses
 **/
class MvrStringInfoGroup
{
public:
  /// Constructor
  MVREXPORT MvrStringInfoGroup();
  /// Destructor
  MVREXPORT virtual ~MvrStringInfoGroup();
  /// Adds a string to the list in the raw format
  MVREXPORT bool addString(const char *name, MvrTypes::UByte2 maxLen, 
			  MvrFunctor2<char *, MvrTypes::UByte2> *functor);

  /// Adds an int to the list in the helped way
  MVREXPORT bool addStringInt(const char *name, MvrTypes::UByte2 maxLen, 
			     MvrRetFunctor<int> *functor, 
			     const char *format = "%d");

  /// Adds a double to the list in the helped way
  MVREXPORT bool addStringDouble(const char *name, MvrTypes::UByte2 maxLen, 
				MvrRetFunctor<double> *functor, 
				const char *format = "%g");

  /// Adds a bool to the list in the helped way
  MVREXPORT bool addStringBool(const char *name, MvrTypes::UByte2 maxLen, 
			      MvrRetFunctor<bool> *functor,
			      const char *format = "%s");

  /// Adds a string to the list in the helped way
  MVREXPORT bool addStringString(const char *name, MvrTypes::UByte2 maxLen, 
			      MvrRetFunctor<const char *> *functor,
			      const char *format = "%s");

  /// Adds a std::string to the list. std::string::c_str() will be used to  
  MVREXPORT bool addStringString(const char *name, MvrTypes::UByte2 maxLen,
            MvrRetFunctor<std::string> *functor);

  /// Adds an int to the list in the helped way
  MVREXPORT bool addStringUnsignedLong(const char *name, 
				      MvrTypes::UByte2 maxLen, 
				      MvrRetFunctor<unsigned long> *functor, 
				      const char *format = "%lu");

  /// Adds an int to the list in the helped way
  MVREXPORT bool addStringLong(const char *name, 
			      MvrTypes::UByte2 maxLen, 
			      MvrRetFunctor<long> *functor, 
			      const char *format = "%ld");

  /// This is the function to add a callback to be called by addString
  MVREXPORT void addAddStringCallback(
	  MvrFunctor3<const char *, MvrTypes::UByte2,
	  MvrFunctor2<char *, MvrTypes::UByte2> *> *functor,
	  MvrListPos::Pos position = MvrListPos::LAST);

protected:
  MvrMutex myDataMutex;
  std::set<std::string, MvrStrCaseCmpOp> myAddedStrings;
  std::list<MvrFunctor3<const char *, MvrTypes::UByte2,
	      MvrFunctor2<char *, MvrTypes::UByte2> *> *> myAddStringCBList;
};


#endif // MVRSTRINGINFOGROUP_H
