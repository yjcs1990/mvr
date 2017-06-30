/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArgumentBuilder.cpp
 > Description  : Base class for device connections
 > Author       : Yu Jie
 > Create Time  : 2017年04月10日
 > Modify Time  : 2017年05月27日
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

char * cppstrdup(const char *str)
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
 * @param isPreCompressQuotes a bool set to true if strings enclosed in 
 * double-quotes should be parsed as a single argument. This should roughly
 * equivalent to calling MvrArgumentBuilder::compressQuoted(false) on the resulting
 * builder. but is more efficient and handles embeded space better. The default value
 * is false and preserves the original behavior where each argument is a 
 * space-seperated alphanumeric string.
 */
 MVREXPORT MvrArgumentBuilder::MvrArgumentBuilder(size_t argvLen, char extraSpaceChar,
                                                  bool ignoreNormalSpaces, bool isPreCompressQuotes)
{
  myArgc = 0;
  myOrigArgc = 0;
  myArgvLen = argvLen;
  myArgv = new char *[myArgvLen];
  myFirstAdd = true;
  myExtraSpace = extraSpaceChar;
  myIgnoreNormalSpaces = ignoreNormalSpaces;
  myIsPreCompressQuotes = isPreCompressQuotes;
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
  for (i=0; i<myArgc;i++)
    myArgv[i] = cppstrdup(builder.getArg(i));
  myIsQuiet = builder.myIsQuiet;
  myExtraSpace = builder.myExtraSpace;
  myIgnoreNormalSpaces = builder.myIgnoreNormalSpaces;
  myIsPreCompressQuotes = builder.myIsPreCompressQuotes;
}

