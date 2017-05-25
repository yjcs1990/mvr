/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMapUtils.h
 > Description  : Contains utility classes for the MvrMap class.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
/*
 * This file contains miscellaneous helper classes for the MvrMap class.  
 * These are primarily related to tracking changes to the map and include
 * the following:
 *
 *  - MvrMapId : The unique identifier for an Mvria map.
 *
 *  - MvrMapFileLine : The data regarding a text line in a map file; this  
 *    includes the line number and text.  
 *
 *  - MvrMapFileLineGroup : A semantic parent / child relationship between 
 *    text lines in a map file. 
 *
 *  - MvrMapFileLineSet : A list of map file line groups to be compared.  
 *    Contains helper methods to determines lines that have been added or 
 *    deleted.
 *
 *  - MvrMapChangeDetails : A collection of MvrMapFileLineSets that describes 
 *    all of the changes that were made to an Mvria map.
 *
 *  - MvrMapFileLineSetWriter : An output functor that is used to populate an
 *    MvrMapFileLineSet.
 *
 *  - MvrMapChangedHelper : A collection of callbacks and methods to invoke 
 *    them after the Mvria map has been changed.
 */

#ifndef ARMAPUTILS_H
#define ARMAPUTILS_H

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"


class MvrArgumentBuilder;
class MvrBasePacket;

// ============================================================================
// MvrMapId
// ============================================================================

/// Enapsulates the data used to uniquely identify an Mvria map
/**
 * MvrMapId is a small helper class that contains all of the information 
 * needed to uniquely identify an Mvria map.  In addition to the standard
 * file attributes (such as filename, size, and timestamps), it 
 * contains the name of the originating source robot or server.  It 
 * also contains the checksum of the file contents.  This checksum is
 * currently calculated by the MvrMD5Calculator class.  All of the 
 * data should allow one to determine whether two map files are the same 
 * with a reasonable degree of certainty.
 * <p>
 * The MvrMapId class is not thread-safe.
 *
 * @see MvrMD5Calculator
**/
class MvrMapId {

public:
  // --------------------------------------------------------------------------
  // Static Methods
  // --------------------------------------------------------------------------

  /// Given a local file name, creates the map ID
  /**
   * This method calculates the checksum of the specified file
   * @param fileName the const char * name of the file for which to create the 
   * map ID
   * @param mapIdOut a pointer to the map ID to be filled in with the results
   * @return bool true if the file was found and the map ID created; false, 
   * otherwise.
  **/
  MVREXPORT static bool create(const char *fileName,
                               MvrMapId *mapIdOut);

  /// Inserts the given map ID into a network packet
  /**
   * The format of the map ID in the network packet is as follows:
   *   string:  source robot or server name
   *   string:  map file name
   *   uByte4:  number of bytes in the checksum
   *   data  :  checksum, included only if the number of bytes in the checksum 
   *            is greater than 0
   *   uByte4:  file size
   *   byte4 :  file last modified time
   * @param mapId the MvrMapId to be inserted into the packet
   * @param packetOut the MvrBasePacket * to be modified
  **/
  MVREXPORT static bool toPacket(const MvrMapId &mapId,
                                 MvrBasePacket *packetOut);
                                
  /// Extracts a map ID from the given network packet
  /**
   * @param packetIn the MvrBasePacket from which to read the map ID
   * @param mapIdOut a pointer to the MvrMapId to be filled in with the results
   * @return bool true if the map ID was successfully read from the given
   * packet, false otherwise.
   * @see toPacket for a description of the packet format
  **/
  MVREXPORT static bool fromPacket(MvrBasePacket *packetIn,
                                  MvrMapId *mapIdOut);

  // --------------------------------------------------------------------------
  // Instance Methods
  // --------------------------------------------------------------------------
  
  /// Default contructor creates a null map ID.
	MVREXPORT MvrMapId();
	
	/// Creates a map ID with the given attributes.
	/**
	 * @param sourceName the const char * name of the robot or server 
	 * from which the map originated
	 * @param fileName the const char * name of the map file
	 * @param checksum the unsigned char * buffer that contains the 
	 * file checksum
	 * @param checksumLength the size_t length of the checksum buffer
	 * @param size the long int number of bytes in the map file
	 * @param timestamp the time_t last modified time of the map file
	 * @see MvrMD5Calculator
	**/
	MVREXPORT MvrMapId(const char *sourceName,
                     const char *fileName,
						         const unsigned char *checksum,
                     size_t checksumLength,
						         long int size,
						         const time_t timestamp);
 
