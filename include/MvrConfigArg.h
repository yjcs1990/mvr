/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#ifndef ARCONFIGARG_H
#define ARCONFIGARG_H

#include <map>

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrArgumentBuilder;
class MvrFileParser;
class MvrSocket;

/// Argument class for MvrConfig
/** 
    MvrConfigArg stores information about a parameter (name, description, type),
    and a pointer to the vmvriable that will actually store the value. This
    vmvriable is normally stored and used by whatever class or module has added the
    parameter to MvrConfig.  (In addition, there are some special types of
    MvrConfigArg that behave differently such as the "holder" types and separator,
    these are used internally or in special cases.)

    Which constructor you use determines the value type of the MvrConfigArg object.
  
    Example:
    @code
    config->addParam(MvrConfigArg("MyParameter", &myTarget, "Example Parameter"), "Example Section");
    @endcode

    Where: <tt>config</tt> is a pointer to an MvrConfig object or subclass (e.g.  use
    <tt>MvrConfig* config = Mvria::getConfig()</tt> to use the global Mvr MvrConfig
    object);
    <tt>myTarget</tt> is a vmvriable (e.g. int) that is a class member whose instance will not
    be destroyed before the end of the program, or which will remove the parameter
    from MvrConfig before being destroyed (the pointer to <tt>myTarget</tt> that is stored
    in MvrConfig must not become invalid.)  The MvrConfigArg object passed to
    addParam() will be copied and stored in MvrConfig.  The type of the target
    vmvriable (<tt>myTarget</tt>) determines the type of the parameter.


    @swignote Swig cannot determine the correct constructor to use
     based on most target langugages types, so you must use subclasses
     defined for various types. Or, use the constructor that accepts 
     functors for dealing with arguments.  Also, Swig cannot use pointers
     to change vmvriables, so you must create MvrConfigArg objects, passing
     in default values, and retain references to those objects,
     in addition to passing them to MvrConfig, and read new values from those
     objects if MvrConfig changes; or pass functors to MvrConfigArg instead
     of the initial value.

    Special features:

    You can add a separator line to the section by adding an MvrConfigArg with type SEPARATOR: 
    @code
      config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), "My Section");
    @endcode

    You can specify a list of possible values by setting a "display hint".  For
    example, for a string parameter with possible values "A", "B" and "C":
    @code
      MvrConfigArg arg("Example", exampleString, "Example string parameter");
      arg.setDisplayHint("Choices:A;;B;;C");
      config->addParam(arg);
    @endcode
    This information is provided to clients, but you should still verify the
    value is not unexpected when it changes or when you use the value.
*/
class MvrConfigArg
{
public:

  typedef enum 
  { 
    INVALID, ///< An invalid argument, the argument wasn't created correctly
    INT, ///< Integer argument
    DOUBLE, ///< Double argument
    STRING, ///< String argument
    BOOL, ///< Boolean argument
    FUNCTOR, ///< Argument that handles things with functors
    DESCRIPTION_HOLDER, ///< Argument that just holds a description
    STRING_HOLDER, ///< this one is for holding strings and reading them in and writing them out but not really letting them get sent anywhere (its for unknown config parameters (so they don't get lost if a feature is turned off)
    SEPARATOR, ///< Empty argument that merely acts as a separator within a (large) section.
    CPPSTRING, ///< Pointer to std::string, use like STRING.
    LIST, ///< Composite argument that contains an ordered list of other args
    LIST_HOLDER, ///< Placeholder for composite argument that are not currently active
    LAST_TYPE = LIST_HOLDER ///< Last value in the enumeration
  } Type;


  /// Indicates the components that must be restarted if the parameter is changed
  enum RestartLevel {
    NO_RESTART,       //< No restart necessary (default value)
    RESTART_CLIENT,   //< Client software must be restarted (TODO Can this be eliminated?)
    RESTART_IO,   //< IO has changed and needs to be restarted
    RESTART_SOFTWARE,   //< Server software must be restarted
    RESTART_HARDWARE,    //< Physical robot must be rebooted
    LAST_RESTART_LEVEL = RESTART_HARDWARE
  };

