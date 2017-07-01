#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrConfigArg.h"
#include "MvrLog.h"
#include "MvrArgumentBuilder.h"
#include "MvrFileParser.h"
#include "MvrSocket.h"

//#define MVRDEBUG_CONFIGARG

#if (defined(MVRDEBUG_CONFIGARG))
//#if (defined(_DEBUG) && defined(MVRDEBUG_CONFIGARG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif 

// KMC 7/16/12 Changed from 2 to 3 just to nicely align with description holder
//
int MvrConfigArg::ourIndentSpaceCount = 3;

MVREXPORT const char *MvrConfigArg::LIST_BEGIN_TAG  = "_beginList";
MVREXPORT const char *MvrConfigArg::LIST_END_TAG    = "_endList";

MVREXPORT const char *MvrConfigArg::NULL_TAG = "NULL";
MVREXPORT const char *MvrConfigArg::NEW_RESOURCE_TAG = "xltrNew";


std::map<std::string, MvrConfigArg::Type, MvrStrCaseCmpOp> *MvrConfigArg::ourTextToTypeMap = NULL;
std::map<std::string, MvrConfigArg::RestartLevel, MvrStrCaseCmpOp> *MvrConfigArg::ourTextToRestartLevelMap = NULL;


MVREXPORT const char *MvrConfigArg::toString(Type t)
{
   switch (t) {
   case INVALID:
     return "INVALID";
   case INT:
     return "INT";
   case DOUBLE:
     return "DOUBLE";
   case STRING:
     return "STRING";
   case CPPSTRING:
     return "CPPSTRING";
   case BOOL:
     return "BOOL";
   case FUNCTOR:
     return "FUNCTOR";
   case DESCRIPTION_HOLDER:
     return "DESCRIPTION_HOLDER";
   case STRING_HOLDER:
     return "STRING_HOLDER";
   case SEPARATOR:
     return "SEPARATOR";
   case LIST:
     return "LIST";
   case LIST_HOLDER:
     return "LIST_HOLDER";

   } // end switch t

   return "Unknown";

} // end method toString


const char *MvrConfigArg::toString(RestartLevel r)
{
  switch (r) {

  case NO_RESTART:
    return "NO_RESTART";
  case RESTART_CLIENT:
    return "RESTART_CLIENT";
  case RESTART_IO:
    return "RESTART_IO";
  case RESTART_SOFTWARE:
    return "RESTART_SOFTWARE";
  case RESTART_HARDWARE:
    return "RESTART_HARDWARE";
  }
  return "Unknown";

} // end method toString


MvrConfigArg::Type MvrConfigArg::typeFromString(const char *text)
{
  if (MvrUtil::isStrEmpty(text)) {
    return INVALID;
  }
  if (ourTextToTypeMap == NULL) {
    ourTextToTypeMap = new std::map<std::string, Type, MvrStrCaseCmpOp>();
    (*ourTextToTypeMap)["INVALID"] = INVALID;
    (*ourTextToTypeMap)["INT"] = INT;
    (*ourTextToTypeMap)["DOUBLE"] = DOUBLE;
    (*ourTextToTypeMap)["STRING"] = STRING;
    (*ourTextToTypeMap)["CPPSTRING"] = CPPSTRING;
    (*ourTextToTypeMap)["BOOL"] = BOOL;
    (*ourTextToTypeMap)["FUNCTOR"] = FUNCTOR;
    (*ourTextToTypeMap)["DESCRIPTION_HOLDER"] = DESCRIPTION_HOLDER;
    (*ourTextToTypeMap)["STRING_HOLDER"] = STRING_HOLDER;
    (*ourTextToTypeMap)["SEPARATOR"] = SEPARATOR;
    (*ourTextToTypeMap)["LIST"] = LIST;
    (*ourTextToTypeMap)["LIST_HOLDER"] = LIST_HOLDER;

  }

  std::map<std::string, Type, MvrStrCaseCmpOp>::iterator iter = ourTextToTypeMap->find(text);
  if (iter != ourTextToTypeMap->end()) {
    return iter->second;
  }
  return INVALID;

} // end method typeFromString


MVREXPORT MvrConfigArg::RestartLevel MvrConfigArg::restartLevelFromString(const char *text)
{
  if (MvrUtil::isStrEmpty(text)) {
    return NO_RESTART;
  }
  if (ourTextToRestartLevelMap == NULL) {
    ourTextToRestartLevelMap = new std::map<std::string, RestartLevel, MvrStrCaseCmpOp>();
    (*ourTextToRestartLevelMap)["NO_RESTART"] = NO_RESTART;
    (*ourTextToRestartLevelMap)["RESTART_CLIENT"] = RESTART_CLIENT;
    (*ourTextToRestartLevelMap)["RESTART_IO"] = RESTART_IO;
    (*ourTextToRestartLevelMap)["RESTART_SOFTWARE"] = RESTART_SOFTWARE;
    (*ourTextToRestartLevelMap)["RESTART_HARDWARE"] = RESTART_HARDWARE;
  }
  std::map<std::string, RestartLevel, MvrStrCaseCmpOp>::iterator iter = ourTextToRestartLevelMap->find(text);
  if (iter != ourTextToRestartLevelMap->end()) {
    return iter->second;
  }
  return NO_RESTART;

} // end method restartLevelFromString


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MVREXPORT MvrConfigArg::MvrConfigArg()
{
  clear(true);
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, int *pointer, 
		      const char * description, int minInt, int maxInt) 
{ 
  clear(true, INT, INT_INT);
  set(INT, name, description);

  myData.myIntData.myIntType = INT_INT;
  myData.myIntData.myMinInt = minInt;
  myData.myIntData.myMaxInt = maxInt;
  myData.myIntData.myIntPointer = pointer;
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, short *pointer, 
		      const char * description, int minInt, int maxInt) 

{ 
  clear(true, INT, INT_SHORT);
  set(INT, name, description);

  myData.myIntData.myIntType = INT_SHORT;
  myData.myIntData.myMinInt = minInt;
  myData.myIntData.myMaxInt = maxInt;
  myData.myIntData.myIntShortPointer = pointer;
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, unsigned short *pointer, 
		      const char * description, int minInt, int maxInt) 
{ 
  clear(true, INT, INT_UNSIGNED_SHORT);
  set(INT, name, description);

  myData.myIntData.myIntType = INT_UNSIGNED_SHORT;
  myData.myIntData.myMinInt = minInt;
  myData.myIntData.myMaxInt = maxInt;
  myData.myIntData.myIntUnsignedShortPointer = pointer;
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, unsigned char *pointer, 
		      const char * description, int minInt, int maxInt) 
{ 
  clear(true, INT, INT_UNSIGNED_CHAR);
  set(INT, name, description);

  myData.myIntData.myIntType = INT_UNSIGNED_CHAR;
  myData.myIntData.myMinInt = minInt;
  myData.myIntData.myMaxInt = maxInt;
  myData.myIntData.myIntUnsignedCharPointer = pointer;
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, double *pointer,
		                              const char * description, 
                                  double minDouble, 
		                              double maxDouble,
                                  int precision) 
{ 
  clear(true, DOUBLE);
  set(DOUBLE, name, description);

  myData.myDoubleData.myMinDouble = minDouble;
  myData.myDoubleData.myMaxDouble = maxDouble;
  myData.myDoubleData.myDoublePointer = pointer;
  myData.myDoubleData.myPrecision = precision;
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, bool *pointer, 
		      const char * description) 

{ 
  clear(true, BOOL);
  set(BOOL, name, description);

  myData.myBoolData.myBoolPointer = pointer;
}


/** @swigomit Use MvrConfigArg_Int subclass instead. */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, int val, 
		      const char * description, int minInt, int maxInt) 
{ 
  clear(true, INT, INT_INT);
  set(INT, name, description);

  myData.myIntData.myIntType = INT_INT;
  myData.myIntData.myMinInt = minInt;
  myData.myIntData.myMaxInt = maxInt;

  myData.myIntData.myIntPointer = new int;
  *myData.myIntData.myIntPointer = val;
  myOwnPointedTo = true;
}

/** @swigomit Use MvrConfigArg_Double subclass instead. */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, 
                                  double val,
		                              const char * description, 
                                  double minDouble, 
		                              double maxDouble,
                                  int precision) 
{ 
  clear(true, DOUBLE);
  set(DOUBLE, name, description);

  myData.myDoubleData.myMinDouble = minDouble;
  myData.myDoubleData.myMaxDouble = maxDouble;
  myData.myDoubleData.myPrecision = precision;

  myData.myDoubleData.myDoublePointer = new double;
  *myData.myDoubleData.myDoublePointer = val;
  myOwnPointedTo = true;
}

/** @swigomit Use MvrConfigArg_Bool subclass instead. */
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, bool val, 
		      const char * description) 
{ 
  clear(true, BOOL);
  set(BOOL, name, description);

  myData.myBoolData.myBoolPointer = new bool;
  *myData.myBoolData.myBoolPointer = val;
  myOwnPointedTo = true;
}

/**
 * This constructor can accept both an already-allocated string,
 * or MvrConfigArg can to the memory managment itself (reallocation
 * and finally deletion). If @a maxStrLen is 0, then MvrConfigArg will
 * do its own memory management, with the contents of @a str copied
 * as the initial value of the internally held string. Otherwise,
 * @a str must point to an allocated string, with its size given by @a
 * maxStrLen.
    @warning The parameter name and value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
 *
 *  @swigomit Use MvrConfigArg_String subclass instead (which has no maxStrLen parameter in its constructor)
*/
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, char *str, 
		      const char * description, size_t maxStrLen) 
{ 
  clear(true, STRING);
  set(STRING, name, description);

  if (maxStrLen == 0)
  {
    // KMC own string change
    // myData.myStringData.myUsingOwnedString = true;
    myOwnPointedTo = true;

    if(myData.myStringData.myString)
      delete myData.myStringData.myString;
    myData.myStringData.myString = new std::string(str);
  }
  else
  {
    myData.myStringData.myStringPointer = str;
    myData.myStringData.myMaxStrLen = maxStrLen;
  }
}

/**
 * This constructor can accept a const char str, then it'll take care
 * of the string itself...

    @warning The parameter name and value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
 *
 *  @swigomit Use MvrConfigArg_String subclass instead (which has no maxStrLen parameter in its constructor)
*/
MVREXPORT MvrConfigArg::MvrConfigArg(const char * name, const char *str, 
				  const char * description) 
{ 
  clear(true, STRING);
  set(STRING, name, description);

  // KMC own string change
  // myData.myStringData.myUsingOwnedString = true;
  myOwnPointedTo = true;
  delete myData.myStringData.myString;
  myData.myStringData.myString = new std::string(str);
}

/** @swigomit */
MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, std::string *strptr, const char *description)
{
  clear(true, CPPSTRING);
  set(CPPSTRING, name, description);
  myOwnPointedTo = false;
  myData.myCppStringData.myCppStringPtr = strptr;
}

/**
   This constructor is for the functor type of argument, this is for
   cases that need to be complicated and have more than one argument
   per name... such as the sonar in a config file.  Where this data
   needs to be used to construct internal data structures.

   @param name Name of this argument

   @param description Description of the purpose of this argument
   
   @param setFunctor When an argument is read it is passed to this
   functor in an MvrArgumentBuilder object. The functor should return
   false if there is an error or problem handling the argument, or
   true otherwise.
   
   @param getFunctor Since configuration needs to be serialized to 
   save files on disk or send data over the network etc., this
   functor will be called to get a list of strings to represent this
   argument and its value as text in the file etc.

    @warning The parameter name and value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
**/
MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, 
		      MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor, 
	      MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *getFunctor,
		      const char *description)
{
  clear(true, FUNCTOR);
  set(FUNCTOR, name, description);

  myData.myFunctorData.mySetFunctor = setFunctor;
  myData.myFunctorData.myGetFunctor = getFunctor;
}

MVREXPORT MvrConfigArg::MvrConfigArg(const char * str, Type type)
{ 
  clear(true, type);
  if (type == DESCRIPTION_HOLDER)
  {
    myType = DESCRIPTION_HOLDER;
    myDescription = str;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrConfigArg: Bad type %d for '%s'", type, str);
  }
}

/**
 * This constructor is used for creating composite (LIST or LIST_HOLDER type)
 * parameters.
**/
MVREXPORT MvrConfigArg::MvrConfigArg(Type type,
                                  const char *name, 
		                              const char *description)
{
  clear(true, type);
  set(type, name, description);
  if ((type == LIST) || 
      (type == LIST_HOLDER) ||
      (type == STRING_HOLDER)) {
    myOwnPointedTo = true; 
  }
}

/**
 * This constructor is useful for creating separators within a config
 * section.
**/
MVREXPORT MvrConfigArg::MvrConfigArg(Type type)
{
  clear(true, type);
  set(type, "", "");
}

/**
    @warning The parameter name and value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
*/
MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, const char *str)
{
  /* MPL Taking this out
  MvrLog::log(MvrLog::Normal,
             "MvrConfigArg::ctor() STRING_HOLDER %s : %s",
             ((name != NULL) ? name : ""),
             ((str != NULL) ? str : ""));
  */

  clear(true, STRING_HOLDER);
  set(STRING_HOLDER, name, "");
  
  // KMC own string change
  // myData.myStringData.myUsingOwnedString = true;
  myOwnPointedTo = true;

  if(myData.myStringData.myString)
    delete myData.myStringData.myString;
  myData.myStringData.myString = new std::string(str);
}


MVREXPORT MvrConfigArg::MvrConfigArg(const char *argName,
                                  const MvrConfigArg & arg)
{
  clear(true, 
        arg.myType, 
        ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT));
 
  copy(arg, false);

  myName = ((argName != NULL) ? argName : "");

} // end ctor


MVREXPORT MvrConfigArg::MvrConfigArg(const MvrConfigArg & arg) 
{
  clear(true, 
        arg.myType, 
        ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT));

  copy(arg, false);
}

MVREXPORT MvrConfigArg &MvrConfigArg::operator=(const MvrConfigArg & arg) 
{
  if (this != &arg) 
  {
    copy(arg, false);
  }
  return *this;
}

MVREXPORT void MvrConfigArg::copyAndDetach(const MvrConfigArg &arg)
{
   copy(arg, true);

}


