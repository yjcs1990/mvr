#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLCDMTX.h"
#include "MvrSensorReading.h"
#include "mvriaOSDef.h"
#include "MvrSerialConnection.h"
#include "mvriaInternal.h"
#include "MvrSystemStatus.h"

#include <time.h>
//#include "../MvrNetworking/include/MvrServerMode.h"

#include <sys/types.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include <ctype.h>


#define MVRLCDMTXDEBUG
#if (defined(ARLCDMTXDEBUG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif


std::string MvrLCDMTX::ourFirmwareBaseDir("/usr/local/apps/marcDownload/");

MVREXPORT MvrLCDMTX::MvrLCDMTX(int lcdBoardNum, const char *name,
	MvrDeviceConnection *conn,
	MvrRobot *robot) :
	mySensorInterpTask(this, &MvrLCDMTX::sensorInterp),
	myConn(conn),
	myName(name),
	myBoardNum(lcdBoardNum),
	myConnFailOption(false),
	myFirmwareVersion(""),
	myMvrExitCB(this, &MvrLCDMTX::disconnect)
{

	myInfoLogLevel = MvrLog::Normal;
	clear();
	setRobot(robot);
	myRobotIdentifierChanged = false;
	myRobotIdentifier[0] = '\0';

	mySendTracking = false;
	myRecvTracking = false;

	myLoggedBatteryError = false;

	// now from the robot params - read each unit
	// searching for the units configured for this
	// board, then create the unit map
	if (robot && robot->getRobotParams()) {
		MvrLog::log(MvrLog::Normal, "%s::MvrLCDMTX LCD board %d params",
			getName(), myBoardNum);
	}
	Mvria::addExitCallback(&myMvrExitCB, -10);
	//myLogLevel = MvrLog::Verbose;
	//myLogLevel = MvrLog::Terse;
	myLogLevel = MvrLog::Normal;
	myRobotRunningAndConnected = false;

}


MVREXPORT MvrLCDMTX::~MvrLCDMTX()
{
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&myProcessCB);
	}
}


MVREXPORT void MvrLCDMTX::setDeviceConnection(
	MvrDeviceConnection *conn)
{
	myConn = conn;
	myConn->setDeviceName(getName());
}

MVREXPORT MvrDeviceConnection *MvrLCDMTX::getDeviceConnection(void)
{
	return myConn;
}


MVREXPORT void MvrLCDMTX::setRobot(MvrRobot *robot)
{
	myRobot = robot;

	if ((robot != NULL) && (robot->getRobotParams())) {

		myConnFailOption = robot->getRobotParams()->getLCDMTXBoardConnFailOption(myBoardNum);
	}

	// this is the code from the laser, i changed the priority to 20 from 90
	// also it puts in mysensorinterptask instead of myprocesscb
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&mySensorInterpTask);
		myRobot->addSensorInterpTask(myName.c_str(), 20, &mySensorInterpTask);
	}
}

void MvrLCDMTX::clear(void)
{
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;
}

MVREXPORT void MvrLCDMTX::lcdSetName(const char *name)
{
	myName = name;
	myDeviceMutex.setLogNameVar("%s::myDeviceMutex", getName());
	myPacketsMutex.setLogNameVar("%s::myPacketsMutex", getName());
	myDataMutex.setLogNameVar("%s::myDataMutex", getName());
	myMvrExitCB.setNameVar("%s::exitCallback", getName());
	myDisconnectOnErrorCBList.setNameVar(
		"%s::myDisconnectOnErrorCBList", myName.c_str());

}

MVREXPORT bool MvrLCDMTX::disconnect(void)
{
	if (!isConnected())
		return true;
	MvrLog::log(MvrLog::Normal, "%s: Disconnecting", getName());
	return true;
}

MVREXPORT int MvrLCDMTX::getReadingCount()
{
	if (myTimeLastReading == time(NULL))
		return myReadingCount;
	if (myTimeLastReading == time(NULL) - 1)
		return myReadingCurrentCount;
	return 0;
}

MVREXPORT void MvrLCDMTX::internalGotReading(void)
{
	if (myTimeLastReading != time(NULL))
	{
		myTimeLastReading = time(NULL);
		myReadingCount = myReadingCurrentCount;
		myReadingCurrentCount = 0;
	}
	myReadingCurrentCount++;

	myLastReading.setToNow();

}

void MvrLCDMTX::failedToConnect(void)
{
	MvrLog::log(MvrLog::Normal,
		"%s:failedToConnect Cound not connect to lcd",
		getName());
	myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();
}

void MvrLCDMTX::sensorInterp(void)
{
	//MvrLCDMTXPacket *packet;
	MvrRobotPacket *packet;

	while (1) {
		myPacketsMutex.lock();
		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			return;
		}

		packet = myPackets.front();
		myPackets.pop_front();
		myPacketsMutex.unlock();
		unsigned char *buf = (unsigned char *)packet->getBuf();

		if (packet->getID() != KEEP_ALIVE) {
			MvrLog::log(MvrLog::Normal,
				"%s:sensorInterp Could not process packet, command invalid 0x%02x of %d length",
				getName(), packet->getID(), packet->getLength());

			delete packet;
			continue;
		}

		myLastReading.setToNow();
		internalGotReading();

		MvrTime time = packet->getTimeReceived();

#if 0 // for raw trace

		char obuf[256];
		obuf[0] = '\0';
		int j = 0;
		for (int i = 0; i < packet->getLength() - 2; i++) {
			sprintf(&obuf[j], "_%02x", buf[i]);
			j = j + 3;
		}
		MvrLog::log(MvrLog::Normal,
			"%s::sensorInterp() packet = %s", getName(), obuf);
#endif

		myDeviceMutex.lock();

		myDeviceMutex.unlock();
		delete packet;

	} // end while
}


