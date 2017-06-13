/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrConfigArg.h
 > Description  : Argument class for MvrConfig
 > Author       : Yu Jie
 > Create Time  : 2017年05月20日
 > Modify Time  : 2017年05月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrConfigArg.h"
#include "MvrArgumentBuilder.h"
#include "MvrFileParser.h"
#include "MvrSocket.h"

#if (defined(MVRDEBUG_CONFIGARG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif

int MvrConfigArg::ourIndentSpaceCount = 3;

MVREXPORT const char *MvrConfigArg::LIST_BEGIN_TAG   = "_beginList";
MVREXPORT const char *MvrConfigArg::LIST_END_TAG     = "_endList";

MVREXPORT const char *MvrConfigArg::NULL_TAG         = "NULL";
MVREXPORT const char *MvrConfigArg::NEW_RESOURCE_TAG = "xltrNew";

std::map<std::string, MvrConfigArg::Type, MvrStrCaseCmpOp> *MvrConfigArg::ourTextToTypeMap = NULL;
std::map<std::string, MvrConfigArg::RestartLevel, MvrStrCaseCmpOp> *MvrConfigArg::ourTextToRestartLevelMap = NULL;

MVREXPORT const char *MvrConfigArg::toString(Type t)
{
  switch(t)
  {
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
  }
  return "Unknown";
}

const char *MvrConfigArg::toString(RestartLevel r)
{
  switch (r)
  {
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
}

MvrConfigArg::Type MvrConfigArg::typeFromString(const char *text)
{
  if (MvrUtil::isStrEmpty(text))
  {
    return INVALID;
  }
  if (ourTextToTypeMap == NULL)
  {
    ourTextToTypeMap = new std::map<std::string, Type, MvrStrCaseCmpOp>();
    (*ourTextToTypeMap)["INVALID"]    = INVALID;
    (*ourTextToTypeMap)["INT"]        = INT;
    (*ourTextToTypeMap)["DOUBLE"]     = DOUBLE;
    (*ourTextToTypeMap)["STRING"]     = STRING;
    (*ourTextToTypeMap)["CPPSTRING"]  = CPPSTRING;
    (*ourTextToTypeMap)["BOOL"]       = BOOL;
    (*ourTextToTypeMap)["FUNCTOR"]    = FUNCTOR;
    (*ourTextToTypeMap)["DESCRIPTION_HOLDER"] = DESCRIPTION_HOLDER;
    (*ourTextToTypeMap)["STRING_HOLDER"] = STRING_HOLDER;
    (*ourTextToTypeMap)["SEPARATOR"]  = SEPARATOR;
    (*ourTextToTypeMap)["LIST"]       = LIST;
    (*ourTextToTypeMap)["LIST_HOLDER"]= LIST_HOLDER;
  }
  std::map<std::string, Type, MvrStrCaseCmpOp>::iterator iter = ourTextToTypeMap->find(text);
  if (iter != ourTextToTypeMap->end()){
    return iter->second;
  }
  return INVALID;
}

MVREXPORT MvrConfigArg::RestartLevel MvrConfigArg::restartLevelFromString(const char *text)
{
  if (MvrUtil::isStrEmpty(text))
  {
    return NO_RESTART;
  }
  if (ourTextToRestartLevelMap == NULL)
  {
    ourTextToRestartLevelMap = new std::map<std::string, RestartLevel, MvrStrCaseCmpOp>();
    (*ourTextToRestartLevelMap)["NO_RESTART"]    = NO_RESTART;
    (*ourTextToRestartLevelMap)["RESTART_CLIENT"]= RESTART_CLIENT;
    (*ourTextToRestartLevelMap)["RESTART_IO"]    = RESTART_IO;
    (*ourTextToRestartLevelMap)["RESTART_SOFTWARE"]    = RESTART_SOFTWARE;
    (*ourTextToRestartLevelMap)["RESTART_HARDWARE"]    = RESTART_HARDWARE;
  }  
  std::map<std::string, RestartLevel, MvrStrCaseCmpOp>::iterator iter = ourTextToRestartLevelMap->find(text);
  if (iter != ourTextToRestartLevelMap->end())
  {
    return iter->second;
  }
  return NO_RESTART;
}


MVREXPORT MvrConfigArg::MvrConfigArg()
{
  clear(true);
}

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, int *pointer,
                                     const char *description, int minInt, int maxInt)
{
  clear(true, INT, INT_INT);
  set(INT, name, description);

  myData.myIntData.myIntType   = INT_INT;
  myData.myIntData.myMinINT    = minInt;
  myData.myIntData.myMaxINT    = maxInt;
  myData.myIntData.myIntPointer= pointer;
}      

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, short *pointer,
                                     const char *description, int minInt, int maxInt)
{
  clear(true, INT, INT_SHORT);
  set(INT, name, description);

  myData.myIntData.myIntType   = INT_SHORT;
  myData.myIntData.myMinINT    = minInt;
  myData.myIntData.myMaxINT    = maxInt;
  myData.myIntData.myIntShortPointer= pointer;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, unsigned short *pointer,
                                     const char *description, int minInt, int maxInt)
{
  clear(true, INT, INT_UNSIGNED_SHORT);
  set(INT, name, description);

  myData.myIntData.myIntType   = INT_UNSIGNED_SHORT;
  myData.myIntData.myMinINT    = minInt;
  myData.myIntData.myMaxINT    = maxInt;
  myData.myIntData.myIntUnsignedShortPointer = pointer;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, unsigned char *pointer,
                                     const char *description, int minInt, int maxInt)
{
  clear(true, INT, INT_UNSIGNED_CHAR);
  set(INT, name, description);

  myData.myIntData.myIntType   = INT_UNSIGNED_CHAR;
  myData.myIntData.myMinINT    = minInt;
  myData.myIntData.myMaxINT    = maxInt;
  myData.myIntData.myIntUnsignedCharPointer = pointer;
} 


MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, double *pointer,
                                     const char *description, double minDouble,
                                     double maxDouble, int precision)
{
  clear(true, DOUBLE);
  set(DOUBLE, name, description);

  myData.myDoubleData.myMinDouble   = minDouble;
  myData.myDoubleData.myMaxDouble   = maxDouble;
  myData.myDoubleData.myPrecision   = precision;
  myData.myDoubleData.myDoublePointer    = pointer;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, bool *pointer,
                                     const char *description)
{
  clear(true, BOOL);
  set(BOOL, name, description);

  myData.myBoolData.myBoolPointer = pointer;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, int val,
                                     const char *description, int minInt, int maxInt)
{
  clear(true, INT, INT_SHORT);
  set(INT, name, description);

  myData.myIntData.myIntType     = INT_INT;
  myData.myIntData.myMinINT      = minInt;
  myData.myIntData.myMaxINT      = maxInt;

  myData.myIntData.myIntPointer  = new int;
  *myData.myIntData.myIntPointer = val;
  myOwnPointedTo                 = true;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, double val,
                                     const char *description, double minDouble,
                                     double maxDouble, int precision)
{
  clear(true, DOUBLE);
  set(DOUBLE, name, description);

  myData.myDoubleData.myMinDouble   = minDouble;
  myData.myDoubleData.myMaxDouble   = maxDouble;
  myData.myDoubleData.myPrecision   = precision;

  myData.myDoubleData.myDoublePointer    = new double;
  *myData.myDoubleData.myDoublePointer   = val;
  myOwnPointedTo                 = true;
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, bool val,
                                     const char *description)
{
  clear(true, BOOL);
  set(BOOL, name, description);

  myData.myBoolData.myBoolPointer = new bool;
  *myData.myBoolData.myBoolPointer= val;
  myOwnPointedTo                  = true;
}

/**
 * This constructor can accept both an already-allocated string,
 * or ArConfigArg can to the memory managment itself (reallocation
 * and finally deletion). If @param maxStrLen is 0, then ArConfigArg will
 * do its own memory management, with the contents of @param str copied
 * as the initial value of the internally held string. Otherwise,
 * @param str must point to an allocated string, with its size given by @param
 * maxStrLen.
 */
MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, char *str,
                                     const char *description, size_t maxStrLen)
{
  clear(true, STRING);
  set(STRING, name, description);

  if (maxStrLen == 0)
  {
    myOwnPointedTo = true;

    if (myData.myStringData.myString)
      delete myData.myStringData.myString;
    myData.myStringData.myString = new std::string(str);
  }
  else
  {
    myData.myStringData.myString = str;
    myData.myStringData.myMaxStrLen = maxStrLen;
  }
} 

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, const char *str,
                                     const char *description)
{
  clear(true, STRING);
  set(STRING, name, description);

  myOwnPointedTo = true;
  delete myData.myStringData.myString;
  myData.myStringData.myString = new std::string(str);
}                                                     

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, std::string *strptr, const char *description)
{
  clear(true, CPPSTRING);
  set(CPPSTRING, name, description);
  myOwnPointedTo = true;
  myData.myCppStringData.myCppStringPtr = strptr;
}                    

/*
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
*/
MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, 
                                     MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor,
                                     MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *getFunctor,
                                     const char *description)
{
  clear(true, FUNCTOR);
  set(FUNCTOR, name, description);

  myData.myFunctorData.mySetFunctor = setFunctor;
  myData.myFunctorData.myGetFunctor = setGunctor;
}                                     

MVREXPORT MvrConfigArg::MvrConfigArg(const char *str, Type type)
{
  clear(true, type);
  if (type == DESCRIPTION_HOLDER)
  {
    myType = DESCRIPTION_HOLDER;
    myDescription = str;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrConfigArg: Bad type %d for '%s'", type,str);
  }
}

/*
 * This constructor is used for creating composite(LIST or LIST_HOLDER type)
 * parameters.
*/
MVREXPORT MvrConfigArg::MvrConfigArg(Type type, const char *name, const char *description)
{
  clear(true, type);
  set(type, name, description);
  if ((type == LIST) || (type == LIST_HOLDER) || (type == STRING_HOLDER))
  {
    myOwnPointedTo = true;
  }
}

/*
 * This constructor is useful for creating separators within a config section
*/
MVREXPORT MvrConfigArg::MvrConfigArg(Type type)
{
  clear(true, type);
  set(type, "", "");
}

MVREXPORT MvrConfigArg::MvrConfigArg(const char *name, const char *str)
{
  clear(true, STRING_HOLDER);
  set(STRING_HOLDER, name, "");

  myOwnPointedTo = true;

  if (myData.myStringData.myString)
    delete myData.myStringData.myString;
  myData.myStringData.myString = new std::string(str);
}

MVREXPORT MvrConfigArg::MvrConfigArg(const char *argName, const MvrConfigArg &arg)
{
  clear(true, arg.myType, ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT));
  copy(arg, false);
  myName = ((argName) ? argName : "");
}

MVREXPORT MvrConfigArg::MvrConfigArg(const MvrConfigArg &arg)
{
  clear(true, arg.myType, ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT));
  copy(arg, false);
}

MVREXPORT MvrConfigArg &MvrConfigArg::operator=(const MvrConfigArg &arg)
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