MVREXPORT MvrArgumentBuilder &MvrArgumentBuilder::operator=(const MvrArgumentBuilder &builder)
{
  if (this != &builder)
  {
    size_t i=0;

    /// Delete old stuff ...
    if (myOrigArgc > 0)
    {
      for (i=0; i<myOrigArgc; ++i)
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
  if (myOrigArgc > 0)
  {
    for (i=0; i<myOrigArgc; ++i)
      delete [] myArgv[i];
  }
  delete [] myArgv;
}

MVREXPORT void MvrArgumentBuilder::removeArg(size_t which, bool isRebuildFullString)
{
  size_t i;
  char *temp;

  if (which < 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrArgumentBuilder::removeArg: cannot remove arg at negative index (%i)",
                which);
    return;
  }
  if (which>myArgc-1)
  {
    MvrLog::log(MvrLog::Terse, "MvrArgumentBuilder::removeArg: %d is greater than the number of arguments which is %d",
                which, myArgc);
    return;
  }

  temp = myArgv[which];
  //delete [] myArgv[which]
  for (i=which; i<myArgc-1; i++)
    myArgv[i] = myArgv[i+1];
  /// delete the one off the end
  myArgc = -1;
  /// Just stuffing the delete argument value at the end of the array
  myArgv[i] = temp;

  /// Note: It seems that compressQuoted calls removeArg and depends on it not
  /// changing the full string. Therefor e, the parameter was added so that 
  /// the desired behavior could be specif ied by the caller
  if (isRebuildFullString)
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
  if ((!myIgnoreNormalSpaces) && (isSpace(c))){
    return true;
  }
  else if ((myExtraSpace != '\0') && (c == myExtraSpace)){
    return true;
  }
  return false;
} // end method isSpace

bool MvrArgumentBuilder::isStartArg(const char *buf, int len, int index, int *endArgFlagsOut)
{
  if (index<len){
    if (!isSpace(buf[index])){
      if ((myIsPreCompressQuotes) && (buf[index] == '\"')){
        if (endArgFlagsOut != NULL){
          *endArgFlagsOut = ANY_SPACE | QUOTE;
        }
      }
      else {  //not precompressed quote
        if (endArgFlagsOut != NULL){
          /// This is set to ANY_SPACE to preserve the original behavior ... i.e. space
          /// character "matches" with myExtraSpace
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
  if (index >= len)
    return true;
  if ((endArgFlags &QUOTE) != 0){
    if (buf[index] == '\"'){
      if ((index+1 >= len) || (buf[index+1] == '\0')){
        index++;
        return true;
      }
    } // end if quote found
  } // end if need to find quote
  else { // just looking for a space
    if (isSpace(buf[index])){
      return true;
    }
  }
  return false;
}  // end method isEndArg

/// Interal function that adds a string starting at some given space
/*
 * @param str the string to add
 * @param position the position to add the string at, a position less
 * than 0 means to add at the end, if this number if greater than how
 * many position exist then it will also be added at the end
 */
MVREXPORT void MvrArgumentBuilder::internalAdd(const char *str, int position)
{
  char buf[10000];
  int i = 0;
  int j = 0;
  size_t k = 0;
  int len = 0;
  bool addAtEnd = true;
  //size_t startingArgc = getArgc();
  bool isArgInProgress = false;
  int curArgStartIndex = -1;

  if (position < 0 || (size_t)position > myArgc)
    addAtEnd = true;
  else
    addAtEnd = false;

  strncpy(buf, str, sizeof(buf));
  len = strlen(buf);

  // can do whatever you want with the buf now
  // first we advance to non-space
  for (i = 0; i < len; ++i)
  {
    if (!isSpace(buf[i])) {
       //(!myIgnoreNormalSpaces && !isspace(buf[i])) || 
       // (myExtraSpace != '\0' && buf[i] != myExtraSpace))
      break;
    } // end if non-space found
  } // end for each char in buffer

  // see if we're done
  if (i == len)
  {
    if (!myIsQuiet) 
    {
      MvrLog::log(MvrLog::Verbose, "All white space add for argument builder.");
    }
    return;
  }

  int endArgFlags = ANY_SPACE;

  // walk through the line until we get to the end of the buffer...
  // we keep track of if we're looking for white space or non-white...
  // if we're looking for white space when we find it we have finished
  // one argument, so we toss that into argv, reset pointers and moven
  for (curArgStartIndex = i; ; ++i)
  {
    // Remove the slash of escaped spaces.  This is primarily done to handle command 
    // line arguments (especially on Linux). If quotes are "pre-compressed", then
    // the backslash is preserved.  Consecutive backslashes are also preserved 
    // (i.e. "\\ " is not modified).
    if (!myIsPreCompressQuotes && (buf[i] == '\\') && (i + 1 < len) && (buf[i + 1] == ' ') && ((i == 0) || (buf[i - 1] != '\\')))
    {
      for (j = i; j < len && j != '\0'; j++)
      {
        buf[j] = buf[j + 1];
      }
      --len;
    } // end if escaped space
  
    // If we're not in the middle of an argument, then determine whether the 
    // current buffer position marks the start of one.
    else if ((!isArgInProgress) &&  (i < len) && (buf[i] != '\0') &&  (isStartArg(buf, len, i, &endArgFlags))) 
    {
      curArgStartIndex = i;
      isArgInProgress = true;
    }
    // If we are in the middle of an argument, then determine whether the current
    // buffer position marks the end of it.  (Note that i may be incremented by
    // isEndArg when quotes are pre-compressed.)
    else if (isArgInProgress &&  ((i == len) || (buf[i] == '\0') || (isEndArg(buf, len, i, endArgFlags)))) 
    {
      // see if we have room in our argvLen
      if (myArgc + 1 >= myArgvLen)
      {
        MvrLog::log(MvrLog::Terse, "MvrArgumentBuilder::Add: could not add argument since argc (%u) has grown beyond the argv given in the constructor (%u)", myArgc, myArgvLen);
      }
      else // room in arg array
      {
        // if we're adding at the end just put it there, also put it
        // at the end if its too far out
        if (addAtEnd)
        {
          myArgv[myArgc] = new char[i - curArgStartIndex + 1];
          strncpy(myArgv[myArgc], &buf[curArgStartIndex], i - curArgStartIndex);
          myArgv[myArgc][i - curArgStartIndex] = '\0';
          // add to our full string
          // if its not our first add a space (or whatever our space char is)
          if (!myFirstAdd && myExtraSpace == '\0')
            myFullString += " ";
          else if (!myFirstAdd)
            myFullString += myExtraSpace;

          myFullString += myArgv[myArgc];
          myFirstAdd = false;

          myArgc++;
          myOrigArgc = myArgc;
        }
        // otherwise stick it where we wanted it if we can or just 
        else // insert arg at specified position
        {
          // first move things down
          for (k = myArgc + 1; k > (size_t)position; k--)
          {
            myArgv[k] = myArgv[k - 1];
          }
          myArgc++;
          myOrigArgc = myArgc;

          myArgv[position] = new char[i - curArgStartIndex + 1];
          strncpy(myArgv[position], &buf[curArgStartIndex], i - curArgStartIndex);
          myArgv[position][i - curArgStartIndex] = '\0';
          position++;

          rebuildFullString();
          myFirstAdd = false;
        }
      } 
      isArgInProgress = false;
      endArgFlags = ANY_SPACE;
    }
    // if we're at the end or its a null, we're at the end of the buf
    if (i == len || buf[i] == '\0')
      break;
  } 

} // end method internalAdd

/*
 * @param str the string to add
 * @param position the position to add ths string at, a position less
 * than 0 means to add at the end, if this number is greater than how
 * many positions exist then it will also be added at the end
 */
 MVREXPORT void MvrArgumentBuilder::addPlain(const char *str, int position)
 {
   internalAdd(str, position);
 }

 /*
  * @param argc how many arguments to add
  * @param argv the strings to add
  * @param position the position to add the string at, a position less
  * than 0 means to add at the end, if this number is greater than how
  * many positions exist then it will also be added at the end
  */
MVREXPORT void MvrArgumentBuilder::addStrings(int argc, char **argv, int position)
{
  int i;
  if (position < 0)
  {
    /// Don't try to use incremental positions, since the sequence would be out
    /// of order, just keep using the same special "at-end" meaning of negative position
    /// value
    for (i=0; i<argc; i++)
      internalAdd(argv[i], position);
  }
  else 
  {
    for (i=0; i<argc; i++)
      internalAdd(argv[i], position+i);
  }
}

 /*
  * @param argc how many arguments to add
  * @param argv the strings to add
  * @param position the position to add the string at, a position less
  * than 0 means to add at the end, if this number is greater than how
  * many positions exist then it will also be added at the end
  */
MVREXPORT void MvrArgumentBuilder::addStringsAsIs(int argc, char **argv, int position)
{
  int i;
  if (position<0)
  {
    /// Don't try to use incremental positions, since the sequence would be out
    /// of order, just keep using the same special "at-end" meaning of negtive position
    /// value
    for (i=0; i<argc; i++)
      internalAddAsIs(argv[i], position);
   }
  else 
  {
    for (i=0; i<argc; i++)
      internalAddAsIs(argv[i], position+i);
  }
}

/*
 * @param str the string to add
 * @param position the position to add the string at, a position less
 * than 0 means to add at the end, if the number is greater than how
 * many positions exist then it will also be added at the end
 */
MVREXPORT void MvrArgumentBuilder::addPlainAsIs(const char *str, int position)
{
  internalAddAsIs(str, position);
}
MVREXPORT void MvrArgumentBuilder::internalAddAsIs(const char *str, int position)
{
  size_t k = 0;
  bool addAtEnd;
  if (position<0 || (size_t)position > myArgc)
    addAtEnd = true;
  else 
    addAtEnd = false;
  
  if (addAtEnd)
  {
    myArgv[myArgc] = new char[strlen(str) + 1];
    strcpy(myArgv[myArgc],str);
    myArgv[myArgc][strlen(str)] = '\0';

    /// add to our full string
    /// if its not our first add a space
    if (!myFirstAdd && myExtraSpace == '\0')
      myFullString += " ";
    else if (!myFirstAdd)
      myFullString += myExtraSpace;
    
    myFullString += myArgv[myArgc];
    myFirstAdd = false;

    myArgc++;
    myOrigArgc = myArgc;
  }
  else 
  {
    /// first move things down
    for (k=myArgc+1; k>(size_t)position; k--)
    {
      myArgv[k] = myArgv[k-1];
    }
    myArgc++;
    myOrigArgc = myArgc;

    myArgv[position] = new char[strlen(str) + 1];
    strcpy(myArgv[position], str);
    myArgv[position][strlen(str)] = '\0';

    rebuildFullString();
    myFirstAdd = false;
  }
}

MVREXPORT size_t MvrArgumentBuilder::getArgc(void) const
{ return myArgc; }

MVREXPORT char **MvrArgumentBuilder::getArgv(void) const
{ return myArgv; }

MVREXPORT const char *MvrArgumentBuilder::getFullString(void) const
{ return myFullString.c_str(); }

MVREXPORT const char *MvrArgumentBuilder::getExtraString(void) const
{ return myExtraString.c_str(); }

MVREXPORT void MvrArgumentBuilder::setExtraString(const char *str)
{ myExtraString = str; }

MVREXPORT void MvrArgumentBuilder::setFullString(const char *str)
{ myFullString = str; }

MVREXPORT const char *MvrArgumentBuilder::getArg(size_t whichArg) const
{
  if ((whichArg > 0) && (whichArg < myArgc)){
    return myArgv[whichArg];
  }
  else {
    return NULL;
  }
}

MVREXPORT void MvrArgumentBuilder::log(void) const
{
  size_t i;
  MvrLog::log(MvrLog::Terse, "num arguments: %d", myArgc);
  for (i=0; i<myArgc; ++i)
    MvrLog::log(MvrLog::Terse, "Mvrg %d: %s", i, myArgv[i]);
}

MVREXPORT bool MvrArgumentBuilder::isArgBool(size_t whichArg) const
{
  if ((whichArg > myArgc) || getArg(whichArg) == NULL)
    return NULL;
  if (strcasecmp(getArg(whichArg), "true") == 0 ||
      strcasecmp(getArg(whichArg), "1") == 0 ||
      strcasecmp(getArg(whichArg), "false") == 0 ||
      strcasecmp(getArg(whichArg), "0") == 0 )
    return true;
  else
    return false;
}

MVREXPORT bool MvrArgumentBuilder::getArgBool(size_t whichArg, bool *ok) const
{
  bool isSuccess = false;
  bool ret = false;

  const char *str = getArg(whichArg);

  // If the arg was successfully obtained...
  if (str != NULL){
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "1") ==0){
      isSuccess = true;
      ret = true;
    }
    else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "0") ==0){
      isSuccess = false;
      ret = false;
    }
  }

  if (ok != NULL){
    *ok = isSuccess;
  }
  if (isSuccess){
    return ret;
  }
  else{
    return 0;
  }
}

MVREXPORT bool MvrArgumentBuilder::isArgInt(size_t whichArg, bool forceHex) const
{
  const char *str;
  char *endPtr;
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;
  
  int base = 10;
  str = getArg(whichArg);

  if (forceHex)
    base = 16;
  /// See if it has the ehx prefix and strip it
  if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
  {
    str = &str[2];
    base = 16;
  }
  
  strtol(str, &endPtr, base);
  if (endPtr[0] == '\0' && endPtr != str)
    return true;
  else
    return false;
}

MVREXPORT int MvrArgumentBuilder::getArgInt(size_t whichArg, bool *ok, bool forceHex) const
{
  bool isSuccess = false;
  int ret = 0;

  const char *str = getArg(whichArg);

  /// If the specified arg was successfully obtained...
  if(str != NULL){
    int base = 10;
    if (forceHex)
      base = 16;
    /// See if it has the hex prefix and strip it
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
     {
       str = &str[2];
       base = 16;
      }
    char *endPtr = NULL;
    ret = strtol(str, &endPtr, base);

    if (endPtr[0] == '\0' && endPtr != str){
      isSuccess = true;
    }
  }

  if (ok != NULL)
    *ok = isSuccess;
  if (isSuccess)
    return ret;
  else
    return 0;
}  // end method getArgInt

MVREXPORT bool MvrArgumentBuilder::isArgLongLongInt(size_t whichArg) const
{
  const char *str;
  char *endPtr;

  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;
  int base = 10;
  str = getArg(whichArg);

  /// See if it has the hex prefix and strip it
  if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
  {
    str = &str[2];
    base = 16;
  }
#ifndef _MSC_VER
  strtoll(str, &endPtr, base);
#else
  strtoi64(str, &endPtr, base);
#endif // _MSC_VER

  if (endPtr[0] == '\0' && endPtr != str)
    return true;
  else
    return false;
}  // end method getArgLongLongInt

MVREXPORT bool MvrArgumentBuilder::isArgDouble(size_t whichArg) const
{
  const char *str;
  char *endPtr;
  if (whichArg > myArgc || getArg(whichArg) == NULL)
    return false;
  
  str = getArg(whichArg);

  if (strcmp(str, "-INF") == 0)
    return true;
  else if (strcmp(str, "INF") == 0)
    return true;
  else
  {
    strtod(str, &endPtr);
    if (endPtr[0] == '\0' && endPtr != str)
      return true;
    else
      return false;
  }
}

MVREXPORT double MvrArgumentBuilder::getArgDouble(size_t whichArg, bool *ok) const
{
  bool isSuccess = false;
  double ret = 0;

  const char *str = getArg(whichArg);

  // If the specified arg was successfully obtained ..
  if (str != NULL)
  {
    if (strcmp(str, "-INF") == 0)
    {
      isSuccess = true;
      ret = -HUGE_VAL;
    }
    else if (strcmp(str, "INF") == 0)
    {
      isSuccess = true;
      ret = HUGE_VAL;
    }
    else
    {
      char *endPtr = NULL;
      ret = strtod(str, &endPtr);
      if (endPtr[0] == '\0' && endPtr != str){
        isSuccess = true;
      }
    }
  }
  if (ok != NULL){
    *ok = isSuccess;
  }
  if (isSuccess)
    return ret;
  else 
    return 0;
}

MVREXPORT void MvrArgumentBuilder::compressQuoted(bool stripQuotationMarks)
{
  size_t argLen;
  size_t i;
  std::string myNewArg;

  for (i=0; i<myArgc; i++)
  {
    argLen = strlen(myArgv[i]);
    if (stripQuotationMarks && argLen >= 2 &&
        myArgv[i][0] == '"' && myArgv[i][argLen-1] == '"')
    {
      myNewArg = &myArgv[i][1];
      myNewArg[myNewArg.size() -1] = '\0';
      delete [] myArgv[i];
      // but replacing ourself with new arg
      myArgv[i] = cppstrdup(myNewArg.c_str());
      // myArgv[i] = strdup(myNewArg.c_str());
      continue;
    }
    // if this arg begins with a quote bute doesn't end with one
    if (argLen >= 2 && myArgv[i][0] == '"' && myArgv[i][argLen-1] != '"')
    {
      /// Start the new value for this arg, if stripping quotations
      /// then start after the quote
      if (stripQuotationMarks)
        myNewArg = &myArgv[i][1];
      else
        myNewArg = myArgv[i];
      bool isEndQuoteFound = false;

      /// now while the end char of the next args isn't the end of our
      /// start quote we toss things into this arg
      while ((i+1 < myArgc) && !isEndQuoteFound)
      {
        int nextArgLen = strlen(myArgv[i+1]);

        /// Check whether the next arg contains the ending quote
        if ((nextArgLen > 0) && (myArgv[i+1][nextArgLen-1] == '"'))
        {
          isEndQuoteFound = true;
        }

        /// Concatenate the next arg to this one ...
        myNewArg += " ";
        myNewArg += myArgv[i+1];
        
        if (stripQuotationMarks && myNewArg.size()>0 && isEndQuoteFound)
          myNewArg[myNewArg.size() -1] = '\0';
        /// remove those next args
        removeArg(i+1);
        delete [] myArgv[i];

        /// but replacing ourself with the new arg
        myArgv[i] = cppstrdup(myNewArg.c_str());
        // myArgv[i] = strdup(myNewArg.c_str());
      }
    }
  }
}

MVREXPORT void MvrArgumentBuilder::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
}

MVREXPORT void MvrArgumentBuilder::rebuildFullString()
{
  myFullString = "";
  for (size_t k=0; k<myArgc; k++)
  {
    myFullString += myArgv[k];
    /// Don't take an extra space on at the end
    if (k<myArgc -1){
      myFullString += " ";
    }
  }
}

MVREXPORT bool MvrArgumentBuilderCompareOp::operator()(MvrArgumentBuilder *arg1,
                MvrArgumentBuilder *arg2) const
{
  if (arg1 == NULL)
    return true;
  else if (arg2 == NULL){
    return false;
  }
  std::string arg1String = arg1->getFullString();
  std::string arg2String = arg2->getFullString();

  return (arg1String.compare(arg2String) < 0);
}