  /// Copy constructor
	MVREXPORT MvrMapId(const MvrMapId &other); 
	
	/// Assignment operator
	MVREXPORT MvrMapId &operator=(const MvrMapId &other);
	
	/// Destructor
	MVREXPORT ~MvrMapId();


  // --------------------------------------------------------------------------
  // Getters
  // --------------------------------------------------------------------------
  
  /// Returns whether this map ID is null 
  MVREXPORT bool isNull() const;

  /// Returns the string name of the originating robot or server
	MVREXPORT const char *getSourceName() const;
	
	/// Returns the string name of the file 
	/**
	 * TODO: Does this include path name??
	**/
	MVREXPORT const char *getFileName() const;
	
	/// Returns a pointer to the buffer that contains the checksum
	MVREXPORT const unsigned char *getChecksum() const;
	/// Returns the length of the checksum
  MVREXPORT size_t getChecksumLength() const;
	
	/// Returns the checksum in a human readable string format
  MVREXPORT const char *getDisplayChecksum() const;
	
	/// Returns the number of bytes in the map file
  MVREXPORT long int getSize() const;
  /// Returns the last modified time of the file
	MVREXPORT time_t getTimestamp() const;

 
  // --------------------------------------------------------------------------
  // Setters
  // --------------------------------------------------------------------------
  
  /// Clears this map ID, i.e. sets it to null
  MVREXPORT void clear();
   
  /// Sets the name of the source robot or server that originated the map file
	MVREXPORT void setSourceName(const char *sourceName);
	/// Sets the map file name
	MVREXPORT void setFileName(const char *fileName);
	/// Sets the checksum of the map file
	MVREXPORT void setChecksum(const unsigned char *checksum,
													   size_t checksumLen);
  /// Sets the number of bytes in the map file
	MVREXPORT void setSize(long int size);
	/// Sets the last modified time of the map file
	MVREXPORT void setTimestamp(const time_t &timestamp);

  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------
 
  /// TODO Think that this is the same as operator==
  MVREXPORT bool isSameFile(const MvrMapId &other) const;
  
  /// Returns whether the source and file names are identical
  MVREXPORT bool isVersionOfSameFile(const MvrMapId &other) const;
  
  /// Returns true if the timestamp is valid; false if it's a special 'not-set' indicator 
  MVREXPORT bool isValidTimestamp() const;
  
  /// Returns whether the two map IDs are equivalent
  /**
   * Note that if either map ID specifies a NULL timestamp, then the timestamp
   * will not be used for comparison purposes.
  **/
	MVREXPORT friend bool operator==(const MvrMapId & id1, const MvrMapId & id2);
	
  /// Returns whether the two map IDs are not equal
  /**
   * Note that if either map ID specifies a NULL timestamp, then the timestamp
   * will not be used for comparison purposes.
  **/
	MVREXPORT friend bool operator!=(const MvrMapId & id1, const MvrMapId & id2);

  /// Writes the map ID to the output log file, with the specified prefix /header.
  MVREXPORT void log(const char *prefix) const;

protected:
  /// Name of the source robot or server from which the map file originated
	std::string mySourceName;
	/// Name of the map file
	std::string myFileName;
	/// Buffer that contains the checksum of the map file
  unsigned char *myChecksum;
  /// Length of the buffer that contains the checksum of the map file
  size_t myChecksumLength;
 
  /// Buffer that contains the checksum in human readable format
  mutable char *myDisplayChecksum;
  /// Length of the displayable checksum buffer
  mutable size_t myDisplayChecksumLength;
  
  /// Number of bytes in the map file
  long int mySize;
  /// Last modified time of the map file
  time_t myTimestamp;

}; // end class MvrMapId


#ifndef SWIG


// ============================================================================
// MvrMapFileLine, etc
// ============================================================================

/// Encapsulates the data regarding a text line in a map file.
/**
 * MvrMapFileLine is the building block for map change comparisons.  It is 
 * simply the line number and associated text.
 * @internal
 * @swigomit
**/
class MvrMapFileLine {

public:

  /// Default constructor
  MvrMapFileLine() : myLineNum(0), myLineText() {}

