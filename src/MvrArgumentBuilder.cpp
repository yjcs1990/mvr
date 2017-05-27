/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArgumentBuilder.cpp
 > Description  : Base class for device connections
 > Author       : Yu Jie
 > Create Time  : 2017年04月10日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrArgumentBuilder.h"
#include "MvrLog.h"
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

char *cppstrdup(const char *str)
{
  char *ret;
  ret = new char[strlen(str) + 1];
  strcpy(ret, str);
  return ret;
}

/*
 * @param argvLen the largest number of arguments to parse
 * @param extraSpaceChar if not NULL, then this character will also
 * be used to break arguments
 * @param ignoreNormalSpaces a bool set to true if only the extraSpaceChar
 * should be used to separate arguments
 * @param isPreCompresseQuotes a bool set to true if strings enclosed in 
 * double-quotes should be parsed as a single argument. This should roughly
 * equivalent to calling MvrArgumentBuilder::compressQuoted(false) on the resulting
 * builder. but is more efficient and hanles embeded space better. The defualt value 
 * is false and preserves the original behavior where each argument is a 
 * space-sepaerated alphanumeric string.
 */
 MVREXPORT MvrArgumentBuilder::MvrArgumentBuilder(size_t argvLen, char extraSpaceChar,
                                                  bool ignoreNormalSpaces, bool isPreCompresseQuotes)
{
  myArgc = 0;
  myOrigArgc = 0;
  myArgvLen = argvLen;
  myArgv = new char *[myArgvLen];
  myFirstAdd = true;
  myExtraSpace = extraSpaceChar;
  myIgnoreNormalSpaces = ignoreNormalSpaces;
  myIsPreCompresseQuotes = isPreCompresseQuotes;
  myIsQuiet = false;
};

MVREXPORT MvrArgumentBuilder::MvrArgumentBuilder(const MvrArgumentBuilder &builder)
{
  size_t i;
  myFullString = builder.myFullString;
  myExtraString = builder.myExtraString;
  myArgc = builder.getArgc();
  myArgvLen = builder.getArgvLen();
  myOrigArgc = myArgc;
  myArgv = new char *[myArgvLen];
  for(i=0; i<myArgc;i++)
    myArgv[i] = cppstrdup(builder.getArg(i));
  myIsQuiet = builder.myIsQuiet;
  myExtraSpace = builder.MyExtraSpace;
  myIgnoreNormalSpaces = builder.myIgnoreNormalSpaces;
  myIsPreCompressQuotes = builder.myIsPreCompressQuotes;
}

MVREXPORT MvrArgumentBuilder &MvrArgumentBuilder::operator=(const MvrArgumentBuilder &builder)
{
  if(this != &builder)
  {
    size_t i=0;

    /// Delete old stuff ...
    if(myOrigArgc > 0)
    {
      for(i=0; i<myOrigArgc; ++i)
        delete [] myArgv[i];
    }
    delete [] myArgv;

    /// Then copy new stuff...
    myFullString = builder.myFullString;
    myExtraString = builder.myExtraString;
    myArgc = builder.getArgc();
    myArgvLen = builder.getArgvLen();

    myOrigArgc = myArgc;
    myArgv = new char *[myArgvLen];
    for (i = 0; i < myArgc; i++) {
      myArgv[i] = cppstrdup(builder.getArg(i));
    }
    myIsQuiet = builder.myIsQuiet;
    myExtraSpace = builder.myExtraSpace;
    myIgnoreNormalSpaces = builder.myIgnoreNormalSpaces;
    myIsPreCompressQuotes = builder.myIsPreCompressQuotes;
  }
  return *this;
}

MVREXPORT MvrArgumentBuilder::~MvrArgumentBuilder()
{
  size_t i;
  if(myOrigArgc > 0)
  {
    for(i=0; i<myOrigArgc; ++i)
      delete [] myArgv[i];
  }
  delete [] myArgv;
}

MVREXPORT void MvrArgumentBuilder::removeArg(size_t which, bool isRebuildFullString)
{
  size_t i;
  char *temp;

  if(which < 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrArgumentBuilder::removeArg: cannot remove arg at negative index (%i)",
                which);
    return;
  }
  if(which>myArgc-1)
  {
    MvrLog::log(MvrLog::Terse, "MvrArgumentBuilder::removeArg: %d is greater than the number of arguments which is %d",
                which, myArgc)
    return;
  }

  temp = myArgv[which];
  //delete [] myArgv[which]
  for(i=which; i<myArgc-1; i++)
    myArgv[i] = myArgv[i+1];
  /// delete the one off the end
  myArgc = -1;
  /// Just stuffing the delete argument value at the end of the array
  myArgv[i] = temp;

  /// Note: It seems that compressQuoted calls removeArg and depends on it not
  /// changing the full string. Therefore, the parameter was added so that 
  /// the desired behavior could be specified by the caller
  if(isRebuildFullString)
    rebuildFullString();
}

MVREXPORT void MvrArgumentBuilder::add(const char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, sizeof(buf), str, ptr);
  internalAdd(buf, -1);
  va_end(ptr);
}

bool MvrArgumentBuilder::isSpace(char c)
{
  if((!myIgnoreNormalSpaces) && (isSpace(c))){
    return true;
  }
  else if((myExtraSpace != '\0') && (c == myExtraSpace)){
    return true;
  }
  return false;
} // end method isSpace

bool MvrArgumentBuilder::isStartArg(const char *buf, int len, int index, int *endArgFlagsOut)
{
  if(index<len){
    if(!isSpace(buf[index])){
      if((myIsPreCompressQuotes) && (buf[index] == '\"')){
        if(endArgFlagsOut != NULL){
          *endArgFlagsOut = ANY_SPACE | QUOTE;
        }
      }
      else {  //not precompressed quote
        if(endArgFlagsOut != NULL){
          /// This is set to ANY_SPACE to preserve the original behavior ... i.e. space
          /// charactre "matches" with myExtraSpace
          *endArgFlagsOut = ANY_SPACE;
        }
      }  // end else not precompressed quote
      return true;
    }  // end if not space
  } // end if not end of buf
  return false;
} // end method isStartArg

bool MvrArgumentBuilder::isEndArg(const char *buf, int len, int &index, int endArgFlags)
{
  if(index >= len)
    return true;
  if((endArgFlags &QUOTE) != 0){
    if(buf[index] == '\"'){
      if((index+1 >= len) || (buf[index+1] == '\0')){
        /// Quoteis not EOF
        index ++;
        return true;
      }
    } // end if quote found
  } // end if need to find quote
  else { // just looking for a space
    if(isSpace(buf[index])){
      return true;
    }
  }
  return false;
}  // end method isEndArg

