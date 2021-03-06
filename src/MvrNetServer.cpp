#include <ctype.h>
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrNetServer.h"
#include "MvrRobot.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include "MvrSyncTask.h"
#include "MvrArgumentBuilder.h"
#include "mvriaInternal.h"


MvrNetServer::MvrNetServer(bool addMvrExitCB, bool doNotAddShutdownServer,
			 const char *name, MvrNetServer *childServer) :
  myTaskCB(this, &MvrNetServer::runOnce),
  myHelpCB(this, &MvrNetServer::internalHelp),
  myEchoCB(this, &MvrNetServer::internalEcho),
  myQuitCB(this, &MvrNetServer::internalQuit),
  myShutdownServerCB(this, &MvrNetServer::internalShutdownServer),
  myMvrExitCB(this, &MvrNetServer::close)
{
  if (name != NULL)
    myName = name;
  else
    myName = "MvrNetServer";
  myChildServer = childServer;
  myMutex.setLogName((myName + "::myMutex").c_str());
  myRobot = NULL;
  myPort = 0;
  myMultipleClients = false;
  myOpened = false;
  myWantToClose = false;
  myLoggingDataSent = false;
  myLoggingDataReceived = false;
  mySquelchNormal = false;
  addCommand("help", &myHelpCB, "gives the listing of available commands");
  addCommand("echo", &myEchoCB, "with no args gets echo, with args sets echo");
  addCommand("quit", &myQuitCB, "closes this connection to the server");
  // MPL 2013_06_10 letting folks take out shutdownServer since it
  // can do no good and much ill
  if (!doNotAddShutdownServer)
    addCommand("shutdownServer", &myShutdownServerCB, "shuts down the server");

  myMutex.setLogName((myName + "::myMutex").c_str());
  myNextCycleSendsMutex.setLogName((myName + "::myNextCycleSendsMutex").c_str());
  
  myMvrExitCB.setName((myName + "::mvriaExit").c_str());
  if (addMvrExitCB)
    Mvria::addExitCallback(&myMvrExitCB, 40);
}

MvrNetServer::~MvrNetServer()
{
  MvrSyncTask *rootTask = NULL;
  MvrSyncTask *proc = NULL;
  // get rid of us running on the robot task
  if (myRobot != NULL && (rootTask = myRobot->getSyncTaskRoot()) != NULL)
  {
    proc = rootTask->findNonRecursive(&myTaskCB);
    if (proc != NULL)
      delete proc;
  }
  close();
}

/**
   Open the server, if you supply a robot this will run in the robots
   attached, if you do not supply a robot then it will be open and
   you'll have to call runOnce yourself (this is only recommended for
   advanced users)

   @param robot the robot that this should be attached to and run in
   the sync task of or NULL not to run in any robot's task

   @param port the port to start up the service on

   @param password the password needed to use the service

   @param multipleClients if false only one client is allowed to connect, 
   if false multiple clients are allowed to connect or just one

   @param openOnIP If not NULL, restrict server port to the network interface
   with this IP address. If NULL, accept connections from any network interface.

   @return true if the server could be started, false otherwise
**/
MVREXPORT bool MvrNetServer::open(MvrRobot *robot, unsigned int port, 
				const char *password, bool multipleClients,
				const char *openOnIP)
{
  MvrSyncTask *rootTask = NULL;
  MvrSyncTask *proc = NULL;
  std::string taskName;

  if (myOpened)
  {
    MvrLog::log(MvrLog::Terse, "%s already inited, cannot reinit",
	       myName.c_str());
    return false;
  }

  myRobot = robot;
  myPort = port;
  myPassword = password;
  myMultipleClients = multipleClients;
  
  if (myServerSocket.open(myPort, MvrSocket::TCP, openOnIP))
  {
    // this can be taken out since the open does this now
    //myServerSocket.setLinger(0);
    myServerSocket.setNonBlock();
    if (openOnIP != NULL)
      MvrLog::log(MvrLog::Normal, "%s opened on port %d on ip %s.", 
		 myName.c_str(), myPort, openOnIP);
    else
      MvrLog::log(MvrLog::Normal, "%s opened on port %d.", 
		 myName.c_str(), myPort);
    myOpened = true;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "%s failed to open: %s", 
	       myName.c_str(), myServerSocket.getErrorStr().c_str());
    myOpened = false;
    return false;
  }

  // add ourselves to the robot if we aren't already there
  if (myRobot != NULL && (rootTask = myRobot->getSyncTaskRoot()) != NULL)
  {    
    proc = rootTask->findNonRecursive(&myTaskCB);
    if (proc == NULL)
    {
      // toss in a netserver (it used to say the port name, but did it wrong so put in gibberish)
      taskName = myName; 
      rootTask->addNewLeaf(taskName.c_str(), 60, &myTaskCB, NULL);
    }
  }
  return true;
  
}