MVREXPORT bool MvrLCDMTX::blockingConnect(bool sendTracking, bool recvTracking,
	int lcdNumber, MvrFunctor1<int> *onCallback,
	MvrFunctor1<int> *offCallback)
{

	mySendTracking = sendTracking;
	myRecvTracking = recvTracking;


	myDeviceMutex.lock();
	if (myConn == NULL) {
		MvrLog::log(MvrLog::Terse,
			"%s: Could not connect because there is no connection defined",
			getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}
	MvrSerialConnection *serConn = NULL;
	serConn = dynamic_cast<MvrSerialConnection *> (myConn);
	if (serConn != NULL)
		serConn->setBaud(115200);
	if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN
		&& !myConn->openSimple()) {
		MvrLog::log(
			MvrLog::Terse,
			"%s: Could not connect because the connection was not open and could not open it",
			getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}

	//myRecvTracking = true;
	//mySendTracking = true;
	myReceiver = new MvrRobotPacketReceiver(myConn, true, HEADER1, HEADER2,
		myRecvTracking,
		"MvrLCDMTX");

	mySender = new MvrRobotPacketSender(myConn, HEADER1, HEADER2,
		mySendTracking,
		"MvrLCDMTX");

	myTryingToConnect = true;
	myDeviceMutex.unlock();
	MvrTime timeDone;
	if (!timeDone.addMSec(30 * 1000)) {
		MvrLog::log(MvrLog::Normal,
			"%s::blockingConnect() error adding msecs (30 * 1000)",
			getName());
	}

	bool stopSent = false;
	bool versionVerified = false;
	bool sysInfoVerified = false;
	MvrRobotPacket *packet;

	do {

		// first have the lcd stop sending
		// ???? not sure if there's a response to this
		if ((!stopSent) && (!sendSystemInfo(SYS_INFO_STOP))) {
			MvrLog::log(MvrLog::Normal,
				"%s::blockingConnect() Could not send system info request to LCD", getName());
			failedToConnect();
			return false;
		}

		stopSent = true;

		MvrUtil::sleep(300);

		if (!versionVerified) {
			if (!sendVersion()) {
				MvrLog::log(MvrLog::Normal,
					"%s::blockingConnect() Could not send version request to LCD", getName());
				failedToConnect();
				return false;
			}

			packet = myReceiver->receivePacket(1000);

			if (packet == NULL) {
				MvrLog::log(MvrLog::Normal, "%s::blockingConnect() Did not get response to version request (%d) - resending",
					getName(), timeDone.mSecTo());
				continue;
			}

			unsigned char *versionBuf = (unsigned char *)packet->getBuf();

			// verify get num trans received
			if (versionBuf[3] != VERSION)  {

				MvrLog::log(MvrLog::Normal,
					"%s::blockingConnect() Invalid response from lcd to send version (0x%x)",
					getName(), versionBuf[3]);
				delete packet;
				continue;
			}
			else {

				char tempBuf[256];

				snprintf(tempBuf, sizeof(tempBuf), "%s", &versionBuf[4]);
				myFirmwareVersion = tempBuf;

				MvrLog::log(MvrLog::Normal, "%s::blockingConnect() LCD firmware version = %s",
					getName(), myFirmwareVersion.c_str());
				delete packet;

			}

			// if the version dosn't verify - we'll need to download new firmware

			if (verifyFwVersion()) {

				// power cycle the HMI

				if (offCallback != NULL)
					offCallback->invoke(lcdNumber);

				MvrUtil::sleep(3000);

				if (onCallback != NULL)
					onCallback->invoke(lcdNumber);

				downloadFirmware();

			}
		}
		versionVerified = true;

		// send a system info

		if (!sysInfoVerified) {
			if (!sendSystemInfo(SYS_INFO_ONCE)) {
				MvrLog::log(MvrLog::Normal,
					"%s::blockingConnect() Could not send sys info request to LCD", getName());
				failedToConnect();
				return false;
			}

			packet = myReceiver->receivePacket(1000);

			if (packet == NULL) {
				MvrLog::log(MvrLog::Normal, "%s::blockingConnect() Did not get response to sys info request (%d) - resending",
					getName(), timeDone.mSecTo());
				continue;
			}

			unsigned char *sysInfoBuf = (unsigned char *)packet->getBuf();

			// verify get num trans received
			if (sysInfoBuf[3] != SYSTEM_INFO)  {

				MvrLog::log(MvrLog::Normal,
					"%s::blockingConnect() Invalid response from lcd to send system info (0x%x)",
					getName(), sysInfoBuf[3]);
				delete packet;
				continue;
			}
			else {

				myStatusFlags = sysInfoBuf[4];
				myCurrentScreen = sysInfoBuf[5];

				MvrLog::log(MvrLog::Normal, "%s::blockingConnect() LCD status flag = %d current screen = %d",
					getName(), myStatusFlags, myCurrentScreen);
				delete packet;

			}
		}

		sysInfoVerified = true;

		if (!sendKeepAlive()) {
			MvrLog::log(MvrLog::Normal,
				"%s::blockingConnect() Could not send keep alive request to LCD", getName());
			failedToConnect();
			return false;
		}

		packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal, "%s::blockingConnect() Did not get response to keep alive request (%d) - resending",
				getName(), timeDone.mSecTo());
			continue;
		}

		unsigned char keepAliveCmd = packet->bufToByte();

		// verify 
		if (keepAliveCmd != KEEP_ALIVE) {
			MvrLog::log(MvrLog::Normal,
				"%s::blockingConnect() Invalid response from lcd to send keep alive (0x%x)",
				getName(), keepAliveCmd);
			delete packet;
			continue;
		}
		else {
			delete packet;
			myIsConnected = true;
			myTryingToConnect = false;
			MvrLog::log(MvrLog::Normal, "%s::blockingConnect() Connection successful",
				getName());

			myLastReading.setToNow();

			runAsync();
			return true;
		}
	} while (timeDone.mSecTo() >= 0);

	MvrLog::log(MvrLog::Normal,
		"%s::blockingConnect() Could not send keep alive Request to LCD", getName());
	failedToConnect();
	return false;

} // end blockingConnect

MVREXPORT const char * MvrLCDMTX::getName(void) const
{
	return myName.c_str();
}