  enum {
    DEFAULT_DOUBLE_PRECISION = 5, ///< Default double precision, originates from previous saving behavior
    TYPE_COUNT = LAST_TYPE + 1, ///< Number of argument types
    RESTART_LEVEL_COUNT = LAST_RESTART_LEVEL + 1, ///< Number of restart levels
  };


  enum SocketIndices {

    SOCKET_INDEX_OF_SECTION_NAME = 0,
    SOCKET_INDEX_OF_ARG_NAME = 1,
    SOCKET_INDEX_OF_DESCRIPTION = 2,
    SOCKET_INDEX_OF_PRIORITY = 3,
    SOCKET_INDEX_OF_TYPE = 4,

    SOCKET_INDEX_OF_VALUE = 5,
    SOCKET_INDEX_OF_MIN_VALUE = 6,
    SOCKET_INDEX_OF_MAX_VALUE = 7,

    SOCKET_INDEX_OF_DISPLAY = 8, 
    SOCKET_INDEX_OF_PARENT_PATH = 9,
    SOCKET_INDEX_OF_SERIALIZABLE = 10

  }; // end enum SocketIndices


  enum ResourceIndices {

    RESOURCE_INDEX_OF_SECTION_NAME = 0,
    RESOURCE_INDEX_OF_ARG_NAME = 1,
    RESOURCE_INDEX_OF_TYPE = 2,
    RESOURCE_INDEX_OF_PRIORITY = 3,
    RESOURCE_INDEX_OF_RESTART_LEVEL = 4,
    RESOURCE_INDEX_OF_PARENT_PATH = 5,
    RESOURCE_INDEX_OF_DESCRIPTION = 6,
    RESOURCE_INDEX_OF_EXTRA = 7,
    RESOURCE_INDEX_OF_DISPLAY = 8, // not yet supported
    RESOURCE_INDEX_OF_NEW = 9 

  }; // end enum ResourceIndices



  /// Keyword that indicates the start of an MvrConfigArg LIST object, for MvrFileParser.
  MVREXPORT static const char *LIST_BEGIN_TAG;
  /// Keyword that indicates the end of an MvrConfigArg LIST object, for MvrFileParser.
  MVREXPORT static const char *LIST_END_TAG;

  /// Resource file keyword that indicates an empty string (cannot write empty for csv).
  MVREXPORT static const char *NULL_TAG;
  /// Resource file keyword that indicates a new entry.
  MVREXPORT static const char *NEW_RESOURCE_TAG;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Static Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns a text representation of the given type, suitable for logging.
  MVREXPORT static const char *toString(Type t);

  /// Returns a text representation of the given RestartLevel, suitable for logging.
  MVREXPORT static const char *toString(RestartLevel r);


  /// Given a text representation, returns the appropriate Type.
  MVREXPORT static Type typeFromString(const char *text);

  /// Given a text representation, returns the appropriate RestartLevel.
  MVREXPORT static RestartLevel restartLevelFromString(const char *text);



  /// Returns the section name contained in the given resource arg.
  MVREXPORT static std::string parseResourceSectionName(MvrArgumentBuilder *arg, 
                                                       const char *logPrefix = "");

  /// Returns the param name contained in the given resource arg.
  MVREXPORT static std::string parseResourceArgName(MvrArgumentBuilder *arg, 
                                                   const char *logPrefix = "");

  /// Returns the arg type contained in the given resource arg.
  MVREXPORT static Type parseResourceType(MvrArgumentBuilder *arg, 
                                         const char *logPrefix = "");

  /// Returns true if the given resource arg is "top-level", i.e. not a list member.
  MVREXPORT static bool isResourceTopLevel(MvrArgumentBuilder *arg, 
                                          const char *logPrefix = "");

  /// Returns the parent path contained in the resource arg for list members.
  MVREXPORT static std::list<std::string> parseResourceParentPath(MvrArgumentBuilder *arg,
                                                                 char separator = '|',
                                                                 const char *logPrefix = "");
  