/**
   This adds a command to the list, when the command is given the
   broken up argv and argc are given along with the socket it came
   from (so that acks can occur)
**/
MVREXPORT bool MvrNetServer::addCommand(const char *command, 
			      MvrFunctor3<char **, int, MvrSocket *> *functor,
				      const char *help)
{
  if (myChildServer != NULL)
    myChildServer->addCommand(command, functor, help); 

  std::map<std::string, MvrFunctor3<char **, int, MvrSocket *> *, MvrStrCaseCmpOp>::iterator it;


  if ((it = myFunctorMap.find(command)) != myFunctorMap.end())
  {
    MvrLog::log(MvrLog::Normal, "%s::addCommand: Already a command for %s", myName.c_str(), command);
    return false;
  }

  myFunctorMap[command] = functor;
  myHelpMap[command] = help;


  return true;
}

/**
   @param command the command to remove
   @return true if the command was there to remove, false otherwise
**/
MVREXPORT bool MvrNetServer::remCommand(const char *command)
{
  if (myChildServer != NULL)
    myChildServer->remCommand(command);

  if (myFunctorMap.find(command) == myFunctorMap.end())
  {
    return false;
  }

  myFunctorMap.erase(command);
  myHelpMap.erase(command);
  return true;  
}


MVREXPORT void MvrNetServer::sendToAllClientsPlain(const char *str)
{
  if (myChildServer != NULL)


    myChildServer->sendToAllClientsPlain(str);

  std::list<MvrSocket *>::iterator it;

  if (myLoggingDataSent)
    MvrLog::log(MvrLog::Terse, "%s::sendToAllClients: Sending %s", 
	       myName.c_str(), str);

  for (it = myConns.begin(); it != myConns.end(); ++it)
  {
    (*it)->setLogWriteStrings(false);
    (*it)->writeString(str);
    (*it)->setLogWriteStrings(myLoggingDataSent);
  }
}


/**
   This sends the given string to all the clients, this string cannot
   be more than 2048 number of bytes
**/
MVREXPORT void MvrNetServer::sendToAllClients(const char *str, ...)
{
  char buf[40000];
  va_list ptr;
  va_start(ptr, str);
  vsprintf(buf, str, ptr);

  sendToAllClientsPlain(buf);
  
  va_end(ptr);
}



MVREXPORT void MvrNetServer::sendToAllClientsNextCyclePlain(const char *str)
{
  if (myChildServer != NULL)
    myChildServer->sendToAllClientsNextCyclePlain(str);

  std::list<MvrSocket *>::iterator it;

  if (myLoggingDataSent)
    MvrLog::log(MvrLog::Terse, "%s::sendToAllClientsNextCycle: Next cycle will send: %s", myName.c_str(), str);

  myNextCycleSendsMutex.lock();
  myNextCycleSends.push_back(str);
  myNextCycleSendsMutex.unlock();
}

MVREXPORT bool MvrNetServer::sendToAllClientsNextCyclePlainBool(const char *str)
{
  sendToAllClientsNextCyclePlain(str);
  return true;
}

/**
   This sends the given string to all the clients, this string cannot
   be more than 2048 number of bytes
**/
MVREXPORT void MvrNetServer::sendToAllClientsNextCycle(const char *str, ...)
{
  char buf[40000];
  va_list ptr;
  va_start(ptr, str);
  vsprintf(buf, str, ptr);

  sendToAllClientsNextCyclePlain(buf);
  
  va_end(ptr);
}


MVREXPORT bool MvrNetServer::isOpen(void)
{
  return myOpened;
}


/**
   @param loggingData if true data will be logged which means that all
   data sent out to the all the clients will be logged
**/
MVREXPORT void MvrNetServer::setLoggingDataSent(bool loggingData)
{
  if (myChildServer != NULL)
    myChildServer->setLoggingDataSent(loggingData);

  myLoggingDataSent = loggingData;
  std::list<MvrSocket *>::iterator it;
  for (it = myConnectingConns.begin(); it != myConnectingConns.end(); ++it)
    (*it)->setLogWriteStrings(loggingData);
  for (it = myConns.begin(); it != myConns.end(); ++it)
    (*it)->setLogWriteStrings(loggingData);
}

/**
   @return if true data will be logged which means that all data sent
   out to the all the clients will be logged
**/
MVREXPORT bool MvrNetServer::getLoggingDataSent(void)
{
  return myLoggingDataSent;
}