MVREXPORT void * MvrLCDMTX::runThread(void *arg)
{


	MvrRobotPacket *packet;

	// initialize the variouse settable values
	myScreenNumberChanged = false;
	myBootTextChanged = false;
	myChangedBootText[0] = '\0';
	myMainScreenStatusChanged = false;
	myChangedStatusText[0] = '\0';
	myMainScreenModeChanged = false;
	myChangedModeText[0] = '\0';
	myBackLightChanged = false;
	myIpAddress = "0.0.0.0";

	getIpAddress();

	setRobotIPStatus(myIpAddress.c_str());
	MvrUtil::sleep(300);

	/* test code
	std::string buf = "going to goal x";
	std::string buf1 = "goal seeking";
	std::string buf2 = "Aram is startup up...";

	setMTXLCDMainScreenStatus(buf.c_str());

	setMTXLCDMainScreenMode(buf1.c_str());

	setMTXLCDBootScreenText(buf2.c_str());

	setMTXLCDBackLight(true);

	setMTXLCDScreenNumber(1);
	*/

	while (getRunning())
	{

		if (myRobotIdentifierChanged) {
			setRobotIdStatus(myRobotIdentifier.c_str());
			myRobotIdentifierChanged = false;
			MvrUtil::sleep(300);
		}

		if (myScreenNumberChanged) {
			setScreenNumber(myChangedScreenNumber);
			myScreenNumberChanged = false;
			MvrUtil::sleep(300);
		}

		if (myBackLightChanged) {
			setBacklight(myChangedBackLight);
			myBackLightChanged = false;
			MvrUtil::sleep(300);
		}

		if (myBootTextChanged) {
			setBootStatus(myChangedBootText);
			myBootTextChanged = false;
			myChangedBootText[0] = '\0';
			MvrUtil::sleep(300);
		}

		if (myMainScreenStatusChanged) {
			setMainStatus(myChangedStatusText);
			myMainScreenStatusChanged = false;
			myChangedStatusText[0] = '\0';
			MvrUtil::sleep(300);
		}

		if (myMainScreenModeChanged) {
			setTextStatus(myChangedModeText);
			myMainScreenModeChanged = false;
			myChangedModeText[0] = '\0';
			MvrUtil::sleep(300);
		}

		if (hasIpAddressChanged()) {
			setRobotIPStatus(myIpAddress.c_str());
			MvrUtil::sleep(300);
		}


		if (!setSystemMeters(getBatteryPercentage(), getWifiPercentage())) {
			IFDEBUG(MvrLog::log(MvrLog::Normal,
				"%s::runThread() call to setSystemMeters failed", getName()));
		}

		MvrUtil::sleep(300);


		if (!sendKeepAlive()) {
			IFDEBUG(MvrLog::log(MvrLog::Normal,
				"%s::runThread() call to sendKeepAlive failed", getName()));
		}

		while (getRunning() && myIsConnected &&
			(packet = myReceiver->receivePacket(500)) != NULL) {
			myPacketsMutex.lock();
			myPackets.push_back(packet);
			myPacketsMutex.unlock();
			if (myRobot == NULL)
				sensorInterp();
		}

		// if we have a robot but it isn't running yet then don't have a
		// connection failure
		if (getRunning() && myIsConnected && checkLostConnection()) {
			MvrLog::log(MvrLog::Terse,
				"%s::runThread()  Lost connection to the MTX lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
				myLastReading.mSecSince() / 1000.0,
				getConnectionTimeoutSeconds());
			myIsConnected = false;
			if (myConnFailOption)
				disconnectOnError();
			continue;
		}

	}
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	/* MPL PS TODO This should lose connection if we
	haven't heard from it in long enough... but this is
	loosing connection anytime we lose one packet
	(which'll always happen sometimes on serial).
	if (getRunning() && myIsConnected) {
	//MvrLog::log (MvrLog::Normal,
	//            "%s::runThread()  Lost connection to the lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
	//            myLastReading.mSecSince() / 1000.0,
	//            getConnectionTimeoutSeconds() );
	MvrLog::log (MvrLog::Normal,
	"%s::runThread()  Lost connection to the lcd because of error %d %d", getName(), getRunning(), myIsConnected);
	myIsConnected = false;
	//laserDisconnectOnError();
	continue;
	}
	*/
	//MvrUtil::sleep(1);
	//MvrUtil::sleep(2000);
	//MvrUtil::sleep(300);

	return NULL;
}


