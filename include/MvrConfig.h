#ifndef MVRCONFIG_H
#define MVRCONFIG_H

#include "MvrConfigArg.h"
#include "MvrFileParser.h"
#include "MvrHasFileName.h"
#include <set>

class MvrArgumentBuilder;
class MvrConfigSection;


/// Stores configuration information which may be read to and from files or other sources
/**
 * The configuration is a set of parameters (or config arguments), organized into
 * sections.  Parameters contain a key or name (a short string), a value 
 * (which may be one of several types), a priority (important, normal, or trivial
 * to most users), a longer description, and a display hint which suggests 
 * what kind of UI control might be most appropriate for the parameter.
 * After adding a parameter to the configuration, its value may be changed
 * when the configuration is loaded or reloaded from a file.
 * Various program modules may register callbacks to be notified
 * when a shared global configuration (such as the static MvrConfig object kept
 * by the Mvr class) is loaded or otherwise changed.
 *
 * Classes dealing with more specialized kinds of config files 
 * inherit from this one.
 *
 * Important methods in this class are: addParam(), addProcessFileCB(),
 * remProcessFileCB(), parseFile(), writeFile().
 *
 * Usually, configuration data are read from and written to a file using
 * parseFile() and writeFile(), or are set
 * by a remote client via MvrNetworking.  It is also possible to import 
 * configuration settings from an MvrArgumentParser (which, for example, may 
 * contain a program's command line arguments) using useArgumentParser().
 *
 * @warning MvrConfig does not escape any special characters when writing or
 * loading to/from a file. Therefore in general parameter names, values, section
 * names, and comments must not contain characters which have special meaning
 * in a config file, such as '#', ';', tab or newline. (see also MvrFileParser) Parameter names may 
 * have spaces, though by convention they generally do not.  

  @ingroup OptionalClasses
*/
class MvrConfig : public MvrHasFileName
{
private:

  /// Current version of the output config file.
  static const char *CURRENT_CONFIG_VERSION;
  /// Parser tag that introduces the config file version.
  static const char *CONFIG_VERSION_TAG;

  /// Current version of the output config resource file.
  static const char *CURRENT_RESOURCE_VERSION;
  /// Parser tag that introduces the config resource file version.
  static const char *RESOURCE_VERSION_TAG;

public:

  /// Sections related to I/O, ARCL, display, network communication, etc.
  MVREXPORT static const char *CATEGORY_ROBOT_INTERFACE;
  /// Sections related to robot behavior, docking, driving, navigation, etc.
  MVREXPORT static const char *CATEGORY_ROBOT_OPERATION;
  /// Sections related to the robot hardware and its sensors/accessories.
  MVREXPORT static const char *CATEGORY_ROBOT_PHYSICAL;
  /// Sections related to the fleet, enterprise manager, etc.
  MVREXPORT static const char *CATEGORY_FLEET;
  /// Sections related to security.
  MVREXPORT static const char *CATEGORY_SECURITY;
  /// Sections related to debug, including logging, replay, etc.
  MVREXPORT static const char *CATEGORY_DEBUG;

  /// Returns NULL if category name is not one of the predefined constants 
  MVREXPORT static const char *toCategoryName(const char *categoryName);

public:
  /// Constructor
  MVREXPORT MvrConfig(const char *baseDirectory = NULL, 
		                bool noBlanksBetweenParams = false,
		                bool ignoreBounds = false,
		                bool failOnBadSection = false,
		                bool saveUnknown = true);
  /// Destructor
  MVREXPORT virtual ~MvrConfig();

  /// Copy constructor
  MVREXPORT MvrConfig(const MvrConfig &config);

  /// Assignment operator
  MVREXPORT MvrConfig &operator=(const MvrConfig &config);

  /// Copies the given config to this one, detaching any pointers so they are not shared
  MVREXPORT virtual void copyAndDetach(const MvrConfig &config);

  /// Stores an optional config and robot name to be used in log messages.
  MVREXPORT virtual void setConfigName(const char *configName,
                                      const char *robotName = NULL);

  /// Turn on this flag to reduce the number of verbose log messages.
  MVREXPORT virtual void setQuiet(bool isQuiet);


  /// Sets an associate config that provides translations for each parameter (as read from a resource file).
  MVREXPORT virtual void setTranslator(MvrConfig *xlatorConfig);

  /// Returns the associated translator config, or NULL if none.
  MVREXPORT virtual MvrConfig *getTranslator() const;


