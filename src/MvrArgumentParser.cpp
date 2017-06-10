/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArgumentParser.cpp
 > Description  : Base class for device connections
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月10日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "MvrLog.h"
#include "mvriaUtil.h"

#include <stdarg.h>

std::list<std::string> MvrArgumentParser::ourDefaultArgumentLocs;
std::list<bool> MvrArgumentParser::ourDefaultArgumentLocIsFile;
/*
   @param argc pointer to program argument count
   @param argv array of program arguments
 */
MVREXPORT MvrArgumentParser::MvrArgumentParser(int *argc, char **argv)
{
  myArgc = argc;
  myArgv = argv;
  myUsingBuilder = false;
  myBuilder      = NULL;
  myOwnBuilder   = false;
  myReallySetOnlyTrue = false;
  myEmptyArg[0]   = '\0';
  myHelp = false;
}
/*
 * @param builder an MvrArgumentBuilder object containing arguments
 */
MVREXPORT MvrArgumentParser::MvrArgumentParser(MvrArgumentBuilder *builder)
{
  myUsingBuilder = false;
  myBuilder      = builder;
  myOwnBuilder   = false;
  myReallySetOnlyTrue = false;
  myEmptyArg[0]   = '\0';
  myHelp = false;
}

MVREXPORT MvrArgumentParser::~MvrArgumentParser()
{
  if (myBuilder)
  {
    delete myBuilder;
    myBuilder = NULL;
  }
}

MVREXPORT bool MvrArgumentParser::checkArgumentVar(const char *argument, ...)
{
  size_t i;
  std::string extraHyphen;
  extraHyphen  = "-";
  extraHyphen += argument;
  for (i=0; i< getArgc(); i++)
  {
    if (strcasecmp(argument, getArgv()[i]) == 0 ||
        strcasecmp(extraHyphen.c_str(), getArgv()[i]) == 0)
    {
      removeArg(i);
      return true;
    }
  }
  return false;
}
/*
   This is like checkParameterArgument but lets you fail out if the
   argument is there but the parameter for it is not

   @param argument the string to check for, if the argument is found
   its pulled from the list of arguments

   @param dest if the parameter to the argument is found then the dest
   is set to the parameter
   @param wasReallySet the target of this pointer is set to true if

   @param ... the extra string to feed into the argument for
   parsing

   @return true if either this argument wasn't there or if the
   argument was there with a valid parameter
 */
MVREXPORT bool MvrArgumentParser::checkParameterArgumentStringVar(const char *argument, 
                                                                  const char **dest, 
                                                                  bool *wasReallySet, 
                                                                  bool returnFirst)
{
  char *param;
  param = checkArgumentVar(argument, returnFirst);

  if (param == NULL)
  {
    if (wasReallySet && !myReallySetOnlyTrue)
      *wasReallySet = false;
    return true;
  }
  else if (param[0] != '\0')
  {
    *dest = param;
    if (wasReallySet)
      *wasReallySet = true;
    return true;
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "No argument given to %s", argument);
    return false;
  }
}                                                                   

/*
 * This is like checkParameterArgument but lets you fail out if the
 * argument is there but the parameter for it is not
 */
MVREXPORT bool MvrArgumentParser::checkParameterArgumentStringVar(bool *wasReallySet, 
                                                                  bool *dest,
                                                                  const char *argument, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg),argument, prt);
  va_end(ptr);
  return checkParameterArgumentBool(arg, dest, wasReallySet);
}                                                                  

/*
   This is like checkParameterArgument but lets you fail out if the
   argument is there but the parameter for it is not
*/
MVREXPORT bool MvrArgumentParser::checkParameterArgumentBool(const char *argument,
                                                             bool *dest,
                                                             bool *wasReallySet,
                                                             bool returnFirst)
{
  char *param;
  param = checkParameterArgument(argument, returnFirst);

  if (param == NULL)
  {
    if (wasReallySet && !myReallySetOnlyTrue)
    {
      *wasReallySet = false;
      return false;
    }
  }
  else if (param[0] != '\0')
  {
    if (strcasecmp(param, "true") == 0 || strcasecmp(param, "1") == 0)
    {
      *dest = true;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
    else if (strcasecmp(param, "false") == 0 || strcasecmp(param, "0") == 0)
    {
      *dest = false;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
    else
    {
      MvrLog::log(MvrLog::Normal,
                  "Argument given to %s was not a bool (true, false, 1, 0) it was the string %s",
                  argument, param);
      return false;
    }
  }
  else
  {
  MvrLog::log(MvrLog::Normal,"No argument given to %s", argument);
  return false;
  }  
}                                                             

/*
   This is like checkParameterArgument but lets you fail out if the
   argument is there but the parameter for it is not
*/
MVREXPORT bool MvrArgumentParser::checkParameterArgumentIntegerVar(bool *wasReallySet, int *dest, const char *argument, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg), argument, ptr);
  va_end(ptr);
  return checkParameterArgumentInteger(arg, dest, wasReallySet);
}