  /// Constructor which sets the line number and text 
  MvrMapFileLine(int lineNum, const char *lineText) : myLineNum(lineNum), myLineText((lineText != NULL) ? lineText : "")
  {}

  /// Copy constructor
  MvrMapFileLine(const MvrMapFileLine &other) : myLineNum(other.myLineNum), myLineText(other.myLineText)
  {}

  /// Assignment operator
  MvrMapFileLine &operator=(const MvrMapFileLine &other) 
  {
    if (this != &other) {
      myLineNum  = other.myLineNum;
      myLineText = other.myLineText;
    }
    return *this;
  }

  /// Destructor
  ~MvrMapFileLine() {}

  /// Returns the line number of the map file line  
  int getLineNum() const { return myLineNum; }
  /// Returns the text of the map file line
  const char *getLineText() const { return myLineText.c_str(); }
	friend inline bool operator< ( const MvrMapFileLine & line1, const MvrMapFileLine & line2 );
  friend struct MvrMapFileLineCompare;

protected:
  int myLineNum;          ///< Line number
  std::string myLineText; ///< Line text

}; // end class MvrMapFileLine


/// Determines whether the first MvrMapFileLine is less than the second
/**
 * Line1 is less than Line2 if its line number is less than that of Line2.
 * If the two line nubmers are equal, then Line1 is less than Line2 if its
 * text is less.
**/
inline bool operator<(const MvrMapFileLine & line1, const MvrMapFileLine & line2 )
{
    bool b = (line1.myLineNum < line2.myLineNum);
    if (!b) {
      if (line1.myLineNum == line2.myLineNum) {
        b = (line1.myLineText < line2.myLineText);
      }
    }
    return b;
}

/// Comparator used to sort MvrMapFileLine objects.
struct MvrMapFileLineCompare : public std::binary_function<const MvrMapFileLine &, const MvrMapFileLine &, bool> 
{
  /// Returns true if line1 is less than line2; false, otherwise.
  bool operator()(const MvrMapFileLine &line1,
                  const MvrMapFileLine &line2) 
  { 
    bool b = (line1.myLineNum < line2.myLineNum);
    if (!b) {
      if (line1.myLineNum == line2.myLineNum) {
        b = (line1.myLineText < line2.myLineText);
      }
    }
    return b;
  }
}; // end struct MapFileLineCompare

// ============================================================================
// MvrMapFileLineGroup
// ============================================================================

/// A group of map file lines which have a logical parent / child relationship.
/**
 * Some of the text lines in a map file may have a parent / child relationship.
 * For example, the text line that starts a route is the parent of the following
 * lines that describe the contents of the route.  This class is used to 
 * express this relationship.  Note that there is currently no concept of 
 * a grandparent relationship in the map file lines.
**/
class MvrMapFileLineGroup {

public:

  /// Contructs a new group with the specified parent.
  MvrMapFileLineGroup(const MvrMapFileLine &parentLine) :
    myParentLine(parentLine),
    myChildLines()
  {}

  /// Destructor
  ~MvrMapFileLineGroup() {}

  /// Returns a pointer to the parent map file line.
  MvrMapFileLine *getParentLine() { return &myParentLine; }
  /// Returns a pointer to the list of child map file lines.
  std::vector<MvrMapFileLine> *getChildLines() { return &myChildLines; }

	friend inline bool operator<(const MvrMapFileLineGroup & line1, 
                               const MvrMapFileLineGroup & line2);
  friend struct MvrMapFileLineGroupCompare;

  /// Writes the group to the Mvria log.
  void log();

public: // users may access class members directly

  MvrMapFileLine myParentLine; ///< The map file line that is the parent of the group
  std::vector<MvrMapFileLine> myChildLines; ///< A list of child map file lines

}; // end class MvrFileLineGroup

/// Determines whether group1's parent text is less than group2's parent text.
/** @internal  
 *  @swigomit
 */
inline bool operator<(const MvrMapFileLineGroup & group1, 
                      const MvrMapFileLineGroup & group2)
{
  bool b = (strcmp(group1.myParentLine.getLineText(), 
                   group2.myParentLine.getLineText()) < 0);
  return b;
}


/// Comparator used to sort groups in order of ascending parent text.

