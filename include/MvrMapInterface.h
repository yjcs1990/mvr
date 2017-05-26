/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMapInterface.h
 > Description  : Contains the set of interfaces that define the Mvria maps.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/

/* The methods that define the Mvria maps are grouped into a set of smaller
 * interfaces which are contained in this file.   These include:
 * 
 *  - MvrMapScanInterface : Methods related to scan data (e.g.  points and
 *    lines) generally obtained from a specific sensor
 *
 *  - MvrMapObjectsInterface : Methods related to the objects in a map.
 *    These include user-defined objects such as goals, docks, and forbidden
 *    lines.  They also include special "data" types of objects that may be
 *    automatically created during the scanning process.
 * 
 *  - MvrMapInfoInterface : Methods related to the various info arguments
 *    stored in a map.  Examples of info include the map object type 
 *    definitions and the routes and macros.
 * 
 *  - MvrMapSupplementInterface : Methods used to access supplemental data 
 *    associated with the map.  This currently consists only of the latitude /
 *    longitude of the origin, but may be extended in the future.
 * 
 * The primary interface implemented by all Mvria maps is also contained in
 * this file:
 *
 *  - MvrMapInterface : All of the methods that are available for an Mvria map.
 *    This interface extends all of the above interfaces and contains a 
 *    few additional file-related methods.
 * 
 * In general, when passing an Mvria map (pointer) as a parameter to a method, 
 * it is preferable to pass MvrMapInterface *.  This will allow for extension 
 * and different implementations of the interface. 
 *
 * <pre>  
 *      MvrMapScanInterface
 *      ^
 *      |    MvrMapObjectsInterface
 *      |    ^
 *      |    |    MvrMapInfoInterface
 *      |    |    ^
 *      |    |    |    MvrMapSupplementInterface
 *      |    |    |    ^  
 *      |    |    |    | (extends)
 *      |    |    |    |
 *       MvrMapInterface
 *
 * </pre>
**/
#ifndef MVRMAPINTERFACE_H
#define MVRMAPINTERFACE_H
 
#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

#include "MvrMapObject.h"
#include "MvrMapUtils.h"

#include "MvrArgumentBuilder.h"
#include "MvrFunctor.h"
#include "MvrHasFileName.h"
#include "MvrMutex.h"

#include <vector>
#include <set>

class MvrFileParser;
class MvrMapChangeDetails;
class MvrMapObject;


// =============================================================================
// MvrMapScanInterface
// =============================================================================

/// The default scan type indicator, when an Mvria map only has obstacle data of one type
#define MVRMAP_DEFAULT_SCAN_TYPE ""
/// The scan type indicator used to obtain collective data from most or all obstacle data (see individual methods for how it is used for specific kinds of map data)
#define MVRMAP_SUMMARY_SCAN_TYPE NULL

/// Methods related to setting and retrieving the scan-related data in an Mvria map.
/**
 * MvrMapScanInterface is an abstract class that defines the methods used
 * to manipulate obstacle data in an Mvria map.  These generally include the 
 * points and lines detected by a particular sensor (a.k.a. scan type).
**/
class MvrMapScanInterface 
{
public:

  /// Determines whether the given scan type is the default (i.e. no prefix on the map tags)
  MVREXPORT static bool isDefaultScanType(const char *scanType);
  /// Determines whether the given scan type represents the summary of all other scan types
  MVREXPORT static bool isSummaryScanType(const char *scanType);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Default constructor
  MVREXPORT MvrMapScanInterface() {}
  /// Destructor
  MVREXPORT virtual ~MvrMapScanInterface() {}