#if 0 // debug runthread code
MVREXPORT void * MvrLCDMTX::runThread(void *arg)
{
	//MvrLCDMTXPacket *packet;
	MvrRobotPacket *packet;
	unsigned char wifi;
	unsigned char battery;
	bool lightstat = true;

	char text1[10000];
	std::string buf = "going to goal x";
	std::string buf1 = "goal seaking";
	std::string buf2 = "Aram is startup up...";
	std::string buf3 = "Nopey";
	std::string buf4 = "10.0.152.155";


	unsigned char screenNum;
	unsigned char batt;
	unsigned char wi;



	MvrUtil::sleep(300);


	if (!setScreenNumber(STATUS_SCREEN)) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setScreenNumber failed", getName()));
	}

	MvrUtil::sleep(300);

	if (!getScreenNumber(&screenNum))
		MvrLog::log(MvrLog::Normal,
		"%s::runThread() call to getScreenNum failed", getName());
	else
		MvrLog::log(MvrLog::Normal,
		"%s::runThread() current screen num = %d", getName(), screenNum);


	int mod;
	int cnt = 0;


	MvrUtil::sleep(300);

	if (!setTextStatus(buf1.c_str())) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setTextStatus failed", getName()));
	}

	MvrUtil::sleep(300);

	if (!setMainStatus(buf.c_str())) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setMainStatus failed", getName()));
	}


	MvrUtil::sleep(300);

	if (!setBootStatus(buf2.c_str())) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setBootStatus failed", getName()));
	}

	MvrUtil::sleep(300);

	if (!setRobotIdStatus(buf3.c_str())) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setRobotIdStatus failed", getName()));
	}
	MvrUtil::sleep(300);

	if (!setRobotIPStatus(buf4.c_str())) {
		IFDEBUG(MvrLog::log(MvrLog::Normal,
			"%s::runThread() call to setRobotIPStatus failed", getName()));
	}

	MvrUtil::sleep(300);

	while (getRunning())
	{

		//#if 0
		if (mod++ % 100 == 0) {

			// #if 0
			text1[0] = '\0';
			if (!getBootStatus(&text1[0])) {
				IFDEBUG(MvrLog::log(MvrLog::Normal,
					"%s::runThread() call to getBootStatus failed", getName()));
			}
			else {
				MvrLog::log(MvrLog::Normal,
					"%s::runThread() Boot Status \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			MvrUtil::sleep(300);

			if (!getMainStatus(&text1[0])) {
				IFDEBUG(MvrLog::log(MvrLog::Normal,
					"%s::runThread() call to getMainStatus failed", getName()));
			}
			else {
				MvrLog::log(MvrLog::Normal,
					"%s::runThread() Main Status  \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			MvrUtil::sleep(300);

			if (!getTextStatus(&text1[0])) {
				IFDEBUG(MvrLog::log(MvrLog::Normal,
					"%s::runThread() call to getTextStatus failed", getName()));
			}
			else {
				MvrLog::log(MvrLog::Normal,
					"%s::runThread() Text Status  \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			MvrUtil::sleep(300);


			//#endif
			/*
			if (cnt == 0) {
			if (!setTextStatus(buf.c_str())) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to setTextStatus failed", getName()));
			}

			if (!getTextStatus(&text1[0])) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to getTextStatus failed", getName()));
			}
			else {
			MvrLog::log (MvrLog::Normal,
			"%s::runThread() Text Status  \"%s\" ", getName(), text1);
			text1[0] = '\0';
			}

			cnt = 1;
			}
			else if (cnt == 1) {

			if (!setMainStatus(buf1.c_str())) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to setMainStatus failed", getName()));
			}

			if (!getMainStatus(&text1[0])) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to getMainStatus failed", getName()));
			}
			else {
			MvrLog::log (MvrLog::Normal,
			"%s::runThread() Main Status  \"%s\" ", getName(), text1);
			text1[0] = '\0';
			}

			cnt = 2;
			}
			else {

			if (!setBootStatus(buf2.c_str())) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to setBootStatus failed", getName()));
			}

			if (!getBootStatus(&text1[0])) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to getBootStatus failed", getName()));
			}
			else {
			MvrLog::log (MvrLog::Normal,
			"%s::runThread() Boot Status \"%s\" ", getName(), text1);

			text1[0] = '\0';
			}

			cnt = 0;
			}
			*/

			if (!getScreenNumber(&screenNum))
				MvrLog::log(MvrLog::Normal,
				"%s::runThread() call to getScreenNum failed", getName());
			else
				MvrLog::log(MvrLog::Normal,
				"%s::runThread() current screen num = %d", getName(), screenNum);




			/*
			MvrUtil::sleep(300);

			if (!setBacklight(lightstat)) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to setBacklight failed", getName()));
			}

			// just using lightstat here as it's 0 or 1 for the 2 screens
			if (!setScreenNumber(lightstat)) {
			IFDEBUG(MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to setScreenNumber failed", getName()));
			}


			if (lightstat)
			lightstat = false;
			else
			lightstat = true;


			MvrUtil::sleep(1000);

			unsigned char backlight;
			if (!getBacklight(&backlight))
			MvrLog::log (MvrLog::Normal,
			"%s::runThread() call to getBacklight failed", getName());
			else
			MvrLog::log (MvrLog::Normal,
			"%s::runThread() current backlight = %d", getName(), backlight);

			MvrUtil::sleep(300);
			*/
			if (!getSystemMeters(&batt, &wi))
				MvrLog::log(MvrLog::Normal,
				"%s::runThread() call to getSystemMeters failed", getName());
			else
				MvrLog::log(MvrLog::Normal,
				"%s::runThread() current battery = %d wifi = %d", getName(), batt, wi);

			wifi = getWifiPercentage();
			battery = getBatteryPercentage();

			MvrUtil::sleep(300);

			if (!setSystemMeters(battery, wifi)) {
				IFDEBUG(MvrLog::log(MvrLog::Normal,
					"%s::runThread() call to setSystemMeters failed", getName()));
			}
		}
		//#endif




		if (!sendKeepAlive()) {
			IFDEBUG(MvrLog::log(MvrLog::Normal,
				"%s::runThread() call to sendKeepAlive failed", getName()));
		}

		while (getRunning() && myIsConnected &&
			(packet = myReceiver->receivePacket(500)) != NULL) {
			myPacketsMutex.lock();
			myPackets.push_back(packet);
			myPacketsMutex.unlock();
			if (myRobot == NULL)
				sensorInterp();
		}

		// if we have a robot but it isn't running yet then don't have a
		// connection failure
		if (getRunning() && myIsConnected && checkLostConnection()) {
			MvrLog::log(MvrLog::Terse,
				"%s::runThread()  Lost connection to the MTX lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
				myLastReading.mSecSince() / 1000.0,
				getConnectionTimeoutSeconds());
			myIsConnected = false;
			if (myConnFailOption)
				disconnectOnError();
			continue;
		}

	}
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	/* MPL PS TODO This should lose connection if we
	haven't heard from it in long enough... but this is
	loosing connection anytime we lose one packet
	(which'll always happen sometimes on serial).
	if (getRunning() && myIsConnected) {
	//MvrLog::log (MvrLog::Normal,
	//            "%s::runThread()  Lost connection to the lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
	//            myLastReading.mSecSince() / 1000.0,
	//            getConnectionTimeoutSeconds() );
	MvrLog::log (MvrLog::Normal,
	"%s::runThread()  Lost connection to the lcd because of error %d %d", getName(), getRunning(), myIsConnected);
	myIsConnected = false;
	//laserDisconnectOnError();
	continue;
	}
	*/
	//MvrUtil::sleep(1);
	//MvrUtil::sleep(2000);
	//MvrUtil::sleep(300);

	return NULL;
}
#endif

/**
This will check if the lcd has lost connection.  If there is no
robot it is a straightforward check of last reading time against
getConnectionTimeoutSeconds.  If there is a robot then it will not
start the check until the lcd is running and connected.
**/
MVREXPORT bool MvrLCDMTX::checkLostConnection(void)
{

	if ((myRobot == NULL || myRobotRunningAndConnected) &&
		getConnectionTimeoutSeconds() > 0 &&
		myLastReading.mSecSince() >  getConnectionTimeoutSeconds() * 1000)
		return true;

	if (!myRobotRunningAndConnected && myRobot != NULL &&
		myRobot->isRunning() && myRobot->isConnected())
	{
		myRobotRunningAndConnected = true;
		myLastReading.setToNow();
	}

	return false;
}

MVREXPORT void MvrLCDMTX::disconnectOnError(void)
{
	MvrLog::log(MvrLog::Normal, "%s: Disconnected because of error", getName());
	myDisconnectOnErrorCBList.invoke();
}

MVREXPORT bool MvrLCDMTX::sendKeepAlive()
{

	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(KEEP_ALIVE);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::sendKeepAlive() Could not send keep alive request to LCD", getName());
		return false;
	}

	//IFDEBUG (
	//
	//	MvrLog::log (MvrLog::Normal,
	//             "%s::sendKeepAlive() keep alive sent to LCD",
	//              getName());

	//); // end IFDEBUG

	return true;

}

MVREXPORT bool MvrLCDMTX::sendVersion()
{

	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(VERSION);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::sendVersion() Could not send version request to LCD", getName());
		return false;
	}

	IFDEBUG(

		MvrLog::log(MvrLog::Normal,
		"%s::sendVersion() version sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}

MVREXPORT bool MvrLCDMTX::sendSystemInfo(unsigned char command)
{

	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SYSTEM_INFO);
	sendPacket.byteToBuf(command);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::sendSystemInfo() Could not send sys info request to LCD", getName());
		return false;
	}

	IFDEBUG(

		MvrLog::log(MvrLog::Normal,
		"%s::sendSystemInfo() sys info sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}

MVREXPORT bool MvrLCDMTX::sendReboot()
{

	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(REBOOT);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::sendReboot() Could not send reboot request to LCD", getName());
		return false;
	}

	IFDEBUG(

		MvrLog::log(MvrLog::Normal,
		"%s::sendReboot() reboot sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}


MVREXPORT bool MvrLCDMTX::getTextField(unsigned char textNumber, char *text)
{

	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_TEXT_FIELD);
	sendPacket.byteToBuf(textNumber);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::getTextField() Could not send get text field request to LCD", getName());
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::getTextField() get text field sent to LCD",
			getName());

		); // end IFDEBUG

		MvrRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal,
				"%s::getTextField() No response to get text field - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *textFieldBuf = (unsigned char *)packet->getBuf();

		// verify get num trans received
		if ((textFieldBuf[3] != GET_TEXT_FIELD) || (textFieldBuf[4] != textNumber)) {
			MvrLog::log(MvrLog::Normal,
				"%s::getTextField() Invalid response from lcd to get text field status (0x%x) text # %d in text # %d",
				getName(), textFieldBuf[3], textFieldBuf[4], textNumber);
			continue;

		}
		else {


			char tempBuf[256];

			snprintf(tempBuf, sizeof(tempBuf), "%s", &textFieldBuf[5]);
			strcpy(text, tempBuf);

			delete packet;
			return true;
		}
	} // endfor

	return false;

}



MVREXPORT bool MvrLCDMTX::getScreenNumber(unsigned char *currentScreenNumber)

{
	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_CURRENT_SCREEN_NUM);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::getScreenNumber() Could not send get screen number to LCD", getName());
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::getScreenNumber() get screen number sent to LCD",
			getName());

		); // end IFDEBUG


		MvrRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal,
				"%s::getScreenNumber() No response to get screen number - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *screenStatusBuf = (unsigned char *)packet->getBuf();

		// verify get num 
		if (screenStatusBuf[3] != GET_CURRENT_SCREEN_NUM) {
			MvrLog::log(MvrLog::Normal,
				"%s::getScreenNumber() Invalid response from lcd to get screen number (0x%x)",
				getName(), screenStatusBuf[3]);
			continue;

		}
		else {
			*currentScreenNumber = screenStatusBuf[4];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

MVREXPORT bool MvrLCDMTX::getBacklight(unsigned char *backlight)

{
	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_BACKLIGHT);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::getBacklight() Could not send get backlight to LCD", getName());
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::getBacklight() get backlight sent to LCD",
			getName());

		); // end IFDEBUG


		MvrRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal,
				"%s::getBacklight() No response to get backlight - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *backlightBuf = (unsigned char *)packet->getBuf();

		// verify 
		if (backlightBuf[3] != GET_BACKLIGHT) {
			MvrLog::log(MvrLog::Normal,
				"%s::getBacklight() Invalid response from lcd to get backlight (0x%x)",
				getName(), backlightBuf[3]);
			continue;

		}
		else {
			*backlight = backlightBuf[4];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

MVREXPORT bool MvrLCDMTX::getSystemMeters(unsigned char *battery, unsigned char *wifi)

{
	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_SYSTEM_METERS);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::getSystemMeters() Could not send get system meters to LCD", getName());
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::getSystemMeters() get system meters sent to LCD",
			getName());

		); // end IFDEBUG


		MvrRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal,
				"%s::getSystemMeters() No response to get system meters - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *systemMetersBuf = (unsigned char *)packet->getBuf();

		// verify 
		if (systemMetersBuf[3] != GET_SYSTEM_METERS) {
			MvrLog::log(MvrLog::Normal,
				"%s::getSystemMeters() Invalid response from lcd to get system meters (0x%x)",
				getName(), systemMetersBuf[3]);
			continue;

		}
		else {
			*battery = systemMetersBuf[4];
			*wifi = systemMetersBuf[5];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

MVREXPORT bool MvrLCDMTX::setScreenNumber(unsigned char screenNumber)
{
	if ((screenNumber == BOOT_SCREEN) || (screenNumber == STATUS_SCREEN)) {

		MvrRobotPacket sendPacket(HEADER1, HEADER2);
		sendPacket.setID(SET_SCREEN_NUM);
		sendPacket.byteToBuf(screenNumber);

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::setScreenNumber() Could not send set screen number to LCD", getName());
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::setScreenNumber() set screen number %d sent to LCD",
			getName(), screenNumber);

		); // end IFDEBUG

		return true;
	}
	else { // bad screen number
		MvrLog::log(MvrLog::Normal,
			"%s::setScreenNumber() Invalid screen number %d", getName(), screenNumber);
		return false;
	}
}

