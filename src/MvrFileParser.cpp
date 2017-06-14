/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFileParser.cpp
 > Description  : Class for parsing files more easily
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月13日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include "MvrFileParser.h"\

#include <ctype.h>

/*
 * @param baseDirectory the char * name of the base directory; the file name
 * is specified relative to this directory
 * @param isPreCompressQuotes a bool set to true if the file parser should 
 * treat strings enclosed in double-quotes as a single argument (such strings
 * must be surrounded by spaces).  This is roughly equivalent to calling 
 * MvrArgumentBuilder::compressQuoted(false) on the resulting builder, but 
 * is more efficient and handles embedded spaces better.  The default value
 * is false and preserves the original behavior where each argument is a 
 * space-separated alphanumeric string.
 */
MVREXPORT MvrFileParser::MvrFileParser(const char *baseDirectory, bool isPreCompressQuotes) :
          myCommentDelimiterList(),
          myPreParseFunctor(NULL),
          myMap(),
          myRemainderHandler(NULL),
          myIsQuiet(false),
          myIsPreCompressQuotes(isPreCompressQuotes),
          myIsInterrupted(false),
          myInterruptMutex()
{
  setBaseDirectory(baseDirectory);

  std::list<std::string> defaultDelimiters;
  defaultDelimiters.push_back(";");
  defaultDelimiters.push_back("#");

  setCommentDelimiters(defaultDelimiters);

  resetCounters();
  setMaxNumArguments();
}

MVREXPORT MvrFileParser::~MvrFileParser(void)
{
  MvrUtil::deleteSetPairs(myMap.begin(), myMap.end());
  myMap.clear();

  delete myRemainderHandler;
}

MVREXPORT bool MvrFileParser::addHandler(const char *keyword, 
               MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor)
{
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  if (keyword == NULL)
  {
    if (myRemainderHandler != NULL)
    {
      MvrLog::log(MvrLog::Verbose, "There is already a functor to handle unhandled lines");
      return false;                  
    }
    else
    {
      delete myRemainderHandler;
      myRemainderHandler = new HandlerCBType(functor);
      return true;
    }
  }
  if ((it = myMap.find(keyword)) != myMap.end())
  {
    if (!myIsQuiet)
    {
      MvrLog::log(MvrLog::Verbose, "There is already a functor to handle keyword '%s'", keyword);
    }
  }
  if (!myIsQuiet)
  {
    MvrLog::log(MvrLog::Verbose, "keyword '%s' handler added'", keyword);      
  }
  myMap[keyword] = new HandlerCBType(functor);
  return true;
}               

/**
   This function has a different name than addProcessFileCB just so
   that if you mean to get this function but have the wrong functor
   you'll get an error.  The rem's are the same though since that
   shouldn't matter.
**/
MVREXPORT bool MvrFileParser::addHandlerWithError(const char *keyword, 
	MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor)
{
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  if (keyword == NULL)
  {
    if (myRemainderHandler != NULL)
    {
      MvrLog::log(MvrLog::Verbose, "There is already a functor to handle unhandled lines");
      return false;
    }
    else
    {
      delete myRemainderHandler;
      myRemainderHandler = new HandlerCBType(functor);
      return true;
    }
  }

  if ((it = myMap.find(keyword)) != myMap.end())
  {
    if (!myIsQuiet) 
    {
      MvrLog::log(MvrLog::Verbose, "There is already a functor to handle keyword '%s'", keyword);
    }
    return false;
  }
  if (!myIsQuiet) 
  {
    MvrLog::log(MvrLog::Verbose, "keyword '%s' handler added", keyword);
  }
  myMap[keyword] = new HandlerCBType(functor);
  return true;
}

MVREXPORT bool MvrFileParser::remHandler(const char *keyword, 
				                                 bool logIfCannotFind)
{
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  HandlerCBType *handler;

  if (keyword == NULL) 
  {

    if (myRemainderHandler != NULL)
    {
      delete myRemainderHandler;
      myRemainderHandler = NULL;
      if (!myIsQuiet) 
      {
        MvrLog::log(MvrLog::Verbose, "Functor for remainder handler removed");
      }
      return true;
    }
    else 
    { // no remainder handler to remove
      if (!myIsQuiet) 
      {
        MvrLog::log(MvrLog::Verbose, "No remainder handler to remove for null keyword");
      }
      return false;
    } // end else no remainder handler to remove
  } // end if keyword is null


  if ((it = myMap.find(keyword)) == myMap.end())
  {
    if (logIfCannotFind)
      MvrLog::log(MvrLog::Normal, "There is no keyword '%s' to remove.", keyword);
    return false;
  }
  if (!myIsQuiet) 
  {
    MvrLog::log(MvrLog::Verbose, "keyword '%s' removed", keyword);
  }
  handler = (*it).second;
  myMap.erase(it);
  delete handler;
  remHandler(keyword, false);
  return true;
}

