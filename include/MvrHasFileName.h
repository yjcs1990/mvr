#ifndef MVRHASFILENAME_H
#define MVRHASFILENAME_H

#include "mvriaTypedefs.h"

/// Interface to access an object's associated file name.
/**
 * MvrHasFileName provides a single abstract method which should be overridden
 * to return the complete file path name of the associated file.  It is 
 * implemented by classes that have external persistent storage, such as 
 * MvrConfig and MvrMap.
 * 
 * Copyright (c) Adept Technology, Inc. All rights reserved.
**/
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


#endif // ARHASFILENAME_H

