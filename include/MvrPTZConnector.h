#ifndef MVRPTZCONNECTOR_H
#define MVRPTZCONNECTOR_H

#include <string>
#include <vector>

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"
#include "MvrRobotParams.h"
#include "mvriaInternal.h"

class MvrArgumentParser;
class MvrPTZ;
class MvrRobot;

/** 
 *  @brief Factory for creating and configuring interfaces for pan/tilt units or camera
 *  pan/tilt/zoom control based on robot parameter file and command-line arguments.
 *
 *  First, create an MvrPTZConnector object before 
 *  calling Mvria::parseArgs().  After connecting to the robot, call
 *  Mvria::parseArgs() to check arguments and parameters from the parameter
 *  file(s).  Then, call connect() to connect to all configured and enabled PTZs. To get
 *  access to a PTZ objects, use getPTZ(int i) and getNumPTZs(), or getPTZs().
 *  MvrPTZ provides an interface to functions that most PTZ implementations
 *  (MvrPTZ subclasses) implement. Some PTZ implementations have additional
 *  features. Use those subclasse directly to use these additional features
 *  (use dynamic_cast to cast an MvrPTZ pointer to a subclass pointer, if
 *  possible.)
 *  
 *  MvrPTZConnector has built in support for all the PTZ types with support
 *  included in the ARIA library, and other libraries or programs may register 
 *  new types as well. (For example, the ArVideo library contains support for
 *  additional camera PTZ types.)
 *
 * The following command-line arguments are checked:
 * @verbinclude MvrPTZConnector_options
 *
 * PTZs are 0-indexed internally in this class, however, argument names,
 * parameter names and log messages displayed to users are 1-indexed. 

  @ingroup OptionalClasses
  @ingroup DeviceClasses
  @since 2.7.6
*/

class MvrPTZConnector {
public:
  /* @arg robot In some cases the robot connection is used to communicate with
   * devices via auxilliary serial connections, so this robot interface is used.
   * May be NULL. */
  MVREXPORT MvrPTZConnector(MvrArgumentParser* argParser, MvrRobot *robot = NULL);
  MVREXPORT ~MvrPTZConnector();

  /** For each PTZ specified in program arguments, and in robot parameters with
   *  PTZAutoConnect set to true, create the
   * appropriate PTZ object (based on type name) and connect to it. 
    @return false on first error, true if all PTZ connections were successful.
  */
  MVREXPORT bool connect();

  /** @copydoc connect() */
  bool connectPTZs() { return connect(); }

  /** @return number of PTZ objects that were created and connected to. */
  size_t getNumPTZs() const { return myConnectedPTZs.size(); }
  
  /** @return a specific PTZ object that was connected by connect() or
   * connectPTZs(), or NULL if none exists.  These are 0-indexed. */
  MvrPTZ* getPTZ(size_t i = 0) const
  {
    if(i >= myConnectedPTZs.size()) return NULL;
    return myConnectedPTZs[i];
  }
  
  /** @copydoc getPTZ(size_t) */
  MvrPTZ* findPTZ(size_t i) const { return getPTZ(i); }

  /** @return list of connected PTZs. 
      0-indexed.  Pointers in the vector may be NULL if no parameters or command
      line arguments were given for them or that PTZ was disabled. These should be
      skipped when iterating on this list and not accessed.
  */
  std::vector<MvrPTZ*> getPTZs() const {
    return myConnectedPTZs;
  }



  /** Arguments passed to function are PTZ index, parameters, parser (may be
     * null) and robot object (may be null) */
  typedef MvrRetFunctor4<MvrPTZ*, size_t, MvrPTZParams, MvrArgumentParser*, MvrRobot*> PTZCreateFunc;  
  typedef MvrGlobalRetFunctor4<MvrPTZ*, size_t, MvrPTZParams, MvrArgumentParser*, MvrRobot*> GlobalPTZCreateFunc;  

  /** Register a new PTZ type. Mvria::init() registers PTZ types built in to
   * ARIA. ArVideo::init() registers new PTZ types implemented in the ArVideo
   * library. You may also add any new PTZ types you create. 
  */
  MVREXPORT static void registerPTZType(const std::string& typeName, MvrPTZConnector::PTZCreateFunc* func);

  /** Change limit on number of PTZ devices. 
 * You must call this
 * before creating an MvrPTZConnector, parsing command line arguments, connecting
 * to a robot or loading a parameter file, or using MvrPTZconnecor to connect to
 * PTZ devices. 
  */
  void setMaxNumPTZs(size_t n)
  {
    Mvria::setMaxNumPTZs(n);
  }

  size_t getMaxNumPTZs()
  {
    return Mvria::getMaxNumPTZs();
  }

  /** Return robot that PTZs are or will be attached to (may be NULL) */
  MvrRobot *getRobot() { return myRobot; }
  
protected:
  bool parseArgs();
  bool parseArgs(MvrArgumentParser *parser);
  bool parseArgsFor(MvrArgumentParser *parser, int which);
  MVREXPORT void logOptions() const;
  void populateRobotParams(MvrRobotParams *params);

  MvrArgumentParser *myArgParser;
  MvrRobot *myRobot;
  MvrRetFunctorC<bool, MvrPTZConnector> myParseArgsCallback;
  MvrConstFunctorC<MvrPTZConnector> myLogOptionsCallback;
  static std::map< std::string, PTZCreateFunc* > ourPTZCreateFuncs;
  MvrFunctor1C<MvrPTZConnector, MvrRobotParams*> myPopulateRobotParamsCB;
  std::vector<MvrPTZParams> myParams;  ///< copied from MvrRobotParams (via myRobot), then in connect() parameters given from command line arguments (myArguments) are merged in.
  std::vector<MvrPTZParams> myArguments; ///< from program command-line options (via the parseArgs() callback called by Mvria::parseArgs())
  //std::vector<MvrPTZParams> myConfiguredPTZs;  ///< if connecting to a PTZ, its parameters are copied here from myParams and myArguments
  //static size_t ourMaxNumPTZs;
  std::vector<MvrPTZ*> myConnectedPTZs;  ///< MvrPTZ objects created and initialized in connect().
};


#endif  // ifdef ARPTZCONNECTOR_H