MVREXPORT bool MvrFileParser::remHandler(MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor)
{
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  HandlerCBType *handler;

  if (myRemainderHandler != NULL && myRemainderHandler->haveFunctor(functor))
  {
    delete myRemainderHandler;
    myRemainderHandler = NULL;
    MvrLog::log(MvrLog::Verbose, "Functor for remainder handler removed");
    return true;
  }

  for (it = myMap.begin(); it != myMap.end(); it++)
  {
    if ((*it).second->haveFunctor(functor))
    {
      if (!myIsQuiet) 
      {
        MvrLog::log(MvrLog::Verbose, "Functor for keyword '%s' removed.", (*it).first.c_str());
      }
      handler = (*it).second;
      myMap.erase(it);
      delete handler;
      remHandler(functor);
      return true;
    }
  }
  return false;

}

MVREXPORT bool MvrFileParser::remHandler(MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor)
{
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  HandlerCBType *handler;

  if (myRemainderHandler != NULL && myRemainderHandler->haveFunctor(functor))
  {
    delete myRemainderHandler;
    myRemainderHandler = NULL;
    MvrLog::log(MvrLog::Verbose, "Functor for remainder handler removed");
    return true;
  }

  for (it = myMap.begin(); it != myMap.end(); it++)
  {
    if ((*it).second->haveFunctor(functor))
    {
      if (!myIsQuiet) 
      {
        MvrLog::log(MvrLog::Verbose, "Functor for keyword '%s' removed.", (*it).first.c_str());
      }
      handler = (*it).second;
      myMap.erase(it);
      delete handler;
      remHandler(functor);
      return true;
    }
  }
  return false;
}

MVREXPORT void MvrFileParser::setBaseDirectory(const char *baseDirectory)
{
  if (baseDirectory != NULL && strlen(baseDirectory) > 0)
    myBaseDir = baseDirectory;
  else
    myBaseDir = "";
}

MVREXPORT const char *MvrFileParser::getBaseDirectory(void) const
{
  return myBaseDir.c_str();
}

/*
 * By default, the ";" and "#" strings are used to indicate comments.  Call
 * this method to override the defaults.
 * @param delimiters each string represents
 * a comment delimiter.  The comment delimiter and any following characters in 
 * the current line will be stripped during parsing of the file
 */
MVREXPORT void MvrFileParser::setCommentDelimiters(const std::list<std::string> &delimiters)
{
  myCommentDelimiterList.clear();

  int i = 0;
  for (std::list<std::string>::const_iterator iter = delimiters.begin();
       iter != delimiters.end();
       iter++, i++) 
  {
    std::string curDelimiter = *iter;
    if (!MvrUtil::isStrEmpty(curDelimiter.c_str())) 
    {
      myCommentDelimiterList.push_back(curDelimiter);
    }
    else 
    {
      MvrLog::log(MvrLog::Normal,
                 "MvrFileParser::setCommentDelimiters cannot set empty delimiter at position %i",
                 i);
    }
  } // end for each given delimiter

} // end method setCommentDelimiters

/**
 * Call this method to indicate that the file type to be parsed does not contain 
 * comments.
**/
MVREXPORT void MvrFileParser::clearCommentDelimiters()
{
  myCommentDelimiterList.clear();

} // end method clearCommentDelimiters


MVREXPORT void MvrFileParser::resetCounters(void)
{
  myLineNumber = 0;
}