void MvrConfigArg::copy(const MvrConfigArg &arg, bool isDetach)
{
  /// clear any existing data, freeing memory as necessary
  clear(false, myType, ((myType == INT) ? myData.myIntData.myIntType : INT_NOT));

  /// If any type has changed, then set up the union for the new type.
  /// Note that the isDelete flat is set to false beacause pointers
  // may contain bogus information (from the previous type)
  if ((myType != arg.myType) || ((myType == INT) && (myData.myIntData.myIntType != arg.myData.myIntData.myINtType)))
  {
    clear(false, arg.myType, ((arg.myType == INT) ? arg.myData.myIntData.myIntType : INT_NOT), false);
  }
  set(arg.myType, arg.myName.c_str(), arg.myDescription.c_str());

  myDisplayName = arg.myDisplayName;

  setExtraExplanation(arg.myExtraExplanation.c_str());

  myOwnPointedTo = arg.myOwnPointedTo || isDetach;

  switch (arg.myType)
  {
    case INT:
      {
        myData.myIntData.myIntType = arg.myData.myIntData.myIntType;

        switch (myData.myIntData.myIntType)
        {
          case INT_INT:
            {
              if (myOwnPointedTo && arg.myData.myIntData.myIntPointer != NULL)
              {
                myData.myIntData.myIntPointer  = new int;
                *myData.myIntData.myIntPointer = *arg.myData.myIntData.myIntPointer;
              }
              else
              {
                myData.myIntData.myIntPointer = arg.myData.myIntData.myIntPointer;
              }
            }
            break;

          case INT_SHORT;
            {
              if (myOwnPointedTo && arg.myData.myIntData.myIntShortPointer != NULL)
              {
                myData.myIntData.myIntShortPointer  = new short;
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
                myData.myIntData.myIntUnsignedShortPointer  = new short;
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
                myData.myIntData.myIntUnsignedCharPointer  = new short;
                *myData.myIntData.myIntUnsignedCharPointer = *arg.myData.myIntData.myIntUnsignedCharPointer;
              }
              else
              {
                myData.myIntData.myIntUnsignedCharPointer = arg.myData.myIntData.myIntUnsignedCharPointer;
              }
            }
            break;  

          case INT_NOE:
            {
              MvrLog::log(MvrLog::Normal, "MvrConfig: Internal warning: MvrConfigArg has INT type, but integer subtype is INT_NOT! Can't copy pointer.");
            }                  
            break;
        }
        myData.myIntData.myMinInt = arg.myData.myIntData.myMinInt;
        myData.myIntData.myMaxInt = arg.myData.myIntData.myMaxInt;
      }
      break;
      
    case DOUBLE:
      {
        if (myOwnPointedTo && arg.myData.myDoubleData.myDoublePointer != NULL)
        {
          myData.myIntData.myDoublePointer  = new short;
          *myData.myIntData.myDoublePointer = *arg.myData.myIntData.myDoublePointer;
        }
        else
        {
          myData.myIntData.myDoublePointer = arg.myData.myIntData.myDoublePointer;
        }
        myData.myDoubleData.myMinDouble = arg.myData.myDoubleData.myMinDouble;
        myData.myDoubleData.myMaxDouble = arg.myData.myDoubleData.myMaxDouble;
        myData.myDoubleData.myPrecision = arg.myData.myDoubleData.myPrecision;
      }
      break;  

    case STRING:
    case STRING_HOLDER:
      {
        myOwnPointedTo = arg.myOwnPointedTo;

        if (myOwnPointedTo)
        {
          myData.myStringData.myStringPointer = NULL;

          if (!arg.myOwnPointedTo && (arg.muyData.myStringData.myStringPointer != NULL){
            if (myData.myStringData.myString)
              delete myData.myStringData.myString;
            myData.myStringData.myString = new std::string(myData.myStringData.myStringPointer);
          }
          else
          {
            if (myData.myStringData.myString)
              delete myData.myStringData.myString;
            if (arg.myData.myStringData.myString != NULL){
              myData.myStringData.myString = new std::string(*arg.myData.myStringData.myString);
            }
            else
            {
              myData.myStringData.myString = NULL;
            }
          }
        } 
        else
        {
          myData.myStringData.myStringPointer = arg.myData.myStringData.myStringPointer;

          if (myData.myStringData.myString != NULL)
            delete myData.myStringData.myString;
          if (arg.myData.myStringData.myString != NULL)
          {
            mydata.myStringData.myString = new std::string(*arg.myData.myStringData.myString);
          }
          else
          {
            myData.myStringData.myString = NULL;
          }
        } 
        mydata.myStringData.myMaxStrLen  = arg.mydata.myStringData.myMaxStrLen;
      }   
      break;

    case CPPSTRING:
      myOwnPointedTo = arg.myOwnPointedTo;
      if (myOwnPointedTo)
      {
        if (myData.myCppStringData.myCppStringPtr)
          delete myData.myCppStringData.myCppStringPtr;
        if (arg.myData.myCppStringData.myCppStringPtr != NULL)
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
          myData.myBoolData.myBoolPointer  = new bool;
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

        if (arg.myData.myListData.myChildArgList != NULL)
        {
          for (std::list<MvrConfigArg>::const_iterator iter = arg.myData.myListData.myChildArgList->begin();
               iter != arg.myData.myListData.myChildArgList->end();
               iter++)
          {
            addArg(*iter);
          }               
        }
      }
      break;

    case INVALID:
    case DESCRIPTION_HOLDER:
    case SEPARATOR:
      break;
    };
  
  myConfigPriority = arg.myConfigPriority;
  myIgnoreBounds   = arg.myIgnoreBounds;
  myDisplayHint    = arg.myDisplayHint;
  myRestartLevel   = arg.myRestartLevel;

  myValueSet       = arg.myValueSet;
  mySuppressChanges= arg.mySuppressChanges;
  myIsTranslated   = arg.myIsTranslated;
  myIsSerializable = arg.myIsSerializable;
}

/*
 * The translation data currently consists of the description and the extra explanation
 */
MVREXPORT bool MvrConfigArg::copyTranslation(const MvrConfigArg &arg)
{
  if (MvrUtil::strcasecmp(getName(), arg.getName()) != 0)
  {
    MvrLog::log(MvrLogL::Normal,
                "MvrConfigArg::copyTranslation() names jdo not match this = %s arg = %s",
                getName(),
                arg.getName());
    return false;                
  }
  setDescription(arg.getDescription());
  setExtraExplanation(arg.getExtraExplanation());

  if (isListType() && arg.isListType())
  {
    for (size_t i=0; i<getArgCount(); i++)
    {
      MvrConfigArg *childArg = getArg(i);
      if (childArg == NULL)
        continue;
      const MvrConfigArg *xlateChildArg = arg.findArg(childArg->getName());
      if (xlateChildArg != NULL)
      {
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfigArg::copyTranslation() found translation for %s child %s",
                            getName(), childArg->getName()));
        childArg->copyTranslation(*xlateChildArg);                          
      }
      else
      {
        MvrLog::log(MvrLog::Normal,
                    "MvrConfigArg::copyTranslation() did not find translation for %s child %s",
                    getName(), childArg->getName());
      }
    }
  }
  myIsTranslated = true;
  return true; 
}

