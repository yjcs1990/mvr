#include "MvrExport.h"


#include "mvriaOSDef.h"
#include "MvrRobotParams.h"
#include "mvriaInternal.h"
#include <sstream>

bool MvrRobotParams::ourUseDefaultBehavior = true;
std::string ourPowerOutputDisplayHint;

void MvrRobotParams::internalSetUseDefaultBehavior(bool useDefaultBehavior,
    const char *powerOutputDisplayHint)
{
  ourUseDefaultBehavior = useDefaultBehavior;
  ourPowerOutputDisplayHint = powerOutputDisplayHint;

  MvrLog::log(MvrLog::Normal, 
	     "MvrRobotParams: Setting use default behavior to %s, with power output display hint '%s'", 
	     MvrUtil::convertBool(ourUseDefaultBehavior),
	     ourPowerOutputDisplayHint.c_str());
}

bool MvrRobotParams::internalGetUseDefaultBehavior(void)
{
  return ourUseDefaultBehavior;
}

MVREXPORT MvrRobotParams::MvrRobotParams() :
  MvrConfig(NULL, true),
  mySonarUnitGetFunctor(this, &MvrRobotParams::getSonarUnits),
  mySonarUnitSetFunctor(this, &MvrRobotParams::parseSonarUnit),
  myIRUnitGetFunctor(this, &MvrRobotParams::getIRUnits),
  myIRUnitSetFunctor(this, &MvrRobotParams::parseIRUnit),
  myCommercialProcessFileCB(this, &MvrRobotParams::commercialProcessFile)
{
  myCommercialConfig = NULL;

  sprintf(myClass, "Pioneer");
  mySubClass[0] = '\0';
  myRobotRadius = 250;
  myRobotDiagonal = 120;
  myRobotWidth = 400;
  myRobotLength = 500; 
  myRobotLengthFront = 0; 
  myRobotLengthRear = 0; 
  myHolonomic = true;
  myAbsoluteMaxVelocity = 0;
  myAbsoluteMaxRVelocity = 0;
  myHaveMoveCommand = true;
  myAngleConvFactor = 0.001534;
  myDistConvFactor = 1.0;
  myVelConvFactor = 1.0;
  myRangeConvFactor = 1.0;
  myVel2Divisor = 20;
  myNumSonar = 0;
  myGyroScaler = 1.626;
  myTableSensingIR = false;
  myNewTableSensingIR = false;
  myFrontBumpers = false;
  myNumFrontBumpers = 5;
  myRearBumpers = false;
  myNumRearBumpers = 5;
  myNumSonarUnits = 0;
  // MPL TODO why do we need these counts?
  mySonarBoardCount = 0;
  myBatteryMTXBoardCount = 0;
  myLCDMTXBoardCount = 0;
  mySonarMTXBoardCount = 0;

  mySonarMap.clear();

  myNumIR = 0;
  myIRMap.clear();

  
  myRequestIOPackets = false;
  myRequestEncoderPackets = false;
  mySwitchToBaudRate = 38400;

  mySettableVelMaxes = true;
  myTransVelMax = 0;
  myRotVelMax = 0;

  mySettableAccsDecs = true;
  myTransAccel = 0;
  myTransDecel = 0;
  myRotAccel = 0;
  myRotDecel = 0;

  myHasLatVel = false;
  myLatVelMax = 0;
  myLatAccel = 0;
  myLatDecel = 0;
  myAbsoluteMaxLatVelocity = 0;

  myGPSX = 0;
  myGPSY = 0;
  strcpy(myGPSPort, "COM2");
  strcpy(myGPSType, "standard");
  myGPSBaud = 9600;

  //strcpy(mySonarPort, "COM2");
  //strcpy(mySonarType, "standard");
  //mySonarBaud = 115200;
			
  //strcpy(myBatteryMTXBoardPort, "COM1");
  //strcpy(myBatteryMTXBoardType, "mtxbatteryv1");
  //myBatteryMTXBoardBaud = 115200;

  strcpy(myCompassType, "robot");
  strcpy(myCompassPort, "");

  if (ourUseDefaultBehavior)
    internalAddToConfigDefault();
}

MVREXPORT MvrRobotParams::~MvrRobotParams()
{

}


