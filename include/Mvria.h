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
#ifndef ARIA_H
#define ARIA_H

#include "mvriaOSDef.h"
#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h" //
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
#include "mvriaInternal.h"
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
#include "MvrActionKeydrive.h"
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
#if !defined(WIN32) && !defined(SWIGWIN)
#include "MvrVersalogicIO.h"
#include "MvrMTXIO.h"
#endif
#include "MvrActionGotoStraight.h"
#include "MvrDataLogger.h"
#include "MvrRobotJoyHandler.h"
#include "MvrRatioInputKeydrive.h"
#include "MvrRatioInputJoydrive.h"
#include "MvrRatioInputRobotJoydrive.h"
#include "MvrActionMovementParameters.h"
#include "MvrSoundPlayer.h"
#include "MvrSoundsQueue.h"
#include "MvrCameraCollection.h"
#include "MvrCameraCommands.h"
#include "MvrStringInfoGroup.h"
#include "MvrSonarAutoDisabler.h"
#include "MvrActionDriveDistance.h"
#include "MvrLaserReflectorDevice.h"
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
#include "MvrActionLimiterRot.h"
#include "MvrRobotBatteryPacketReader.h"
#include "MvrLMS1XX.h"
#include "MvrUrg_2_0.h"
#include "MvrActionMovementParametersDebugging.h"
#include "MvrZippable.h"
#include "MvrS3Series.h"
#include "MvrSZSeries.h"
#include "MvrRobotPacketReaderThread.h"
#include "MvrHasFileName.h"

#endif // ARIA_H