MVREXPORT bool MvrConfigArg::promoteList(const MvrConfigArg &arg)
{
  if ((getType() != LIST_HOLDER) || (arg.getType() != LIST))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::promoteList() incorrect type for %s",
                arg.getName());
    return false;                
  }
  myType             = arg.myType;
  myName             = arg.myName;
  myDescription      = arg.myDescription;
  myDisplayName      = arg.myDisplayName;
  myExtraExplanation = arg.myExtraExplanation;

  myConfigPriority   = arg.myConfigPriority;
  myDisplayHint      = arg.myDisplayHint;
  myRestartLevel     = arg.myRestartLevel;
  
  myOwnPointedTo     = arg.myOwnPointedTo;
  myValueSet         = arg.myValueSet;
  myIgnoreBounds     = arg.myIgnoreBounds;
  myIsTranslated     = arg.myIsTranslated;
  mySuppressChanges  = arg.mySuppressChanges;
  myIsSerializable   = arg.myIsSerializable;

  return true;
}

MVREXPORT MvrConfigArg::~MvrConfigArg()
{
  clear(false, myType, ((myType == INT) ? myData.myIntData.myIntType : INT_NOT));
}

/*
 * @param initial a bool set to true if this is the initial construciton of the
 * arg, and member fields need to be initialized
 * @param type the Type of the arg
 * @param intType the IntType of the arg, applicable only when type is INT
 * @param isDelete a bool set to true (the default) if clear should free any 
 * used memory; false if the fields should merely be initialized.  The latter
 * is used when the type of an arg needs to be changed (for example, from a 
 * STRING_HOLDER to a "real" value).
 */
