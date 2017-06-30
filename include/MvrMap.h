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
/*
 * 
 * 
 *
 * Mvria maps are implemented by a collection of classes and interfaces.  The 
 * most important is the MvrMapInterface, which is defined in MvrMapInterface.h
 * and specifies the methods that all Mvria maps must provide.  
 * 
 * This file contains the top-level concrete implementation of the 
 * MvrMapInterface.  MvrMap is basically the map that is used by the robot 
 * to navigate its environment.  In addition to implementing the methods of 
 * MvrMapInterface, MvrMap also provides a means of hooking into the Mvria config.  
 * When the map file name is specified in the Mvria config and is changed during 
 * runtime, MvrMap loads the new file and notifies all listeners that the map 
 * has been changed.
 * 
 * In order to accomplish this, MvrMap has been implemented using the Proxy 
 * design pattern (refer to "Design Patterns", Gamma et al, 1995).  The 
 * participants are as follows:
 * 
 *   - Subject: MvrMapInterface.  The common interface for both the RealSubject
 *     and the Proxy.  It is defined in MvrMapInterface.h and is actually 
 *     the combination of several smaller interfaces:  MvrMapScanInterface,
 *     MvrMapObjectsInterface, MvrMapInfoInterface, and MvrMapSupplementInterface.
 * 
 *   - RealSubject: MvrMapSimple.  The real object that the Proxy represents.
 *     It is defined in MvrMapComponents.h, and is implemented by a set of 
 *     helper classes that are also defined in that header file. 
 *
 *   - Proxy: MvrMap.  Creates and controls access to the RealSubject.  MvrMap
 *     actually contains references to two MvrMapSimple objects.  One is
 *     permanent and is the map object that is used by the robot.  The other
 *     is transient and is the map object that is currently being read from 
 *     a file following an Mvria configuration update.  If the file is 
 *     successfully read, then the loading MvrMapSimple object is copied to 
 *     the main permanent one and mapChanged notifications are sent to
 *     registered listeners.
 * 
 * The following "diagram" illustrates the basic hierarchy:
 *
 * <pre>  
 *        MvrMapInterface                          , Defined in MvrMapInterface.h
 *        ^ (extends)  ^ 
 *        |            |
 *        |            |
 *     MvrMap +-------> MvrMapSimple * +-------> MvrMapScan *
 *                 (contains)        |
 *                                   +-------> MvrMapObjects *
 *                                   |
 *                                   +-------> MvrMapInfo *
 *                                   |
 *                                   +-------> MvrMapSupplement *
 *
 *                                              * : Defined in MvrMapComponents.h
 * </pre>
 * 
 *
 * This header file originally also contained the definition of MvrMapObject.  
 * That class has been moved into its own header file (MvrMapObject.h).
 * 
 *
 * Maintenance Note #1: In this case, the use of the Proxy design pattern implies 
 * that modifying the MvrMap external interface requires multiple changes:  
 * First, the MvrMapInterface (or sub-interface) must be updated.  Then the MvrMap 
 * and MvrMapSimple classes must be modified.  Depending on the change, one of the
 * helper map components may also need to be modified.  It is expected that 
 * such changes to the interface will be infrequent.
 *
 * @see MvrMapInfo
 * @see MvrMapObject
 * @see MvrMapObjects
 * @see MvrMapScan
 * @see MvrMapSupplement
**/
#ifndef ARMAP_H
#define ARMAP_H
 
#include "ariaTypedefs.h"
#include "ariaUtil.h"

#include "MvrMapComponents.h"
#include "MvrMapInterface.h"
#include "MvrMapUtils.h"

#include "MvrFunctor.h"
#include "MvrArgumentBuilder.h"
#include "MvrMutex.h"

#include "MvrGPSCoords.h" // for MvrLLACoords

#include <vector>

class MvrFileParser;

