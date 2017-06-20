/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrKeyHandler.cpp
 > Description  : Perform actions when keyboard keys are pressed
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaInternal.h"
#include "MvrLog.h"
#include "MvrKeyHandler.h"

#ifdef WIN32
#include <conio.h>
#else
#include <stdio.h>
#endif

/*
 * @param blocking whether or not to block waiting on keys, default is
 * false, ie not to wait... you probably only want to block if you are
 * using checkKeys yourself like after you start a robot run or in its
 * own thread or something along those lines
 * @param addMvriaExitCB true to add an aria exit cb to restore the keys 
 * @param stream the FILE * pointer to use, if this is NULL (the default)
 * then use stdin, otherwise use this...
 * @param takeKeysInConstructor whether to take the keys when created or not
 * (default is true)
 */
MVREXPORT MvrKeyHandler::MvrKeyHandler(bool blocking, bool addMvriaExitCB, 
                                       FILE *stream, bool takeKeysInConstructor) :
          myMvriaExitCB(this, &MvrKeyHandler::restore)                                       
{
  myMvriaExitCB.setName("MvrKeyHandlerExit");
  
}                                       