MVREXPORT MvrConfigArg::clear(bool initial, Type type, IntType intType, isDelete)
{
  if (initial)
  {
    myOwnPointedTo = false;
    myIsTranslated = false;

    switch (type)
    {
      case INT:
        switch (intType)
          {
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
    }

    myParentArg = NULL;

    set(type, "", "");
  }

  set (type, "", "");

  switch (type)
  {
    case INT:
      myData.myIntData.myIntType = intType;

      switch (myData.myIntData.myIntType)
      {
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
          break; // some question
      }
      myData.myIntData.myMinInt = INT_MIN;
      myData.myIntData.myMaxInt = INT_MAX;
      break;
    case DOUBLE:
      if (isDelete && myOwnPointedTo && myData.myDoubleData.myDoublePointer != NULL)
        delete myData.myDoubleData.myDoublePointer;
      myData.myDoubleData.myDoublePointer = NULL;
      myData.myDoubleData.myMinDouble     = -HUGE_VAL;
      myData.myDoubleData.myMaxDouble     = HUGE_VAL;
      myData.myDoubleData.myPrecision     = DEFAULT_DOUBLE_PRECISION;
      break;
    case STRING:
    case STRING_HOLDER:
      myData.myStringData.myStringPointer = NULL;
      myData.myStringData.myMaxStrLen     = 0;

      myOwnPointedTo                      = false;

      if (isDelete && myData.myStringData.myString != NULL)
        delete myData.myStringData.myString;
      myData.myStringData.myString        = NULL;
      break;
    
    case CPPSTRING:
      if (isDelete && myOwnPointedTo && myData.myCppStringData.myCppStringPtr)
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
      if (isDelete && myData.myListData.myChildArgList)
        delete myData.myListData.myChildArgList;
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
  }
  myConfigPriority = MvrPriority::NORMAL;
  myIgnoreBounds   = false;
  myDisplayHint    = "";
  myRestartLevel   = NO_RESTART;

  myValueSet       = false;

  myParentArg      = NULL;

  myOwnPointedTo   = false;
  myIsTranslated   = false;
  mySuppressChanges= false;
  myIsSerializable = false;
}

MVREXPORT void MvrConfigArg::replaceSpacesInName(void)
{
  size_t i;
  size_t len = myName.size();

  for (i=0; i<len; i++)
  {
    if (isspace(myName[i]))
      myName[i] = "_";
  }
}

void MvrConfigArg::set(MvrConfigArg::Type type,
                       const char *name,
                       const char *description,
                       IntType intType)
{
  myType = type;
  if (myType == INT)
  {
    myData.myIntData.myIntType = intType;
  }
  myName = name;
  myDescription = description;

  myDisplayName = "";
}                       

/*
 * @param parentArg the MvrConfigArg * to be stroed as the parent
 */
void MvrConfigArg::setParent(Mvrconfig *parentArg)
{
  myParentArg = parentArg;
}

/*
 * @return Type the type of this arg, used to determine which other attributes are valid
 */
MVREXPORT MvrConfigArg::Type MvrConfigArg::getType(void) const
{
  return myType;
}

/*
 * @return char *, the name of the arg or "" if none
 */
MVREXPORT const char *MvrConfigArg::getName(void) const
{
  return myName.c_str();
}

/*
 * @return char *, the description of the arg or "" if none
 */
MVREXPORT const char *MvrConfigArg::getDescription(void) const
{
  return myDescription.c_str();
}

MVREXPORT void MvrConfigArg::setExtraExplanation(const char *extraExplanation)
{
  myExtraExplanation = ((extraExplanation != NULL) ? extraExplanation : "");
}

MVREXPORT const char *MvrConfigArg::getExtraExplanation() const
{
  return myExtraExplanation.c_str();
}

/****************************************************************************
                          Method for INT Type
****************************************************************************/
MVREXPORT int MvrConfigArg::getInt(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == INT);
  }

  if (myType == INT)
  {
    switch (myData.myIntDat.myIntType)
    {
      case INT_NOT:
      case INT_INT:
        // only one of these will be valid
        if (myData.myIntDat.myIntPointer != NULL)
          return *myData.myIntDat.myIntPointer;
        break;
      case INT_SHORT:
        if (myData.myIntDat.myIntShortPointer != NULL)
          return *myData.myIntDat.myIntShortPointer;
        break;     
      case INT_UNSIGNED_SHORT:
        if (myData.myIntDat.myIntUnsignedShortPointer != NULL)
          return *myData.myIntDat.myIntUnsignedShortPointer;
        break;          
      case INT_UNSIGNED_CHAR:
        if (myData.myIntDat.myIntUnsignedCharPointer != NULL)
          return *myData.myIntDat.myIntUnsignedCharPointer;
        break;          
    }
  }
  return 0;
}

/*
 * This method is applicable only when getType() returns INT.
 * @param  ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return int the minimum value of this arg, or INT_MIN if 
 * none
**/
MVREXPORT int MvrConfigArg::getMinInt(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == INT);
  }
  if (myType == INT)
  {
    return myData.myIntData.myMinInt;
  }

  return INT_MIN;
}

/*
 * This method is applicable only when getType() returns INT.
 * @param ok an optional pointer to a bool that can be set by 
 * this method to indicate success (true) or failure (false)
 * @return int the maximum value of this arg, or INT_MAX if 
 * none
**/
MVREXPORT int MvrConfigArg::getMaxInt(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == INT);
  }
  if (myType == INT)
  {
    return myData.myIntData.myMaxInt;
  }

  return INT_MAX;
}

MVREXPORT bool MvrConfigArg::setrInt(int val, char *errorBuffer,
                                     size_t errorBufferLem, bool doNotSet)
{
  if (myType != INT)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::setInt() cannot set %s to %d, wrong type (%s)",
                getName(), val, toString(myType));
    return false;
  }

  myValueSet = true;
  if (!myIgnoreBounds && val < myData.myIntData.myMinInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: SetInt value %d below range [%d, %d]",
                getName(), val, myData.myIntData.myMinInt, myData.myIntData.myMaxInt);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %d is below minimum of %d."
               getName(), val, myData.myIntData.myMinInt);
    return false;
  }
  if (!myIgnoreBounds && val > myData.myIntData.myMaxInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: SetInt value %d above range [%d, %d]",
                getName(), val, myData.myIntData.myMinInt, myData.myIntData.myMaxInt);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %d is above maximum of %d."
               getName(), val, myData.myIntData.myMaxInt);
    return false;    
  }
  if ((myData.myIntData.myIntType == INT_INT && myData.myIntData.myIntPointer == NULL) ||
      (myData.myIntData.myIntType == INT_SHORT && myData.myIntData.myIntShortPointer == NULL) ||
      (myData.myIntData.myIntType == INT_UNSIGNED_SHORT && myData.myIntData.myIntUnsignedShortPointer == NULL) ||
      (myData.myIntData.myIntType == INT_UNSIGNED_CHAR && myData.myIntData.myIntUnsignedCharPointer == NULL))
  {
    MvrLog::log(MvrLog::Normal, "MvrConfigArg of %s: setInt called with NULL int pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;
  }

  if (!doNotSet)
  {
    switch (myData.myIntData.myIntType)
    {
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
        MvrLog::log(MvrLog::Normal,
                    "MvrConfigArg of %s: int is bad type.",getName());
        if (errorBuffer != NULL)
          snprintf(errorBuffer, errorBufferLen, "%s int is bad type (%d).", getName(), myData.myIntData.myIntType);
        return false;                
    }
  }
  return true;
}