struct MvrMapFileLineGroupCompare : public std::binary_function<const MvrMapFileLineGroup &, 
                                                                const MvrMapFileLineGroup &, 
                                                                bool> 
{
  bool operator()(const MvrMapFileLineGroup &group1,
                  const MvrMapFileLineGroup &group2) 
  { 
    bool b = (strcmp(group1.myParentLine.getLineText(), 
                     group2.myParentLine.getLineText()) < 0);
    return b;
  }
}; // end struct MvrMapFileLineGroupCompare


/// Comparator used to sort groups in order of ascending parent line number.

struct MvrMapFileLineGroupLineNumCompare : 
                  public std::binary_function<const MvrMapFileLineGroup &, 
                                              const MvrMapFileLineGroup &, 
                                              bool> {
  bool operator()(const MvrMapFileLineGroup &group1,
                  const MvrMapFileLineGroup &group2) 
  { 
    bool b = (group1.myParentLine.getLineNum() < 
              group2.myParentLine.getLineNum());
    return b;
  }
}; // end struct MvrMapFileLineGroupLineNumCompare


// ============================================================================
// MvrMapFileLineSet
// ============================================================================

/// A set of map file line groups.
/**
 * MvrMapFileLineSet is a container of MvrMapFileLineGroup objects -- i.e. a 
 * set of parent/child text lines in an Mvria map.  The class has been 
 * defined to enable comparisons of map file versions.  Each section of 
 * an Mvria map is written to an MvrMapFileLineSet and then the standard
 * algorithm set_difference can be used to determine changes within a 
 * section.  The static method MvrMapFileLineSet::calculateChanges()
 * performs this comparison.
**/
class MvrMapFileLineSet : public std::vector<MvrMapFileLineGroup>
{
public:

  // ---------------------------------------------------------------------------
  /// Determines the changes that have been made to a set of MvrMapFileLines
  /**
   * @param origLines the MvrMapFileLineSet that contains the original map file
   * lines
   * @param newLines the MvrMapFileLineSet that contains the new map file lines
   * @param deletedLinesOut a pointer to the MvrMapFileLineSet that is populated
   * with lines that have been deleted, i.e. that are contained in origLines 
   * but not in newLines
   * @param addedLinesOut a pointer to the MvrMapFileLineSet that is populated
   * with lines that have been added, i.e. that are contained in newLines but
   * not in origLines
   * @param isCheckChildren a bool set to true if child lines should also be 
   * checked; if false, then only the parent lines are checked
   * @return bool true if the changes were successfully determined; false if
   * an error occurred
  **/
  MVREXPORT static bool calculateChanges(MvrMapFileLineSet &origLines,
                                         MvrMapFileLineSet &newLines,
                                         MvrMapFileLineSet *deletedLinesOut,
                                         MvrMapFileLineSet *addedLinesOut,
                                         bool isCheckChildren = true);

  // ---------------------------------------------------------------------------

  /// Constructor 
  MvrMapFileLineSet() {}

  /// Copy constructor
  MvrMapFileLineSet(const MvrMapFileLineSet &other) : std::vector<MvrMapFileLineGroup>(other)
  {}

  /// Assignment operator
  MvrMapFileLineSet &operator=(const MvrMapFileLineSet &other) 
  {
    if (this != &other) {
      std::vector<MvrMapFileLineGroup>::operator =(other);
    }
    return *this;
  }

  /// Destructor
  ~MvrMapFileLineSet() {}

  /// Searches the set for the given parent line.
  MVREXPORT iterator find(const MvrMapFileLine &groupParent);

  /// Writes the set to the Mvria output log.
  MVREXPORT void log(const char *prefix);

}; // end class MvrMapFileLineSet


// ============================================================================
// MvrMapChangeDetails
// ============================================================================

/// Helper class used to track changes to an Mvria map.
/**
 * MvrMapChangeDetails is a simple helper class that is used to track changes
 * to an Mvria map.  These changes are determined based on set comparisons 
 * (and thus everything in the map must be ordered in a repeatable manner).
 *
 * The class itself provides very little functionality.  It is basically 
 * a repository of change information that may be accessed directly by the 
 * application. The methods return pointers to the internal data members 
 * which may be directly manipulated.  There is no error checking, 
 * thread-safety, etc.  The class's use and scope is expected to be very 
 * limited (to the Mvria map and related classes).
 * @swigomit
 * @internal
**/
class MvrMapChangeDetails
{
public:

