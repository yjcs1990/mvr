/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMapComponents.h
 > Description  : Contains the set of interfaces that define the Mvria maps.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
/* The MvrMap class (defined in MvrMap.h) is composed of several smaller 
 * classes that are defined in this header file.  These include:
 * 
 *  - MvrMapScan: An implementation of the MvrMapScanInterface.  This 
 *    contains all of the data related to the sensed obstacles (i.e.
 *    data points and lines).  An instance of this class is created 
 *    for each scan type that is defined in the map.
 *
 *  - MvrMapObjects: An implementation of the MvrMapObjectsInterface.
 *    This stores all of the map objects for the Mvria map.
 *
 *  - MvrMapInfo: An implementation of the MvrMapInfoInterface.  This
 *    contains all of the info (MvrArgumentBuilder) tags defined for 
 *    the map, including MapInfo, TaskInfo, and RouteInfo.
 *
 *  - MvrMapSupplement: An implementation of the MvrMapSupplementInterface.
 *    This is a repository for all of the extra data that does not fit
 *    into any of the above categories.
 * 
 *  - MvrMapSimple:  The RealSubject of the MvrMap Proxy.  This implements
 *    the MvrMapInterface and is an aggregate of all of the above map
 *    components.
 * 
 * The following "diagram" illustrates the basic hierarchy:
 *
 * <pre>  
 *
 *          ________MvrMapSupplementInterface________
 *         ^                                         ^
 *         |   ________MvrMapInfoInterface________   |
 *         |   ^                                 ^   |
 *         |   |   __MvrMapObjectsInterface___   |   |
 *         |   |   ^                         ^   |   |
 *         |   |   |   MvrMapScanInterface   |   |   |
 *         |   |   |   ^                 ^   |   |   |
 *         |   |   |   |                 |   |   |   |
 *        MvrMapInterface                |   |   |   |  
 *        ^ (extends)                    |   |   |   | (extends)
 *        |                              |   |   |   |
 *        |                              |   |   |   |
 *     MvrMapSimple +----------> MvrMapScan  |   |   |
 *                 |   (contains)            |   |   |
 *                 +-----------> MvrMapObjects   |   |
 *                 |                             |   |
 *                 +------------------> MvrMapInfo   |
 *                 |                                 |
 *                 +----------------> MvrMapSupplement
 *
 * </pre>

 * @see MvrMapInterface
 * @see MvrMap
**/
#ifndef MVRMAPCOMPONENTS_H
#define MVRMAPCOMPONENTS_H

#include "MvrMapInterface.h"

class MvrMapChangeDetails;
class MvrMapFileLineSet;
class MvrFileParser;
class MvrMD5Calculator;


// ============================================================================
// MvrMapScan 
// ============================================================================

/// The map data related to the sensable obstacles in the environment.
/**
 * MvrMapScan encapsulates the data for a particular sensor that is generated 
 * during the scanning process (i.e. during the creation of a .2d file).  
 * The class's primary attributes are the points and line segments that 
 * were detected during the scan.  It contains methods to get and set these 
 * coordinates, and to read and write the data from and to a file.
 * <p>
 * The <code>scanType</code> parameters identify the sensor used for scanning. 
 * The parameter is used in the constructor, but it is generally disregarded
 * in the other methods.  (The method signatures are defined in 
 * MvrMapScanInterface, which is also implemented by MvrMap.  The map provides
 * access to the scan data for all of the sensors -- and therefore uses the 
 * <code>scanType</code> parameters.  This interface was chosen in order
 * to maintain backwards compatibility with the original map.)
 * <p>
 * If the scanType is specified, then it is used as a prefix to the DATA and
 * LINES tags that are contained in the map file.
**/
class MvrMapScan : public MvrMapScanInterface 
{
public:

  /// Constructor
  /**
   * Creates a new map scan object for the specified scan type.
   * @param scanType the const char * identifier of the scan; must be 
   * non-NULL and must not contain whitespaces
  **/
  MVREXPORT MvrMapScan(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  /// Copy constructor
  MVREXPORT MvrMapScan(const MvrMapScan &other);

  /// Assignment operator
  MVREXPORT MvrMapScan &operator=(const MvrMapScan &other);

  /// Destructor
  MVREXPORT virtual ~MvrMapScan();


  // --------------------------------------------------------------------------
  // MvrMapScanInterface Methods
  // --------------------------------------------------------------------------

  MVREXPORT virtual const char *getDisplayString
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual std::vector<MvrPose> *getPoints
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual std::vector<MvrLineSegment> *getLines
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual MvrPose getMinPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual MvrPose getMaxPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual int getNumPoints(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual bool isSortedPoints(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const;

  MVREXPORT virtual void setPoints(const std::vector<MvrPose> *points,
                                   const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                   bool isSortedPoints = false,
                                   MvrMapChangeDetails *changeDetails = NULL);


  MVREXPORT virtual MvrPose getLineMinPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual MvrPose getLineMaxPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual int getNumLines(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual bool isSortedLines(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const;

  MVREXPORT virtual void setLines(const std::vector<MvrLineSegment> *lines,
                                  const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedLines = false,
                                  MvrMapChangeDetails *changeDetails = NULL);

  MVREXPORT virtual int getResolution(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual void setResolution(int resolution,
                                       const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                       MvrMapChangeDetails *changeDetails = NULL);


  MVREXPORT virtual void writeScanToFunctor
            (MvrFunctor1<const char *> *functor, 
 			       const char *endOfLineChars, const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual void writePointsToFunctor
 		        (MvrFunctor2<int, std::vector<MvrPose> *> *functor,
             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
             MvrFunctor1<const char *> *keywordFunctor = NULL);

  MVREXPORT virtual void writeLinesToFunctor
 	          (MvrFunctor2<int, std::vector<MvrLineSegment> *> *functor,
             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
             MvrFunctor1<const char *> *keywordFunctor = NULL);

  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);

  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);

  MVREXPORT virtual bool readDataPoint( char *line);

  MVREXPORT virtual bool readLineSegment( char *line);

  MVREXPORT virtual void loadDataPoint(double x, double y);

  MVREXPORT virtual void loadLineSegment(double x1, double y1, double x2, double y2);
  
  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------
 
  /// Resets the scan data, clearing all points and line segments
  MVREXPORT virtual void clear();
  
  /// Combines the given other scan with this one.
  /**
   * @param other the MvrMapScan * to be united with this one
   * @param isIncludeDataPointsAndLines a bool set to true if the other scan's
   * data points and lines should be copied to this scan; if false, then only
   * the summary (bounding box, counts, etc) information is copied.
  **/
  MVREXPORT virtual bool unite(MvrMapScan *other,
                               bool isIncludeDataPointsAndLines = false);
  
  /// Returns the time at which the scan data was last changed.
  MVREXPORT virtual MvrTime getTimeChanged() const;

  // TODO: Which of these need to be in the MvrMapScanInterface?

  /// Returns the unique string identifier of the associated scan type.
  MVREXPORT virtual const char *getScanType() const;

  /// Returns the keyword that designates the scan's data points in the map file.
  MVREXPORT virtual const char *getPointsKeyword() const;
  /// Returns the keyword that designates the scan's data lines in the map file.
  MVREXPORT virtual const char *getLinesKeyword() const;

  /// Writes the scan's data points (and introductory keyword) to the given functor.
  MVREXPORT virtual void writePointsToFunctor
             (MvrFunctor1<const char *> *functor, 
 			        const char *endOfLineChars,
               const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  /// Writes the scan's data lines (and introductory keyword) to the given functor.
  MVREXPORT virtual void writeLinesToFunctor
             (MvrFunctor1<const char *> *functor, 
 			        const char *endOfLineChars,
              const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  
  /// Adds the handlers for the data points and lines keywords to the given file parser.
  /**
   * These handlers are "extra" because they are added after all of the summary
   * keyword have been parsed.
  **/
  MVREXPORT virtual bool addExtraToFileParser(MvrFileParser *fileParser,
                                              bool isAddLineHandler);

  /// Removes the handlers for the data points and lines keywords from the given file parser.
  MVREXPORT virtual bool remExtraFromFileParser(MvrFileParser *fileParser);

protected:

  /// Writes the list of data lines to the given functor.
  /**
   * @param functor the MvrFunctor1<const char *> * to which to write the 
   * data lines
   * @param lines the vector of MvrLineSegments to be written to the functor
   * @param endOfLineChars an optional string to be appended to the end of 
   * each text line written to the functor
   * @param scanType the unique string identifier of the scan type associated
   * with the data lines
  **/
  MVREXPORT virtual void writeLinesToFunctor(MvrFunctor1<const char *> *functor, 
                                             const std::vector<MvrLineSegment> &lines,
                                             const char *endOfLineChars,
                                             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);


  // Function to read the minimum pos
  bool handleMinPos(MvrArgumentBuilder *arg);
  // Function to read the maximum pos
  bool handleMaxPos(MvrArgumentBuilder *arg);
  // Function to read the number of points
  bool handleNumPoints(MvrArgumentBuilder *arg);
  // Function to read whether the points are sorted
  bool handleIsSortedPoints(MvrArgumentBuilder *arg);

  // Function to read the line minimum pos
  bool handleLineMinPos(MvrArgumentBuilder *arg);
  // Function to read the line maximum pos
  bool handleLineMaxPos(MvrArgumentBuilder *arg);
  // Function to read the number of lines
  bool handleNumLines(MvrArgumentBuilder *arg);
  // Function to read whether the lines are sorted
  bool handleIsSortedLines(MvrArgumentBuilder *arg);

  // Function to handle the resolution
  bool handleResolution(MvrArgumentBuilder *arg);
  /// Callback to handle the Display string.
  bool handleDisplayString(MvrArgumentBuilder *arg);

  // Function to snag the map points (mainly for the getMap over the network)
  bool handlePoint(MvrArgumentBuilder *arg);
  // Function to snag the line segments (mainly for the getMap over the network)
  bool handleLine(MvrArgumentBuilder *arg);
  
  /// Adds the specified argument handler to the given file parser.
  bool addHandlerToFileParser(MvrFileParser *fileParser,
                              const char *keyword,
                              MvrRetFunctor1<bool, MvrArgumentBuilder *> *handler);

  /// Returns the keyword prefix for this scan type.
  const char *getKeywordPrefix() const;

  /// Parses a pose from the given arguments.
  bool parsePose(MvrArgumentBuilder *arg,
                 const char *keyword,
                 MvrPose *poseOut);

  /// Parses an integer from the given text line.
  bool parseNumber(char *line, 
                   size_t lineLen, 
                   size_t *charCountOut,
                   int *numOut) const;

  /// Parses whitespace from the given text line.
  bool parseWhitespace(char *line,
                       size_t lineLen,
                       size_t *charCountOut) const;

private:
  /// Constant appended to the end of each scan data text line.
  static const char *EOL_CHARS;

protected:

  /// The unique string identifier of this scan type.
  std::string myScanType;
  /// Whether this is a special summary of the other scans.
  bool myIsSummaryScan;

  /// The prefix prepended to the output log file messages.
  std::string myLogPrefix;
  /// The prefix prepended to the map file keywords (e.g. DATA and LINES)
  std::string myKeywordPrefix;
  /// The keyword that designates this scan's data points in the map file.
  std::string myPointsKeyword;
  /// The keyword that designates this scan's data lines in the map file.
  std::string myLinesKeyword;

  /// Time that this scan data was last modified.
  MvrTime myTimeChanged;

  /// Displayable text for this scan type.
  std::string myDisplayString;

  /// Number of data points in the scan.
  int myNumPoints;
  /// Number of data lines in the scan.
  int myNumLines;
  /// Resolution of the data points (in mm).
  int myResolution;
  /// Maximum x/y values of all of the data points in the scan.
  MvrPose myMax;
  /// Minimum x/y values of all of the data points in the scan.
  MvrPose myMin;
  /// Maximum x/y values of all of the data lines in the scan.
  MvrPose myLineMax;
  /// Minimum x/y values of all of the data lines in the scan.
  MvrPose myLineMin;

  /// Whether the data points in myPoints have been sorted in ascending order.
  bool myIsSortedPoints;
  /// Whether the data lines in myLines have been sorted in ascending order.
  bool myIsSortedLines;

  /// List of data points contained in this scan data.
  std::vector<MvrPose> myPoints;
  /// List of data lines contained in this scan data.
  std::vector<MvrLineSegment> myLines;

  /// Callback to parse the minimum poise from the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myMinPosCB;
  /// Callback to parse the maximum pose from the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myMaxPosCB;
  /// Callback to parse whether the points in the map file have been sorted.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myIsSortedPointsCB;
  /// Callback to parse the number of data points in the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myNumPointsCB;

  /// Callback to parse the minimum line pose from the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myLineMinPosCB;
  /// Callback to parse the maximum line pose from the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myLineMaxPosCB;
  /// Callback to parse whether the lines in the map file have been sorted.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myIsSortedLinesCB;
  /// Callback to parse the number of data lines in the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myNumLinesCB;

  /// Callback to parse the resolution in the map file.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myResolutionCB;
  /// Callback to parse the displayable text for this scan type.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myDisplayStringCB;

  /// Callback to parse a data point.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myPointCB;
  /// Callback to parse a data line.
  MvrRetFunctor1C<bool, MvrMapScan, MvrArgumentBuilder *> myLineCB;

}; // end class MvrMapScan

// ============================================================================
// MvrMapObjects
// ============================================================================

/// The collection of map objects that are contained in an Mvria map.
/**
 * MvrMapObjects contains a list of objects defined in an Mvria map.  There are
 * two basic classes of objects:  user-defined objects such as goals and 
 * forbidden areas; and, special data objects that are usually automatically
 * generated during the scanning process.
**/
class MvrMapObjects : public MvrMapObjectsInterface				
{

public :

  /// Default keyword that prefixes each map object line in the map file
  MVREXPORT static const char *DEFAULT_KEYWORD;

  /// Constructor
  /**
   * @param keyword the char * keyword that prefixes each map object line in
   * the map file
  **/
  MVREXPORT MvrMapObjects(const char *keyword = "Cairn:");

  /// Copy constructor
  MVREXPORT MvrMapObjects(const MvrMapObjects &other);

  /// Assignment operator
  MVREXPORT MvrMapObjects &operator=(const MvrMapObjects &other);

  /// Destructor
  MVREXPORT virtual ~MvrMapObjects();


  // ---------------------------------------------------------------------------
  // MvrMapObjectsInterface Methods
  // ---------------------------------------------------------------------------

  MVREXPORT virtual MvrMapObject *findFirstMapObject(const char *name, 
                                                     const char *type,
                                                     bool isIncludeWithHeading = false);

  MVREXPORT virtual MvrMapObject *findMapObject(const char *name, 
 				                                        const char *type = NULL, 
                                                bool isIncludeWithHeading = false);

  MVREXPORT virtual std::list<MvrMapObject *> findMapObjectsOfType
            (const char *type, bool isIncludeWithHeading = false);

  MVREXPORT virtual std::list<MvrMapObject *> *getMapObjects(void);

  MVREXPORT virtual void setMapObjects(const std::list<MvrMapObject *> *mapObjects,
                                       bool isSortedObjects = false,
                                       MvrMapChangeDetails *changeDetails = NULL); 
   
  MVREXPORT void writeObjectListToFunctor(MvrFunctor1<const char *> *functor, 
		                                      const char *endOfLineChars);


  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Clears the map objects and deletes them.
  MVREXPORT virtual void clear();

  /// Adds the keyword and handler for the map objects to the given file parser.
  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);

  /// Removes the keyword and handler for the map objects from the given file parser.
  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);

  /// Returns the time at which the map objects were last changed.
  MVREXPORT virtual MvrTime getTimeChanged() const;
 
protected:

  // Function to handle the cairns
  bool handleMapObject(MvrArgumentBuilder *arg);

  /// Sorts the given list of map objects in order of increasing object pose.
  void sortMapObjects(std::list<MvrMapObject *> *mapObjects);

  /// Writes the map objects to the given MvrMapFileLineSet.
  void createMultiSet(MvrMapFileLineSet *multiSet);

  /// Writes the given MvrMapFileLineSet to the output log with the specified prefix.
  void logMultiSet(const char *prefix,
                   MvrMapFileLineSet *multiSet);

protected:

  /// Time at which the map objects were last changed.
  MvrTime myTimeChanged;
  /// Whether the myMapObjects list has been sorted in increasing (pose) order.
  bool myIsSortedObjects;

  /// Keyword that prefixes each map object in the map file.
  std::string myKeyword;

  /// List of map objects contained in the Mvria map.
  std::list<MvrMapObject *> myMapObjects;

  /// Callback to parse the map object from the map file.
  MvrRetFunctor1C<bool, MvrMapObjects, MvrArgumentBuilder *> myMapObjectCB;

}; // end class MvrMapObjects


// ============================================================================
// MvrMapInfo
// ============================================================================


/// A container for the various "info" tags in an Mvria map.
/**
 * MvrMapInfo is an implementation of MvrMapInfoInterface that provides access
 * to a collection of "info" arguments (such as MapInfo and RouteInfo).  An Mvria 
 * map may have one or more categories of info, each implemented by an ordered 
 * list of MvrArgumentBuilder's.
 *
 * Info types are currently identified by a unique integer.  The default types
 * are defined in MvrMapInfoInterface::InfoType, but applications may define 
 * additional types.  (See MvrMapInfo::MvrMapInfo(int*, char**, size_t))
**/
class MvrMapInfo : public MvrMapInfoInterface
{
public:

  /// Contructor
  /**
   * @param infoNameList an array of the char * keywords for each of the
   * standard MvrMapInfo::InfoType's; if NULL, then the default keywords are
   * used
   * @param infoNameCount the size_t length of the infoNameList array
   * @param keywordPrefix optional prefix to add to keywords.
  **/ 
  MVREXPORT MvrMapInfo(const char **infoNameList = NULL,
                     size_t infoNameCount = 0,
                     const char *keywordPrefix = NULL);

  /// Copy contructor
  MVREXPORT MvrMapInfo(const MvrMapInfo &other);
  /// Assignment operator
  MVREXPORT MvrMapInfo &operator=(const MvrMapInfo &other);
  /// Destructor
  MVREXPORT virtual ~MvrMapInfo();

  // ---------------------------------------------------------------------------
  // MvrMapInfoInterface Methods
  // ---------------------------------------------------------------------------

  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(const char *infoName);

  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(int infoType);
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getMapInfo(void);

  MVREXPORT virtual int getInfoCount() const;

  MVREXPORT virtual std::list<std::string> getInfoNames() const;

  MVREXPORT virtual bool setInfo(const char *infoName,
                                 const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails);


  MVREXPORT virtual bool setInfo(int infoType,
 						                     const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL); 
  MVREXPORT virtual bool setMapInfo(const std::list<MvrArgumentBuilder *> *mapInfo,
                                    MvrMapChangeDetails *changeDetails = NULL); 


  MVREXPORT virtual const char *getInfoName(int infoType);

  MVREXPORT virtual void writeInfoToFunctor(MvrFunctor1<const char *> *functor, 
 			                                      const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // Other Methods
  // ---------------------------------------------------------------------------

  /// Clears all info arguments and deletes them.
  MVREXPORT virtual void clear();
 
  /// Adds handlers for all of the info types to the given file parser.
  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);
  /// Removes handlers for all of the info types from the given file parser.
  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);

  /// Returns the time at which the info were last changed.
  MVREXPORT virtual MvrTime getTimeChanged() const;

protected:

  /// Processes the given argument for the specified info.
  bool handleInfo(MvrArgumentBuilder *arg);

  /// Give MvrMapSimple access to the createMultiSet() and  setChanged() methods
  friend class MvrMapSimple;
  
  /// Writes the specified info arguments to the given MvrMapFileLineSet.
  /**
   * @param infoName unique identifier for the info to be written
   * @param multiSet the MvrMapFileLineSet * to which to write the info; 
   * must be non-NULL
   * @param changeDetails the MvrMapChangeDetails * that specifies the 
   * parent/child relationship amongst info lines 
   * @see MvrMapChangeDetails::isChildArg
  **/
  void createMultiSet(const char *infoName, 
                      MvrMapFileLineSet *multiSet,
                      MvrMapChangeDetails *changeDetails);

  /// Basically updates the timeChanged to now.
  void setChanged();
  
  /// Populates this object with the default info names / keywords
  void setDefaultInfoNames();



protected:

  struct MvrMapInfoData {

    MvrMapInfo *myParent;
    int myType;
    std::string myKeyword;
    std::list<MvrArgumentBuilder *> myInfo;
    MvrRetFunctor1C<bool, MvrMapInfo, MvrArgumentBuilder *> *myInfoCB;

    MvrMapInfoData(MvrMapInfo *parent,
                   const char *name = NULL,
                   int type = -1);
    ~MvrMapInfoData();
    MvrMapInfoData(MvrMapInfo *parent,
                   const MvrMapInfoData &other);
    MvrMapInfoData &operator=(const MvrMapInfoData &other);

  }; // end struct MvrMapInfoData

  typedef std::map<std::string, MvrMapInfoData *, MvrStrCaseCmpOp> MvrInfoNameToDataMap;


  MVREXPORT MvrMapInfoData *findData(const char *infoName);

  MVREXPORT MvrMapInfoData *findDataByKeyword(const char *keyword);


   /// Time at which the info was last changed
  MvrTime myTimeChanged;

  // Sigh... In retrospect, this should have been structured differently
  // and we probably should've used a string for the info identifier...

  /// Number of info types contained in this collection
  int myNumInfos;
  std::string myPrefix;

  std::map<int, std::string> myInfoTypeToNameMap;
  MvrInfoNameToDataMap myInfoNameToDataMap;
  std::map<std::string, std::string, MvrStrCaseCmpOp> myKeywordToInfoNameMap;

}; // end class MvrMapInfo


// ============================================================================
// MvrMapSupplement
// ============================================================================

/// Supplemental data associated with an Mvria map.
/**
 * MvrMapSupplement is a repository for extra, miscellaneous data that is 
 * associated with an Mvria map but which does not fit neatly into any of the 
 * other components.  
**/
class MvrMapSupplement : public MvrMapSupplementInterface 
{
public:

  /// Constructor
  MVREXPORT MvrMapSupplement();

  /// Copy constructor
  MVREXPORT MvrMapSupplement(const MvrMapSupplement &other);

  /// Assignment operator
  MVREXPORT MvrMapSupplement &operator=(const MvrMapSupplement &other);

  /// Destructor
  MVREXPORT virtual ~MvrMapSupplement();


  // --------------------------------------------------------------------------
  // MvrMapSupplementInterface Methods
  // --------------------------------------------------------------------------

  MVREXPORT virtual bool hasOriginLatLongAlt();
  
  MVREXPORT virtual MvrPose getOriginLatLong();
  
  MVREXPORT virtual double getOriginAltitude();

  MVREXPORT virtual void setOriginLatLongAlt(bool hasOriginLatLong,
                                             const MvrPose &originLatLong,
                                             double altitude,
                                             MvrMapChangeDetails *changeDetails = NULL);
  
  MVREXPORT virtual void writeSupplementToFunctor
            (MvrFunctor1<const char *> *functor, const char *endOfLineChars);

  // --------------------------------------------------------------------------
  // Other Methods
  // --------------------------------------------------------------------------

  /// Resets the map supplement to its default values.
  MVREXPORT virtual void clear();

  /// Adds handlers for all of the supplement keywords to the given file parser.
  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);
  /// Removes handlers for all of the supplement keywords from the given file parser.
  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);
  
  /// Returns the time at which the supplement data were last changed.
  MVREXPORT virtual MvrTime getTimeChanged() const;

protected:

  // Function to get the origin lat long altitude
  bool handleOriginLatLongAlt(MvrArgumentBuilder *arg);
  
private:

  /// Constant appended to the end of each supplement text line.
  static const char *EOL_CHARS;

protected:

  /// Time at which the supplement was last changed
  MvrTime myTimeChanged;

  /// Whether the supplement data contains latitude/longitude information for the origin
  bool myHasOriginLatLongAlt;
  /// The latitude/longitude of the origin; only if myHasOriginLatLongAlt is true
  MvrPose myOriginLatLong;
  /// The altitude (in m) of the origin; only if myHasOriginLatLongAlt is true
  double myOriginAltitude;

  /// Callback that parses the origin latitude/longitude/altitude information
  MvrRetFunctor1C<bool, MvrMapSupplement, MvrArgumentBuilder *> myOriginLatLongAltCB;

}; // end class MvrMapSupplement


// =============================================================================
// MvrMapSimple
// =============================================================================
 
/// Comparator used to sort scan data types in a case-insensitive manner. 
struct MvrDataTagCaseCmpOp 
{
public:
  bool operator() (const std::string &s1, const std::string &s2) const
  {
    size_t s1Len = s1.length();
    size_t s2Len = s2.length();

    if (s1Len < s2Len) {
      return strncasecmp(s1.c_str(), s2.c_str(), s1Len) < 0;
    }
    else {
      return strncasecmp(s1.c_str(), s2.c_str(), s2Len) < 0;
    }
  }
}; // end struct MvrDataTagCaseCmpOp

/// Type definition for a map of scan types to scan data.
typedef std::map<std::string, MvrMapScan *, MvrStrCaseCmpOp> MvrTypeToScanMap;

/// Type definition for a map of data tags to scan types
typedef std::map<std::string, std::string, MvrDataTagCaseCmpOp> MvrDataTagToScanTypeMap;


/// Simple map that can be read from and written to a file
/**
 * MvrMapSimple is the real subject of the MvrMap proxy.  Functionally, it is identical
 * to the MvrMap, @b except that it is not well-suited for for loading from a file at
 * runtime and therefore doesn't provide any hooks into the Mvria config.  In general,
 * MvrMap should be used instead.  The exception to this rule may be in off-line 
 * authoring tools where error checking can be performed at a higher level.
**/
class MvrMapSimple : public MvrMapInterface
{
public:

  /// Constructor
  /**
   * @param baseDirectory the name of the directory in which to search for map
   * files that are not fully qualified
   * @param tempDirectory the name of the directory in which to write temporary
   * files when saving a map; if NULL, then the map file is written directly.  
   * Note that using a temp file reduces the risk that the map will be corrupted
   * if the application crashes.
   * @param overrideMutexName an optional name to be used for the map object's
   * mutex; useful for debugging when multiple maps are active
  **/
  MVREXPORT MvrMapSimple(const char *baseDirectory = "./",
                         const char *tempDirectory = NULL,
                         const char *overrideMutexName = NULL);

  /// Copy constructor
  MVREXPORT MvrMapSimple(const MvrMapSimple &other);
  /// Assignment operator
  MVREXPORT MvrMapSimple &operator=(const MvrMapSimple &other);
  /// Destructor
  MVREXPORT virtual ~MvrMapSimple(void);


  MVREXPORT virtual void clear();

  MVREXPORT virtual bool set(MvrMapInterface *other);

  MVREXPORT virtual MvrMapInterface *clone();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Scan Types Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   
  MVREXPORT virtual std::list<std::string> getScanTypes() const;

  MVREXPORT virtual bool setScanTypes(const std::list<std::string> &scanTypeList);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Locking / Semaphore Method
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  MVREXPORT virtual int lock();

  MVREXPORT virtual int tryLock();

  MVREXPORT virtual int unlock();


  // ---------------------------------------------------------------------------
  // MvrMapInfoInterface
  // ---------------------------------------------------------------------------

  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(const char *infoName);
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(int infoType);
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getMapInfo(void);
  
  MVREXPORT virtual int getInfoCount() const;
  
  MVREXPORT virtual std::list<std::string> getInfoNames() const;

  MVREXPORT virtual bool setInfo(const char *infoName,
 						                     const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL);

  MVREXPORT virtual bool setInfo(int infoType,
 						                     const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL);

  MVREXPORT virtual bool setMapInfo(const std::list<MvrArgumentBuilder *> *mapInfo,
                                    MvrMapChangeDetails *changeDetails = NULL);

  MVREXPORT virtual void writeInfoToFunctor
 				(MvrFunctor1<const char *> *functor, const char *endOfLineChars);

  MVREXPORT virtual const char *getInfoName(int infoType);

  // ---------------------------------------------------------------------------
  // MvrMapObjectsInterface
  // ---------------------------------------------------------------------------

  MVREXPORT virtual MvrMapObject *findFirstMapObject(const char *name, 
                                                     const char *type,
                                                     bool isIncludeWithHeading = false);

  MVREXPORT virtual MvrMapObject *findMapObject(const char *name, 
                                                const char *type = NULL,
                                                bool isIncludeWithHeading = false);

  MVREXPORT virtual std::list<MvrMapObject *> findMapObjectsOfType
                                                    (const char *type,
                                                     bool isIncludeWithHeading = false);

  MVREXPORT virtual std::list<MvrMapObject *> *getMapObjects(void);

  MVREXPORT virtual void setMapObjects(const std::list<MvrMapObject *> *mapObjects,
                                       bool isSortedObjects = false,
                                       MvrMapChangeDetails *changeDetails = NULL); 


   MVREXPORT virtual void writeObjectListToFunctor(MvrFunctor1<const char *> *functor, 
			                                             const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // MvrMapSupplementInterface
  // ---------------------------------------------------------------------------

  MVREXPORT virtual bool hasOriginLatLongAlt();

  MVREXPORT virtual MvrPose getOriginLatLong();

  MVREXPORT virtual double getOriginAltitude();

  MVREXPORT virtual void setOriginLatLongAlt
                                        (bool hasOriginLatLong,
                                         const MvrPose &originLatLong,
                                         double altitude,
                                         MvrMapChangeDetails *changeDetails = NULL);


  MVREXPORT virtual void writeSupplementToFunctor(MvrFunctor1<const char *> *functor, 
			                                            const char *endOfLineChars);

  // ---------------------------------------------------------------------------
  // MvrMapScanInterface
  // ---------------------------------------------------------------------------

  MVREXPORT virtual const char *getDisplayString
                                 (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual std::vector<MvrPose> *getPoints
                                 (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual MvrPose getMinPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual MvrPose getMaxPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual int getNumPoints(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual bool isSortedPoints(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const;

  MVREXPORT virtual void setPoints(const std::vector<MvrPose> *points,
                                  const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedPoints = false,
                                  MvrMapChangeDetails *changeDetails = NULL);


  MVREXPORT virtual std::vector<MvrLineSegment> *getLines
                                 (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual MvrPose getLineMinPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual MvrPose getLineMaxPose(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual int getNumLines(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);
  MVREXPORT virtual bool isSortedLines(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const;

  MVREXPORT virtual void setLines(const std::vector<MvrLineSegment> *lines,
                                 const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                 bool isSortedLines = false,
                                 MvrMapChangeDetails *changeDetails = NULL);


  MVREXPORT virtual int getResolution(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual void setResolution(int resolution,
                                      const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                      MvrMapChangeDetails *changeDetails = NULL);

  
  MVREXPORT virtual void writeScanToFunctor
                              (MvrFunctor1<const char *> *functor, 
			                         const char *endOfLineChars,
                               const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE);

  MVREXPORT virtual void writePointsToFunctor
                              (MvrFunctor2<int, std::vector<MvrPose> *> *functor,
                               const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                               MvrFunctor1<const char *> *keywordFunctor = NULL);

  MVREXPORT virtual void writeLinesToFunctor
 	                            (MvrFunctor2<int, std::vector<MvrLineSegment> *> *functor,
                               const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                               MvrFunctor1<const char *> *keywordFunctor = NULL);

  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);
  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);


  MVREXPORT virtual bool readDataPoint( char *line);
  MVREXPORT virtual bool readLineSegment( char *line);


  /** Public for MvrQClientMapProducer **/

  MVREXPORT virtual void loadDataPoint(double x, double y);
  MVREXPORT virtual void loadLineSegment(double x1, double y1, double x2, double y2);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Map Changed / Callback Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  MVREXPORT virtual void mapChanged(void);

  MVREXPORT virtual void addMapChangedCB(MvrFunctor *functor, int position = 50);

  MVREXPORT virtual void remMapChangedCB(MvrFunctor *functor);

  MVREXPORT virtual void addPreMapChangedCB(MvrFunctor *functor, int position = 50);

  MVREXPORT virtual void remPreMapChangedCB(MvrFunctor *functor);

  MVREXPORT virtual void setMapChangedLogLevel(MvrLog::LogLevel level);

  MVREXPORT virtual MvrLog::LogLevel getMapChangedLogLevel(void);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Persistence
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  

  MVREXPORT virtual void writeToFunctor(MvrFunctor1<const char *> *functor, 
 			                                 const char *endOfLineChars);

  MVREXPORT virtual void writeObjectsToFunctor(MvrFunctor1<const char *> *functor, 
 			                                         const char *endOfLineChars,
                                               bool isOverrideAsSingleScan = false,
                                               const char *maxCategory = NULL);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File I/O Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  MVREXPORT virtual void addPreWriteFileCB(MvrFunctor *functor,
                                           MvrListPos::Pos position = MvrListPos::LAST);
  MVREXPORT virtual void remPreWriteFileCB(MvrFunctor *functor);
 
  MVREXPORT virtual void addPostWriteFileCB(MvrFunctor *functor,
                                            MvrListPos::Pos position = MvrListPos::LAST);
  MVREXPORT virtual void remPostWriteFileCB(MvrFunctor *functor);

  MVREXPORT virtual bool readFile(const char *fileName, 
 			                            char *errorBuffer = NULL, 
                                  size_t errorBufferLen = 0,
                                  unsigned char *md5DigestBuffer = NULL,
                                  size_t md5DigestBufferLen = 0);

  MVREXPORT virtual bool writeFile(const char *fileName, 
                                   bool internalCall = false,
                                   unsigned char *md5DigestBuffer = NULL,
                                   size_t md5DigestBufferLen = 0,
                                   time_t fileTimestamp = -1);

#ifndef SWIG
  /// @swigomit
  MVREXPORT virtual struct stat getReadFileStat() const;
#endif

  MVREXPORT virtual bool getMapId(MvrMapId *mapIdOut,
                                  bool isInternalCall = false);

  MVREXPORT virtual bool calculateChecksum(unsigned char *md5DigestBuffer,
                                           size_t md5DigestBufferLen);
  
  MVREXPORT virtual const char *getBaseDirectory(void) const;

  MVREXPORT virtual void setBaseDirectory(const char *baseDirectory);

  MVREXPORT virtual const char *getTempDirectory(void) const;

  MVREXPORT virtual void setTempDirectory(const char *tempDirectory);

  MVREXPORT virtual std::string createRealFileName(const char *fileName);

  MVREXPORT virtual const char *getFileName(void) const;

  MVREXPORT virtual void setSourceFileName(const char *sourceName,
                                           const char *fileName,
                                           bool isInternalCall = false);

  MVREXPORT virtual bool refresh();


  virtual void setIgnoreEmptyFileName(bool ignore);
  virtual bool getIgnoreEmptyFileName(void);
  virtual void setIgnoreCase(bool ignoreCase = false);
  virtual bool getIgnoreCase(void);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Inactive Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  MVREXPORT virtual MvrMapInfoInterface *getInactiveInfo();

  MVREXPORT virtual MvrMapObjectsInterface *getInactiveObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Child Objects Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  MVREXPORT virtual MvrMapObjectsInterface *getChildObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  MVREXPORT virtual MvrArgumentBuilder *findMapObjectParams(const char *mapObjectName);

  MVREXPORT virtual bool setMapObjectParams(const char *mapObjectName,
                                           MvrArgumentBuilder *params, 
                                           MvrMapChangeDetails *changeDetails = NULL);



  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getRemainder();

  MVREXPORT virtual void setQuiet(bool isQuiet);
 	
  MVREXPORT bool parseLine(char *line);
  MVREXPORT void parsingComplete(void);

  MVREXPORT bool isLoadingDataStarted(); 
  MVREXPORT bool isLoadingLinesAndDataStarted(); 


  // ---------------------------------------------------------------------
#ifndef SWIG
  /// Searches the given CairnInfo list for an entry that matches the given mapObject.
  /**
   * The CairnInfo list stores the parameter information (if any) for map 
   * objects. If a map object is removed (or activated), then the CairnInfo 
   * must also be updated.
   * @param mapObjectName the MvrMapObject for which to find the parameters
   * @param cairnInfoList the list of MvrArgumentBuilder *'s that contain the
   * map object parameters (also may be set to the inactive section)
   * @return iterator that points to the parameter information for the map
   * object, or cairnInfoList.end() if not found
  **/
  MVREXPORT static std::list<MvrArgumentBuilder *>::iterator findMapObjectParamInfo
             (const char *mapObjectName,
              std::list<MvrArgumentBuilder*> &cairnInfoList);
#endif
protected:

  MVREXPORT bool setInactiveInfo(const char *infoName,
 						                    const std::list<MvrArgumentBuilder *> *infoList,
                                MvrMapChangeDetails *changeDetails = NULL);

  MVREXPORT void setInactiveObjects(const std::list<MvrMapObject *> *mapObjects,
                                   bool isSortedObjects = false,
                                   MvrMapChangeDetails *changeDetails = NULL); 

  MVREXPORT void setChildObjects(const std::list<MvrMapObject *> *mapObjects,
                                bool isSortedObjects = false,
                                MvrMapChangeDetails *changeDetails = NULL); 

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /// Callback that handles the different types of map categories (e.g. 2D-Map, 2D-Map-Ex)
  /**
   * This method replaces the old handle2DMap method.  It determines which category
   * was detected and sets the myMapCategory attribute accordingly.
   * @param arg a pointer to the parsed MvrArgumentBuilder; no arguments are expected
  **/
  bool handleMapCategory(MvrArgumentBuilder *arg);
  
  /// Callback that handles the Sources keyword
  /**
   * @param arg a pointer to the parsed MvrArgumentBuilder; a list of string scan type
   * arguments are expected
  **/
  bool handleSources(MvrArgumentBuilder *arg);


  /// Callback that handles the different types of data introductions (e.g. DATA, LINES)
  /**
   * This method replaces the old handleData and handleLines methods.  It determines
   * which keyword was detected and updates the myLoadingDataTag and myLoadingScan
   * attributes accordingly.
   * @param arg a pointer to the parsed MvrArgumentBuilder; no arguments are expected
  **/
  bool handleDataIntro(MvrArgumentBuilder *arg);


  bool handleRemainder(MvrArgumentBuilder *arg);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns the MvrMapScan for the specified scan type.
  MVREXPORT virtual MvrMapScan *getScan(const char *scanType) const;

  /// Sets up the map to contain teh specified scan types.
  /**
   * Any scans which are currently in the map are cleared and removed.
   * This method is not virtual because it is called by the constructor.
   * @param scanTypeList a list of the scan type string identifiers to be 
   * created; the list must be non-empty and must not contain duplicates;
   * if the list contains more than one entry, then they all must be 
   * non-empty
   * @return bool true if the scans were successfully created; false otherwise
  **/
  bool createScans(const std::list<std::string> &scanTypeList);

  /// Adds all of the map's scan types to the current file parser.
  /**
   * This method calls addToFileParser() on each of the map's scans.  It also
   * adds handlers for each of the scans' data point and line introduction 
   * keywords.
   * @return bool true if the scans were successfully added to the current
   * file parser
  **/
  bool addScansToParser();

  /// Removes all of the map's scan types from the current file parser.
  bool remScansFromParser(bool isRemovePointsAndLinesKeywords = true);

  MVREXPORT void writeScanTypesToFunctor(MvrFunctor1<const char *> *functor, 
			                                  const char *endOfLineChars);

  MVREXPORT MvrTime findMaxMapScanTimeChanged();

  MVREXPORT MvrMapScan *findScanWithDataKeyword(const char *myLoadingDataTag,
                                              bool *isLineDataTagOut);
  
  MVREXPORT void updateSummaryScan();
  

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  

  MVREXPORT virtual const char *getMapCategory();


  MVREXPORT virtual void updateMapCategory(const char *updatedInfoName = NULL);

  MVREXPORT virtual bool mapInfoContains(const char *arg0Text);

  MVREXPORT bool isDataTag(const char *line); 

  MVREXPORT void reset();

  MVREXPORT void updateMapFileInfo(const char *realFileName);



  MVREXPORT static int getNextFileNumber();

  MVREXPORT void invokeCallbackList(std::list<MvrFunctor*> *cbList);
  
  MVREXPORT void addToCallbackList(MvrFunctor *functor,
                                  MvrListPos::Pos position,
                                  std::list<MvrFunctor*> *cbList);

  MVREXPORT void remFromCallbackList(MvrFunctor *functor,
                                    std::list<MvrFunctor*> *cbList);

protected:

  // static const char *ourDefaultInactiveInfoNames[INFO_COUNT];


  static int ourTempFileNumber;
  static MvrMutex ourTempFileNumberMutex;

  // lock for our data
  MvrMutex myMutex;

  std::list<std::string> myMapCategoryList;
  std::string myMapCategory;


  MvrMD5Calculator *myChecksumCalculator;

  std::string myBaseDirectory;
  std::string myFileName;
  struct stat myReadFileStat;

  std::list<MvrFunctor*> myPreWriteCBList;
  std::list<MvrFunctor*> myPostWriteCBList;

  bool myIsWriteToTempFile;
  std::string myTempDirectory;

  MvrMapId myMapId;

  MvrFileParser *myLoadingParser;

  // std::string myConfigParam;
  bool myIgnoreEmptyFileName;
  bool myIgnoreCase;

  MvrMapChangedHelper *myMapChangedHelper;

  /***
  // things for our config
  bool myConfigProcessedBefore;
  char myConfigMapName[MAX_MAP_NAME_LENGTH];
  ***/

  bool myLoadingGotMapCategory; 
  // TODO: Need to change for multi scans
  bool myLoadingDataStarted;
  bool myLoadingLinesAndDataStarted;

  MvrMapInfo       * const myMapInfo;
  MvrMapObjects    * const myMapObjects;
  MvrMapSupplement * const myMapSupplement;

  std::list<std::string> myScanTypeList;
  MvrTypeToScanMap myTypeToScanMap;
  MvrMapScan    * mySummaryScan;

  MvrDataTagToScanTypeMap myDataTagToScanTypeMap;

  std::string    myLoadingDataTag;
  MvrMapScan    * myLoadingScan;

  MvrMapInfo    * const myInactiveInfo;
  MvrMapObjects * const myInactiveObjects;

  MvrMapObjects * const myChildObjects;

  std::map<std::string, MvrArgumentBuilder *, MvrStrCaseCmpOp> myMapObjectNameToParamsMap;

  /// List of map file lines that were not recognized
  std::list<MvrArgumentBuilder *> myRemainderList;

  MvrTime myTimeMapInfoChanged;
  MvrTime myTimeMapObjectsChanged;
  MvrTime myTimeMapScanChanged;
  MvrTime myTimeMapSupplementChanged;

  // callbacks
  MvrRetFunctor1C<bool, MvrMapSimple, MvrArgumentBuilder *> myMapCategoryCB;
  MvrRetFunctor1C<bool, MvrMapSimple, MvrArgumentBuilder *> mySourcesCB;
  MvrRetFunctor1C<bool, MvrMapSimple, MvrArgumentBuilder *> myDataIntroCB;

  // Handler for unrecognized lines
  MvrRetFunctor1C<bool, MvrMapSimple, MvrArgumentBuilder *> myRemCB;

  bool myIsQuiet;
  bool myIsReadInProgress;
  bool myIsCancelRead;

}; // end class MvrMapSimple

/// --------------------------------------------------------------------------- 

#endif // MVRMAPCOMPONENTS_H

