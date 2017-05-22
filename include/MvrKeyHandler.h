/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrKeyHandler.h
 > Description  : Perform actions when keyboard keys are pressed
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
/// @ingroup OptionalClasses
#ifndef MVRKEYHANDLER_H
#define MVRKEYHANDLER_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include <map>
#include <stdio.h>

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#endif

class MvrKeyHandler
{
public:
  /// This object will take over key capture when constructed, and release
  /// kye capture when destroyed
  MVREXPORT MvrKeyHandler(bool blocking=false, bool addMvriaExitCB=true, FILE *stream=NULL,
                          bool takeKeysInConstructor=true);
  /// Destructor. Reseases control of the keyboard and restores state before
  /// this key handler was created.
  MVREXPORT ~MvrKeyHandler();

  /// These are symbols for the non-ascii keys
  enum KEY {
    UP = 256,       /// < Up arrow (keypad or 4 key dirs)
    DOWN,           /// < Down arrow (keypad or 4 kye dirs)
    LEFT,           /// < Left arrow (keypad or 4 kye dirs)
    RIGHT,          /// < Right arrow (keypad or 4 kye dirs)
    ESCAPE,         /// < Escape key
    SPACE,          /// < Space key
    TAB,            /// < Tab key
    ENTER,          /// < Enter key
    BACKSPACE,      /// < Backspace key
    _StartFKeys,    /// < F key offset (internal, don;t use).
    F1,             /// < F1
    F2,             /// < F2
    F3,             /// < F3
    F4,             /// < F4
    F5,             /// < F5 (not supported on Windows yet)
    F6,             /// < F6 (not supported on Windows yet)
    F7,             /// < F7 (not supported on Windows yet)
    F8,             /// < F8 (not supported on Windows yet)
    F9,             /// < F9 (not supported on Windows yet)
    F10,            /// < F10 (not supported on Windows yet)
    F11,            /// < F11 (not supported on Windows yet)
    F12,            /// < F12 (not supported on Windows yet)
    _EndFKeys,      /// < F key range (internal; don't use)
    PAGEUP,         /// < Page Up (not supported on Windows yet)
    PAGEDOWN,       /// < Page Down (not supported on Windows yet)
    HOME,           /// < Home key (not supported on Windows yet)
    END,            /// < End key (not supported on Windows yet)
    INSERT,         /// < Insert key (not supported on Windows yet)
    DEL             /// < Special delete key (often forward-delete) (not supported on Windows yet)
  };

  /// This adds a keyhandler, when the keyToHandle is hit, functor will fire
protected:
};

#endif  // MVRKEYHANDLER_H