void MvrConfigArg::copy(const MvrConfigArg &arg,
                       bool isDetach)
{
  // Clear any existing data, freeing memory as necessary.
  clear(false, 
        myType, 
        ((myType == INT) ? myData.myIntData.myIntType : INT_NOT));

  // If the type has changed,then set up the union for the new type.
  // Note that the isDelete flag is set to false because pointers 
  // may contain bogus information (from the previous type).
  // 
  if ((myType != arg.myType) ||
      ((myType == INT) &&
       (myData.myIntData.myIntType != arg.myData.myIntData.myIntType))) {

    clear(false, 
          arg.myType, 
          ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT),
          false);
  }

  set(arg.myType, 
      arg.myName.c_str(), 
      arg.myDescription.c_str());

  myDisplayName = arg.myDisplayName;

  setExtraExplanation(arg.myExtraExplanation.c_str());

  myOwnPointedTo = arg.myOwnPointedTo || isDetach;

  switch (arg.myType) {

  case INT:
    {

      myData.myIntData.myIntType = arg.myData.myIntData.myIntType;

      switch (myData.myIntData.myIntType) {
      case INT_INT:
        {
          if (myOwnPointedTo && arg.myData.myIntData.myIntPointer != NULL)
          {
            myData.myIntData.myIntPointer = new int;
            *myData.myIntData.myIntPointer = *arg.myData.myIntData.myIntPointer;
          }
          else
          {
            myData.myIntData.myIntPointer = arg.myData.myIntData.myIntPointer;
          }
        }
        break;
      case INT_SHORT:
        {
          if (myOwnPointedTo && arg.myData.myIntData.myIntShortPointer != NULL)
          {
            myData.myIntData.myIntShortPointer = new short;
            *myData.myIntData.myIntShortPointer = *arg.myData.myIntData.myIntShortPointer;
          }
          else
          {
            myData.myIntData.myIntShortPointer = arg.myData.myIntData.myIntShortPointer;
          }
        }
        break;
      case INT_UNSIGNED_SHORT:
      {
          if (myOwnPointedTo && arg.myData.myIntData.myIntUnsignedShortPointer != NULL)
          {
            myData.myIntData.myIntUnsignedShortPointer = new unsigned short;
            *myData.myIntData.myIntUnsignedShortPointer = *arg.myData.myIntData.myIntUnsignedShortPointer;
          }
          else
          {
            myData.myIntData.myIntUnsignedShortPointer = arg.myData.myIntData.myIntUnsignedShortPointer;
          }
        }
        break;
      case INT_UNSIGNED_CHAR:
        {
          if (myOwnPointedTo && arg.myData.myIntData.myIntUnsignedCharPointer != NULL)
          {
            myData.myIntData.myIntUnsignedCharPointer = new unsigned char;
            *myData.myIntData.myIntUnsignedCharPointer = *arg.myData.myIntData.myIntUnsignedCharPointer;
          }
          else
          {
            myData.myIntData.myIntUnsignedCharPointer = arg.myData.myIntData.myIntUnsignedCharPointer;
          }
        }
        break;

      case INT_NOT:
        {
          MvrLog::log(MvrLog::Normal, "MvrConfigArg: Internal warning: MvrConfigArg has INT type, but integer subtype is INT_NOT!  Can't copy pointer.");
        }
        break;

      } // end switch int type

      myData.myIntData.myMinInt = arg.myData.myIntData.myMinInt;
      myData.myIntData.myMaxInt = arg.myData.myIntData.myMaxInt;

    }
    break;

  case DOUBLE:
    {
      if (myOwnPointedTo && arg.myData.myDoubleData.myDoublePointer != NULL) 
      {
        myData.myDoubleData.myDoublePointer = new double;
        *myData.myDoubleData.myDoublePointer = *arg.myData.myDoubleData.myDoublePointer;
      }
      else
      {
        myData.myDoubleData.myDoublePointer = arg.myData.myDoubleData.myDoublePointer;
      }
      myData.myDoubleData.myMinDouble = arg.myData.myDoubleData.myMinDouble;
      myData.myDoubleData.myMaxDouble = arg.myData.myDoubleData.myMaxDouble;
      myData.myDoubleData.myPrecision = arg.myData.myDoubleData.myPrecision;
    }
    break;

  case STRING:
  case STRING_HOLDER:
    {
      /** Originally...
      myData.myStringData.myStringPointer = arg.myData.myStringData.myStringPointer;
      myData.myStringData.myUsingOwnedString = arg.myData.myStringData.myUsingOwnedString; 
      myData.myStringData.myString = arg.myData.myStringData.myString;
      **/

      // KMC own string change
      myOwnPointedTo = arg.myOwnPointedTo;
      // myData.myStringData.myUsingOwnedString = arg.myData.myStringData.myUsingOwnedString || isDetach;
      //if (myData.myStringData.myUsingOwnedString) {
      if (myOwnPointedTo) {
        myData.myStringData.myStringPointer = NULL;
        //if (!arg.myData.myStringData.myUsingOwnedString && (arg.myData.myStringData.myStringPointer != NULL)) {
        if (!arg.myOwnPointedTo && (arg.myData.myStringData.myStringPointer != NULL)) {

          if(myData.myStringData.myString)
            delete myData.myStringData.myString;
          myData.myStringData.myString = new std::string(arg.myData.myStringData.myStringPointer);
        }
        else  {

          if(myData.myStringData.myString)
            delete myData.myStringData.myString;
          if (arg.myData.myStringData.myString != NULL) {
            myData.myStringData.myString = new std::string(*arg.myData.myStringData.myString);
          }
          else {
            myData.myStringData.myString = NULL;
          }
        }
      }
      else { // use shared string pointer

        myData.myStringData.myStringPointer = arg.myData.myStringData.myStringPointer;

        if (myData.myStringData.myString != NULL)
          delete myData.myStringData.myString;
        if (arg.myData.myStringData.myString != NULL) {
          myData.myStringData.myString = new std::string(*arg.myData.myStringData.myString);
        }
        else {
          myData.myStringData.myString = NULL;
        }
      } // end else use shared string pointer
  
      myData.myStringData.myMaxStrLen = arg.myData.myStringData.myMaxStrLen;

    }
    break;

  case CPPSTRING:
    myOwnPointedTo = arg.myOwnPointedTo;
    if(myOwnPointedTo) 
    {
      if (myData.myCppStringData.myCppStringPtr)
        delete myData.myCppStringData.myCppStringPtr;
      if(arg.myData.myCppStringData.myCppStringPtr != NULL)
        myData.myCppStringData.myCppStringPtr = new std::string(*(arg.myData.myCppStringData.myCppStringPtr));
      else
        myData.myCppStringData.myCppStringPtr = NULL;
    }
    else
    {
      myData.myCppStringData.myCppStringPtr = arg.myData.myCppStringData.myCppStringPtr; 
    }

    break;
  

  case BOOL:
    {
      if (myOwnPointedTo && arg.myData.myBoolData.myBoolPointer != NULL)
      {
        myData.myBoolData.myBoolPointer = new bool;
        *myData.myBoolData.myBoolPointer = *arg.myData.myBoolData.myBoolPointer;
      }
      else
      {
        myData.myBoolData.myBoolPointer = arg.myData.myBoolData.myBoolPointer;
      }
    }
    break;

  case FUNCTOR:
    {
      myData.myFunctorData.mySetFunctor = arg.myData.myFunctorData.mySetFunctor;
      myData.myFunctorData.myGetFunctor = arg.myData.myFunctorData.myGetFunctor; 
    }
    break;

  case LIST:
  case LIST_HOLDER:
    {
      myOwnPointedTo = true;
      if (arg.myData.myListData.myChildArgList != NULL) {
        for (std::list<MvrConfigArg>::const_iterator iter = arg.myData.myListData.myChildArgList->begin();
             iter != arg.myData.myListData.myChildArgList->end();
             iter++) {
          addArg(*iter);
        }
      }
    }
    break;

    case INVALID:
    case DESCRIPTION_HOLDER:
    case SEPARATOR:
      // No value to copy.
      break;

  }; // end switch arg type



  // KMC 7/9/12 I think that this should change based on the type... i.e. it's not
  // necessary to set all of them anymore.

  myConfigPriority = arg.myConfigPriority;
  myIgnoreBounds = arg.myIgnoreBounds;
  myDisplayHint = arg.myDisplayHint;
  myRestartLevel = arg.myRestartLevel;

  myValueSet = arg.myValueSet;
  mySuppressChanges = arg.mySuppressChanges;
  myIsTranslated = arg.myIsTranslated;
  myIsSerializable = arg.myIsSerializable;

  // Intentionally not copying the parent (at least currently)

} // end method copy


/**
 * The translation data currently consists of the description and the
 * extra explanation.
**/
MVREXPORT bool MvrConfigArg::copyTranslation(const MvrConfigArg &arg)
{
  if (MvrUtil::strcasecmp(getName(), arg.getName()) != 0) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::copyTranslation() names do not match this = %s arg = %s",
               getName(),
               arg.getName());
    return false;
  }

  setDescription(arg.getDescription());
  setExtraExplanation(arg.getExtraExplanation());

  if (isListType() && arg.isListType()) {
  
   for (size_t i = 0; i < getArgCount(); i++) {
     MvrConfigArg *childArg = getArg(i);
     if (childArg == NULL) {
       continue;
     }
     const MvrConfigArg *xlateChildArg = arg.findArg(childArg->getName());
     if (xlateChildArg != NULL) {

       IFDEBUG(MvrLog::log(MvrLog::Normal,
                          "MvrConfigArg::copyTranslation() found translation for %s child %s",
                           getName(), childArg->getName()));
       childArg->copyTranslation(*xlateChildArg);
     }
     else {
       MvrLog::log(MvrLog::Normal,
                  "MvrConfigArg::copyTranslation() did not find translation for %s child %s",
                  getName(), childArg->getName());
     }
   }

  } // end if lists 

  myIsTranslated = true;

  return true;

} // end method copyTranslation
  

MVREXPORT bool MvrConfigArg::promoteList(const MvrConfigArg &arg)
{
  if ((getType() != LIST_HOLDER) || (arg.getType() != LIST)) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::promoteList() incorrect type for %s",
               arg.getName());
    return false;
  }
  myType = arg.myType;
  myName = arg.myName;
  myDescription = arg.myDescription;
  myExtraExplanation = arg.myExtraExplanation;
  myDisplayName = arg.myDisplayName;
  
  ///MvrConfigArgData myData;

  myConfigPriority = arg.myConfigPriority;
  myDisplayHint = arg.myDisplayHint;
  myRestartLevel = arg.myRestartLevel;
  //MvrConfigArg *myParentArg;

  myOwnPointedTo = arg.myOwnPointedTo;
  myValueSet = arg.myValueSet;
  myIgnoreBounds = arg.myIgnoreBounds;
  myIsTranslated = arg.myIsTranslated;
  mySuppressChanges = arg.mySuppressChanges;
  myIsSerializable = arg.myIsSerializable;   

  return true;
}


MVREXPORT MvrConfigArg::~MvrConfigArg()
{
  clear(false, 
        myType, 
        ((myType == INT) ? myData.myIntData.myIntType : INT_NOT));
}

/**
 * @param initial a bool set to true if this is the initial construciton of the
 * arg, and member fields need to be initialized
 * @param type the Type of the arg
 * @param intType the IntType of the arg, applicable only when type is INT
 * @param isDelete a bool set to true (the default) if clear should free any 
 * used memory; false if the fields should merely be initialized.  The latter
 * is used when the type of an arg needs to be changed (for example, from a 
 * STRING_HOLDER to a "real" value).
**/
void MvrConfigArg::clear(bool initial, 
                        Type type, 
                        IntType intType,
                        bool isDelete)
{
  if (initial)
  {
    myOwnPointedTo = false;
    myIsTranslated = false;
    

    switch (type) {
    case INT:
      switch (intType) {
      case INT_NOT:
      case INT_INT:
        myData.myIntData.myIntPointer = NULL;
        break;
      case INT_SHORT:
        myData.myIntData.myIntShortPointer = NULL;
        break;
      case INT_UNSIGNED_SHORT:
        myData.myIntData.myIntUnsignedShortPointer = NULL;
        break;
      case INT_UNSIGNED_CHAR:
        myData.myIntData.myIntUnsignedCharPointer = NULL;  
        break;
      }
      break;
    case DOUBLE:
      myData.myDoubleData.myDoublePointer = NULL;
      break;
    case STRING:
    case STRING_HOLDER:
     myData.myStringData.myString = NULL;
     break;
    case CPPSTRING:
     myData.myCppStringData.myCppStringPtr = NULL;
     break;
    case BOOL:
      myData.myBoolData.myBoolPointer = NULL;
      break;
    case LIST:
    case LIST_HOLDER:
      myData.myListData.myChildArgList = NULL;
      break;
    case FUNCTOR:
      break;
    case DESCRIPTION_HOLDER:
    case SEPARATOR:
    case INVALID:
      break;
    } // end switch type
    
    myParentArg = NULL;
 
    set(type, "", "");

  } // end if initial

  set(type, "", "");

  switch (type) {
  case INT:
  
    myData.myIntData.myIntType = intType;

    switch (myData.myIntData.myIntType) {
    case INT_NOT:
    case INT_INT:
      
      if (isDelete && myOwnPointedTo && myData.myIntData.myIntPointer != NULL)
        delete myData.myIntData.myIntPointer;
      myData.myIntData.myIntPointer = NULL;
      break;
   
    case INT_SHORT:
      if (isDelete && myOwnPointedTo && myData.myIntData.myIntShortPointer != NULL)
        delete myData.myIntData.myIntShortPointer;
      myData.myIntData.myIntShortPointer = NULL;
      break;

    case INT_UNSIGNED_SHORT:
      if (isDelete && myOwnPointedTo && myData.myIntData.myIntUnsignedShortPointer != NULL)
        delete myData.myIntData.myIntUnsignedShortPointer;
      myData.myIntData.myIntUnsignedShortPointer = NULL;
      break;

    case INT_UNSIGNED_CHAR:
      if (isDelete && myOwnPointedTo && myData.myIntData.myIntUnsignedCharPointer != NULL)
        delete myData.myIntData.myIntUnsignedCharPointer;
      myData.myIntData.myIntUnsignedCharPointer = NULL;

    } break;

    myData.myIntData.myMinInt = INT_MIN;
    myData.myIntData.myMaxInt = INT_MAX;
    break;

  case DOUBLE:
    if (isDelete && myOwnPointedTo && myData.myDoubleData.myDoublePointer != NULL)
      delete myData.myDoubleData.myDoublePointer;

    myData.myDoubleData.myDoublePointer = NULL;
    myData.myDoubleData.myMinDouble = -HUGE_VAL;
    myData.myDoubleData.myMaxDouble = HUGE_VAL;
    myData.myDoubleData.myPrecision = DEFAULT_DOUBLE_PRECISION;
    break;

  case STRING:
  case STRING_HOLDER:

    myData.myStringData.myStringPointer = NULL;
    myData.myStringData.myMaxStrLen = 0;
    // KMC own string change
    // myData.myStringData.myUsingOwnedString = false;
    myOwnPointedTo = false;

    if (isDelete && myData.myStringData.myString) {
      delete myData.myStringData.myString;
    }
    myData.myStringData.myString = NULL;
 
    break;

  case CPPSTRING:
    if(isDelete && myOwnPointedTo && myData.myCppStringData.myCppStringPtr)
      delete myData.myCppStringData.myCppStringPtr;
    myData.myCppStringData.myCppStringPtr = NULL;
    myOwnPointedTo = false;
    break;

  case BOOL:
    if (isDelete && myOwnPointedTo && myData.myBoolData.myBoolPointer != NULL)
      delete myData.myBoolData.myBoolPointer;
    myData.myBoolData.myBoolPointer = NULL;
    break;
  case LIST:
  case LIST_HOLDER:
    // LIST type implies myOwnPointedTo
    if (isDelete && myData.myListData.myChildArgList) {
      delete myData.myListData.myChildArgList;
    }
    myData.myListData.myChildArgList = NULL;
    break;
  case FUNCTOR:
    myData.myFunctorData.mySetFunctor = NULL;
    myData.myFunctorData.myGetFunctor = NULL;  
    break;
  case DESCRIPTION_HOLDER:
  case SEPARATOR:
  case INVALID:
    break;
  } // end switch type

  myConfigPriority = MvrPriority::NORMAL;
  myIgnoreBounds = false;
  myDisplayHint = "";
  myRestartLevel = NO_RESTART;

  myValueSet = false;

  myParentArg = NULL;
  
  myOwnPointedTo = false;
  myIsTranslated = false;
  mySuppressChanges = false;
  myIsSerializable = true;

} // end method clear



MVREXPORT void MvrConfigArg::replaceSpacesInName(void)
{
  size_t i;
  size_t len = myName.size();
  for (i = 0; i < len; i++)
  {
    if (isspace(myName[i]))
      myName[i] = '_';
  }
}

void MvrConfigArg::set(MvrConfigArg::Type type,
                      const char *name,
                      const char *description,
                      IntType intType)
{
  myType = type;
  if (myType == INT) {
    myData.myIntData.myIntType = intType;
  }
  myName = name;
  myDescription = description;

  myDisplayName = ""; // TODO

}


/**
 * @param parentArg the MvrConfigArg * to be stored as the parent; 
 * may be NULL
**/
void MvrConfigArg::setParent(MvrConfigArg *parentArg)
{
  myParentArg = parentArg;
}


/**
 * @return Type the type of this arg, used to determine which 
 * other attributes are valid
*/
MVREXPORT MvrConfigArg::Type MvrConfigArg::getType(void) const
{
  return myType;
}

/**
 * @return char *, the name of this arg or "" if none
**/
MVREXPORT const char *MvrConfigArg::getName(void) const
{
  return myName.c_str();
}

/**
 * @return char *, the description of this arg or "" if none
**/
MVREXPORT const char *MvrConfigArg::getDescription(void) const
{
  return myDescription.c_str();
}



MVREXPORT void MvrConfigArg::setExtraExplanation(const char *extraExplanation)
{
  myExtraExplanation = ((extraExplanation != NULL) ? extraExplanation
                                                   : "");

} // end method setExtraExplanation

  
MVREXPORT const char *MvrConfigArg::getExtraExplanation() const
{
  return myExtraExplanation.c_str();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for INT Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


MVREXPORT int MvrConfigArg::getInt(bool *ok) const
{ 
  if (ok != NULL) {
    *ok = (myType == INT);
  }
  
  if (myType == INT) {
    switch (myData.myIntData.myIntType) {
    case INT_NOT:
    case INT_INT:
      // only one of these will be valid
      if (myData.myIntData.myIntPointer != NULL)
        return *myData.myIntData.myIntPointer;
      break;

    case INT_SHORT:
      if (myData.myIntData.myIntShortPointer != NULL)
        return *myData.myIntData.myIntShortPointer;
      break;

    case INT_UNSIGNED_SHORT: 
      if (myData.myIntData.myIntUnsignedShortPointer != NULL)
        return *myData.myIntData.myIntUnsignedShortPointer;
      break;

    case INT_UNSIGNED_CHAR: 
      if (myData.myIntData.myIntUnsignedCharPointer != NULL)
        return *myData.myIntData.myIntUnsignedCharPointer;
      break;
    }
  } // end if int

  // Possibly not ok if this point reached?

  return 0;

}

/**
 * This method is applicable only when getType() returns INT.
 * @param ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return int the minimum value of this arg, or INT_MIN if 
 * none
**/
MVREXPORT int MvrConfigArg::getMinInt(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == INT);
  }
  if (myType == INT) {
    return myData.myIntData.myMinInt;
  }
  
  return INT_MIN;
}

/**
 * This method is applicable only when getType() returns INT.
 * @param ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return int the maximum value of this arg, or INT_MAX if 
 * none
**/
MVREXPORT int MvrConfigArg::getMaxInt(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == INT);
  }
  if (myType == INT) {

    return myData.myIntData.myMaxInt;
  }
  return INT_MAX;
}