  MVREXPORT virtual const char *getDisplayString
                                  (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Point Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns a pointer to the map points for the specified scan.
  /**
   *  Note that this returns a pointer to the object's internal vector.
   *  The map must be locked before this method is called, and must be
   *  unlocked after the caller has finished using the vector.
   *  @param scanType the const char * identifier of the scan type for
   *  which to return the points; must be non-NULL
   *  @return a pointer to the std::vector<MvrPose> that contains the 
   *  specified scan's points; NULL if the scanType is undefined for 
   *  the map
  **/
  MVREXPORT virtual std::vector<MvrPose> *getPoints
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns the lower left point (minimum x and y) of the scan's points.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the minimum pose; if MVRMAP_SUMMARY_SCAN_TYPE, then the 
   * minimum pose for all scans is returned
  **/
  MVREXPORT virtual MvrPose getMinPose
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns the upper right point (maximum x and y) of the scan's points.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the maximum pose; if MVRMAP_SUMMARY_SCAN_TYPE, then the 
   * maximum pose for all scans is returned
  **/
  MVREXPORT virtual MvrPose getMaxPose
            const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns the number of points in the specified scan.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the point count; if MVRMAP_SUMMARY_SCAN_TYPE, then the 
   * point count for all scans is returned
  **/
  MVREXPORT virtual int getNumPoints
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;
  

  /// Returns whether the point vector for the specified scan is sorted
  /**
   * Note that this method returns the state of the point vector when it 
   * was read from the file, or after setPoints has been called.  If the
   * application calls getPoints and directly manipulates the point vector's
   * contents, then isSortedPoints will not necessarily be correct.
   * 
   * @param scanType the const char * identifier of the scan type for 
   * which to return the sorted state; if MVRMAP_SUMMARY_SCAN_TYPE, then this method
   * returns true if and only if all scans have sorted points
  **/
  MVREXPORT virtual bool isSortedPoints
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const = 0;

  /// Sets the points (copies those passed in)
  /**
   * This method will sort the given points in ascending order (according 
   * to the MvrPose less-than operator.  If the points vector is already sorted, 
   * then set the isSortedPoints parameter to true in order to improve performance.
   * @param points a pointer to the MvrPose vector to be copied to the scan data
   * @param scanType the const char * identifier of the scan type for which 
   * to set the points; must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   * @param isSortedPoints a bool set to true if the points vector has already
   * been sorted in ascending order
   * @param changeDetails a pointer to the optional MvrMapChangeDetails in which
   * to store a description of the changes to the scan data; if NULL then the 
   * changes are not tracked.
   * @see MvrMapChangeDetails
  **/
  MVREXPORT virtual void setPoints(const std::vector<MvrPose> *points,
                                   const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                   bool isSortedPoints = false,
                                   MvrMapChangeDetails *changeDetails = NULL) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Line Segment Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns a pointer to the line segments for the specified scan.
  /**
   *  Note that this returns a pointer to the object's internal vector.
   *  The map must be locked before this method is called, and must be
   *  unlocked after the caller has finished using the vector.
   *  @param scanType the const char * identifier of the scan type for
   *  which to return the line segments; must be non-NULL
   *  @return a pointer to the std::vector<MvrPose> that contains the 
   *  specified scan's points; NULL if the scanType is undefined for 
   *  the map
  **/
  MVREXPORT virtual std::vector<MvrLineSegment> *getLines
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns the lower left point (minimum x and y) of the scan's line segments.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the minimum line segment pose; if MVRMAP_SUMMARY_SCAN_TYPE, 
   * then the minimum line segment pose for all scans is returned
  **/  
  MVREXPORT virtual MvrPose getLineMinPose
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns the upper right point (maximum x and y) of the scan's line segments.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the maximum line segment pose; if MVRMAP_SUMMARY_SCAN_TYPE, 
   * then the maximum line segment pose for all scans is returned
  **/
  MVREXPORT virtual MvrPose getLineMaxPose
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;
  
  /// Returns the number of line segments in the specified scan.
  /**
   * @param scanType the const char * identifier of the scan type for 
   * which to return the line segment count; if MVRMAP_SUMMARY_SCAN_TYPE, then the 
   * line segment count for all scans is returned
  **/
  MVREXPORT virtual int getNumLines
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Returns whether the line segment vector for the specified scan is sorted
  /**
   * Note that this method returns the state of the line segment vector when it 
   * was read from the file, or after setLines has been called.  If the
   * application calls getLines and directly manipulates the line segment vector's
   * contents, then isSortedLines will not necessarily be correct.
   * 
   * @param scanType the const char * identifier of the scan type for 
   * which to return the sorted state; if MVRMAP_SUMMARY_SCAN_TYPE, then this method
   * returns true if and only if all scans have sorted line segments
  **/
  MVREXPORT virtual bool isSortedLines(const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) const = 0;


  /// Sets the lines (copies those passed in)
  /**
   * This method will sort the given line segments in ascending order (according 
   * to the MvrLineSegment less-than operator.  If the vector is already sorted, 
   * then set the isSortedLiens parameter to true in order to improve performance.
   * @param lines a pointer to the MvrLineSegment vector to be copied to the scan data
   * @param scanType the const char * identifier of the scan type for which 
   * to set the points; must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   * @param isSortedLines a bool set to true if the line segment vector has already
   * been sorted in ascending order
   * @param changeDetails a pointer to the optional MvrMapChangeDetails in which
   * to store a description of the changes to the scan data; if NULL then the 
   * changes are not tracked.
   * @see MvrMapChangeDetails
  **/
  MVREXPORT virtual void setLines(const std::vector<MvrLineSegment> *lines,
                                  const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedLines = false,
                                  MvrMapChangeDetails *changeDetails = NULL) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Other Attributes
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Gets the resolution (-1 if none specified)
  /**
   * @param scanType the const char * identifier of the scan for which to return
   * the resolution; if MVRMAP_SUMMARY_SCAN_TYPE then the lowest resolution (highest 
   * numerical value) of all scans is returned
  **/
  MVREXPORT virtual int getResolution
            (const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Sets the resolution (-1 if none specified)
  /**
   * @param resolution the int number of mm between the scan data point readings
   * @param scanType the const char * identifier of the scan for which to return
   * the resolution; must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   * @param changeDetails a pointer to the optional MvrMapChangeDetails in which
   * to store a description of the changes to the scan data; if NULL then the 
   * changes are not tracked.
   * @see MvrMapChangeDetails
  **/
  MVREXPORT virtual void setResolution(int resolution,
                                       const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
                                       MvrMapChangeDetails *changeDetails = NULL) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Persistence
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
  // Writes the scan header information to the given functor; does not write the points or lines.
  /**
   * This method writes a text line for each of the scan header attributes 
   * (such as MinPose, MaxPose, NumPoints).  Depending on the scan type, a 
   * prefix may be prepended to the keyword (e.g. HokuyoURGMinPose).
   * 
   * @param functor the MvrFunctor1 to which to write the scan header information
   * (as text lines)
   * @param endOfLineChars the const char * string to be appended to the end
   * of each text line
   * @param scanType the const char * identifier of the scan data to be written to
   * the functor; must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   *
  **/
  MVREXPORT virtual void writeScanToFunctor
            (MvrFunctor1<const char *> *functor, 
 			       const char *endOfLineChars,
             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE) = 0;

  /// Writes the scan data points to a functor.
  /**
   * A pointer to the entire data point vector is passed directly to the 
   * functor in order to improve performance.  The functor must not
   * modify the vector's contents.
   * @param functor a pointer to the MvrFunctor2 that takes the number of points
   * and the vector of MvrPoses, and writes the information
   * @param scanType the const char * identifier of the scan points to be written;
   * must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   * @param keywordFunctor a pointer to an optional MvrFunctor1 that writes a 
   * text line to introduce the associated MvrPoses; if NULL, then the header is not
   * written
  **/
  MVREXPORT virtual void writePointsToFunctor
            (MvrFunctor2<int, std::vector<MvrPose> *> *functor,
             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
             MvrFunctor1<const char *> *keywordFunctor = NULL) = 0;

  /// Writes the map line segments to a functor.
  /**
   * A pointer to the entire data line segment vector is passed directly to the 
   * functor in order to improve performance.  The functor must not
   * modify the vector's contents.
   * @param functor a pointer to the MvrFunctor2 that takes the number of line
   * segments and the vector of MvrLineSegments, and writes the information
   * @param scanType the const char * identifier of the scan points to be written;
   * must not be MVRMAP_SUMMARY_SCAN_TYPE or NULL
   * @param keywordFunctor a pointer to an optional MvrFunctor1 that writes a 
   * text line to introduce the associated MvrLineSegments; if NULL, then the
   * header is not written
  **/
  MVREXPORT virtual void writeLinesToFunctor
 	          (MvrFunctor2<int, std::vector<MvrLineSegment> *> *functor,
             const char *scanType = MVRMAP_DEFAULT_SCAN_TYPE,
             MvrFunctor1<const char *> *keywordFunctor = NULL) = 0;


  /// Adds handlers for this scan data's header information to the given file parser
  /**
   * @param fileParser a pointer to the MvrFileParser to which to add the handlers
   * @return bool true if the handlers were successfully added; false if an error
   * occurred
  **/
  MVREXPORT virtual bool addToFileParser(MvrFileParser *fileParser) = 0;

  /// Removes the handlers for this scan data's header information from the given file parser
  /**
   * @param fileParser a pointer to the MvrFileParser from which to remove the handlers
   * @return bool true if the handlers were successfully removed; false if an error
   * occurred
  **/
  MVREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser) = 0;


  /// Reads a data point from the given line and adds it to this scan data.
  /**
   * The data point is expected to be two integers (x y), separated by whitespace.
   * @param line the char * text line to be parsed
   * @return bool true if the point was successfully read and added to this scan
   * data; false if an error occurred
  **/
  MVREXPORT virtual bool readDataPoint( char *line) = 0;
  
  /// Reads a line segment from the given line and adds it to this scan data.
  /**
   * The line segment is expected to be four integers (x1 y1 x2 y2), separated 
   * by whitespace
   * @param line the char * text line to be parsed
   * @return bool true if the line segment was successfully read and added to 
   * this scan data; false if an error occurred
  **/
  MVREXPORT virtual bool readLineSegment( char *line) = 0;

  /*** Public for MvrQClientMapProducer ***/
  /// Adds the specified data point to the scan data.
  MVREXPORT virtual void loadDataPoint(double x, double y) = 0;
  /// Adds the specified line segment to the scan data.
  MVREXPORT virtual void loadLineSegment(double x1, double y1, 
                                         double x2, double y2) = 0;

}; // end class MvrMapScanInterface


// =============================================================================
// MvrMapObjectsInterface
// =============================================================================

/// Methods related to setting and retrieving the objects in an Mvria map.
/**
 * MvrMapObjectsInterface is an abstract class that defines the methods used
 * to manipulate the MvrMapObjects that are contained in an Mvria map.
**/
class MvrMapObjectsInterface 
{

public :
 
