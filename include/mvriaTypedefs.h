#ifndef MVRTYPEDEFS_H
#define MVRTYPEDEFS_H

#include <time.h>
#include <string>
#include <map>
#include <list>

#ifdef WIN32


#ifndef SWIG
#if !defined(ARIA_STATIC) && !defined(MVREXPORT) && !defined(MINGW)
#define MVREXPORT _declspec(dllimport)
#elif !defined(MVREXPORT) // ARIA_STATIC
#define MVREXPORT
#endif // ARIA_STATIC
#else
#define MVREXPORT
#endif

#include <winsock2.h>
#include <windows.h>

#endif //WIN32L


#ifndef WIN32

#define MVREXPORT
////
//// Linux
////

#endif // linux


typedef std::map<int, std::string> MvrStrMap;

/// has enum for position in list
class MvrListPos
{
public:
  typedef enum {
      FIRST = 1, ///< place item first in the list
      LAST = 2 ///< place item last in the list
  } Pos;
};

/// Contains platform independent sized vmvriable types
class MvrTypes
{
public:
  /// A single signed byte
  typedef char Byte;
  /// Two signed bytes
  typedef short Byte2;
  /// Four signed bytes
  typedef int Byte4;
  /// Eight signed bytes
  typedef long long Byte8;

  /// A single unsigned byte
  typedef unsigned char UByte;
  /// Two unsigned bytes
  typedef unsigned short UByte2;
  /// Four unsigned bytes
  typedef unsigned int UByte4;
  /// Eight unsigned bytes
  typedef unsigned long long UByte8;
};


#endif
