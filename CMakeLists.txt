CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MVR)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native ")
SET(CMAKE_EXE_LINKER_FLAGS "-lpthread -lrt -ldl")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\ -O3 -march=native")
# Check C++11 or C++0x support
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -lrt -ldl")
    add_definitions(-DCOMPILEDWITHC11)
    message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -lpthread -lrt -ldl")
    add_definitions(-DCOMPILEDWITHC0X)
    message(STATUS "Using flag -std=c++0x.")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
SET(RUNTIME_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include
)

# ADD_LIBRARY(Mvria SHARED
# 	src/MvrAction.cpp
# 	src/MvrActionAvoidFront.cpp
# 	src/MvrActionAvoidSide.cpp
# 	src/MvrActionBumpers.cpp
# 	src/MvrActionColorFollow.cpp
# 	src/MvrActionConstantVelocity.cpp
# 	src/MvrActionDeceleratingLimiter.cpp
# 	src/MvrActionDesired.cpp
# 	src/MvrActionDriveDistance.cpp
# 	src/MvrActionGoto.cpp
# 	src/MvrActionGotoStraight.cpp
# 	src/MvrActionGroup.cpp
# 	src/MvrActionGroups.cpp
# 	src/MvrActionInput.cpp
# 	src/MvrActionIRs.cpp
# 	src/MvrActionJoydrive.cpp
# 	src/MvrActionKeydrive.cpp
# 	src/MvrActionLimiterBackwards.cpp
# 	src/MvrActionLimiterForwards.cpp
# 	src/MvrActionLimiterRot.cpp
# 	src/MvrActionLimiterTableSensor.cpp
# 	src/MvrActionMovementParameters.cpp
# 	src/MvrActionMovementParametersDebugging.cpp
# 	src/MvrActionRatioInput.cpp
# 	src/MvrActionRobotJoydrive.cpp
# 	src/MvrActionStallRecover.cpp
# 	src/MvrActionStop.cpp
# 	src/MvrActionTriangleDriveTo.cpp
# 	src/MvrActionTurn.cpp
# 	src/MvrACTS.cpp
# 	src/MvrAMPTU.cpp
# 	src/MvrAnalogGyro.cpp
# 	src/MvrArg.cpp
# 	src/MvrArgumentBuilder.cpp
# 	src/MvrArgumentParser.cpp
# 	src/MvrASyncTask.cpp
# 	src/MvrBasePacket.cpp
# 	src/MvrBatteryConnector.cpp
# 	src/MvrBatteryMTX.cpp
# 	src/MvrBumpers.cpp
# 	src/MvrCameraCommands.cpp
# 	src/MvrCameraCollection.cpp
# 	src/MvrCondition.cpp
# 	src/MvrConfig.cpp
# 	src/MvrConfigArg.cpp
# 	src/MvrConfigGroup.cpp
# 	src/MvrDataLogger.cpp
# 	src/MvrDeviceConnection.cpp
# 	src/MvrDPPTU.cpp
#   src/MvrFileDeviceConnection.cpp
# 	src/MvrFileParser.cpp
# 	src/MvrForbiddenRangeDevice.cpp
# 	src/MvrFunctorASyncTask.cpp
# 	src/MvrGPS.cpp
# 	src/MvrGPSConnector.cpp
# 	src/MvrGPSCoords.cpp
# 	src/MvrGripper.cpp
# 	src/MvrInterpolation.cpp
# 	src/MvrIrrfDevice.cpp
# 	src/MvrIRs.cpp
# 	src/MvrJoyHandler.cpp
# 	src/MvrKeyHandler.cpp
# 	src/MvrLaser.cpp
# 	src/MvrLaserConnector.cpp
# 	src/MvrLaserFilter.cpp
# 	src/MvrLaserLogger.cpp
# 	src/MvrLaserReflectorDevice.cpp
# 	src/MvrLCDConnector.cpp
# 	src/MvrLCDMTX.cpp
# 	src/MvrLineFinder.cpp
# 	src/MvrLMS1XX.cpp
# 	src/MvrLMS2xx.cpp
# 	src/MvrLMS2xxPacket.cpp
# 	src/MvrLMS2xxPacketReceiver.cpp
# 	src/MvrLog.cpp
# 	src/MvrLogFileConnection.cpp
# 	src/MvrMap.cpp
# 	src/MvrMapComponents.cpp
# 	src/MvrMapInterface.cpp
# 	src/MvrMapObject.cpp
# 	src/MvrMapUtils.cpp
# 	src/MvrMD5Calculator.cpp
# 	src/MvrMode.cpp
# 	src/MvrModes.cpp
# 	src/MvrModule.cpp
# 	src/MvrModuleLoader.cpp
# 	src/MvrMutex.cpp
# 	src/MvrMutex.cpp
# 	src/MvrNetServer.cpp
# 	src/MvrNMEAParser.cpp
# 	src/MvrNovatelGPS.cpp
# 	src/MvrP2Arm.cpp
# 	src/MvrPriorityResolver.cpp
# 	src/MvrPTZ.cpp
#   src/MvrPTZConnector.cpp
# 	src/MvrRangeBuffer.cpp
# 	src/MvrRangeDevice.cpp
# 	src/MvrRangeDeviceThreaded.cpp
# 	src/MvrRatioInputKeydrive.cpp
# 	src/MvrRatioInputJoydrive.cpp
# 	src/MvrRatioInputRobotJoydrive.cpp
# 	src/MvrRecurrentTask.cpp
# 	src/MvrRobot.cpp
# 	src/MvrRobotBatteryPacketReader.cpp
# 	src/MvrRobotConfig.cpp
# 	src/MvrRobotConfigPacketReader.cpp
# 	src/MvrRobotConnector.cpp
# 	src/MvrRobotJoyHandler.cpp
# 	src/MvrRobotPacket.cpp
# 	src/MvrRobotPacketReceiver.cpp
# 	src/MvrRobotPacketReaderThread.cpp
# 	src/MvrRobotPacketSender.cpp
# 	src/MvrRobotParams.cpp
# 	src/MvrRobotTypes.cpp
# 	src/MvrRVisionPTZ.cpp
# 	src/MvrS3Series.cpp
# 	src/MvrSZSeries.cpp
# 	src/MvrSick.cpp
# 	src/MvrSimpleConnector.cpp
# 	src/MvrSimulatedLaser.cpp
# 	src/MvrSocket.cpp
# 	src/MvrSonarConnector.cpp
# 	src/MvrSonarDevice.cpp
# 	src/MvrSonarMTX.cpp
# 	src/MvrSensorReading.cpp
# 	src/MvrSonyPTZ.cpp
# 	src/MvrSoundsQueue.cpp
# 	src/MvrSoundPlayer.cpp
# 	src/MvrStringInfoGroup.cpp
# 	src/MvrSyncLoop.cpp
# 	src/MvrSyncTask.cpp
# 	src/MvrSystemStatus.cpp
# 	src/MvrSonarAutoDisabler.cpp
# 	src/MvrSpeech.cpp
# 	src/MvrTCM2.cpp
# 	src/MvrTCMCompassDirect.cpp
# 	src/MvrTCMCompassRobot.cpp
# 	src/MvrTcpConnection.cpp
# 	src/MvrThread.cpp
# 	src/MvrTransform.cpp
# 	src/MvrTrimbleGPS.cpp
# 	src/MvrUrg.cpp
# 	src/MvrUrg_2_0.cpp
# 	src/MvrVCC4.cpp
# 	src/MvrSerialConnection.cpp
# 	src/MvrSignalHandler.cpp
# 	src/Mvria.cpp
# 	src/mvriaUtil.cpp
# 	src/md5.cpp
#     )


ADD_EXECUTABLE(testMutexLockWarning tests/testMutexLockWarning.cpp)
target_link_libraries(testMutexLockWarning ${LIBRARY_OUTPUT_PATH}/libMvria.so -ldl)