  /// Returns the description contained in the given resource arg.
  MVREXPORT static std::string parseResourceDescription(MvrArgumentBuilder *arg, 
                                                       const char *logPrefix = "");
  /// Returns the extra explanation contained in the given resource arg.
  MVREXPORT static std::string parseResourceExtra(MvrArgumentBuilder *arg, 
                                                       const char *logPrefix = "");

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Instance Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Default empty contructor
  MVREXPORT MvrConfigArg();
  /// Constructor for making an integer argument by pointer (4 bytes)
  MVREXPORT MvrConfigArg(const char * name, int *pointer, 
		       const char * description = "", 
		       int minInt = INT_MIN, 
		       int maxInt = INT_MAX); 
  /// Constructor for making an int argument thats a short (2 bytes)
  MVREXPORT MvrConfigArg(const char * name, short *pointer, 
		       const char * description = "", 
		       int minInt = SHRT_MIN, 
		       int maxInt = SHRT_MAX); 
  /// Constructor for making an int argument thats a ushort (2 bytes)
  MVREXPORT MvrConfigArg(const char * name, unsigned short *pointer, 
		       const char * description = "", 
		       int minInt = 0, 
		       int maxInt = USHRT_MAX); 
  /// Constructor for making an char (1 byte) argument by pointer (treated as int)
  MVREXPORT MvrConfigArg(const char * name, unsigned char *pointer, 
		       const char * description = "", 
		       int minInt = 0,
		       int maxInt = 255); 
  /// Constructor for making a double argument by pointer
  MVREXPORT MvrConfigArg(const char * name, 
                       double *pointer,
		                   const char * description = "", 
		                   double minDouble = -HUGE_VAL,
		                   double maxDouble = HUGE_VAL,
                       int precision = DEFAULT_DOUBLE_PRECISION); 
  /// Constructor for making a boolean argument by pointer
  MVREXPORT MvrConfigArg(const char * name, bool *pointer,
		       const char * description = ""); 
  /// Constructor for making an argument of a string by pointer (see details)
  MVREXPORT MvrConfigArg(const char *name, char *str, 
		       const char *description,
		       size_t maxStrLen);
  /// Constructor for making an argument of a string by pointer (see details)
  MVREXPORT MvrConfigArg(const char *name, const char *str, 
		       const char *description);

  /// Constructor for making an argument of a C++ std::string 
  MVREXPORT MvrConfigArg(const char *name, std::string *str, const char *description);
  
  /// Constructor for making an integer argument
  MVREXPORT MvrConfigArg(const char * name, int val, 
		       const char * description = "", 
		       int minInt = INT_MIN, 
		       int maxInt = INT_MAX); 
  /// Constructor for making a double argument
  MVREXPORT MvrConfigArg(const char * name, 
                       double val,
		                   const char * description = "", 
		                   double minDouble = -HUGE_VAL,
		                   double maxDouble = HUGE_VAL,
                       int precision = DEFAULT_DOUBLE_PRECISION); 
  /// Constructor for making a boolean argument
  MVREXPORT MvrConfigArg(const char * name, bool val,
		       const char * description = ""); 
  /// Constructor for making an argument that has functors to handle things
  MVREXPORT MvrConfigArg(const char *name, 
		 MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor, 
		 MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *getFunctor,
		 const char *description);

  /// Constructor for just holding a description (for MvrConfig)
  MVREXPORT MvrConfigArg(const char *str, Type type = DESCRIPTION_HOLDER);
  /// Constructor for holding an unknown argument (STRING_HOLDER)
  MVREXPORT MvrConfigArg(const char *name, const char *str);

  /// Constructs a new named argument of the specified type.
  MVREXPORT MvrConfigArg(Type type,
                       const char *name, 
		                   const char *description);

  /// Constructs a new argument of the specified type.
  MVREXPORT MvrConfigArg(Type type);

  /// Destructor
  MVREXPORT virtual ~MvrConfigArg();

