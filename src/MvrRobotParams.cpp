/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotParams.h
 > Description  : Stores a set of video device parameters read from one of the video sections 
                  of a robot parameter file.
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年06月21日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaInternal.h"
#include "MvrRobotParams.h"

#include <sstream>

bool MvrRobotParams::ourUseDefaultBehavior = true;
std::string ourPowerOutputDisplayHint;

void MvrRobotParams::internalSetUseDefaultBehavior(bool useDefaultBehavior, const char *powerOutputDisplayHint)
{
  ourUseDefaultBehavior     = useDefaultBehavior;
  ourPowerOutputDisplayHint = powerOutputDisplayHint;

  MvrLog::log(MvrLog::Normal,
              "MvrRobotParams: Setting use default behavior to %s, with power output display hint '%s'",
              MvrUtil::convertBool(ourUseDefaultBehavior),
              ourPowerOutputDisplayHint);
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
          myCommercialProcessFileCB(this, &MvrRobotParams::myCommercialProcessFileCB)
{
  myCommercialConfig = NULL;

  sprintf(myClass, "Joyounger");
  mySubClass[0]         = '\0';
  myRobotRadius         = 430;      // TODO modify
  myRobotDiagonal       = 125;
  myRobotWidth          = 300;
  myRobotLength         = 300;
  myRobotLengthFront    = 0;
  myRobotLengthRear     = 0;
  myHolonomic           = true;  // false
  myAbsoluteMaxVelocity = 0;
  myAbsoluteMaxRVelocity= 0;
  myHaveMoveCommand     = true;
  myAngleConvFactor     = 0.001534;
  myDistConvFactor      = 1.0;
  myVelConvFactor       = 1.0;
  myRangeConvFactor     = 1.0;
  myVel2Divisor         = 20;
  myNumSonar            = 0;
  myGyroScaler          = 1.626;
  myTableSensingIR      = false;
  myNewTableSensingIR   = false;
  myFrontBumpers        = false;
  myNumFrontBumpers     = 5;
  myRearBumpers         = false;
  myNumRearBumpers      = 5;
  myNumSonarUnits       = 0;
  // TODO wey we need these counts?
  mySonarBoardCount       = 0;
  myBatteryMTXBoardCount  = 0;
  myLCDMTXBoardCount      = 0;
  mySonarMTXBoardCount    = 0;

  mySonarMap.clear();

  myNumIR = 0;
  myIRMap.clear();

  myRequestIOPackets      = false;
  myRequestEncoderPackets = false;
  mySwitchToBaudRate      = 38400;

  mySettableVelMaxes      = true;
  myTransVelMax           = 0;
  myRotVelMax             = 0;

  mySettableAccsDecs      = true;
  myTransAccel            = 0;
  myTransDecel            = 0;
  myRotAccel              = 0;
  myRotDecel              = 0;

  myHasLatVel             = false;
  myLatVelMax             = 0;
  myLatAccel              = 0;
  myLatDecel              = 0;
  myAbsoluteMaxLatVelocity= 0;

  myGPSX = 0;
  myGPSY = 0;
  strcpy(myGPSPort, "COM2");
  strcpy(myGPSType,"standard");
  myGPSBaud = 9600;

  //strcpy(mySonarPort, "COM2");
  //strcpy(mySonarType, "standard");
  //mySonarBaud = 115200;
			
  //strcpy(myBatteryMTXBoardPort, "COM1");
  //strcpy(myBatteryMTXBoardType, "mtxbatteryv1");
  //myBatteryMTXBoardBaud = 115200;

  strcpy(myCompassType, "robot");
  strcpy(myCompassType, "");

  if (ourUseDefaultBehavior)
    internalAddToConfigDefault();
}          

MVREXPORT MvrRobotParams::~MvrRobotParams()
{

}

void MvrRobotParams::internalAddToConfigDefault(void)
{
  addComment("Robot parameter file");
  // addComment("");
  // addComment("General settings");
  std::string section;
  section = "Gerneral settings";
  addParam(MvrConfigArg("Class", myClass, "general type of robot", sizeof(myClass)), 
           section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("Subclass", mySubClass, "special type of robot", sizeof(mySubClass)), 
           section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RobotRadius", &myRobotRadius, "radius in mm"),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotDiagonal", &myRobotDiagonal, "half-height to diagonal of octagon"),
           "Gerneral settings", MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("RobotWidth", &myRobotWidth, "width in mm"),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLength", &myRobotLength, "length in mm of the whole robot"),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLengthFront", &myRobotLengthFront, "length in mm to the front of the robot (if this is 0 (or non existent) this value will be set to half of RobotLength)"),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RobotLengthRear", &myRobotLengthRear, "length in mm to the rear of the robot (if this is 0 (or non existent) this value will be set to half of RobotLength"),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("Holonomic", &myHolonomic, "turns in own radius"),
           section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("MaxRVelocity", &myAbsoluteMaxRVelocity, "absolute maximum degrees /sec"),
           section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("MaxVelocity", &myAbsoluteMaxVelocity, "absolute maximum mm /sec"),
           section.c_str(), MvrPriority::TRIVIAL); 
  addParam(MvrConfigArg("MaxLatVelocity", &myAbsoluteMaxLatVelocity, "absolute lateral maximum mm /sec"),
           section.c_str(), MvrPriority::TRIVIAL);                    
  addParam(MvrConfigArg("HasMoveCommand", &myHaveMoveCommand, "has build in move command"),
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("RequestIOPackets", &myRequestIOPackets, "automatically request IO packets"),
           section.c_str(), MvrPriority::NORMAL);                         
  addParam(MvrConfigArg("RequestEncoderPackets", &myRequestEncoderPackets, "automatically requeset encode packets"),
           section.c_str(), MvrPriority::NORMAL); 
  addParam(MvrConfigArg("SwitchToBaudRate", &mySwitchToBaudRate, "switch to this baud if non-0 and supported on robot"),
           section.c_str(), MvrPriority::NORMAL);            
                                              
  section = "Conversion factors";
  addParam(MvrConfigArg("AngleConvFactor", &myAngleConvFactor, "radians per angular unit (2PI/4096)"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("DistConvFactor", &myDistConvFactor, "multiplier to mm from robot units"), 
           section.c_str(), MvrPriority::IMPORTANT);             
  addParam(MvrConfigArg("VelConvFactor", &myVelConvFactor, "multiplier to mm/sec from robot units"), 
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("RangeConvFactor", &myRangeConvFactor, "multiplier to mm from sonar units"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("DiffConvFactor", &myDiffConvFactor, "ratio of angular velocity to wheel velocity (unused in newer firmware that calculates and return this)"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("Vel2Divisor", &myVel2Divisor, "divisor for VEL2 commands"), 
           section.c_str(), MvrPriority::TRIVIAL);
  addParam(MvrConfigArg("GyroScaler", &myGyroScaler, "Scaling factor for gyro readings"), 
           section.c_str(), MvrPriority::IMPORTANT);       
 
  section = "Accessories the robot has";
  addParam(MvrConfigArg("TableSensingIR", &myTableSensingIR, "if robot has upwards facing table sensing IR"), 
           section.c_str(), MvrPriority::TRIVIAL);   
  addParam(MvrConfigArg("NewTableSensingIR", &myNewTableSensingIR, "if table sensing IR are sent in IO packet"), 
           section.c_str(), MvrPriority::TRIVIAL); 
  addParam(MvrConfigArg("FrontBumpers", &myFrontBumpers, "if robot has a front bumpers on the robot"), 
           section.c_str(), MvrPriority::TRIVIAL);   
  addParam(MvrConfigArg("NumFrontBumpers", &myNumFrontBumpers, "number of front bumper on the robot"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("RearBumpers", &myRearBumpers, "if the robot has a rear bumper ring"), 
           section.c_str(), MvrPriority::IMPORTANT);                                            
  addParam(MvrConfigArg("NumRearBumpers", &myNumRearBumpers, "number of rear bumpers on the robot"), 
           section.c_str(), MvrPriority::TRIVIAL);   

  section = "IR parameters"; 
  addParam(MvrConfigArg("IRNum", &myNumIR, "number of IRs on the robot"), 
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("IRUnit", &myIRUnitSetFunctor, &myIRUnitGetFunctor, "IRUnit <IR Number> <IR Type> <Persistance, cycles> <x position, mm> <y position, mm>"), 
	         section.c_str(), MvrPriority::TRIVIAL);

  section = "Movement control parameters";
  setSectionComment(section.c_str(), "if these are 0 the parameters from robot flash will be used, otherwise these value will be used")                    ;
  addParam(MvrConfigArg("SettableVelMaxes", &mySettableVelMaxes, "if TransVelMax and RotVelMax can be set"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("TransVelMax", &myTransVelMax, "maximum desired translational velocity for the robot "), 
           section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("RotVelMax", &myRotVelMax, "maximum desired rotational velocity for the robot "), 
           section.c_str(), MvrPriority::IMPORTANT);  
  addParam(MvrConfigArg("SettableAccsDecs", &mySettableAccsDecs, "if the accel and decel parameters can be set"), 
           section.c_str(), MvrPriority::TRIVIAL);  
  addParam(MvrConfigArg("TransAccel", &myTransAccel, "translational acceleration"), 
           section.c_str(), MvrPriority::IMPORTANT);  
  addParam(MvrConfigArg("TransDecel", &myTransDecel, "translational deceleration"), 
           section.c_str(), MvrPriority::IMPORTANT); 
  addParam(MvrConfigArg("RotAccel", &myRotAccel, "rotational acceleration"), 
           section.c_str(), MvrPriority::IMPORTANT);              
  addParam(MvrConfigArg("RotDecel", &myRotDecel, "rotational deceleration"), 
           section.c_str(), MvrPriority::IMPORTANT);     

  addParam(MvrConfigArg("HasLatVel", &myHasLatVel, "if the robot has later velocity"), 
           section.c_str(), MvrPriority::TRIVIAL);   
  addParam(MvrConfigArg("LatVelMax", &myLatVelMax, "maximum desired lateral velocity for the robot"), 
           section.c_str(), MvrPriority::IMPORTANT);  
  addParam(MvrConfigArg("LatAccel", &myLatAccel, "lateral acceleration"), 
           section.c_str(), MvrPriority::IMPORTANT);         
  addParam(MvrConfigArg("LatDecel", &myLatDecel, "lateral deceleration"), 
           section.c_str(), MvrPriority::IMPORTANT);     

  section = "GPS parameters";
  // There is a "P" in the middle of the position parameters. Don't remove it if
  // you think it's irrelevant, it will break all robot parameter files.
  addParam(MvrConfigArg("GPSPX", &myGPSX,
           "x location of gps receiver antenna on robot, mm"), 
           section.c_str(), MvrPriority::NORMAL); 
  addParam(MvrConfigArg("GPSPY", &myGPSY,
           "y location of gps receiver antenna on robot, mm"), 
           section.c_str(), MvrPriority::NORMAL); 
  addParam(MvrConfigArg("GPSType", myGPSType,
           "type of gps receiver (trimble, novatel, standard)", sizeof(myGPSType)),
           section.c_str(), MvrPriority::IMPORTANT);
  addParam(MvrConfigArg("GPSPort", myGPSPort, "port the gps is on", sizeof(myGPSPort)),
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("GPSBaud", &myGPSBaud,
           "gps baud rate (9600, 19200, 38400, etc.)"),
           section.c_str(), MvrPriority::NORMAL);

  section = "Compass parameters";
  addParam(MvrConfigArg("CompassType", myCompassType,
           "type of compass: robot (typical configuration), or serialTCM (computer serial port)", sizeof(myCompassType))), 
           section.c_str(), MvrPriority::NORMAL); 
  addParam(MvrConfigArg("CompassPort", myCompassPort, "serial port name, if CompassType is serialTCM", sizeof(myCompassPort)),
           section.c_str(), MvrPriority::NORMAL);           

  section = "Sonar parameters";
  addParam(MvrConfigArg("SonarNum", &mySonarNum, "Number of sonars on the robot"), 
           section.c_str(), MvrPriority::NORMAL);
  addParam(MvrConfigArg("SonarUnit", &mySonarUnitSetFunctor, &mySonarUnitGetFunctor, "SonarUnit <sonarNumber> <x position, mm> <y position, mm> <heading of disc, degrees> <MTX sonar board> <MTX sonar board unit position> <MTX gain> <MTX detection threshold> <MTX max range> <autonomous driving sensor flag>"),
           section.c_str(), MvrPriority::TRIVIAL);

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
      sprintf(buf, "Laser %d parameters",i);
      section = buf;
    }
    addLaserToConfig(i, this, ourUseDefaultBehavior, section.c_str());
  }           

  for (i = 1; i <= Mvria::getMaxNumBatteries(); i++)
    addBatteryToConfig(i, this, ourUseDefaultBehavior);

  for (i = 1; i <= Mvria::getMaxNumLCDs(); i++)
    addLCDToConfig(i, this, ourUseDefaultBehavior);

  for (i = 1; i <= Mvria::getMaxNumPTZs(); i++)
    addPTZToConfig(i, this);  

  /// Parameters used by MvrVideo library
  myVideoParams.resize(Mvria::getMaxNumVideoDevices());
  for (size_t i = 0; i < Mvria::getMaxNumVideoDevices(); ++i)
    addVideoToConfig(i, this);  
}

MVREXPORT void MvrRobotParams::addLaserToConfig(int laserNumber, MvrConfig *config, bool useDefaultBehavior, const char *section)
{
  MvrConfigArg::RestartLevel restartLevel;
  // if we're using default behavior set if to none, since we can't change it
  if (useDefaultBehavior)
    restartLevel = MvrConfigArg::NO_RESTART;
    // otherwise make it restart the software
  else
    restartLevel = MvrConfigArg::RESTART_SOFTWARE;

  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL, section,
                     "Information about the connection to this laser and its position on the vehicle");
  LaserData *laserData = new LaserData;
  myLasers[laserNumber] = laserData;

  strcpy(laserData->mySection, section);

  std::string displayHintPlain    = "Visible:LaserAutoConnect = true";
  std::string displayHintCheckbox = displayHintPlain + "&&Checkbox";

  std::string displayHintCustom;

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf), "Laser_%d exists and should be automatically connected at startup.", laserNumber);
  displayHintCustom = "Checkbox";

  config->addParam(MvrConfigArg("LaserAutoConnect", &laserData->myLaserAutoConnect, tempDescBuf), 
                   section, MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);  

  config->addParam(MvrConfigArg("LaserX", &laserData->myLaserX,
                   "Location (in mm) of the laser in X(+ front, -back) relative to the robot's idealized center of the rotation.",
                   section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  config->addParam(MvrConfigArg("LaserY", &laserData->myLaserY,
                   "Location (in mm) of the laser in Y(+ front, -back) relative to the robot's idealized center of the rotation.",
                   section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  config->addParam(MvrConfigArg("LaserTh", &laserData->myLaserTh,
                   "Rotation (in deg) of the laser (+ counterclockwise, -clockwise)," -180.0, 180.0),
                   section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);  

  config->addParam(MvrConfigArg("LaserZ", &laserData->myLaserZ,
                   "Height (in mm) of the laser from the ground. 0 means unknown.", 0),
                   section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);  

  config->addParam(MvrConfigArg("LaserIgnore", &laserData->myLaserIgnore,
                   "Angles (in deg) at which to ignore readings, +/1 one degree. Angles are entered as strings, separated by a space.",
                   sizeof(laserData->myLaserIgnore)), section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  snprintf(tempDescBuf, sizeof(tempDescBuf), "Laser_%d is upside-down.", laserNumber);      

  config->addParam(MvrConfigArg("LaserFilpped", &laserData->myLaserFilpped, tempDescBuf),
                   section, MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
                   section, MvrPriority::FACTORY); 

  displayHintCustom = displayHintPlain + "&&" + Mvria::laserGetChoices();

  config->addParam(MvrConfigArg("LaserType", laserData->myLaserType,
                   "Type of laser.", sizeof(laserData->myLaserType)), 
                   section, MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel); 

  displayHintCustom = (displayHintPlain + "&&" + Mvria::deviceConnectionGetChoices());                   

  config->addParam(MvrConfigArg("LaserPortType", laserData->myLaserPortType,
                   "Type of port the laser is on.", sizeof(laserData->myLaserPortType)),  
          	       section, MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("LaserPort", laserData->myLaserPort,
                   "Port the laser is on.", sizeof(laserData->myLaserPort)), 
	                 section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);  

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;

  config->addParam(MvrConfigArg("LaserPowerOutput", laserData->myLaserPowerOutput,
                   "Power output that controls this laser's power.", sizeof(laserData->myLaserPowerOutput)), 
	                 section, MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);   

  config->addParam(MvrConfigArg("LaserStartingBaudChoice", laserData->myLaserStartingBaudChoice, 
                   "StartingBaud for this laser. Leave blank to use the default.", sizeof(laserData->myLaserStartingBaudChoice)),
	                 section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel); 
  config->addParam(MvrConfigArg("LaserAutoBaudChoice", laserData->myLaserAutoBaudChoice,
                   "AutoBaud for this laser. Leave blank to use the default.", sizeof(laserData->myLaserAutoBaudChoice)),
	                 section, MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR),
		               section, MvrPriority::FACTORY);     

  if (!useDefaultBehavior)  
    return;

  config->addParam(MvrConfigArg("LaserPowerControlled", &laserData->myLaserPowerControlled, 
                   "When enabled (true), this indicates that the power to the laser is controlled by the serial port line."), 
	                 section, MvrPriority::NORMAL); 
  config->addParam(MvrConfigArg("LaserMaxRange", (int *)&laserData->myLaserMaxRange, 
                   "Maximum range (in mm) to use for the laser. This should be specified only when the range needs to be shortened. 0 to use the default range."),
                   section,MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserCumulativeBufferSize", (int *)&laserData->myLaserCumulativeBufferSize, 
		      "Cumulative buffer size to use for the laser. 0 to use the default."), section, MvrPriority::NORMAL);
  
  config->addParam(MvrConfigArg("LaserStartDegrees", laserData->myLaserStartDegrees, 
		            "Start angle (in deg) for the laser. This may be used to constrain the angle. Fractional degrees are permitted. Leave blank to use the default.",
                sizeof(laserData->myLaserStartDegrees)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserEndDegrees", laserData->myLaserEndDegrees, 
		            "End angle (in deg) for the laser. This may be used to constrain the angle. Fractional degreees are permitted. Leave blank to use the default.",
                sizeof(laserData->myLaserEndDegrees)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserDegreesChoice", laserData->myLaserDegreesChoice, 
		            "Degrees choice for the laser. This may be used to constrain the range. Leave blank to use the default.",
                sizeof(laserData->myLaserDegreesChoice)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserIncrement", laserData->myLaserIncrement, 
		               "Increment (in deg) for the laser. Fractional degrees are permitted. Leave blank to use the default.",
                   sizeof(laserData->myLaserIncrement)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserIncrementChoice", laserData->myLaserIncrementChoice, 
		               "Increment choice for the laser. This may be used to increase the increment. Leave blank to use the default.",
                   sizeof(laserData->myLaserIncrementChoice)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserUnitsChoice", laserData->myLaserUnitsChoice,
                   "Units for the laser. This may be used to increase the size of the units. Leave blank to use the default.",
                   sizeof(laserData->myLaserUnitsChoice)), section, MvrPriority::NORMAL);

  config->addParam(MvrConfigArg("LaserReflectorBitsChoice", laserData->myLaserReflectorBitsChoice,
                   "ReflectorBits for the laser. Leave blank to use the default.",
                   sizeof(laserData->myLaserReflectorBitsChoice)), section, MvrPriority::NORMAL);
}

MVREXPORT void MvrRobotParams::addBatteryToConfig(int batteryNumber, MvrConfig* config, bool useDefaultBehavior)
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

  config->addParam(MvrConfigArg("BatteryAutoConnect", &batteryMTXBoardData->myBatteryMTXBoardAutoConn, tempDescBuf),
                   section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::batteryGetChoices();
  config->addParam(MvrConfigArg("BatteryType", batteryMTXBoardData->myBatteryMTXBoardType 
                   "Type of battery.", sizeof(batteryMTXBoardData->myBatteryMTXBoardType)),
                   section.c_str(), MvrPriority::FACTORY,
                   displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + Mvria::deviceConnectionGetChoices());
  config->addParam(MvrConfigArg("BatteryPortType", batteryMTXBoardData->myBatteryMTXBoardPortType, 
		               "Port type that the battery is on.", sizeof(batteryMTXBoardData->myBatteryMTXBoardPortType)), 
                   section.c_str(), MvrPriority::FACTORY,
                   displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("BatteryPort", batteryMTXBoardData->myBatteryMTXBoardPort, 
                   "Port the battery is on.", sizeof(batteryMTXBoardData->myBatteryMTXBoardPort)),
                   section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  config->addParam(MvrConfigArg("BatteryBaud", &batteryMTXBoardData->myBatteryMTXBoardBaud, 
                   "Baud rate to use for battery communication (9600, 19200, 38400, etc.)."), 
                   section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  //TODO remove this since it's redundant (it's set in the constructor)
  //batteryMTXBoardData->myBatteryMTXBoardAutoConn = false;

}

MVREXPORT void MvrRobotParams::addLCDToConfig(int lcdNumber, MvrConfig* config, bool useDefaultBehavior)
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
                     section.c_str(), "The physical definition of this LCD.");
  
  LCDMTXBoardData *lcdMTXBoardData = new LCDMTXBoardData;
  myLCDMTXBoards[lcdNumber] = lcdMTXBoardData;

  /// MPL TODO what's this for?
  myLCDMTXBoardCount++;

  std::string displayHintPlain = "Visible:LCDAutoConnect=true";
  std::string displayHintCheckbox = displayHintPlain + "&&Checkbox";

  std::string displayHintCustom;

  /// This is already set in the constructor
  //lcdMTXBoardData->myLCDMTXBoardAutoConn = false;
  displayHintCustom = "Checkbox&&Visible:Generation!=Legacy";

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf),
           "%s exists and should automatically be connected at startup.",
           lcdName.c_str());

  config->addParam(MvrConfigArg("LCDAutoConnect", &lcdMTXBoardData->myLCDMTXBoardAutoConn, tempDescBuf),
                   section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);
  /// This is already set in the constructor
  //lcdMTXBoardData->myLCDMTXBoardConnFailOption = false

  config->addParam(MvrConfigArg("LCDDisconnectOnConnectFailure", &lcdMTXBoardData->myLCDMTXBoardConnFailOption, 
                   "The LCD is a key component and is required for operation. If this is enabled and there is a failure in the LCD communications, then the robot will restart."),
                   section.c_str(), MvrPriority::FACTORY, displayHintCheckbox.c_str(), restartLevel);
  
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::lcdGetChoices();
  config->addParam(MvrConfigArg("LCDType", lcdMTXBoardData->myLCDMTXBoardType, 
                   "Type of LCD.", sizeof(lcdMTXBoardData->myLCDMTXBoardType)), 
                   section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + Mvria::deviceConnectionGetChoices());
  config->addParam(MvrConfigArg("LCDPortType", lcdMTXBoardData->myLCDMTXBoardPortType, 
                   "Port type that the LCD is on.", sizeof(lcdMTXBoardData->myLCDMTXBoardPortType)), 
                   section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("LCDPort", lcdMTXBoardData->myLCDMTXBoardPort, 
                   "Port that the LCD is on.", sizeof(lcdMTXBoardData->myLCDMTXBoardPort)), 
                   section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;
  config->addParam(MvrConfigArg("LCDPowerOutput", lcdMTXBoardData->myLCDMTXBoardPowerOutput, 
                   "Power output that controls this LCD's power.", sizeof(lcdMTXBoardData->myLCDMTXBoardPowerOutput)), 
                   section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("LCDBaud", &lcdMTXBoardData->myLCDMTXBoardBaud, 
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
  
  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL, section.c_str(), "Information about the connection to this Sonar Board.");

  SonarMTXBoardData *sonarMTXBoardData = new SonarMTXBoardData;
  mySonarMTXBoards[sonarBoardNumber] = sonarMTXBoardData;

  /// MPL TODO what's this do?
  mySonarMTXBoardCount++;

  /// MPL TODO remove this next line (it's in the constructor
  //sonarMTXBoardData->mySonarMTXBoardAutoConn = false;
  displayHintCustom = "Checkbox&&Visible:Generation!=Legacy";

  char tempDescBuf[512];
  snprintf(tempDescBuf, sizeof(tempDescBuf), "%s exists and should be automatically connected at startup.", sonarBoardName.c_str());
 
  config->addParam(MvrConfigArg("SonarAutoConnect", &sonarMTXBoardData->mySonarMTXBoardAutoConn,tempDescBuf),
	                 section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);
  
  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), section.c_str(), MvrPriority::FACTORY);

  displayHintCustom = displayHintPlain + "&&" + Mvria::sonarGetChoices();
  config->addParam(MvrConfigArg("SonarBoardType", sonarMTXBoardData->mySonarMTXBoardType, "Type of the sonar board.", 
		sizeof(sonarMTXBoardData->mySonarMTXBoardType)), 
	  section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  displayHintCustom = (displayHintPlain + "&&" + Mvria::deviceConnectionGetChoices());
  config->addParam(MvrConfigArg("SonarBoardPortType", sonarMTXBoardData->mySonarMTXBoardPortType, "Port type that the sonar is on.", sizeof(sonarMTXBoardData->mySonarMTXBoardPortType)),
	  section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("SonarBoardPort", sonarMTXBoardData->mySonarMTXBoardPort, "Port the sonar is on.", sizeof(sonarMTXBoardData->mySonarMTXBoardPort)), 
	  section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  if (!ourPowerOutputDisplayHint.empty())
    displayHintCustom = displayHintPlain + "&&" + ourPowerOutputDisplayHint;
  else
    displayHintCustom = displayHintPlain;
  config->addParam(
	  MvrConfigArg("SonarBoardPowerOutput", sonarMTXBoardData->mySonarMTXBoardPowerOutput, "Power output that controls this Sonar Board's power.", sizeof(sonarMTXBoardData->mySonarMTXBoardPowerOutput)), 
	  section.c_str(), MvrPriority::FACTORY, displayHintCustom.c_str(), restartLevel);

  config->addParam(MvrConfigArg("SonarBaud", &sonarMTXBoardData->mySonarMTXBoardBaud, 
                   "Baud rate for the sonar board communication. (9600, 19200, 38400, etc.)."), 
	                 section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  /// Remove this, it's in the constructor
  //  sonarMTXBoardData->mySonarDelay = 1;
  config->addParam(MvrConfigArg("SonarDelay", &sonarMTXBoardData->mySonarDelay, "Sonar delay (in ms).", 0, 10), 
	                 section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  /// MPL Remove this sonar gain, it's in the constructor
  //sonarMTXBoardData->mySonarGain = 5;
  config->addParam(MvrConfigArg("SonarGain", &sonarMTXBoardData->mySonarGain, "Default sonar gain for the board, range 0-31.", 0, 31), 
	                 section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  // MPL TODO remove (moved this to constructor)
  //strcpy(&sonarMTXBoardData->mySonarThreshold[0],"3000|1500|2000");
  config->addParam(MvrConfigArg("SonarDetectionThreshold", &sonarMTXBoardData->mySonarDetectionThreshold, "Default sonar detection threshold for the board.",0, 65535),
                   section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

	/*
  config->addParam(MvrConfigArg("SonarNoiseDelta", &sonarMTXBoardData->mySonarNoiseDelta, "Default sonar noise delta for the board.",0, 65535),
                   section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);
	*/

  config->addParam(MvrConfigArg("SonarMaxRange", &sonarMTXBoardData->mySonarMaxRange, "Default maximum sonar range for the board.",0, 255*17),
	                 section.c_str(), MvrPriority::FACTORY, displayHintPlain.c_str(), restartLevel);

  
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