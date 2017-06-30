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
MVREXPORT bool MvrArgumentParser::checkParameterArgumentString(const char *argument,
                                                               const char **dest, 
                                                               bool *wasReallySet, 
                                                               bool returnFirst)
{
  char *param;
  param = checkParameterArgument(argument, returnFirst);

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
MVREXPORT bool MvrArgumentParser::checkParameterArgumentBoolVar(bool *wasReallySet, 
                                                                  bool *dest,
                                                                  const char *argument, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, argument);
  vsnprintf(arg, sizeof(arg),argument, ptr);
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
      *wasReallySet = false;
    return true;
  }
  else if (param[0] != '\0')
  {
    if (strcasecmp(param, "true") == 0 || strcmp(param, "1") == 0)
    {
      *dest = true;
      if (wasReallySet)
	      *wasReallySet = true;
      return true;
    }
    else if (strcasecmp(param, "false") == 0 || strcmp(param, "0") == 0)
    {
      *dest = false;
      if (wasReallySet)
	      *wasReallySet = true;
      return true;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, 
		              "Mvrgument given to %s was not a bool (true, false, 1, 0) it was the string %s",
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
    intVal = strtol(param, &endPtr, 10);
    if (endPtr[0] == '\0')
    {
      *dest = intVal;
      if (wasReallySet)
        *wasReallySet = true;
      return true;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, "Mvrgument given to %s was not an interger it was the string %s",
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

MVREXPORT bool MvrArgumentParser::checkParameterArgumentFloatVar(bool *wasReallySet, float *dest, const char *argument, ...)
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
MVREXPORT bool MvrArgumentParser::checkParameterArgumentFloat(const char *argument, float *dest, bool *wasReallySet, bool returnFirst)
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
      MvrLog::log(MvrLog::Normal, "Mvrgument given with %s was not a valid number", argument);
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

MVREXPORT bool MvrArgumentParser::checkParameterArgumentDouble(const char *argument, double *dest, bool *wasReallySet, bool returnFirst)
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
      MvrLog::log(MvrLog::Normal, "Mvrgument given with %s was not a valid number", argument);
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
  return checkParameterArgument(arg);
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

MVREXPORT size_t MvrArgumentParser::getArgc(void) const
{
  if (myUsingBuilder)
    return myBuilder->getArgc();
  else
    return *myArgc;
}

MVREXPORT char **MvrArgumentParser::getArgv(void) const
{
  if (myUsingBuilder)
    return myBuilder->getArgv();
  else
    return myArgv;
}
MVREXPORT const char *MvrArgumentParser::getArg(size_t whichArg) const
{
  if (whichArg >= getArgc())
    return NULL;
  else
    return getArgv()[whichArg];
}

MVREXPORT void MvrArgumentParser::log(void) const
{
  size_t i;
  MvrLog::log(MvrLog::Terse, "Num arguments: %d", getArgc());
  for (i=0; i<getArgc(); ++i)
  {
    MvrLog::log(MvrLog::Terse, "Mvrg %d: %s", i, getArgv()[i]);
  }
}

MVREXPORT const char *MvrArgumentParser::getStartingArguments(void) const
{
  if (myUsingBuilder)
    return myBuilder->getFullString();
  else
    return NULL; 
}

MVREXPORT void MvrArgumentParser::addDefaultArgument(const char *argument, int position)
{
  if (!myUsingBuilder)
  {
    myBuilder       = new MvrArgumentBuilder;
    myBuilder->addStringsAsIs(*myArgc, myArgv);
    myOwnBuilder    = true;
    myUsingBuilder  = true;
  }
  myBuilder->addPlain(argument, position);
}

/**
 * Search all locations for argument defaults and parse them.
 * These locations may be environment variables to read argument varues
 * from, or files to read. 
 */

MVREXPORT void MvrArgumentParser::loadDefaultArguments(int position)
{
  std::list<std::string>::iterator it;
  std::list<bool>::iterator bIt;
  const char *str;
  char *argumentsPtr;
  char arguments[100000];

  if (!myUsingBuilder)
    {
      myBuilder = new MvrArgumentBuilder;
      myBuilder->addStringsAsIs(*myArgc, myArgv);
      myOwnBuilder = true;
      myUsingBuilder = true;
    }

  for (it = ourDefaultArgumentLocs.begin(), 
        bIt = ourDefaultArgumentLocIsFile.begin();
       it != ourDefaultArgumentLocs.end(); 
       it++, bIt++)
  {
    str = (*it).c_str();
    // see if its an environmental variable
    if (!(*bIt) && (argumentsPtr = getenv(str)) != NULL)
    {
      MvrArgumentBuilder compressed;
      compressed.addPlain(argumentsPtr);
      compressed.compressQuoted(true);
      myBuilder->addStringsAsIs(compressed.getArgc(), compressed.getArgv(), 
                              position);
      MvrLog::log(MvrLog::Normal, 
		 "Added arguments from environmental variable '%s'", str);
    }
    // see if we have a file
    else if ((*bIt) && 
	     MvrUtil::getStringFromFile(str, arguments, sizeof(arguments)))
    {
      MvrArgumentBuilder compressed;
      compressed.addPlain(arguments);
      compressed.compressQuoted(true);
      myBuilder->addStringsAsIs(compressed.getArgc(), compressed.getArgv(), 
                              position);
      MvrLog::log(MvrLog::Normal, "Added arguments from file '%s'", 
		 str);
    }
    // the file or env didn't exit
    // this'll return true otherwise it'll return false)
    else
    {
      MvrLog::log(MvrLog::Verbose, 
		 "Could not load from environmental variable or file '%s'", 
		 str);
    }
  }
}
/*
  This adds a file to the list of default argument locations.
  @param file Name of the file
 */
MVREXPORT void MvrArgumentParser::addDefaultArgumentFile(const char *file)
{
  ourDefaultArgumentLocs.push_back(file);
  ourDefaultArgumentLocIsFile.push_back(true);
}

/**
   This adds an environment variable to the list of default argument
   locations.
   @param env Name of the environment variable 
 **/
 MVREXPORT void MvrArgumentParser::addDefaultArgumentEnv(const char *env)
 {
   std::list<std::string>::iterator it;
   std::list<bool>::iterator bIt;

   MvrLog::log(MvrLog::Normal, "Default argument files or environmental variables:");

  for (it=ourDefaultArgumentLocs.begin(), bIt=ourDefaultArgumentLocIsFile.begin();
       it!=ourDefaultArgumentLocs.end(); 
       it++, bIt++)
  {
    if (*bIt)
      MvrLog::log(MvrLog::Normal, "%10s%-10s%s", "", "file", (*it).c_str());
    else
      MvrLog::log(MvrLog::Normal, "%10s%-10s%s", "", "envVar", (*it).c_str());
  }
 }

 /**
 * Check whether a special -help flag was given in the arguments
 * The following are the help flags: -help, -h, --help, /?, /h.  
 * @return false if a help flag was found true otherwise.
 */
MVREXPORT bool MvrArgumentParser::checkHelp()
{
  if (myHelp || checkArgument("-help") || checkArgument("-h") ||
      checkArgument("/?") || checkArgument("/h"))
  {
    myHelp = true;
    return false;
  }
  return true;
}

/**
 * Check whether a special -help flag was given in the arguments. If a -help
 * flag was not given, also check whether any arguments remain unparsed by this
 * argument parser.
 */
MVREXPORT bool MvrArgumentParser::checkHelpAndWarnUnparsed(unsigned int numArgsOkay)
{
  if (!checkHelp())
    return false;
  
  if (getArgc() < 1 + numArgsOkay)
    return false;
  
  size_t i;
  char buf[2048];
  sprintf(buf, "Unhandled arguments to program:");
  for (i=1+(int)numArgsOkay; i< getArgc(); i++)
    sprintf(buf, "%s %s", buf, getArg(i));
  MvrLog::log(MvrLog::Normal, buf);
  MvrLog::log(MvrLog::Normal, "Program will continue but to see the help listing type '%s -help", getArg(0));

  return true;
}

MVREXPORT void MvrArgumentParser::setWasReallySetOnlyTrue(bool setWasReallySetOnlyTrue)
{
  myReallySetOnlyTrue = setWasReallySetOnlyTrue;
}

MVREXPORT bool MvrArgumentParser::getWasReallySetOnlyTrue(void)
{
  return myReallySetOnlyTrue;
}