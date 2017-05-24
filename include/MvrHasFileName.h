/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrHasFileName.h
 > Description  : Interface to access an object's associated file name.
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRHASFILENAME_H
#define MVRHASFILENAME_H

#include "mvriaTypedefs.h"

/// Interface to access an object's associated file name.

class MvrHasFileName
{
public:

  /// Constructor
	MVREXPORT MvrHasFileName() 
  {}
	
  /// Copy constructor
  MvrHasFileName(MvrHasFileName const &) 
  {}

	/// Assignment operator.
  MvrHasFileName &operator=(MvrHasFileName const & )
  {
    return *this;
  }

  /// Destructor
	MVREXPORT virtual ~MvrHasFileName()
  {}

  /// Returns the complete file path name of the associated file
  MVREXPORT virtual const char *getFileName() const  = 0;

}; // end class


#endif // MVRHASFILENAME_H