   /// Constructor 
   MVREXPORT MvrMapObjectsInterface() {}
 
   /// Destructor
   MVREXPORT virtual ~MvrMapObjectsInterface() {}
 
 
   /// Returns the first map object of given name and type, or NULL if none is found
   /**
    * A pointer to the actual map object is returned.  It is not safe to 
    * store this pointer because it will be deleted when the map is changed.
    * If the caller needs the map object, then it should create its own copy.
    * This method is not thread-safe.
    * 
    * @param name the const char * name of the object to be found; if NULL then
    * any object of the specified type is a match
    * @param type the const char * type of the object to be found; if NULL then
    * search all object types
    * @param isIncludeWithHeading a bool set to true if the given type represents a 
    * pose and both "heading-less" and "with-heading" objects should be searched; 
    * if false, then only objects of the exact type are searched
    * @return MvrMapObject * the matching map object, or NULL if none found
   **/
   MVREXPORT virtual MvrMapObject *findFirstMapObject(const char *name, 
                                                      const char *type,
                                                      bool isIncludeWithHeading = false) = 0;
 
   /// Returns the  map object of given name and type, or NULL if none is found
   /**
    * A pointer to the actual map object is returned.  It is not safe to 
    * store this pointer because it will be deleted when the map is changed.
    * If the caller needs the map object, then it should create its own copy.
    * This method is not thread-safe.
    * 
    * @param name the const char * name of the object to be found
    * @param type the const char * type of the object to be found; if NULL then
    * search all object types
    * @param isIncludeWithHeading a bool set to true if the given type represents a 
    * pose and both "heading-less" and "with-heading" objects should be searched; 
    * if false, then only objects of the exact type are searched
    * @return MvrMapObject * the matching map object, or NULL if none found
   **/
   MVREXPORT virtual MvrMapObject *findMapObject(const char *name, 
 				                                         const char *type = NULL,
                                                 bool isIncludeWithHeading = false) = 0;
 

   /// Returns a list of all map objects of the specified type.
   /**
    * A list of pointers to the actual map objects is returned.  It is not 
    * safe to store these pointers because they will be deleted when the map 
    * is changed.  If the caller needs the map objects at a later time, then 
    * it should create its own copy of each object in the list.
    * This method is not thread-safe.
    * 
    * @param type the const char * type of the objects to be found; if NULL then
    * all objects are returned
    * @param isIncludeWithHeading a bool set to true if the given type represents a 
    * pose and both "heading-less" and "with-heading" objects should be searched; 
    * if false, then only objects of the exact type are searched
    * @return a list of pointers to all of the MvrMapObject's  that match the given
    * type
   **/
   MVREXPORT virtual std::list<MvrMapObject *> findMapObjectsOfType
             (const char *type, bool isIncludeWithHeading = false) = 0;

   /// Returns a pointer to the internal list of map objects.
   /**
    * Ideally, callers of this method should not use the pointer to modify
    * the map objects directly.  It is preferable to modify a copy and then
    * call setMapObjects.
    *
    * It is not safe to store the returned pointer list because the pointers will 
    * be deleted when the map is changed.  If the caller needs the map objects at 
    * a later time, then it should create its own copy of each object in the list.
    * This method is not thread-safe.   
    * @return a list of pointers to all of the MvrMapObject's in the map
   **/
   MVREXPORT virtual std::list<MvrMapObject *> *getMapObjects(void) = 0;
 
   /// Sets the map objects (copies those passed in)
   /**
    * This method sets its internal list to contain a copy of all of the given 
    * map objects.  Any map objects which were originally in the list but are
    * no longer referenced are deleted.  The list of map objects will be sorted
    * by increasing pose (i.e. objects in the upper left will be placed before
    * objects in the lower right).
    * This method is not thread-safe.
    *
    * @param mapObjects a pointer to the list of MvrMapObject *'s to be copied
    * @param isSortedObjects a bool set to true if the objects in the given 
    * list have already been sorted by increasing pose; this can improve 
    * the performance of this method
    * @param changeDetails an optional pointer to the MvrMapChangeDetails in 
    * which to accumulate a description of the changes to the map objects; 
    * if NULL, then changes are not tracked
    * @see MvrMapChangeDetails
   **/
   MVREXPORT virtual void setMapObjects
             (const std::list<MvrMapObject *> *mapObjects,
              bool isSortedObjects = false, MvrMapChangeDetails *changeDetails = NULL) = 0; 
 

   // TODO Seems like it would be awfully nice to have an addMapObject and a
   // removeMapObject method


   /// Writes the list of map objects to a text-based functor.
   /**
    * This method writes a Cairn text line for each of the MvrMapObject's.  
    * This method is not thread-safe.
    *
    * @param functor a pointer to the MvrFunctor1 that writes the text lines
    * @param endOfLineChars the const char * string that indicates the end of
    * each text line
   **/
   MVREXPORT virtual void writeObjectListToFunctor(MvrFunctor1<const char *> *functor, 
 			                                             const char *endOfLineChars) = 0;
 
 }; // end class MvrMapObjectsInterface


// =============================================================================
// MvrMapInfoInterface
// =============================================================================

/// Methods related to setting and retrieving the various "info" tags in an Mvria map.
/**
 * MvrMapInfoInterface is an abstract class that defines the methods used to 
 * get and set an Mvria map's "info" arguments.  An Mvria map may have one or more
 * categories of info, each implemented by an ordered list of MvrArgumentBuilder's.
 * The MvrMapInfoInterface defines access to a collection of these info categories.
**/
class MvrMapInfoInterface 
{
public :
  

