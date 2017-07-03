/// Represents an x, y position with an orientation
class MvrPose
{
public:
  /// Constructor, with optional initial values
  /** 
      @param x the double to set the x position to, default of 0
      @param y the double to set the y position to, default of 0
      @param th the double value for the pose's heading (or th), default of 0
  */
  MvrPose(double x = 0, double y = 0, double th = 0) :
    myX(x),
    myY(y),
    myTh(MvrMath::fixAngle(th))
  {}
    
  /// Copy Constructor
  MvrPose(const MvrPose &pose) : 
    myX(pose.myX), myY(pose.myY), myTh(pose.myTh) {}

  /// Destructor
  virtual ~MvrPose() {}
  /// Sets the position to the given values
  /** 
      @param x the position to set the x position to
      @param y the position to set the y position to
      @param th the position to set the th position to, default of 0
  */
  virtual void setPose(double x, double y, double th = 0);

  /// Sets the position equal to the given position
  /** @param position the position value this instance should be set to */
  virtual void setPose(MvrPose position);
  /// Sets the x position
  void setX(double x);
  /// Sets the y position
  void setY(double y);
  /// Sets the heading
  void setTh(double th);
  /// Sets the heading, using radians
  void setThRad(double th);
  /// Gets the x position
  double getX(void) const;
  /// Gets the y position
  double getY(void) const;
  /// Gets the heading
  double getTh(void) const;
  /// Gets the heading, in radians
  double getThRad(void) const;

  /// Gets the whole position in one function call
  void getPose(double *x, double *y, double *th = NULL) const;

  /// Finds the distance from this position to the given position
  virtual double findDistanceTo(MvrPose position) const;

  /// Finds the square distance from this position to the given position
  virtual double squaredFindDistanceTo(MvrPose position) const;

  /// Finds the angle between this position and the given position
  virtual double findAngleTo(MvrPose position) const;
  /// Logs the coordinates using MvrLog
  virtual void log(void) const
    { MvrLog::log(MvrLog::Terse, "%.0f %.0f %.1f", myX, myY, myTh); }

  /// Add the other pose's X, Y and theta to this pose's X, Y, and theta (sum in theta will be normalized to (-180,180)), and return the result
  virtual MvrPose operator+(const MvrPose& other) const;

  /// Substract the other pose's X, Y, and theta from this pose's X, Y, and theta (difference in theta will be normalized to (-180,180)), and return the result

  virtual MvrPose operator-(const MvrPose& other) const;
  
  /// Adds the given pose to this one.

	MvrPose & operator+= ( const MvrPose & other);

	/// Subtracts the given pose from this one.
	MvrPose & operator-= ( const MvrPose & other);


  /// Finds the distance between two poses
  static double distanceBetween(MvrPose pose1, MvrPose pose2);

  /// Return true if the X value of p1 is less than the X value of p2
  static bool compareX(const MvrPose& p1, const MvrPose &p2);

  /// Return true if the Y value of p1 is less than the X value of p2
  static bool compareY(const MvrPose& p1, const MvrPose &p2);

  bool isInsidePolygon(const std::vector<MvrPose>& vertices) const;

protected:

  double myX;
  double myY;
  double myTh;
};


/// A class for time readings and measuring durations

class MvrTime
{
public:
  /// Constructor. Time is initialized to the current time.
  MvrTime() { setToNow(); }

  /// Copy constructor
  MvrTime(const MvrTime &other) :
    mySec(other.mySec),
    myMSec(other.myMSec)
  {}

  /// Assignment operator 
  MvrTime &operator=(const MvrTime &other) ;

  /// Destructor
  ~MvrTime() {}
  
  /// Gets the number of milliseconds since the given timestamp to this one
  long mSecSince(MvrTime since) const ;
    
  /// Gets the number of milliseconds since the given timestamp to this one
  long long mSecSinceLL(MvrTime since) const ;

  /// Gets the number of seconds since the given timestamp to this one
  long secSince(MvrTime since) const;

  /// Gets the number of seconds since the given timestamp to this one
  long long secSinceLL(MvrTime since) const;

  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  long mSecTo(void) const;

  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  long long mSecToLL(void) const;

  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  long secTo(void) const;

  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  long long secToLL(void) const;

  /// Finds the number of milliseconds from this timestamp to now
  long mSecSince(void) const;

  /// Finds the number of milliseconds from this timestamp to now
  long long mSecSinceLL(void) const;

  /// Finds the number of seconds from when this timestamp was set to now
  long secSince(void) const;

  /// Finds the number of seconds from when this timestamp was set to now
  long long secSinceLL(void) const;

  /// returns whether the given time is before this one or not
  bool isBefore(MvrTime testTime) const;

  /// returns whether the given time is equal to this time or not
  bool isAt(MvrTime testTime) const;
  /// returns whether the given time is after this one or not
  /// @ingroup easy
  bool isAfter(MvrTime testTime) const;

  /// Resets the time
  MVREXPORT void setToNow(void);

  /// Add some milliseconds (can be negative) to this time
  bool addMSec(long ms);

  /// Add some milliseconds (can be negative) to this time
  bool addMSecLL(long long ms);
  
  /// Sets the seconds value (since the arbitrary starting time)
  void setSec(unsigned long sec);
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSec(unsigned long msec);
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long getSec(void) const;
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long getMSec(void) const;

  /// Sets the seconds value (since the arbitrary starting time)
  void setSecLL(unsigned long long sec);
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSecLL(unsigned long long msec);
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long long getSecLL(void) const;
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long long getMSecLL(void) const;

  /// Logs the time
  void log(const char *prefix = NULL) const;
  /// Gets if we're using a monotonic (ever increasing) clock
  static bool usingMonotonicClock()
    {
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
      return ourMonotonicClock;
#endif
#ifdef WIN32
      return true;
#endif
      return false;
    }
  
  /// Equality operator (for sets)
  bool operator==(const MvrTime& other) const
  {
    return isAt(other);
  }

  bool operator!=(const MvrTime& other) const
  {
    return (!isAt(other));
  }
 
  // Less than operator for sets
  /// @ingroup easy
  bool operator<(const MvrTime& other) const
  {
    return isBefore(other);
  } // end operator <

  /// @ingroup easy
  bool operator>(const MvrTime& other) const
  {
    return isAfter(other);
  }

protected:
  unsigned long long mySec;
  unsigned long long myMSec;
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  static bool ourMonotonicClock;
#endif 

}; // end class MvrTime