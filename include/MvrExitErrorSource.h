#ifndef MVREXITERRORSOURCE_H
#define MVREXITERRORSOURCE_H 

/// Small interface for obtaining exit-on-error information
/**
 * MvrExitErrorSource may be implemented by classes that wish to 
 * provide information if and when they cause an erroneous 
 * application exit.  The use of this interface is entirely at 
 * the discretion of the application.  Mvr does not invoke its
 * methods.
**/
class MvrExitErrorSource 
{
public:
 
  /// Constructor
  MvrExitErrorSource() {}

  /// Destructor
  virtual ~MvrExitErrorSource() {}

  /// Returns a textual description of the error source.
  /**
   * @param buf a char array in which the method puts the output 
   * error description 
   * @param bufLen the int number of char's in the array
   * @return bool true if the description was successfully written;
   * false if an error occurred
  **/
  virtual bool getExitErrorDesc(char *buf, int bufLen) = 0;

  /// Returns a textual description of the error source intended for a user (it will be prefixed by something stating the action taking place)
  /**
   * @param buf a char array in which the method puts the output user 
   * error description 
   * @param bufLen the int number of char's in the array
   * @return bool true if the description was successfully written;
   * false if an error occurred
  **/
  virtual bool getExitErrorUserDesc(char *buf, int bufLen) = 0;

  /// Returns the error code used for the exit call
  /**
   * Ideally, the returned error code should be unique across all
   * error sources.  (Past implementations have the method spec and
   * body on a single line so that it's easily searchable...  Current
   * implementations have that OR this string on a line so that it'll
   * show up searchable easily still).
  **/
  virtual int getExitErrorCode() const = 0;

}; // end class MvrExitErrorSource

#endif // AREXITERRORSOURCE_H