  /// Parse a config file
  MVREXPORT bool parseFile(const char *fileName, 
                          bool continueOnError = false,
                          bool noFileNotFoundMessage = false, 
                          char *errorBuffer = NULL,
                          size_t errorBufferLen = 0,
                          std::list<std::string> *sectionsToParse = NULL,
                          MvrPriority::Priority highestPriority = MvrPriority::FIRST_PRIORITY,
                          MvrPriority::Priority lowestPriority  = MvrPriority::LAST_PRIORITY,
                          MvrConfigArg::RestartLevel *restartLevelNeeded = NULL);
      
  /// Write out a config file
  MVREXPORT bool writeFile(const char *fileName, 
                          bool append = false,
                          std::set<std::string> *alreadyWritten = NULL,
                          bool writeExtras = false,
                          std::list<std::string> *sectionsToWrite = NULL,
                          MvrPriority::Priority highestPriority = MvrPriority::FIRST_PRIORITY,
                          MvrPriority::Priority lowestPriority  = MvrPriority::LAST_PRIORITY);

  /// Parse a set of text lines, in the same format as the config file.
  MVREXPORT bool parseText(const std::list<std::string> &configLines,
                          bool continueOnErrors = false,
                          bool *parseOk = NULL,
                          bool *processOk = NULL,
                          char *errorBuffer = NULL,
                          size_t errorBufferLen = 0,
                          std::list<std::string> *sectionsToParse = NULL,
                          MvrPriority::Priority highestPriority = MvrPriority::FIRST_PRIORITY,
                          MvrPriority::Priority lowestPriority  = MvrPriority::LAST_PRIORITY,
                          MvrConfigArg::RestartLevel *restartLevelNeeded = NULL);

  
  /// Parse a config resource file, for translation.
  MVREXPORT bool parseResourceFile(const char *fileName, 
                                  bool continueOnError = true,
                                  char *errorBuffer = NULL,
                                  size_t errorBufferLen = 0,
                                  std::list<std::string> *sectionsToParse = NULL);
                                        
  /// Parse a config resource file with parameters suitable for custom commands.
  MVREXPORT void parseResourceFile(MvrArgumentBuilder *builder);
  
  /// Write a config resource file, for translation.
  MVREXPORT bool writeResourceFile(const char *fileName, 
                                  bool append = false,
                                  std::set<std::string> *alreadyWritten = NULL,
                                  std::list<std::string> *sectionsToWrite = NULL);

  /// Write a config resource file with parameters suitable for custom commands.
  MVREXPORT void writeResourceFile(MvrArgumentBuilder *builder);
  
  /// Command to add a section and its description to the specified category.
  /**
   * The category is expected to be one of the CATEGORY_ constants defined above.
   * If the section already exists but has not yet been assigned to a category,
   * then this method can be called to add it to the specified category.
  **/
  MVREXPORT bool addSection(const char *categoryName,
                           const char *sectionName,
                           const char *sectionDescription);


  /// Command to add a parameter to the given section with given priority
  MVREXPORT bool addParam(const MvrConfigArg &arg, 
                         const char *sectionName = "", 
                         MvrPriority::Priority priority = MvrPriority::NORMAL,
                         const char *displayHint = NULL,
                         MvrConfigArg::RestartLevel restart = MvrConfigArg::NO_RESTART);

  /// Command to add a new comment to the given section with given priority
  MVREXPORT bool addComment(const char *comment, const char *sectionName = "", 
			                     MvrPriority::Priority priority = MvrPriority::NORMAL);

  /// Adds a parameter that has all the other information on it
  /// already set
  MVREXPORT bool addParamAsIs(const MvrConfigArg &arg, 
			     const char *sectionName = "");

  /// Sets the comment for a section
  MVREXPORT void setSectionComment(const char *sectionName, 
				                          const char *comment);


  /// Uses this argument parser after it parses a file before it processes
  MVREXPORT void useArgumentParser(MvrArgumentParser *parser);