  /// Copy constructor that allows a new name to be assigned
  MVREXPORT MvrConfigArg(const char *argName,
                       const MvrConfigArg & arg);

  /// Copy constructor
  MVREXPORT MvrConfigArg(const MvrConfigArg & arg);
  /// Assignment operator
  MVREXPORT MvrConfigArg &operator=(const MvrConfigArg &arg);

  /// Copies the given arg to this one, detaching any pointers so they are not shared
  MVREXPORT void copyAndDetach(const MvrConfigArg &arg);


  /// Copies the translation data from given arg to this one.
  MVREXPORT bool copyTranslation(const MvrConfigArg &arg);

  /// Converts a list holder argument to an actual list and copies the children from arg.
  MVREXPORT bool promoteList(const MvrConfigArg &arg);
  
  /// Whether the arg type is LIST or LIST_HOLDER
  MVREXPORT bool isListType() const;

  /// Gets the type of the argument
  MVREXPORT MvrConfigArg::Type getType(void) const;
  /// Gets the name of the argument
  MVREXPORT const char *getName(void) const;
  /// Gets the brief description of the argument
  MVREXPORT const char *getDescription(void) const;

  /// Sets the description (normally given in the constructor)
  MVREXPORT void setDescription(const char *description);

  /// For arguments that require more than a brief description, set the extra explanation.
  MVREXPORT void setExtraExplanation(const char *extraExplanation);

  /// Returns the extra explanation, if any, for this argument
  MVREXPORT const char *getExtraExplanation() const;


  /// Sets the argument value, for int arguments
  MVREXPORT bool setInt(int val, 
                       char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                       bool doNotSet = false);

  /// Sets the argument value, for double arguments
  MVREXPORT bool setDouble(double val, 
                          char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                          bool doNotSet = false);

  /// Sets the argument value, for bool arguments
  MVREXPORT bool setBool(bool val, 
                        char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                        bool doNotSet = false);

  /// Sets the argument value for string arguments
  MVREXPORT bool setString(const char *str, 
                          char *errorBuffer = NULL, size_t errorBufferLen = 0, 
                          bool doNotSet = false);

  MVREXPORT bool setCppString(const std::string &str,
                          char *errorBuffer = NULL, size_t errorBufferLen = 0,
                          bool doNotSet = false);

  /// Sets the argument by calling the setFunctor callback
  MVREXPORT bool setArgWithFunctor(MvrArgumentBuilder *argument, 
				  char *errorBuffer = NULL,
				  size_t errorBufferLen = 0,
				  bool doNotSet = false);



  /// Gets the argument value, for int arguments
  MVREXPORT int getInt(bool *ok = NULL) const; 

  /// Gets the minimum int value
  MVREXPORT int getMinInt(bool *ok = NULL) const;
  /// Gets the maximum int value
  MVREXPORT int getMaxInt(bool *ok = NULL) const;


  /// Gets the argument value, for double arguments
  MVREXPORT double getDouble(bool *ok = NULL) const;
  /// Gets the minimum double value
  MVREXPORT double getMinDouble(bool *ok = NULL) const;
  /// Gets the maximum double value
  MVREXPORT double getMaxDouble(bool *ok = NULL) const;
  /// Gets the decimal precision of the double
  MVREXPORT int getDoublePrecision(bool *ok = NULL) const;

  /// Gets the argument value, for bool arguments
  MVREXPORT bool getBool(bool *ok = NULL) const;

  /// Gets the argument value, for string (and string holder) arguments
  MVREXPORT const char *getString(bool *ok = NULL) const;

  /// Gets the argument value, which is a list of argumentbuilders here
  MVREXPORT const std::list<MvrArgumentBuilder *> *getArgsWithFunctor(bool *ok = NULL) const;

  MVREXPORT std::string getCppString(bool *ok = NULL) const;

  MVREXPORT const std::string* getCppStringPtr(bool *ok = NULL) const;

  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Methods for LIST Type 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Adds a child arg to this arg.  Valid only for LIST type; otherwise returns false.
  MVREXPORT bool addArg(const MvrConfigArg &arg); 

