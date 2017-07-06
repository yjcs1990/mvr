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
#ifndef ARNETSERVER_H
#define ARNETSERVER_H

#include "mvriaTypedefs.h"
#include "MvrSocket.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"

#include <list>


class MvrRobot;

class MvrArgumentBuilder;

/// Class for running a simple net server to send/recv commands via text
/**
   This class is for running a simple server which will have a
   list of commands to use and a fairly simple set of interactions...
   Start the server with the open() function, add commands with the
   addCommand() function and remove commands with remCommand(), and close
   the server with the close() function.

   A client can connect via TCP on the port provided to open() and send
   a line of text where the first word is the command and the following
   words are extra arguments or data (space separated).  The line should
   end with a newline ("\n") or carriage return character.   The first
   line sent should be a password and must match the password given to
   open() in order to continue.

   You can use the "telnet" program as a general client to any MvrNetServer server.

   It has a built in mutex, if you only use sendToAllClients() through
   the normal commands or during the robot loop you don't need to
   worry about locking anything and the server is locked before any of
   the callbacks for the commands are called so you really only need
   to lock the server if you're dealing with from another thread....
   From another thread you can use sendToAllClientsNextCycle which
   takes care of all the locking itself in a threadsafe way (it puts
   the message in a list, then sends it in the next cycle of the
   loop).  The only real reason to use the
   lock/sendToAllClients/unlock method is if you're highly concerned
   about synchronizing the different types of output.

    @ingroup OptionalClasses
**/
class MvrNetServer
{
public:
  /// Constructor
  MVREXPORT MvrNetServer(bool addMvrExitCB = true,
		       bool doNotAddShutdownServer = false, 
		       const char *name = "MvrNetServer", 
		       MvrNetServer *childServer = NULL);
  /// Destructor
  MVREXPORT ~MvrNetServer();
  
  /// Initializes the server
  MVREXPORT bool open(MvrRobot *robot, unsigned int port, 
		     const char *password, bool multipleClients = true,
		     const char *openOnIP = NULL);

  /// Closes the server
  MVREXPORT void close(void);

  /// Adds a new command
  MVREXPORT bool addCommand(const char *command, 
			   MvrFunctor3<char **, int, MvrSocket *> *functor, 
			   const char *help);

  /// Removes a command
  MVREXPORT bool remCommand(const char *command);

  /// Gets the name of this instance
  const char *getName(void) { return myName.c_str(); }
#ifndef SWIG
  /** @brief Sends the given string to all the clients.  See also the
   *  notes on locking in the class description.
   *  @swigomit @sa
   *  sendToAllClientsPlain()
   */
  MVREXPORT void sendToAllClients(const char *str, ...);
#endif

  /// Sends the given string to all the clients, no varargs, wrapper for java
  MVREXPORT void sendToAllClientsPlain(const char *str);

#ifndef SWIG
  /** @brief Sends the given string to all the clients next cycle
   *  @swigomit
   *  @sa sendToAllClientsNextCyclePlain()
   */
  MVREXPORT void sendToAllClientsNextCycle(const char *str, ...);
#endif

  /// Sends the given string to all the clients next cycle, no varargs
  MVREXPORT void sendToAllClientsNextCyclePlain(const char *str);

  /// Sends the given string to all the clients next cycle, no varargs... helper for config changes
  MVREXPORT bool sendToAllClientsNextCyclePlainBool(const char *str);

#ifndef SWIG
  /** @brief Sends the given string to the (hopefully) the client given (this method may go away)
   *  @swigomit
   *  @sa sendToClientPlain()
   */
  MVREXPORT void sendToClient(MvrSocket *socket, const char *ipString,
			     const char *str, ...);
#endif

  /// Sends the given plain string to the (hopefully) the client given (this method may go away)
  MVREXPORT void sendToClientPlain(MvrSocket *socket, const char *ipString,
				  const char *str);

  /// Sees if the server is running and open
  MVREXPORT bool isOpen(void);

  /// Sets whether we are logging all data sent or not
  MVREXPORT void setLoggingDataSent(bool loggingData);
  