MVREXPORT bool MvrConfigArg::setInt(int val, char *errorBuffer, 
				                          size_t errorBufferLen, bool doNotSet)
{
  if (myType != INT) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::setInt() cannot set %s to %d, wrong type (%s)",
               getName(), val, toString(myType));
    return false;
  }

  // KMC 7/9/12 If the "doNotSet" flag is true, then why is the myValueSet flag
  // set to true?

  myValueSet = true;
  if (!myIgnoreBounds && val < myData.myIntData.myMinInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setInt value %d below range [%d, %d]", getName(), val, myData.myIntData.myMinInt, myData.myIntData.myMaxInt);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %d is below minimum of %d.", getName(), val, myData.myIntData.myMinInt);
    return false;
  }
  if (!myIgnoreBounds && val > myData.myIntData.myMaxInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setInt value %d above range [%d, %d]", getName(), val, myData.myIntData.myMinInt, myData.myIntData.myMaxInt);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %d is above maximum of %d.", getName(), val, myData.myIntData.myMaxInt);
    return false;
  }
  if ((myData.myIntData.myIntType == INT_INT && myData.myIntData.myIntPointer == NULL) || 
      (myData.myIntData.myIntType == INT_SHORT && myData.myIntData.myIntShortPointer == NULL) || 
      (myData.myIntData.myIntType == INT_UNSIGNED_SHORT && 
       myData.myIntData.myIntUnsignedShortPointer == NULL) || 
      (myData.myIntData.myIntType == INT_UNSIGNED_CHAR && myData.myIntData.myIntUnsignedCharPointer == NULL))
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setInt called with NULL int pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }

  if (!doNotSet)
  {
    switch (myData.myIntData.myIntType) {
    case INT_INT:
      *myData.myIntData.myIntPointer = val;
      break;
      
    case INT_SHORT:
      *myData.myIntData.myIntShortPointer = val;
      break;

    case INT_UNSIGNED_SHORT:
      *myData.myIntData.myIntUnsignedShortPointer = val;
      break;

    case INT_UNSIGNED_CHAR:
      *myData.myIntData.myIntUnsignedCharPointer = val;
      break;

    default:
      MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: int is bad type.", getName());
      if (errorBuffer != NULL)
	      snprintf(errorBuffer, errorBufferLen, "%s int is bad type (%d).", getName(), myData.myIntData.myIntType);
      return false;
    }
  }
  return true;

} // end method setInt



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for DOUBLE Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


MVREXPORT double MvrConfigArg::getDouble(bool *ok) const 
{
  if (ok != NULL) {
    *ok = (myType == DOUBLE);
  }

  if (myType == DOUBLE) {
    if (myData.myDoubleData.myDoublePointer != NULL)
      return *myData.myDoubleData.myDoublePointer; 
  }
  
  return 0;
}

/**
 * This method is applicable only when getType() returns DOUBLE.
 * @param ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return double the maximum value of this arg, or -HUGE_VAL if 
 * none
**/
MVREXPORT double MvrConfigArg::getMinDouble(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == DOUBLE);
  }
  if (myType == DOUBLE) {
    return myData.myDoubleData.myMinDouble;
  }
  return -HUGE_VAL;
}

/**
 * This method is applicable only when getType() returns DOUBLE.
 * @param ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return double the maximum value of this arg, or HUGE_VAL if 
 * none
**/
MVREXPORT double MvrConfigArg::getMaxDouble(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == DOUBLE);
  }
  if (myType == DOUBLE) {
    return myData.myDoubleData.myMaxDouble;
  }
  return HUGE_VAL;
}

MVREXPORT int MvrConfigArg::getDoublePrecision(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == DOUBLE);
  }
  if (myType == DOUBLE) {
    return myData.myDoubleData.myPrecision;
  }
  return 0;

} // end method getDoublePrecision


MVREXPORT bool MvrConfigArg::setDouble(double val, char *errorBuffer,
				                            size_t errorBufferLen, bool doNotSet)
{ 
  if (myType != DOUBLE) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::setDouble() cannot set %s to %g, wrong type (%s)",
               getName(), val, toString(myType));
    return false;
  }

  myValueSet = true;
  if (!myIgnoreBounds && val < myData.myDoubleData.myMinDouble)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setDouble value %g below range [%g, %g]", getName(), val, myData.myDoubleData.myMinDouble, myData.myDoubleData.myMaxDouble);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %g is below minimum of %g.", getName(), val, myData.myDoubleData.myMinDouble);
    return false;
  }
  if (!myIgnoreBounds && val > myData.myDoubleData.myMaxDouble)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setDouble value %g above range [%g, %g]", getName(), val, myData.myDoubleData.myMinDouble, myData.myDoubleData.myMaxDouble);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %g is above maximum of %g.", getName(), val, myData.myDoubleData.myMaxDouble);
    return false;
  }
  if (myData.myDoubleData.myDoublePointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setDouble called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }
  // if we got to here we're good
  if (!doNotSet)
    *myData.myDoubleData.myDoublePointer = val;
  return true;
}




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for BOOL Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MVREXPORT bool MvrConfigArg::getBool(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == BOOL);
  }
  if (myType == BOOL) {
    if (myData.myBoolData.myBoolPointer != NULL)
      return *myData.myBoolData.myBoolPointer;
  }
  return false;
}

MVREXPORT bool MvrConfigArg::setBool(bool val, char *errorBuffer,
				   size_t errorBufferLen, bool doNotSet)
{
  if (myType != BOOL) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::setBool() cannot set %s to %i, wrong type (%s)",
               getName(), val, toString(myType));
    return false;
  }

  myValueSet = true;
  if (myData.myBoolData.myBoolPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setBool called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }
  if (!doNotSet)
    *myData.myBoolData.myBoolPointer = val;
  return true;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for STRING and STRING_HOLDER Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MVREXPORT const char *MvrConfigArg::getString(bool *ok) const
{
  if (ok != NULL) {
    *ok = ((myType == STRING) || 
           (myType == STRING_HOLDER) ||
           (myType == CPPSTRING));
  }

  if ((myType == STRING) || (myType == STRING_HOLDER)) {
    // KMC own string change
    // if (myData.myStringData.myUsingOwnedString) {
    if (myOwnPointedTo) {
      if (myData.myStringData.myString != NULL) {
        return myData.myStringData.myString->c_str();
      }
      else {
        return "";
      }
    }
    else if (myData.myStringData.myStringPointer != NULL)
      return myData.myStringData.myStringPointer;
  } else if(myType == CPPSTRING) {
    return getCppString().c_str();
  }

  // KMC 7/9/12 Mvre we sure we want to return NULL and not ""?
  return NULL;
}

/**
    @warning The value must not contain any characters with
    special meaning when saved and loaded from a config file, such as '#', ';',
    tab, or newline.  
*/
MVREXPORT bool MvrConfigArg::setString(const char *str, char *errorBuffer,
				                             size_t errorBufferLen, bool doNotSet)
{

  if ((myType != STRING) && (myType != STRING_HOLDER)) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::setString() cannot set %s to %s, wrong type (%s)",
               getName(), str, toString(myType));
    return false;
  }

  myValueSet = true;
  size_t len;

  // KMC own string change
  // if (myData.myStringData.myUsingOwnedString)
  if (myOwnPointedTo)
  {
    if(myData.myStringData.myString)
      delete myData.myStringData.myString;
    myData.myStringData.myString = new std::string(str);
    return true;
  }
  if (myData.myStringData.myStringPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setString called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }
  // this is >= so that if it wouldn't have room with NULL that's
  // taken care of too
  if ((len = strlen(str)) >= myData.myStringData.myMaxStrLen)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setString called with argument %d long, when max length is %d.", getName(), len, myData.myStringData.myMaxStrLen);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s string is %lu long when max length is %lu.", getName(), len, myData.myStringData.myMaxStrLen);
    return false;
  }
  if (!doNotSet)
    strcpy(myData.myStringData.myStringPointer, str);
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for CPPSTRING Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// Get a copy of the value of the target std::string. (If there is a NULL
/// target, return "" and set @a ok to false if @a ok is not NULL.)
MVREXPORT std::string MvrConfigArg::getCppString(bool *ok) const
{
  if(myData.myCppStringData.myCppStringPtr == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "MvrConfigArg::getCppString: internal std::string pointer for argument %s is NULL!", getName());
    if(ok)
      *ok = false;
    return "";
  }
  if(ok)
    *ok = true;
  return *(myData.myCppStringData.myCppStringPtr);
}

/// Get the internal pointer to the target std::string. Do not modify. (If there is a NULL
/// target, return NULL and set @a ok to false if @a ok is not NULL.)
MVREXPORT const std::string* MvrConfigArg::getCppStringPtr(bool *ok) const
{
  if(myData.myCppStringData.myCppStringPtr == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "MvrConfigArg::getCppString: internal std::string pointer for argument %s is NULL!", getName());
    if(ok)
      *ok = false;
    return NULL;
  }
  if(ok)
    *ok = true;
  return myData.myCppStringData.myCppStringPtr;
}

MVREXPORT bool MvrConfigArg::setCppString(const std::string& str, char *errorBuffer, size_t errorBufferLen, bool doNotSet)
{
  if(myType != CPPSTRING)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg::setCppString() cannot set %s to %s, wrong type (%s)", getName(), str.c_str(), toString(myType));
    return false;
  }

  if(myOwnPointedTo && !doNotSet)
  {
    if(myData.myCppStringData.myCppStringPtr != NULL)
      delete myData.myCppStringData.myCppStringPtr;
    myData.myCppStringData.myCppStringPtr = new std::string(str);
    myValueSet = true;
    return true;
  }
  
  if(myData.myCppStringData.myCppStringPtr == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setCppString called but std::string target pointer is NULL.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }

  if(!doNotSet)
  {
    myData.myCppStringData.myCppStringPtr->assign(str);
    myValueSet = true;
  }
  
  return true;
}
  


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for LIST Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * If the given child arg is not a separator, then it must have a unique, 
 * non-empty name.
 * 
 * @param arg the MvrConfigArg to be added as a child of this composite arg
 * @return bool true if the child arg was successfully added; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::addArg(const MvrConfigArg &arg)
{
  // Children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType()) {

    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::addArg() child %s not added to %s (type %s), type must be %s or %s",
               arg.getName(),
               getName(),
               toString(getType()),
               toString(LIST),
               toString(LIST_HOLDER));
    return false;
  }

  // Children must own their own data.  They cannot store pointers to members of other
  // objects.
  if (arg.hasExternalDataReference()) {

    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::addArg() child %s not added to %s, child must own data",
               arg.getName(),
               getName());
    return false;
  }

  MvrConfigArg newChildArg(arg);

  MvrConfigArg *existingChildArg = NULL;
  if (!MvrUtil::isStrEmpty(arg.getName())) {
    existingChildArg = findArg(arg.getName());
  }

  // If the child has a name (i.e. is not a separator), then it must be unique.
  if (existingChildArg != NULL) {
    
    if (!existingChildArg->isPlaceholder() && existingChildArg->isSerializable()) {
      MvrLog::log(MvrLog::Normal,
                 "MvrConfigArg::addArg() child %s not added to %s, duplicate exists",
                 arg.getName(),
                 getName());
      return false;
    }
    else if (existingChildArg->getType() == LIST_HOLDER) {
      
      if (newChildArg.getArgCount() == 0) {

        IFDEBUG(MvrLog::log(MvrLog::Normal,
                           "MvrConfigArg::addArg() child %s promoted to list in %s",
                           arg.getName(),
                           getName()));
        existingChildArg->promoteList(arg);

        // This is slow and perhaps silly, but trying to eliminate argument 
        // ordering issues.
        newChildArg = *existingChildArg;

        if (!removeArg(*existingChildArg)) {
          MvrLog::log(MvrLog::Normal,
                     "MvrConfigArg::addArg() error removing existing list %s from %s",
                      existingChildArg->getName(),
                      getName());
        }

      }
    }
    else if (existingChildArg->getType() == STRING_HOLDER) {
      // KMC Replace the existing child arg with the new arg, but keep its value.
      MvrArgumentBuilder holderBuilder;
      holderBuilder.add(existingChildArg->getString());

      //MvrConfigArg newChildArg(arg);

      bool isParseSuccessful = newChildArg.parseArgument(&holderBuilder, 
				                                                 NULL,
                                                         0); 

      if (isParseSuccessful) {  

        if (!removeArg(*existingChildArg)) {
          MvrLog::log(MvrLog::Normal,
                     "MvrConfigArg::addArg() error removing child %s from %s",
                     newChildArg.getName(),
                     getName());
        }
      }
      else {
        MvrLog::log(MvrLog::Normal,
                   "MvrConfigArg::addArg() child %s not added to %s, error parsing '%s'",
                    arg.getName(),
                    getName(),
                    existingChildArg->getString());
        return false;
      } 
    }
    else if (!existingChildArg->isSerializable()) {

      // Well, the idea here is that we should allow a non-serializable
      // parameter to simply be overwritten.  We won't be reading any
      // updated values from the configuration file.

      bool isValueSet = existingChildArg->setValue(newChildArg);
      MvrLog::log(MvrLog::Normal,
                 "MvrConfigArg::addArg() child %s not serializable, setValue returns %i",
                 existingChildArg->getName(),
                 isValueSet);
      return isValueSet;
    }
  }

  // If the member list has not yet been allocated, then create it.
  if (myData.myListData.myChildArgList == NULL) {
    
    myData.myListData.myChildArgList = new std::list<MvrConfigArg>();

    // Check for memory allocation errors.
    if (myData.myListData.myChildArgList == NULL) {
      MvrLog::log(MvrLog::Normal,
                 "MvrConfigArg::addArg() memory allocation error");      
      return false;
    }

  } // end if child list not yet created


  IFDEBUG(MvrLog::log(MvrLog::Normal,
                     "MvrConfigArg::addArg() adding %s to %s",
                      newChildArg.getName(),
                      getName()));

  myData.myListData.myChildArgList->push_back(newChildArg);
  // Modify the parent of the arg that has actually been stored.
  myData.myListData.myChildArgList->back().setParent(this);

  return true;

} // end method addArg
  

MVREXPORT bool MvrConfigArg::removeArg(const MvrConfigArg  &arg)
{
  // Children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType()) {

    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::removeArg() child %s not removed from %s (type %s), type must be %s or %s",
               arg.getName(),
               getName(),
               toString(getType()),
               toString(LIST),
               toString(LIST_HOLDER));
    return false;
  }

  if (MvrUtil::isStrEmpty(arg.getName())) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::removeArg() child not removed from %s, empty name",
               getName());
    return false;
  }
  
  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       iter !=myData.myListData.myChildArgList->end(); 
       iter++) {
    if (MvrUtil::strcasecmp((*iter).getName(), arg.getName()) == 0) {
      myData.myListData.myChildArgList->erase(iter);
      return true;
    }
  }

  return false; // Not found
}


/**
 * @return bool set to true if this is a list arg that contains child args; false, otherwise
 **/
MVREXPORT bool MvrConfigArg::hasArgs() const
{
  // Children can only be added to LIST or LIST_HOLDER type parameters
  if ((!isListType()) ||
      (myData.myListData.myChildArgList == NULL)) {
    return 0;
  }
  return !myData.myListData.myChildArgList->empty();
}

/**
 * @return int the number of child args in this list arg; or 0 if none.
**/
MVREXPORT size_t MvrConfigArg::getArgCount() const          
{
  // Children can only be added to LIST or LIST_HOLDER type parameters
  if ((!isListType()) ||
      (myData.myListData.myChildArgList == NULL)) {
    return 0;
  }
  return myData.myListData.myChildArgList->size();

} // end method getArgCount


MVREXPORT size_t MvrConfigArg::getDescendantArgCount() const
{
  // Children can only be added to LIST type parameters
  if ((myType != LIST) ||
      (myData.myListData.myChildArgList == NULL)) {
    return 0;
  }
  int count = 0;
  for (std::list<MvrConfigArg>::const_iterator iter = myData.myListData.myChildArgList->begin();
       iter != myData.myListData.myChildArgList->end();
       iter++) {
    // The child counts as a descendent
    count++;
    // And so do the child's descendents
    count += (*iter).getDescendantArgCount();
  } // end for each arg

  return count;

} // end method getDescendantArgCount


/**
 * @return std::list<MvrConfigArg> a list of all of the child arg (copies) 
 * in this arg.
**/
MVREXPORT std::list<MvrConfigArg> MvrConfigArg::getArgs(bool *ok) const
{
  std::list<MvrConfigArg> retList;
 
  if (ok != NULL) {
    *ok = isListType();
  }
  if ((isListType()) &&
      (myData.myListData.myChildArgList != NULL)) {
    retList = *myData.myListData.myChildArgList;
  }
  return retList;

} // end method getArgs

  
/**
 * @return std::list<MvrConfigArg> a list of all of the child arg (copies) 
 * in this arg.
**/
MVREXPORT const MvrConfigArg *MvrConfigArg::getArg(size_t index) const
{
  MvrConfigArg *arg = NULL;
  if ((isListType()) &&
      (myData.myListData.myChildArgList != NULL)) {
    size_t curIndex = 0;
    for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
         iter != myData.myListData.myChildArgList->end();
         iter++, curIndex++) {
      if (curIndex == index) {
        arg = &(*iter);
        break;
      }
    }
  }

  return arg;

} // end method getArg

MVREXPORT MvrConfigArg *MvrConfigArg::getArg(size_t index) 
{
  MvrConfigArg *arg = NULL;
  if ((isListType()) &&
      (myData.myListData.myChildArgList != NULL)) {
    size_t curIndex = 0;
    for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
         iter != myData.myListData.myChildArgList->end();
         iter++, curIndex++) {
      if (curIndex == index) {
        arg = &(*iter);
        break;
      }
    }
  }

  return arg;

} // end method getArg