void MvrRobotParams::internalAddToConfigDefault(void)
{
  addComment("Robot parameter file");
//  addComment("");
  //addComment("General settings");
  std::string section;
  section = "General settings";
  addParam(MvrConfigArg("Class", myClass, "general type of robot", 
		 sizeof(myClass)), section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("Subclass", mySubClass, "specific type of robot", 
		       sizeof(mySubClass)), section.c_str(), 
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RobotRadius", &myRobotRadius, "radius in mm"), 
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotDiagonal", &myRobotDiagonal, 
		 "half-height to diagonal of octagon"), "General settings",
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RobotWidth", &myRobotWidth, "width in mm"), 
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLength", &myRobotLength, "length in mm of the whole robot"),
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLengthFront", &myRobotLengthFront, "length in mm to the front of the robot (if this is 0 (or non existent) this value will be set to half of RobotLength)"),
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLengthRear", &myRobotLengthRear, "length in mm to the rear of the robot (if this is 0 (or non existent) this value will be set to half of RobotLength)"), 
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("Holonomic", &myHolonomic, "turns in own radius"), 
	   section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("MaxRVelocity", &myAbsoluteMaxRVelocity, 
		       "absolute maximum degrees / sec"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("MaxVelocity", &myAbsoluteMaxVelocity, 
		 "absolute maximum mm / sec"), section.c_str(), 
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("MaxLatVelocity", &myAbsoluteMaxLatVelocity, 
		 "absolute lateral maximum mm / sec"), section.c_str(), 
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("HasMoveCommand", &myHaveMoveCommand, 
		 "has built in move command"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RequestIOPackets", &myRequestIOPackets,
		 "automatically request IO packets"), section.c_str(),
	   MvrPriority::NORMAL);
  addParam(MvrConfigArg("RequestEncoderPackets", &myRequestEncoderPackets,
		       "automatically request encoder packets"), 
	   section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("SwitchToBaudRate", &mySwitchToBaudRate, 
		 "switch to this baud if non-0 and supported on robot"), 
	   section.c_str(), MvrPriority::IMPORTANT);
  
  section = "Conversion factors";
  addParam(MvrConfigArg("AngleConvFactor", &myAngleConvFactor,
		     "radians per angular unit (2PI/4096)"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("DistConvFactor", &myDistConvFactor,
		       "multiplier to mm from robot units"), section.c_str(),
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("VelConvFactor", &myVelConvFactor,
		     "multiplier to mm/sec from robot units"), 
	   section.c_str(),
	   MvrPriority::NORMAL);
  addParam(MvrConfigArg("RangeConvFactor", &myRangeConvFactor, 
		       "multiplier to mm from sonar units"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("DiffConvFactor", &myDiffConvFactor, 
		     "ratio of angular velocity to wheel velocity (unused in newer firmware that calculates and returns this)"), 
	   section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("Vel2Divisor", &myVel2Divisor, 
		       "divisor for VEL2 commands"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("GyroScaler", &myGyroScaler, 
		     "Scaling factor for gyro readings"), section.c_str(),
	   MvrPriority::IMPORTANT);

  section = "Accessories the robot has";
  addParam(MvrConfigArg("TableSensingIR", &myTableSensingIR,
		       "if robot has upwards facing table sensing IR"), 
	   section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("NewTableSensingIR", &myNewTableSensingIR,
		 "if table sensing IR are sent in IO packet"), 
	   section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("FrontBumpers", &myFrontBumpers, 
		 "if robot has a front bump ring"), section.c_str(),
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("NumFrontBumpers", &myNumFrontBumpers,
		     "number of front bumpers on the robot"), 
	   section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RearBumpers", &myRearBumpers,
		       "if the robot has a rear bump ring"), section.c_str(),
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("NumRearBumpers", &myNumRearBumpers,
		       "number of rear bumpers on the robot"), section.c_str(),
	   MvrPriority::TRIVIAL);

  section = "IR parameters";
  addParam(MvrConfigArg("IRNum", &myNumIR, "number of IRs on the robot"), section.c_str(), MvrPriority::NORMAL);
   addParam(MvrConfigArg("IRUnit", &myIRUnitSetFunctor, &myIRUnitGetFunctor,
			"IRUnit <IR Number> <IR Type> <Persistance, cycles> <x position, mm> <y position, mm>"), 
	    section.c_str(), MvrPriority::TRIVIAL);


    
  section = "Movement control parameters";
  setSectionComment(section.c_str(), "if these are 0 the parameters from robot flash will be used, otherwise these values will be used");
  addParam(MvrConfigArg("SettableVelMaxes", &mySettableVelMaxes, "if TransVelMax and RotVelMax can be set"), section.c_str(),
	   MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("TransVelMax", &myTransVelMax, "maximum desired translational velocity for the robot"), section.c_str(), 
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("RotVelMax", &myRotVelMax, "maximum desired rotational velocity for the robot"), section.c_str(),
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("SettableAccsDecs", &mySettableAccsDecs, "if the accel and decel parameters can be set"), section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("TransAccel", &myTransAccel, "translational acceleration"), 
	   section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("TransDecel", &myTransDecel, "translational deceleration"), 
	   section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("RotAccel", &myRotAccel, "rotational acceleration"), 
	   section.c_str());
  addParam(MvrConfigArg("RotDecel", &myRotDecel, "rotational deceleration"),
	   section.c_str(), MvrPriority::IMPORTANT);

  addParam(MvrConfigArg("HasLatVel", &myHasLatVel, "if the robot has lateral velocity"), section.c_str(), MvrPriority::TRIVIAL);

  addParam(MvrConfigArg("LatVelMax", &myLatVelMax, "maximum desired lateral velocity for the robot"), section.c_str(), 
	   MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("LatAccel", &myLatAccel, "lateral acceleration"), 
	   section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("LatDecel", &myLatDecel, "lateral deceleration"), 
	   section.c_str(), MvrPriority::IMPORTANT);

  section = "GPS parameters";
    // Yes, there is a "P" in the middle of the position parameters. Don't remove it if
    // you think it's irrelevant, it will break all robot parameter files.
  addParam(MvrConfigArg("GPSPX", &myGPSX, "x location of gps receiver antenna on robot, mm"), section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("GPSPY", &myGPSY, "y location of gps receiver antenna on robot, mm"), section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("GPSType", myGPSType, "type of gps receiver (trimble, novatel, standard)", sizeof(myGPSType)), section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("GPSPort", myGPSPort, "port the gps is on", sizeof(myGPSPort)), section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("GPSBaud", &myGPSBaud, "gps baud rate (9600, 19200, 38400, etc.)"), section.c_str(), MvrPriority::NORMAL);

  section = "Compass parameters";
  addParam(MvrConfigArg("CompassType", myCompassType, "type of compass: robot (typical configuration), or serialTCM (computer serial port)", sizeof(myCompassType)), section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("CompassPort", myCompassPort, "serial port name, if CompassType is serialTCM", sizeof(myCompassPort)), section.c_str(), MvrPriority::NORMAL);


  section = "Sonar parameters";
  addParam(MvrConfigArg("SonarNum", &myNumSonar, 
		     "Number of sonars on the robot."), section.c_str(),
	   MvrPriority::NORMAL);

  addParam(MvrConfigArg("SonarUnit", &mySonarUnitSetFunctor, &mySonarUnitGetFunctor,
		       "SonarUnit <sonarNumber> <x position, mm> <y position, mm> <heading of disc, degrees> <MTX sonar board> <MTX sonar board unit position> <MTX gain> <MTX detection threshold> <MTX max range> <autonomous driving sensor flag>"), section.c_str(), MvrPriority::TRIVIAL);

  int i;
  for (i = 1; i <= Mvria::getMaxNumSonarBoards(); i++)
    addSonarBoardToConfig(i, this, ourUseDefaultBehavior);

  for (i = 1; i <= Mvria::getMaxNumLasers(); i++)
  {
    if (i == 1)
    {
      section = "Laser parameters";
    }
    else
    {
      char buf[1024];
      sprintf(buf, "Laser %d parameters", i);
      section = buf;
    }

    addLaserToConfig(i, this, ourUseDefaultBehavior, section.c_str());
  }

  for (i = 1; i <= Mvria::getMaxNumBatteries(); i++)
    addBatteryToConfig(i, this, ourUseDefaultBehavior);

  for (i = 1; i <= Mvria::getMaxNumLCDs(); i++)
    addLCDToConfig(i, this, ourUseDefaultBehavior);

  /** PTZ parameters here too */
  myPTZParams.resize(Mvria::getMaxNumPTZs());
  for(size_t i = 0; i < Mvria::getMaxNumPTZs(); ++i)
    addPTZToConfig(i, this);

  /* Parameters used by MvrVideo library */
  myVideoParams.resize(Mvria::getMaxNumVideoDevices());
  for(size_t i = 0; i < Mvria::getMaxNumVideoDevices(); ++i)
    addVideoToConfig(i, this);
}



MVREXPORT void MvrRobotParams::addLaserToConfig(
	int laserNumber, MvrConfig* config, bool useDefaultBehavior, 
	const char *section)
{
  MvrConfigArg::RestartLevel restartLevel;
  // if we're using default behavior set it to none, since we can't
  // change it
  if (useDefaultBehavior)
    restartLevel = MvrConfigArg::NO_RESTART;
  // otherwise make it restart the software
  else 
    restartLevel = MvrConfigArg::RESTART_SOFTWARE;

  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section,
                     "Information about the connection to this laser and its position on the vehicle.");

  LaserData *laserData = new LaserData;
  myLasers[laserNumber] = laserData;

  strcpy(laserData->mySection, section);

  std::string displayHintPlain = "Visible:LaserAutoConnect=true";
  std::string displayHintCheckbox = displayHintPlain + "&&Checkbox";

  std::string displayHintCustom;

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "Laser_%d exists and should be automatically connected at startup.",
           laserNumber);
       
  displayHintCustom = "Checkbox";

  config->addParam(
	  MvrConfigArg("LaserAutoConnect", &laserData->myLaserAutoConnect,
                tempDescBuf),
	  section, MvrPriority::FACTORY, 
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section, MvrPriority::FACTORY);

  config->addParam(
	  MvrConfigArg("LaserX", &laserData->myLaserX, 
		      "Location (in mm) of the laser in X (+ front, - back) relative to the robot's idealized center of rotation."), 
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("LaserY", &laserData->myLaserY, 
		      "Location (in mm) of the laser in Y (+ left, - right) relative to the robot's idealized center of rotation."), 
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("LaserTh", &laserData->myLaserTh, 
		      "Rotation (in deg) of the laser (+ counterclockwise, - clockwise).", -180.0, 180.0), 
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("LaserZ", &laserData->myLaserZ, 
		      "Height (in mm) of the laser from the ground. 0 means unknown.", 0),
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("LaserIgnore", laserData->myLaserIgnore, 
		      "Angles (in deg) at which to ignore readings, +/1 one degree. Angles are entered as strings, separated by a space.",
		      sizeof(laserData->myLaserIgnore)), 
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "Laser_%i is upside-down.",
           laserNumber);

  config->addParam(
	  MvrConfigArg("LaserFlipped", &laserData->myLaserFlipped,
		            tempDescBuf),
	  section, MvrPriority::FACTORY, 
	  displayHintCheckbox.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section, MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::laserGetChoices();
  config->addParam(
	  MvrConfigArg("LaserType", laserData->myLaserType, 
		      "Type of laser.", 
		      sizeof(laserData->myLaserType)), 
	  section,MvrPriority::FACTORY, 
	  displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + 
		 Mvria::deviceConnectionGetChoices());
  config->addParam(
	  MvrConfigArg("LaserPortType", laserData->myLaserPortType, 
		      "Type of port the laser is on.", 
		      sizeof(laserData->myLaserPortType)), 

	  section, MvrPriority::FACTORY, 
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("LaserPort", laserData->myLaserPort, 
		      "Port the laser is on.", 
		      sizeof(laserData->myLaserPort)), 
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;


  config->addParam(
	  MvrConfigArg("LaserPowerOutput", 
		      laserData->myLaserPowerOutput,
		      "Power output that controls this laser's power.",
		      sizeof(laserData->myLaserPowerOutput)), 
	  section, MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("LaserStartingBaudChoice", 
		      laserData->myLaserStartingBaudChoice, 
		      "StartingBaud for this laser. Leave blank to use the default.", 
		      sizeof(laserData->myLaserStartingBaudChoice)),
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("LaserAutoBaudChoice", 
		      laserData->myLaserAutoBaudChoice, 
		      "AutoBaud for this laser. Leave blank to use the default.", 
		      sizeof(laserData->myLaserAutoBaudChoice)),
	  section, MvrPriority::FACTORY, 
	  displayHintPlain.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section, MvrPriority::FACTORY);

  if (!useDefaultBehavior)
    return;

  config->addParam(
	  MvrConfigArg("LaserPowerControlled", 
		            &laserData->myLaserPowerControlled,
		            "When enabled (true), this indicates that the power to the laser is controlled by the serial port line."), 
	  section,
	  MvrPriority::NORMAL);
      config->addParam(
	  MvrConfigArg("LaserMaxRange", (int *)&laserData->myLaserMaxRange, 
		      "Maximum range (in mm) to use for the laser. This should be specified only when the range needs to be shortened. 0 to use the default range."),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserCumulativeBufferSize", 
		      (int *)&laserData->myLaserCumulativeBufferSize, 
		      "Cumulative buffer size to use for the laser. 0 to use the default."), 
	  section,
	  MvrPriority::NORMAL);
  
  config->addParam(
	  MvrConfigArg("LaserStartDegrees", laserData->myLaserStartDegrees, 
		            "Start angle (in deg) for the laser. This may be used to constrain the angle. Fractional degrees are permitted. Leave blank to use the default.",
                sizeof(laserData->myLaserStartDegrees)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserEndDegrees", laserData->myLaserEndDegrees, 
		            "End angle (in deg) for the laser. This may be used to constrain the angle. Fractional degreees are permitted. Leave blank to use the default.",
                sizeof(laserData->myLaserEndDegrees)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserDegreesChoice", laserData->myLaserDegreesChoice, 
		            "Degrees choice for the laser. This may be used to constrain the range. Leave blank to use the default.",
                sizeof(laserData->myLaserDegreesChoice)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserIncrement", laserData->myLaserIncrement, 
		            "Increment (in deg) for the laser. Fractional degrees are permitted. Leave blank to use the default.",
                sizeof(laserData->myLaserIncrement)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserIncrementChoice", laserData->myLaserIncrementChoice, 
		            "Increment choice for the laser. This may be used to increase the increment. Leave blank to use the default.",
                sizeof(laserData->myLaserIncrementChoice)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserUnitsChoice", laserData->myLaserUnitsChoice, 
		            "Units for the laser. This may be used to increase the size of the units. Leave blank to use the default.",
                sizeof(laserData->myLaserUnitsChoice)),
	  section,
	  MvrPriority::NORMAL);
  config->addParam(
	  MvrConfigArg("LaserReflectorBitsChoice", 
		            laserData->myLaserReflectorBitsChoice, 
		            "ReflectorBits for the laser. Leave blank to use the default.",
                sizeof(laserData->myLaserReflectorBitsChoice)),
    section,
	  MvrPriority::NORMAL);

}

MVREXPORT void MvrRobotParams::addBatteryToConfig(
	int batteryNumber, MvrConfig* config, bool useDefaultBehavior)
{
  MvrConfigArg::RestartLevel restartLevel;
  // if we're using default behavior set it to none, since we can't
  // change it
  if (useDefaultBehavior)
    restartLevel = MvrConfigArg::NO_RESTART;
  // otherwise make it restart the software
  else 
    restartLevel = MvrConfigArg::RESTART_SOFTWARE;

  char buf[1024];
  sprintf(buf, "Battery_%d", batteryNumber);
  std::string section = buf;
  std::string batteryName = buf;
 
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section.c_str(),
                     "Information about the connection to this battery.");

  BatteryMTXBoardData *batteryMTXBoardData = new BatteryMTXBoardData;
  myBatteryMTXBoards[batteryNumber] = batteryMTXBoardData;

  myBatteryMTXBoardCount++;

  std::string displayHintPlain = "Visible:BatteryAutoConnect=true";

  std::string displayHintCustom;

  displayHintCustom = "Checkbox&&Visible:Generation!=Legacy";

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "%s exists and should be automatically connected at startup.",
           batteryName.c_str());

  config->addParam(
	  MvrConfigArg("BatteryAutoConnect", 
		            &batteryMTXBoardData->myBatteryMTXBoardAutoConn, 
                tempDescBuf),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::batteryGetChoices();
  config->addParam(
	  MvrConfigArg("BatteryType", 
		      batteryMTXBoardData->myBatteryMTXBoardType, 
		      "Type of battery.", 
		      sizeof(batteryMTXBoardData->myBatteryMTXBoardType)),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + 
		 Mvria::deviceConnectionGetChoices());
  config->addParam(
	  MvrConfigArg("BatteryPortType", 
		      batteryMTXBoardData->myBatteryMTXBoardPortType, 
		      "Port type that the battery is on.", 
		      sizeof(batteryMTXBoardData->myBatteryMTXBoardPortType)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("BatteryPort", 
		      batteryMTXBoardData->myBatteryMTXBoardPort, 
		      "Port the battery is on.", 
		      sizeof(batteryMTXBoardData->myBatteryMTXBoardPort)),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);
  config->addParam(
	  MvrConfigArg("BatteryBaud", 
		      &batteryMTXBoardData->myBatteryMTXBoardBaud, 
		      "Baud rate to use for battery communication (9600, 19200, 38400, etc.)."), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  // MPL TODO remove this since it's redundant (it's set in the constructor)
  //batteryMTXBoardData->myBatteryMTXBoardAutoConn = false;

}

MVREXPORT void MvrRobotParams::addLCDToConfig(
	int lcdNumber, MvrConfig* config, bool useDefaultBehavior)
{
  MvrConfigArg::RestartLevel restartLevel;
  // if we're using default behavior set it to none, since we can't
  // change it
  if (useDefaultBehavior)
    restartLevel = MvrConfigArg::NO_RESTART;
  // otherwise make it restart the software
  else 
    restartLevel = MvrConfigArg::RESTART_SOFTWARE;

  char buf[1024];
  sprintf(buf, "LCD_%d", lcdNumber);
  std::string section = buf;
  std::string lcdName = buf;

  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section.c_str(),
                     "The physical definition of this LCD.");
  
  LCDMTXBoardData *lcdMTXBoardData = new LCDMTXBoardData;
  myLCDMTXBoards[lcdNumber] = lcdMTXBoardData;

  /// MPL TODO what's this for?
  myLCDMTXBoardCount++;

  std::string displayHintPlain = "Visible:LCDAutoConnect=true";
  std::string displayHintCheckbox = displayHintPlain + "&&Checkbox";

  std::string displayHintCustom;

  /// MPL TODO remove, this is already set in the constructor
  //lcdMTXBoardData->myLCDMTXBoardAutoConn = false;
  displayHintCustom = "Checkbox&&Visible:Generation!=Legacy";

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "%s exists and should automatically be connected at startup.",
           lcdName.c_str());

  config->addParam(
	  MvrConfigArg("LCDAutoConnect", 
		            &lcdMTXBoardData->myLCDMTXBoardAutoConn, 
                tempDescBuf),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);
  /// MPL TODO remove, this is already set in the constructor
  //lcdMTXBoardData->myLCDMTXBoardConnFailOption = false

  config->addParam(
	  MvrConfigArg("LCDDisconnectOnConnectFailure", 
		      &lcdMTXBoardData->myLCDMTXBoardConnFailOption, 
		      "The LCD is a key component and is required for operation. If this is enabled and there is a failure in the LCD communications, then the robot will restart."),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCheckbox.c_str(), restartLevel);
  
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::lcdGetChoices();
  config->addParam(
	  MvrConfigArg("LCDType", 
		      lcdMTXBoardData->myLCDMTXBoardType, 
		      "Type of LCD.", 
		      sizeof(lcdMTXBoardData->myLCDMTXBoardType)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + 
		       Mvria::deviceConnectionGetChoices());
  config->addParam(
	  MvrConfigArg("LCDPortType", 
		      lcdMTXBoardData->myLCDMTXBoardPortType, 
		      "Port type that the LCD is on.",
		      sizeof(lcdMTXBoardData->myLCDMTXBoardPortType)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("LCDPort", 
		      lcdMTXBoardData->myLCDMTXBoardPort, 
		      "Port that the LCD is on.", 
		      sizeof(lcdMTXBoardData->myLCDMTXBoardPort)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;
  config->addParam(
	  MvrConfigArg("LCDPowerOutput", 
		      lcdMTXBoardData->myLCDMTXBoardPowerOutput, 
		      "Power output that controls this LCD's power.", 
		      sizeof(lcdMTXBoardData->myLCDMTXBoardPowerOutput)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("LCDBaud", 
		      &lcdMTXBoardData->myLCDMTXBoardBaud, 
		      "Baud rate for the LCD communication (9600, 19200, 38400, etc.)."), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);
}

MVREXPORT void MvrRobotParams::addSonarBoardToConfig(
	int sonarBoardNumber, MvrConfig* config, bool useDefaultBehavior)
{
  MvrConfigArg::RestartLevel restartLevel;
  // if we're using default behavior set it to none, since we can't
  // change it
  if (useDefaultBehavior)
    restartLevel = MvrConfigArg::NO_RESTART;
  // otherwise make it restart the software
  else 
    restartLevel = MvrConfigArg::RESTART_SOFTWARE;

  char buf[1024];
  sprintf(buf, "SonarBoard_%d", sonarBoardNumber);
  std::string section = buf;
  std::string sonarBoardName = buf;

  std::string displayHintPlain = "Visible:SonarAutoConnect=true";

  std::string displayHintCustom;
  
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section.c_str(),
                     "Information about the connection to this Sonar Board.");

  SonarMTXBoardData *sonarMTXBoardData = new SonarMTXBoardData;
  mySonarMTXBoards[sonarBoardNumber] = sonarMTXBoardData;

  /// MPL TODO what's this do?
  mySonarMTXBoardCount++;

  /// MPL TODO remove this next line (it's in the constructor
  //sonarMTXBoardData->mySonarMTXBoardAutoConn = false;
  displayHintCustom = "Checkbox&&Visible:Generation!=Legacy";

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "%s exists and should be automatically connected at startup.",
           sonarBoardName.c_str());
 
  config->addParam(
	  MvrConfigArg("SonarAutoConnect", 
		       &sonarMTXBoardData->mySonarMTXBoardAutoConn, 
           tempDescBuf),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);
  
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::sonarGetChoices();
  config->addParam(
	  MvrConfigArg("SonarBoardType", 
		      sonarMTXBoardData->mySonarMTXBoardType, 
		      "Type of the sonar board.", 
		      sizeof(sonarMTXBoardData->mySonarMTXBoardType)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + 
		 Mvria::deviceConnectionGetChoices());
  config->addParam(
	  MvrConfigArg("SonarBoardPortType", 
		      sonarMTXBoardData->mySonarMTXBoardPortType, 
		      "Port type that the sonar is on.", 
		      sizeof(sonarMTXBoardData->mySonarMTXBoardPortType)),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("SonarBoardPort", 
		       sonarMTXBoardData->mySonarMTXBoardPort, 
		       "Port the sonar is on.", 
		      sizeof(sonarMTXBoardData->mySonarMTXBoardPort)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;
  config->addParam(
	  MvrConfigArg("SonarBoardPowerOutput", 
		      sonarMTXBoardData->mySonarMTXBoardPowerOutput, 
		      "Power output that controls this Sonar Board's power.", 
		      sizeof(sonarMTXBoardData->mySonarMTXBoardPowerOutput)), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintCustom.c_str(), restartLevel);

  config->addParam(
	  MvrConfigArg("SonarBaud", 
		      &sonarMTXBoardData->mySonarMTXBoardBaud, 
		      "Baud rate for the sonar board communication. (9600, 19200, 38400, etc.)."), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  /// MPL Remove this, it's in the constructor
  //  sonarMTXBoardData->mySonarDelay = 1;
  config->addParam(
	  MvrConfigArg("SonarDelay", 
		      &sonarMTXBoardData->mySonarDelay, 
		      "Sonar delay (in ms).", 0, 10), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  /// MPL Remove this sonar gain, it's in the constructor
  //sonarMTXBoardData->mySonarGain = 5;
  config->addParam(
	  MvrConfigArg("SonarGain", 
		      &sonarMTXBoardData->mySonarGain, 
		      "Default sonar gain for the board, range 0-31.", 0, 31), 
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  // MPL TODO remove (moved this to constructor)
  //strcpy(&sonarMTXBoardData->mySonarThreshold[0],"3000|1500|2000");
  config->addParam(
	  MvrConfigArg("SonarDetectionThreshold", 
		      &sonarMTXBoardData->mySonarDetectionThreshold, 
		      "Default sonar detection threshold for the board.",
		      0, 65535),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

	/*
  config->addParam(
	  MvrConfigArg("SonarNoiseDelta", 
		      &sonarMTXBoardData->mySonarNoiseDelta, 
		      "Default sonar noise delta for the board.",
		      0, 65535),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);
	*/

  config->addParam(
	  MvrConfigArg("SonarMaxRange", 
		      &sonarMTXBoardData->mySonarMaxRange, 
		      "Default maximum sonar range for the board.",
		      0, 255*17),
	  section.c_str(), MvrPriority::FACTORY,
	  displayHintPlain.c_str(), restartLevel);

  
/*
  MvrLog::log(MvrLog::Normal, "MvrRobotParams: added sonar board %d with params %s %s %s %d %d %d",
  i, &sonarMTXBoardData->mySonarMTXBoardType[0], sonarMTXBoardData->mySonarMTXBoardPortType, 
  sonarMTXBoardData->mySonarMTXBoardPort, sonarMTXBoardData->mySonarMTXBoardBaud, 
  sonarMTXBoardData->mySonarMTXBoardAutoConn, 
  sonarMTXBoardData->mySonarDelay");
*/
  
}

void MvrRobotParams::addPTZToConfig(int i, MvrConfig *config)
{
  std::stringstream sectionStream;
  sectionStream << "PTZ " << i+1 << " parameters";
  std::string section = sectionStream.str();
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL, section.c_str(), "Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom control (PTZ) of a camera");
  config->addParam(MvrConfigArg("PTZAutoConnect", &(myPTZParams[i].connect), "If true, connect to this PTZ by default."), section.c_str());
  config->addParam(MvrConfigArg("PTZType", &(myPTZParams[i].type), "PTZ or PTU type"), section.c_str());
  config->addParam(MvrConfigArg("PTZInverted", &(myPTZParams[i].inverted), "If unit is mounted inverted (upside-down)"), section.c_str());
  config->addParam(MvrConfigArg("PTZSerialPort", &(myPTZParams[i].serialPort), "serial port, or none if not using serial port communication"), section.c_str());
  config->addParam(MvrConfigArg("PTZRobotAuxSerialPort", &(myPTZParams[i].robotAuxPort), "Pioneer aux.  serial port, or -1 if not using aux.  serial port for communication."), section.c_str());
  config->addParam(MvrConfigArg("PTZAddress", &(myPTZParams[i].address), "IP address or hostname, or none if not using network communication."), section.c_str());
  config->addParam(MvrConfigArg("PTZTCPPort", &(myPTZParams[i].tcpPort), "TCP Port to use for HTTP network connection"), section.c_str());
}


void MvrRobotParams::addVideoToConfig(int i, MvrConfig *config)
{
  std::stringstream sectionStream;
  sectionStream << "Video " << i+1 << " parameters";
  std::string section = sectionStream.str();
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL, section.c_str(), "Information about the connection to a video acquisition device, framegrabber, or camera");
  config->addParam(MvrConfigArg("VideoAutoConnect", &(myVideoParams[i].connect), "If true, connect to this device by default."), section.c_str());
  config->addParam(MvrConfigArg("VideoType", &(myVideoParams[i].type), "Device type"), section.c_str());
  //if (i == 0)
  //  printf("XXX added VideoType param for Video 1, its target is 0x%x, initial value is %s\n", &(myVideoParams[i].type), myVideoParams[i].type.c_str());
  config->addParam(MvrConfigArg("VideoInverted", &(myVideoParams[i].inverted), "If image should be flipped (for cameras mounted inverted/upside-down)"), section.c_str());
  config->addParam(MvrConfigArg("VideoWidth", &(myVideoParams[i].imageWidth), "Desired width of image, or -1 for default"), section.c_str());
  addParam(MvrConfigArg("VideoHeight", &(myVideoParams[i].imageHeight), "Desired height of image, or -1 for default"), section.c_str());
  addParam(MvrConfigArg("VideoDeviceIndex", &(myVideoParams[i].deviceIndex), "Device index, or -1 for default"), section.c_str());
  config->addParam(MvrConfigArg("VideoDeviceName", &(myVideoParams[i].deviceName), "Device name (overrides VideoDeviceIndex)"), section.c_str());
  config->addParam(MvrConfigArg("VideoChannel", &(myVideoParams[i].channel), "Input channel, or -1 for default"), section.c_str());
  addParam(MvrConfigArg("VideoAnalogSignalFormat", &(myVideoParams[i].analogSignalFormat), "NTSC or PAL, or empty for default. Only used for analog framegrabbers."), section.c_str(), MvrPriority::NORMAL, "Choices:NTSC;;PAL");
  config->addParam(MvrConfigArg("VideoAddress", &(myVideoParams[i].address), "IP address or hostname, or none if not using network communication."), section.c_str());
  config->addParam(MvrConfigArg("VideoTCPPort", &(myVideoParams[i].tcpPort), "TCP Port to use for HTTP network connection"), section.c_str());
}




/** Called by subclasses in MvrRobotTypes.h to set defaults (before file load; parameter file parsing uses parseSonar() instead)
    If any value is -1, then it is not set and any exisiting stored value is kept.
    @param num Index of sonar sensor on robot (starts at 0). NOTE These must form a set of consecutive integers over all calls to this function; if any are skipped then their entries in sonar unit parameters will be unset (uninitialized)!
    @param x X position of sensor on robot
    @param y Y position of sensor on robot
    @param th Angle at which the sensor points
    @param mtxboard For MTX sonar, which sonar module controls this sensor. Starts at 1.
    @param mtxunit For MTX sonar, index of this sensor in the MTX sonar module @a mtxboard. Starts at 1.
    @param mtxgain For MTX sonar, gain to set, or 0 to use default.
    @param mtxthresh For MTX sonar, detection threshold to set, or 0 to use default.
    @param mtxmax for MTX sonar, max range set on sonar to limit sensing
*/
MVREXPORT void MvrRobotParams::internalSetSonar(int num, int x, int y, int th,
    int mtxboard, int mtxunit, int mtxgain, int mtxthresh, int mtxmax )
{
  if(num < 0) 
  {
    MvrLog::log(MvrLog::Terse, "MvrRobotParams::internalSetSonar: Error: Invalid SonarUnit # %d (must be > 0).", num);
    return;
  }
  mySonarMap[num][SONAR_X] = x;
  mySonarMap[num][SONAR_Y] = y;
  mySonarMap[num][SONAR_TH] = th;
  mySonarMap[num][SONAR_BOARD] = mtxboard;
  mySonarMap[num][SONAR_BOARDUNITPOSITION] = mtxunit;
  mySonarMap[num][SONAR_GAIN] = mtxgain;
  mySonarMap[num][SONAR_DETECTION_THRESHOLD] = mtxthresh;
  mySonarMap[num][SONAR_MAX_RANGE] = mtxmax;
  mySonarMap[num][SONAR_USE_FOR_AUTONOMOUS_DRIVING] = true;
  myNumSonarUnits = MvrUtil::findMax(myNumSonarUnits, (num+1));
}

#if 0
MVREXPORT const std::list<MvrArgumentBuilder *> *MvrRobotParams::getSonarUnits(void)
{
  std::map<int, std::map<int, int> >::iterator it;
  int num, x, y, th;
  MvrArgumentBuilder *builder;

  for (it = mySonarMap.begin(); it != mySonarMap.end(); it++)
  {
    num = (*it).first;
    x = (*it).second[SONAR_X];
    y = (*it).second[SONAR_Y];
    th = (*it).second[SONAR_TH];
    builder = new MvrArgumentBuilder;
    builder->add("%d %d %d %d", num, x, y, th);
    myGetSonarUnitList.push_back(builder);
  }
  return &myGetSonarUnitList;
}
#endif

MVREXPORT bool MvrRobotParams::parseSonarUnit (MvrArgumentBuilder *builder)
{
	// If only three values given, then its pre-MTX style. If more, then its for MTX.
	if (builder->getArgc() == 4) {

    // pre-MTX style:

		if (builder->getArgc() != 4 || !builder->isArgInt (0) ||
		    !builder->isArgInt (1) || !builder->isArgInt (2) ||
		    !builder->isArgInt (3)) {
			MvrLog::log (MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid");
			return false;
		}

		const int num = builder->getArgInt(0);
		if(num < 0) 
		{
		  MvrLog::log(MvrLog::Terse, "MvrRobotParams: Error: Invalid SonarUnit # %d (must be > 0).", num);
		  return false;
		}

		mySonarMap[num][SONAR_X] = builder->getArgInt (1);
		mySonarMap[num][SONAR_Y] = builder->getArgInt (2);
		mySonarMap[num][SONAR_TH] = builder->getArgInt (3);

		MvrLog::log(MvrLog::Verbose, "MvrRobotParams::parseSonarUnit done parsing");
		
		return true;
	} else {

    // MTX style:

		return parseMTXSonarUnit (builder);
	}
}

MVREXPORT bool MvrRobotParams::parseMTXSonarUnit(MvrArgumentBuilder *builder)
{
  // there has to be at least 5 arguments, 1st 5 are ints
  // fix for bug 1959
  //if (5 < builder->getArgc() > 10 || !builder->isArgInt(0) || 
  if (builder->getArgc() < 5 /*|| builder->getArgc() > 8*/ || !builder->isArgInt(0) || 
      !builder->isArgInt(1) || !builder->isArgInt(2) ||
      !builder->isArgInt(3) || !builder->isArgInt(4) ||
      !builder->isArgInt(5))
	{
		MvrLog::log(MvrLog::Normal, "MvrRobotParams: SonarUnit parameters invalid, must include at least 5 integer values (MTX-style SonarUnit).",
								builder->getArgc());
		return false;
	}

  const int num = builder->getArgInt(0);
  myNumSonarUnits = MvrUtil::findMax(myNumSonarUnits, (num+1));
  if(num < 0) 
  {
    MvrLog::log(MvrLog::Terse, "MvrRobotParams: Error: Invalid SonarUnit # %d (must be > 0).", num);
    return false;
  }
  mySonarMap[num][SONAR_X] = builder->getArgInt(1);
  mySonarMap[num][SONAR_Y] = builder->getArgInt(2);
  mySonarMap[num][SONAR_TH] = builder->getArgInt(3);
  const int boardnum = builder->getArgInt(4);
  mySonarMap[num][SONAR_BOARD] = boardnum;
  mySonarMap[num][SONAR_BOARDUNITPOSITION] = builder->getArgInt(5);

  SonarMTXBoardData *sonarMTXBoardData = getSonarMTXBoardData(boardnum);
  if(sonarMTXBoardData)
	  sonarMTXBoardData->myNumSonarTransducers = MvrUtil::findMax(sonarMTXBoardData->myNumSonarTransducers,  mySonarMap[num][SONAR_BOARDUNITPOSITION]);
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrRobotParams: Error: Invalid MTX sonar board # %d in SonarUnit # %d.", boardnum, num);
    return false;
  }
	
  // prob should get these defaults from board
  mySonarMap[num][SONAR_GAIN] = 0;//SONAR_DEFAULT_GAIN;
	/*
  mySonarMap[builder->getArgInt(0)][SONAR_NOISE_DELTA] = 0;
	*/
  mySonarMap[num][SONAR_DETECTION_THRESHOLD] = 0;
  mySonarMap[num][SONAR_MAX_RANGE] = 0;
  mySonarMap[num][SONAR_USE_FOR_AUTONOMOUS_DRIVING] = true;
  
  if (builder->getArgc() > 6) {
    // gain arg will either be an int or "default"
    if (builder->isArgInt(6)) {
      mySonarMap[builder->getArgInt(0)][SONAR_GAIN] = builder->getArgInt(6);
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid, 7th value (gain) must be an integer value.");
      return false;
    }
  }
  
  if (builder->getArgc() > 7) {
    // gain arg will either be an int or "default"
    if (builder->isArgInt(7)) {
      mySonarMap[builder->getArgInt(0)][SONAR_DETECTION_THRESHOLD] = builder->getArgInt(7);
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid, 8th value (detect. thresh.) must be an integer value.");
      return false;
    }
  }
  if (builder->getArgc() > 8) {
    // gain arg will either be an int or "default"
    if (builder->isArgInt(8)) {
      mySonarMap[builder->getArgInt(0)][SONAR_MAX_RANGE] = builder->getArgInt(8);
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid, 9th value (max range) must be an integer value.");
      return false;
    }
  }
  if (builder->getArgc() > 9) {
    // 
    if (builder->isArgBool(9)) {
      mySonarMap[builder->getArgBool(0)][SONAR_USE_FOR_AUTONOMOUS_DRIVING] = builder->getArgBool(9);
    }
    else
    {
     MvrLog::log(MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid, 10th value (use for autonomous) must be a boolean value.");

     return false;
    }
  }

/*
if (builder->getArgc() > 10) {
    // gain arg will either be an int or "default"
    if (builder->isArgInt(10)) {
      mySonarMap[builder->getArgInt(0)][SONAR_NOISE_FLOOR] = builder->getArgInt(9);
    }
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrRobotParams: SonarUnit parameters invalid, 11th value (noise floor) must be an integer value.");
      return false;
    }
  }
*/

  MvrLog::log(MvrLog::Verbose, "MvrRobotParams::parseSonarUnit() parsed unit %d %d %d ", myNumSonarUnits,
	     mySonarMap[builder->getArgInt(0)][SONAR_BOARD],
	     mySonarMap[builder->getArgInt(0)][SONAR_BOARDUNITPOSITION]);



  // PS 9/5/12 - make numsonar = noumsonarunits
  myNumSonar = myNumSonarUnits;

  return true;
}

MVREXPORT const std::list<MvrArgumentBuilder *> *MvrRobotParams::getSonarUnits(void)
//MVREXPORT const std::list<MvrArgumentBuilder *> *MvrRobotParams::getMTXSonarUnits(void)
{
//  MvrLog::log(MvrLog::Normal, "Saving sonar units?");

  std::map<int, std::map<int, int> >::iterator it;
  int unitNum, x, y, th, boardNum, boardUnitPosition, gain, /*noiseDelta,*/ detectionThreshold, numEchoSamples;
	bool useForAutonomousDriving;
  MvrArgumentBuilder *builder;

  for (it = mySonarMap.begin(); it != mySonarMap.end(); it++)
  {
    unitNum = (*it).first;
    x = (*it).second[SONAR_X];
    y = (*it).second[SONAR_Y];
    th = (*it).second[SONAR_TH];
    boardNum = (*it).second[SONAR_BOARD];
    boardUnitPosition = (*it).second[SONAR_BOARDUNITPOSITION];
    gain = (*it).second[SONAR_GAIN];
		/*
    noiseDelta = (*it).second[SONAR_NOISE_DELTA];
		*/
    detectionThreshold = (*it).second[SONAR_DETECTION_THRESHOLD];
    numEchoSamples = (*it).second[SONAR_MAX_RANGE];
    useForAutonomousDriving = (*it).second[SONAR_USE_FOR_AUTONOMOUS_DRIVING];
    builder = new MvrArgumentBuilder;

		/* for noiseDelta
    builder->add("%d %d %d %d %d %d %d %d %d %d", 
		 unitNum, x, y, th, boardNum, boardUnitPosition, gain, noiseDelta, detectionThreshold, numEchoSamples);
		*/
	if(boardNum < 1 || boardUnitPosition < 1 || gain < 0 || detectionThreshold < 0 || numEchoSamples < 0)
		builder->add("%d %d %d %d", unitNum, x, y, th);
	else
		builder->add("%d %d %d %d %d %d %d %d %d %d", 
		 unitNum, x, y, th, boardNum, boardUnitPosition, gain, detectionThreshold, numEchoSamples, useForAutonomousDriving);

    myGetSonarUnitList.push_back(builder);
  }
  return &myGetSonarUnitList;
}


MVREXPORT bool MvrRobotParams::parseIRUnit(MvrArgumentBuilder *builder)
{
  if (builder->getArgc() != 5 || !builder->isArgInt(0) || 
      !builder->isArgInt(1) || !builder->isArgInt(2) || 
      !builder->isArgInt(3) || !builder->isArgInt(4))
  {
    MvrLog::log(MvrLog::Terse, "MvrRobotParams: IRUnit parameters invalid");
    return false;
  }
  myIRMap[builder->getArgInt(0)][IR_TYPE] = builder->getArgInt(1);
  myIRMap[builder->getArgInt(0)][IR_CYCLES] = builder->getArgInt(2);
  myIRMap[builder->getArgInt(0)][IR_X] = builder->getArgInt(3);
  myIRMap[builder->getArgInt(0)][IR_Y] = builder->getArgInt(4);
  return true;
}

MVREXPORT const std::list<MvrArgumentBuilder *> *MvrRobotParams::getIRUnits(void)
{
  std::map<int, std::map<int, int> >::iterator it;
  int num, type, cycles,  x, y;
  MvrArgumentBuilder *builder;

  for (it = myIRMap.begin(); it != myIRMap.end(); it++)
  {
    num = (*it).first;
    type = (*it).second[IR_TYPE];
    cycles = (*it).second[IR_CYCLES];
    x = (*it).second[IR_X];
    y = (*it).second[IR_Y];
    builder = new MvrArgumentBuilder;
    builder->add("%d %d %d %d %d", num, type, cycles, x, y);
    myGetIRUnitList.push_back(builder);
  }
  return &myGetIRUnitList;
}

MVREXPORT void MvrRobotParams::internalSetIR(int num, int type, int cycles, int x, int y)
{
  myIRMap[num][IR_TYPE] = type;
  myIRMap[num][IR_CYCLES] = cycles;
  myIRMap[num][IR_X] = x;
  myIRMap[num][IR_Y] = y;
}

MVREXPORT bool MvrRobotParams::save(void)
{
  char buf[10000];
  sprintf(buf, "%sparams/", Mvria::getDirectory());
  setBaseDirectory(buf);
  sprintf(buf, "%s.p", getSubClassName());
  return writeFile(buf, false, NULL, false);
}

void MvrRobotParams::internalAddToConfigCommercial(
	MvrConfig *config)
{
  MvrLog::log(MvrLog::Normal, "MvrRobotParams: Adding to config");
  
  // initialize some values
  myCommercialConfig = config;
  myCommercialAddedConnectables = false;
  myCommercialProcessedSonar = false;
  myCommercialNumSonar = 16;
  myCommercialMaxNumberOfLasers = 4;
  myCommercialMaxNumberOfBatteries = 1;
  myCommercialMaxNumberOfLCDs = 1;
  myCommercialMaxNumberOfSonarBoards = 2;

  // add the callback
  myCommercialConfig->addProcessFileCB(&myCommercialProcessFileCB, 90);

  /// reset some values from their default
  sprintf(myClass, "MTX");
  /// these probably aren't actually used anywhere, but just in case
  myFrontBumpers = true;
  myRearBumpers = true;
  myRobotRadius = 255;
  myRobotWidth = 400;
  myRobotLengthFront = 255;
  myRobotLengthRear = 255;

  // now add the normal config
  std::string section = "General";
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section.c_str(),
                     "The general definition of this vehicle");

  MvrConfigArg generationArg(
	  "Generation", myClass, 
	  "The generation of technology this is.  The MT400 and Motivity Core this should be Legacy.  Everything else is MTX.", 
	  sizeof(myClass));
  generationArg.setExtraExplanation("This main external thing this affects is that for a Legacy lasers are named by type (for backwards compatibility with existing config files), whereas for MTX they are named by their number (for easier future compatibility).");

  myCommercialConfig->addParam(
	  generationArg,
	  section.c_str(), MvrPriority::FACTORY, "Choices:Legacy;;MTX",
	  MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(
	  MvrConfigArg("Model", mySubClass, 
		      "The model name. This should be human readable and is only for human consumption.", 
		      sizeof(mySubClass)), 
	  section.c_str(), MvrPriority::FACTORY, "",
	  MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

  myCommercialConfig->addParam(
	  MvrConfigArg("Radius", 
		      &myRobotRadius, 
		      "The radius in mm that is needed to turn in place safely. (mm)", 
		      1), 
	  section.c_str(), MvrPriority::FACTORY, "",
	  MvrConfigArg::RESTART_SOFTWARE);
  myCommercialConfig->addParam(
	  MvrConfigArg("Width", 
		      &myRobotWidth, 
		      "Width in mm (mm)",
	      1), 
	  section.c_str(), MvrPriority::FACTORY, "",
	  MvrConfigArg::RESTART_SOFTWARE);
  myCommercialConfig->addParam(
	  MvrConfigArg("LengthFront", 
		      &myRobotLengthFront, 
		      "Length in mm from the idealized center of rotation to the front (mm)", 
		      1),
	   section.c_str(), MvrPriority::FACTORY, "",
	   MvrConfigArg::RESTART_SOFTWARE);
  myCommercialConfig->addParam(
	  MvrConfigArg("LengthRear", 
		      &myRobotLengthRear, 
		      "Length in mm from the idealized center of rotation to the rear (mm)", 
		      1), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "", MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);


  myCommercialConfig->addParam(
	  MvrConfigArg("DistanceCalibrationFactor",
		      &myDistConvFactor, "The per-vehicle calibration factor for distance errors.  A perfect vehicle would have a value of 1.  Travelled distances are multiplied by this, so if the vehicle drove 1% to far you'd make this value .99. This is to account for differences within a model that ideally (ideally there wouldn't be any).  (multiplier)", 0),
	  section.c_str(), MvrPriority::CALIBRATION,
	  "", MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

  myCommercialConfig->addParam(
	  MvrConfigArg("NumberOfFrontBumpers", &myNumFrontBumpers,
			       "Number of front bumpers", 0, 7), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox", MvrConfigArg::RESTART_SOFTWARE);
  myCommercialConfig->addParam(
	  MvrConfigArg("NumberOfRearBumpers", &myNumRearBumpers,
		      "Number of rear bumpers", 0, 7), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox", MvrConfigArg::RESTART_SOFTWARE);
  
  myCommercialConfig->addParam(
	  MvrConfigArg(MvrConfigArg::SEPARATOR),
	  section.c_str(), MvrPriority::FACTORY);

  myCommercialConfig->addParam(
	  MvrConfigArg("MaxNumberOfLasers", 
		      &myCommercialMaxNumberOfLasers,
		      "Max number of lasers", 1, 9), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox", MvrConfigArg::RESTART_SOFTWARE);
  
  myCommercialConfig->addParam(
	  MvrConfigArg("MaxNumberOfBatteries", 
		      &myCommercialMaxNumberOfBatteries,
		      "Max number of Batteries", 0, 9), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox&&Visible:Generation!=Legacy", 
	  MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(
	  MvrConfigArg("MaxNumberOfLCDs", 
		      &myCommercialMaxNumberOfLCDs,
		      "Max number of LCDs", 0, 9), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox&&Visible:Generation!=Legacy", 
	  MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(
	  MvrConfigArg("MaxNumberOfSonarBoards", 
		      &myCommercialMaxNumberOfSonarBoards,
		      "Max number of Sonar Boards", 0, 9), 
	  section.c_str(), MvrPriority::FACTORY, 
	  "SpinBox&&Visible:Generation!=Legacy", 
	  MvrConfigArg::RESTART_SOFTWARE);

  myCommercialConfig->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		   section.c_str(), MvrPriority::FACTORY);

}

MVREXPORT bool MvrRobotParams::commercialProcessFile(void)
{
  myRobotLength = myRobotLengthFront + myRobotLengthRear;

  // MPL CONFIG TODO process the sonar

  if (myCommercialAddedConnectables && !myCommercialProcessedSonar)
  {
    processSonarCommercial(myCommercialConfig);
    myCommercialProcessedSonar = true;
  }

  if (!myCommercialAddedConnectables)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobotParams: Adding connectables");

    myCommercialAddedConnectables = true;
    Mvria::setMaxNumLasers(myCommercialMaxNumberOfLasers);

    // if it's an MTX set the types
    if (MvrUtil::strcasecmp(myClass, "Legacy") != 0)
    {
      Mvria::setMaxNumBatteries(myCommercialMaxNumberOfBatteries);
      Mvria::setMaxNumLCDs(myCommercialMaxNumberOfLCDs);
      Mvria::setMaxNumSonarBoards(myCommercialMaxNumberOfSonarBoards);
      addSonarToConfigCommercial(myCommercialConfig, true);
    }
    else
    {
      Mvria::setMaxNumBatteries(0);
      Mvria::setMaxNumLCDs(0);
      Mvria::setMaxNumSonarBoards(0);
      addSonarToConfigCommercial(myCommercialConfig, false);
    }

    int i;
    for (i = 1; i <= Mvria::getMaxNumSonarBoards(); i++)
      addSonarBoardToConfig(i, myCommercialConfig, ourUseDefaultBehavior);
    
    for (i = 1; i <= Mvria::getMaxNumLasers(); i++)
    {
      char buf[1024];
      sprintf(buf, "Laser_%d", i);
      
      addLaserToConfig(i, myCommercialConfig, ourUseDefaultBehavior, buf);
    }
    
    for (i = 1; i <= Mvria::getMaxNumBatteries(); i++)
      addBatteryToConfig(i, myCommercialConfig, ourUseDefaultBehavior);
    
    for (i = 1; i <= Mvria::getMaxNumLCDs(); i++)
      addLCDToConfig(i, myCommercialConfig, ourUseDefaultBehavior);
  }
  

  return true;
}


void MvrRobotParams::addSonarToConfigCommercial(MvrConfig *config, 
					       bool isMTXSonar)
{
  std::string section = "Sonar";

  int maxSonar = 64;
  
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section.c_str(),
                     "Definition of the sonar on this vehicle.");

  config->addParam(MvrConfigArg("NumSonar", &myCommercialNumSonar, "Number of sonars on this robot.", 0, maxSonar),
		   section.c_str(), MvrPriority::FACTORY,
		   "SpinBox", MvrConfigArg::RESTART_SOFTWARE);
  
  MvrConfigArg sonar(MvrConfigArg::LIST, "Sonar", "Definition of this single sonar transducer.");
  sonar.setConfigPriority(MvrPriority::FACTORY);
  sonar.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);

  MvrConfigArg sonarX("X", 0, 
		     "Location (in mm) of this sonar transducer in X (+ front, - back) relative to the robot's idealized center of rotation.");
  sonarX.setConfigPriority(MvrPriority::FACTORY);
  sonarX.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
  myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_X;
  sonar.addArg(sonarX);

  MvrConfigArg sonarY("Y", 0, 
		     "Location (in mm) of this sonar transducer in Y (+ left, - right) relative to the robot's idealized center of rotation.");
  sonarY.setConfigPriority(MvrPriority::FACTORY);
  sonarY.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
  myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_Y;
  sonar.addArg(sonarY);

  MvrConfigArg sonarTh("Th", 0, 
		     "Rotation (in deg) of this sonar transducer (+ counterclockwise, - clockwise).", 
		     -180, 180);
  sonarTh.setConfigPriority(MvrPriority::FACTORY);
  sonarTh.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
  myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_TH;
  sonar.addArg(sonarTh);

  if (isMTXSonar)
  {
    MvrConfigArg sonarBoard("SonarBoard", 0, 
			   "Sonar board that is used by this transducer. 0 means that it is not yet configured.",
			   0, Mvria::getMaxNumSonarBoards());
    sonarBoard.setConfigPriority(MvrPriority::FACTORY);
    sonarBoard.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_BOARD;
    sonar.addArg(sonarBoard);

    MvrConfigArg sonarBoardUnitPosition("SonarBoardUnitPosition", 0, 
          "Position of the transducer on the sonar board. 0 means that it is not yet configured.",
				       0, 8);
    sonarBoardUnitPosition.setConfigPriority(MvrPriority::FACTORY);
    sonarBoardUnitPosition.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_BOARDUNITPOSITION;
    sonar.addArg(sonarBoardUnitPosition);
    
    MvrConfigArg sonarGain("Gain", 0, 
			  "Sonar gain to be used by this transducer. 0 to use the board default.", 0, 31);

    sonarGain.setConfigPriority(MvrPriority::FACTORY);
    sonarGain.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_GAIN;
    sonar.addArg(sonarGain);

		/*
    MvrConfigArg sonarNoiseDelta("NoiseDelta", 0, 
				"Sonar noise delta to be used by this transducer. 0 to use the board default.", 0, 65535);
    sonarNoiseDelta.setConfigPriority(MvrPriority::FACTORY);
    sonarNoiseDelta.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_NOISE_DELTA;
    sonar.addArg(sonarNoiseDelta);
		*/

    MvrConfigArg sonarDetectionThreshold("DetectionThreshold", 0, 
					"Sonar detection threshold to be used by this transducer. 0 to use the board default.", 0, 65535);
    sonarDetectionThreshold.setConfigPriority(MvrPriority::FACTORY);
    sonarDetectionThreshold.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_DETECTION_THRESHOLD;
    sonar.addArg(sonarDetectionThreshold);

    MvrConfigArg sonarMaxRange("MaxRange", 0, 
			      "Maximum range for this transducer. 0 to use the board default.", 0, 17*255);
    sonarMaxRange.setConfigPriority(MvrPriority::FACTORY);
    sonarMaxRange.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_MAX_RANGE;
    sonar.addArg(sonarMaxRange);

    MvrConfigArg sonarUseForAutonomousDriving("UseForAutonomousDriving", true, 
					"Checked means use for Autonomous Driving (aka Path Planning) as well as all other driving.  Not checked means use the sonar will still be used by all other driving.");
    sonarUseForAutonomousDriving.setConfigPriority(MvrPriority::FACTORY);
    sonarUseForAutonomousDriving.setRestartLevel(MvrConfigArg::RESTART_SOFTWARE);
    sonarUseForAutonomousDriving.setDisplayHint("Checkbox");
    myCommercialSonarFieldMap[sonar.getArgCount()] = SONAR_USE_FOR_AUTONOMOUS_DRIVING;
    sonar.addArg(sonarUseForAutonomousDriving);
  }

  char displayHintBuf[1024];
  char nameBuf[1024];
  
  for (int ii = 0; ii < maxSonar; ii++)
  {
    snprintf(nameBuf, sizeof(nameBuf), 
             "Sonar_%d",
	     ii+1);

    snprintf(displayHintBuf, sizeof(displayHintBuf), 
             "Visible:NumSonar>%d",
	     ii);

    MvrConfigArg *arg = new MvrConfigArg(
	    nameBuf, sonar);

    config->addParam(*arg, section.c_str(), MvrPriority::FACTORY,
		     displayHintBuf, MvrConfigArg::RESTART_SOFTWARE);
  }    

  
}

void MvrRobotParams::processSonarCommercial(MvrConfig *config)
{
  int ii;


  std::string configSection = "Sonar";

  MvrConfigSection *section = NULL;
  if ((section = Mvria::getConfig()->findSection(configSection.c_str())) == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobotParams:processSonarCommercial: Can't find section '%s'", 
	       configSection.c_str());
    return;
  }

  char nameBuf[1024];

	myNumSonarUnits = myCommercialNumSonar;
	myNumSonar = myCommercialNumSonar;

  for (ii = 0; ii < myCommercialNumSonar; ii++)
  {
    snprintf(nameBuf, sizeof(nameBuf), 
             "Sonar_%d",
	     ii+1);

    MvrConfigArg *argFromConfig = NULL;
    if ((argFromConfig = section->findParam(nameBuf)) == NULL)
    {
      MvrLog::log(MvrLog::Normal, 
	       "MvrRobotParams:processSonarCommercial: Can't find parameter '%s' in section '%s'", 
		 nameBuf, configSection.c_str());
      continue;
    }

    for (int jj = 0; jj < argFromConfig->getArgCount(); jj++)
    {
      // this sets the sonar number ii field jj (mapped when added) to
      // the value of the integer, doing the same thing normally done
      // with the SonarInfo enum, but automatically instead of relying
      // on pesky humans typing
      if (argFromConfig->getArg(jj)->getType() == MvrConfigArg::INT)
	mySonarMap[ii][jj] = argFromConfig->getArg(jj)->getInt();    
      else if (argFromConfig->getArg(jj)->getType() == MvrConfigArg::BOOL)
	mySonarMap[ii][jj] = argFromConfig->getArg(jj)->getBool();    
    }
  }


  
}


MVREXPORT void MvrVideoParams::merge(const MvrVideoParams& other)
{
//  printf("MvrVideoParams::merge: other.type=%s, this.type=%s.\n", other.type.c_str(), type.c_str());
  if(other.type != "unknown" && other.type != "none" && other.type != "")
  {
    //printf("MvrVideoParams::merge: replacing this type %s with other %s\n", type.c_str(), other.type.c_str());
    type = other.type;
  }
  if(other.connectSet)
  {
 //   printf("MvrVideoParams::merge: replacing this connect %d with other %d. other.connectSet=%d, this.connectSet=%d\n", connect, other.connect, other.connectSet, connectSet);
    connect = other.connect;
    connectSet = true;
  }
  if(other.imageWidth != -1)
  {
    imageWidth = other.imageWidth;
  }
  if(other.imageHeight != -1)
  {
    imageHeight = other.imageHeight;
  }
  if(other.deviceIndex != -1)
  {
    deviceIndex = other.deviceIndex;
  }
  if(other.deviceName != "none" && other.deviceName != "")
  {
    deviceName = other.deviceName;
  }
  if(other.channel != -1)
  {
    channel = other.channel;
  }
  if(other.analogSignalFormat != "none" && other.analogSignalFormat != "")
  {
    analogSignalFormat = other.analogSignalFormat;
  }
  //printf("MvrVideoParams::merge: this address is %s, other address is %s\n", address.c_str(), other.address.c_str());
  if(other.address != "none" && other.address != "")
  {
	  //printf("MvrVideoParams::merge: replacing this address %s with other %s\n", address.c_str(), other.address.c_str());
    address = other.address;
  }
  if(other.tcpPortSet)
  {
    tcpPort = other.tcpPort;
    tcpPortSet = true;
  }
  if(other.invertedSet)
  {
    //printf("MvrVideoParams::merge: replacing this inverted %d with other %d\n", inverted, other.inverted);
    inverted = other.inverted;
    invertedSet = true;
  }
}

void MvrPTZParams::merge(const MvrPTZParams& other)
{
  //printf("MvrPTZParams::merge: other.type=%s, this.type=%s.\n", other.type.c_str(), type.c_str());
  if(other.type != "unknown" && other.type != "none" && other.type != "")
  {
    //printf("MvrPTZParams::merge: replacing this type %s with other %s\n", type.c_str(), other.type.c_str());
    type = other.type;
  }
  if(other.connectSet)
  {
    //pgintf("MvrPTZParams::merge: replacing this connect %d with other %d\n", connect, other.connect);
    //printf("MvrPTZParams::merge: replacing this connect %d with other %d. other.connectSet=%d, this.connectSet=%d\n", connect, other.connect, other.connectSet, connectSet);
    connect = other.connect;
    connectSet = true;
  }
  if(other.serialPort != "none" && other.serialPort != "")
  {
    //printf("MvrPTZParams::merge: replacing this serialPort %s with other %s\n", serialPort.c_str(), other.serialPort.c_str());
    serialPort = other.serialPort;
  }
  if(other.robotAuxPort != -1)
  {
    //printf("MvrPTZParams::merge: replacing this robotAuxPort %d with other %d\n", robotAuxPort, other.robotAuxPort);
    robotAuxPort = other.robotAuxPort;
  }
  if(other.address != "none")
  {
    //printf("MvrPTZParams::merge: replacing this address %s with other %s\n", address.c_str(), other.address.c_str());
    address = other.address;
  }
  if(other.tcpPortSet)
  {
    //printf("MvrPTZParams::merge: replacing this tcpPort %d with other %d\n", tcpPort, other.tcpPort);
    tcpPort = other.tcpPort;
    tcpPortSet = true;
  }
  if(other.invertedSet)
  {
    //printf("MvrPTZParams::merge: replacing this inverted %d with other %d\n", inverted, other.inverted);
    inverted = other.inverted;
    invertedSet = true;
  }
}