	enum MapLineChangeType {
		DELETIONS,  ///< Lines that have been deleted from the Mvria map
		ADDITIONS,  ///< Lines that have been added to the Mvria map 
    LAST_CHANGE_TYPE = ADDITIONS  ///< Last value in the enumeration
	};

  enum {
    CHANGE_TYPE_COUNT = LAST_CHANGE_TYPE + 1 ///< Number of map change types
  };

  /// Constructor
  MVREXPORT MvrMapChangeDetails();

  /// Copy constructor
  MVREXPORT MvrMapChangeDetails(const MvrMapChangeDetails &other);

  /// Assignment operator
  MVREXPORT MvrMapChangeDetails &operator=(const MvrMapChangeDetails &other);

  /// Destructor
  MVREXPORT ~MvrMapChangeDetails();

  // ---------------------------------------------------------------------------
  // Map ID Methods
  // ---------------------------------------------------------------------------

  /// Returns the original pre-change map ID
  /**
   * @param mapIdOut a pointer to the MvrMapId to be returned 
   * @return bool true if the map ID was successfully retrieved; false if an 
   * error occurred
  **/
  MVREXPORT bool getOrigMapId(MvrMapId *mapIdOut);

  /// Sets the original pre-change map ID
  /**
   * @param mapId the MvrMapId of the map before it was changed
  **/
  MVREXPORT void setOrigMapId(const MvrMapId &mapId);

  /// Returns the new post-change map ID
  /**
   * @param mapIdOut a pointer to the MvrMapId to be returned 
   * @return bool true if the map ID was successfully retrieved; false if an 
   * error occurred
  **/
  MVREXPORT bool getNewMapId(MvrMapId *mapIdOut);

  /// Sets the new post-change map ID
  /**
   * @param mapId the MvrMapId of the map after it was changed
  **/
  MVREXPORT void setNewMapId(const MvrMapId &mapId);

  // ---------------------------------------------------------------------------
  // Change Info
  // ---------------------------------------------------------------------------

  /// Returns a pointer to the list of scan types that have been modified
  /**
   * This list includes the scan types in the map before and after it was changed.
   * Scan types usually won't be added or removed, but it may occur when one 
   * map is inserted into another.
   * A pointer to the actual internal attribute is returned.
   * @return a pointer to the list of scan type identifier strings
  **/
  MVREXPORT std::list<std::string> *getScanTypes();

  /// Returns a pointer to the data points that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * points are to be returned
   * @param scanType the const char * identifier of the scan for which the points
   * are to be returned; must be non-NULL
   * @return a non-NULL pointer to the vector of MvrPose's that have been changed
  **/
  MVREXPORT std::vector<MvrPose> *getChangedPoints(MapLineChangeType change,
                                                 const char *scanType);

  /// Returns a pointer to the data line segments that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * line segments are to be returned
   * @param scanType the const char * identifier of the scan for which the line
   * segments are to be returned; must be non-NULL
   * @return a non-NULL pointer to the vector of MvrLineSegment's that have been changed
  **/
  MVREXPORT std::vector<MvrLineSegment> *getChangedLineSegments
            (MapLineChangeType change, const char *scanType);

  /// Returns a pointer to the header lines that have been changed for the specified scan type
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * scan header lines are to be returned
   * @param scanType the const char * identifier of the scan for which the changes
   * are to be returned; must be non-NULL
   * @return a non-NULL pointer to the MvrMapFileLineSet that describes the changes
  **/
  MVREXPORT MvrMapFileLineSet *getChangedSummaryLines(MapLineChangeType change, const char *scanType);

  /// Returns a pointer to the map supplement lines that have been changed 
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * supplement lines are to be returned
   * @return a non-NULL pointer to the MvrMapFileLineSet that describes the changes
  **/
  MVREXPORT MvrMapFileLineSet *getChangedSupplementLines(MapLineChangeType change);

  /// Returns a pointer to the map object (i.e. Cairn) lines that have been changed 
  /**
   * @param change the MapLineChangeType that indicates whether added or removed
   * map object lines are to be returned
   * @return a non-NULL pointer to the MvrMapFileLineSet that describes the changes
  **/
  MVREXPORT MvrMapFileLineSet *getChangedObjectLines(MapLineChangeType change);

