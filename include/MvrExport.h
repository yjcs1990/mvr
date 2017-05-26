/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrExport.cpp
 > Description  : This class is the Base class for actions
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年04月24日
***************************************************************************************************/
#ifndef MVREXPORT_H
#define MVREXPORT_H

/// Including this header file forces AREXPORT to be defined as the dllexport declaration in Windows DLL builds.
/// It should only be included in source files in ARIA and other libraries. Header files should include ariaTypedefs instead.

#if (defined(_WIN32) || defined(WIN32)) && !defined(MINGW)

#ifndef SWIG
#ifndef MVRIA_STATIC
#undef MVREXPORT
#define MVREXPORT _declspec(dllexport)
#else // MVRIA_STATIC
#define MVREXPORT
#endif // MVRIA_STATIC
#endif // SWIG

#else // WIN32

#define MVREXPORT

#endif // WIN32

#endif // MVREXPORT_H