/****************************************************************************
                          Method for DOUBLE Type
****************************************************************************/
MVREXPORT double MvrConfigArg::getDouble(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == DOUBLE);
  }

  if (myType == DOUBLE)
  {
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
 */
MVREXPORT double MvrConfigArg::getMinDouble(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == DOUBLE);
  }

  if (myType == DOUBLE)
  {
    return myData.myDoubleData.myMinDouble;
  }
  return -HUGE_VAL;
}

MVREXPORT int MvrConfigArg::getDoublePrecision(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == DOUBLE);
  }  
  if (myType == DOUBLE)
  {
    return myData.myDoubleData.myPrecision;
  }
  return 0;
}  

MVREXPORT bool MvrConfigArg::setDouble(double val, char *errorBuffer,
                                       size_t errorBufferLen, bool doNotSet)
{
  if (myType != DOUBLE)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::setDouble() cannot set %s to %g, wrong type (%s)",
                getName(), val, toString(myType));
    return false;                
  }
  
  myValueSet = true;
  if (!myIgnoreBounds && val < myData.myDoubleData.myMinDouble)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg of %s: setDouble value %g below range [%g, %g]",
                getName(), myDoubleData.myMinDouble, myData.myDoubleData.myMaxDouble);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %g is below minimum of %g",
               getName(), val, myData.myDoubleData.myMinDouble);
    return false;               
  }
  if (!myIgnoreBounds && val > myData.myDoubleData.myMinDouble)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg of %s: setDouble value %g above range [%g, %g]",
                getName(), myDoubleData.myMinDouble, myData.myDoubleData.myMaxDouble);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s value of %g is above maximum of %g",
               getName(), val, myData.myDoubleData.myMaxDouble);
    return false;               
  }  
  if (myData.myDoubleData.myDoublePointer == NULL)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg of %s: setDouble called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL",getName());
    return false;               
  }  
  // if we got to here we're good
  if (!doNotSet)
    *myData.myDoubleData.myDoublePointer = val;
  return true;
}             

/****************************************************************************
                          Method for BOOL Type
****************************************************************************/
MVREXPORT bool MvrConfigArg::getBool(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = (myType == BOOL);
  }

  if (myType == BOOL)
  {
    if (myData.myDoubleData.myBoolPointer != NULL)
          return *myData.myDoubleData.myBoolPointer;
  }
  return 0;
}

MVREXPORT bool MvrConfigArg::setBool(bool val, char *errorBuffer,
                                     size_t errorBufferLen, bool doNotSet)
{
  if (myType != DOUBLE)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::setBool() cannot set %s to %i, wrong type (%s)",
                getName(), val, toString(myType));
    return false;                
  }
  
  myValueSet = true;
  if (myData.myBoolData.myBoolPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg of %s: setBool value called with NULL pointer", getName);
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL", getName());
    return false;               
  }
  // if we got to here we're good
  if (!doNotSet)
    *myData.myBoolData.myBoolPointer = val;
  return true;
}             

/****************************************************************************
                  Method for STRING and STRING_HOLDER Type
****************************************************************************/
MVREXPORT const char *MvrConfigArg::getString(bool *ok) const
{
  if (ok != NULL)
  {
    *ok = ((myType == STRING) || (myType == STRING_HOLDER) || (myType == CPPSTRING))
  }
  if ((myType == STRING) || (myType == STRING_HOLDER))
  {
    if (myOwnPointedTo)
    {
      if (myData.myStringData.myString != NULL)
      {
        return myData.myStringData.myString.c_str();
      }
      else
      {
        return "";
      }
    }
    else if (myData.myStringData.myStringPointer != NULL)
      return myData.myStringData.myStringPointer;

  }
  else if (myType == CPPSTRING)
    return getCppString().c_str();
  return NULL;
}

/*
 * @warning The value must not contain any characters with
 * special meaning when saved and loaded from a config file, such as '#', ';',
 * tab, or newline.  
 */
 MVREXPORT bool MvrConfigArg::setString(const char *str, char *errorBuffer,
                                        size_t errorBufferLen, bool doNotSet)
{
  if ((myType != STRING) && (myType != STRING_HOLDER))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::setString() cannot set %s to %s, wrong type (%s)",
                getName(), str, toString(myType));
    return false;
  }
  myValueSet = true;
  size_t len;

  if (myOwnPointedTo)
  {
    if (myData.myStringData.myString)
      delete myData.myStringData.myString;
    myData.myStringData.myString = new std::string(str);
    return true;
  }
  if (myData.myStringData.myStringPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg of %s: setString called with NULL pointer.", getName());
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s string is %lu long when max length is %lu.",getName(), len, myData.myStringData.myMaxStrLen);
    return false;  
  }
  if (!doNotSet)
  {
    strcpy(myData.myStringData.myStringPointer, str);
  }
  return true;
}                                        


/****************************************************************************
                          Method for CPPSTRING Type
****************************************************************************/
/// Get a copy of the value of the target std::string. (If there is a NULL
/// target, return "" and set @param ok to false if @param ok is not NULL.)
MVREXPORT std::string MvrConfigArg::getCppString(bool *ok) const
{
  if (myData.myCppStringData.myCppStringPtr == NULL)
  {
    MvrLog::log(MvrLog::Verbose,
                "MvrConfigArg::getCppString: internal std::string pointer for argument %s is NULL", getName());
    if (ok)
      *ok = false;
    return NULL;                
  }
  if (ok)
    *ok = true;
  return myData.myCppStringData.myCppStringPtr;
}