  /// Returns a pointer to the specified info lines that have been changed
  /**
   * @param infoName the int identifier of the info type to be returned
   * @param change the MapLineChangeType that indicates whether added or removed
   * map info lines are to be returned
   * @return a non-NULL pointer to the MvrMapFileLineSet that describes the changes
  **/
  MVREXPORT MvrMapFileLineSet *getChangedInfoLines(const char *infoName, MapLineChangeType change);


  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Returns a list of the info types that have been changed
  /**
   * This method searches the internal changed info lines and returns the info
   * types that have non-empty change information.
   * @return list of the string info names that have non-empty changes
  **/
  MVREXPORT std::list<std::string> findChangedInfoNames() const;

  /// Determines whether the given argument for the specified info type is a "child".
  /**
   * "Child" arguments have a non-NULL parent.  For example, an argument that
   * represents a robot task may be a child of one that defines a macro.  When
   * a child argument is changed, then the parent and all of its children 
   * must be included in the change details.  Note that currently only one
   * level of parent-ness is supported (i.e. there are no grandparents).
  **/
  MVREXPORT bool isChildArg(const char *infoName, MvrArgumentBuilder *arg) const;

  /// Determines whether the given arg 0 for the info type is a "child".
  MVREXPORT bool isChildArg(const char *infoName, const char *arg0Text) const;

  /// Creates a map of args that are considered to be a "child" of another arg.
  MVREXPORT void createChildArgMap();

  /// Writes the change details to the Mvria log.
  MVREXPORT void log();

  /// Locks the change details for multithreaded access.
  MVREXPORT void lock();
  //// Unlocks the change details for multithreaded access.
  MVREXPORT void unlock();

protected:

  /// Summary of changes for a specific map scan type.
  struct MvrMapScanChangeDetails {

    std::vector<MvrPose> myChangedPoints[CHANGE_TYPE_COUNT];
    std::vector<MvrLineSegment> myChangedLineSegments[CHANGE_TYPE_COUNT];

    MvrMapFileLineSet myChangedSummaryLines[CHANGE_TYPE_COUNT];

    MvrMapScanChangeDetails();
    ~MvrMapScanChangeDetails();

  }; // end struct MvrMapScanChangeDetails

  MvrMapScanChangeDetails *getScanChangeDetails(const char *scanType);

protected:

  /// Mutex to protect multithreaded access.
  MvrMutex myMutex;

  /// Identifier of the map before the changes were made.
  MvrMapId myOrigMapId;
  /// Identifier of the map after the changes were made.
  MvrMapId myNewMapId;

  /// Map of info type identifiers to argument types, and whether each argument type is a child.
  std::map<std::string, std::map<std::string, bool> > myInfoNameToMapOfChildArgsMap;

  /// List of scan types included in the change details.
  std::list<std::string> myScanTypeList;
  /// Map of scan types to the changes for the scan.
  std::map<std::string, MvrMapScanChangeDetails*> myScanTypeToChangesMap;
  /// Value returned when no scan data was changed.
  MvrMapScanChangeDetails myNullScanTypeChanges;

  /// Change details for the map's supplemental data.
  MvrMapFileLineSet myChangedSupplementLines[CHANGE_TYPE_COUNT];
  /// Change details for the map's object list.
  MvrMapFileLineSet myChangedObjectLines[CHANGE_TYPE_COUNT];
  /// Change details for the map's info data.
  std::map<std::string, MvrMapFileLineSet> myInfoToChangeMaps[CHANGE_TYPE_COUNT];

}; // end class MvrMapChangeDetails

// ============================================================================
// MvrMapFileLineSetWriter
// ============================================================================

/// Functor that populates a specified MvrMapFileLineSet.
/**
 * MvrMapFileLineSetWriter is used to create an MvrMapFileLineSet, using the 
 * normal Mvria map writeToFunctor mechanism.
**/
class MvrMapFileLineSetWriter : public MvrFunctor1<const char *>
{
public:

  /// Constructs a new writer for the given map file line set.
  MvrMapFileLineSetWriter(MvrMapFileLineSet *multiSet) :
    myLineNum(0),
    myChildLineNum(0),
	  myMultiSet(multiSet),
    myIsAddingChildren(false)
  {}
  
  /// Destructor
  ~MvrMapFileLineSetWriter()
  {}
  
  /// Returns whether children are currently being added to the map file line set.
  bool isAddingChildren() const
  {
    return myIsAddingChildren;
  }