MVREXPORT bool MvrFileParser::parseLine(char *line, 
				                                char *errorBuffer, 
                                        size_t errorBufferLen)
{
  char keyword[512];
  char *choppingPos;
  char *valueStart;
  size_t textStart;
  size_t len;
  size_t i;
  bool noArgs;
  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp>::iterator it;
  HandlerCBType *handler;

  myLineNumber++;
  noArgs = false;
  

  if (myPreParseFunctor != NULL) 
  {
    myPreParseFunctor->invoke(line);
  }


  // chop out the comments
  for (std::list<std::string>::iterator iter = myCommentDelimiterList.begin();
       iter != myCommentDelimiterList.end();
       iter++) 
  {
    std::string commentDel = *iter;
    if ((choppingPos = strstr(line, commentDel.c_str())) != NULL) 
    {
      line[choppingPos-line] = '\0';
    }
  }
  
  int chopCount = 0;

  // chop out the new line if its there
  if ((choppingPos = strstr(line, "\n")) != NULL) 
  {
    chopCount++;
    line[choppingPos-line] = '\0';
  }
  // chop out the windows new line if its there
  while ((choppingPos = strstr(line, "\r")) != NULL) 
  {
    chopCount++;
    memmove(choppingPos, choppingPos + 1, strlen(line));
  }

  // see how long the line is
  len = strlen(line);
  
  // find the keyword
  // if this is 0 then we have an empty line so we continue
  if (len == 0)
  {
    return true;
  }
  // first find the start of the text
  for (i = 0; i < len; i++)
  {
    // if its not a space we're done
    if (!isspace(line[i]))
    {
      textStart = i;
      break;
    };
  }
  // if we reached the end of the line then continue
  if (i == len)
  {
    if (!myIsQuiet) 
    {
      MvrLog::log(MvrLog::Verbose, "line %d: just white space at start of line", myLineNumber);
    }
    return true;
  }
  // now we chisel out the keyword 
  // adding it so that if the text is quoted it pulls the whole keyword
  bool quoted = false;
  for (i = textStart; i < len && i < sizeof(keyword) + textStart - 3; i++)
  {
    // if we're on the start and its a quote just note that and continue
    if (!quoted && i == textStart && line[i] == '"')
    {
      // set quoted to true since we're going to move textStart ahead
      // and don't want to loop this
      quoted = true;
      // note that our text starts on the next char really
      textStart++;
      continue;
    }
    // if we're not looking for the end quote and its a space we're done
    if (!quoted && isspace(line[i]))
    {
      break;
    }
    else if (quoted && line[i] == '"')
    {
      keyword[i-textStart] = '\0';
      i++;
      break;
    }
    // if not its part of the keyword
    else
      keyword[i-textStart] = line[i];
  }

  keyword[i-textStart] = '\0';
  for (; i < len; i++)
  {
    // if its not a space we're done
    if (!isspace(line[i]))
    {
      valueStart = &line[i];
      break;
    };
  }
  // lower that keyword
  MvrUtil::lower(keyword, keyword, 512);

  bool usingRemainder = false;
  // see if we have a handler for the keyword
  if ((it = myMap.find(keyword)) != myMap.end())
  {
    // we have a handler, so pull that out
    handler = (*it).second;
    // valueStart was set above but make sure there's an argument
    if (i == len)
      noArgs = true;
  }
  // if we don't then check for a remainder handler
  else
  {
    // if we have one set it
    if (myRemainderHandler != NULL)
    {
      usingRemainder = true;
      handler = myRemainderHandler;
      // reset the value to the start of the text
      valueStart = &line[textStart];
    }
    // if we don't just keep going
    else
    {
      MvrLog::log(MvrLog::Verbose, "line %d: unknown keyword '%s' line '%s', continuing", 
		              myLineNumber, keyword, &line[textStart]);
      return true;
    }
  }
  
  MvrArgumentBuilder builder(myMaxNumArguments,
                             '\0',  // no special space character
                             false, // do not ignore normal spaces
                             myIsPreCompressQuotes); // whether to pre-compress quotes
  // if we have arguments add them
  if (!noArgs)
    builder.addPlain(valueStart);
  // if not we still set the name of whatever we parsed (unless we
  // didn't have a param of course)
  if (!usingRemainder)
    builder.setExtraString(keyword);

  // make sure we don't overwrite any errors
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
    
  // call the functor and see if there are errors;
  // if we had an error and aren't continuing on errors then we keep going
  if (!handler->call(&builder, errorBuffer, errorBufferLen))
  {
    // put the line number in the error message (this won't overwrite
    // anything because of the check above
    if (errorBuffer != NULL)
    {
      std::string errorString = errorBuffer;
      snprintf(errorBuffer, errorBufferLen, "Line %d: %s", myLineNumber, 
	       errorString.c_str());
      
    }
    return false;
  }
  return true;
}