/**
 * This method performs a case-insensitive search of the child arg names.
 *
 * @param childParamName the char * name of the child arg to be retrieved; must be
 * non-empty
 * @return MvrConfigArg * a pointer to the requested child arg; or NULL if not 
 * found.
**/
MVREXPORT const MvrConfigArg *MvrConfigArg::findArg(const char *childParamName)  const
{
  // Cannot look up empty name 
  if (MvrUtil::isStrEmpty(childParamName)) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::findArg() cannot find child with empty name in arg %s",
               getName());
    return NULL;
  }
   // Children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType()) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::findArg() cannot find child %s in arg %s (type %s), type must be %s or %s",
               childParamName,
               getName(),
               toString(getType()),
               toString(LIST),
               toString(LIST_HOLDER));
    return NULL;
  }
  
  // If empty list, return NULL
  if (myData.myListData.myChildArgList == NULL) {
    // This isn't really an error worth logging
    return NULL;
  }

  MvrConfigArg *child = NULL;

  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       ((iter != myData.myListData.myChildArgList->end()) && (child == NULL));
       iter++) {
    MvrConfigArg &curArg = *iter;
    if (!MvrUtil::isStrEmpty(curArg.getName()) &&
        (MvrUtil::strcasecmp(childParamName, curArg.getName()) == 0)) {
      child = &curArg;
    } // end if name matches
  } 
  return child;

} // end method findArg

/**
 * This method performs a case-insensitive search of the child arg names.
 *
 * @param childParamName the char * name of the child arg to be retrieved; must be
 * non-empty
 * @return MvrConfigArg * a pointer to the requested child arg; or NULL if not 
 * found.
**/
MVREXPORT MvrConfigArg *MvrConfigArg::findArg(const char *childParamName)
{
  // Cannot look up empty name 
  if (MvrUtil::isStrEmpty(childParamName)) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::findArg() cannot find child with empty name in arg %s",
               getName());
    return NULL;
  }
   // Children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType()) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::findArg() cannot find child %s in arg %s (type %s), type must be %s or %s",
               childParamName,
               getName(),
               toString(getType()),
               toString(LIST),
               toString(LIST_HOLDER));
    return NULL;
  }
  
  // If empty list, return NULL
  if (myData.myListData.myChildArgList == NULL) {
    // This isn't really an error worth logging
    return NULL;
  }

  MvrConfigArg *child = NULL;

  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       ((iter != myData.myListData.myChildArgList->end()) && (child == NULL));
       iter++) {
    MvrConfigArg &curArg = *iter;
    if (!MvrUtil::isStrEmpty(curArg.getName()) &&
        (MvrUtil::strcasecmp(childParamName, curArg.getName()) == 0)) {
      child = &curArg;
    } // end if name matches
  } 
  return child;

} // end method findArg


MVREXPORT bool MvrConfigArg::getAncestorList
                            (std::list<MvrConfigArg*> *ancestorListOut) 
{
 
  if (getParentArg() == NULL) {
    return false;
  }
  if (ancestorListOut == NULL) {
    return true; // parent arg, but no work to do
  }
  ancestorListOut->push_front(this);

  MvrConfigArg *parentParam = getParentArg();

  ancestorListOut->push_front(parentParam);

  while (parentParam->getParentArg() != NULL) {
    ancestorListOut->push_front(parentParam->getParentArg());
    parentParam = parentParam->getParentArg();
  } 
  return true;

} // end method getAncestorArgs


MVREXPORT const MvrConfigArg *MvrConfigArg::getTopLevelArg() const
{
  MvrConfigArg *parentArg = getParentArg();
  if (parentArg == NULL) {
    return this;
  }
  while (parentArg->getParentArg() != NULL) {
    parentArg = parentArg->getParentArg();
  }
  return parentArg;

}

  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods for FUNCTOR Type 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MVREXPORT const std::list<MvrArgumentBuilder *> *MvrConfigArg::getArgsWithFunctor(bool *ok) const
{
  if (ok != NULL) {
    *ok = (myType == FUNCTOR);
  }
  if (myType == FUNCTOR) {
    if (myData.myFunctorData.myGetFunctor != NULL)
      return myData.myFunctorData.myGetFunctor->invokeR();
  }
  return NULL;
}

MVREXPORT bool MvrConfigArg::setArgWithFunctor(MvrArgumentBuilder *argument, 
					     char *errorBuffer,
					     size_t errorBufferLen, 
					     bool doNotSet)
{
  if (myType != FUNCTOR) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::setArgWithFunctor() cannot set %s, wrong type (%s)",
               getName(), toString(myType));
    return false;
  }

  myValueSet = true;
  bool ret = true;
  if (myData.myFunctorData.mySetFunctor == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setArgWithFunctor called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }
  if (!doNotSet)
    ret = myData.myFunctorData.mySetFunctor->invokeR(argument);
  return ret;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// General Methods (for all types) 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
/**
 * @return MvrConfigArg * the parent arg, if this is a child in a list arg; 
 * otherwise, NULL
**/
MVREXPORT MvrConfigArg *MvrConfigArg::getParentArg() const
{
  return myParentArg;

} // end method getParentArg

std::string MvrConfigArg::getParentPathName(char separator) const
{
  std::string parentPath;
  MvrConfigArg *curParent = myParentArg;
  while (curParent != NULL) {
    if (!parentPath.empty()) {
      parentPath = separator + parentPath;
    }
    parentPath = curParent->getName() + parentPath;

    curParent = curParent->getParentArg();
  }

  return parentPath;
}

MVREXPORT std::list<std::string> MvrConfigArg::splitParentPathName(const char *parentPathName,
                                                                 char separator)
{
  std::list<std::string> pathList;
  if (MvrUtil::isStrEmpty(parentPathName)) {
    return pathList;
  }

  MvrArgumentBuilder builder(512, 
                             separator);
  builder.add(parentPathName);

  for (int c = 0; c < builder.getArgc(); c++) {
    IFDEBUG(MvrLog::log(MvrLog::Normal,
                       "MvrConfigArg::splitParentPathName() %s - adding %s to list",
                       parentPathName, builder.getArg(c)));
    pathList.push_back(builder.getArg(c));
  }

  return pathList;

} // end method splitParentPathName

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// File Parsing
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * For normal args, this method simply adds the parserCB handler for the keyword
 * (i.e. arg name).  For list args, this method adds the parserCB handler
 * for all child arg names.  The list arg itself is handled by MvrConfig using 
 * the special keywords (_beginList and _endList).
 * 
 * @param parser the MvrFileParser * to which to add the handler
 * @param parserCB the MvrFunctor * that will parse the arg when the keyword is
 * found
 * @param logPrefix the char * prefix to use in debug log messages 
 * @param isQuiet a bool set to true if minimal log messages should be generated;
 * false if verbosity is desired.
 * @return bool true if the hanlder was successfully added to the parser; false 
 * if an error occurred
 * 
**/
MVREXPORT bool MvrConfigArg::addToFileParser(MvrFileParser *parser,
                                           MvrRetFunctor3C<bool, 
                                                          MvrConfig, 
                                                          MvrArgumentBuilder *, 
                                                          char *, 
                                                          size_t> *parserCB,
                                           const char *logPrefix,
                                           bool isQuiet) const
{
  if ((parser == NULL) || (logPrefix == NULL)) {
    return false;
  }

  bool isSuccess = true;

  if (!isListType()) {

    if (!parser->addHandlerWithError(getName(), parserCB)) {
      if (!isQuiet) {
        MvrLog::log(MvrLog::Verbose, 
		                "%sCould not add keyword %s (probably unimportant)",
                    logPrefix,
		                getName());
      }
    }
  }
  // Otherwise, it's a list.  These are handled in MvrConfig by the special 
  // _beginList and _endList keywords.  The list members need to be added to
  // the parser however.
  else if (myData.myListData.myChildArgList != NULL) {
     
    for (std::list<MvrConfigArg>::const_iterator iter = 
                                      myData.myListData.myChildArgList->begin();
         iter != myData.myListData.myChildArgList->end();
         iter++) {

      if (!(*iter).addToFileParser(parser,
                                   parserCB,
                                   logPrefix,
                                   isQuiet)) {
        isSuccess = false;
      }
    } // end for each list member 

  } // end else list

  return isSuccess;

} // end method addToFileParser