  /// Gets whether we are logging all data sent or not
  MVREXPORT bool getLoggingDataSent(void);

  /// Sets whether we are logging all data received or not
  MVREXPORT void setLoggingDataReceived(bool loggingData);
  
  /// Gets whether we are logging all data received or not
  MVREXPORT bool getLoggingDataReceived(void);

  /// Sets whether we're using the wrong (legacy) end chars or not
  MVREXPORT void setUseWrongEndChars(bool useWrongEndChars);

  /// Gets whether we're using the wrong (legacy) end chars or not
  MVREXPORT bool getUseWrongEndChars(void);

  /// the internal sync task we use for our loop
  MVREXPORT void runOnce(void);

  /// the internal function that gives the greeting message
  MVREXPORT void internalGreeting(MvrSocket *socket);
  
  /// The internal function that does the help
  MVREXPORT void internalHelp(MvrSocket *socket);
  /// The internal function for the help cb
  MVREXPORT void internalHelp(char **argv, int argc, MvrSocket *socket);
  /// The internal function for echo
  MVREXPORT void internalEcho(char **argv, int argc, MvrSocket *socket);
  /// The internal function for closing this connection
  MVREXPORT void internalQuit(char **argv, int argc, MvrSocket *socket);
  /// The internal function for shutting down
  MVREXPORT void internalShutdownServer(char **argv, int argc, 
				       MvrSocket *socket);
  /// The internal function for parsing a command on a socket
  MVREXPORT void parseCommandOnSocket(MvrArgumentBuilder *args, 
				     MvrSocket *socket, bool allowLog = true);
  /// The internal function that adds a client to our list
  MVREXPORT void internalAddSocketToList(MvrSocket *socket);
  /// The internal function that adds a client to our delete list
  MVREXPORT void internalAddSocketToDeleteList(MvrSocket *socket);
  /// This squelchs all the normal commands and help
  MVREXPORT void squelchNormal(void);
  /// Sets an extra string that the server holds for passing around
  MVREXPORT void setExtraString(const char *str) { myExtraString = str; }
  /// Gets an extra string that the server holds for passing around
  MVREXPORT const char *getExtraString(void) { return myExtraString.c_str(); }
  /// Lock the server
  MVREXPORT int lock() {return(myMutex.lock());}
  /// Try to lock the server without blocking
  MVREXPORT int tryLock() {return(myMutex.tryLock());}
  /// Unlock the server
  MVREXPORT int unlock() {return(myMutex.unlock());}
protected:
  std::string myName;
  MvrNetServer *myChildServer;
  MvrMutex myMutex;
  MvrSocket myAcceptingSocket;
  std::map<std::string, MvrFunctor3<char **, int, MvrSocket *> *, MvrStrCaseCmpOp> myFunctorMap;
  std::map<std::string, std::string, MvrStrCaseCmpOp> myHelpMap;
  bool myLoggingDataSent;
  bool myLoggingDataReceived;
  bool myUseWrongEndChars;
  bool myOpened;
  bool myWantToClose;
  bool mySquelchNormal;
  MvrSocket myServerSocket;
  MvrRobot *myRobot;
  std::string myPassword;
  bool myMultipleClients;
  unsigned int myPort;
  std::string myExtraString;
  std::list<MvrSocket *> myConns;
  std::list<MvrSocket *> myConnectingConns;
  std::list<MvrSocket *> myDeleteList;
  
  MvrMutex myNextCycleSendsMutex;
  std::list<std::string> myNextCycleSends;
  
  MvrFunctorC<MvrNetServer> myTaskCB;
  MvrFunctor3C<MvrNetServer, char **, int, MvrSocket *> myHelpCB;
  MvrFunctor3C<MvrNetServer, char **, int, MvrSocket *> myEchoCB;
  MvrFunctor3C<MvrNetServer, char **, int, MvrSocket *> myQuitCB;
  MvrFunctor3C<MvrNetServer, char **, int, MvrSocket *> myShutdownServerCB;
  MvrFunctorC<MvrNetServer> myMvrExitCB;
};

#endif // ARNETSERVER_H