  /// Sets whether children are currently being added to the map file line set.
  /**
   * When a child line is detected by the map file parser, setAddingChildren(true) 
   * must be called.  The new children will be added to the most recently 
   * created MvrMapFileLineSet.  Likewise, when the child list is complete, 
   * setAddingChildren(false) must be called.
  **/
  void setAddingChildren(bool isAddingChildren) {
    if (myIsAddingChildren == isAddingChildren) {
      return;
    }
    myIsAddingChildren = isAddingChildren; 
    if (myIsAddingChildren) {
      myChildLineNum = 0;
    }
  } // end method setAddingChildren


  /// Invokes the functor; this method shouldn't be called.
  virtual void invoke(void) {};

  /// Invokes the functor
  /**
     @param p1 the char * map file line to be written
  */
  virtual void invoke(const char *p1)
  {
    myLineNum++;

    if ((myMultiSet != NULL) && (p1 != NULL)) {
      if (!myIsAddingChildren) {
        // Being verbose on the parameters here in order to make it more clear.
        // Create a new map file line for the parent, create a new group for 
        // that parent, and add it to the set.
	      myMultiSet->push_back(MvrMapFileLineGroup(MvrMapFileLine(myLineNum, p1)));
      }
      else if (!myMultiSet->empty()) {
        myChildLineNum++;
        MvrMapFileLineGroup &curParent = myMultiSet->back();
        curParent.getChildLines()->push_back(MvrMapFileLine(myChildLineNum, p1));
      }
	  }
  } // end method invoke
 
private:

  /// Disabled copy constructor
  MvrMapFileLineSetWriter(const MvrMapFileLineSetWriter &other);
  /// Disabled assignment operator
  MvrMapFileLineSetWriter &operator=(const MvrMapFileLineSetWriter &other);

protected:

  /// Line number currently being written
  int myLineNum;
  /// "Local" line number of the current child in relation to its parent
  int myChildLineNum;

  /// Map file line set that is being populated
  MvrMapFileLineSet *myMultiSet; 
  /// Whether a child is currently being added
  bool myIsAddingChildren;

}; // end class MvrMapFileLineSetWriter


// ============================================================================
// MvrMapChangedHelper
// ============================================================================

/// Helper class that stores and invokes the map changed callbacks.
/** @swigomit
 * @internal 
 */
class MvrMapChangedHelper
 {
 public:
 
   ///  Constructor 
   MVREXPORT MvrMapChangedHelper();
   /// Destructor
   MVREXPORT virtual ~MvrMapChangedHelper();
   
   /// Function that invokes the map changed callbacks
   MVREXPORT virtual void invokeMapChangedCallbacks(void);
 

   /// Adds a callback to be invoked when the map is changed
   MVREXPORT virtual void addMapChangedCB(MvrFunctor *functor, 
					 int position = 50);
   /// Removes a callback invoked when the map is changed
   MVREXPORT virtual void remMapChangedCB(MvrFunctor *functor);
 

   /// Adds a callback called before the map changed callbacks are called
   /**
    * Note that these callbacks are simply invoked before the "normal" map changed
    * callbacks.  They are not 
   **/
   MVREXPORT virtual void addPreMapChangedCB(MvrFunctor *functor,
                                            int position = 50);
   /// Removes a callback called before the map changed callbacks are called
   MVREXPORT virtual void remPreMapChangedCB(MvrFunctor *functor);
 
   /// Sets the level we log our map changed callback at
   MVREXPORT virtual void setMapChangedLogLevel(MvrLog::LogLevel level);
   /// Gets the level we log our map changed callback at
   MVREXPORT virtual MvrLog::LogLevel getMapChangedLogLevel(void);
   
 private:
 
   /// Disabled copy constructor
   MVREXPORT MvrMapChangedHelper(const MvrMapChangedHelper &other);
   ///  Disabled assignment operaotr
   MVREXPORT MvrMapChangedHelper &operator=(const MvrMapChangedHelper &other);
 
 protected:
 
   MvrLog::LogLevel myMapChangedLogLevel;
 
   MvrCallbackList myMapChangedCBList;
   MvrCallbackList myPreMapChangedCBList;
 
 }; // end class MvrMapChangedHelper
 
#endif // ifndef SWIG

#endif // ARMAPUTILS_H