 	MVREXPORT static const char *MAP_INFO_NAME; 
 	MVREXPORT static const char *META_INFO_NAME;
 	MVREXPORT static const char *TASK_INFO_NAME;   
 	MVREXPORT static const char *ROUTE_INFO_NAME; 
 	MVREXPORT static const char *SCHED_TASK_INFO_NAME;
 	MVREXPORT static const char *SCHED_INFO_NAME; 
 	MVREXPORT static const char *CAIRN_INFO_NAME;  
 	MVREXPORT static const char *CUSTOM_INFO_NAME;



  /// Constructor
  MVREXPORT MvrMapInfoInterface() {}

  /// Destructor
  MVREXPORT virtual ~MvrMapInfoInterface() {}

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Getters
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns the number of info categories that are contained in this collection.
  MVREXPORT virtual int getInfoCount() const = 0;

  /// Returns a list of the info category names that are contained in this collection.
  /**
   * @return the list of string names of the info categories (for example, 
   * MAP_INFO_NAME, META_INFO_NAME, ...)
   * This method is not thread-safe.
  **/
  MVREXPORT virtual std::list<std::string> getInfoNames() const = 0;


  /// Returns the arguments for the specified info category; or NULL if none found
  /**
  * This method returns a pointer to the actual MvrArgumentBuilder list that is
  * contained in the map.  It is not safe to store these pointers because they 
  * will be deleted when the map is changed.  If the caller needs the info
  * arguments at a later time, then it should create its own copy of each 
  * argument in the list.
  * This method is not thread-safe.
  *
  * @param infoName the unique char * identifier of the info category to be 
  * returned; must be non-NULL and a member of 
  * @return std::list<MvrArgumentBuilder *> * a pointer to the specified Info 
  * list; NULL if infoType was invalid
  **/
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(const char *infoName) = 0;


  /// Gets the strings for the specified Info category.
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getInfo(int infoType) = 0;

  /// Gets the map info strings
  /**
   * This method is equivalent to getInfo(MAP_INFO_NAME).
   * @see getInfo
  **/
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getMapInfo(void) = 0;


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Setters
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  /// Sets the contents of the specified Info category (copies those passed in)
  /**
   * This method is not thread-safe.
   * @param infoName the unique const char * identifier of the 
   * @param infoList the std::list<MvrArgumentBuilder *> * that defines the
   * Info category's contents; NULL to clear the Info
   * @param changeDetails a pointer to the MvrMapChangeDetails in which to 
   * accumulate changes to the map's info lines; if NULL, then changes are
   * not tracked
   * @return bool set to true if the contents were successfully set; false,
   * if an error occurred
   * @see MvrMapChangeDetails
   * @see setInfoNames
  **/
  MVREXPORT virtual bool setInfo(const char *infoName,
                                 const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL) = 0;


  /// Sets the contents of the specified Info category (copies those passed in)
  MVREXPORT virtual bool setInfo(int infoType,
 						                     const std::list<MvrArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL) = 0; 


  /// Sets the map info (copies those passed in)
  /**
   * This method is equivalent to setInfo(MAP_INFO, mapInfo, changeDetails);
  **/
  MVREXPORT virtual bool setMapInfo(const std::list<MvrArgumentBuilder *> *mapInfo,
                                    MvrMapChangeDetails *changeDetails = NULL) = 0;


  //MVREXPORT virtual bool setInfoNames(const std::list<std::string> &infoNameList) = 0;


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Writes the info collection to a text-based functor.
  /**
  * This method writes a text line for each of the info's MvrArgumentBuilders.
  * This method is not thread-safe.
  *
  * @param functor a pointer to the MvrFunctor1 that writes the text lines
  * @param endOfLineChars the const char * string that indicates the end of
  * each text line
  **/
  MVREXPORT virtual void writeInfoToFunctor
 				     (MvrFunctor1<const char *> *functor, const char *endOfLineChars) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Deprecated
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// List of the standard Info categories defined for Mvria maps
  enum InfoType {
 	  MAP_INFO,               ///< MapInfo lines that define the valid map object types
    FIRST_INFO = MAP_INFO,  ///< First value in the enumeration
    META_INFO,              ///< MetaInfo lines that provide overview info about the map
 	  TASK_INFO,              ///< TaskInfo lines that define the available robot tasks 
 	  ROUTE_INFO,             ///< RouteInfo lines that are the goal assocs, macros, and routes
    SCHED_TASK_INFO,        ///< SchedTaskInfo that define special tasks that may be scheduled
    SCHED_INFO,             ///< SchedInfo lines that are the schedules of route patrols
    CAIRN_INFO,             ///< CairnInfo lines that contain optional arguments for map objects
    CUSTOM_INFO,            ///< CustomInfo lines contain application specific data
 	  LAST_INFO = CUSTOM_INFO ///< Last value in the enumeration
  };

  enum {
 	  INFO_COUNT = LAST_INFO + 1 ///< Number of standard Info categories
  };


  /// Returns the name of the specified Info type 
  MVREXPORT virtual const char *getInfoName(int infoType) = 0;


}; // end class MvrMapInfoInterface


// =============================================================================
// MvrMapSupplementInterface
// =============================================================================

/// Methods related to miscellaneous extra data in an Mvria map.
/**
 * MvrMapSupplementInterface is basically a repository for any pieces of data 
 * that do not fit in the other categories (i.e. scan data, map objects, or 
 * info).  It currently stores the optional latitude/longitude information.
**/
class MvrMapSupplementInterface 
{
public:

  /// Default constructor
  MVREXPORT MvrMapSupplementInterface() {}
  
  /// Destructor
  MVREXPORT virtual ~MvrMapSupplementInterface() {}

  /// Returns whether this map has an origin specified in latitude/longitude (and altitude)
  MVREXPORT virtual bool hasOriginLatLongAlt() = 0;
  
  /// Returns the latitude/longitude origin; valid only if hasOriginLatLongAlt returns true
  MVREXPORT virtual MvrPose getOriginLatLong() = 0;
  
  /// Gets the altitude of the origin; valid only if hasOriginLatLongAlt returns true
  /**
   * @return double the altitude (in meters) of the origin
  **/
  MVREXPORT virtual double getOriginAltitude() = 0;