  /// Removes the child arg that has the same name as the specified one. Valid only for LIST type.
  MVREXPORT bool removeArg(const MvrConfigArg  &arg);

  /// Returns whether the list contains child args. Valid only for LIST type; otherwise returns false.
  MVREXPORT bool hasArgs() const;

  /// Returns the number of child args. Valid only for LIST type; otherwise returns 0.
  MVREXPORT size_t getArgCount() const;   

  /// Returns the total number of descendent args (children, grandchildren, etc). Valid only for LIST type; otherwise returns 0.
  MVREXPORT size_t getDescendantArgCount() const;

  // KMC 7/9/12 Right now, the returned args will not have the parent set to this arg.
  // I suspect that this may present an implementation issue later but am not sure.
  // Perhaps the addition of an iterator would suffice.
  //
  /// Returns a list of all child args in this arg.  Valid only for LIST type; otherwise returns an empty list.
  MVREXPORT std::list<MvrConfigArg> getArgs(bool *ok = NULL) const;

  /// Returns the child arg at the specified index (between 0 and getArgCount()).
  MVREXPORT const MvrConfigArg *getArg(size_t index) const;

  /// Returns the child arg at the specified index (between 0 and getArgCount()).
  MVREXPORT MvrConfigArg *getArg(size_t index);

  /// Finds the specified child arg.  Valid only for LIST type; otherwise returns NULL.
  MVREXPORT const MvrConfigArg *findArg(const char *childParamName) const;

  /// Finds the specified child arg.  Valid only for LIST type; otherwise returns NULL.
  MVREXPORT MvrConfigArg *findArg(const char *childParamName);

  /// If the arg is a list member, returns all ancestors in order.
  MVREXPORT bool getAncestorList(std::list<MvrConfigArg*> *ancestorListOut);

  /// If the arg is a list member, returns the top-most arg. Otherwise, returns this.
  MVREXPORT const MvrConfigArg *getTopLevelArg() const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous Attributes
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  /// Gets the priority (only used by MvrConfig)
  MVREXPORT MvrPriority::Priority getConfigPriority(void) const;
  /// Sets the priority (only used by MvrConfig)
  MVREXPORT void setConfigPriority(MvrPriority::Priority priority);

  /// Returns the display hint for this arg, or NULL if none is defined.
  MVREXPORT const char *getDisplayHint() const;
  /// Sets the display hint for this arg.
  MVREXPORT void setDisplayHint(const char *hintText);

  /// Gets the restart level of this parameter (only used by MvrConfig)
  MVREXPORT RestartLevel getRestartLevel() const;
  /// Sets the restart level of this parameter (only used by MvrConfig)
  MVREXPORT void setRestartLevel(RestartLevel level);

  /// Gets if notifications of changes are suppressed (for the central
  /// server config)
  MVREXPORT bool getSuppressChanges(void) const;
  /// Sets if notifications of changes are suppressed (for the central
  /// server config)
  MVREXPORT void setSuppressChanges(bool suppressChanges);


  /// Returns whether the configuration parameter should be saved in the file (default is true).
  MVREXPORT bool isSerializable() const;

  /// Sets whether the configuration parameter should be saved in the file (default is true).
  MVREXPORT void setSerializable(bool isSerializable);


  /// Returns a pointer to the immediate parent arg.  If this is not a child MvrConfigArg, then returns NULL.
  MVREXPORT MvrConfigArg *getParentArg() const;

  /// If getParentArg() is not null, then returns the path to the top level, as a single string
  MVREXPORT std::string getParentPathName(char separator = '|') const;

  /// Given a parent path, as a single string, splits it in a format useable by MvrConfigSection findParam
  MVREXPORT static std::list<std::string> splitParentPathName(const char *parentPathName,
                                                             char separator = '|');


  /// Replaces spaces in the name with underscores
  MVREXPORT void replaceSpacesInName(void);

  /// Returns whether the arg has a minimum value, currently applicable to INTs and DOUBLEs
  MVREXPORT bool hasMinBound() const;