  /// for inheritors this is called after the file is processed
  /**
     For classes that inherit from MvrConfig this function is called
     after parseFile and all of the processFileCBs are called... If
     you want to call something before the processFileCBs then just
     add a processFileCB... this is only called if there were no
     errors parsing the file or continueOnError was set to false when
     parseFile was called

     @return true if the config parsed was good (parseFile will return
     true) false if the config parsed wasn't (parseFile will return false)
   **/
  MVREXPORT virtual bool processFile(void) { return true; }
  /// Adds a callback to be invoked when the configuration is loaded or
  /// reloaded.
  MVREXPORT void addProcessFileCB(MvrRetFunctor<bool> *functor, 
				 int priority = 0);
  /// Adds a callback to be invoked when the configuration is loaded
  /// or reloaded.... if you really want errors you should use
  /// addProcessFileWithErrorCB, this is just to catch mistakes
  MVREXPORT void addProcessFileCB(MvrRetFunctor2<bool, char *, size_t> *functor, 
				 int priority = 0);
  /// Adds a callback to be invoked when the configuration is loaded or
  /// reloaded, which may also receive error messages
  MVREXPORT void addProcessFileWithErrorCB(
	  MvrRetFunctor2<bool, char *, size_t> *functor, 
	  int priority = 0);
  /// Removes a processedFile callback
  MVREXPORT void remProcessFileCB(MvrRetFunctor<bool> *functor);
  /// Removes a processedFile callback
  MVREXPORT void remProcessFileCB(
	  MvrRetFunctor2<bool, char *, size_t> *functor);
  /// Call the processFileCBs
  MVREXPORT bool callProcessFileCallBacks(bool continueOnError,
					 char *errorBuffer = NULL,
					 size_t errorBufferLen = 0);
  /// This parses the argument given (for parser or other use)
  MVREXPORT bool parseArgument(MvrArgumentBuilder *arg, 
			      char *errorBuffer = NULL,
			      size_t errorBufferLen = 0);

  /// Parses the config file version information.
  MVREXPORT bool parseVersion(MvrArgumentBuilder *arg, 
			                       char *errorBuffer = NULL,
			                        size_t errorBufferLen = 0);

  /// This parses the section change (for parser or other use)
  MVREXPORT bool parseSection(MvrArgumentBuilder *arg, 
			      char *errorBuffer = NULL,
			      size_t errorBufferLen = 0);

  MVREXPORT bool parseListBegin(MvrArgumentBuilder *arg,
				                       char *errorBuffer,
				                       size_t errorBufferLen);

  MVREXPORT bool parseListEnd(MvrArgumentBuilder *arg,
				                     char *errorBuffer,
				                     size_t errorBufferLen);

  /// This parses an unknown argument (so we can save it)
  MVREXPORT bool parseUnknown(MvrArgumentBuilder *arg, 
			     char *errorBuffer = NULL,
			     size_t errorBufferLen = 0);

  /// Gets the restart level needed
  MVREXPORT MvrConfigArg::RestartLevel getRestartLevelNeeded(void) const;

  /// Gets the restart level needed
  MVREXPORT void resetRestartLevelNeeded(void);

  /// Get the base directory
  MVREXPORT const char *getBaseDirectory(void) const;
  /// Set the base directory
  MVREXPORT void setBaseDirectory(const char *baseDirectory);
  /// Get the file name we loaded
  MVREXPORT const char *getFileName(void) const;

  /// Set whether we have blanks between the params or not
  MVREXPORT void setNoBlanksBetweenParams(bool noBlanksBetweenParams);

  /// Get whether we have blanks between the params or not
  MVREXPORT bool getNoBlanksBetweenParams(void);

  /// Use an argument parser to change the config
  MVREXPORT bool parseArgumentParser(MvrArgumentParser *parser,
				    bool continueOnError = false,
				    char *errorBuffer = NULL,
				    size_t errorBufferLen = 0);


  /// Returns the list of categories contained in this config.
  MVREXPORT std::list<std::string> getCategoryNames() const;

  /// Returns the names of the sections that have been added to the specified category.
  MVREXPORT std::list<std::string> getSectionNamesInCategory(const char *categoryName) const;

  /// Returns the names of all sections in the config.
  MVREXPORT std::list<std::string> getSectionNames() const;

  /// Get the sections themselves (use only if you know what to do)
  MVREXPORT std::list<MvrConfigSection *> *getSections(void);



  /// Find the section with the given name.  
  /// @return section object, or NULL if not found.
  MVREXPORT MvrConfigSection *findSection(const char *sectionName) const;

  /// Set the log level used when loading or reloading the configuration
  MVREXPORT void setProcessFileCallbacksLogLevel(MvrLog::LogLevel level) 
    {  myProcessFileCallbacksLogLevel = level; }
  /// Get the log level used when loading or reloading the configuration
  MVREXPORT MvrLog::LogLevel getProcessFileCallbacksLogLevel(void)
    {  return myProcessFileCallbacksLogLevel; }

  /// Sets whether we save unknown items (if we don't save 'em we ignore 'em)
  MVREXPORT void setSaveUnknown(bool saveUnknown) 
    { mySaveUnknown = saveUnknown; }
  /// Gets whether we save unknowns (if we don't save 'em we ignore 'em)
  MVREXPORT bool getSaveUnknown(void) { return mySaveUnknown; }

  /// Clears out all the section information
  MVREXPORT void clearSections(void);
  /// Clears out all the section information and the processFileCBs
  MVREXPORT void clearAll(void);

