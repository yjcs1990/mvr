/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArgumentBuilder.h
 > Description  : Base class for device connections
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/

#ifndef MVRARGUMENTBUILDER_H
#define MVRARGUMENTBUILDER_H

#include "mvriaTypedefs.h"

/// This class is to build arguments for things that require argc and argv
/// @ingroup ImportantClasses

class MvrArgumentBuilder
{
public:
  /// Constructor
  MVREXPORT MvrArgumentBuilder(size_t argvLen = 512, char extraSpaceChar = '\0',
			       bool ignoreNormalSpaces = false, bool isPreCompressQuotes = false);
  /// Copy Constructor
  MVREXPORT MvrArgumentBuilder(const MvrArgumentBuilder &builder);

  MVREXPORT MvrArgumentBuilder &operator=(const MvrArgumentBuilder &builder);

  /// Destructor
  MVREXPORT virtual ~MvrArgumentBuilder();
#ifndef SWIG
  /** @brief Adds the given string, with varargs, separates if there are spaces
   *  @swignote Not available
   */
  MVREXPORT void add(const char *str, ...);
#endif
  /// Adds the given string, without varargs (wrapper for java)
  MVREXPORT void addPlain(const char *str, int position = -1);
  /// Adds the given string, without varargs and without touching the str
  MVREXPORT void addPlainAsIs(const char *str, int position = -1);
  /// Adds the given string thats divided
  MVREXPORT void addStrings(char **argv, int argc, int position = -1);
  /// Adds the given string thats divided
  MVREXPORT void addStrings(int argc, char **argv, int position = -1);
  /// Adds the given string thats divided (but doesn't touch the strings)
  MVREXPORT void addStringsAsIs(int argc, char **argv, int position = -1);
  /// Gets the original string of the input
  MVREXPORT const char *getFullString(void) const;
  /// Sets the full string (this is so you can have a more raw full string)
  MVREXPORT void setFullString(const char *str);
  /// Gets the extra string of the input, used differently by different things
  MVREXPORT const char *getExtraString(void) const;
  /// Sets the extra string of the input, used differently by different things
  MVREXPORT void setExtraString(const char *str);
  /// Prints out the arguments
  MVREXPORT void log(void) const;
  /// Gets the argc
  MVREXPORT size_t getArgc(void) const;
  /// Gets the argv
  MVREXPORT char** getArgv(void) const;
  /// Gets a specific argument as a string
  MVREXPORT const char* getArg(size_t whichArg) const;

  /// Sees if an argument is a bool
  MVREXPORT bool isArgBool(size_t whichArg) const;

  /// Gets the value of an argument as a boolean 
  /**
   * Valid boolean values are "true" and "false", and "1" and "0".
   * There are two ways to to verify that the specified argument is a bool.
   * Either call isArgBool() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient since the 
   * argument string is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return bool the retrieved argument value; valid only if ok or isArgBool
   * is true
  **/
  MVREXPORT bool getArgBool(size_t whichArg, bool *ok = NULL) const;

  /// Sees if an argument is an int
  /**
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param forceHex if true this makes it find the int in base 16 instead of 10
   */
  MVREXPORT bool isArgInt(size_t whichArg, bool forceHex = false) const;

  /// Gets the value of an argument as an integer
  /**
   * There are two ways to to verify that the specified argument is an integer.
   * Either call isArgInt() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @param forceHex if true this makes it find the int in base 16 instead of 10
   * @return int the retrieved argument value; valid only if ok or isArgInt 
   * is true
  **/
  MVREXPORT int getArgInt(size_t whichArg, bool *ok = NULL, bool forceHex = false) const;

  /// Sees if an argument is a long long int
  MVREXPORT bool isArgLongLongInt(size_t whichArg) const;

  /// Gets the value of an argument as a long long integer
  /**
   * There are two ways to to verify that the specified argument is an integer.
   * Either call isArgInt() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return int the retrieved argument value; valid only if ok or isArgInt 
   * is true
  **/
  MVREXPORT int getArgLongLongInt(size_t whichArg, bool *ok = NULL) const;

  /// Sees if an argument is a double
  MVREXPORT bool isArgDouble(size_t whichArg) const;

