#ifndef MVRARGUMENTPARSER_H
#define MVRARGUMENTPARSER_H

#include "mvriaTypedefs.h"
#include "MvrArgumentBuilder.h"

/// Parse and store program command-line arguments for use by other ARIA classes.
/**
   This class is made for parsing arguments from the argv and argc vmvriables
   passed into a program's main() function by the operating system, from
   an MvrArgumentBuilder object, or just from a string (e.g. provided by WinMain() 
   in a Windows MFC program.)

   It will also load default argument values if you call
   loadDefaultArguments().   Mvria::init() adds the file /etc/Mvr.args and the 
   environment vmvriable ARIAARGS as locations for argument defaults, so 
   loadDefaultArguments() will always search those. You can
   use this mechanism to avoid needing to always supply command line parameters
   to all programs. For example, if you use different serial ports than the defaults
   for the robot and laser, you can put a -robotPort or -laserPort argument in 
   /etc/Mvr.args for all programs that call loadDefaultArguments() to use.
   You can add other files or environment vmvriables
   to the list of default argument locations with
   addDefaultArgumentFile() and addDefaultArgumentEnv().

   Note that the wasReallySet behavior has changed.  It used to set
   the vmvriable to false or to true, now it will only set it to false
   if 'setWasReallySetOnlyTrue' is set to false.  By default that is
   set to false, but you can just set it to true after you make the
   parser with no ill effects since all of the built in Mvr parsers
   will play nicely with this value, setting it to true or false while
   they parse but then setting it to whatever it was when they
   started.  This change in behavior is so that checking for two
   things (like -robotPort and -rp) can work without a lot of extra
   work.

   @ingroup ImportantClasses
**/
class MvrArgumentParser
{
public:
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
  MVREXPORT bool checkParameterArgumentString(const char *argument, 
					     const char **dest, 
					     bool *wasReallySet = NULL,
					     bool returnFirst = false);
  /// Returns the integer after given argument 
  MVREXPORT bool checkParameterArgumentInteger(const char *argument, int *dest,
					      bool *wasReallySet = NULL, 
					      bool returnFirst = false);
  /// Returns the word/argument after given argument 
  MVREXPORT bool checkParameterArgumentBool(const char *argument, bool *dest,
					   bool *wasReallySet = NULL,
					   bool returnFirst = false);
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
  MVREXPORT void addDefaultArgumentAsIs(const char *argument, 
				       int position = -1);
  /// Adds args from default files and environmental vmvriables
  MVREXPORT void loadDefaultArguments(int positon = 1);
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
  /// Adds another file or environmental vmvriable to the list of defaults
  MVREXPORT static void addDefaultArgumentFile(const char *file);
  /// Adds another file or environmental vmvriable to the list of defaults
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
						const char **dest, 
						const char *argument, ...);
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
						float *dest,
						 const char *argument, ...);
  /** @brief Returns the double after given argument 
   *  @swigomit
   */
  MVREXPORT bool checkParameterArgumentDoubleVar(bool *wasReallySet, 
						double *dest,
						const char *argument, ...);
#endif
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


#endif // ARARGUMENTPARSER_H
