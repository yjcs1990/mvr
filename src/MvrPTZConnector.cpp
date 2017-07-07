#include <string>
#include <vector>
#include <sstream>
#include "MvrExport.h"
#include "mvriaUtil.h"
#include "mvriaInternal.h"
#include "MvrLog.h"
#include "MvrFunctor.h"
#include "MvrPTZ.h"
#include "MvrPTZConnector.h"
#include "MvrConfig.h"
#include "MvrRobot.h"
#include "MvrRobotParams.h"

std::map<std::string, MvrPTZConnector::PTZCreateFunc*> MvrPTZConnector::ourPTZCreateFuncs;


MVREXPORT MvrPTZConnector::MvrPTZConnector(MvrArgumentParser* argParser, MvrRobot *robot) :
  myArgParser(argParser),
  myRobot(robot),
  myParseArgsCallback(this, &MvrPTZConnector::parseArgs),
  myLogOptionsCallback(this, &MvrPTZConnector::logOptions)
  //myPopulateRobotParamsCB(this, &MvrPTZConnector::populateRobotParams)
{
  myParseArgsCallback.setName("MvrPTZConnector parse args callback");
  myLogOptionsCallback.setName("MvrPTZConnector log options callback");
  Mvria::addParseArgsCB(&myParseArgsCallback);
  Mvria::addLogOptionsCB(&myLogOptionsCallback);
  //MvrRobotParams::addPopulateParamsCB(&myPopulateRobotParamsCB);
}
  

MVREXPORT MvrPTZConnector::~MvrPTZConnector()
{
  ///@todo not in Mvr but should be: Mvria::remParseArgsCB(&myParseArgsCallback);
  ///@todo not in Mvr but should be: Mvria::remLogOptionsCB(&myLogOptionsCallback);
  ///@todo delete objects from myConnectedPTZs
  ///@todo depopulate parameter slots from any MvrRobotParams object that called our populate callback.
//  MvrRobotParams::remPopulateParamsCB(&myPopulateRobotParamsCB);
}

MVREXPORT bool MvrPTZConnector::connect()
{
  // Copy MvrRobot's default parameters:
  myParams.resize(Mvria::getMaxNumPTZs());
  if(myRobot)
  {
	  if(myRobot->getRobotParams())
		 myParams = myRobot->getRobotParams()->getPTZParams();
	  else
		  MvrLog::log(MvrLog::Normal, "MvrPTZConnector: Warning: no robot parameters, cannot set defaults for the robot type or loaded from parameter file. (To do so, connect to robot before connecting to PTZs.)");
  }
  else
  {
	  MvrLog::log(MvrLog::Normal, "MvrPTZConnector: Warning: cannot use defaults for specific robot type or get configuration from robot parameter file, no robot connection.");
  }

  // "arguments" are from program command line. "parameters"/"params" are from
  // robot parameter file(s) or ARIA's internal defaults (MvrRobotTypes.cpp)
  myConnectedPTZs.reserve(myParams.size());  // index in myConnectedPTZs corresponds to index from parameters and command line options, but the MvrPTZ* may be NULL if not connected.
  size_t i = 0;
  size_t picount = 0;
  //assert(myArguments.size() >= myParams.size());
  for(std::vector<MvrPTZParams>::iterator pi = myParams.begin(); 
      pi != myParams.end(); 
      ++pi, ++i, ++picount)
  {
  //printf("]] myArguments.size is %d, i is %d\n", myArguments.size(), i);
    if(i < myArguments.size())
    {
//      printf("]] merging myArguments[%d] into myParams[%d]\n", i, picount);
      pi->merge(myArguments[i]); 
    }

    if(pi->type == "" || pi->type == "none" || pi->connect == false)   // this ptz # was not specified in any parameters, or it was but with false connect flag
    {
        //puts("null type or \"none\" type or false connect flag, not creating or connecting.");
        //printf("connectflag=%d\n", pi->connect);
      //myConnectedPTZs.push_back(NULL);
      continue;
    }
    MvrLog::log(MvrLog::Normal, "MvrPTZConnector: Connecting to PTZ #%d (type %s)...", i+1, pi->type.c_str());
    if(ourPTZCreateFuncs.find(pi->type) == ourPTZCreateFuncs.end())
    {
      MvrLog::log(MvrLog::Terse, "MvrPTZConnector: Error: unrecognized PTZ type \"%s\" for PTZ #%d", pi->type.c_str(), i+1);
      //myConnectedPTZs.push_back(NULL);
      return false;
    }
    PTZCreateFunc *func = ourPTZCreateFuncs[pi->type];
    MvrPTZ *ptz = func->invokeR(i, *pi, myArgParser, myRobot);
    if(!ptz)
    {
      MvrLog::log(MvrLog::Terse, "MvrPTZConnector: Error connecting to PTZ #%d (type %s).", i+1, pi->type.c_str());
      MvrLog::log(MvrLog::Normal, "MvrPTZConnector: Try specifying -ptzType (and -ptzSerialPort, -ptzRobotAuxSerialPort or -ptzAddress) program arguments, or set type and connection options in your robot's parameter file. Run with -help for all connection program options.");
      //myConnectedPTZs.push_back(NULL);
      return false;
    }
    if(pi->serialPort != "" && pi->serialPort != "none")
    {
      // memory leak? who is responsible for destroying serial connection, do we
      // need to store it and destroy it in our destructor or a disconnectAll
      // function?
      std::string logname = pi->type;
      logname += " ";
      logname += i;
      logname += " control serial connection on ";
      logname += pi->serialPort;
      MvrDeviceConnection *serCon = MvrDeviceConnectionCreatorHelper::createSerialConnection(pi->serialPort.c_str(), NULL, logname.c_str());
      ptz->setDeviceConnection(serCon);
    }
    else if(pi->robotAuxPort != -1)
    {
      ptz->setAuxPort(pi->robotAuxPort);
    }

    ptz->setInverted(pi->inverted);

    if(ptz->init())
      MvrLog::log(MvrLog::Verbose, "MvrPTZConnector: Sucessfully initialized %s PTZ #%d ", ptz->getTypeName(), i+1);
    else
      MvrLog::log(MvrLog::Normal, "MvrPTZConnector: Warning: Error initializing PTZ #%d (%s)", i+1, ptz->getTypeName());
  
	// Resize ConnectedPTZs vector so that we can place this framegrabber at its correct index, even if any previous 
	// PTZs were not stored because of errors creating them or they are not present in parameters or program options.
	// Any new elements created here are set to NULL.
	myConnectedPTZs.resize(i+1, NULL);

	// Add this PTZ to the connected list at its proper index (matching its index in parameters or program options)
    myConnectedPTZs[i] = ptz;

  }
  return true;
}