/**

  @internal
 * 
 * @return bool true if the arg was successfully parsed; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::parseArgument(
	MvrArgumentBuilder *arg, 
	char *errorBuffer,
	size_t errorBufferLen,
	const char *logPrefix,
	bool isQuiet, 
	bool *changed)
{
  if ((arg == NULL) || 
      (logPrefix == NULL) ||  
      ( (errorBuffer != NULL) && (errorBufferLen < 0) )) {
    MvrLog::log(MvrLog::Normal,
      "MvrConfigArg::parseArgument() invalid input");
    return false;
  }

  if ( (arg->getArg(0) == NULL) &&
       ( (getType() != MvrConfigArg::STRING) &&
         (getType() != MvrConfigArg::FUNCTOR) ) ) {

      if (!isQuiet) {
        MvrLog::log(MvrLog::Verbose, "%sparameter '%s' has no argument.",
          logPrefix,
          getName());
      }
      // continue;
      return true;
  }

  // this controls if we print out what changed
  bool printing = false;
  bool ok = true;

  switch (getType()) {

    // MPL added the string holder on 2/27 to get rid of the
    // unknown type message
  case DESCRIPTION_HOLDER:
  case SEPARATOR:
  case STRING_HOLDER:
  // KMC 4/17/13 Not entirely sure about the LIST_HOLDER but it 
  // seems consistent with the others
  case LIST_HOLDER: 
    {
      // There's no need to read anything for these types
    }
    break;

  case INT:
    {
      int origInt = getInt();
      int valInt = arg->getArgInt(0, &ok);
      if (ok) {
        ok = setInt(valInt, errorBuffer, errorBufferLen);

        // KMC 8/5/13 Think that these checks should be in the "if ok" block.
  	    if (origInt != getInt() && changed != NULL && !mySuppressChanges)
        {
          if (printing)
            MvrLog::log(MvrLog::Normal, "%sParameter %s (int) changed from %d to %d",
                 logPrefix, getName(), origInt, getInt());

          *changed = true;
        }

        if (ok) {
          IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                             "%sSet parameter '%s' to '%d'",
                             logPrefix, getName(), valInt));
        }
        else { // error setting int 
          // KMC 7/11/12 Why isn't this condition worthy of the error buffer?
          // And should it really be verbose??  Seems like a potential problem.
          MvrLog::log(MvrLog::Verbose, 
                     "%sCould not set parameter '%s' to '%d'",
                     logPrefix, getName(), valInt);
        }
      }
      else { // error parsing int
        if (errorBuffer != NULL) {
          snprintf(errorBuffer, errorBufferLen, 
                   "%s is an integer parameter but was given non-integer argument of '%s'", 
                    getName(), arg->getArg(0));
        }
      } // end else error parsing int
    }
    break;

  case DOUBLE:
    {
      double origDouble = getDouble();
      double valDouble = arg->getArgDouble(0, &ok);

      if (ok) {
        ok = setDouble(valDouble, errorBuffer, errorBufferLen);

        if (fabs(origDouble-getDouble()) > MvrMath::epsilon() && 
            changed != NULL && !mySuppressChanges)
        {
          if (printing)
            MvrLog::log(MvrLog::Normal, "%sParameter %s (double) changed from %g to %g",
                 logPrefix, getName(), origDouble, getDouble());

          *changed = true;
        }

        if (ok) {
          IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                             "%sSet parameter '%s' to '%.10f'",
                             logPrefix, getName(), valDouble));
        }
        else { // error setting double

          MvrLog::log(MvrLog::Verbose, 
                     "%sCould not set parameter '%s' to '%.10f'",
                     logPrefix, getName(), valDouble);
        }
      }
      else { // not a double

        MvrLog::log(MvrLog::Terse, 
                   "%sparameter '%s' is a double parameter but was given non-double argument of '%s'", 
                   logPrefix, getName(), arg->getArg(0));

        if (errorBuffer != NULL) {
          snprintf(errorBuffer, errorBufferLen, 
                   "%s is a double parameter but was given non-double argument of '%s'", 
                   getName(), arg->getArg(0));
        }
      } // end else not a double
    }
    break;

  case BOOL:
    {
      bool origBool = getBool();
      bool valBool = arg->getArgBool(0, &ok);
      if (ok) {
        ok = setBool(valBool, errorBuffer, errorBufferLen);

        if (origBool != getBool() && changed != NULL && !mySuppressChanges)
        {
          if (printing)
            MvrLog::log(MvrLog::Normal, "%sParameter %s (bool) changed from %s to %s",
                 logPrefix, getName(), 
                 MvrUtil::convertBool(origBool), MvrUtil::convertBool(getBool()));
          *changed = true;
        }

        if (ok) {
          IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                             "%sSet parameter '%s' to %s",
                             logPrefix, getName(), valBool ? "true" : "false" ));
        }
        else  { // error setting bool
          MvrLog::log(MvrLog::Verbose, 
                     "%sCould not set parameter '%s' to %s",
                     logPrefix, getName(), valBool ? "true" : "false" );
        } // end else error setting bool
      }
      else { // error parsing bool

        MvrLog::log(MvrLog::Terse, 
                  "%sparameter '%s' is a bool parameter but was given non-bool argument of '%s'", 
                  logPrefix, getName(), arg->getArg(0));
 
        if (errorBuffer != NULL) {
          snprintf(errorBuffer, errorBufferLen, 
                   "%s is a bool parameter but was given non-bool argument of '%s'", 
                   getName(), arg->getArg(0));
        }
      } // end else error parsing bool
    }
    break;

  case STRING:
    {
      std::string origString = getString();
      ok = setString(arg->getFullString());

      if (MvrUtil::strcmp(origString, getString()) != 0 && changed != NULL)
      {
        if (printing)
          MvrLog::log(MvrLog::Normal, "%sParameter %s (string) changed from '%s' to '%s'",
                     logPrefix, getName(), 
                     origString.c_str(), getString());
      
        *changed = true;
      }
	

      if (ok) {

        IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                           "%sSet parameter string '%s' to '%s'",
                           logPrefix,
                           getName(), getString()));

      }
      else { // error setting setring  
        MvrLog::log(MvrLog::Verbose, 
                   "%sCould not set string parameter '%s' to '%s'",
                   logPrefix,
                   getName(), getString());

        // KMC 7/11/12 Looks like this intentionally won't overwrite an existing error
        // message (but other types do).  Why the inconsistency?
        if (errorBuffer != NULL && errorBuffer[0] == '\0')
          snprintf(errorBuffer, errorBufferLen, 
                   "%s could not be set to '%s'.", 
                   getName(), arg->getFullString());

      } // end else error setting string 
    }
    break;

  case LIST:
    {
       MvrLog::log(MvrLog::Normal, 
                  "%sReceived LIST arg '%s' with '%s'",   
                  logPrefix, getName(), arg->getFullString());

      // MPL 9/6/12 for KMC The child count doesn't seem to do
      // anything here, if it does something elsewhere or it does
      // something later please set up something to set the changed
      // bool poitner like the other entries in here.
      
       int childCount = arg->getArgInt(0, &ok);
       if (ok) {
      
       }
       else { // error parsing int
         if (errorBuffer != NULL) {
           snprintf(errorBuffer, errorBufferLen, 
                   "%s is an list parameter but was given non-integer child count argument of '%s'", 
                    getName(), arg->getArg(0));
         }
       } // end else error parsing int
    }
    break;

  case FUNCTOR:
    {
      ok = setArgWithFunctor(arg);
      
      // MPL 9/6/12 we can't really tell if functor arguments
      // changed but we shouldn't be using them anymore... so just
      // assume that it's changed 
      if (changed != NULL)
      {
        *changed = true;
        MvrLog::log(MvrLog::Verbose, 
             "%sAssuming arg '%s' changed because it's a functor MvrConfigArg",
             logPrefix, getName());
      }
      
      if (ok) {
        IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                           "%sSet arg '%s' with '%s'",
                           logPrefix, getName(), arg->getFullString()));
        
      }
      else { // error setting from functor

        MvrLog::log(MvrLog::Verbose, 
                   "MvrConfig: Could not set parameter '%s' to '%s'",
                   logPrefix,
                   getName(), arg->getFullString());
        
        // KMC 7/11/12: This comment implies that the error buffer is being passed
        // to setArgWithFunctor (but this does not seem to be happening). Why?
        //
        // if it didn't put in an error message make one
        if (errorBuffer != NULL && errorBuffer[0] == '\0') {
          snprintf(errorBuffer, errorBufferLen, 
                   "%s could not be set to '%s'.", 
                   getName(), arg->getFullString());
        }
      }
    }
    break;

  case CPPSTRING:
    {
      std::string origString = getCppString();
      ok = setCppString(arg->getFullString());
      if(origString != getCppString() && changed != NULL)
      {
        if(printing)  
            MvrLog::log(MvrLog::Normal, "%sParameter %s (cppstring) changed from '%s' to '%s'",
                       logPrefix, getName(), 
                       origString.c_str(), getCppString().c_str());
            *changed = true;
      }
      if (ok) {
        IFDEBUG(MvrLog::log(MvrLog::Verbose, 
                           "%sSet parameter string '%s' to '%s'",
                           logPrefix,
                           getName(), getString()));
      }
      else { // error setting string  
        MvrLog::log(MvrLog::Verbose, 
                   "%sCould not set cppstring parameter '%s' to '%s'",
                   logPrefix,
                   getName(), getCppString().c_str());
        if (errorBuffer != NULL)
          snprintf(errorBuffer, errorBufferLen, 
                   "%s could not be set to '%s'.", getName(), arg->getFullString());
      }
    } 
    break;

  default:
    {
      // While this seems like it would be an error condition, the original code
      // did not set retFlag to false.
      //
      MvrLog::log(MvrLog::Verbose, 
                 // KMC 7/11/12 Is the section information really necessary?  
                 "%sWarning: Don't know the argument type for '%s' in section, got string '%s'", //, in section '%s'.", 
                 logPrefix, arg->getExtraString(), arg->getFullString());
                 // , mySection.c_str());
    }

  } // end switch type

  return ok;

} // end method parseArgument


/**
 * @param file the FILE * to be written, must be non-NULL
 * @param lineBuf a char array to be used as a temporary write buffer,
 * must be non-NULL
 * @param lineBufSize the int number of chars in lineBuf, must be positive
 * @param startCommentColumn the int column index at which to indent
 * the arg description / comment
 * @param isWriteExtra a bool set to true if the arg priority, display
 * hint, restart level and other later extras should also be written
 * to the file
 * @param logPrefix the char * prefix to use in debug log messages, must be
 * non-NULL
 * @param indentLevel the int level of indentation for the arg (children
 * of lists are indented further); must be non-negative
 * @return bool true if the arguments were successfully written; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::writeArguments(FILE *file,
                                          char *lineBuf,
                                          int lineBufSize,
                                          int startCommentColumn,
                                          bool isWriteExtra,
                                          const char *logPrefix,
                                          int indentLevel) const
{
  if ((file == NULL) || 
      (lineBuf == NULL) || 
      (lineBufSize <= 0) || 
      (logPrefix == NULL) ||
      (indentLevel < 0)) {
    MvrLog::log(MvrLog::Normal, 
               "MvrConfigArg::writeArguments() invalid input");
    return false;
  }
  if (!isSerializable()) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::writeArguments() skipping non-serializable parameter %s",
               getName());
    return true; // no error occurred     
  }

  lineBuf[0] = '\0';

  //char tempBuf[1024];

  // holds the fprintf
  char startLine[128];
  startLine[0] = '\0';

  char nameBuf[256];
  nameBuf[0] = '\0';

  char indentBuf[128];
  indentBuf[0] = '\0';
  snprintf(indentBuf, sizeof(indentBuf),
           "%*s", (indentLevel * ourIndentSpaceCount), "");

  // if the type is a functor then we need to handle all of it up
  // here since its a special case both in regards to comments and values
  if (getType() == MvrConfigArg::FUNCTOR)
  {
    // put the comments in the file first
    int nextChar = snprintf(lineBuf, lineBufSize, "; ");

    if (!MvrUtil::isStrEmpty(getDescription())) {
      writeMultiLineComment(getDescription(),
                            file,
                            lineBuf, 
                            lineBufSize,
                            startLine);
    } // end if description

    std::list<MvrArgumentBuilder *>::const_iterator argIt;
    const std::list<MvrArgumentBuilder *> *argList = getArgsWithFunctor();
    if (argList != NULL) {
      for (argIt = argList->begin(); argIt != argList->end(); argIt++)
      {
        // if there's a space in the name then quote the param name
        if (strchr(getName(), ' ') != NULL || 
            strchr(getName(), '\t') != NULL)
          fprintf(file, "\"%s\" %s\n", getName(), 
          (*argIt)->getFullString());
        else
          fprintf(file, "%s %s\n", getName(), 
          (*argIt)->getFullString());
      }
    } // end if arg list is not null

    return true;
    
  } // end if special FUNCTOR type param

  // If this code is reached, the param is not a special FUNCTOR type

    //param->writeArguments(lineBuf, lineBufSize, logPrefix);



  // ----------------------------- HERE
  //// if its a string holder just write the name without quotes
  //// (since its the value not really the name)
  //if (param->getType() == MvrConfigArg::STRING_HOLDER)
  //{
  //  fprintf(file, "%s %s\n", param->getName(), param->getString());
 
  //  continue;
  //}

  // Write the param name...

  // If there's a space in the name, then surround it by quotes.
  // Unless this is a string holder.  In that case, "its the value
  // not really the name".  
  writeName(nameBuf, sizeof(nameBuf),
            ((!isListType()) ? indentLevel : 0));


  // Write the param value...
  switch (getType()) {

  case INT:
    {
      snprintf(lineBuf, lineBufSize,
                "%s %d", nameBuf, getInt());
    }
    break;

  case DOUBLE:
    {
      // Easier to debug...
      double d = getDouble();
      int p = getDoublePrecision();

      if (p >= 0) {
        snprintf(lineBuf, lineBufSize,
                 "%s %.*f", nameBuf, p, d);
      }
      else { // Old-style, inconsistent precision...
        snprintf(lineBuf, lineBufSize,
                 "%s %g", nameBuf, d);
      }
    }
    break;

  case BOOL:
    {
      snprintf(lineBuf, lineBufSize,
               "%s %s", nameBuf, getBool() ? "true" : "false"); 
    }
    break;

  case STRING:
  case STRING_HOLDER:
    {
      snprintf(lineBuf, lineBufSize,
               "%s %s", nameBuf, getString());
    }
    break;

  case CPPSTRING:
    {
      snprintf(lineBuf, lineBufSize, 
          "%s %s", nameBuf, getCppString().c_str());
    }
    break;

  case LIST:
  case LIST_HOLDER:
    {
      snprintf(lineBuf, lineBufSize,
               "%s%s %s", 
               indentBuf, LIST_BEGIN_TAG, nameBuf);


    } 
    break;

  case DESCRIPTION_HOLDER:
    {
      //if (strlen(param->getDescription()) == 0)
      //{
      //  fprintf(file, "\n");
      //  continue;
      //}
      // ????
    }
    break;
    // KMC 7/11/12: Seems like it must be possible to work this 
    // into the existing framework, maybe return later
  //case FUNCTOR:
  //  {
  //    std::list<MvrArgumentBuilder *>::const_iterator argIt;
  //    const std::list<MvrArgumentBuilder *> *argList = param->getArgsWithFunctor();
  //    if (argList != NULL) {
  //      for (argIt = argList->begin(); argIt != argList->end(); argIt++)
  //      {
  //        // if there's a space in the name then quote the param name
  //        if (strchr(param->getName(), ' ') != NULL || 
  //            strchr(param->getName(), '\t') != NULL)
  //          fprintf(file, "\"%s\" %s\n", param->getName(), 
  //          (*argIt)->getFullString());
  //        else
  //          fprintf(file, "%s %s\n", param->getName(), 
  //          (*argIt)->getFullString());
  //      }
  //    } // end if arg list is not null
  //  } // 
  //  break;

  default:
    {
      MvrLog::log(MvrLog::Terse, "%s in writeArguments(): unhandled argument type %s for config arg %s", logPrefix, toString(getType()), getName());
    }
    break;
  } // end switch type
  
  // Make sure there's no overrun
  lineBuf[lineBufSize - 1] = '\0';

  if (getType() == STRING_HOLDER) {
    fprintf(file, "%s\n", lineBuf);
    return true;
  }

  // configure our start of line part
  if  (getType() == MvrConfigArg::DESCRIPTION_HOLDER) {
    sprintf(startLine, "; %%s");
    //sprintf(startLine, "; %%s");
  }
  else { // KMC if (getType() != MvrConfigArg::STRING_HOLDER)
    sprintf(startLine, "%%-%ds;", startCommentColumn);
  }
  

   // KMC 8/3/12 IMPORTANT NOTE:  While it would be extremely nice 
   // to prevent buffer overruns by using snprintf instead of sprintf,
   // it is currently ill-advised.  On Linux, the two methods behave
   // differently when the line buffer is used as both the target buffer
   // and as formatting input (as in all the prints below). It yields 
   // the desired results with sprintf, but substitutes an empty string
   // for the formatting input with snprintf.  This, in turn, causes
   // the output config data to be garbage. (On Windows, there is no 
   // problem.)  

  // if our line is already longer then where we want to go put in
  // an extra space
  if (strlen(lineBuf) >= startCommentColumn) {
    sprintf(lineBuf, "%s ;", lineBuf);
  }
  // if its not then just put the start in
  // KMC 1/22/13 Added the test for isStrEmpty.  For unknown reasons, if
  // the lineBuf is empty, then the sprintf results in garbage being 
  // printed.  (See Bug 12735).  If, however, the empty string is directly
  // passed -- as in the else clause below -- then there isn't a problem.
  else if (!MvrUtil::isStrEmpty(lineBuf)) {
    sprintf(lineBuf, startLine, lineBuf);
  }
  else {
    sprintf(lineBuf, startLine, "");
  } 


  // KMC I think that this might be an issue.  If the comment
  // should appear right after the bounds.  Otherwise, 
  writeBounds(lineBuf, lineBufSize, logPrefix);

  
  // if we have a description to put in, put it in with word wrap
  if (!MvrUtil::isStrEmpty(getDescription()))
  {
    writeMultiLineComment(getDescription(),
                          file,
                          lineBuf, 
                          lineBufSize,
                          startLine);
  }
  // else no description, just end the line
  else {
    fprintf(file, "%s\n", lineBuf);

  }
  lineBuf[0] = '\0';

  // For list types, indent and write their children.  This is done
  // outside the main switch statement so that the description can 
  // remain with the list intro.
  //
  if (isListType()) {

    if (myData.myListData.myChildArgList != NULL) {
      for (std::list<MvrConfigArg>::const_iterator cIter = myData.myListData.myChildArgList->begin();
            cIter != myData.myListData.myChildArgList->end();
            cIter++) {

        (*cIter).writeArguments(file,
                                lineBuf,
                                lineBufSize,
                                startCommentColumn,
                                isWriteExtra,
                                logPrefix,
                                indentLevel + 1);

      } // end for each child arg
    } // end if child arg list

    lineBuf[0] = '\0';

    writeName(lineBuf, lineBufSize, 
              ((isListType()) ? indentLevel : 0));

    snprintf(lineBuf, lineBufSize,
              "%s%s %s", 
              indentBuf, LIST_END_TAG, nameBuf);

    fprintf(file, "%s\n", lineBuf);
    lineBuf[0] = '\0';

  } // end if list



  // if they have a config priority put that on its own line
  if (isWriteExtra)
  {
    sprintf(lineBuf, startLine, "");
    fprintf(file, "%s Priority: %s\n", lineBuf,
      MvrPriority::getPriorityName(getConfigPriority()));
    lineBuf[0] = '\0';

    sprintf(lineBuf, startLine, "");
    fprintf(file, "%s DisplayHint: %s\n", lineBuf, getDisplayHint());

    lineBuf[0] = '\0';

    sprintf(lineBuf, startLine, "");
    fprintf(file, "%s RestartLevel: %d\n", lineBuf, getRestartLevel());

    lineBuf[0] = '\0';
    
  }

  return true;

} // end method writeArguments


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sockets
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      

bool MvrConfigArg::isNone(const char *argText) 
{
  if (MvrUtil::isStrEmpty(argText)) {
    return true;
  }
  else if (MvrUtil::strcasecmp(argText, "None") == 0) {
    return true;
  }
  return false;

} // end method isNone

MVREXPORT bool MvrConfigArg::parseSocket(const MvrArgumentBuilder &args,
                                       char *errorBuffer,
                                       size_t errorBufferLen)
{

  MvrConfigArg configArg;
  bool ok = true;

  // This is a redundant check. The MVRCL command handler should have verified the count
  // before calling this method and will display a better error message.
  if (args.getArgc() <=  SOCKET_INDEX_OF_TYPE) {
    if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
      snprintf(errorBuffer, errorBufferLen,
               "Insufficient arguments");
    }
    return false;

  } // end if too few args
 
  std::string name = args.getArg(SOCKET_INDEX_OF_ARG_NAME);

  if (name.empty())
  {
    if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
      snprintf(errorBuffer, errorBufferLen,
               "Syntax: <name> was empty, it must have a value.");
    }
    return false;

  } // end if name empty


  std::string description = args.getArg(SOCKET_INDEX_OF_DESCRIPTION);
  // Okay for the description to be empty
  
  std::string priorityText = args.getArg(SOCKET_INDEX_OF_PRIORITY);
  
  MvrPriority::Priority priority = MvrPriority::getPriorityFromName
                                                (priorityText.c_str(),
                                                 &ok);

  if (!ok) {

    if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
      snprintf(errorBuffer, errorBufferLen,
               "Priority was '%s' but can only be one of Basic, Intermediate, Advanced, Expert, Factory, Calibration",
	             args.getArg(SOCKET_INDEX_OF_PRIORITY));
    }
    return false;

  } // end if priority not parsed


  std::string typeText = args.getArg(SOCKET_INDEX_OF_TYPE);

  MvrConfigArg::Type type = typeFromString(typeText.c_str());

  
  switch (type) {

  case INT:
  {
    int defaultInt = -1; 
    int minInt = INT_MIN;
    int maxInt = INT_MAX;
    
    if (args.getArgc() > SOCKET_INDEX_OF_VALUE) {
    
      defaultInt = args.getArgInt(SOCKET_INDEX_OF_VALUE, &ok);

      if (!ok) {
        if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
          snprintf(errorBuffer, errorBufferLen,
 	                 "type was int, but default value was '%s' when it should be an int",
	                 args.getArg(SOCKET_INDEX_OF_VALUE));
        }
        return false;

      } // end if error parsing int 
    }
    else { // insufficient args
    
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
  	             "type was int, but no default value was given");
      }
      return false;

    } // end else insufficient args

    
    if (args.getArgc() > SOCKET_INDEX_OF_MIN_VALUE)
    {
      if (!isNone(args.getArg(SOCKET_INDEX_OF_MIN_VALUE))) {
	
        minInt = args.getArgInt(SOCKET_INDEX_OF_MIN_VALUE, &ok);

        if (!ok) {
          if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
            snprintf(errorBuffer, errorBufferLen,
                     "type was int, but min was '%s' when it should be an int or 'None'",
                     args.getArg(SOCKET_INDEX_OF_MIN_VALUE));
          }
          return false;

        } // end if error parsing min int
      } // end if min specified
    } // end if min arg included


    if (args.getArgc() > SOCKET_INDEX_OF_MAX_VALUE)
    {
      if (!isNone(args.getArg(SOCKET_INDEX_OF_MAX_VALUE))) {
	
        maxInt = args.getArgInt(SOCKET_INDEX_OF_MAX_VALUE, &ok);

        if (!ok) {
          if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
            snprintf(errorBuffer, errorBufferLen,
                     "type was int, but max was '%s' when it should be an int or 'None'",
                     args.getArg(SOCKET_INDEX_OF_MAX_VALUE));
          }
          return false;

        } // end if error parsing max int
      } // end if max specified

    } // end if max arg included

    if (minInt > maxInt) 
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
         	       "type was int, but min (%d) is greater than max (%d)",
                 minInt, maxInt);
      }
      return false;
    }

    if (defaultInt < minInt)
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
 	               "type was int, but defaultVal (%d) is less than min (%d)",
	               defaultInt, minInt);
      }
      return false;
    }

    if (defaultInt > maxInt)
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
                 "type was int, but defaultVal (%d) is greater than max (%d)",
                 defaultInt, maxInt);
      }
      return false;
    }

    configArg = MvrConfigArg(name.c_str(), 
                            defaultInt, 
                            description.c_str(),
                            minInt, 
                            maxInt);

  } // end case INT

  break;


  case DOUBLE:
  {
    double defaultDouble = -1;
    double minDouble = -HUGE_VAL;
    double maxDouble = HUGE_VAL;

    if (args.getArgc() > SOCKET_INDEX_OF_VALUE) {
    
      defaultDouble = args.getArgDouble(SOCKET_INDEX_OF_VALUE, &ok);

      if (!ok) {
        if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
          snprintf(errorBuffer, errorBufferLen,
                   // KMC 8/9/13 Did not fix "an" just in case client is already relying on message
	                 "type was double, but default value was '%s' when it should be an double",
	                 args.getArg(SOCKET_INDEX_OF_VALUE));
        }
        return false;

      } // end if error parsing double 
    }
    else { // insufficient args
    
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
	               "type was double, but no default value was given");
      }
      return false;

    } // end else insufficient args

    
    if (args.getArgc() > SOCKET_INDEX_OF_MIN_VALUE)
    {
      if (!isNone(args.getArg(SOCKET_INDEX_OF_MIN_VALUE))) {
	
        minDouble = args.getArgDouble(SOCKET_INDEX_OF_MIN_VALUE, &ok);

        if (!ok) {
          if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
            snprintf(errorBuffer, errorBufferLen,
		                 "type was double, but min was '%s' when it should be a double or 'None'",
                     args.getArg(SOCKET_INDEX_OF_MIN_VALUE));
          }
          return false;

        } // end if error parsing min double 
      } // end if min specified
    } // end if min arg included


    if (args.getArgc() > SOCKET_INDEX_OF_MAX_VALUE)
    {
      if (!isNone(args.getArg(SOCKET_INDEX_OF_MAX_VALUE))) {
	
        maxDouble = args.getArgDouble(SOCKET_INDEX_OF_MAX_VALUE, &ok);

        if (!ok) {
          if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
            snprintf(errorBuffer, errorBufferLen,
		                 "type was double, but max was '%s' when it should be a double or 'None'",
                     args.getArg(SOCKET_INDEX_OF_MAX_VALUE));
          }
          return false;

        } // end if error parsing max double 
      } // end if max specified

    } // end if max arg included

    if (minDouble > maxDouble) 
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
	               "type was double, but min (%g) is greater than max (%g)",
                 minDouble, maxDouble);
      }
      return false;
    }

    if (defaultDouble < minDouble)
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
	               "type was double, but defaultVal (%g) is less than min (%g)",
	               defaultDouble, minDouble);
      }
      return false;
    }

    if (defaultDouble > maxDouble)
    {
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
	               "type was double, but defaultVal (%g) is greater than max (%g)",
                 defaultDouble, maxDouble);
      }
      return false;
    }
    configArg = MvrConfigArg(name.c_str(), 
                            defaultDouble, 
                            description.c_str(),
		                        minDouble, 
                            maxDouble);
  } // end case double
  break;

  case BOOL:
  {
    bool defaultBool = false;
    if (args.getArgc() > SOCKET_INDEX_OF_VALUE) {
    
      defaultBool = args.getArgBool(SOCKET_INDEX_OF_VALUE, &ok);

      if (!ok) {
        if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
          snprintf(errorBuffer, errorBufferLen,
                   // KMC 8/9/13 Did not fix "an" just in case client is already relying on message
	                 "type was bool, but default value was '%s' when it should be an bool",
	                 args.getArg(SOCKET_INDEX_OF_VALUE));
        }
        return false;

      } // end if error parsing double 
    }
    else { // insufficient args
    
      if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
        snprintf(errorBuffer, errorBufferLen,
	               "type was bool, but no default value was given");
      }
      return false;

    } // end else insufficient args

    configArg = MvrConfigArg(name.c_str(), 
                            defaultBool, 
                            description.c_str());

  } // end case bool

  break;

  case STRING:
  {
    std::string defaultString;
    if (args.getArgc() > SOCKET_INDEX_OF_VALUE) {
      defaultString = args.getArg(SOCKET_INDEX_OF_VALUE);
    }

    configArg = MvrConfigArg(name.c_str(), 
                            defaultString.c_str(), 
			                      description.c_str());

  } // end case string
  break;

  case SEPARATOR:
  {
    configArg = MvrConfigArg(MvrConfigArg::SEPARATOR);
  }
  break;

  case LIST:
  {
    configArg = MvrConfigArg(LIST,
                            name.c_str(), 
			                      description.c_str());

  } // end case list
  break; 

  default:
  {     
    if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
      snprintf(errorBuffer, errorBufferLen,
               "Unknown type '%s' given, choices are int|double|string|bool|separator",
               args.getArg(SOCKET_INDEX_OF_TYPE));
    }
    return false;
  
  } // end default
  break;

  } // end switch type 


  std::string displayHint;
 
  if ((args.getArgc() > SOCKET_INDEX_OF_DISPLAY) && 
      (!isNone(args.getArg(SOCKET_INDEX_OF_DISPLAY)))) {

    displayHint = args.getArg(SOCKET_INDEX_OF_DISPLAY);
  }
  configArg.setDisplayHint(displayHint.c_str());

  configArg.setConfigPriority(priority);

  // There are no pointers so the "detach" aspect isn't terribly important
  copyAndDetach(configArg);

  return true;

} // end method parseSocket


MVREXPORT bool MvrConfigArg::writeValue(MvrSocket *socket,
                                      const char *intro) const
{
  char value[10000];
  const size_t valueLength = sizeof(value);  

  switch (getType()) { 
  case INT:
	  snprintf(value, valueLength,
             "%d", getInt());
    break;
  
  case DOUBLE:
	  snprintf(value, valueLength,
             "%g", getDouble());
    break;

  case BOOL:
	  snprintf(value, valueLength,
             "%s", MvrUtil::convertBool(getBool()));
    break;
	
  case STRING:
    snprintf(value, valueLength,
             "%s", getString());
    break;

  case LIST:
    snprintf(value, valueLength,
             "%s",
             LIST_BEGIN_TAG);
    break;
 
  default:
    // Don't write anything for other arg types
    return false;

  } // end switch
      
  socket->writeString("%s %s %s", 
                      ((intro != NULL) ? intro : ""),
                      getName(), 
                      value);

  if (getType() == LIST) {

    char introBuf[512];
    const size_t introBufLength = sizeof(introBuf);

    snprintf(introBuf, introBufLength, 
             "%s    ", intro);

    for (int c = 0; c < getArgCount(); c++) {
      const MvrConfigArg *child = getArg(c);
      if (child == NULL) {
        continue;
      }
      child->writeValue(socket, introBuf);

    } // end for each child


    snprintf(value, valueLength,
             "%s",
             LIST_END_TAG);

    socket->writeString("%s %s %s", 
                        ((intro != NULL) ? intro : ""),
                        getName(), 
                        value);

  } // end list type

  return true;
 
} // end method writeValue


MVREXPORT bool MvrConfigArg::writeInfo(MvrSocket *socket,
                                     const char *intro) const
{
  // Don't write nameless parameters
  if (MvrUtil::isStrEmpty(getName())) {
    return false;
  }

  char min[512];
  char max[512];
  char type[512];
  char level[512];
  char delimiter[512];
  const size_t delimiterLength = 512;

  snprintf(min, sizeof(min), "None");
  snprintf(max, sizeof(max), "None");


  const char *priorityName = MvrPriority::getPriorityName(getConfigPriority());

  if (priorityName == NULL) {

	  MvrLog::log(MvrLog::Normal, "Config parameter %s has unknown priority", 
               getName());
    return false;
  }

  if (getConfigPriority() < MvrPriority::FACTORY) {

    snprintf(level, sizeof(level),
             priorityName);
  }
  else { // convert to all caps

    // This was done to match the original "FACTORY" output
    size_t i = 0;
    size_t priorityNameLength = strlen(priorityName);

    for (;
         ((i < priorityNameLength) && (i < sizeof(level))); 
         i++) {
      level[i] = ::toupper(priorityName[i]);
    } 
    if (i >= sizeof(level)) {
      i = sizeof(level) -1;
    }
    level[i] = '\0';

  } // else convert to all caps

  switch (getType()) { 
  case INT:
  	snprintf(type, sizeof(type), "Int");
  	snprintf(min, sizeof(min), "%d", getMinInt());
  	snprintf(max, sizeof(max), "%d", getMaxInt());
     break;
  
  case DOUBLE:
	  snprintf(type, sizeof(type), "Double");
	  snprintf(min,  sizeof(min),  "%g", getMinDouble());
	  snprintf(max,  sizeof(max),  "%g", getMaxDouble());
    break;

  case BOOL:
	  snprintf(type, sizeof(type), "Bool");
    break;
	
  case STRING:
	  snprintf(type, sizeof(type), "String");
    break;

  case LIST:
	  snprintf(type, sizeof(type), "List");

    snprintf(delimiter, delimiterLength,
             "%s",
             LIST_BEGIN_TAG);
    break;
 
  default:
    // Don't write anything for other arg types
    return false;

  } // end switch
     
 
  socket->writeString("%s %s %s %s %s %s \"%s\" \"%s\" \"%s\"", 
                      ((intro != NULL) ? intro : ""),
	                    type, 
                      getName(), 
                      level, 
              	      min, 
                      max, 
                      ((getDescription() != NULL) ? getDescription() : ""), 
                      ((getDisplayHint() != NULL) ? getDisplayHint() : ""),
                      delimiter);


  if (getType() == LIST) {

    char introBuf[512];
    const size_t introBufLength = sizeof(introBuf);

    snprintf(introBuf, introBufLength, 
             "%s    ", intro);

    for (int c = 0; c < getArgCount(); c++) {
      const MvrConfigArg *child = getArg(c);
      if (child == NULL) {
        continue;
      }
      child->writeInfo(socket, introBuf);

    } // end for each child


    snprintf(delimiter, delimiterLength,
             "%s",
             LIST_END_TAG);

    socket->writeString("%s %s %s %s %s %s \"%s\" \"%s\" \"%s\"", 
                        ((intro != NULL) ? intro : ""),
	                      type, 
                        getName(), 
                        level, 
                	      min, 
                        max, 
                        ((getDescription() != NULL) ? getDescription() : ""), 
                        ((getDisplayHint() != NULL) ? getDisplayHint() : ""),
                        delimiter);


  } // end list type

  return true;

} // end method writeInfo
 


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Resource/Translator Files
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MVREXPORT bool MvrConfigArg::parseResource(MvrArgumentBuilder *arg, 
				                                 char *errorBuffer,
				                                 size_t errorBufferLen,
                                         const char *logPrefix,
                                         bool isQuiet)
{
    
  if (arg == NULL) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::parseResource(), invalid input",
               ((logPrefix != NULL) ? logPrefix : ""));
    return false;
  }

  if (arg->getArgc() == 0) {
    // Don't log error because this just indicates an empty line
    return false;
  }

  if (arg->getArgc() <= RESOURCE_INDEX_OF_DESCRIPTION) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::parseResource(), invalid input, too few args (%i)",
               ((logPrefix != NULL) ? logPrefix : ""),
               arg->getArgc());
    return false;
  }

  if (MvrUtil::isStrEmpty(arg->getArg(RESOURCE_INDEX_OF_ARG_NAME))) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::parseResource(), empty name",
               ((logPrefix != NULL) ? logPrefix : ""));   
  }

 char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];
    
  if (arg->getArgc() > RESOURCE_INDEX_OF_DESCRIPTION) {
    if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_DESCRIPTION),
                             buf,
                             MAX_RESOURCE_ARG_TEXT_LENGTH)) {
      myDescription = buf;
    }
  }
  
  if (arg->getArgc() > RESOURCE_INDEX_OF_EXTRA) {
    if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_EXTRA),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
      myExtraExplanation = buf;
    }
  }
  
  if (arg->getArgc() > RESOURCE_INDEX_OF_DISPLAY) {
    if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_DISPLAY),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
      myDisplayName = buf;
    }
  }

  if (arg->getArgc() > RESOURCE_INDEX_OF_NEW) {
    if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_NEW),
                             buf,
                             MAX_RESOURCE_ARG_TEXT_LENGTH)) {
      if (MvrUtil::strcasecmp(buf, NEW_RESOURCE_TAG) == 0) {
        myIsTranslated = false;
      }
      else {
        myIsTranslated = true;
      }
    }
  }

  return true;
  
} // end method parseResource


MVREXPORT std::string MvrConfigArg::parseResourceSectionName(MvrArgumentBuilder *arg, 
                                                           const char *logPrefix)
{
  std::string sectionName;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_SECTION_NAME)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::parseResourceSectionName() invalid input",
                ((logPrefix != NULL) ? logPrefix : ""));
    return sectionName;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_SECTION_NAME),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    sectionName = buf;
  }
  return sectionName;

} // end method parseResourceSectionName

MVREXPORT std::string MvrConfigArg::parseResourceArgName(MvrArgumentBuilder *arg, 
                                                           const char *logPrefix)
{
  std::string argName;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_ARG_NAME)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::parseResourceArgName() invalid input (%s)",
                ((logPrefix != NULL) ? logPrefix : ""),
                ((arg != NULL) ? arg->getFullString() : "NULL"));
    return argName;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_ARG_NAME),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    argName = buf;
  }
  return argName;

} // end method parseResourceArgName


MVREXPORT MvrConfigArg::Type MvrConfigArg::parseResourceType(MvrArgumentBuilder *arg, 
                                                          const char *logPrefix)
{
  Type t = INVALID;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_TYPE)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::parseResourceType() invalid input",
                ((logPrefix != NULL) ? logPrefix : ""));
    return t;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_TYPE),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    t = typeFromString(buf);
  }
  return t;

} // end method parseResourceType

MVREXPORT  bool MvrConfigArg::isResourceTopLevel(MvrArgumentBuilder *arg, 
                                               const char *logPrefix)
{
  bool b = true;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_PARENT_PATH)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::isResourceTopLevel() invalid input",
                ((logPrefix != NULL) ? logPrefix : ""));
    return b;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_PARENT_PATH),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    b = MvrUtil::isStrEmpty(buf);
  }
  return b;

} // end method isResourceTopLevel


MVREXPORT std::list<std::string> MvrConfigArg::parseResourceParentPath(MvrArgumentBuilder *arg, 
                                                                     char separator,
                                                                     const char *logPrefix)
{
  std::list<std::string> path;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_PARENT_PATH)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::isResourceTopLevel() invalid input",
                ((logPrefix != NULL) ? logPrefix : ""));
    return path;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_PARENT_PATH),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    path = splitParentPathName(buf, separator);
  }

  return path;

} // end method parseResourceParentPath

MVREXPORT std::string MvrConfigArg::parseResourceDescription(MvrArgumentBuilder *arg, 
                                                           const char *logPrefix)
{
  std::string desc;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_DESCRIPTION)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::parseResourceDescription() invalid input (%s)",
                ((logPrefix != NULL) ? logPrefix : ""),
                ((arg != NULL) ? arg->getFullString() : "NULL"));
    return desc;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_DESCRIPTION),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    desc = buf;
  }
  return desc;

} // end method parseResourceDescription

MVREXPORT std::string MvrConfigArg::parseResourceExtra(MvrArgumentBuilder *arg, 
                                                     const char *logPrefix)
{
  std::string desc;

  if ((arg == NULL) ||
      (arg->getArgc() <= RESOURCE_INDEX_OF_EXTRA)) {
    
     MvrLog::log(MvrLog::Normal,
                "%sMvrConfigArg::parseResourceExtra() invalid input (%s)",
                ((logPrefix != NULL) ? logPrefix : ""),
                ((arg != NULL) ? arg->getFullString() : "NULL"));
    return desc;
  }

  char buf[MAX_RESOURCE_ARG_TEXT_LENGTH];

  if (parseResourceArgText(arg->getArg(RESOURCE_INDEX_OF_EXTRA),
                           buf,
                           MAX_RESOURCE_ARG_TEXT_LENGTH)) {
    desc = buf;
  }
  return desc;

} // end method parseResourceExtra

MVREXPORT  bool MvrConfigArg::parseResourceArgText(const char *argText,
                                                 char *bufOut,
                                                 size_t bufLen)
{
  if ((argText == NULL) || (bufOut == NULL) || (bufLen <= 0)) {
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::parseResourceArgText() error, invalid input");
    return false;
  }

  char otherBuf[MAX_RESOURCE_ARG_TEXT_LENGTH];
  
  strncpy(otherBuf, argText, MAX_RESOURCE_ARG_TEXT_LENGTH);
  otherBuf[MAX_RESOURCE_ARG_TEXT_LENGTH - 1] = '\0';
  
  int len = strlen(otherBuf);

  for (int j = len - 1; j >= 0; j--) {
   if ((otherBuf[j] < 0) || isspace(otherBuf[j])) {
     otherBuf[j] = '\0';
   }
   else {
     break;
   }
 }

 len = strlen(otherBuf);

 int k = 0;
 for (k = 0; k < len; k++) {
   if ((otherBuf[k] >= 0) && !isspace(otherBuf[k])) {
     break;
   }
 }
 if (k < len) {
   bool isStripped = MvrUtil::stripQuotes(bufOut, &otherBuf[k], bufLen); 
 }
 else {
   bufOut[0] = '\0';
 }

 bool isNullArg = false;
 if ((strlen(bufOut) == 0) ||
     (strcmp(bufOut, NULL_TAG) == 0)) {
   isNullArg = true;
 }

  if (isNullArg) {
    bufOut[0] = '\0';
  }
 
  return true;

} // end method parseResourceArgText

 
MVREXPORT bool MvrConfigArg::writeResource(FILE *file,
                                         char *lineBuf,
                                         int lineBufSize,
                                         char separatorChar,
                                         const char *sectionName,
                                         const char *logPrefix) const
{
     
  bool ignoreNormalSpaces = true;
  MvrArgumentBuilder builder(512, separatorChar, ignoreNormalSpaces); 

  if ((file == NULL) || (lineBuf == NULL) || (lineBufSize <= 0)) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::writeCsv() error writing %s, invalid input",
               ((logPrefix != NULL) ? logPrefix : ""),
               ((getName() != NULL) ? getName() : ""));
    return false;
  }

  // KMC 5/15/13 Do not write string holders until they are better handled.
  if (getType() == STRING_HOLDER) {
    return true;
  }

  std::string parentPathName = getParentPathName();
  if (parentPathName.empty()) {
    parentPathName = NULL_TAG;
  }
    
  builder.addPlain((sectionName != NULL) ? sectionName : NULL_TAG);
  builder.addPlain(myName.c_str());
  builder.addPlain(toString(myType));
  builder.addPlain(MvrPriority::getPriorityName(myConfigPriority));
  builder.addPlain(toString(myRestartLevel));
  builder.addPlain(parentPathName.c_str());
  builder.addPlain(((!myDescription.empty()) ? myDescription.c_str() : NULL_TAG));
  builder.addPlain(((!myExtraExplanation.empty()) ? myExtraExplanation.c_str() : NULL_TAG));
   
  builder.addPlain(((!myDisplayName.empty()) ? myDisplayName.c_str() : NULL_TAG));
 
  builder.addPlain(((myIsTranslated) ? NULL_TAG : NEW_RESOURCE_TAG));
  
      
  fprintf(file, "%s\n", builder.getFullString());

  if (isListType()) {
    for (size_t i = 0; i < getArgCount(); i++) {
      const MvrConfigArg *childArg = getArg(i);
      if (childArg != NULL) {
        bool isChildSuccess = childArg->writeResource(file,
                                                      lineBuf,
                                                      lineBufSize,
                                                      separatorChar,
                                                      sectionName,
                                                      logPrefix);
      }
    }
  } // end if list

  return true;

} // end method writeResource

 
MVREXPORT int MvrConfigArg::writeResourceHeader(FILE *file,
                                         char *lineBuf,
                                         int lineBufSize,
                                         char separatorChar,
                                         const char *sectionTitle,
                                         const char *logPrefix)
{

  if ((file == NULL) || (lineBuf == NULL) || (lineBufSize <= 0)) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::writeResourceHeader() error writing header, invalid input",
               ((logPrefix != NULL) ? logPrefix : ""));
    return false;
  }


  IFDEBUG(MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::writeResourceHeader() writing header with section title = %s",
               ((logPrefix != NULL) ? logPrefix : ""),
               ((sectionTitle != NULL) ? sectionTitle : "NULL")));
  
  snprintf(lineBuf, lineBufSize,
           //0   1   2   3   4   5   6   7   8   9
           "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s",
           //"\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"\n",
           ((!MvrUtil::isStrEmpty(sectionTitle)) ? sectionTitle : "SECTION"),  // 0
           separatorChar,
           "NAME",                                       // 1
           separatorChar,
           "TYPE",                                       // 2
           separatorChar,
           "PRIORITY",                                   // 3
           separatorChar,
           "RESTART",                                    // 4
           separatorChar,
           "PARENT",                                     // 5
           separatorChar,
           "DESCRIPTION",                                // 6
           separatorChar,
           "EXTRA",                                      // 7
           separatorChar,
           "DISPLAY",                                    // 8
           separatorChar,
           "NEW");                                       // 9
  
  fprintf(file, "%s\n", lineBuf);

  const char *roText = "read-only";
  const char *edText = "editable";
  snprintf(lineBuf, lineBufSize,
          // 0   1   2   3   4   5   6   7   8   9
           "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s",
           //"\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"\n",
           roText, //  "SECTION",                                    // 0
           separatorChar,
           roText, // "NAME",                                       // 1
           separatorChar,
           roText, // "TYPE",                                       // 2
           separatorChar,
           roText, // "PRIORITY",                                   // 3
           separatorChar,
           roText, // "RESTART",                                    // 4
           separatorChar,
           roText, // "PARENT",                                     // 5
           separatorChar,
           edText,      //"DESCRIPTION",                            // 6
           separatorChar,
           edText,      // "EXTRA"                                  // 7
           separatorChar,
           "future",    // "DISPLAY"                                // 8
           separatorChar,
           edText);     // "NEW"                                    // 9 
  fprintf(file, "%s\n", lineBuf);
  
  fprintf(file, "\n");

  return 3;

} // end method writeResourceHeader

MVREXPORT int MvrConfigArg::writeResourceSectionHeader(FILE *file,
                                                     char *lineBuf,
                                                     int lineBufSize,
                                                     char separatorChar,
                                                     const char *sectionName,
                                                     const char *sectionDesc,
                                                     const char *sectionExtra,
                                                     const char *sectionDisplayName,
                                                     bool isTranslated,
                                                     const char *logPrefix)
{
  if ((file == NULL) || (lineBuf == NULL) || (lineBufSize <= 0) ||
      (MvrUtil::isStrEmpty(sectionName))) {
    MvrLog::log(MvrLog::Normal,
               "%sMvrConfigArg::writeResourceSectionHeader() error writing header, invalid input",
               ((logPrefix != NULL) ? logPrefix : ""));
    return false;
  }


  
  snprintf(lineBuf, lineBufSize,
           //0   1   2   3   4   5   6   7   8   9
           "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s",
           sectionName,                                  // 0
           separatorChar,
           NULL_TAG,                                       // 1
           separatorChar,
           NULL_TAG,                                       // 2
           separatorChar,
           NULL_TAG,                                       // 3
           separatorChar,
           NULL_TAG,                                       // 4
           separatorChar,
           NULL_TAG,                                       // 5
           separatorChar,
           ((!MvrUtil::isStrEmpty(sectionDesc)) ? sectionDesc : NULL_TAG),   // 6
           separatorChar,
           ((!MvrUtil::isStrEmpty(sectionExtra)) ? sectionExtra : NULL_TAG), // 7
           separatorChar,
           ((!MvrUtil::isStrEmpty(sectionDisplayName)) ? sectionDisplayName : NULL_TAG), // 8
           separatorChar,
           ((isTranslated) ? NULL_TAG : NEW_RESOURCE_TAG));  // TODO Any indication for new sections?
  
  fprintf(file, "%s\n", lineBuf);

  return true;
} // end method writeResourceSectionHeader

/**
 * @param lineBuf a char array to which to write the name,
 * must be non-NULL
 * @param lineBufSize the int number of chars in lineBuf, must be positive
 * @param indentLevel the int level of indentation for the arg (children
 * of lists are indented further); must be non-negative
 * @return bool true if the name was successfully written; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::writeName(char *lineBuf,
                                     int lineBufSize,
                                     int indentLevel) const
{
  if ((lineBuf == NULL) || (lineBufSize <= 0)) {
    return false;
  }

  snprintf(lineBuf, lineBufSize,
           "%*s", (indentLevel * ourIndentSpaceCount), "");

  if ((getType() != MvrConfigArg::STRING_HOLDER) &&
      ( (strchr(getName(), ' ') != NULL || 
          strchr(getName(), '\t') != NULL) )) {

    snprintf(lineBuf, lineBufSize,
             "%s\"%s\"", lineBuf, getName());
  }
  else {
    snprintf(lineBuf, lineBufSize,
             "%s%s", lineBuf, getName());
  }
  return true;

} // end method writeName


/**
 * @param comment the char * string to be written to the file, must be 
 * non-NULL
 * @param file the FILE * to be written, must be non-NULL
 * @param lineBuf a char array to be used as a temporary write buffer,
 * must be non-NULL
 * @param lineBufSize the int number of chars in lineBuf, must be positive
 * @param startComment the char * text that should prefix the comment
 * on the first line
 * @return bool true if the comment was successfully written; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::writeMultiLineComment(const char *comment,
                                                 FILE *file,
                                                 char *lineBuf,
                                                 int lineBufSize,
                                                 const char *startComment)
{
  if ((comment == NULL) || 
      (file == NULL) ||
      (lineBuf == NULL) ||
      (lineBufSize <= 0) ||
      (startComment == NULL)) { 
    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::writeMultiLineComment() invalid input");
    return false;
  }
  // KMC 7/11/12 It seems like this could be made more efficient.
  // And should also check for bounds on the lineBuf.
  //
  MvrArgumentBuilder descr;
  descr.setQuiet(true); // KMC TODO Add flag myIsQuiet);
  descr.addPlain(comment);

  for (unsigned int i = 0; i < descr.getArgc(); i++)
  {
     // KMC 8/3/12 IMPORTANT NOTE:  While it would be extremely nice 
     // to prevent buffer overruns by using snprintf instead of sprintf,
     // it is currently ill-advised.  On Linux, the two methods behave
     // differently when the line buffer is used as both the target buffer
     // and as formatting input (as in all the prints below). It yields 
     // the desired results with sprintf, but substitutes an empty string
     // for the formatting input with snprintf.  This, in turn, causes
     // the output config data to be garbage. (On Windows, there is no 
     // problem.)  
  
    // see if we're over, if we are write this line out and start
    // the next one
    if (strlen(lineBuf) + strlen(descr.getArg(i)) > 78)
    {
      fprintf(file, "%s\n", lineBuf);

      sprintf(lineBuf, startComment, "");
      sprintf(lineBuf, "%s %s", lineBuf, descr.getArg(i));
    }
    // if its not the end of the line toss this word in
    else { 
    
      sprintf(lineBuf, "%s %s", lineBuf, descr.getArg(i));
    }
  }
  // put the last line into the file
  fprintf(file, "%s\n", lineBuf);

  return true;

} // end method writeMultiLineComment



/**
 * @param line a char array to be used as a temporary write buffer,
 * must be non-NULL
 * @param lineLen the int number of chars in line, must be positive
 * @param logPrefix the char * prefix to use in debug log messages, must be
 * non-NULL
 * @return bool true if the values were successfully written; false if an 
 * error occurred
**/
MVREXPORT bool MvrConfigArg::writeBounds(char *line,
                                       size_t lineLen,
                                       const char *logPrefix) const
{
   if ((line == NULL) || 
       (lineLen <= 0) ||
       (logPrefix == NULL)) {

    MvrLog::log(MvrLog::Normal,
               "MvrConfigArg::writeBounds() invalid input");
    return false;
   }
 
   // KMC 8/3/12 IMPORTANT NOTE:  While it would be extremely nice 
   // to prevent buffer overruns by using snprintf instead of sprintf,
   // it is currently ill-advised.  On Linux, the two methods behave
   // differently when the line buffer is used as both the target buffer
   // and as formatting input (as in all the prints below). It yields 
   // the desired results with sprintf, but substitutes an empty string
   // for the formatting input with snprintf.  This, in turn, causes
   // the output config data to be garbage. (On Windows, there is no 
   // problem.)  

   switch (getType()) {
    
   case INT:
    {
      if (hasMinBound()) {
        
        if (hasMaxBound()) {
          sprintf(line, 
                  "%s range [%d, %d], ", 
                  line, 
                  getMinInt(), getMaxInt());
        }
        else { // no max, just write min
          sprintf(line, 
                  // KMC 7/11/12 Corrected misspelling, will this be a problem?
                  "%s minimum %d, ", 
                  line, 
                  getMinInt());
        } // end else no max, just write min
      }
      else if (hasMaxBound()) {
        sprintf(line, 
                "%s maximum %d, ", 
                line, 
                getMaxInt());
      }
    }
    break;

   case DOUBLE:
    {
      if (hasMinBound()) {
        
        if (hasMaxBound()) {
          sprintf(line, 
                  "%s range [%g, %g], ", 
                  line, 
                  getMinDouble(), getMaxDouble());
        }
        else {
          sprintf(line, 
                  "%s minimum %g, ", 
                  line, 
                  getMinDouble());
        }
      }
      else if (hasMaxBound()) {
        // KMC 7/11/12 Made case consistent, problem?
        sprintf(line,
                "%s maximum %g, ", 
                line, 
                getMaxDouble());
      }
    }
    break;

   default:
     // No other types currently have bounds 
     break;
   } // end switch

   line[lineLen - 1] = '\0';

   return true;

} // end method writeBounds


