/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : Mvria.h
 > Description  : Some head file 
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/

#ifndef MVRIA_H
#define MVRIA_H

#include "mvriaOSDef.h"
#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrSimpleConnector.h"
#include "MvrLogFileConnection.h"
#include "MvrLog.h"
#include "MvrRobotPacket.h"
#include "MvrRobotPacketSender.h"
#include "MvrRobotPacketReceiver.h"
#include "MvrRobotConfigPacketReader.h"
#include "MvrRobotTypes.h"
#include "mvriaUtil.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "MvrFileParser.h"
#include "MvrConfig.h"
#include "MvrConfigArg.h"
#include "MvrConfigGroup.h"
#include "MvrRobot.h"
#include "MvrCommands.h"
#include "MvrJoyHandler.h"
#include "MvrSyncTask.h"
#include "MvrTaskState.h"
#include "MvriaInternal.h"
#include "MvrSonarDevice.h"
#include "MvrPriorityResolver.h"
#include "MvrAction.h"
#include "MvrActionGroup.h"
#include "MvrActionGroups.h"
#include "MvrActionDeceleratingLimiter.h"
#include "MvrActionLimiterForwards.h"
#include "MvrActionLimiterBackwards.h"
#include "MvrActionLimiterTableSensor.h"
#include "MvrActionBumpers.h"
#include "MvrActionIRs.h"
#include "MvrActionStallRecover.h"
#include "MvrActionAvoidFront.h"
#include "MvrActionAvoidSide.h"
#include "MvrActionConstantVelocity.h"
#include "MvrActionInput.h"
#include "MvrActionRobotJoydrive.h"
#include "MvrActionJoydrive.h"
#include "MvrActionKeyDrive.h"
#include "MvrActionTriangleDriveTo.h"
#include "MvrActionTurn.h"
#include "MvrActionRatioInput.h"
#include "MvrActionStop.h"
#include "MvrActionGoto.h"
#include "MvrModule.h"
#include "MvrModuleLoader.h"
#include "MvrRecurrentTask.h"
#include "MvrInterpolation.h"
#include "MvrGripper.h"
#include "MvrSonyPTZ.h"
#include "MvrRVisionPTZ.h"
#include "MvrAMPTU.h"
#include "MvrP2Arm.h"
#include "MvrACTS.h"
#include "MvrSick.h"
#include "MvrLaserLogger.h"
#include "MvrIrrfDevice.h"
#include "MvrKeyHandler.h"
#include "MvrDPPTU.h"
#include "MvrVCC4.h"
#include "MvrMode.h"
#include "MvrModes.h"
#include "MvrNetServer.h"
#include "MvrSignalHandler.h"
#include "MvrAnalogGyro.h"
#include "MvrMapInterface.h"
#include "MvrMapObject.h"
#include "MvrMap.h"
#include "MvrLineFinder.h"
#include "MvrBumpers.h"
#include "MvrIRs.h"
#include "MvrDrawingData.h"
#include "MvrForbiddenRangeDevice.h"
#include "MvrTCM2.h"
#if !define(WIN32) && !defined(SWIGWIN)
#include "MvrVersalogicIO.h"
#include "MvrMTXIO.h"
#endif  // WIN32
#include "MvrActionGotoStraight.h"
#include "MvrDataLogger.h"
#include "MvrRobotJoyHandler.h"
#include "MvrRatioInputKeydrive.h"
#include "MvrRatioInputJoydrive.h"
#include "MvrRatioInputRobotJoydrive.h"
#include "MvrActionMovementParameters.h"
#include "MvrSoundPlayer.h"
#include "MvrSoundQueue.h"
#include "MvrCameraCollection.h"
#include "MvrCameraCommands.h"
#include "MvrStringInfoGroup.h"
#include "MvrSonarAutoDisabler.h"
#include "MvrActionDriveDistance.h"
#include "MvrLaserRelectorDriver.h"
#include "MvrRobotConfig.h"
#include "MvrTCMCompassRobot.h"
#include "MvrTCMCompassDirect.h"
#include "MvrLaserFilter.h"
#include "MvrUrg.h"
#include "MvrSpeech.h"
#include "MvrGPS.h"
#include "MvrTrimbleGPS.h"
#include "MvrNovatelGPS.h"
#include "MvrGPSCoords.h"
#include "MvrLaser.h"
#include "MvrRobotConnector.h"
#include "MvrLaserConnector.h"
#include "MvrSonarConnector.h"
#include "MvrBatteryConnector.h"
#include "MvrLCDConnector.h"
#include "MvrSonarMTX.h"
#include "MvrBatteryMTX.h"
#include "MvrLCDMTX.h"
#include "MvrSimulatedLaser.h"
#include "MvrExitErrorSource.h"
#include "MvrActionLimterRot.h"
#include "MvrRobotBatteryPacketReader.h"
#include "MvrLMS1XX.h"
#include "MvrUrg_2_0.h"
#include "MvrActionMovementParametersDebuggering.h"
#include "MvrZippable.h"
#include "MvrS3Series.h"
#include "MvrSZSeries.h"
#include "MvrRobotPacketReaderThread.h"
#include "MvrHasFileName.h"

/// TODO
#endif  // MVRIA_H