bool MvrPTZConnector::parseArgs() 
{
  if(!myArgParser) return false;
  return parseArgs(myArgParser);
}

bool MvrPTZConnector::parseArgs(MvrArgumentParser *parser)
{
  // Store command-line arguments in myArguments. They will be merged into
  // parameters in connect().

//  puts("]]] MvrPTZConnector::parseArgs");
  if(!parser) return false;
  // -1 is a special case, checks for arguments implied for first PTZ, e.g.
  // -ptzType is the same as -ptz1Type. 
  // Then proceeds normally with 0 for the first ptz (-ptz1...).
  for(int i = -1; i < (int)getMaxNumPTZs(); ++i)
  {
    //printf("parse args for %d\n", i);
    if(!parseArgsFor(parser, i))
      return false;
  }
  return true;
}

bool MvrPTZConnector::parseArgsFor(MvrArgumentParser *parser, int which)
{
  MvrPTZParams newargs;
  const char *type = NULL;
  const char *serialPort = NULL;
  int auxPort = -1;
  const char *address = NULL;
  int tcpPort = -1;
  bool inverted = false;

  std::stringstream prefixconcat;
  prefixconcat << "-ptz";

  // If which is -1 then we check for just -ptz... with no number, but it is
  // same as first ptz ("-ptz1...") 
  if(which >= 0)
    prefixconcat << which+1;
  if(which < 0)
    which = 0;

  std::string prefix = prefixconcat.str();
  //printf("checking for arguments for PTZ #%lu with prefix %s (e.g. %s)\n", which, prefix.c_str(), (prefix+"Type").c_str());

  if(parser->checkParameterArgumentString( (prefix+"Type").c_str(), &type) && type != NULL)
  {
    if(ourPTZCreateFuncs.find(type) == ourPTZCreateFuncs.end())
    {
      MvrLog::log(MvrLog::Terse, "MvrPTZConnector: Error parsing arguments: unrecognized PTZ type \"%s\" given with %sType.", type, prefix.c_str());
      return false;
    }
    newargs.type = type;
    newargs.connect = true;
    newargs.connectSet = true;
    //printf("found command line argument %sType. set type to %s and set connect and connectSet to true for ptz %d.\n", prefix.c_str(), type, which);
  }

  size_t nConOpt = 0;

  if(parser->checkParameterArgumentString( (prefix+"SerialPort").c_str(), &serialPort) && serialPort != NULL) 
  {
    ++nConOpt;
    newargs.serialPort = serialPort;
    //printf("got serial port %s\n", serialPort);
  }

  if(parser->checkParameterArgumentInteger( (prefix+"RobotAuxSerialPort").c_str(), &auxPort) && auxPort != -1)
  {
     ++nConOpt;
    newargs.robotAuxPort = auxPort;
    //printf("got aux port %d\n", auxPort);
  }

  if(parser->checkParameterArgumentString( (prefix+"Address").c_str(), &address) && address != NULL) 
  {
    ++nConOpt;
    newargs.address = address;
    //printf("got address %s\n", address);
  }

  // only one of serial port, aux port or address can be given
  if(nConOpt > 1)
  {
    MvrLog::log(MvrLog::Terse, "MvrPTZConnector: Error: Only one of %sSerialPort, %sRobotAuxSerialPort or %sAddress may be given to select connection.",
      prefix.c_str(),
      prefix.c_str(),
      prefix.c_str()
    );
    return false;
  }

  if(parser->checkParameterArgumentInteger( (prefix+"TCPPort").c_str(), &tcpPort) && tcpPort != -1)
  {
    newargs.tcpPort = tcpPort;
    newargs.tcpPortSet = true;
    //printf("got tcp port %d\n", tcpPort);
  }

  bool wasSet = false;
  if(parser->checkArgument((prefix+"Inverted").c_str()))
  {
    newargs.inverted = true;
    newargs.invertedSet = true;
    //printf("got inverted %d\n", inverted);
  }
  if(parser->checkParameterArgumentBool( (prefix+"Inverted").c_str(), &inverted, &wasSet) && wasSet)
  {
    newargs.inverted = inverted;
    newargs.invertedSet = true;
    //printf("got inverted %d\n", inverted);
  }
  
  if((size_t)which >= myArguments.size())
  {
    // try not to assume we will be called with regular increasing which parameters,
    // (though we probably will be) so resize and set rather than use push_back:
    //printf("setting new, resized myArguments[%d] to these new params.\n", which);
    myArguments.resize(which+1);
  }

  // merge these rather than replacing them -- don't replace items in existing
  // myArguments[which] if not set in newargs.
  //myArguments[which] = newargs;
  myArguments[which].merge(newargs);

  return true;
}