  /// Returns whether the arg has a maximum value, currently applicable to INTs and DOUBLEs
  MVREXPORT bool hasMaxBound() const;

  /// Sets whether to ignore bounds or not (default is to not to)
  MVREXPORT void setIgnoreBounds(bool ignoreBounds = false);

  /// Returns true if this arg points to a member of another object, false if arg is self-contained.
  MVREXPORT bool hasExternalDataReference() const;

  /// Returns true if this is a special placeholder arg, i.e. string, list, or description.
  MVREXPORT bool isPlaceholder() const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Generic (Type-Independent) Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File Parsing 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Adds given parserCB handler for the appropriate keyword(s) to the given file parser.
  MVREXPORT bool addToFileParser(MvrFileParser *parser,
                                MvrRetFunctor3C<bool, 
                                               MvrConfig, 
                                               MvrArgumentBuilder *, 
                                               char *, 
                                               size_t> *parserCB,                              
                                const char *logPrefix = "",
                                bool isQuiet = false) const;



  /// Sets the value of this arg to the data described in the given MvrArgumentBuilder
  MVREXPORT bool parseArgument(MvrArgumentBuilder *arg, 
				                      char *errorBuffer,
				                      size_t errorBufferLen,
                              const char *logPrefix = "",
                              bool isQuiet = false,
                              bool *changed = NULL);

  /// Writes this arg to the given file, in a format suitable for reading by parseArgument.
  MVREXPORT bool writeArguments(FILE *file,
                               char *lineBuf,
                               int lineBufSize,
                               int startCommentColumn,
                               bool isWriteExtra = false,
                               const char *logPrefix = "",
                               int indentLevel = 0) const;

  /// Writes the name of this arg to the given buffer, indenting as specified.
  MVREXPORT bool writeName(char *lineBuf,
                          int lineBufSize,
                          int indentLevel) const;

  /// Writes the min/max values of this arg to the given buffer, as applicable.
  MVREXPORT bool writeBounds(char *line,
                            size_t lineLen,
                            const char *logPrefix = "") const;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Sockets
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Determines whether the given arg text is empty or the special "None" identifier. 
  MVREXPORT static bool isNone(const char *argText);

  /// Parses a new config arg definition from the given arguments (generally received on a text socket).
  /**
   * The given args are formatted according to the SocketIndices defined above.
  **/
  MVREXPORT bool parseSocket(const MvrArgumentBuilder &args,
                            char *errorBuffer,
                            size_t errorBufferLen);

  /// Writes the value of this parameter, and all child parameters, to the given text socket.
  /**
   *  This method is primarily intended for limited, internal use. The format of the 
   *  output string is:
   *     <i>intro</i> <i>paramName</i> <i>paramValue</i>
  **/
  MVREXPORT bool writeValue(MvrSocket *socket,
                           const char *intro) const;

  /// Writes the definition of this parameter, and all child parameters, to the given text socket.
  /**
   * This method is primarily intended for limited, internal use. The format of the 
   * output string is:
   *    <i>intro</i> <i>type</i> <i>paramName</i> <i>priority</i> <i>min</i> <i>max</i> <i>description</i> <i>displayHint</i> <i>listDelimiter</i>
  **/
  MVREXPORT bool writeInfo(MvrSocket *socket,
                          const char *intro) const;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Misc
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
  /// Logs the type, name, and value of this argument
  MVREXPORT void log(bool verbose = false,
                    int indentCount = 1,
                    const char *logPrefix = "") const;

  /// Checks only the name, type, and value attributes and returns whether they are equal.
  MVREXPORT bool isValueEqual(const MvrConfigArg &other) const;
  
  /// If the given source is of the same type, copies its value to this arg
  /**
   * Note that this method currently only works for the primitive arg
   * types (i.e. int, bool, etc.).  It doesn't copy functors or description
   * holders.
   *
   * @param source the MvrConfigArg whose value is to be copied to this arg
   * @param isVerifyArgNames a bool set to true if the argument value should
   * be set only if the given source has the same name as this argument;
   * the default value is false
   * @return bool true if the value was copied; false if the source was of a 
   * different (or non-copyable) type
  **/
  MVREXPORT bool setValue(const MvrConfigArg &source,
                         bool isVerifyArgNames = false);

