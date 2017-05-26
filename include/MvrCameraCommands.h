/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrCameraCommands.h
 > Description  : Commands that are supported by the camera (MvrPTZ) and video classes.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRCAMERACOMMANDS_H
#define MVRCAMERACOMMANDS_H

#include "mvriaUtil.h"

/**
 * MvrCameraCommands defines the commands that are commonly supported by 
 * the Mvria camera and video classes.  The commands that are actually 
 * supported for a particular camera are specified in the MvrCameraCollection.
**/
class MvrCameraCommands
{
public:

  // ---------------------------------------------------------------------------
  // Double Format Commands
  // ---------------------------------------------------------------------------

  /// Returns the current pan/tilt/zoom of the camera (in double format).
	MVREXPORT static const char *GET_CAMERA_DATA;

  /// Returns general information about the camera (in double format).
  /**
   * General information about the camera includes the pan/tilt/zoom ranges
   * and whether zooming is available.  In general, clients need only 
   * to request this data once at startup.
  **/
  MVREXPORT static const char *GET_CAMERA_INFO;

  /// Returns the last image obtained from the camera.
  MVREXPORT static const char *GET_PICTURE;
  
  /// Returns the last image obtained from the camera.
  MVREXPORT static const char *GET_PICTURE_OPTIM;

  /// Returns a high resolution image from the camera, suitable for saving.
  /**
   * This command is not yet fully supported.
  **/
	MVREXPORT static const char *GET_SNAPSHOT;

  /// Returns whether the camera/image should be displayed by the client.
  /**
   * This command is not yet fully supported.
  **/
	MVREXPORT static const char *GET_DISPLAY;
	

  /// Sends a command to the camera to set its pan/tilt/zoom, in absolute terms (in double format).
	MVREXPORT static const char *SET_CAMERA_ABS;

  /// Sends a command to the camera to set its pan/tilt, as specified by a percent offset of its current frame.
	MVREXPORT static const char *SET_CAMERA_PCT;

  /// Sends a command to the camera to set its pan/tilt/zoom, relative to its current position (in double format).
	MVREXPORT static const char *SET_CAMERA_REL;

  /// Sends a packet back with the list of modes
  MVREXPORT static const char *GET_CAMERA_MODE_LIST;

  /// A packet that gets sent out when the mode is changed
  /**
     Note that each mode might have arguments after it for just that
     mode, so only the mode name at the start is generic.
   **/
  MVREXPORT static const char *CAMERA_MODE_UPDATED;

  /// A command that sets the mode of the camera
  /**
     Note that each mode can have its own arguments so only the mode
     name at the start is generic.
   **/
  MVREXPORT static const char *SET_CAMERA_MODE;

  /// A command that resets the camera 
  MVREXPORT static const char *RESET_CAMERA;

  // ---------------------------------------------------------------------------
  // Integer Format Commands
  //    The following commands are maintained solely for backwards-compatibility. 
  // ---------------------------------------------------------------------------


  /// Returns the current pan/tilt/zoom of the camera (as byte2 integers).
 	MVREXPORT static const char *GET_CAMERA_DATA_INT;

  /// Returns general information about the camera (as byte2 integers).
  MVREXPORT static const char *GET_CAMERA_INFO_INT;

  /// Sends a command to the camera to set its pan/tilt/zoom, in absolute terms (as byte integers).
	MVREXPORT static const char *SET_CAMERA_ABS_INT;

  /// Sends a command to the camera to set its pan/tilt/zoom, relative to its current position (as byte integers).
	MVREXPORT static const char *SET_CAMERA_REL_INT;

  /// Sends a command to the camera to set its pan/tilt, as specified by a percent offset (as byte integers).
	MVREXPORT static const char *SET_CAMERA_PCT_INT;
 
  MVREXPORT static const char *GET_VIDEO;
  
  MVREXPORT static const char *GET_SNAPSHOT_PLAIN;

}; // end class MvrCameraCommands

#endif // MVRCAMERACOMMANDS_H
