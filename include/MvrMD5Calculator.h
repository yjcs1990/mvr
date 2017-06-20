/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMD5Calculator.h
 > Description  : Calculates the MD5 checksum when reading or writing a text file using MvrFunctors.
 > Author       : Yu Jie
 > Create Time  : 2017年06月20日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#ifndef MVRMD5CALCULATOR_H
#define MVRMD5CALCULATOR_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "md5.h"

/*
 * MvrMD5Calculator is a small helper class used to calculate MD5 checksums
 * on text files.  It contains a static method that will calculate the 
 * checksum of a given file, independently performing all I/O.  
 * 
 * The calculator also contains a functor that can be used to calculate
 * a checksum interactively.  For example, it can accumulate the checksum 
 * while a file is being written using a writeToFunctor method.  If the 
 * optional second functor is specified, then it will be invoked after the 
 * checksum functor. 
 */

#endif  // MVRMD5CALCULATOR_H
