/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSocket.cpp
 > Description  : For connecting to a device through a TCP network socket
 > Author       : Yu Jie
 > Create Time  : 2017年05月18日
 > Modify Time  : 2017年06月15日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrFunctor.h"
#include "MvrSocket.h"

#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

/// We're always initialized in Linux
bool MvrSocket::ourInitialized = true;

bool MvrSocket::init()
{
  return true;
}