MVREXPORT bool MvrLCDMTX::setTextField(unsigned char textNumber, const char *text)
{


	if ((textNumber == BOOT_STATUS_TEXT) ||
		(textNumber == MAIN_STATUS_TEXT) ||
		(textNumber == MODE_TEXT) ||
		(textNumber == ROBOT_ID_TEXT) ||
		(textNumber == ROBOT_IP_TEXT)) {

		MvrRobotPacket sendPacket(HEADER1, HEADER2);
		sendPacket.setID(SET_TEXT_FIELD);
		sendPacket.byteToBuf(textNumber);
		sendPacket.strToBuf(text);

		if (!mySender->sendPacket(&sendPacket)) {
			MvrLog::log(MvrLog::Terse,
				"%s::setTextField() Could not send mode %d set text field \"%s\" to LCD",
				getName(), textNumber, text);
			return false;
		}

		IFDEBUG(

			MvrLog::log(MvrLog::Normal,
			"%s::setTextField() mode %d text \"%s\" field sent to LCD",
			getName(), textNumber, text);

		); // end IFDEBUG

		return true;
	}
	else { // bad text number
		MvrLog::log(MvrLog::Normal,
			"%s::setTextField() Invalid mode %d", getName(), textNumber);
		return false;
	}
}

MVREXPORT bool MvrLCDMTX::setBacklight(bool backlight)
{


	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SET_BACKLIGHT);
	if (backlight)
		sendPacket.byteToBuf(99);
	else
		sendPacket.byteToBuf(0);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::setBacklight() Could not send set backlight %d to LCD",
			getName(), backlight);
		return false;
	}

	IFDEBUG(

		MvrLog::log(MvrLog::Normal,
		"%s::setBacklight() set backlight %d sent to LCD",
		getName(), backlight);

	); // end IFDEBUG

	return true;

}