  /// adds a flag to a section
  MVREXPORT bool addSectionFlags(const char *sectionName, 
				const char *flags);
  /// Removes a flag from a section
  MVREXPORT bool remSectionFlag(const char *sectionName, 
			       const char *flag);

  /// calls clearValueSet on the whole config (internal for default configs)
  MVREXPORT void clearAllValueSet(void);
  /// Removes all unset values from the config (internal for default configs)
  MVREXPORT void removeAllUnsetValues(void);
  /// Removes all values from sections in which no value has been set (internal)
  MVREXPORT void removeAllUnsetSections(void);

  /// Logs the config
  MVREXPORT void log(bool isSummary = true,
                    std::list<std::string> *sectionNameList = NULL,
                    const char *logPrefix = "");

  /// Sets permissions on some things 
  MVREXPORT void setPermissions(bool allowFactory = true, 
			       bool rememberUnknowns = true);

  /// Adds a section for this config to always skip
  MVREXPORT void addSectionNotToParse(const char *section);
  /// Removes a section for this config to always skip
  MVREXPORT void remSectionNotToParse(const char *section);
 
  /// Adds the children of the given parent arg to the config parser. 
  MVREXPORT void addListNamesToParser(const MvrConfigArg &parent);

protected:

  /// Write out a section  
  MVREXPORT void writeSection(MvrConfigSection *section, 
                             FILE *file,
                             std::set<std::string> *alreadyWritten,
                             bool writeExtras,
                             MvrPriority::Priority highestPriority,
                             MvrPriority::Priority lowestPriority);

  /// Write out a section in CSV format for translation
  MVREXPORT void writeSectionResource(MvrConfigSection *section, 
                                     FILE *file,
                                     std::set<std::string> *alreadyWritten);

  MVREXPORT void translateSection(MvrConfigSection *section);

  void copySectionsToParse(std::list<std::string> *from);

  /// Removes unset values from the config.
  /**
   * @param isRemovingUnsetSectionsOnly a bool set to true if unset parameters should
   * not be removed from sections that have any set parameters; false if all unset 
   * values should be removed
  **/
  MVREXPORT void removeAllUnsetValues(bool isRemovingUnsetSectionsOnly);


  /**
     This class's job is to make the two functor types largely look
     like the same one from the code's perspective, this is so we can
     store them both in the same map for order of operations purposes.
     
     The funkiness with the constructor is because the retfunctor2
     looks like the retfunctor and winds up always falling into that
     constructor.
  **/
  class ProcessFileCBType
  {
    public:
    ProcessFileCBType(
	    MvrRetFunctor2<bool, char *, size_t> *functor)
    {
      myCallbackWithError = functor;
      myCallback = NULL;
    }
    ProcessFileCBType(MvrRetFunctor<bool> *functor)
    {
      myCallbackWithError = NULL;
      myCallback = functor;
    }
    ~ProcessFileCBType() {}
    bool call(char *errorBuffer, size_t errorBufferLen) 
    { 
      if (myCallbackWithError != NULL) 
	      return myCallbackWithError->invokeR(errorBuffer, errorBufferLen);
      else if (myCallback != NULL) 
        return myCallback->invokeR(); 
      // if we get here there's a problem
      MvrLog::log(MvrLog::Terse, "MvrConfig: Horrible problem with process callbacks");
      return false;
    }
    bool haveFunctor(MvrRetFunctor2<bool, char *, size_t> *functor)
    { 
      if (myCallbackWithError == functor) 
        return true; 
      else 
        return false; 
    }
    bool haveFunctor(MvrRetFunctor<bool> *functor)
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
      MvrLog::log(MvrLog::Terse, "MvrConfig: Horrible problem with process callback names");
      return NULL;
    }
    protected:
    MvrRetFunctor2<bool, char *, size_t> *myCallbackWithError;
    MvrRetFunctor<bool> *myCallback;
  };

  void addParserHandlers(void);
  void remParserHandlers(void);

  /// Optional name of the robot with which the config is associated.
  std::string myRobotName;
  /// Optional name of the config instance.
  std::string myConfigName;
  /// Prefix to be inserted in log messages (contains the robot and config names).
  std::string myLogPrefix;

  MvrArgumentParser *myArgumentParser;
  std::multimap<int, ProcessFileCBType *> myProcessFileCBList;
  bool myNoBlanksBetweenParams;

  /// Version information read from config file
  std::string myConfigVersion;

  MvrConfig *myTranslator;
  char myCsvSeparatorChar;

  std::string mySection;
  std::list<std::string> *mySectionsToParse;
  std::set<std::string> mySectionsNotToParse;
  MvrPriority::Priority myHighestPriorityToParse;
  MvrPriority::Priority myLowestPriorityToParse;
  
  MvrConfigArg::RestartLevel myRestartLevelNeeded;
  bool myCheckingForRestartLevel;

  bool mySectionBroken;
  bool mySectionIgnored;
  bool myUsingSections;

  std::list<std::string> myParsingListNames;
  bool myIsParsingListBroken;

  std::string myFileName;
  std::string myBaseDirectory;
  MvrFileParser myParser;

  bool myIgnoreBounds;
  bool myFailOnBadSection;
  bool myDuplicateParams;
  bool mySaveUnknown;
  bool myIsQuiet;

  bool myPermissionAllowFactory;
  bool myPermissionSaveUnknown;

  MvrLog::LogLevel myProcessFileCallbacksLogLevel;

  /// Map that defines the section names contained in each category. 
  std::map<std::string, std::list<std::string> > myCategoryToSectionsMap;

  // our list of sections which has in it the argument list for each
  std::list<MvrConfigSection *> mySections;

  // callback for the file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> myParserCB;
  // callback for the config version in the file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> myVersionCB;
  // callback for the section in the file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> mySectionCB;

  // callback for the _listBegin in the file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> myListBeginCB;
  // callback for the _listEnd in the file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> myListEndCB;

  // callback for the unknown param (or whatever) file parser
  MvrRetFunctor3C<bool, MvrConfig, MvrArgumentBuilder *, char *, size_t> myUnknownCB;

};


