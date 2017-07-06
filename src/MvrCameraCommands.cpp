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
#include "MvrExport.h"
#include "mvriaUtil.h"
#include "MvrCameraCommands.h"


MVREXPORT const char *MvrCameraCommands::GET_CAMERA_DATA = "getCameraData";
MVREXPORT const char *MvrCameraCommands::GET_CAMERA_INFO = "getCameraInfo";

MVREXPORT const char *MvrCameraCommands::GET_PICTURE  = "getPicture";
MVREXPORT const char *MvrCameraCommands::GET_PICTURE_OPTIM  = "getPictureOptim";
MVREXPORT const char *MvrCameraCommands::GET_SNAPSHOT = "getSnapshot";

MVREXPORT const char *MvrCameraCommands::GET_DISPLAY = "getDisplay";


MVREXPORT const char *MvrCameraCommands::SET_CAMERA_ABS = "setCameraAbs";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_REL = "setCameraRel";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_PCT = "setCameraPct";

MVREXPORT const char *MvrCameraCommands::GET_CAMERA_MODE_LIST = "getCameraModeList";
MVREXPORT const char *MvrCameraCommands::CAMERA_MODE_UPDATED = "cameraModeUpdated";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_MODE = "setCameraMode";

MVREXPORT const char *MvrCameraCommands::RESET_CAMERA = "resetCamera";
// TODO Make these the original packet names?  Move to MvrNetworking?

MVREXPORT const char *MvrCameraCommands::GET_CAMERA_DATA_INT = "getCameraDataInt";
MVREXPORT const char *MvrCameraCommands::GET_CAMERA_INFO_INT = "getCameraInfoInt";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_ABS_INT = "setCameraAbsInt";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_REL_INT = "setCameraRelInt";
MVREXPORT const char *MvrCameraCommands::SET_CAMERA_PCT_INT = "setCameraPctInt";
MVREXPORT const char *MvrCameraCommands::GET_VIDEO = "getVideo";

MVREXPORT const char *MvrCameraCommands::GET_SNAPSHOT_PLAIN = "getSnapshotPlain";