MVREXPORT bool MvrLCDMTX::setSystemMeters(unsigned char battery, unsigned char wifi)
{


	MvrRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SET_BATTERY_WIFI);
	sendPacket.byteToBuf(battery);
	sendPacket.byteToBuf(wifi);

	if (!mySender->sendPacket(&sendPacket)) {
		MvrLog::log(MvrLog::Terse,
			"%s::setSystemMeters() Could not send set system meters %d %d to LCD",
			getName(), battery, wifi);
		return false;
	}

	/*
	IFDEBUG (

	MvrLog::log (MvrLog::Normal,
	"%s::setSystemMeters() set system meters %d %d sent to LCD",
	getName(), battery, wifi);

	); // end IFDEBUG
	*/

	return true;

}


MVREXPORT void MvrLCDMTX::writeToLCD()
{


#if 0
	// grab the status
	MvrServerMode *netMode;
	if ((netMode = MvrServerMode::getActiveMode()) != NULL)
	{
		MvrLog::log(MvrLog::Normal,
			"%s::writeToLCD status = %s", getName(), netMode->getStatus());
	}
	else
	{
		MvrLog::log(MvrLog::Normal,
			"%s::writeToLCD could not get status");

	}
#endif
}

MVREXPORT unsigned char MvrLCDMTX::getBatteryPercentage()
{
	if (myRobot->haveStateOfCharge())
		return myRobot->getStateOfCharge();
	else {
		if (!myLoggedBatteryError) {
			MvrLog::log(MvrLog::Normal,
				"%s::getBatteryPercentage() State of charge not available setting it to 0",
				getName());
			myLoggedBatteryError = true;
		}
		return 0;
	}
}

MVREXPORT unsigned char MvrLCDMTX::getWifiPercentage()
{

	// if the link signal exists - then grab the wifi%
	if (MvrSystemStatus::getMTXWirelessLink() == 1) {
		return MvrSystemStatus::getMTXWirelessQuality();
	}
	else
		return 0;
}

MVREXPORT void MvrLCDMTX::getIpAddress()
{

	char ip[1000];
	sprintf(ip, "%d.%d.%d.%d",
		MvrSystemStatus::getMTXWirelessIpAddress1(),
		MvrSystemStatus::getMTXWirelessIpAddress2(),
		MvrSystemStatus::getMTXWirelessIpAddress3(),
		MvrSystemStatus::getMTXWirelessIpAddress4());

	myIpAddress = ip;

}

MVREXPORT bool MvrLCDMTX::hasIpAddressChanged()
{

	char ip[1000];
	sprintf(ip, "%d.%d.%d.%d",
		MvrSystemStatus::getMTXWirelessIpAddress1(),
		MvrSystemStatus::getMTXWirelessIpAddress2(),
		MvrSystemStatus::getMTXWirelessIpAddress3(),
		MvrSystemStatus::getMTXWirelessIpAddress4());

	std::string tempIpAddress = ip;

	if (tempIpAddress.compare(myIpAddress) != 0) {
		myIpAddress = ip;
		return true;
	}

	return false;

}

MVREXPORT bool MvrLCDMTX::setMainStatus(const char *status)
{
#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setMainStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!setTextField(MAIN_STATUS_TEXT, status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setMainStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::setMainStatus() set status successful = %s",
		getName(), status));

	return true;


}

MVREXPORT bool MvrLCDMTX::setTextStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setTextStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(MODE_TEXT, status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setTextStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::setTextStatus() set status successful = %s",
		getName(), status));

	return true;

}


MVREXPORT bool MvrLCDMTX::setRobotIdStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setRobotIdStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(ROBOT_ID_TEXT, status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setRobotIdStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::setRobotIdStatus() set status successful = %s",
		getName(), status));

	return true;

}

MVREXPORT bool MvrLCDMTX::setRobotIPStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setRobotIPStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(ROBOT_IP_TEXT, status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setRobotIdStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::setRobotIPStatus() set status successful = %s",
		getName(), status));

	return true;

}


MVREXPORT bool MvrLCDMTX::setBootStatus(const char *status)
{

#if 0
	if (!setScreenNumber(BOOT_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setBootStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	MvrLog::log(MvrLog::Normal, "%s:setBootStatus: (BOOT_STATUS_TEXT) text '%s'",
		getName(), status);


	if (!setTextField(BOOT_STATUS_TEXT, status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::setBootStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::setBootStatus() set status successful = %s",
		getName(), status));

	return true;
}


MVREXPORT bool MvrLCDMTX::getMainStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getMainStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!getTextField(MAIN_STATUS_TEXT, (char *)status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getMainStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::getMainStatus() get status successful = %s",
		getName(), status));

	return true;


}
MVREXPORT bool MvrLCDMTX::getTextStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getTextStatus() can not set screen number",
			getName());
		return false;
	}
#endif


	if (!getTextField(MODE_TEXT, (char *)status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getTextStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::getTextStatus() get status successful = %s",
		getName(), status));

	return true;

}