  /// Gets whether this value has been set since it was last cleared or not
  bool isValueSet(void) { return myValueSet; }
  
  /// Tells the configArg that the value hasn't been set
  void clearValueSet(void) { myValueSet = false; }

  /// For special arg types (such as LIST), sets the flag to indicate the value was set.
  void setValueSet(void) { myValueSet = true; }


  /// Returns whether the arg has been translated by a resource file.
  MVREXPORT bool isTranslated() const;
  
  /// Sets whether the arg has been translated by a resource file.
  MVREXPORT void setTranslated(bool b);
  

  enum {
    MAX_RESOURCE_ARG_TEXT_LENGTH = 1024
  };

  /// Sets the value of this arg to the resource data described in the given MvrArgumentBuilder
  MVREXPORT bool parseResource(MvrArgumentBuilder *arg, 
                              char *errorBuffer,
				                      size_t errorBufferLen,
                              const char *logPrefix = "",
                              bool isQuiet = false);



  /// Parses the text obtained from the resource arg, strips surrounding quotes, blanks, and funny chars.
  MVREXPORT static bool parseResourceArgText(const char *argText,
                                            char *bufOut,
                                            size_t bufLen);

  /// Writes this arg to the given file, in a format suitable for reading by parseResource.
  MVREXPORT bool writeResource(FILE *file,
                         char *lineBuf,
                         int lineBufSize,
                         char separatorChar,
                         const char *sectionName,
                         const char *logPrefix = "") const;
 

  /// Writes column labels and edit info to the given resource file, returns number of lines written
  MVREXPORT static int writeResourceHeader(FILE *file,
                                          char *lineBuf,
                                          int lineBufSize,
                                          char separatorChar,
                                          const char *sectionTitle,
                                          const char *logPrefix = "");
  
  /// Writes the section description to the resource file.
  MVREXPORT static int writeResourceSectionHeader(FILE *file,
                                                 char *lineBuf,
                                                 int lineBufSize,
                                                 char separatorChar,
                                                 const char *sectionName,
                                                 const char *sectionDesc,
                                                 const char *sectionExtra,
                                                 const char *sectionDisplayName,
                                                 bool isTranslated,
                                                 const char *logPrefix = "");

protected:

  // Giving MvrConfig access to the write method
  friend class MvrConfig;

  /// Writes the given comment to the specified file, spanning multiple lines as necessary.
  MVREXPORT static bool writeMultiLineComment(const char *comment,
                                    FILE *file,
                                    char *lineBuf,
                                    int lineBufSize,
                                    const char *startComment);


private:

 enum IntType {
    INT_NOT, ///< Not an int
    INT_INT, ///< An int (4 bytes) 
    INT_SHORT, ///< A short (2 bytes)
    INT_UNSIGNED_SHORT, ///< An unsigned short (2 bytes)
    INT_UNSIGNED_CHAR ///< An unsigned char (1 byte)
  };

  /// Internal helper function
  void clear(bool initial, 
             Type type = INVALID, 
             IntType intType = INT_NOT,
             bool isDelete = true);

  /// Copies the given arg to this one, optionally detaching any internal pointer
  void copy(const MvrConfigArg &arg, bool isDetach = false);
  
  void set(MvrConfigArg::Type type,
           const char *name,
           const char *description,
           IntType intType = INT_NOT);

  /// Sets the parent pointer of this arg.
  void setParent(MvrConfigArg *parentArg);


// KMC 7/11/12 Changed from protected to private so future changes are less
// of a concern
// protected:
private:

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to INT args
  struct MvrConfigIntData {

    // int data
    MvrConfigArg::IntType myIntType;