MVREXPORT std::string MvrConfigArg::setCppString(const std::string &str, char *errorBuffer, 
                                                 size_t errorBufferLen, bool doNotSet)
{
  if (myType != CPPSTRING)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::setCppString() cannot set %s to %s, wrong type (%s)",
                getName(), str.c_str(), toString(myType));
    return false;                 
  }

  if (myOwnPointedTo && !doNotSet)
  {
    if (myData.myCppStringData.myCppStringPtr != NULL)
      delete myData.myCppStringData.myCppStringPtr;
    myData.myCppStringData.myCppStringPtr = new std::string(str);
    myValueSet = true;
    return true;
  }

  if (myData.myCppStringData.myCppStringPtr == NULL)
  {
    MvrLog::log(MvrLog::Normal,
                "ArConfigArg of %s: setCppString called but std::string target pointer is NULL.", getName());    
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "%s pointer is NULL.", getName());
    return false;                
  }

  if (!doNotSet)
  {
    myData.myCppStringData.myCppStringPtr->assign(str);
    myValueSet = true;
  }
  return true;
}                                                 

/****************************************************************************
                          Method for LIST  Type
****************************************************************************/
/**
 * If the given child arg is not a separator, then it must have a unique, 
 * non-empty name.
 * 
 * @param arg the ArConfigArg to be added as a child of this composite arg
 * @return bool true if the child arg was successfully added; false if an 
 * error occurred
 */
MVREXPORT bool MvrConfigArg::addArg(const MvrConfigArg &arg)
{
  // Children cna only be added to LIST or LIST_HOLDER type parameters
  if (!isListType())
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::addArg() child %s not added to %s (type %s), type must be %s or %s",
                arg.getName(), getName(), toString(getType()), toString(LIST), toString(LIST_HOLDER));
    return false;                
  }

  // Children must own their own data. They cannot store pointer to members of other objects
  MvrConfigArg newChildArg(arg);

  MvrConfigArg *existingChildArg = NULL;
  if (!MvrUtil::isStrEmpty(arg.getName())
  {
    existingChildArg = findArg(arg.getName());
  }
  if (arg.hasExternalDataReference())
  {
    if (!existingChildArg->isPlaceholder() && existingChildArg->isSerializable())
    {
      MvrLog::log(MvrLog::Normal,
                  "MvrConfigArg::addArg() child %s not added to %s, duplicate exists",
                  arg.getName(), getName());
      return false;                  
    }
    else if (existingChildArg->getType() == LIST_HOLDER)
    {
      if (newChildArg.getArgCount() == 0){
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfigArg::addArg() child %s promoted to list in %s",
                            arg.getName(), getName()));
        existingChildArg->promoteList(arg);      

        newChildArg = *existingChildArg                      ;

        if (!removeArg(*existingChildArg))
        {
          MvrLog::log(MvrLog::Normal,
                      "MvrConfigArg::addArg() error removing existing list %s from %s",
                      existingChildArg->getName(), getName());
        }
      }
    }
    else if (existingChildArg->getType() == STRING_HOLDER)
    {
       MvrArgumentBuilder holderBuilder;
       holderBuilder.add(existingChildArg->getString());

       bool isParseSuccessful = newChildArg.parseArgument(&holderBuilder, NULL, 0);

       if (isParseSuccessful)
       {
         if (!removeArg(*existingChildArg))
         {
            MvrLog::log(MvrLog::Normal,
                        "MvrConfigArg::addArg() error removing child %s from %s",
                        newChildArg->getName(), getName());           
         }
       }
       else
       {
          MvrLog::log(MvrLog::Normal,
                      "MvrConfigArg::addArg() child %s not added to %s, error parsing '%s'",
                      arg->getName(), getName(),existingChildArg->getString());
          return false;                                   
       }
    }
    else if (!existingChildArg->isSerializable())
    {
      bool isValueSet = existingChildArg->setValue(newChildArg);
      MvrLog::log(MvrLog::Normal,
                  "MvrConfigArg::addArg() child %s not serializable, setValue return %i",
                  existingChildArg->getName(), isValueSet);
      return isValueSet;
    }
  }
  // If the member list has not yet been allocated, then create it.
  if (myData.myListData.myChildArgList == NULL)
  {
    myData.myListData.myChildArgList = new std::list<MvrConfigArg>();

    // check for memory allocation errors
    if (myData.myListData.myChildArgList == NULL)
    {
      MvrLog::log(MvrLog::Normal,
                  "MvrConfig::AddArg() memory allocation error");
      return false;                  
    }
  }
  IFDEBUG(MvrLog::log(MvrLog::Normal,
                      "MvrLog::addArg() adding %s to %s",
                      newChildArg.getName(),
                      getName()));
  myData.myListData.myChildArgList->push_back(newChildArg);
  // Modify the parent of the arg that has actually been stored.
  myData.myListData.myChildArgList->back().setParent(this);

  return true;                      
}

MVREXPORT bool MvrConfigArg::removeArg(const MvrConfigArg &arg)
{
  // children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType)
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::removeArg() child %s not removed from %s (type %s), type must be %s or %s",
                arg.getName(), getName(), toString(getType()), toString(LIST), toString(LIST_HOLDER));
    return false;                
  }
  if (MvrUtil::isStrEmpty(arg.getName()))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::removeArg() child not removed from %s, empty name", getName());
    return false;                
  }
  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       iter != myData.myListData.myChildArgList->end();
       iter++)
  {
    if (MvrUtil::strcasecmp((*iter).getName(), arg.getName()) == 0)
    {
      myData.myListData.myChildArgList->erase(iter);
      return true;
    } 
  }
  return false;
}

/*
 * @return bool set to true if this is a list arg that contains child args; false, otherwise
 */