/**
   @param loggingData if true data will be logged which means that all
   commands received from clients are logged
**/
MVREXPORT void MvrNetServer::setLoggingDataReceived(bool loggingData)
{
  if (myChildServer != NULL)
    myChildServer->setLoggingDataReceived(loggingData);

  myLoggingDataReceived = loggingData;
}

/**
   @return if true data will be logged which means that all commands
   received from clients are logged 
**/
MVREXPORT bool MvrNetServer::getLoggingDataReceived(void)
{
  return myLoggingDataReceived;
}


/**
   Set reverse line ending characters for compatibility with certain old
   clients.
   @param useWrongEndChars if true the wrong end chars will be sent ('\\n\\r' instead
   of '\\r\\n'); a nonstandard, deprecated line ending, but may be required for certain old
   clients.
**/
MVREXPORT void MvrNetServer::setUseWrongEndChars(bool useWrongEndChars)
{
  if (myChildServer != NULL)
    myChildServer->setUseWrongEndChars(useWrongEndChars);

  myUseWrongEndChars = useWrongEndChars;
  std::list<MvrSocket *>::iterator it;
  for (it = myConnectingConns.begin(); it != myConnectingConns.end(); ++it)
    (*it)->setStringUseWrongEndChars(useWrongEndChars);
  for (it = myConns.begin(); it != myConns.end(); ++it)
    (*it)->setStringUseWrongEndChars(useWrongEndChars);
}

/**
   @return if true data will be logged which means that all data sent
   out to the all the clients will be logged
**/
MVREXPORT bool MvrNetServer::getUseWrongEndChars(void)
{
  return myUseWrongEndChars;
}

