/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArgumentParser.h
 > Description  : Base class for device connections
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/

#ifndef MVRARGUMENTPARSER_H
#define MVRARGUMENTPARSER_H

#include "mvriaTypedefs.h"
#include "MvrArgumentBuilder.h"

class MvrArgumentParser
{
/// Constructor, takes the argc argv
  MVREXPORT MvrArgumentParser(int *argc, char **argv);
  /// Constructor, takes an argument builder
  MVREXPORT MvrArgumentParser(MvrArgumentBuilder *builder);
  /// Destructor
  MVREXPORT ~MvrArgumentParser();
  /// If we should only set wasReallySet to true
  MVREXPORT void setWasReallySetOnlyTrue(bool wasReallySetOnlyTrue);
  /// If we only set wasReallySet to true
  MVREXPORT bool getWasReallySetOnlyTrue(void);
  /// Returns true if the argument was found
  MVREXPORT bool checkArgument(const char *argument);
  /// Returns the word/argument after given argument 
  MVREXPORT char *checkParameterArgument(const char *argument, 
					bool returnFirst = false);
  /// Returns the word/argument after given argument 
  MVREXPORT bool checkParameterArgumentString(const char *argument, const char **dest, 
					  bool *wasReallySet = NULL, bool returnFirst = false);
  /// Returns the integer after given argument 
  MVREXPORT bool checkParameterArgumentInteger(const char *argument, int *dest,
					  bool *wasReallySet = NULL, bool returnFirst = false);
  /// Returns the word/argument after given argument 
  MVREXPORT bool checkParameterArgumentBool(const char *argument, bool *dest,
					  bool *wasReallySet = NULL, bool returnFirst = false);
  /// Returns the floating point number after given argument 
  MVREXPORT bool checkParameterArgumentFloat(const char *argument, float *dest, 
            bool *wasReallySet = NULL, bool returnFirst = false);
  /// Returns the floating point number after given argument 
  MVREXPORT bool checkParameterArgumentDouble(const char *argument, double *dest, 
            bool *wasReallySet = NULL, bool returnFirst = false);
  /// Adds a string as a default argument
  MVREXPORT void addDefaultArgument(const char *argument, int position = -1);
  /// Adds a string as a default argument as is (without touching
  /// spaces or what not)
  MVREXPORT void addDefaultArgumentAsIs(const char *argument, int position = -1);
  /// Adds args from default files and environmental variables
  MVREXPORT void loadDefaultArguments(int position = 1);
  /// Checks for the help strings and warns about unparsed arguments
  MVREXPORT bool checkHelpAndWarnUnparsed(unsigned int numArgsOkay = 0);
  /// Checks for the help argument  
  MVREXPORT bool checkHelp();
  /// Gets how many arguments are left in this parser
  MVREXPORT size_t getArgc(void) const;
  /// Gets the argv
  MVREXPORT char** getArgv(void) const;
  /// Gets the argument builder, if one is being used (may be NULL)
  MVREXPORT const MvrArgumentBuilder *getArgumentBuilder(void) const 
    { return myBuilder; }
  /// Gets a specific argument
  MVREXPORT const char* getArg(size_t whichArg) const;

  /// Prints out the arguments left in this parser
  MVREXPORT void log(void) const;
  /// Gets the arguments this parser started with (if possible, NULL otherwise)
  MVREXPORT const char *getStartingArguments(void) const;
  /// Internal function to remove an argument that was parsed
  MVREXPORT void removeArg(size_t which);
  /// Adds another file or environmental variable to the list of defaults
  MVREXPORT static void addDefaultArgumentFile(const char *file);
  /// Adds another file or environmental variable to the list of defaults
  MVREXPORT static void addDefaultArgumentEnv(const char *env);
  /// Logs the default argument locations
  MVREXPORT static void logDefaultArgumentLocations(void);
#ifndef SWIG
  /** @brief Returns true if the argument was found
   *  @swigomit
   */
  MVREXPORT bool checkArgumentVar(const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  MVREXPORT char *checkParameterArgumentVar(const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentStringVar(bool *wasReallySet, 
						const char **dest, const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentBoolVar(bool *wasReallySet, bool *dest,
					  const char *argument, ...);
  /** @brief Returns the integer after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentIntegerVar(bool *wasReallySet, int *dest,
						const char *argument, ...);
  /** @brief Returns the float after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentFloatVar(bool *wasReallySet, 
						float *dest, const char *argument, ...);
  /** @brief Returns the double after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentDoubleVar(bool *wasReallySet, 
						double *dest, const char *argument, ...);
#endif  // SWIG
protected:
  static std::list<std::string> ourDefaultArgumentLocs;
  static std::list<bool> ourDefaultArgumentLocIsFile;
  bool myOwnBuilder;
  MvrArgumentBuilder *myBuilder;
  bool myUsingBuilder;
  char **myArgv;
  int *myArgc;
  bool myHelp;
  char myEmptyArg[1];
  bool myReallySetOnlyTrue;
};
#endif  // MVRARGUMENTPARSER_H