MVREXPORT void MvrFileParser::setPreParseFunctor(MvrFunctor1<const char *> *functor)
{
  myPreParseFunctor = functor;

}



/**
   @param fileName the file to open
   @param continueOnErrors whether to continue or immediately bail upon an error 
   @param noFileNotFoundMessage whether or not to log if we find a
   file (we normally want to but for robot param files that'd be too
   annoying since we test for a lot of files)

   @param errorBuffer buffer to put errors into if not NULL. Only the
   first error is saved, and as soon as this function is called it
   immediately empties the errorBuffer
   
   @param errorBufferLen the length of @a errorBuffer
*/
MVREXPORT bool MvrFileParser::parseFile(const char *fileName, 
                                        bool continueOnErrors, 
                                        bool noFileNotFoundMessage,
                                        char *errorBuffer,
                                        size_t errorBufferLen)
{
  myInterruptMutex.lock();
  myIsInterrupted = false;
  myInterruptMutex.unlock();

  FILE *file = NULL;

  char line[10000];
  bool ret = true;

  if (errorBuffer)
    errorBuffer[0] = '\0';

  std::string realFileName;
  if (fileName[0] == '/' || fileName[0] == '\\')
  {
    realFileName = fileName;
  }
  else
  {
    realFileName = myBaseDir;
    realFileName += fileName;
  }

  MvrLog::log(MvrLog::Verbose, "Opening file %s from fileName given %s and base directory %s", realFileName.c_str(), fileName, myBaseDir.c_str());
    
  //char *buf = new char[4096];

  if ((file = MvrUtil::fopen(realFileName.c_str(), "r")) == NULL)
  {
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "cannot open file %s", fileName);
    if (!noFileNotFoundMessage)
      MvrLog::log(MvrLog::Terse, "MvrFileParser::parseFile: Could not open file %s to parse file.", realFileName.c_str());
    return false;
  }

  resetCounters();

  // read until the end of the file
  while (!isInterrupted() && (fgets(line, sizeof(line), file) != NULL)) 
  {
    if (!parseLine(line, errorBuffer, errorBufferLen))
    {
      MvrLog::log(MvrLog::Terse, "## Last error on line %d of file '%s'", 
		              myLineNumber, realFileName.c_str());
      ret = false;
      if (!continueOnErrors)
	      break;
    }
  }
  fclose(file);
  return ret;
}
  

bool MvrFileParser::isInterrupted() 
{

  myInterruptMutex.lock();
  bool b = myIsInterrupted;
  myInterruptMutex.unlock();

  return b;

}


MVREXPORT void MvrFileParser::cancelParsing()
{
  myInterruptMutex.lock();
  myIsInterrupted = true;
  myInterruptMutex.unlock();

}


/**
 * @param file File pointer for a file to be parsed. The file must be open for
 * reading (e.g. with MvrUtil::fopen()) and this pointer must not be NULL.
 * @param buffer a non-NULL char array in which to read the file
 * @param bufferLength the number of chars in the buffer; must be greater than 0
 * @param continueOnErrors a bool set to true if parsing should continue
 * even after an error is detected
 * @param errorBuffer buffer to put errors into if not NULL. Only the
 * first error is saved, and as soon as this function is called it
 * immediately empties the errorBuffer
 * @param errorBufferLen the length of @a errorBuffer
*/
MVREXPORT bool MvrFileParser::parseFile(FILE *file, 
                                        char *buffer, 
                                        int bufferLength, 
                                        bool continueOnErrors,
                                        char *errorBuffer,
                                        size_t errorBufferLen)
{
  myInterruptMutex.lock();
  myIsInterrupted = false;
  myInterruptMutex.unlock();

  if (errorBuffer)
    errorBuffer[0] = '\0';

  if ((file == NULL) || (buffer == NULL) || (bufferLength <= 0)) 
  {
    if (errorBuffer != NULL)
      snprintf(errorBuffer, errorBufferLen, "parseFile: bad setup");
    return false;
  }

  bool ret = true;
  resetCounters();

  // read until the end of the file
  while (!isInterrupted() && (fgets(buffer, bufferLength, file) != NULL))
  {
    if (!parseLine(buffer, errorBuffer, errorBufferLen))
    {
      ret = false;
      if (!continueOnErrors)
        break;
    }
  }
  return ret;
}

void MvrFileParser::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
}