    // If unions need to be avoided for some reason, then this can simply 
    // be turned into a struct.
    //
    // begin union (based on myIntType)
    union {
      int            *myIntPointer;
      short          *myIntShortPointer;
      unsigned short *myIntUnsignedShortPointer;
      unsigned char  *myIntUnsignedCharPointer;
    };
    // end union (based on myIntType)

    int myMinInt;
    int myMaxInt;

  }; // end struct MvrConfigIntData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to DOUBLE args
  struct MvrConfigDoubleData {
   
    double *myDoublePointer;
    double  myMinDouble;
    double  myMaxDouble;
    int     myPrecision;

  }; // end struct MvrConfigDoubleData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to BOOL args
  struct MvrConfigBoolData {
    
    bool *myBoolPointer;

  }; // end struct MvrConfigBoolData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to STRING and STRING_HOLDER args
  struct MvrConfigStringData {

    // string data
    char *myStringPointer;
    size_t myMaxStrLen;
    // KMC own string change
    // Not seeing the functional difference between this and myOwnPointedTo 
    // (which was previously applied to all but strings)
    // bool myUsingOwnedString;
    std::string *myString; ///< Used if myOwnedString is true. (Union member cannot have a copy constructor so using a pointer)

  }; // end struct MvrConfigStringData

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to CPPSTRING
  struct MvrConfigCppStringData {
    std::string *myCppStringPtr;
  };
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to LIST args
  struct MvrConfigListData {

    std::list<MvrConfigArg> *myChildArgList;

  }; // end struct MvrConfigListData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /// Data applicable to FUNCTOR args
  struct MvrConfigFunctorData {

    MvrRetFunctor1<bool, MvrArgumentBuilder *> *mySetFunctor;
    MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *myGetFunctor;

  }; // end struct MvrConfigFunctorData


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // If unions need to be avoided for some reason, then this can simply 
  // be turned into a struct.
  //
  /// Data that varies according to type of arg
  union MvrConfigArgData { // begin union (based on myType)
    MvrConfigIntData     myIntData;
    MvrConfigDoubleData  myDoubleData;
    MvrConfigBoolData    myBoolData;
    MvrConfigStringData  myStringData;
    MvrConfigListData    myListData;
    MvrConfigFunctorData myFunctorData;
    MvrConfigCppStringData myCppStringData;
  }; // end union (based on myType)
  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Number of spaces to indent each level of list contents
  static int ourIndentSpaceCount;

  static std::map<std::string, Type, MvrStrCaseCmpOp> *ourTextToTypeMap;
  static std::map<std::string, RestartLevel, MvrStrCaseCmpOp> *ourTextToRestartLevelMap;

  /// Type of this arg
  MvrConfigArg::Type myType;
  /// Name of this arg (may be empty for SEPARATOR args)
  std::string myName;
  /// Brief description of this arg
  std::string myDescription;
  /// An optional extra explanation for args that need clarification
  std::string myExtraExplanation;

  /// Name to be displayed in client applications. (Not yet supported.)
  std::string myDisplayName;
  
  MvrConfigArgData myData;

  /// Priority of this arg
  MvrPriority::Priority myConfigPriority;
  /// Optional display hint used by clients
  std::string myDisplayHint;
  /// Indicates whether modifying the arg will result in a system restart
  RestartLevel myRestartLevel;

  /// Pointer to the parent arg, or NULL if this arg is not a member of a LIST 
  MvrConfigArg *myParentArg;

  /// Whether this arg "owns" its data, i.e. does not point to a member of another object
  bool myOwnPointedTo : 1;
  /// Whether this arg has been set to its initial value
  bool myValueSet     : 1;
  /// Whether any min/max bounds requirements on this arg should be ignored
  bool myIgnoreBounds : 1;
  /// Whether this arg has been translated by a resource file
  bool myIsTranslated : 1;
  /// Supressses change notification (for central server config params that should cause a restart on a robot, but not on the central server)
  bool mySuppressChanges : 1;
  /// Whether this arg should be written to the configuration file
  bool myIsSerializable : 1;

}; // end class MvrConfigArg

#endif // ARCONFIGARG_H
