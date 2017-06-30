/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrDrawingData.h
 > Description  : A class for holding color information Data
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRDRAWINGDATA_H
#define MVRDRAWINGDATA_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

class MvrColor
{
public:
  /// Constructor (colors use full range of 0-255)
  MvrColor(unsigned char red, unsigned char green, unsigned char blue)
  { myRed = red; myGreen = green; myBlue = blue; }
  /// Constructor
  MvrColor() { myRed = 255; myGreen = 255; myBlue = 255; }

  /// Constructs a color from the given RGB value
  MvrColor(MvrTypes::Byte4 rgbValue)
  {
    myRed   = (rgbValue & 0xFF0000) >> 16;
    myGreen = (rgbValue & 0x00FF00) >> 8;
    myBlue  = (rgbValue & 0x0000FF) ;
  }

  /// Destructor
  virtual ~MvrColor() {}
  /// Gets the red value (uses full range of 0-255)
  unsigned char getRed(void) { return myRed; }
  /// Gets the green value (uses full range of 0-255)
  unsigned char getGreen(void) { return myGreen; }
  /// Gets the blue value (uses full range of 0-255)
  unsigned char getBlue(void) { return myBlue; }
  /// Gets the color in a byte 4 for putting into a buffer
  MvrTypes::Byte4 colorToByte4(void) 
  { return ((myRed << 16) | (myGreen << 8) | myBlue); }
protected:
  unsigned char myRed;
  unsigned char myGreen;
  unsigned char myBlue;
};

class MvrDrawingData
{
public:

  enum {
    DEFAULT_REFRESH_TIME = 200 ///< Default number of ms between data refresh requests
  };


  /// Constructor
  /**
     @param shape the name of the shape to draw  (see above / MobileEyes docs for meaning)
     @param primaryColor the main color (meaning depends on shape)
     @param size the size (meaning varies depends on shape)
     @param layer the layer to draw on (see above / MobileEyes docs for meaning)
     @param defaultRefreshTime how often we want to draw it in ms
     @param visibility a string that indicates whether the data is visible 
            whether the user is allowed to change the visibility (see above / MobileEyes
            docs for valid values).  
   **/
  MvrDrawingData(const char *shape, 
			           MvrColor primaryColor, 
			           int size,
			           int layer, 
			           unsigned int defaultRefreshTime = DEFAULT_REFRESH_TIME, 
                 const char *visibility = "DefaultOn") :
    myShape(shape),
    myPrimaryColor(primaryColor),
    mySize(size),
    myLayer(layer),
    myDefaultRefreshTime(defaultRefreshTime),
    mySecondaryColor(MvrColor(0,0,0)),
    myVisibility(visibility)
  {}

  /// Constructor
  /**
     @param shape the name of the shape to draw  (see above / MobileEyes docs for meaning)
     @param primaryColor the main color (meaning depends on shape)
     @param size the size (meaning varies depends on shape)
     @param layer the layer to draw on (see above / MobileEyes docs for meaning)
     @param defaultRefreshTime how often we want to draw it in ms
     @param secondaryColor the secondary color (meaning depends on shape)
     @param visibility a string that indicates whether the data is visible 
            whether the user is allowed to change the visibility (see above / MobileEyes
            docs for valid values).  
   **/
  MvrDrawingData(const char *shape, 
                 MvrColor primaryColor, 
                 int size,
                 int layer, 
                 unsigned int defaultRefreshTime, 
                 MvrColor secondaryColor,
                 const char *visibility = "DefaultOn")
    { 
      myShape = shape; 
      myPrimaryColor = primaryColor; 
      mySize = size; 
      myLayer = layer; 
      myDefaultRefreshTime = defaultRefreshTime;
      mySecondaryColor = secondaryColor; 
      myVisibility = visibility;
    }
  /// Destructor
  virtual ~MvrDrawingData() {}
  /// Returns the shape of data to draw
  const char * getShape(void) { return myShape.c_str(); }
  /// Gets the primary color (meaning depending on shape)
  MvrColor getPrimaryColor(void) { return myPrimaryColor; }
  /// Gets the size (meaning depends on shape, but its in mm)
  int getSize(void) { return mySize; }
  /// Gets the layer to draw at (see MobileEyes docs for what layer means)
  int getLayer(void) { return myLayer; }
  /// Gets how often this data should be drawn (0 == only when explicitly sent)
  unsigned int getDefaultRefreshTime(void) { return myDefaultRefreshTime; }
  /// Gets the secondary color (meaning depends on shape)
  MvrColor getSecondaryColor(void) { return mySecondaryColor; }
  /// Gets the visibility of the drawing data
  const char *getVisibility(void) { return myVisibility.c_str(); }

  /// Sets the shape of data to draw
  void setShape(const char *shape) { myShape = shape; }
  /// Sets the primary color (meaning depends on shape)
  void setPrimaryColor(MvrColor color) { myPrimaryColor = color; }
  /// Sets the size (meaning depends on shape, but its in mm)
  void setSize(int size) { mySize = size; }
  /// Sets the layer (see MobileEyes for docs on what layer means)
  void setLayer(int layer) { myLayer = layer; }
  /// Gets how often this data should be drawn (0 == only when explicitly sent)
  void setDefaultRefreshTime(unsigned int refreshTime)
    { myDefaultRefreshTime = refreshTime; }
  /// Sets the secondary color (meaning depends on shape)
  void setSecondaryColor(MvrColor color) { mySecondaryColor = color; }
  /// Sets the visibility of the drawing data
  void setVisibility(const char *visibility) { myVisibility = visibility; }
protected:
  std::string myShape;
   MvrColor myPrimaryColor;
  int mySize;
  int myLayer;
  unsigned int myDefaultRefreshTime;
  MvrColor mySecondaryColor;
  std::string myVisibility;
};
#endif  // MVRDRAWINGDATA_H