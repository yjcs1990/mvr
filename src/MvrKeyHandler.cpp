#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrKeyHandler.h"
#include "MvrLog.h"

#ifdef WIN32
#include <conio.h>
#else // if not win32
#include <stdio.h>
#endif

#include "mvriaInternal.h"

/**
   @param blocking whether or not to block waiting on keys, default is
   false, ie not to wait... you probably only want to block if you are
   using checkKeys yourself like after you start a robot run or in its
   own thread or something along those lines
   @param addMvriaExitCB true to add an aria exit cb to restore the keys 
   @param stream the FILE * pointer to use, if this is NULL (the default)
   then use stdin, otherwise use this...
   @param takeKeysInConstructor whether to take the keys when created or not
   (default is true)
**/
MVREXPORT MvrKeyHandler::MvrKeyHandler(bool blocking, bool addMvriaExitCB, 
				    FILE *stream, 
				    bool takeKeysInConstructor) :
  myMvriaExitCB(this, &MvrKeyHandler::restore)
{
  myMvriaExitCB.setName("MvrKeyHandlerExit");
  if (addMvriaExitCB)
    Mvria::addExitCallback(&myMvriaExitCB);

  myStream = stream;
  myTookKeys = false;

  if (takeKeysInConstructor)
    takeKeys(blocking);
}

MVREXPORT MvrKeyHandler::~MvrKeyHandler()
{
  Mvria::remExitCallback(&myMvriaExitCB);
  restore();
}

MVREXPORT void MvrKeyHandler::takeKeys(bool blocking)
{
  myBlocking = blocking;
#ifndef WIN32
  struct termios newTermios;

  if (myStream == NULL)
  {
    tcgetattr(fileno(stdin), &myOriginalTermios);
    tcgetattr(fileno(stdin), &newTermios);
  }
  else
  {
    tcgetattr(fileno(myStream), &myOriginalTermios);
    tcgetattr(fileno(myStream), &newTermios);
  }

  newTermios.c_cc[VTIME] = 0;
  if (myBlocking)
  {
    newTermios.c_cc[VMIN] = 1;
  }
  else
  {
/*
  We need 0 here for linux (otherwise the read blocks) but we need 1
  for mac osx, there's probably better def to check someday.
*/
#ifdef linux
    newTermios.c_cc[VMIN] = 0;
#else 
    newTermios.c_cc[VMIN] = 1;
#endif 
  }
  newTermios.c_lflag &= (~ECHO & ~ICANON);

  if (myStream == NULL)
    tcsetattr(fileno(stdin), TCSANOW, &newTermios);
  else
    tcsetattr(fileno(myStream), TCSANOW, &newTermios);

#endif
  myRestored = false;
  myTookKeys = true;
}

MVREXPORT void MvrKeyHandler::restore(void)
{
  if (!myTookKeys)
    return;

#ifndef WIN32
  if (myStream == NULL)
    tcsetattr(fileno(stdin), TCSANOW, &myOriginalTermios);
  else
    tcsetattr(fileno(myStream), TCSANOW, &myOriginalTermios);

#endif
  myRestored = true;
  myTookKeys = false;
}

/**
   @param keyToHandle A character value, such as 'a' or '1'
     or '[', or a member of the KEY enum.

   @param functor Functor to call when @a keyToHandle is received

   @return true If no previous key handler functor exists for @a keyToHandle and
   the handler functor was stored, or false if a handler for that key already 
   exists.
   that key
*/
MVREXPORT bool MvrKeyHandler::addKeyHandler(int keyToHandle, MvrFunctor *functor)
{
  if (myMap.find(keyToHandle) != myMap.end())
  {
    if (keyToHandle >= '!' && keyToHandle <= '~')
      MvrLog::log(MvrLog::Normal, "There is already a key to handle '%c' which is number %d 0x%x", keyToHandle, keyToHandle, keyToHandle);
    else
      MvrLog::log(MvrLog::Normal, "There is already a key to handle number %d 0x%x", keyToHandle, keyToHandle);
    return false;
  }
  //MvrLog::log(MvrLog::Verbose, "keyhandler %p added key '%c' number '%d'", 
  //this, keyToHandle, keyToHandle);
  myMap[keyToHandle] = functor;

  return true;
}

