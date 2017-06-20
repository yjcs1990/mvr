/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotPacketSender.cpp
 > Description  : Given a device connection which sends commands through it to the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrDeviceConnection.h"
#include "MvrRobotPacketSender.h"

/*
 * Use setDeviceConnection() to set the device connection before use.
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 */
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(unsigned char sync1, unsigned char sync2) :
          myPacket(sync1, sync2)
{
  myDeviceConn = NULL;
  myTracking   = false;
  myTrackingLogName.clear();
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
} 

/*
 * @param deviceConnection device connection to send packets to
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 */
MVREXPORT MvrRobotPacket::MvrRobotPacket(MvrDeviceConnection *deviceConnection,unsigned char sync1, unsigned char sync2) :
          myPacket(sync1, sync2)
{
  myDeviceConn = deviceConnection;
  myTracking   = false;
  myTrackingLogName.clear();
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
}                                                   

/*
 * @param deviceConnection device connection to send packets to
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
*/
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(MvrDeviceConnection *deviceConnction, unsigned char sync1, unsigned char sync2) :
          myPacket(sync1, sync2)
{
  myDeviceConn = deviceConnction;
  myTracking   = false;
  myTrackingLogName.clear();
  mySendingMutex.setLogName("MvrRobotPacketSender");
  myPacketSentCallback = NULL;
}          

/*
 * @param deviceConnection device connection to send packets to
 * @param sync1 first byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param sync2 second byte of the header this sender will send, this 
 * should be left as the default in nearly all cases, or it won't work with any
 * production robot. ie don't mess with it
 * @param tracking if true write packet-tracking log messages for each packet sent.
 * @param trackingLogName name (packet type) to include in packet-tracking log messages 
 */
MVREXPORT MvrRobotPacketSender::MvrRobotPacketSender(MvrDeviceConnection *deviceConnction,)