/// A map of a two-dimensional space the robot can navigate within, and which can be updated via the Mvria config
/**
* MvrMap contains data that represents the operating space of the robot, and can
* be used for space searching, localizing, navigating etc.  MobileRobots' ARNL
* and SONARNL localization and navigation libraries use MvrMap objects.
* MvrMap also provides methods to read and write the map from and to a file,
* along with a mechanism for setting the map file via the global Mvria config.
* 
* Types of data stored in a map include sensable obstacles (e.g. walls and 
* furniture in a room) that are represented either as a collection of data 
* points or lines.  The data points are similar to a raster or bit map and 
* are useful for high resolution sensors like lasers.  The lines create a 
* vector map that is useful for low resolution sensors like sonar.  For 
* advanced applications, the sensable obstacle data can be categorized according
* to the type of scan or sensor; see MvrMapScanInterface for more information.
* 
* In addition to the obstacle data, the Mvria map may contain goals, forbidden
* areas, and other points and regions of interest (a.k.a. "map objects").  
* Advanced applications can extend the set of predefined map object types.
* See @ref MapObjects for more information.
*
* If the application needs to be aware of any changes that are made to the 
* Mvria map at runtime, then it should install "mapChanged" callbacks on the map.
* If the map file is re-read while the robot is running, then the callbacks 
* are automatically invoked.  If the application makes other changes to the 
* map by calling any of the set methods, then it should call mapChanged() when
* it is finished.
* 
* See @ref MapFileFormat for the exact format of the actual .map file.
* 
* @section MapThreading Thread issuses, and changing the map
* 
* Different threads will need to access the same map data (for example ARNL, 
* MvrForbiddenRangeDevice, networking). However, the MvrMap class is not 
* inherently thread-safe.  You must call lock() and  unlock() methods, before 
* and after any access to the map data (e.g. calls to getMapObjects(), 
* getPoints(), setMapObjects(), setPoints()).  
* 
* If you are going to use setMapObjects(), setPoints(), setLines(), or
* setMapInfo(), then you should lock() the map beforehand, call the methods,
* then call mapChanged() to invoke the callbacks, and then finally 
* unlock() the map when done.  Note that mapChanged() will only invoke
* the callbacks if the data has actually changed.
* 
* However, there is an exception: the readFile() and writeFile() 
* methods @b do automatically lock the map while they read and write.
* 
* @section MapObjects Map Objects
* 
* In addition to lines and points, maps may contain "map objects", points
* or regions in space that have special meaning. 
*
* Certain types of objects are predefined for all maps.  These include
* Goal, GoalWithHeading, Dock, ForbiddenLine, ForbiddenArea, and RobotHome.  
* 
*  - Goal and GoalWithHeading are basically named MvrPoses, the difference being
*    that the "th" (heading) value is only valid for GoalWithHeading.
*  - Dock is an MvrPose that must always have a heading.
*  - ForbiddenLine is a boundary line, and ForbiddenArea is a rectangular
*    "sector". The extents of these objects are given as a pair of 
*    poses, a "from" point and a "to" point.
*  - RobotHome may be either an MvrPose or a rectangular sector.
* 
* Rectangular objects may also have an associated angle of rotation, which is 
* stored in the object pose theta value (MvrMapObject::getPose().getTh()).
* The actual global coordinates of the rectangle must be calculated
* using this angle and its "from-to" values. You can get a list of the 4
* MvrLineSegment objects that comprise the rectangle's edges using
* MvrMapObject::getFromToSegments(). If you want to do your own calculations,
* see MvrMapObject::ArMapObject().
*
* You can get a pointer to the current list of map objects with getMapObjects(),
* and directly modify the list.  You can also replace the current list of 
* map objects with a new one by calling setMapObjects(); this will destroy 
* the old list of map objects.  Call mapChanged() to notify other components 
* that the map has changed.
*
* In addition to the standard map object types,  is also possible to define 
* custom types of map objects using the "MapInfo" metadata section of the
* map file.  For example, if you wished to program some special behavior that 
* would only occur upon reaching certain goals, you could define a new 
* "SpecialGoal" type in the map file, and check for it in your program.  
* See @ref MapFileFormat for the syntax for defining new object types in the 
* map file.  Custom types will appear in Mapper3 and MobilePlanner in 
* drop-down menus, and instances of the custom types will be displayed in
* MobileEyes.  
*
* <i>Important Note</i>: if a map defines special GoalType or DockType items, 
* then it must define <b>all</b> possible goal or dock types, including the 
* default "Goal", "GoalWithHeading", and "Dock" types if you want those
* types to remain available.
*
 * @ingroup OptionalClasses
*/
class MvrMap : public MvrMapInterface
{
   
public:

  /// Constructor
  /**
   * @param baseDirectory the name of the directory in which to search for map
   * files that are not fully qualified
   * @param addToGlobalConfig a bool set to true if the map file name parameter 
   * should be added to the global config, Mvria::getConfig(); false, otherwise
   * @param configSection the char * name of the config section to which to 
   * add the map file name parameter name; applicable only if addToGlobalConfig 
   * is true
   * @param configParam the char * name of the parameter to be added to the 
   * specified configSection; applicable only if addToGlobalConfig is true
   * @param configDesc the char * description of the configParam; applicable
   * only if addToGlobalConfig is true
   * @param ignoreEmptyFileName a bool set to true if an empty file name is a 
   * valid config parameter value; set to false if a failure should be reported
   * when the file name is empty; applicable only if addToGlobalConfig is true
   * @param priority the MvrPriority::Priority of the config parameter; 
   * applicable only if addToGlobalConfig is true
   * @param tempDirectory the name of the directory in which to write temporary
   * files when saving a map; if NULL, then the map file is written directly.  
   * Note that using a temp file reduces the risk that the map will be corrupted
   * if the application crashes.
   * @param configProcessFilePriority priority at which MvrMap's configuration
   * parameters should be processed by MvrConfig.
  **/
  AREXPORT MvrMap(const char *baseDirectory = "./",
 		             bool addToGlobalConfig = true, 
 		             const char *configSection = "Files",
 		             const char *configParam = "Map",
 		             const char *configDesc = 
 		                  "Map of the environment that the robot uses for navigation",
 		             bool ignoreEmptyFileName = true,
                 MvrPriority::Priority priority = MvrPriority::IMPORTANT,
                 const char *tempDirectory = NULL,
		             int configProcessFilePriority = 100);

  /// Copy constructor
  AREXPORT MvrMap(const MvrMap &other);

  /// Assignment operator
  AREXPORT MvrMap &operator=(const MvrMap &other);

  /// Destructor
  AREXPORT virtual ~ArMap(void);


  // ===========================================================================
  // MvrMapInterface Methods
  // ===========================================================================

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Scan Types Methods 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   
  AREXPORT virtual std::list<std::string> getScanTypes() const;

  AREXPORT virtual bool setScanTypes(const std::list<std::string> &scanTypeList);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Locking / Mutex Methods 
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual int lock();

  AREXPORT virtual int tryLock();