MVREXPORT bool MvrConfigArg::hasArgs() const
{
  // Children can only added to LIST to LIST_HOLDER type parameters
  if ((!isListType()) || (myData.myListData.myChildArgList == NULL))
  {
    return 0;
  }
  return !myData.myListData.myChildArgList->empty();
}

/*
 * @return int the number of child args in this list arg; or 0 if none.
 */
MVREXPORT size_t MvrConfigArg::getArgCount() const
{
    // Children can only added to LIST to LIST_HOLDER type parameters
  if ((!isListType()) || (myData.myListData.myChildArgList == NULL))
  {
    return 0;
  }
  return myData.myListData.myChildArgList->size();
}

MVREXPORT size_t MvrConfigArg::getDescendantArgCount() const
{
  // Children can only added to LIST to LIST_HOLDER type parameters
  if ((!isListType()) || (myData.myListData.myChildArgList == NULL))
  {
    return 0;
  }

  int count = 0;
  for (std::list<MvrConfigArg>::const_iterator iter = myData.myListData.myChildArgList->begin();
       iter != myData.myListData.myChildArgList->end();
       iter++)
  {
    count++;
    count += (*iter).getDescendantArgCount();
  }       
  return count;
}


/*
 * @return std::list<ArConfigArg> a list of all of the child arg (copies) 
 * in this arg.
 */
MVREXPORT std::list<MvrConfigArg> MvrConfigArg::getArgs(bool *ok) const
{
  std::list<MvrConfigArg> retList;

  if (ok != NULL)
  {
    *ok = isListType();
  }
  if ((isListType()) && (myData.myListData.myChildArgList != NULL))
  {
    retList = *myData.myListData.myChildArgList;
  }
  return retList;
} 

/*
 * @return std::list<ArConfigArg> a list of all of the child arg (copies) 
 * in this arg.
 */
MVREXPORT const MvrConfigArg *MvrConfigArg::getArg(size_t index) const
{
  MvrConfigArg *arg = NULL;
  if ((isListType()) && (myData.myListData.myChildArgList != NULL))
  {
    size_t curIndex = 0;
    for (std::list<MvrConfigArg>::const_iterator iter = myData.myListData.myChildArgList->begin();
         iter != myData.myListData.myChildArgList->end();
         iter++)
    {
      if (curIndex == index)
      {
        arg = &(*iter);
        break;
      }
    }         
  }
  return arg;
}

MVREXPORT MvrConfigArg *MvrConfigArg::getArg(size_t index) 
{
  MvrConfigArg *arg = NULL;
  if ((isListType()) && (myData.myListData.myChildArgList != NULL))
  {
    size_t curIndex = 0;
    for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
         iter != myData.myListData.myChildArgList->end();
         iter++)
    {
      if (curIndex == index)
      {
        arg = &(*iter);
        break;
      }
    }         
  }
  return arg;  
}

/*
 * This method performs a case-insensitive search of the child arg names.
 *
 * @param childParamName the char * name of the child arg to be retrieved; must be
 * non-empty
 * @return ArConfigArg * a pointer to the requested child arg; or NULL if not 
 * found.
 */
MVREXPORT const MvrConfigArg *MvrConfigArg::findArg(const char *childParamName) const
{
  // cannot look up empty name
  if (MvrUtil::isStrEmpty(childParamName))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::findArg() cannot find child with empty name in arg %s",
                getName());
    return NULL;
  }
  if (isListType())
  {
    MvrLog::log(MvrLog::Normal, 
                "MvrConfigArg::findArg() cannot find child %s in arg %s (type %s), type must be %s or %s",
                childParamName, getName(), toString(getType()), toString(LIST), toString(LIST_HOLDER));
    return NULL;
  }

  // If empty list, return NULL
  if (myData.myListData.myChildArgList == NULL)
  {
    return NULL;
  }
  MvrConfigArg *child = NULL;

  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       ((iter != myData.myListData.myChildArgList->end()) && (child == NULL));
       iter++)
  {
    MvrConfigArg &curArg = *iter;
    if (!MvrUtil::isStrEmpty(curArg.getName()) &&
        (MvrUtil::strcasecmp(childParamName, curAge.getName()) == 0))
    {
      child = &curArg;
    }
  }       
  return child;
}

/*
 * This method performs a case-insensitive search of the child arg names.
 *
 * @param childParamName the char * name of the child arg to be retrieved; must be
 * non-empty
 * @return MvrConfigArg * a pointer to the requested child arg; or NULL if not 
 * found.
 */
MVREXPORT MvrConfigArg *MvrConfig::findArg(const char *childParamName)
{
  // cannot look up empty name
  if (MvrUtil::isStrEmpty(childParamName))
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::findArg() cannot find child with empty name in arg %s",
                getName());
    return NULL;
  }
  // Children can only be added to LIST or LIST_HOLDER type parameters
  if (!isListType())
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigArg::findArg() cannot find child %s in arg %s (type %s), type must be %s or %s",
                childParamName, getName(), toString(getType()), toString(LIST), toString(LIST_HOLDER));
    return NULL;
  }

  // If empty list, return NULL
  if (myData.myListData.myChildArgList == NULL)
  {
    return NULL;
  }

  MvrConfigArg *child = NULL;

  for (std::list<MvrConfigArg>::iterator iter = myData.myListData.myChildArgList->begin();
       ((iter != myData.myListData.myChildArgList->end()) && (child == NULL));
       iter++)
  {
    MvrConfigArg &curArg = *iter;
    if (!MvrUtil::isStrEmpty(curArg.getName()) &&
        (MvrUtil::strcasecmp(childParamName, curAge.getName()) == 0))
    {
      child = &curArg;
    }
  }
  return child;
}

