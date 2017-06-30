CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MVR)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3 -march=native")
# Check C++11 or C++0x support
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    add_definitions(-DCOMPILEDWITHC11)
    message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
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

ADD_LIBRARY(Mvria SHARED
	src/MvrAction.cpp 
	src/MvrArg.cpp 
	src/MvrArgumentBuilder.cpp 
	src/MvrArgumentParser.cpp 
	src/MvrASyncTask.cpp 
	src/MvrBasePacket.cpp
	src/MvrCondition.cpp 
	src/MvrConfig.cpp
	src/MvrConfigArg.cpp
	src/MvrConfigGroup.cpp
	src/MvrDataLogger.cpp
	src/MvrDeviceConnection.cpp
    src/MvrFileDeviceConnection.cpp
	src/MvrFileParser.cpp
	src/MvrFunctorASyncTask.cpp
	src/MvrInterpolation.cpp
	src/MvrJoyHandler.cpp
	src/MvrKeyHandler.cpp
	src/MvrLog.cpp
	src/MvrLogFileConnection.cpp
	src/MvrMode.cpp
	src/MvrModes.cpp
	src/MvrModule.cpp
	src/MvrModuleLoader.cpp
	src/MvrMutex.cpp
	src/MvrRobot.cpp
	src/MvrRobotBatteryPacketReader.cpp
	src/MvrRobotConfig.cpp
	src/MvrRobotConfigPacketReader.cpp
	src/MvrRobotConnector.cpp
	src/MvrRobotJoyHandler.cpp
	src/MvrRobotPacket.cpp
	src/MvrRobotPacketReceiver.cpp
	src/MvrRobotPacketReaderThread.cpp
	src/MvrRobotPacketSender.cpp
	src/MvrRobotParams.cpp
	src/MvrRobotTypes.cpp
	src/MvrSimpleConnector.cpp
	src/MvrSocket.cpp
	src/MvrStringInfoGroup.cpp
	src/MvrSyncLoop.cpp
	src/MvrSyncTask.cpp
	src/MvrSystemStatus.cpp
	src/MvrThread.cpp
	src/MvrTransform.cpp
	src/Mvria.cpp
	src/mvriaUtil.cpp
	src/md5.cpp
    )

# ADD_EXECUTABLE(MVRTest tests/test.cpp)
ADD_EXECUTABLE(timeTest tests/timeTest.cpp)
target_link_libraries(timeTest ${LIBRARY_OUTPUT_PATH}/libMvria.so)