  AREXPORT virtual int unlock();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MvrMapScanInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual const char *getDisplayString
                                 (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual std::vector<ArPose> *getPoints
                         (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual MvrPose getMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual MvrPose getMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual int getNumPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual bool isSortedPoints(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;

  AREXPORT virtual void setPoints(const std::vector<ArPose> *points,
                                  const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                  bool isSortedPoints = false,
                                  MvrMapChangeDetails *changeDetails = NULL);

  AREXPORT virtual std::vector<ArLineSegment> *getLines
                         (const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual MvrPose getLineMinPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual MvrPose getLineMaxPose(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual int getNumLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);
  AREXPORT virtual bool isSortedLines(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE) const;
   
  AREXPORT virtual void setLines(const std::vector<ArLineSegment> *lines,
                                 const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                 bool isSortedLines = false,
                                 MvrMapChangeDetails *changeDetails = NULL);
  
  AREXPORT virtual int getResolution(const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void setResolution(int resolution,
                                      const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                                      MvrMapChangeDetails *changeDetails = NULL);
 
  AREXPORT virtual void writeScanToFunctor
                              (MvrFunctor1<const char *> *functor, 
			                         const char *endOfLineChars,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE);

  AREXPORT virtual void writePointsToFunctor
                              (MvrFunctor2<int, std::vector<ArPose> *> *functor,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                               MvrFunctor1<const char *> *keywordFunctor = NULL);

   AREXPORT virtual void writeLinesToFunctor
 		                          (MvrFunctor2<int, std::vector<ArLineSegment> *> *functor,
                               const char *scanType = ARMAP_DEFAULT_SCAN_TYPE,
                               MvrFunctor1<const char *> *keywordFunctor = NULL);
  
   AREXPORT virtual bool addToFileParser(MvrFileParser *fileParser);

   AREXPORT virtual bool remFromFileParser(MvrFileParser *fileParser);


   AREXPORT virtual bool readDataPoint( char *line);

   AREXPORT virtual bool readLineSegment( char *line);
 
   /** Public for MvrQClientMapProducer **/
 
   AREXPORT virtual void loadDataPoint(double x, double y);
   AREXPORT virtual void loadLineSegment(double x1, double y1, double x2, double y2);
   
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MvrMapObjectsInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   AREXPORT virtual MvrMapObject *findFirstMapObject(const char *name, 
                                                    const char *type,
                                                    bool isIncludeWithHeading = false);
 
   AREXPORT virtual MvrMapObject *findMapObject(const char *name, 
 				                                       const char *type = NULL,
                                               bool isIncludeWithHeading = false);
 
   AREXPORT virtual std::list<ArMapObject *> findMapObjectsOfType
                                               (const char *type,
                                                bool isIncludeWithHeading = false);

   AREXPORT virtual std::list<ArMapObject *> *getMapObjects(void);
 
   AREXPORT virtual void setMapObjects(const std::list<ArMapObject *> *mapObjects,
                                       bool isSortedObjects = false,
                                       MvrMapChangeDetails *changeDetails = NULL); 
 
   AREXPORT virtual void writeObjectListToFunctor
                              (MvrFunctor1<const char *> *functor, 
 			                         const char *endOfLineChars);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MvrMapInfoInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(const char *infoName);

   AREXPORT virtual std::list<ArArgumentBuilder *> *getInfo(int infoType);

   AREXPORT virtual std::list<ArArgumentBuilder *> *getMapInfo(void);

   AREXPORT virtual int getInfoCount() const;

   AREXPORT virtual std::list<std::string> getInfoNames() const;

   AREXPORT virtual bool setInfo(const char *infoName,
 						                     const std::list<ArArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL); 

   AREXPORT virtual bool setInfo(int infoType,
 						                     const std::list<ArArgumentBuilder *> *infoList,
                                 MvrMapChangeDetails *changeDetails = NULL); 

   AREXPORT virtual bool setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                    MvrMapChangeDetails *changeDetails = NULL); 

   AREXPORT virtual const char *getInfoName(int infoType);

   AREXPORT virtual void writeInfoToFunctor(MvrFunctor1<const char *> *functor, 
 			                                      const char *endOfLineChars);
 

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MvrMapSupplementInterface
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual bool hasOriginLatLongAlt();
  bool hasOriginLatLonAlt() { return hasOriginLatLongAlt(); }
  AREXPORT virtual MvrPose getOriginLatLong();
  MvrPose getOriginLatLon() { return getOriginLatLong(); }
  AREXPORT virtual double getOriginAltitude();
  double getOriginLatitude() {
    return getOriginLatLong().getX();
  }
  double getOriginLongitude() {
    return getOriginLatLong().getY();
  }


  MvrLLACoords getOriginLLA() {
    return MvrLLACoords(getOriginLatLon(), getOriginAltitude());
  }


  AREXPORT virtual void setOriginLatLongAlt
                                        (bool hasOriginLatLong,
                                         const MvrPose &originLatLong,
                                         double altitude,
                                         MvrMapChangeDetails *changeDetails = NULL);

  void setOriginLatLonAlt
                                        (bool hasOriginLatLong,
                                         const MvrPose &originLatLong,
                                         double altitude,
                                         MvrMapChangeDetails *changeDetails = NULL)
  {
    setOriginLatLongAlt(hasOriginLatLong, originLatLong, altitude, changeDetails);
  }


  AREXPORT virtual void writeSupplementToFunctor(MvrFunctor1<const char *> *functor, 
			                                          const char *endOfLineChars);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Remaining MvrMapInterface Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void clear();

  AREXPORT virtual bool set(MvrMapInterface *other);

  AREXPORT virtual MvrMapInterface *clone();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Map Changed / Callback Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void mapChanged(void);

  AREXPORT virtual void addMapChangedCB(MvrFunctor *functor, int position = 50);

  AREXPORT virtual void remMapChangedCB(MvrFunctor *functor);

  AREXPORT virtual void addPreMapChangedCB(MvrFunctor *functor,
					   int position = 50);

  AREXPORT virtual void remPreMapChangedCB(MvrFunctor *functor);

  AREXPORT virtual void setMapChangedLogLevel(MvrLog::LogLevel level); 

  AREXPORT virtual MvrLog::LogLevel getMapChangedLogLevel(void); 


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Persistence
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual void writeToFunctor(MvrFunctor1<const char *> *functor, 
 			                                const char *endOfLineChars);

  AREXPORT virtual void writeObjectsToFunctor(MvrFunctor1<const char *> *functor, 
 			                                        const char *endOfLineChars,
                                              bool isOverrideAsSingleScan = false,
                                              const char *maxCategory = NULL);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // File I/O Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  AREXPORT virtual void addPreWriteFileCB(MvrFunctor *functor,
                                          MvrListPos::Pos position = MvrListPos::LAST);
  AREXPORT virtual void remPreWriteFileCB(MvrFunctor *functor);

  AREXPORT virtual void addPostWriteFileCB(MvrFunctor *functor,
                                           MvrListPos::Pos position = MvrListPos::LAST);
  AREXPORT virtual void remPostWriteFileCB(MvrFunctor *functor);

  /// Forces the map to reload if the config is changed/reloaded
  AREXPORT void forceMapLoadOnConfigProcessFile(void) 
    { myForceMapLoad = true; }

#ifndef SWIG
  /** @swigomit (can't write to arguments yet) */
  AREXPORT virtual bool readFile(const char *fileName, 
 			                           char *errorBuffer,
                                 size_t errorBufferLen,
                                 unsigned char *md5DigestBuffer,
                                 size_t md5DigestBufferLen);
  AREXPORT bool readFile(const char *fileName, char *errorBuffer, size_t errorBufferLen);
#endif
  AREXPORT bool readFile(const char *fileName);

#ifndef SWIG
  /** @swigomit (can't write to arguments yet) */
  AREXPORT virtual bool writeFile(const char *fileName, 
                                  bool internalCall,
                                  unsigned char *md5DigestBuffer = NULL,
                                  size_t md5DigestBufferLen = 0,
                                  time_t fileTimestamp = -1);
#endif

  virtual bool writeFile(const char *fileName) 
  {
    return writeFile(fileName, false, NULL, 0, -1);
  }

#ifndef SWIG
  /// @swigomit
  AREXPORT virtual struct stat getReadFileStat() const;
#endif

  AREXPORT virtual bool getMapId(MvrMapId *mapIdOut,
                                 bool isInternalCall = false);

  AREXPORT virtual bool calculateChecksum(unsigned char *md5DigestBuffer,
                                          size_t md5DigestBufferLen);


  AREXPORT virtual const char *getBaseDirectory(void) const;

  AREXPORT virtual void setBaseDirectory(const char *baseDirectory);

  AREXPORT virtual const char *getTempDirectory(void) const;

  AREXPORT virtual void setTempDirectory(const char *tempDirectory);


 	AREXPORT virtual std::string createRealFileName(const char *fileName);

  AREXPORT virtual const char *getFileName(void) const;

  AREXPORT virtual void setSourceFileName(const char *sourceName,
                                          const char *fileName,
                                          bool isInternalCall = false);

  AREXPORT virtual bool refresh();

  AREXPORT virtual void setIgnoreEmptyFileName(bool ignore);

  AREXPORT virtual bool getIgnoreEmptyFileName(void);

  AREXPORT virtual void setIgnoreCase(bool ignoreCase = false);

  AREXPORT virtual bool getIgnoreCase(void);


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Inactive Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual MvrMapInfoInterface *getInactiveInfo();

  AREXPORT virtual MvrMapObjectsInterface *getInactiveObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Child Objects Section
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  AREXPORT virtual MvrMapObjectsInterface *getChildObjects();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Miscellaneous
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  AREXPORT virtual MvrArgumentBuilder *findMapObjectParams
                                          (const char *mapObjectName);

  AREXPORT virtual bool setMapObjectParams(const char *mapObjectName,
                                           MvrArgumentBuilder *params,
                                           MvrMapChangeDetails  *changeDetails = NULL);


  AREXPORT virtual std::list<ArArgumentBuilder *> *getRemainder();

  AREXPORT virtual void setQuiet(bool isQuiet);
 	
  AREXPORT virtual bool parseLine(char *line);

  AREXPORT virtual void parsingComplete(void);

  AREXPORT virtual bool isLoadingDataStarted();

  AREXPORT virtual bool isLoadingLinesAndDataStarted();

  // ===========================================================================
  // End of MvrMapInterface
  // ===========================================================================



  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MvrMap Methods
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  /// Reads a map and changes the config map name to that file
  AREXPORT bool readFileAndChangeConfig(const char *fileName);
  /// Changes the config map name
  AREXPORT void changeConfigMapName(const char *fileName);



 protected:
 
   /// Processes changes to the Mvria configuration; loads a new map file if necessary
   bool processFile(char *errorBuffer, size_t errorBufferLen);
 
 protected:
 
   // Lock to protect data during file I/O operations
   MvrMutex myMutex;
 
   /// File path in which to find the map file name
   std::string myBaseDirectory;
   /// Name of the map file 
   std::string myFileName;
   /// File statistics for the map file
   struct stat myReadFileStat;

   /// Name of the Mvria config parameter that specifies the map file name
   std::string myConfigParam;
   /// Whether to ignore (not process) an empty Mvria config parameter
   bool myIgnoreEmptyFileName;
   /// Whether to ignore case when comparing map file names
   bool myIgnoreCase;
 
   /// Whether the Mvria config has already been processed at least once
   bool myConfigProcessedBefore;
   /// The name of the map file specified in the Mvria config parameter
   char myConfigMapName[MAX_MAP_NAME_LENGTH];
   /// Whether we want to force loading the map for some reasing
   bool myForceMapLoad;
 
   /// The current map used by the robot
   MvrMapSimple * const myCurrentMap;
   /// The map that is being loaded, i.e. read from a file; will be copied to the current map if successful
   MvrMapSimple * myLoadingMap;
   
   /// Whether to run in "quiet mode", i.e. logging less information
   bool myIsQuiet;
  
   /// Callback that processes changes to the Mvria config.
   MvrRetFunctor2C<bool, MvrMap, char *, size_t> myProcessFileCB;
 
}; // end class MvrMap

 
#endif // ARMAP_H
 
 