MVREXPORT void MvrConfigArg::log(bool verbose, 
                               int indentCount,
                               const char *logPrefix) const
{
  std::list<MvrArgumentBuilder *>::const_iterator it;
  const std::list<MvrArgumentBuilder *> *argList = NULL;
  std::string intType;

  // Mvrbitrarily capping at 11 tabs
  char indent[12];
  int i = 0;
  for (; ((i < indentCount) && (i < 12 - 1)); i++) {
    indent[i] = '\t';
  } // end for each indentation
  // At most, i should be 11 here.
  indent[i] = '\0';

  void *intPointer = NULL;

  MvrLog::log(MvrLog::Terse, "%s%s%s (name)",
       ((logPrefix != NULL) ? logPrefix : ""),
	     indent,
	     getName());


  switch (getType()) 
  {
  case MvrConfigArg::INVALID:
    MvrLog::log(MvrLog::Terse, 
	       "\tType: %10s.  This argument was not created properly.", 
	       "invalid");

  case MvrConfigArg::INT:

    switch (myData.myIntData.myIntType) {
    case INT_NOT:
      intType = "Not";
      intPointer = NULL;
      break;
    case INT_INT:
      intType = "Int";
      intPointer = myData.myIntData.myIntPointer;
      break;
    case INT_SHORT:
      intType = "Short";
      intPointer = myData.myIntData.myIntShortPointer;
      break;
    case INT_UNSIGNED_SHORT:
      intType = "Unsigned Short";
      intPointer = myData.myIntData.myIntUnsignedShortPointer;
      break;
    case INT_UNSIGNED_CHAR:
      intType = "Unsigned Short";
      intPointer = myData.myIntData.myIntUnsignedCharPointer;
      break;
    default:
      intType = "Unknown";
      intPointer = NULL;
      break;
    }


    MvrLog::log(MvrLog::Terse, "%s\tValue: %d \tType: %s intType: %s",
               indent,
	       getInt(), "int", intType.c_str());
    if (!myOwnPointedTo)
      MvrLog::log(MvrLog::Terse, "%s\tPointer: %p",
                 indent, intPointer);

    if (verbose)
      MvrLog::log(MvrLog::Terse, "%s\tMin: %10d     Max: %10d", 
                 indent,
		             myData.myIntData.myMinInt, myData.myIntData.myMaxInt);
    break;

  case MvrConfigArg::DOUBLE:
    MvrLog::log(MvrLog::Terse, "%s\tValue: %f \tType: %s",
               indent,
	       getDouble(), "double");
    if (!myOwnPointedTo)
      MvrLog::log(MvrLog::Terse, "%s\tPointer: %p",
                 indent, myData.myDoubleData.myDoublePointer);
    if (verbose)
      MvrLog::log(MvrLog::Terse, "%s\tMin: %10g     Max: %10g", 
                 indent,
		             myData.myDoubleData.myMinDouble, myData.myDoubleData.myMaxDouble);
    break; 

  case MvrConfigArg::STRING:
  case MvrConfigArg::STRING_HOLDER:

    MvrLog::log(MvrLog::Terse, "%s\tValue: %s \tType: %s", 
               indent,
	       getString(), "string");
    if (!myOwnPointedTo)
      MvrLog::log(MvrLog::Terse, "%s\tPointer: %p",
                 indent, myData.myStringData.myStringPointer);
    if (verbose)
      MvrLog::log(MvrLog::Terse, "%s\tLength: %d", 
                 indent,
                 myData.myStringData.myMaxStrLen);
    break;

  case MvrConfigArg::CPPSTRING:
    MvrLog::log(MvrLog::Terse, "%s\tValue: %s \tType: %s", 
      indent,
      getString(), "cppstring");
    if (!myOwnPointedTo)
      MvrLog::log(MvrLog::Terse, "%s\tPointer: %p",
                 indent, myData.myCppStringData.myCppStringPtr);
    break;

  case MvrConfigArg::BOOL:

    MvrLog::log(MvrLog::Terse, "%s\tValue: %s \tType: %s", 
               indent,
	       MvrUtil::convertBool(getBool()),  "bool");
    if (!myOwnPointedTo)
      MvrLog::log(MvrLog::Terse, "%s\tPointer: %p",
                 indent, myData.myBoolData.myBoolPointer);
    break;


  case MvrConfigArg::LIST:
  case MvrConfigArg::LIST_HOLDER:

    MvrLog::log(MvrLog::Terse, "%sChildren: %d type: %10s", 
               indent,
	       getArgCount(), "list");

    // KMC 7/10/12 Should the children only be logged if verbose is set to true?

    break;

  case MvrConfigArg::FUNCTOR:

    MvrLog::log(MvrLog::Terse, "%sType: %s", 
	       indent,
	       "functor");
    MvrLog::log(MvrLog::Terse, "%s\t\t\tValues:",
               indent);

    argList = myData.myFunctorData.myGetFunctor->invokeR();
    for (it = argList->begin(); it != argList->end(); it++)
      MvrLog::log(MvrLog::Terse, "%s\t\t%s", indent, (*it)->getFullString());
    break;

  case MvrConfigArg::DESCRIPTION_HOLDER:

    MvrLog::log(MvrLog::Terse, "%sType: %20s Description: %s", 
               indent,
	             "description_holder", getDescription());

  default:
    MvrLog::log(MvrLog::Terse, 
	             "\tType: %10s.  This type doesn't have a case in MvrConfigArg::log.",
	             toString(myType));
    break;
  }

  MvrLog::log(MvrLog::Terse, "\t\tPriority: %s", 
	     MvrPriority::getPriorityName(myConfigPriority));
  if (strlen(getDescription()) != 0)
    MvrLog::log(MvrLog::Terse, "%s\tDescription: %s",
	       indent,
	       getDescription());

  if ((isListType()) && 
      (myData.myListData.myChildArgList != NULL)) {
    int c = 1;
    for (std::list<MvrConfigArg>::const_iterator iter = myData.myListData.myChildArgList->begin();
	 iter != myData.myListData.myChildArgList->end();
	 iter++, c++) {
      MvrLog::log(MvrLog::Terse, 
		 "%sChild Mvrg #%i:",
		 indent,
		 c);
      (*iter).log(verbose, indentCount + 1);
    } // end for each child arg
  }

}

