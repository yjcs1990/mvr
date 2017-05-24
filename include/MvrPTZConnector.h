/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrPTZConnector.h
 > Description  : Factory for creating and configuring interfaces for pan/tilt units or camera
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRPTZCONNECTOR_H
#define MVRPTZCONNECTOR_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"
#include "MvrRobotParams.h"
#include "mvriaInternal.h"

#include <string>
#include <vector>

class MvrArgumentParser;
class MvrPTZ;
class MvrRobot;

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
   * ARIA. MvrVideo::init() registers new PTZ types implemented in the MvrVideo
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

#endif  // MVRPTZCONNECTOR_H