MVREXPORT bool MvrLCDMTX::getBootStatus(const char *status)
{

#if 0
	if (!setScreenNumber(BOOT_SCREEN)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getBootStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!getTextField(BOOT_STATUS_TEXT, (char *)status)) {
		MvrLog::log(MvrLog::Normal,
			"%s::getBootStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(MvrLog::log(MvrLog::Normal,
		"%s::getBootStatus() get status successful = %s",
		getName(), status));

	return true;
}


MVREXPORT bool MvrLCDMTX::verifyFwVersion()
{

	// now make sure we have a file

	std::string hmiFile;
	hmiFile.clear();

	char hmiFilePrefix[100];
	hmiFilePrefix[0] = '\0';


	sprintf(hmiFilePrefix, "AdeptHmi");

	hmiFile = searchForFile(ourFirmwareBaseDir.c_str(), hmiFilePrefix, ".ds");

	char hmiFileOut[100];
	hmiFileOut[0] = '\0';

	if (hmiFile.empty()) {
		MvrLog::log(MvrLog::Normal,
			"%s::verifyFwVersion() can't find hmi file with prefix = %s",
			getName(), hmiFilePrefix);
		return false;
	}
	else {
		// validate that the file dosn't match the current version

		sprintf(hmiFileOut, "AdeptHmi_%s.ds",
			myFirmwareVersion.c_str());

		if (strcmp(hmiFile.c_str(), hmiFileOut) != 0) {
			myNewFwFile = ourFirmwareBaseDir + hmiFile;
			MvrLog::log(MvrLog::Normal,
				"%s::verifyFwVersion() versions mismatch %s %s",
				getName(), hmiFileOut, hmiFile.c_str());
			return true;
		}
		else {
			MvrLog::log(MvrLog::Normal,
				"%s::verifyFwVersion() hmi file found but version matches (%s)",
				getName(), hmiFileOut);
			return false;
		}
	}

	return false;
}


MVREXPORT bool MvrLCDMTX::downloadFirmware()
{



	// now connect to the serial port

	MvrSerialConnection *serConn = NULL;
	serConn = dynamic_cast<MvrSerialConnection *> (myConn);
	if (serConn != NULL)
		serConn->setBaud(115200);
	if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN
		&& !myConn->openSimple()) {
		MvrLog::log(
			MvrLog::Normal,
			"%s::downloadFirmware() Could not connect because the connection was not open and could not open it",
			getName());

		return false;
	}



	//  now send hello - and see if we get a response

	unsigned char hello = 0xc1;

	//// MvrBasePacket *packet;
	// while ((packet = readPacket()) != NULL)

	MvrTime timeDone;

	if (!timeDone.addMSec(30 * 1000))
	{
		MvrLog::log(MvrLog::Normal,
			"%s::downloadFirmware() error adding msecs (30 * 1000)",
			getName());
	}

	unsigned char helloResp[4];
	bool gotResponse = false;
	int hmiVersion = 0;
	int hmiRevision = 0;

	while (timeDone.mSecTo() > 0) {

		if ((myConn->write((char *)&hello, 1)) == -1) {

			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() Could not send hello to LCD", getName());
			return false;
		}

		if ((myConn->read((char *)&helloResp[0], 4, 500)) > 0) {

			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() received hello response 0x%02x 0x%02x 0x%02x 0x%02x",
				getName(), helloResp[0], helloResp[1], helloResp[2], helloResp[3]);

			if ((helloResp[0] == 0xc0) && (helloResp[3] == 0x4b)) {
				MvrLog::log(MvrLog::Normal,
					"%s::downloadFirmware() received hello response",
					getName());

				gotResponse = true;
				hmiVersion = helloResp[1];
				hmiRevision = helloResp[2];

				break;

			}

		}
		else {
			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() read failed",
				getName());

		}
	}

	if (!gotResponse) {
		MvrLog::log(MvrLog::Normal,
			"%s::downloadFirmware() Received no hello response", getName());
		return false;
	}



	FILE *file;
	char line[10000];
	unsigned char c;

	if ((file = MvrUtil::fopen(myNewFwFile.c_str(), "r")) == NULL) {
		MvrLog::log(MvrLog::Normal,
			"%s::downloadFirmware() Could not open file %s for reading errno (%d)",
			getName(), myNewFwFile.c_str(), errno);
		return false;
	}

	line[0] = '\0';

	MvrLog::log(MvrLog::Normal,
		"%s::downloadFirmware() Updating LCD firmware....",
		getName());

	while (fgets(line, sizeof (line), file) != NULL) {

		MvrArgumentBuilder builder;
		builder.add(line);

		int i;

		std::string data;
		data.clear();

		for (i = 0; i < builder.getArgc(); i++) {

			if (!builder.isArgInt(i, true)) {

				MvrLog::log(MvrLog::Normal,
					"%s::downloadFirmware() Could not convert file", getName());
				return false;

			}

			char ch = builder.getArgInt(i, NULL, true);
			data.push_back(ch);



		} // end for


		//MvrLog::log(MvrLog::Normal,
		//			"%s::downloadFirmware() data = %s size = %d", getName(), data.c_str(), data.size());

		//MvrLog::log(MvrLog::Normal,
		//		"%s::downloadFirmware() %d %c 0x%02x", getName(), data, data, data);
		if ((myConn->write((char *)data.c_str(), data.size())) == -1) {
			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() Could not send data size(%d) to LCD errno (%d)", getName(), data.length(), errno);
			return false;
		}


		// wait a sec for the response
		if ((myConn->read((char *)&c, 1, 1000)) > 0) {

			if (c == 0x4b)
				continue;
			else {
				MvrLog::log(MvrLog::Normal,
					"%s::downloadFirmware() Invalid response %x02%x from LCD to load data",
					getName(), c);
				return false;
			}

		}
		else {
			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() Did not get response from LCD to load data", getName());
			return false;

		}

	} // end while
	if (feof(file)) {
		// end of file reached
		MvrLog::log(MvrLog::Normal,
			"%s::downloadFirmware() LCD firmware updated",
			getName());
		fclose(file);

		MvrUtil::sleep(5000);

		// now reconnect to the serial port

		MvrSerialConnection *serConn = NULL;
		serConn = dynamic_cast<MvrSerialConnection *> (myConn);
		if (serConn != NULL)
			serConn->setBaud(115200);
		if (myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN
			&& !myConn->openSimple()) {
			MvrLog::log(
				MvrLog::Normal,
				"%s::downloadFirmware() Could not connect because the connection was not open and could not open it",
				getName());

			return false;
		}
		else {
			MvrLog::log(
				MvrLog::Normal,
				"%s::downloadFirmware() Reestablished the serial connection",
				getName());
		}
		// need to go get the latest fw version ?? - probably not - just use the one from the file


		if (!sendVersion()) {
			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() Could not send version request to LCD", getName());
			return false;
		}

		MvrRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			MvrLog::log(MvrLog::Normal, "%s::downloadFirmware() Did not get response to version request (%d) - resending",
				getName(), timeDone.mSecTo());
			return false;
		}

		unsigned char *versionBuf = (unsigned char *)packet->getBuf();

		// verify get num trans received
		if (versionBuf[3] != VERSION)  {

			MvrLog::log(MvrLog::Normal,
				"%s::downloadFirmware() Invalid response from lcd to send version (0x%x)",
				getName(), versionBuf[3]);
			delete packet;
			return false;
		}
		else {

			char tempBuf[256];

			snprintf(tempBuf, sizeof(tempBuf), "%s", &versionBuf[4]);
			myFirmwareVersion = tempBuf;

			MvrLog::log(MvrLog::Normal, "%s::downloadFirmware() LCD firmware version = %s",
				getName(), myFirmwareVersion.c_str());
			delete packet;

		}

		return true;

	}
	else {

		MvrLog::log(MvrLog::Normal,
			"%s::downloadFirmware() failed updating LCD firmware",
			getName());
		fclose(file);

		return false;

	}
}