/*
   This is like checkParameterArgument but lets you fail out if the
   argument is there but the parameter for it is not
*/
MVREXPORT bool MvrArgumentParser::checkParameterArgumentInteger(const char *argument, int *dest, bool *wasReallySet, bool returnFirst)
{
  char *param;
  char *endPtr;
  int intVal;

  param = checkParameterArgument(argument, returnFirst);

  if (param == NULL)
  {
    if (wasReallySet && !myReallySetOnlyTrue)
      *wasReallySet = false;
    return true;
  }
  else if (param[0] != '\0')
  {
    intVal = strol(param, &endPtr, 10);
    if (endPtr[0] == '\0')
    {
      *dest = intVal;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, "Argument given to %s was not an interger it was the string %s",
                  argument, param);
      return false;
    }
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "No argument given to %s", argument);
    return false;
  }
}   

MVREXPORT bool MvrArgumentParser::checkParameterArgumentIntegerVar(bool *wasReallySet, int *dest, const char *argument, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg), argument, ptr);
  va_end(ptr);
  return checkParameterArgumentFloat(arg, dest, wasReallySet);
}

/*
   This is like checkParameterArgument but lets you fail out if the
   argument is there but the parameter for it is not
*/
MVREXPORT bool MvrArgumentParser::checkParameterArgumentFloat(const char *argument, int *dest, bool *wasReallySet, bool returnFirst)
{
  char *param = checkParameterArgument(argument, returnFirst);

  if (param == NULL)
  {
    if (wasReallySet && !myReallySetOnlyTrue)
      *wasReallySet = false;
    return true;
  }
  else if (param[0] != '\0')
  {
    char *endPtr;
    float floatVal = strtod(param, &endPtr);
    if (endPtr == param)
    {
      MvrLog::log(MvrLog::Normal, "Argument given with %s was not a valid number", argument);
      return false;
    }
    else
    {
      *dest = floatVal;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "No argument given to %s", argument);
    return false;    
  }
}  
MVREXPORT bool MvrArgumentParser::checkParameterArgumentDoubleVar(bool *wasReallySet, double *dest, const char *argument, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg), argument, ptr);
  va_end(ptr);
  return checkParameterArgumentDouble(arg, dest, wasReallySet);
}

MVREXPORT bool MvrArgumentParser::checkParameterArgumentDouble(const char *argument, int *dest, bool *wasReallySet, bool returnFirst)
{
  char *param = checkParameterArgument(argument, returnFirst);

  if (param == NULL)
  {
    if (wasReallySet && !myReallySetOnlyTrue)
      *wasReallySet = false;
    return true;
  }
  else if (param[0] != '\0')
  {
    char *endPtr;
    float doubleVal = strtod(param, &endPtr);
    if (endPtr == param)
    {
      MvrLog::log(MvrLog::Normal, "Argument given with %s was not a valid number", argument);
      return false;
    }
    else
    {
      *dest = doubleVal;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "No argument given to %s", argument);
    return false;    
  }
}

MVREXPORT char *MvrArgumentParser::checkParameterArgumentVar(const char *argument,...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg), argument, ptr);
  va_end(ptr);
  return checkParameterArgument(arg));
}

MVREXPORT char *MvrArgumentParser::checkParameterArgument(const char *argument, bool returnFirst)
{
  char *ret;
  char *retRecursive;
  size_t i;
  std::string extraHyphen;

  extraHyphen = "-";
  extraHyphen += argument;

  for (i=0; i<getArgc(); i++)
  {
    if (strcasecmp(argument, getArgv()[i]) == 0 || strcasecmp(extraHyphen.c_str(), getArgv()[i]) == 0)
    {
      if (getArgc() > i+1)
      {
        ret = getArgv()[i+1];
      }
      else
      {
        ret = myEmptyArg;
      }
      removeArg(i);

      if (ret != NULL && ret != myEmptyArg)
        removeArg(i);
      
      if (returnFirst)
      {
        return ret;
      }
      else if ((retRecursive = checkParameterArgument(argument)) != NULL)
      {
        return retRecursive;
      }
      else
      {
        return ret;
      }
    }
  }
  return NULL;
}

void MvrArgumentParser::removeArg(size_t which)
{
  if (which >= getArgc())
  {
    MvrLog::log(MvrLog::Terse, "MvrArgumentParser::removeArg: %d is greater than the number of arguments which is %d", which, getArgc());
    return ;
  }
  if (myUsingBuilder)
  {
    myBuilder->removeArg(which);
  }
  else
  {
    size_t i;
    for (i=which; i<getArgc()-1; i++)
    {
      myArgv[i] = myArgv[i+1];
    }
    *myArgc -= 1;
  }
}