MVREXPORT void MvrPTZConnector::logOptions() const
{
  MvrLog::log(MvrLog::Terse, "Common PTU and Camera PTZ options:\n");
  MvrLog::log(MvrLog::Terse, "\t-ptzType <type>\tSelect PTZ/PTU type. Required.  Available types are:");
  for(std::map<std::string, PTZCreateFunc*>::const_iterator i = ourPTZCreateFuncs.begin();
      i != ourPTZCreateFuncs.end();
      ++i)
  {
    MvrLog::log(MvrLog::Terse, "\t\t%s", (*i).first.c_str());
  }
  MvrLog::log(MvrLog::Terse, "\t\tnone");
  MvrLog::log(MvrLog::Terse, "\t-ptzInverted <true|false>\tIf true, reverse tilt and pan axes for cameras mounted upside down.");
  MvrLog::log(MvrLog::Terse, "\nOnly one of the following sets of connection parameters may be given:");
  MvrLog::log(MvrLog::Terse, "\nFor computer serial port connections:");
  MvrLog::log(MvrLog::Terse, "\t-ptzSerialPort <port>\tSerial port name.");
  MvrLog::log(MvrLog::Terse, "\nFor Pioneer robot auxilliary serial port connections:");
  MvrLog::log(MvrLog::Terse, "\t-ptzRobotAuxSerialPort <1|2|3>\tUse specified Pioneer robot auxilliary serial port.");
  MvrLog::log(MvrLog::Terse, "\nFor network connections:");
  MvrLog::log(MvrLog::Terse, "\t-ptzAddress <address>\tNetwork address or hostname for network connection.");
  MvrLog::log(MvrLog::Terse, "\t-ptzTcpPort <port>\tTCP port number for network connections.");
  MvrLog::log(MvrLog::Terse, "\nParameters for multiple cameras/units may be given like: -ptz1Type, -ptz2Type, -ptz3Type, etc.");
  MvrLog::log(MvrLog::Terse, "Some PTZ/PTU types may accept additional type-specific options. Refer to option documentation text specific to those types.");
}






  MVREXPORT  void MvrPTZConnector::registerPTZType(const std::string& typeName, MvrPTZConnector::PTZCreateFunc* func)
  {
    ourPTZCreateFuncs[typeName] = func;
  }