/** Represents a section in the configuration. Sections are used to
 *  group items used by separate parts of Mvr.
 */
class MvrConfigSection
{
public:
  MVREXPORT MvrConfigSection(const char *name = NULL, 
						               const char *comment = NULL,
                           bool isQuiet = false,
                           const char *categoryName = NULL);

  MVREXPORT virtual ~MvrConfigSection();
  MVREXPORT MvrConfigSection(const MvrConfigSection &section);
  MVREXPORT MvrConfigSection &operator=(const MvrConfigSection &section);

  /// Copies the given section to this one, detaching any pointers so they are not shared
  MVREXPORT virtual void copyAndDetach(const MvrConfigSection &section);

  /** @return The name of this section */
  const char *getName(void) const { return myName.c_str(); }

  /** @return A comment describing this section */
  const char *getComment(void) const { return myComment.c_str(); }

  /// Returns the name of the category that contains this section.
  MVREXPORT const char *getCategoryName() const;

  const char *getFlags(void) const { return myFlags->getFullString(); }
  MVREXPORT bool hasFlag(const char *flag) const;
  
  std::list<MvrConfigArg> *getParams(void) { return &myParams; }
  
  MVREXPORT void setName(const char *name);

  MVREXPORT void setComment(const char *comment);

  MVREXPORT bool addFlags(const char *flags, bool isQuiet = false);
  MVREXPORT bool remFlag(const char *dataFlag);

  /// Finds a parameter item in this section with the given name.  Returns NULL if not found.
  MVREXPORT MvrConfigArg *findParam(const char *paramName,
                                  bool isAllowStringHolders = false);

  /// Finds a list member parameter with the specified name path.  Returns NULL if not found.
  MVREXPORT MvrConfigArg *findParam(const std::list<std::string> &paramNamePath,
                                  bool isAllowHolders = false); 

  /// Finds a list member parameter with the specified name path.  Returns NULL if not found.
  MVREXPORT MvrConfigArg *findParam(const char **paramNamePath, 
                                  int pathLength,
                                  bool isAllowHolders = false); 

  /// Determines whether the current section contains parameters in the specified priority ran
  MVREXPORT bool containsParamsOfPriority(MvrPriority::Priority highestPriority,
                                         MvrPriority::Priority lowestPriority);

  /// Removes a string holder for this param, returns true if it found one
  MVREXPORT bool remStringHolder(const char *paramName); 

  /// Turn on this flag to reduce the number of verbose log messages.
  MVREXPORT void setQuiet(bool isQuiet); 

protected:

  /// Give the config access to the protected category name setter.
  friend class MvrConfig;
  
  /// Sets the name of the category to which this section belongs.
  void setCategoryName(const char *categoryName);

protected:

  std::string myName;
  std::string myComment;
  std::string myCategoryName;
  std::string myDisplayName; // Not yet supported
  MvrArgumentBuilder *myFlags;
  std::list<MvrConfigArg> myParams;
  bool myIsQuiet;

}; // end class MvrConfigSection

#endif // ARCONFIG