MVREXPORT void MvrNetServer::runOnce(void)
{

  //  MvrSocket acceptingSocket;
  MvrSocket *socket;
  char *str;
  std::list<MvrSocket *> removeList;
  std::list<MvrSocket *>::iterator it;
  MvrArgumentBuilder *args = NULL;
  std::string command;

  if (!myOpened)
  {
    return;
  }

  // copy the strings we want to send next cycle
  myNextCycleSendsMutex.lock();

  std::list<std::string> nextCycleSends;
  std::list<std::string>::iterator ncsIt;;
  
  nextCycleSends = myNextCycleSends;
  myNextCycleSends.clear();

  myNextCycleSendsMutex.unlock();


  lock();
  // get any new sockets that want to connect
  while (myServerSocket.accept(&myAcceptingSocket) &&
	 myAcceptingSocket.getFD() >= 0)
  {
    //myAcceptingSocket.setNonBlock();
    // see if we want more sockets
    if (!myMultipleClients && (myConns.size() > 0 ||
			       myConnectingConns.size() > 0))
    {
      // we didn't want it, so politely tell it to go away
      myAcceptingSocket.writeString("Conn refused.");
      myAcceptingSocket.writeString(
	      "Only client allowed and it is already connected.");
      myAcceptingSocket.close();
      MvrLog::log(MvrLog::Terse, "%s not taking multiple clients and another client tried to connect from %s.", myName.c_str(), myAcceptingSocket.getIPString());
    }
    else 
    {
      // we want the client so we put it in our list of connecting
      // sockets, which means that it is waiting to give its password
      socket = new MvrSocket;
      socket->setLogWriteStrings(myLoggingDataSent);
      socket->setStringUseWrongEndChars(myUseWrongEndChars);
      socket->transfer(&myAcceptingSocket);
      socket->setIPString((myName + "::" + socket->getIPString()).c_str());
      socket->setNonBlock();
      if (!myPassword.empty())
      {
	socket->writeString("Enter password:");
	myConnectingConns.push_front(socket);
	MvrLog::log(MvrLog::Normal, 
		   "%s: Client connecting from %s.",
		   myName.c_str(), socket->getIPString());
      }
      else
      {
	MvrLog::log(MvrLog::Normal, 
		   "%s: Client from %s connected (with no password required).",
		   myName.c_str(), socket->getIPString());
	myConns.push_front(socket);
	internalGreeting(socket);
      }
    }
  }

  // now we read in data from our connecting sockets and see if
  // they've given us the password
  for (it = myConnectingConns.begin(); it != myConnectingConns.end(); ++it)
  {
    socket = (*it);
    // read in what the client has to say
    if ((str = socket->readString()) != NULL)
    {
      if (str[0] == '\0')
	continue;
      // now see if the word matchs the password
      if (myPassword == str)
      {
	MvrLog::log(MvrLog::Normal, 
		   "%s: Client from %s gave password and connected.",
		   myName.c_str(), socket->getIPString());
	myConns.push_front(socket);
	removeList.push_front(socket);
	internalGreeting(socket);
      }
      else
      {
	socket->close();
	myDeleteList.push_front(socket);
	MvrLog::log(MvrLog::Terse, 
		   "%s: Client from %s gave wrong password and is being disconnected.", 
		   myName.c_str(), socket->getIPString());
      }
    }
    // if we couldn't read a string it means we lost a connection
    else
    {
      MvrLog::log(MvrLog::Normal, 
		 "%s: Connection to %s lost.", 
		 myName.c_str(), socket->getIPString());
      socket->close();
      myDeleteList.push_front(socket);
    }
  }
  // now we clear out the ones we want to remove from our connecting
  // clients list
  while ((it = removeList.begin()) != removeList.end())
  {
    socket = (*it);
    myConnectingConns.remove(socket);
    removeList.pop_front();
  }



  // first send it all the things to be sent the next cycle... this
  // could be done in the for loop below this one, but since we want
  // to only log once per broadcast it's done here
  for (ncsIt = nextCycleSends.begin(); 
       ncsIt != nextCycleSends.end(); 
       ncsIt++)
  {

    // now we read in data from our connected sockets 
    for (it = myConns.begin(); it != myConns.end() && myOpened; ++it)
    {
      socket = (*it);

      socket->setLogWriteStrings(false);

      if (myLoggingDataSent)
	MvrLog::log(MvrLog::Terse, "%s::sendToAllClientsNextCycle: Sending: %s", myName.c_str(), (*ncsIt).c_str());
      socket->writeString((*ncsIt).c_str());
      
      socket->setLogWriteStrings(myLoggingDataSent);
    }
  }

  // now we read in data from our connected sockets 
  for (it = myConns.begin(); it != myConns.end() && myOpened; ++it)
  {
    socket = (*it);

    // read in what the client has to say
    while ((str = socket->readString()) != NULL)
    {
      // if this is null then there wasn't anything said
      if (str[0] == '\0')
	break;
      // make sure we read something
      // set up the arguments and then call the function for the
      // argument
      args = new MvrArgumentBuilder;
      args->addPlain(str);
      //args->log();
      parseCommandOnSocket(args, socket);
      delete args;
      args = NULL;
    }
    // if str was NULL we lost connection
    if (str == NULL)
    {
      MvrLog::log(MvrLog::Normal, 
		 "%s: Connection to %s lost.", myName.c_str(), 
		 socket->getIPString());
      socket->close();
      myDeleteList.push_front(socket);
    }
  }

  // now we delete the ones we want to delete (we could do this above
  // but then it wouldn't be symetrical with above)
  while ((it = myDeleteList.begin()) != myDeleteList.end())
  {
    socket = (*it);
    myConnectingConns.remove(socket);
    myConns.remove(socket);
    // remove this instead of the old pop since there could be two of
    // the same in the list if it lost connection exactly when it
    // parsed the quit
    myDeleteList.remove(socket);
    socket->close();
    delete socket;
  }

  if (myWantToClose)
  {
    close();
  }
  unlock();
}

MVREXPORT void MvrNetServer::close(void)
{
  std::list<MvrSocket *>::iterator it;
  MvrSocket *socket;
  
  if (!myOpened)
    return;
  myWantToClose = false;
  MvrLog::log(MvrLog::Normal, "%s shutting down server.", myName.c_str());
  sendToAllClients("Shutting down server");
  for (it = myConnectingConns.begin(); it != myConnectingConns.end(); ++it)
  {
    (*it)->writeString("Shutting down server");
  }
  myOpened = false;

  while ((it = myConnectingConns.begin())!= myConnectingConns.end())
  {
    socket = (*it);
    myConnectingConns.pop_front();
    socket->close();
    delete socket;
  }
  while ((it = myConns.begin()) != myConns.end())
  {
    socket = (*it);
    myConns.pop_front();
    socket->close();
    delete socket;
  }
  myServerSocket.close();
}

MVREXPORT void MvrNetServer::internalGreeting(MvrSocket *socket)
{
  if (mySquelchNormal)
    return;
  socket->writeString("Welcome to the server.");
  socket->writeString("You can type 'help' at any time for the following help list.");
  internalHelp(socket);
}

MVREXPORT void MvrNetServer::internalHelp(MvrSocket *socket)
{
 std::map<std::string, std::string, MvrStrCaseCmpOp>::iterator it;
  
 socket->writeString("Commands:");
 for (it = myHelpMap.begin(); it != myHelpMap.end(); ++it)
   socket->writeString("%15s%10s%s", it->first.c_str(), "", 
		       it->second.c_str());
 socket->writeString("End of commands");
}