  /// Sets the latitude/longitude and altitude of the origin.
  /**
   * @param hasOriginLatLong a bool set to true if the map has latitude/longitude
   * information for the origin
   * @param originLatLong the MvrPose that specifies the latitude/longitude of the
   * origin
   * @param altitude the double altitude (in meters) of the origin 
   * @param changeDetails a pointer to the optional MvrMapChangeDetails in which
   * to store a description of the changes to the scan data; if NULL then the 
   * changes are not tracked.
  **/
  MVREXPORT virtual void setOriginLatLongAlt
                                        (bool hasOriginLatLong,
                                         const MvrPose &originLatLong,
                                         double altitude,
                                         MvrMapChangeDetails *changeDetails = NULL) = 0;
  
  /// Writes the supplemental data to the given functor
  /**
   * @param functor the MvrFunctor1 to which to write the supplemental data
   * (as text lines)
   * @param endOfLineChars the const char * string to be appended to the end
   * of each text line
  **/
  MVREXPORT virtual void writeSupplementToFunctor
                                (MvrFunctor1<const char *> *functor, 
 			                           const char *endOfLineChars) = 0;

}; // end class MvrMapSupplementInterface


// =============================================================================
// MvrMapInterface
// =============================================================================

/**
 * MvrMapInterface defines the methods that are available on all Mvria maps.
 * These maps represent the operating space of a robot, and can be used for
 * space searching, localizing, navigating etc.  The types of data stored 
 * in a map include sensable obstacles (e.g. walls and furniture in a room)
 * represented either as a collection of data points (similar to a raster 
 * or bit map, useful for high resolution sensors like a laser), or lines 
 * (a vector map, useful for low resolution sensors like the sonar), 
 * goals, and other points or regions of interest ("map objects"). 
 * 
 * The methods in MvrMapInterface can be broadly categorized as follows:
 *   - Scan Methods:  These provide access the sensable obstacles that are
 *     represented as a collection of data points or lines.  These are 
 *     typically generated during the scanning process (i.e. the creation of 
 *     the .2d file).  If more than one sensor is used, then the data is
 *     organized on a per-sensor basis.  The scan methods are grouped into 
 *     the MvrMapScanInterface.  
 *   - Object Methods:  These provide access to the "high-level" objects 
 *     in the environment.  Such objects include goals, docks, forbidden 
 *     areas, and other user-defined points of interest.  They may also 
 *     include special data objects that are actually part of the operating
 *     environment and are generally added automatically and are not editable
 *     by the user.  The object methods are grouped into the 
 *     MvrMapObjectsInterface.
 *   - Info Methods: A wide variety of supporting information is included 
 *     in various "info" categories.  This includes definitions for the 
 *     types of map objects that can be stored in the map.  It also includes
 *     various optional features such as macros and schedules.  The info
 *     methods are grouped into the MvrMapInfoInterface.
 *   - Extra Data Methods: These are essentially the "leftovers" -- i.e. 
 *     methods that are related directly to map data but which do not 
 *     fit into any of the above categories.  They are defined in the 
 *     MvrMapSupplementInterface.
 *   - Callback Methods:  Users of the Mvria map may install callbacks onto
 *     the map in order to be notified when the map contents has changed.
 *     These methods are defined below.
 *   - File and I/O Methods: Methods to read and write map files are also
 *     included below.  In addition, the MD5 checksum of the map contents
 *     may be calculated.
 *
 * TODO: 
 *  - Possibly make the calculation of checksums optional?
**/
class MvrMapInterface : public MvrHasFileName,
                       public MvrMapInfoInterface,
                       public MvrMapObjectsInterface,
                       public MvrMapScanInterface,
                       public MvrMapSupplementInterface
{
public:

  enum {
    MAX_MAP_NAME_LENGTH = 512 ///< Maximum length of the map file's name
  };

  MVREXPORT static const char *MAP_CATEGORY_2D;
  MVREXPORT static const char *MAP_CATEGORY_2D_MULTI_SOURCES;
  /// Superset of multi-sources; includes advanced Info types, CairnInfo and CustomInfo
  MVREXPORT static const char *MAP_CATEGORY_2D_EXTENDED;
  /// Superset of extended; includes group objects and parent maps
  MVREXPORT static const char *MAP_CATEGORY_2D_COMPOSITE;


  /// Helper method creates a full file path name from the given components.
  MVREXPORT static std::string createRealFileName(const char *baseDirectory,
                                                 const char *fileName,
                                                 bool isIgnoreCase);

  /// Constructor 
  MVREXPORT MvrMapInterface() {}

  /// Destructor
  MVREXPORT virtual ~MvrMapInterface(void) {}


  /// Clears the map, removing all info, objects and data points and lines.
  MVREXPORT virtual void clear() = 0;

  /// Sets this map to be "equivalent" to the given other map.
  /**
   * Modifies this map so that is effectively a copy of the given map.  All info,
   * objects, and data points and lines in the other map are copied and stored in 
   * this map.
   * @param other a pointer to the MvrMapInterface to be copied; must not be NULL
   * @return bool true if the map was successfully copied to this one; false if 
   * an error occurred
  **/
  MVREXPORT virtual bool set(MvrMapInterface *other) = 0;

  /// Creates a new map that is "equivalent" to this map.
  /**
   * Creates a new map that is effectively a copy of this map.  Note, however,
   * that the returned map may not be of exactly the same class (so the term
   * "clone" is being used somewhat loosely).  In particular, if the active MvrMap
   * that is associated with the robot configuration is cloned, then the resulting
   * map will be simpler and not associated with the robot configuration.
   * @return MvrMapInterface * a new copy of this map
   * @javanote Use cloneMap() instead
  **/
  MVREXPORT virtual MvrMapInterface *clone() = 0;


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Scan Types Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns a list of the scan types that are defined for this map
  /**
   * This method is not thread-safe.
   *
   * @return std::list<std::string> a list of the scan types that are defined 
   * for this map
  **/
  MVREXPORT virtual std::list<std::string> getScanTypes() const = 0;

  /// Sets the scan types that are defined for this map
  /**
   * This method clears all of the exisiting scans (i.e. point and line data).
   * This method is not thread-safe.
   *
   * @param scanTypeList the list of scan type string identifiers to be set;
   * the list must not contain any duplicate entries
   * @return bool true if the scan types were successfully set; false otherwise
  **/
  MVREXPORT virtual bool setScanTypes(const std::list<std::string> &scanTypeList) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Locking / Semaphore Method
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Lock the map instance
  MVREXPORT virtual int lock() = 0;
  /// Try to lock the map instance without blocking
  MVREXPORT virtual int tryLock() = 0;
  /// Unlock the map instance
  MVREXPORT virtual int unlock() = 0;


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Map Changed / Callback Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Function that will call the map changed CBs if needed
  /**
   * Users of Mvria maps should invoke this method after making any changes to the
   * map.  It causes the installed map changed handlers to be invoked.
   * 
   * This method is not thread-safe.  It should be surrounded by calls to lock() 
   * and unlock().  For example:
   * 
   * <code>
   *   std::list<MvrMapObject*> newMapObjects;
   *   // Add some objects...
   *   myMap->lock();
   *   myMap->setMapObjects(&newMapObjects);
   *   myMap->mapChanged();
   *   myMap->unlock();
   * </code>
   *
   * Note that this method is automatically invoked under some circumstances
   * (such as when the map file is re-read following a change to the robot
   * configuration).  Also note that this method will not invoke any callbacks
   * if the map has not been modified via an explicit call to a set method.
   *
   * @see addMapChangedCB
   * @see addPreMapChangedCB
  **/
  MVREXPORT virtual void mapChanged(void) = 0;


  /// Adds a callback that is invoked when the map has been changed.
  /**
   * The given functor should assume that the map has been lock()-ed when
   * it is invoked. It should also not attempt to make changes to the 
   * mapChanged() callback list during the invoke method.
   * This method is not thread-safe.
   *
   * This method is just a wrapper for compatibility, the one that
   * takes position as an integer is the main one that should be used
   * now.
   * 
   * @param functor a pointer to the MvrFunctor to be invoked; must be non-NULL
   * @param position the MvrListPos::Pos indication at which to add the functor
   * (i.e. at the beginning or at the end of the callback list)
  **/
  MVREXPORT virtual void addMapChangedCB(MvrFunctor *functor, 
					MvrListPos::Pos position);


  /// Adds a callback that is invoked when the map has been changed.
  /**
   * The given functor should assume that the map has been lock()-ed when
   * it is invoked. It should also not attempt to make changes to the 
   * mapChanged() callback list during the invoke method.
   * This method is not thread-safe.
   *
   * @param functor a pointer to the MvrFunctor to be invoked; must be non-NULL
   *
   * @param position this indicates the order in which the functors
   * will be called, the nominal range is 0 to 100, highest is called first
  **/
  MVREXPORT virtual void addMapChangedCB
                            (MvrFunctor *functor, 
			     int position = 50) = 0;

  /// Removes a callback called when the map has been changed
  /**
   * This method is not thread-safe.
   *
   * @param functor a pointer to the MvrFunctor to be removed; must be non-NULL
  **/
  MVREXPORT virtual void remMapChangedCB(MvrFunctor *functor) = 0;

  /// Adds a callback called before the map changed callbacks are called
  /**
   * The "pre-map-changed callbacks" are invoked *after* the map has been 
   * changed, but before the other "map-changed callbacks" are invoked.
   * This method is not thread-safe.
   * 
   * This method is just a wrapper for compatibility, the one that
   * takes position as an integer is the main one that should be used
   * now.
   *
   * @param functor a pointer to the MvrFunctor to be invoked; must be non-NULL
   * @param position the MvrListPos::Pos indication at which to add the functor
   * (i.e. at the beginning or at the end of the callback list)
   *
   * @swignote call as addPreMapChangedCBPos() if passing an MvrListPos.Pos
   * object for @a position rather than an int
  **/
  MVREXPORT virtual void addPreMapChangedCB(MvrFunctor *functor, MvrListPos::Pos position);

  /// Adds a callback called before the map changed callbacks are called
  /**
   * The "pre-map-changed callbacks" are invoked *after* the map has been 
   * changed, but before the other "map-changed callbacks" are invoked.
   * This method is not thread-safe.
   * 
   * @param functor a pointer to the MvrFunctor to be invoked; must be non-NULL
   *
   * @param position this indicates the order in which the functors
   * will be called, the nominal range is 0 to 100, highest is called
   * first
  **/
  MVREXPORT virtual void addPreMapChangedCB
                          (MvrFunctor *functor,
                           int position = 50) = 0;


  /// Removes the specified "pre-map-changed callback".
  /**
   * This method is not thread-safe.
   *
   * @param functor a pointer to the MvrFunctor to be removed; must be non-NULL
   **/
  MVREXPORT virtual void remPreMapChangedCB(MvrFunctor *functor) = 0;


  /// Sets the level at which to log information about the map changed callbacks
  MVREXPORT virtual void setMapChangedLogLevel(MvrLog::LogLevel level) = 0; 

  /// Returns the level at which information about the map changed callbacks is logged
  MVREXPORT virtual MvrLog::LogLevel getMapChangedLogLevel(void) = 0; 


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Persistence
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  

  /// Writes all of the map to the given text-based functor 
  /**
   * This method is not thread-safe
   * @param functor a pointer to the MvrFunctor1 to which to write the map 
   * header as text lines
   * @param endOfLineChars the const char * string to use as an end-of-line 
   * indicator
  **/
  MVREXPORT virtual void writeToFunctor(MvrFunctor1<const char *> *functor, 
 			                                const char *endOfLineChars) = 0;

  /// Writes the map header information and objects to a text-based functor.
  /**
   * This method writes all of the map scan headers, the info arguments, and 
   * the Cairn map objects to the given functor.  It does not write the actual 
   * scan point or line segment data (nor the header lines that introduce the
   * data).
   * This method is not thread-safe.
   * 
   * @param functor a pointer to the MvrFunctor1 to which to write the map 
   * header as text lines
   * @param endOfLineChars the const char * string to use as an end-of-line 
   * indicator
   * @param isOverrideAsSingleScan a bool set to true if only a single scan 
   * header should be written; this is generally the "summary scan" and is 
   * used to maintain backwards compatibility with client applications that
   * do not expect multiple scan types in a single map.
   * @param maxCategory if given, limit map category
  **/
  MVREXPORT virtual void writeObjectsToFunctor(MvrFunctor1<const char *> *functor, 
 			                                        const char *endOfLineChars,
                                              bool isOverrideAsSingleScan = false,
                                              const char *maxCategory = NULL) = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File I/O Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Adds a callback to be invoked before the map file is written.
  /**
   * @param functor the MvrFunctor * callback to be added
   * @param position the MvrListPos::Pos that specifies whether the callback
   * should be added at the front or the back of the list
  **/
  MVREXPORT virtual void addPreWriteFileCB(MvrFunctor *functor,
                                          MvrListPos::Pos position = MvrListPos::LAST) = 0;

  /// Removes the given callback from the list of pre-write callbacks.
  /**
   * @param functor the MvrFunctor * callback to be removed 
  **/
  MVREXPORT virtual void remPreWriteFileCB(MvrFunctor *functor) = 0;

  /// Adds a callback to be invoked after the map file is written.
  /**
   * @param functor the MvrFunctor * callback to be added 
   * @param position the MvrListPos::Pos that specifies whether the callback
   * should be added at the front or the back of the list
  **/
  MVREXPORT virtual void addPostWriteFileCB(MvrFunctor *functor,
                                           MvrListPos::Pos position = MvrListPos::LAST) = 0;

  /// Removes the given callback from the list of post-write callbacks.
  /**
   * @param functor the MvrFunctor * callback to be removed 
  **/
  MVREXPORT virtual void remPostWriteFileCB(MvrFunctor *functor) = 0;

#ifndef SWIG
  /// Reads the map from the specified file
  /**
   * If the file is successfully read into the map, then this method calls
   * mapChanged() afterwards to invoke the installed callbacks.
   * 
   * This method automatically calls lock() and unlock() during operation.  Do 
   * not call this method if the map is already locked.
   *
   * @param fileName the name of the file to read; Unless an absolute path
   * to a file is given (starting with "/" or "\" or, on Windows, a drive letter
   * root such as "C:\", "D:\", etc.), it is combined 
   * with this map's base directory (see getBaseDirectory()) to form the complete file path name; must be 
   * non-NULL
   * @param errorBuffer a pointer to a char buffer in which specific read errors 
   * can be recorded; if NULL, then the return value is the only success indication
   * @param errorBufferLen the size_t of the error buffer
   * @param md5DigestBuffer an optional pointer to a buffer in which to store the
   * calculated checksum of the map; if NULL, then the checksum is not output
   * @param md5DigestBufferLen the size_t of the checksum buffer
   * @return bool true if the file was successfully read and the map was populated;
   * false if an error occurred
   * @see MvrMD5Calculator
  **/
  MVREXPORT virtual bool readFile(const char *fileName, 
 			                           char *errorBuffer = NULL, 
                                 size_t errorBufferLen = 0,
                                 unsigned char *md5DigestBuffer = NULL,
                                 size_t md5DigestBufferLen = 0) = 0;

  /// Writes the map to the specified file
  /**
   * By default, this method automatically calls lock() and unlock() during its
   * operation. If the map is already locked when the file needs to be written,
   * then set the internalCall parameter to true to override the default locking
   * behavior
   *
   * @param fileName the const char * name of the file to written; it is combined 
   * with the getBaseDirectory() to form the complete file path name; must be 
   * non-NULL
   * @param internalCall a bool set to true if writeFile is being called while 
   * the map is already locked; set to false to indicate that the map should 
   * lock itself during the method
   * @param md5DigestBuffer an optional pointer to a buffer in which to store the
   * calculated checksum of the map; if NULL, then the checksum is not output
   * @param md5DigestBufferLen the size_t of the checksum buffer
   * @param fileTimestamp the time_t to which to set the file write time; in 
   * general, this should be left as -1 to indicate that the actual write time is
   * desired; a real time value can be used to synchronize the map across many 
   * robots
   * @return bool true if the file was successfully written; false if an error
   * occurred
   * @see MvrMD5Calculator
  **/
  MVREXPORT virtual bool writeFile(const char *fileName, 
                                  bool internalCall = false,
                                  unsigned char *md5DigestBuffer = NULL,
                                  size_t md5DigestBufferLen = 0,
                                  time_t fileTimestamp = -1) = 0;
#endif

#ifndef SWIG
  /// Returns information about the map file that was read.
  /** @swigomit */
  MVREXPORT virtual struct stat getReadFileStat() const = 0;
#endif

  /// Retrieves the map ID.
  /**
   * The map ID is a unique identifier based on the map file name and the
   * checksum data.  (Perhaps more accurately, it is highly likely to be 
   * unique during normal usage.)  
   * @param mapIdOut a pointer to the MvrMapId to be set 
   * @param isInternalCall a bool set to true only when getMapId is called
   * within the context of a method that has already locked the map; if 
   * false, then the map is locked by this method
   * @return bool true if the map ID was successfully set; false, otherwise
  **/
  MVREXPORT virtual bool getMapId(MvrMapId *mapIdOut,
                                 bool isInternalCall = false) = 0;

  /// Calculates the checksum of the map.
  /**
   * @param md5DigestBuffer the unsigned char buffer in which to store
   * the calculated checksum
   * @param md5DigestBufferLen the length of the md5DigestBuffer; should
   * be MvrMD5Calculator::DIGEST_LENGTH
   * @return bool true if the checksum was successfully calculated; 
   * false if an error occurrred
   * @see MvrMD5Calculator
  **/
  MVREXPORT virtual bool calculateChecksum(unsigned char *md5DigestBuffer,
                                          size_t md5DigestBufferLen) = 0;


  /// Gets the base directory
  MVREXPORT virtual const char *getBaseDirectory(void) const = 0;
  /// Sets the base directory
  MVREXPORT virtual void setBaseDirectory(const char *baseDirectory) = 0;

  /// Gets the temp directory
  MVREXPORT virtual const char *getTempDirectory(void) const = 0;
  /// Sets the temp directory
  MVREXPORT virtual void setTempDirectory(const char *tempDirectory) = 0;


  /// Prepends the appropriate directory information on the given filename.
 	MVREXPORT virtual std::string createRealFileName(const char *fileName) = 0;

  /// Gets the fileName that was loaded
  MVREXPORT virtual const char *getFileName(void) const = 0;

  /// Sets the name of the source and the file from which the map was loaded.
  /**
   * This method is primarily used to track when a map has been received
   * from the central server or another robot.  The source and file name
   * can be retrieved from the map ID.
   * @param sourceName the const char * name of the central server or robot
   * from which the map was obtained
   * @param fileName the const char * name of the map file on the source
   * @param isInternalCall a bool set to true if this method is being called
   * while the map is locked; if false, then this method will lock the map
  **/
  MVREXPORT virtual void setSourceFileName(const char *sourceName,
                                          const char *fileName,
                                          bool isInternalCall = false) = 0;



  /// Determines whether the map file needs to be re-read.
  /**
   * This method is primarily applicable to the active Mvria map that is 
   * associated with the robot configuration.  It checks to see if the map 
   * file has been modified since it was read, and re-reads it if necessary.  
   * The method may do nothing for "simpler" maps.
  **/
  MVREXPORT virtual bool refresh() = 0;

  /// Sets whether we ignore empty file names or fail if we encounter one
  /**
   * This method is primarily applicable to the active Mvria map that is 
   * associated with the robot configuration. 
  **/
  virtual void setIgnoreEmptyFileName(bool ignore) = 0;
  /// Gets whether we ignore empty file names or fail if we encounter one
  /**
   * This method is primarily applicable to the active Mvria map that is 
   * associated with the robot configuration. 
  **/
  virtual bool getIgnoreEmptyFileName(void) = 0;
  /// Sets whether we ignore case or not
  /**
   * This method is primarily applicable to the active Mvria map that is 
   * associated with the robot configuration. 
  **/
  virtual void setIgnoreCase(bool ignoreCase = false) = 0;
  /// Gets whether we ignore case or not
  /**
   * This method is primarily applicable to the active Mvria map that is 
   * associated with the robot configuration. 
  **/
  virtual bool getIgnoreCase(void) = 0;


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Inactive Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Provides direct access to the inactive map info.
  /**
   * The "inactive" info is a section of the Mvria map that applications may 
   * use to store "_XInfo:" lines that are not currently active or applicable.
   * The inactive info will be written to the map file, but it will not be 
   * included in any of the inherited MvrMapInfoInterface calls (e.g. getInfo(type)).
   * This is primarily intended for advanced specialized use where something
   * needs to be removed from the map file, but it may be necessary to restore
   * it later.
   * This method is not thread-safe.
   * @return MvrMapInfoInterface * a pointer to the inactive map info section
  **/
  MVREXPORT virtual MvrMapInfoInterface *getInactiveInfo() = 0;

  /// Provides direct access to the inactive map objects.
  /**
   * The "inactive" objects is a section of the Mvria map that applications may 
   * use to store "_Cairn" lines that are not currently active or applicable.
   * The inactive objects will be written to the map file, but they will not be 
   * included in any of the inherited MvrMapObjectsInterface calls 
   * (e.g. getMapObjects()).  This is primarily intended for advanced specialized 
   * use where an object needs to be removed from the map file, but it may be 
   * necessary to restore it later.
   * This method is not thread-safe.
   * @return MvrMapObjectsInterface * a pointer to the inactive map objects section
  **/
  MVREXPORT virtual MvrMapObjectsInterface *getInactiveObjects() = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Child Objects Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Provides direct access to the child map objects which are used to define group templates.
  MVREXPORT virtual MvrMapObjectsInterface *getChildObjects() = 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Returns the optional parameters associated with a map object, or NULL if none.
  /**
   * The Mvria map provides an advanced feature that allows a parameter list
   * to be defined for custom map object types.  (Refer to the MvrMap MapFileFormat
   * for more information.)  
   *
   * This convenience method provides access to the parameter values for a 
   * specified map object.  It is basically a shortcut for searching and 
   * parsing the CairnInfo argument list.  Unlike the CairnInfo arguments 
   * (which also include an introduction and map object name), the
   * argument builder returned by this method only contains the parameter 
   * values.  Also note that if the argument builder contains string 
   * parameters, then they may be surrounded by quotes.
   *
   * The returned pointer should not be stored, modified, or deleted.  
   *
   * See setMapObjectParams for important information about changing the
   * parameter values or the associated map object.
   *
   * This method is not thread-safe.  (The call and use of the returned 
   * MvrArgumentBuilder should be surrounded by calls to lock()/unlock().)
   *
   * @param mapObjectName the unique char * name of the map object whose
   * parameters are to be retrieved
   * @return MvrArgumentBuilder * a pointer to the map object's parameter
   * values; 
  **/
  MVREXPORT virtual MvrArgumentBuilder *findMapObjectParams
                                          (const char *mapObjectName) = 0;


  /// Sets the optional parameters associated with a map object.
  /**
   * This method sets the parameter values for the specified map object.
   * It should be used only for custom map object types that have a 
   * parameter list defined in the MapInfo.  (Refer to the MvrMap MapFileFormat
   * for more information.)  
   *
   * Like findMapObjectParams(), this is basically a convenience method
   * that simplifies access to the data stored in the CairnInfo argument
   * list.  
   *
   * If the given params is non-NULL, then it is copied and stored in 
   * the map.  (Note that this deletes the pointer previously returned by 
   * findMapObjectParams(mapObjectName).  Do not store that pointer.)
   * It is entirely the caller's responsibility to ensure that the 
   * params arg count and types are correct.
   * 
   * If the given params is NULL, then the parameter information for the 
   * map object is cleared.  If a mapObjectWithParams is to be deleted,
   * then setMapObjectParams(mapObjectWithParams->getName(), NULL) must
   * be called first.
   *
   * @param mapObjectName the unique char * name of the map object whose
   * parameters are to be updated
   * @param params the MvrArgumentBuiler * containing the new parameter 
   * values; if NULL, then the parameter information for the map object
   * is deleted
   * @param changeDetails an optional pointer to the MvrMapChangeDetails in 
   * which to accumulate a description of the changes to the map; if NULL, 
   * then changes are not tracked
   * @see MvrMapChangeDetails
  */
  MVREXPORT virtual bool setMapObjectParams(const char *mapObjectName,
                                           MvrArgumentBuilder *params,
                                           MvrMapChangeDetails *changeDetails = NULL) = 0;

  
  /// Returns a list of the map file lines that were not recognized.
  /**
   * Ideally, the returned list should be empty.  The remainder list is 
   * primarily used to determine whether the editor is up-to-date for the 
   * current map version and to try to minimize lost data.
   * 
   * Note that this method returns a pointer to the actual list that is stored 
   * in the map object.  It is not safe to store this pointer.  
   *
   * This method is not thread-safe.
  **/
  MVREXPORT virtual std::list<MvrArgumentBuilder *> *getRemainder() = 0;

  /// Turn on this flag to reduce the number of verbose log messages.
  MVREXPORT virtual void setQuiet(bool isQuiet) = 0;
 	

  /** Public for MvrQMapProducer **/

  /// Parses a map line
  MVREXPORT virtual bool parseLine(char *line) = 0;
  /// Says that the parsing by lines is done and to use the parsed data
  MVREXPORT virtual void parsingComplete(void) = 0;

  // When loading a map, returns whether all header, objects, and lines have completed loading.
  /**
  * This value returns true once the first DATA tag has been reached.  
  * The rest of the map contains data points.
  **/
  MVREXPORT virtual bool isLoadingDataStarted() = 0;

  // When loading a map, returns whether all header and objects have completed loading.
  /**
  * This value returns true once the first LINES tag has been reached.  
  * The rest of the map contains data lines and points.
  **/
  MVREXPORT virtual bool isLoadingLinesAndDataStarted() = 0;

}; // end class MvrMapInterface

#endif // MVRMAPINTERFACE_H