/**
   @param keyToHandle The character value or code to clear the handler for.

    @return true if the remKeyHandler succeeded, which means that a
     key handler for @a keyToHandle was found and rmeoved, or false if no
     handler for that value was found.
 **/
MVREXPORT bool MvrKeyHandler::remKeyHandler(int keyToHandle)
{
  if (myMap.find(keyToHandle) == myMap.end())
  {
    //MvrLog::log(MvrLog::Normal, "There is no key to handle '%c' which is number %d 0x%x", keyToHandle, keyToHandle, keyToHandle);
    return false;
  }
  if (keyToHandle >= '!' && keyToHandle <= '~')
    MvrLog::log(MvrLog::Verbose, "keyhandler %p removed key '%c' number '%d'",
	       this, keyToHandle, keyToHandle);
  else
    MvrLog::log(MvrLog::Verbose, "keyhandler %p removed key number '%d'",
	       this, keyToHandle);
  myMap.erase(keyToHandle);
  return true;
}

/**
   @param functor the functor of the handler to remove

   @return true if the remKeyHandler succeeded, which means that the
    functor was found and removed from the handlers, or false if no
    handler with the given functor was found.
 **/
MVREXPORT bool MvrKeyHandler::remKeyHandler(MvrFunctor *functor)
{
  std::map<int, MvrFunctor *>::iterator it;
  std::list<std::map<int, MvrFunctor *>::iterator> iters;
  std::list<std::map<int, MvrFunctor *>::iterator>::iterator iterIter;

  for (it = myMap.begin(); it != myMap.end(); ++it)
  {
    if (it->second == functor)
    {
      iters.push_front(it);
    }
  }
  if (iters.size() > 0)
  {
    while((iterIter = iters.begin()) != iters.end())
    {
      myMap.erase((*iterIter));
      iters.pop_front();
    }
    MvrLog::log(MvrLog::Verbose, "keyhandler %p removed functor %p", this, 
	       functor);
    return true;
  }
  return false;
}

MVREXPORT void MvrKeyHandler::checkKeys(void)
{
  int key;
  std::map<int, MvrFunctor *>::iterator it;

  if (myRestored)
    return;

  // get keys until they're gone
  while (!myRestored && (key = getKey()) != -1)
  {
    // if there's something to handle it, handle it
    if ((it = myMap.find(key)) != myMap.end())
    {
      //MvrLog::log(MvrLog::Verbose, "key '%c' num %d pressed\n", key, key);
      it->second->invoke();
    }
  }
}

#ifndef WIN32

MVREXPORT int MvrKeyHandler::getChar(void)
{
  if (myStream == NULL)
    return getchar();
  else
    return getc(myStream);
}

