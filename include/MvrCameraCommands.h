#ifndef MVRCAMERACOMMANDS_H
#define MVRCAMERACOMMANDS_H

#include "mvriaUtil.h"

/// Commands that are supported by the camera (MvrPTZ) and video classes.
/**
 * MvrCameraCommands defines the commands that are commonly supported by 
 * the Mvria camera and video classes.  The commands that are actually 
 * supported for a particular camera are specified in the MvrCameraCollection.
 * <p>
 * There are basically two classes of commands:
 * <ol>
 *   <li>Double Format Commands:  These commands use the new double interface 
 *   to the Mvria camera classes.  They are generally preferred for future
 *   development since they provide finer control over the camera movements. 
 *   In addition, only these commands will work correctly in a multi-camera 
 *   configuration.</li>
 * 
 *   <li>Integer Format Commands:  These commands are maintained for backwards 
 *   compatibility.  They will not work correctly in a multi-camera 
 *   configuration.  (Only the first camera will be used.)</li>
 * </ol>
 *
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
  /**
   * @deprecated GET_CAMERA_DATA is preferred instead
  **/
 	MVREXPORT static const char *GET_CAMERA_DATA_INT;

  /// Returns general information about the camera (as byte2 integers).
  /**
   * @deprecated GET_CAMERA_INFO is preferred instead
  **/
  MVREXPORT static const char *GET_CAMERA_INFO_INT;

  /// Sends a command to the camera to set its pan/tilt/zoom, in absolute terms (as byte integers).
  /**
   * @deprecated SET_CAMERA_ABS is preferred instead
  **/
	MVREXPORT static const char *SET_CAMERA_ABS_INT;

  /// Sends a command to the camera to set its pan/tilt/zoom, relative to its current position (as byte integers).
  /**
   * @deprecated SET_CAMERA_REL is preferred instead
  **/
	MVREXPORT static const char *SET_CAMERA_REL_INT;

  /// Sends a command to the camera to set its pan/tilt, as specified by a percent offset (as byte integers).
  /**
   * @deprecated SET_CAMERA_PCT is preferred instead
  **/
	MVREXPORT static const char *SET_CAMERA_PCT_INT;
 
  /**
   * @deprecated GET_PICTURE is preferred instead
  **/
  MVREXPORT static const char *GET_VIDEO;
  
  /**
   * @deprecated GET_SNAPSHOT is preferred instead
  **/
  MVREXPORT static const char *GET_SNAPSHOT_PLAIN;

}; // end class MvrCameraCommands

#endif // MVRCAMERACOMMANDS_H