MVREXPORT void MvrNetServer::internalHelp(char **argv, int argc, 
					MvrSocket *socket)
{
  internalHelp(socket);
}


MVREXPORT void MvrNetServer::internalEcho(char **argv, int argc, 
					    MvrSocket *socket)
{
  // if they just typed it we tell them if its on or off
  if (argc == 1)
  {
    if (socket->getEcho())
      socket->writeString("Echo is on.");
    else
      socket->writeString("Echo is off.");
  }
  // if the have two words see if they have the right args
  else if (argc == 2 && strcasecmp(argv[1], "on") == 0)
  {
    socket->writeString("Echo turned on.");
    socket->setEcho(true);
  }
  else if (argc == 2 && strcasecmp(argv[1], "off") == 0)
  {
    socket->writeString("Echo turned off.");
    socket->setEcho(false);
  }
  else
  {
    socket->writeString("usage: echo <on/off>");
  }
}

MVREXPORT void MvrNetServer::internalQuit(char **argv, int argc, 
					 MvrSocket *socket)
{
  socket->writeString("Closing connection");

  myDeleteList.push_front(socket);
  MvrLog::log(MvrLog::Normal, "%s: Client from %s quit.", 
	     myName.c_str(), socket->getIPString());
}

MVREXPORT void MvrNetServer::internalShutdownServer(char **argv, int argc, 
						  MvrSocket *socket)
{
  sendToAllClients("Shutting down server");
  myWantToClose = true;
  if (myRobot != NULL)
    myRobot->stopRunning();
  
}

MVREXPORT void MvrNetServer::parseCommandOnSocket(MvrArgumentBuilder *args, 
						MvrSocket *socket, bool allowLog)
{

  std::map<std::string, MvrFunctor3<char **, int, MvrSocket *> *, MvrStrCaseCmpOp>::iterator fIt;
  char **argv;
  int argc;

  if (myLoggingDataReceived && !mySquelchNormal && allowLog)
    MvrLog::log(MvrLog::Normal, "%s: Command received from %s: %s",
	       myName.c_str(), socket->getIPString(), args->getFullString());
  else if (myLoggingDataReceived && mySquelchNormal && allowLog)
    MvrLog::log(MvrLog::Normal, "%s: %s",
	       socket->getIPString(), args->getFullString());
  argv = args->getArgv();
  argc = args->getArgc();
  // if we have some command see if it has a functor
  if (argc >= 1 && 
      (fIt = myFunctorMap.find(argv[0])) != myFunctorMap.end())
  {
    fIt->second->invoke(argv, argc, socket);
  }
  // it didn't have a functor so we don't know it as a command
  else if (argc >= 1)
  {
    if (!mySquelchNormal)
      socket->writeString("Unknown command %s", argv[0]);
  }
}

MVREXPORT void MvrNetServer::internalAddSocketToList(MvrSocket *socket)
{
  if (socket != NULL)
  {
    socket->setNonBlock();
    socket->setStringUseWrongEndChars(myUseWrongEndChars);
  }
  myConns.push_front(socket);
}


MVREXPORT void MvrNetServer::internalAddSocketToDeleteList(MvrSocket *socket)
{
  myDeleteList.push_front(socket);
}

MVREXPORT void MvrNetServer::squelchNormal(void)
{
  mySquelchNormal = true;
  remCommand("help");
  remCommand("echo");
  remCommand("quit");
  remCommand("shutdownServer");

}

MVREXPORT void MvrNetServer::sendToClientPlain(
	MvrSocket *socket, const char *ipString, const char *str)
{
  std::list<MvrSocket *>::iterator it;

  for (it = myConns.begin(); it != myConns.end(); ++it)
  {
    if ((*it) == socket && strcmp((*it)->getIPString(), ipString) == 0)
    {
      if (myLoggingDataSent)
	MvrLog::log(MvrLog::Terse, 
		   "%s::sendToClient: Sending '%s' to %s", myName.c_str(), str,
		   ipString);
      (*it)->setLogWriteStrings(false);
      (*it)->writeString(str);
      (*it)->setLogWriteStrings(myLoggingDataSent);
    }
  }
}

/**
   This sends the given string to all the clients, this string cannot
   be more than 2048 number of bytes
**/
MVREXPORT void MvrNetServer::sendToClient(MvrSocket *socket, const char *ipString,
			   const char *str, ...)
{
  char buf[2049];
  va_list ptr;
  va_start(ptr, str);
  vsprintf(buf, str, ptr);

  sendToClientPlain(socket, ipString, buf);
  
  va_end(ptr);
}