/**
   The priority of this argument when used in MvrConfig.
 **/
MVREXPORT MvrPriority::Priority MvrConfigArg::getConfigPriority(void) const
{
  return myConfigPriority;
}

/**
   The priority of this argument when used in MvrConfig.
 **/

MVREXPORT void MvrConfigArg::setConfigPriority(MvrPriority::Priority priority)
{
  myConfigPriority = priority;
}


MVREXPORT const char *MvrConfigArg::getDisplayHint() const
{
  if (myDisplayHint.length() > 0) {
    return myDisplayHint.c_str();
  }
  else {
    return NULL;
  }
} // end method getDisplayHint


/**
 * The "display hint" is a text string that may be used by the client
 * to improve the display of the argument.  Multiple, non-confilicting
 * display hints may be specified by using the "&&" separator.  For example:
 *   <code>"Choices:High;;Low&&Visible:PinEnabled=true"</code>
 * 
 * The display hints that are currently supported by MobileEyes and
 * MobilePlanner are described below.
 *
 * For all MvrConfigArg types:
 * <ul>
 *
 * <li>
 *   <code>Visible:</code><i>OtherParamName=OtherParamValue</i>
 *   <ul>
 *   <li>The current MvrConfigArg will be displayed in the configuration editor 
 *       only when the other parameter equals the specified value.  This is used 
 *       to vary a set of displayed items based on list choices.  The presentation 
 *       will be similar to the current implementation of child objects in the map.</li>
 *   </ul>
 * </li>
 * 
 * <li>
 *   <code>Visible:</code><i>OtherParamName!=OtherParamValue</i>  
 *   <ul>
 *   <li>The current MvrConfigArg will be displayed in the configuration editor 
 *       only when the other parameter does not equal the specified value.  This is used 
 *       to vary a set of displayed items based on list choices.  The presentation 
 *       will be similar to the current implementation of child objects in the map.</li>
 *   </ul> 
 * </li>
 * 
 * <li>
 *   <code>Visible:</code><i>OtherParamName>OtherParamValue</i>
 *   <ul>
 *   <li>EXPLANATION</li>
 *   </ul>
 * </li>
 *
 * <li>
 *   <code>Visible:</code><code>false</code>
 *   <ul>
 *   <li>The MvrConfigArg is never displayed</li>
 *   </ul>
 * </li>
 * 
 * <li>
 *   <code>Editable:</code><i>OtherParamName=OtherParamValue</i>
 *   <ul>
 *   <li>The current MvrConfigArg is read-only unless the other parameter equals 
 *       the specified value.  This allows fine-grained dependencies, primarily 
 *       for small �enabled� flags.</li>
 *   </ul>
 * </li>
 * 
 * <li>
 *   <code>Editable:</code><i>OtherParamName!=OtherParamValue</i>  
 *   <ul>
 *   <li>The current MvrConfigArg is read-only unless the other parameter equals 
 *       the specified value.  This allows fine-grained dependencies, primarily 
 *       for small �enabled� flags.</li>
 *   </ul> 
 * </li>
 * 
 * <li>
 *   <code>Editable:</code><i>OtherParamName>OtherParamValue</i>
 *   <ul>
 *   <li>The current MvrConfigArg is read-only unless the other parameter exceeds 
 *       the specified value.  This is provided primarily for consistency with the 
         Visible case.  Its usefulness is questionable.</li>
 *   </ul>
 * </li>
 *
 * <li>
 *   <code>Editable:</code><code>false</code>
 *   <ul>
 *   <li>The current MvrConfigArg is always read-only</li>
 *   </ul>
 * </li>
 * </ul>
 *
 * 
 * For type STRING:
 * <ul>
 *
 * <li><code>Choices:</code><i>choice1</i>(<code>;;</code><i>choice#i</i>)*
 *   <ul>
 *     <li>For example, <code>"Choices:North;;South;;East;;West"</code></li>
 *     <li>To include an empty choice, start the choice list with <code>;;</code>.  
 *         For example, <code>"Choices:;;FirstNonEmpty;;SecondNonEmpty"</code></li>
 *   </ul>
 * </li>
 * <li><code>MapItem:</code><i>type</i><code>|</code>(<code>Parent</code>|<code>SubType</code>|<code>SubTypeAndParent</code>|<code>SubTypeGlob</code>)<code>|</code>(<code>Optional</code>|<code>Required</code>)
 *   <ul>
 *     <li>For example, <code>"MapItem:Dock|Parent|Optional"</code></li>
 *     <li>If <code>SubTypeGlob</code> is specified, then <i>type</i> may contain 
 *         wildchar characters <code>*</code> and <code>?</code></li>
 *   </ul>
 * </li>
 * <li><code>ChildObject:</code><i>type</i><code>|</code>(<code>Parent</code>|<code>SubType</code>|<code>SubTypeAndParent</code>|<code>SubTypeGlob</code>)<code>|</code>(<code>Optional</code>|<code>Required</code>)
 *   <ul>
 *     <li>This value is only supported for parameters that are associated 
 *         with a MapInfo.  It allows the application to create a parent/child 
 *         relationship amongst certain map items.  Mapper3 automatically manages 
 *         the creation of the child items. </li>
 *     <li>To specify a default value for the associated parameter, set the 
 *         MvrConfigArg's string value to: 
 *         <code><i>ParentTypeName</i>_\@_<i>ParameterName</i>_\@_<i>DefaultChildTypeName</i></code>.</li>
 *   </ul>
 * </li>
 * <li>  <code>Macro:</code><i>taskClass1</i>(<code>;;</code><i>taskClass#i</i>)*|(<code>Optional</code>|<code>Required</code>)
 *   <ul><li>For example, "Macro:Audio;;Camera;;Video|Optional"</li></ul>
 * </li>
 * <li>  <code>Macro</code>
 *   <ul><li>When used with no qualifiers, there are no task class restrictions and 
 *           the macro is optional</li></ul>
 * </li>
 * 
 * <li>  <code>Time:</code><i>format</i>
 *   <ul>
 *       <li>The format must conform to Qt's QTime format definition. (Briefly: 
 *           h = hour, m = minute, s = second, z = millisecond, and AP = AM/PM.
 *           Double characters indicate that a leading 0 should be used to pad.)</li>
 *       <li>For example, "Time:h:mm AP" or "Time:hh:mm:ss"</li>
 *       <li>Note that this is a time-of-day, not a duration.  Seconds and minutes
 *           range from 0 to 59, and hour ranges from 0 to 23 (or 1 to 12 with AM/PM).</li>
 *       <li>AP (i.e. AM/PM) only affects the display of the time; the string that
 *           is stored in the MvrConfigArg should always be converted to 24 hour 
 *           format.</li>
 *   </ul>
 * </li>
 *
 * <li> <code>Time</code>
 *   <ul><li>When there are no qualifiers, the default format "h:mm AP" is used</li></ul>
 * </li>
 *
 * <li><code>RobotFile:</code><i>fileFilterNames</i><code>|</code><i>fileFilters</i>
 *   <ul>
 *      <li>where:
 *        <ul>
 *          <li><i>fileFilterNames</i> is a string that contains the complete displayable 
 *                    names for all of the file filters 
 *                    (e.g. <code>Map files (*.map);;World files
 *                    (*.wld)</code>), or, 
 *                    empty if no filter needs to be applied, and</li>
 *          <li><i>fileFilters</i> is a string that contains just file extensions
 *                    for each of the file filters (e.g. <code>*.map;;*.wld"</code>).  
 *                    The number and order of these filters must match 
 *                    that of the fileFilterNames parameter.</li>
 *          </ul>
 *        </li>
 *      <li>For example, <code>"RobotFile:Map Files (*.map);;World Files
 *      (*.wld)|*.map;;*.wld"</code></li>
 *      <li>For further example, <code> "All audio files (*.ogg *.wav);;Ogg Vorbis files (*.ogg);;WAV files (*.wav);;*.ogg *.wav;;*.ogg;;*.wav"</code></li>
 *   </ul>
 * </li>
 * </ul>
 *
 * For type INT:
 * <ul>
 *   <li><code>Color</code></li>
 * </ul>
 *
 * For type BOOL:
 * <ul>
 *   <li><code>Checkbox</code></li>
 * </ul>
 *
**/
MVREXPORT void MvrConfigArg::setDisplayHint(const char *hintText)
{
  if (hintText != NULL) {
    myDisplayHint = hintText;
  }
  else {
    myDisplayHint = "";
  }
} // end method setDisplayHint

  
MVREXPORT MvrConfigArg::RestartLevel MvrConfigArg::getRestartLevel() const
{
  return myRestartLevel;
}
  