MVREXPORT std::string MvrLCDMTX::searchForFile(
	const char *dirToLookIn, const char *prefix, const char *suffix)
{

	/***
	MvrLog::log(MvrLog::Normal,
	"MvrUtil::matchCase() dirToLookIn = \"%s\" fileName = \"%s\"",
	dirToLookIn,
	fileName);
	***/
#ifndef WIN32

	DIR *dir;
	struct dirent *ent;

	/*
	for (it = split.begin(); it != split.end(); it++)
	{
	printf("@@@@@@@@ %s\n", (*it).c_str());
	}
	*/

	// how this works is we start at the base dir then read through
	// until we find what the next name we need, if entry is a directory
	// and we're not at the end of our string list then we change into
	// that dir and the while loop keeps going, if the entry isn't a
	// directory and matchs and its the last in our string list we've
	// found what we want
	if ((dir = opendir(dirToLookIn)) == NULL)
	{
		MvrLog::log(MvrLog::Normal,
			"AramUtil::findFile: No such directory '%s' for base",
			dirToLookIn);
		return "";
	}

	while ((ent = readdir(dir)) != NULL)
	{
		// ignore some of these
		if (ent->d_name[0] == '.')
		{
			//printf("Ignoring %s\n", ent->d_name[0]);
			continue;
		}
		//printf("NAME %s finding %s\n", ent->d_name, finding.c_str());

		// continue if the prefix should be searched for and doesn't match
		if (prefix != NULL && prefix[0] != '\0' &&
			strncasecmp(ent->d_name, prefix, strlen(prefix)) != 0)
			continue;

		// continue if the suffix should be searched for and doesn't match
		if (suffix != NULL && suffix[0] != '\0' &&
			strlen(ent->d_name) > strlen(suffix) + 1 &&
			strncasecmp(&ent->d_name[strlen(ent->d_name) - strlen(suffix)],
			suffix, strlen(suffix)) != 0)
			continue;

		std::string ret = ent->d_name;
		closedir(dir);
		return ret;
	}
	//printf("!!!!!!!! %s", finding.c_str());
	closedir(dir);

#endif // ifndef WIN32

	return "";
}

MVREXPORT bool MvrLCDMTX::setMTXLCDMainScreenText(const char *status)
{

	if (strlen(status) < 248) {
		myMainScreenStatusChanged = true;
		strcpy(myChangedStatusText, status);
		return true;
	}

	MvrLog::log(MvrLog::Normal,
		"%s::setMTXLCDMainScreenMode invalid string length (%d) needs to be < 248",
		getName(), strlen(status));
	return false;


}

/*
MVREXPORT bool MvrLCDMTX::setMTXLCDMainScreenMode(const char *status)
{

if (strlen(status) < 248) {
myMainScreenModeChanged = true;
strcpy(myChangedModeText, status);
return true;
}

MvrLog::log (MvrLog::Normal,
"%s::setMTXLCDMainScreenMode invalid string length (%d) needs to be < 248",
getName(), strlen(status));
return false;

}
*/

MVREXPORT bool MvrLCDMTX::setMTXLCDBootScreenText(const char *status)

{


	if (strlen(status) < 248) {
		myBootTextChanged = true;
		strcpy(myChangedBootText, status);
		return true;
	}

	MvrLog::log(MvrLog::Normal,
		"%s::setMTXLCDBootScreenText() invalid string length (%d) needs to be < 248",
		getName(), strlen(status));
	return false;

}

MVREXPORT bool MvrLCDMTX::setMTXLCDScreenNumber(unsigned char screenNumber)
{

	if ((screenNumber == BOOT_SCREEN) || (screenNumber == STATUS_SCREEN)) {

		myScreenNumberChanged = true;
		myChangedScreenNumber = screenNumber;
		return true;
	}

	MvrLog::log(MvrLog::Normal,
		"%s::setMTXLCDScreenNumber() invalid screen number %d",
		getName(), screenNumber);
	return false;

}

MVREXPORT bool MvrLCDMTX::setMTXLCDBackLight(bool backLight)
{

	myBackLightChanged = true;
	myChangedBackLight = backLight;
	return true;

}

MVREXPORT void MvrLCDMTX::setIdentifier(const char *identifier)
{
	myRobotIdentifierChanged = true;
	myRobotIdentifier = identifier;

}

MVREXPORT bool MvrLCDMTX::internalMTXLCDOverrideMainScreenText(const char *status)
{
	myMainScreenStatusChanged = false;
	return setMainStatus(status);
}

MVREXPORT bool MvrLCDMTX::internalMTXLCDOverrideBootScreenText(const char *status)
{
	myBootTextChanged = false;
	return setBootStatus(status);
}

MVREXPORT bool MvrLCDMTX::internalMTXLCDOverrideBackLight(bool backlight)
{
	myChangedBackLight = false;
	return setBacklight(backlight);
}