MVREXPORT int MvrKeyHandler::getKey(void)
{
 /*
  * What follows is a somewhat poor implementation of getch(), basically, since
  * we want to get control keys but don't want to have to use curses.
  */

  int key;
  int k[5] = {-1, -1, -1, -1, -1};   // used for escape sequences

  key = getChar();
  switch(key)
  {
    case -1:
    case 0:
      return -1;

    //case -1: return ESCAPE; //?
    case ' ': return SPACE;
    case '\t': return TAB;
    case 10: return ENTER;
    case 13: return ENTER;
    case 8: return BACKSPACE;
    case 127: return BACKSPACE;

    case 27: // Escape key, or Begin special control key sequence
      key = getChar();
      switch(key)
      {
        case -1: return ESCAPE;
        case 79: // first four F keys
          key = getChar();
          switch(key)
          {
            case 80: return F1;
            case 81: return F2;
            case 82: return F3;
            case 83: return F4;
            case 70: return END;
            case 72: return HOME;
            default: return key;
          }
        case '[':  // keypad keys and extended F-key sequences start with this 
          // Read in all characters in the special key sequence. -1 is the
          // terminator (no more keys). We also check for the beginning of a new special key
          // sequence (27) since if the key repeat is high enough, they begin
          // overlapping. TODO: check trailing keys for modifiers line shift and set an
          // (optional parameter) bitmask.
          for(short i = 0; key != -1 && key != 27 && i < 5; i++) 
          {
            k[i] = key = getChar();
            //printf("MvrKeyHandler::getKey: read extended key component %d/%d.\n", k[i], key);
          }
          ungetc(key, stdin); // put last key back. (Esp. important if it was the beginning of a new control sequence (27).

          switch(k[0])
          {
            case 65: return UP;
            case 66: return DOWN;
            case 67: return RIGHT;
            case 68: return LEFT; 

            case 51: return DEL;
            case 53: return PAGEUP;
            case 54: return PAGEDOWN;

            case 50: 
              switch(k[1])
              {
                case 126: return INSERT;
                case 48:  return F9;
                case 49:  return F10;
                case 51:  return F11;
                case 52:  return F12;
              }
              return k[1];


            case 49:
              switch(k[1])
              {
                case 53: return F5;
                case 55: return F6;
                case 56: return F7;
                case 57: return F8;
              }
              return k[1];
            default: return -1;
          }
        default: return -1;
      }

    default: return key;
  }
}

#if 0
/* This is a previous implementation of getKey(), just for reference or
 * quick reversion: */
MVREXPORT int MvrKeyHandler::getKey(void)
{
  char key;

  key = getChar();
  if (key == -1 || key == 0)
      return -1;

  if (key == 27)
  {
    key = getChar();
    if (key == '[')
    {
      key = getChar();
      if (key == 'A')
        return UP;
      else if (key == 'B')
        return DOWN;
      else if (key == 'C')
        return RIGHT;
      else if (key == 'D')
        return LEFT;
      else
        return getKey();
    }
    else if (key == -1)
      return ESCAPE;
    else if (key == 79)
    {
       key = getChar();
       if (key == 'P')
         return F1;
       else if (key == 'Q')
         return F2;
       else if (key == 'R')
         return F3;
       else if (key == 'S')
         return F4;
       else
         return getKey();
    }
  }
  else if (key == ' ')
    return SPACE;
  else if (key == '\t')
    return TAB;
  else if (key == 10)
    return ENTER;
  else if (key == 8 || key == 127)
    return BACKSPACE;
  return key;
}
#endif


#else // if it is win32

MVREXPORT int MvrKeyHandler::getKey(void)
{
  int key;

  if (!myBlocking && !kbhit())
    return -1;

  key = _getch();
  if (key == 224)
  {
    key = _getch();
    if (key == 'H')
      return UP;
    else if (key == 'P')
      return DOWN;
    else if (key == 'K')
      return LEFT;
    else if (key == 'M')
      return RIGHT;
    else 
      return getKey();
  }
  else if (key == 0)
  {
    key = _getch();
    if (key == ';')
      return F1;
    else if (key == '<')
      return F2;
    else if (key == '=')
      return F3;
    else if (key == '>')
      return F4;
    else if (key == 'H')
      return UP;
    else if (key == 'P')
      return DOWN;
    else if (key == 'K')
      return LEFT;
    else if (key == 'M')
      return RIGHT;
    else
      return getKey();
  }
  else if (key == ' ')
    return SPACE;
  else if (key == '\t')
    return TAB;
  else if (key == 10)
    return ENTER;
  else if (key == 13)
    return ENTER;
  else if (key == 8)
    return BACKSPACE;
  else if (key == 27)
	return ESCAPE;

  return key;

}

#endif // WIN32


