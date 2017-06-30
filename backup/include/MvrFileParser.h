/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFileParser.h
 > Description  : Class for parsing files more easily
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#ifndef MVRFILEPARSER_H
#define MVRFILEPARSER_H

#include "mvriaTypedefs.h"
#include "MvrArgumentParser.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"

class MvrFileParser
{
public:
  /// Constructor
  MVREXPORT MvrFileParser(const char *baseDirectory = "./", bool isPreCompressQuotes = false);

  /// Destructor
  MVREXPORT ~MvrFileParser(void);

  /// Adds a functor to handle a keyword that wants an easily parsable string
  MVREXPORT bool addHandler(const char *keyword, MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor);
  /// Adds a functor to handle a keyword that wants an easily parsable string and returns error messages
  MVREXPORT bool addHandlerWithError(const char *keyword,MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor);
  /// Removes a handler for a keyword
  MVREXPORT bool remHandler(const char *keyword, bool logIfCannotFind = true);
  /// Removes any handlers with this functor
  MVREXPORT bool remHandler(MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor);
  /// Removes any handlers with this functor
  MVREXPORT bool remHandler(MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor);
  
	MVREXPORT void setPreParseFunctor(MvrFunctor1<const char *> *functor);

  /// Opens, parses, and then closes the specified file.
  MVREXPORT bool parseFile(const char *fileName, bool continueOnErrors = true,
			      bool noFileNotFoundMessage = false, char *errorBuffer = NULL, size_t errorBufferLen = 0);

  /// Parses an open file; the file is not closed by this method.
  /**
   * @param file the open FILE* to be parsed; must not be NULL
   * @param buffer a non-NULL char array in which to read the file
   * @param bufferLength the number of chars in the buffer; must be greater than 0
   * @param continueOnErrors a bool set to true if parsing should continue
   * even after an error is detected
  **/
  MVREXPORT bool parseFile(FILE *file, char *buffer, int bufferLength, 
			      bool continueOnErrors = true, char *errorBuffer = NULL, size_t errorBufferLen = 0);

  /// If parseFile is currently in progress, then terminates it as soon as possible.
  MVREXPORT void cancelParsing();

  /// Gets the base directory
  MVREXPORT const char *getBaseDirectory(void) const;
  /// Sets the base directory
  MVREXPORT void setBaseDirectory(const char *baseDirectory);

  /// Sets the strings used to mark comments in the file to be parsed.
  MVREXPORT void setCommentDelimiters(const std::list<std::string> &delimiters);

  /// Clears the strings used to mark comments in the file to be parsed.
  MVREXPORT void clearCommentDelimiters();

  /// Function to parse a single line 
  MVREXPORT bool parseLine(char *line, char *errorBuffer = NULL, size_t errorBufferLen = 0);
  /// Function to reset counters
  MVREXPORT void resetCounters(void);
  /// Sets the maximum number of arguments in a line we can expect
  MVREXPORT void setMaxNumArguments(size_t maxNumArguments = 512) { myMaxNumArguments = maxNumArguments; }
  /// Turn on this flag to reduce the number of verbose log messages.
  MVREXPORT void setQuiet(bool isQuiet);

protected:

  /// Returns true if cancelParsing() has been called during parseFile()
  bool isInterrupted();

  class HandlerCBType
  {
    public:
    HandlerCBType(MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor)
    {
      myCallbackWithError = functor;
      myCallback = NULL;
    }
    HandlerCBType(MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor)
    {
      myCallbackWithError = NULL;
      myCallback = functor;
    }
    ~HandlerCBType() {}
    bool call(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen) 
    { 
      if (myCallbackWithError != NULL) 
	      return myCallbackWithError->invokeR(arg, errorBuffer, errorBufferLen);
      else if (myCallback != NULL) 
	      return myCallback->invokeR(arg); 
      // if we get here there's a problem
      MvrLog::log(MvrLog::Terse, "MvrFileParser: Horrible problem with process callbacks");
      return false;
    }
    bool haveFunctor( MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *functor)
    { 
      if (myCallbackWithError == functor) 
      	return true; 
      else 
	      return false; 
    }
    bool haveFunctor(MvrRetFunctor1<bool, MvrArgumentBuilder *> *functor)
    { 
      if (myCallback == functor) 
	      return true; 
      else 
	      return false; 
    }
    const char *getName(void) 
    { 
      if (myCallbackWithError != NULL)
	      return myCallbackWithError->getName();
      else if (myCallback != NULL)
      	return myCallback->getName();
      // if we get here there's a problem
      MvrLog::log(MvrLog::Terse, "MvrFileParser: Horrible problem with process callback names");
      return NULL;
    }
    protected:
    MvrRetFunctor3<bool, MvrArgumentBuilder *, char *, size_t> *myCallbackWithError;
    MvrRetFunctor1<bool, MvrArgumentBuilder *> *myCallback;
  };
  size_t myMaxNumArguments;
  int myLineNumber;
  std::string myBaseDir;
  std::list<std::string> myCommentDelimiterList;

  MvrFunctor1<const char *> *myPreParseFunctor;

  std::map<std::string, HandlerCBType *, MvrStrCaseCmpOp> myMap;
  // handles that NULL case
  HandlerCBType *myRemainderHandler;
  bool myIsQuiet;
  bool myIsPreCompressQuotes;
  bool myIsInterrupted;
  MvrMutex myInterruptMutex;

};
#endif  // MVRFILEPARSER_H