  /// Gets the value of an argument as a double
  /**
   * There are two ways to to verify that the specified argument is a double.
   * Either call isArgDouble() before calling this method, or specify a non-NULL
   * ok parameter value. The latter is somewhat more efficient because the 
   * digit status of each character is checked only once.
   * 
   * @param whichArg the size_t index of the arg to retrieve; must be >= 0
   * and less than getArgc()
   * @param ok an optional pointer to a bool that will be set to true if the 
   * arg was successfully retrieved or to false if an error occurred
   * @return double the retrieved argument value; valid only if ok or 
   * isArgDouble is true
  **/
  MVREXPORT double getArgDouble(size_t whichArg, bool *ok = NULL) const;

  /// Delete a particular arg, you MUST finish adding before you can remove
  MVREXPORT void removeArg(size_t which, bool isRebuildFullString = false);
  /// Combines quoted arguments into one
  MVREXPORT void compressQuoted(bool stripQuotationMarks = false);

  /// Turn on this flag to reduce the number of verbose log messages.
  MVREXPORT void setQuiet(bool isQuiet);

protected:
  MVREXPORT void internalAdd(const char *str, int position = -1);
  MVREXPORT void internalAddAsIs(const char *str, int position = -1);
	MVREXPORT void rebuildFullString();

  /// Characters that may be used to separate arguments; bitwise flags so QUOTE can be combined with spaces
  enum ArgSeparatorType {
    SPACE              = 1,               // Normal space character
    SPECIAL            = SPACE << 1,      // The special "extra" space character, if any
    ANY_SPACE          = SPACE | SPECIAL, // Either normal space or special extra space
    QUOTE              = SPECIAL << 1,    // Double-quote, must be used in combination with spaces 
 };

  /// Determines whether the current buffer position marks the start of an argument
  /**
   * This method should only be called when an argument is not currently being 
   * parsed (i.e. the previous character was a space).  
   * @param buf the char * buffer that is being parsed; must be non-NULL
   * @param len the maximum number of characters in the buffer
   * @param index the int buffer position of the character to be tested
   * @param endArgFlagsOut a pointer to an output int that will indicate which separators
   * will mark the end of the argument.  If quotes are being pre-processed, and the
   * current argument starts with space-quote, then the argument must end with a quote-space.
  **/
  bool isStartArg(const char *buf, int len, int index, int *endArgFlagsOut);

  /// Determines whether the current buffer position marks the end of an argument
  /**
   * This method should only be called when an argument is currently being 
   * parsed (i.e. isStartArg returned true).  
   * @param buf the char * buffer that is being parsed; must be non-NULL
   * @param len the maximum number of characters in the buffer
   * @param indexInOut the input/output int buffer position of the character to be tested; 
   * if the argument ends with a quote-space, then the index will be incremented to mark
   * the space; otherwise, it will remain unchanged
   * @param endArgFlags an int that indicates which separators mark the end of the 
   * argument.  If quotes are being pre-processed, and the current argument started
   * with space-quote, then the argument must end with a quote-space.
  **/
  bool isEndArg(const char *buf, int len, int &indexInOut, int endArgFlags);

  /// Determines whether the specified character is an acceptable space (either normal or extra)
  bool isSpace(char c);

  size_t getArgvLen(void) const { return myArgvLen; }
  // how many arguments we had originally (so we can delete 'em)
  size_t myOrigArgc;
  // how many arguments we have
  size_t myArgc;
  // argument list
  char **myArgv;
  // argv length
  size_t myArgvLen;
  // the extra string (utility thing)
  std::string myExtraString;
  // the full string
  std::string myFullString;
  // whether this is our first add or not
  bool myFirstAdd;
  // a character to alternately treat as a space
  char myExtraSpace;
  // if we should ignore normal spaces
  bool myIgnoreNormalSpaces;
  /// Whether to treat double-quotes as arg delimiters (preserving spaces within)
  bool myIsPreCompressQuotes;

  bool myIsQuiet;
};

// ----------------------------------------------------------------------------

/// Comparator that returns true if arg1's full string is less than arg2's.
struct MvrArgumentBuilderCompareOp
{
public:
  /// Compares arg1's full string to arg2's.
  MVREXPORT bool operator() (MvrArgumentBuilder* arg1, MvrArgumentBuilder* arg2) const;

}; // end struct MvrArgumentBuilderCompareOp


#endif  // MVRARGUMENTBUILDER_H