MVREXPORT void MvrConfigArg::setRestartLevel(RestartLevel level)
{
  myRestartLevel = level;
}


MVREXPORT bool MvrConfigArg::getSuppressChanges() const
{
  return mySuppressChanges;
}
  
MVREXPORT void MvrConfigArg::setSuppressChanges(bool suppressChanges)
{
  mySuppressChanges = suppressChanges;
}


MVREXPORT bool MvrConfigArg::isSerializable() const
{
  return myIsSerializable;
}

MVREXPORT void MvrConfigArg::setSerializable(bool isSerializable)
{
  myIsSerializable = isSerializable;
}
  
MVREXPORT bool MvrConfigArg::hasMinBound() const
{
  bool isMinValid = false;

  switch (getType()) {

  case INT:
    isMinValid = myData.myIntData.myMinInt != INT_MIN;

    //switch (myData.myIntData.myIntType) {
    //case INT_INT:
    //  isMinValid = (myData.myIntData.myMinInt != INT_MIN);
    //  break;
    //case INT_SHORT:
    //  isMinValid = (myData.myIntData.myMinInt != SHRT_MIN);
    //  break;
    //case INT_UNSIGNED_SHORT:
    //  // KMC 7/13/12 Wondering whether this should just always be true
    //  // (i.e. always min 0 at least).  Same with uchar
    //  isMinValid = (myData.myIntData.myMinInt != 0);
    //  break;
    //case INT_UNSIGNED_CHAR:
    //  isMinValid = (myData.myIntData.myMinInt != 0);
    //  break;
    //} // end switch int type
    break;
  case DOUBLE:
    isMinValid = (MvrMath::fabs(myData.myDoubleData.myMinDouble + HUGE_VAL) > MvrMath::epsilon());
    break;
  default:
    isMinValid = false;
    break;
  } // end switch type

  return isMinValid;
  
} // end method hasMinBound

  
MVREXPORT bool MvrConfigArg::hasMaxBound() const
{
  bool isMaxValid = false;

  switch (getType()) {

  case INT:
    isMaxValid = (myData.myIntData.myMaxInt != INT_MAX);

    //switch (myData.myIntData.myIntType) {
    //case INT_INT:
    //  isMaxValid = (myData.myIntData.myMaxInt != INT_MAX);
    //  break;
    //case INT_SHORT:
    //  isMaxValid = (myData.myIntData.myMaxInt != SHRT_MAX);
    //  break;
    //case INT_UNSIGNED_SHORT:
    //  isMaxValid = (myData.myIntData.myMaxInt != USHRT_MAX);
    //  break;
    //case INT_UNSIGNED_CHAR:
    //  isMaxValid = (myData.myIntData.myMaxInt != 255);
    //  break;
    //} // end switch int type
    break;
  case DOUBLE:
    isMaxValid = (MvrMath::fabs(myData.myDoubleData.myMaxDouble - HUGE_VAL) > MvrMath::epsilon());
    break;
  default:
    isMaxValid = false;
    break;
  } // end switch type

  return isMaxValid;

} // end method hasMaxBound

/**
   This is for debugging and will prevent the bounds checking from
   happening, you shouldn't normally use it
 **/
MVREXPORT void MvrConfigArg::setIgnoreBounds(bool ignoreBounds)
{
  myIgnoreBounds = ignoreBounds;
}

MVREXPORT bool MvrConfigArg::hasExternalDataReference() const
{
  bool b = false;

  switch (getType()) {

  case INT:
  case DOUBLE:
  case BOOL:
    b = !myOwnPointedTo;
    break;

  case STRING:
  case STRING_HOLDER:
  case CPPSTRING:
    // KMC 7/13/12 Not sure why the myOwnPointedTo attribute does
    // not apply to strings ??
    // KMC own string change
    // b = !myData.myStringData.myUsingOwnedString;
    b = !myOwnPointedTo;
    break;

  default:
    b = false;
    break;
  }

  return b;

} // end method hasExternalDataReference

  
MVREXPORT bool MvrConfigArg::isPlaceholder() const
{
  switch (getType()) {
  case STRING_HOLDER:
  case LIST_HOLDER:
  case DESCRIPTION_HOLDER:
    return true;
  default:
    return false;
  }
  return false;
}


MVREXPORT bool MvrConfigArg::isValueEqual(const MvrConfigArg &other) const
{
  if (strcmp(getName(), other.getName()) != 0) {
    return false;
  }
  Type t = getType();
  if (t != other.getType()) {
    return false;
  }
  bool isEqual = false;

  switch (t) {
  case INVALID:
    isEqual = true; // Seems logical that two invalid args are equal...
    break;

  case INT:
    isEqual = (getInt() == other.getInt());
    break;

  case DOUBLE:
    isEqual = (MvrMath::fabs(getDouble() - other.getDouble()) < MvrMath::epsilon());
    break;

  case STRING:
    isEqual = (strcmp(getString(), other.getString()) == 0);
    break;

  case CPPSTRING:
    isEqual = *(getCppStringPtr()) == *(other.getCppStringPtr());
    break;

  case BOOL:
    isEqual = (getBool() == other.getBool());
    break;

  case SEPARATOR:
    isEqual = true;
    break;

  // Since STRING_HOLDERs do not appear to have equal values, assuming
  // the same for LIST_HOLDERs.
  case LIST:
    {
      if ((myData.myListData.myChildArgList != NULL) &&
          (other.myData.myListData.myChildArgList != NULL)) {

        if (myData.myListData.myChildArgList->size() == 
                              other.myData.myListData.myChildArgList->size()) {
          for (std::list<MvrConfigArg>::const_iterator 
                                          iter1 = myData.myListData.myChildArgList->begin(),
                                          iter2 = other.myData.myListData.myChildArgList->begin();
               ((iter1 != myData.myListData.myChildArgList->end()) &&
                (iter2 != other.myData.myListData.myChildArgList->end()));
               iter1++, iter2++) {
           
             if (!(*iter1).isValueEqual(*iter2)) {
               isEqual = false;
               break;
             }
           }
        } 
        else { // different sizes, not equal
          isEqual = false;
        }

      }
      else {
        isEqual = ((myData.myListData.myChildArgList == NULL) &&
                   (other.myData.myListData.myChildArgList == NULL));
      }

    }
    break;

  case FUNCTOR:
    // Hmmm... Equal if they are the same functors??  Not sure how much
    // sense this makes...
    isEqual = ((myData.myFunctorData.mySetFunctor == other.myData.myFunctorData.mySetFunctor) &&
               (myData.myFunctorData.myGetFunctor == other.myData.myFunctorData.myGetFunctor));
    break;

  case DESCRIPTION_HOLDER:
    isEqual = (strcmp(getDescription(), other.getDescription()) == 0);
    break;

  default:
    isEqual = false;
    break;

  } // end switch type

  return isEqual;

} // end method isValueEqual


MVREXPORT bool MvrConfigArg::setValue(const MvrConfigArg &source,
                                    bool isVerifyArgNames)
{
  Type t = getType();
  if (t != source.getType()) {
    return false;
  }

  if (isVerifyArgNames && MvrUtil::strcmp(getName(), source.getName()) != 0) {
    /**
    MvrLog::log(MvrLog::Verbose,
               "MvrConfigArg::setValue() unverified names, cannot set %s to %s",
               getName(), source.getName());
    **/
    return false;
  }

  bool isSuccess = true;

  switch (t) {
  case INVALID:
    // Nothing to copy with invalid args
    break;

  case INT:
    isSuccess = setInt(source.getInt());
    break;

  case DOUBLE:
    isSuccess = setDouble(source.getDouble());
    break;

  case STRING:
    isSuccess = setString(source.getString());
    break;

  case CPPSTRING:
    isSuccess = setCppString(source.getCppString());
    break;

  case BOOL:
    isSuccess = setBool(source.getBool());
    break;

  case SEPARATOR:
    break;

  // Since STRING_HOLDERs are apparently not copied, assuming the 
  // same for LIST_HOLDERs.
  case LIST:
    // Not entirely sure about this, but I think it's consistent 
    if ((myData.myListData.myChildArgList != NULL) && 
        (source.myData.myListData.myChildArgList != NULL)) {

      if (myData.myListData.myChildArgList->size() == 
              source.myData.myListData.myChildArgList->size()) {

        std::list<MvrConfigArg>::const_iterator iter2 = 
                                  source.myData.myListData.myChildArgList->begin();

        for (std::list<MvrConfigArg>::iterator 
                 iter1 = myData.myListData.myChildArgList->begin();
             ((iter1 != myData.myListData.myChildArgList->end()) && 
              (iter2 != source.myData.myListData.myChildArgList->end()));
             iter1++, iter2++) {

          // This has the unfortunate potential to leave a list half-changed.
          // It would probably be better to copy the list or something.
          // Care would need to be taken for the parent pointer.
          if (!((*iter1).setValue(*iter2, isVerifyArgNames))) {
            isSuccess = false;
            break;
          }
        } // end for each child arg list
      }
      else { // different size lists
        isSuccess = false;
      }
    }
    else {
      isSuccess = ((myData.myListData.myChildArgList != NULL) && 
                   (source.myData.myListData.myChildArgList != NULL));
    }

    break;

  case FUNCTOR:
    // Not copying functors right now...
    isSuccess = false;
    break;

  case DESCRIPTION_HOLDER:
    // Not copying description holders either right now...
    isSuccess = false;
    break;

  default:
    isSuccess = false;
    break;

  } // end switch type

  return isSuccess;

} // end method setValue

  
MVREXPORT bool MvrConfigArg::isTranslated() const
{
  return myIsTranslated;

} // end method isTranslated

  
MVREXPORT void MvrConfigArg::setTranslated(bool b) 
{
  myIsTranslated = b;

} // end method setTranslated


MVREXPORT void MvrConfigArg::setDescription(const char *description)
{
  myDescription = description; 
}

MVREXPORT bool MvrConfigArg::isListType() const
{
  return ((myType == LIST) || (myType == LIST_HOLDER));
}
