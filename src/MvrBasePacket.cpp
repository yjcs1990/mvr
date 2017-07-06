#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrBasePacket.h"
#include "MvrLog.h"

#include <stdio.h>
#include <string.h>

/**
@param bufferSize size of the buffer
@param headerLength length of the header
@param buf buffer packet uses, if NULL, instance will allocate memory
@param footerLength length of the footer following the data
*/
MVREXPORT MvrBasePacket::MvrBasePacket(MvrTypes::UByte2 bufferSize, 
                                    MvrTypes::UByte2 headerLength,
                                    char * buf,
                                    MvrTypes::UByte2 footerLength) 
{
  if (buf == NULL && bufferSize > 0) 
  {
    myOwnMyBuf = true;
    myBuf = new char[bufferSize];
    // memset(myBuf, 0, bufferSize);
  } 
  else 
  {
    myOwnMyBuf = false;
    myBuf = buf;
  }
  myHeaderLength = headerLength;
  myFooterLength = footerLength;
  myReadLength = myHeaderLength;
  myMaxLength = bufferSize;
  myLength = myHeaderLength;
  myIsValid = true;
}


MVREXPORT MvrBasePacket::MvrBasePacket(const MvrBasePacket &other) :
  myHeaderLength(other.myHeaderLength),
  myFooterLength(other.myFooterLength),
  myMaxLength(other.myLength),
  myReadLength(other.myReadLength),
  myOwnMyBuf(true),
  myBuf((other.myLength > 0) ? new char[other.myLength] : NULL),
  myLength(other.myLength),
  myIsValid(other.myIsValid)
{
  if ((myBuf != NULL) && (other.myBuf != NULL)) {
    memcpy(myBuf, other.myBuf, myLength);
  }
}

MVREXPORT MvrBasePacket &MvrBasePacket::operator=(const MvrBasePacket &other)
{
  if (this != &other) {

    myHeaderLength = other.myHeaderLength;
    myFooterLength = other.myFooterLength;
    myReadLength   = other.myReadLength;

    if (myLength != other.myLength) {
      if (myOwnMyBuf && myBuf != NULL)
	delete [] myBuf;
      myOwnMyBuf = true;
      myBuf = NULL;
      if (other.myLength > 0) {
        myBuf = new char[other.myLength];
      }
      myLength = other.myLength;
      myMaxLength = other.myLength;
    }

    if ((myBuf != NULL) && (other.myBuf != NULL)) {
      memcpy(myBuf, other.myBuf, myLength);
    }
   
    myIsValid = other.myIsValid;
  }
  return *this;
}



MVREXPORT MvrBasePacket::~MvrBasePacket()
{
  if (myOwnMyBuf && myBuf != NULL)
    delete[] myBuf;
}


MVREXPORT void MvrBasePacket::setBuf(char *buf, MvrTypes::UByte2 bufferSize)
{
  if (myOwnMyBuf) 
  {
    delete[] myBuf;
    myOwnMyBuf = false;
  } 
  myBuf = buf;
  myMaxLength = bufferSize;
}

MVREXPORT void MvrBasePacket::setMaxLength(MvrTypes::UByte2 bufferSize)
{
  if (myMaxLength >= bufferSize)
    return;
  if (myOwnMyBuf) 
  {
    delete[] myBuf;
    myOwnMyBuf = false;
  } 
  myBuf = new char[bufferSize];
  // memset(myBuf, 0, bufferSize);

  myMaxLength = bufferSize;
  myOwnMyBuf = true;
}

MVREXPORT bool MvrBasePacket::setLength(MvrTypes::UByte2 length)
{
  if (myOwnMyBuf && length > myMaxLength)
    return false;

  myLength = length;
  return true;
}

MVREXPORT void MvrBasePacket::setReadLength(MvrTypes::UByte2 readLength)
{
  myReadLength = readLength;
}

MVREXPORT bool MvrBasePacket::setHeaderLength(MvrTypes::UByte2 length)
{
  if (myOwnMyBuf && length > myMaxLength)
    return false;

  myHeaderLength = length;
  return true;
}

/** 
Sets the length read back to the header length so the packet can be
reread using the other methods
*/

MVREXPORT void MvrBasePacket::resetRead(void)
{
  myReadLength = myHeaderLength;
  resetValid();
}

MvrTypes::UByte2 MvrBasePacket::getDataLength(void) const { 
 
  // KMC 12/20/13 Do not allow negative values to be returned.  (They are basically 
  // converted to an erroneous positive value by the UByte2.)
  int len = myLength - myHeaderLength - myFooterLength; 
  if (len >= 0) {
    return len;
  }
  else {
/****
    MvrLog::log(MvrLog::Normal,
               "MvrBasePacket::getDataLength() negative myLength = %i, myHeaderLength = %i, myFooterLength = %i",
               myLength,
               myHeaderLength,
               myFooterLength);
***/
    return 0;
  }
}

/**
Sets the packet length back to be the packets header length again
*/

MVREXPORT void MvrBasePacket::empty(void)
{
  myLength = myHeaderLength;
  resetValid();
}

MVREXPORT bool MvrBasePacket::isNextGood(int bytes)
{
  if (bytes <= 0)
    return false;

  // make sure it comes in before the header
  if (myReadLength + bytes <= myLength - myFooterLength)
    return true;

  myIsValid = false;

  return false;
}


MVREXPORT bool MvrBasePacket::hasWriteCapacity(int bytes)
{
  if (bytes < 0) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::hasWriteCapacity(%d) cannot write negative amount",
               bytes);
    return false;
  }

  // Make sure there's enough room in the packet 
  if ((myLength + bytes) <= myMaxLength) {
     return true;
  }

  myIsValid = false;

  return false;

} // end method hasWriteCapacity


/**
 * A packet is considered "invalid" if an attempt is made to write too much
 * data into the packet, or to read too much data from the packet.  Calls to
 * empty() and resetRead() will restore the valid state.
**/
MVREXPORT bool MvrBasePacket::isValid(void)
{
  return myIsValid;

} // end method isValid

/**
 * Resets the packet to the "valid" state.  This method should generally
 * only be called externally when the application has taken some recovery
 * action.  For example, if an attempt to write a long string to the packet
 * fails (and isValid() returns false), then a smaller string may be written
 * instead.
**/
MVREXPORT void MvrBasePacket::resetValid()
{
  myIsValid = true;
}

MVREXPORT const char *MvrBasePacket::getBuf(void) const
{
  return myBuf;
}

MVREXPORT char *MvrBasePacket::getBuf(void) 
{
  return myBuf;
}

MVREXPORT void MvrBasePacket::byteToBuf(MvrTypes::Byte val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }

  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

MVREXPORT void MvrBasePacket::byte2ToBuf(MvrTypes::Byte2 val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }

  unsigned char c;
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

MVREXPORT void MvrBasePacket::byte4ToBuf(MvrTypes::Byte4 val)
{
  if (!hasWriteCapacity(4)) {
    return;
  }

  unsigned char c;
  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;

}

MVREXPORT void MvrBasePacket::byte8ToBuf(MvrTypes::Byte8 val)
{
  if (!hasWriteCapacity(8)) {
    return;
  }

  unsigned char c;
  c = (val >> 56) & 0xff;
  memcpy(myBuf+myLength+7, &c, 1);
  c = (val >> 48) & 0xff;
  memcpy(myBuf+myLength+6, &c, 1);
  c = (val >> 40) & 0xff;
  memcpy(myBuf+myLength+5, &c, 1);
  c = (val >> 32) & 0xff;
  memcpy(myBuf+myLength+4, &c, 1);

  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;

}

MVREXPORT void MvrBasePacket::uByteToBuf(MvrTypes::UByte val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }
  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

MVREXPORT void MvrBasePacket::uByte2ToBuf(MvrTypes::UByte2 val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }
  // Note that MSB is placed one byte after the LSB in the end of the buffer:
  unsigned char c;
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

MVREXPORT void MvrBasePacket::uByte4ToBuf(MvrTypes::UByte4 val)
{
  if (!hasWriteCapacity(4)) {
    return;
  }
  
  /*
  MPL 2013_10_23 this doesn't match anything else with regards to how
  it's happening, and while it didn't matter when we're just going
  from x86 to x86 it may matter for others... if it causes problems
  just put back the old code

  memcpy(myBuf+myLength, &val, 4);
  myLength += 4;
  */

  unsigned char c;
  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;
}

MVREXPORT void MvrBasePacket::uByte8ToBuf(MvrTypes::UByte8 val)
{
  if (!hasWriteCapacity(8)) {
    return;
  }
  /*
    MPL 2013_10_23 this was how would have matched the old uByte4ToBuf
    but since that didn't match anything else I changed it

    memcpy(myBuf+myLength, &val, 8);
    myLength += 8;
  */

  unsigned char c;
  c = (val >> 56) & 0xff;
  memcpy(myBuf+myLength+7, &c, 1);
  c = (val >> 48) & 0xff;
  memcpy(myBuf+myLength+6, &c, 1);
  c = (val >> 40) & 0xff;
  memcpy(myBuf+myLength+5, &c, 1);
  c = (val >> 32) & 0xff;
  memcpy(myBuf+myLength+4, &c, 1);

  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;
}

/**
@param str string to copy into buffer
*/
MVREXPORT void MvrBasePacket::strToBuf(const char *str)
{
  if (str == NULL) {
    str = "";
  }
  MvrTypes::UByte2 tempLen = strlen(str) + 1;

  if (!hasWriteCapacity(tempLen)) {
    return;
  }

  memcpy(myBuf+myLength, str, tempLen);
  myLength += tempLen;
}

/**
 * This method performs no bounds checking on the given length and
 * the contents of the string.  For string operations, strNToBufPadded()
 * is preferred.  For raw data operations, dataToBuf() is preferred.
@param str character array to copy into the packet buffer
@param length how many characters to copy from str into the packet buffer
*/
MVREXPORT void MvrBasePacket::strNToBuf(const char *str, int length)
{
  // Do not perform bounds checking because it breaks existing code.

  //byte4ToBuf(length);
  memcpy(myBuf+myLength, str, length);
  myLength+=length;

}


/**
If string ends before length it pads the string with NUL ('\\0') characters.
@param str character array to copy into buffer
@param length how many bytes to copy from the str into packet
*/
MVREXPORT void MvrBasePacket::strToBufPadded(const char *str, int length)
{
  if (str == NULL) {
    str = "";
  }
  MvrTypes::UByte2 tempLen = strlen(str);

  if (!hasWriteCapacity(length)) {
    return;
  }

  if (tempLen >= length) {
    memcpy(myBuf + myLength, str, length);
    myLength += length;
  }
  else // string is smaller than given length
  {
    memcpy(myBuf + myLength, str, tempLen);
    myLength += tempLen;
    memset(myBuf + myLength, 0, length - tempLen);
    myLength += length - tempLen;
  }
}


/**
@param data chacter array to copy into buffer
@param length how many bytes to copy from data into packet
*/
MVREXPORT void MvrBasePacket::dataToBuf(const char *data, int length)
{
  if (data == NULL) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::dataToBuf(NULL, %d) cannot add from null address", length);
    return;
  }

  if (!hasWriteCapacity(length)) {
    return;
  }

  memcpy(myBuf+myLength, data, length);
  myLength+=length;

}

/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.
@param data chacter array to copy into buffer
@param length how many bytes to copy from data into packet
*/
MVREXPORT void MvrBasePacket::dataToBuf(const unsigned char *data, int length)
{
  if (data == NULL) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::dataToBuf(NULL, %d) cannot add from null address",
               length);
    return;
  }

  if (!hasWriteCapacity(length)) {
    return;
  }

  memcpy(myBuf+myLength, data, length);
  myLength+=length;

}


MVREXPORT MvrTypes::Byte MvrBasePacket::bufToByte(void)
{
  MvrTypes::Byte ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

MVREXPORT MvrTypes::Byte2 MvrBasePacket::bufToByte2(void)
{
  MvrTypes::Byte2 ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (c1 & 0xff) | (c2 << 8);
    myReadLength+=2;
  }

  return ret;
}

MVREXPORT MvrTypes::Byte4 MvrBasePacket::bufToByte4(void)
{
  MvrTypes::Byte4 ret=0;
  unsigned char c1, c2, c3, c4;

  if (isNextGood(4))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    ret = (c1 & 0xff) | (c2 << 8) | (c3 << 16) | (c4 << 24);
    myReadLength+=4;
  }

  return ret;
}

MVREXPORT MvrTypes::Byte8 MvrBasePacket::bufToByte8(void)
{
  MvrTypes::Byte8 ret=0;
  unsigned char c1, c2, c3, c4, c5, c6, c7, c8;

  if (isNextGood(8))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    memcpy(&c5, myBuf+myReadLength+4, 1);
    memcpy(&c6, myBuf+myReadLength+5, 1);
    memcpy(&c7, myBuf+myReadLength+6, 1);
    memcpy(&c8, myBuf+myReadLength+7, 1);
    ret = ((MvrTypes::Byte8)c1 & 0xff) | ((MvrTypes::Byte8) c2 << 8) | ((MvrTypes::Byte8) c3 << 16) | ((MvrTypes::Byte8) c4 << 24) | ((MvrTypes::Byte8) c5 << 32) | ((MvrTypes::Byte8) c6 << 40) | ((MvrTypes::Byte8) c7 << 48) | ((MvrTypes::Byte8) c8 << 56);
    myReadLength+=8;
  }

  return ret;
}

MVREXPORT MvrTypes::UByte MvrBasePacket::bufToUByte(void)
{
  MvrTypes::UByte ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

MVREXPORT MvrTypes::UByte2 MvrBasePacket::bufToUByte2(void)
{
  MvrTypes::UByte2 ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (c1 & 0xff) | (c2 << 8);
    myReadLength+=2;
  }

  return ret;
}

MVREXPORT MvrTypes::UByte4 MvrBasePacket::bufToUByte4(void)
{
  /// MPL 2013_10_23 this was Byte4 not UByte4
  //MvrTypes::Byte4 ret=0;
  MvrTypes::UByte4 ret=0;
  unsigned char c1, c2, c3, c4;

  if (isNextGood(4))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    ret = (c1 & 0xff) | (c2 << 8) | (c3 << 16) | (c4 << 24);
    myReadLength+=4;
  }

  return ret;
}

MVREXPORT MvrTypes::UByte8 MvrBasePacket::bufToUByte8(void)
{
  MvrTypes::UByte8 ret=0;
  unsigned char c1, c2, c3, c4, c5, c6, c7, c8;

  if (isNextGood(8))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    memcpy(&c5, myBuf+myReadLength+4, 1);
    memcpy(&c6, myBuf+myReadLength+5, 1);
    memcpy(&c7, myBuf+myReadLength+6, 1);
    memcpy(&c8, myBuf+myReadLength+7, 1);
    ret = ((MvrTypes::UByte8)c1 & 0xff) | ((MvrTypes::UByte8)c2 << 8) | ((MvrTypes::UByte8)c3 << 16) | ((MvrTypes::UByte8)c4 << 24) | ((MvrTypes::UByte8)c5 << 32) | ((MvrTypes::UByte8)c6 << 40) | ((MvrTypes::UByte8)c7 << 48) | ((MvrTypes::UByte8) c8 << 56);
    myReadLength+=8;
  }

  return ret;
}

/** 
Copy a string from the packet buffer into the given buffer, stopping when
the end of the packet buffer is reached, the given length is reached,
or a NUL character ('\\0') is reached.  If the given length is not large
enough, then the remainder of the string is flushed from the packet.
A NUL character ('\\0') is appended to @a buf if there is sufficient room
after copying the sting from the packet, otherwise no NUL is added (i.e.
if @a len bytes are copied).
@param buf Destination buffer
@param len Maximum number of characters to copy into the destination buffer
*/
MVREXPORT void MvrBasePacket::bufToStr(char *buf, int len)
{
   if (buf == NULL) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::bufToStr(NULL, %d) cannot write to null address",
               len);
    return;
  }

  int i;

  buf[0] = '\0';
  // see if we can read
  if (isNextGood(1))
  {
    // while we can read copy over those bytes
    for (i = 0; 
         isNextGood(1) && i < (len - 1) && myBuf[myReadLength] != '\0';
         ++myReadLength, ++i) {
      buf[i] = myBuf[myReadLength];
    }
    // if we stopped because of a null then copy that one too
    if (myBuf[myReadLength] == '\0')
    {
      buf[i] = myBuf[myReadLength];
      myReadLength++;
    }
    else if (i >= (len - 1)) { 

      // Otherwise, if we stopped reading because the output buffer was full,
      // then attempt to flush the rest of the string from the packet

      // This is a bit redundant with the code below, but wanted to log the  
      // string for debugging
      myBuf[len - 1] = '\0';

      MvrLog::log(MvrLog::Normal, "MvrBasePacket::bufToStr(buf, %d) output buf is not large enough for packet string %s",
                 len, myBuf);

      while (isNextGood(1) && (myBuf[myReadLength] != '\0')) {
        myReadLength++;
      }
      if (myBuf[myReadLength] == '\0') {
        myReadLength++;
      }
    } // end else if output buffer filled before null-terminator
  } // end if something to read

  // Make absolutely sure that the string is null-terminated...
  buf[len - 1] = '\0';

}

/// Note the string obtained from the packet can have at most 512 characters.
MVREXPORT std::string MvrBasePacket::bufToString()
{
  char buf[512];
  bufToStr(buf, 512);
  return buf;
}

/**
copies length bytes from the buffer into data, length is passed in, not read
from packet
@param data character array to copy the data into
@param length number of bytes to copy into data
*/
MVREXPORT void MvrBasePacket::bufToData(char *data, int length)
{
  if (data == NULL) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::bufToData(NULL, %d) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += length;
  }
}


/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.

copies length bytes from the buffer into data, length is passed in, not read
from packet
@param data character array to copy the data into
@param length number of bytes to copy into data
*/
MVREXPORT void MvrBasePacket::bufToData(unsigned char *data, int length)
{
  if (data == NULL) {
    MvrLog::log(MvrLog::Normal, "MvrBasePacket::bufToData(NULL, %d) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += length;
  }
}


/**
Copies the given packets buffer into the buffer of this packet, also
sets this length and readlength to what the given packet has
@param packet the packet to duplicate
*/
MVREXPORT void MvrBasePacket::duplicatePacket(MvrBasePacket *packet)
{
  myLength = packet->getLength();
  myReadLength = packet->getReadLength();

  // KMC Added this because otherwise... If myMaxLength < packet->getMaxLength(),
  // then this will overwrite memory.
  //
  if (myMaxLength < myLength) {
    setMaxLength(myLength);
  }

  memcpy(myBuf, packet->getBuf(), myLength);
}

MVREXPORT void MvrBasePacket::log(void)
{
  int i;
  MvrLog::log(MvrLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    MvrLog::log(MvrLog::Terse, "  [%03i] % 5d\t0x%x", i,(unsigned char) myBuf[i],
        (unsigned char) myBuf[i]);
  MvrLog::log(MvrLog::Terse, "\n");
}

MVREXPORT void MvrBasePacket::printHex(void)
{
  int i;
  MvrLog::log(MvrLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    MvrLog::log(MvrLog::Terse, "  [%i] 0x%x ", i,(unsigned char) myBuf[i]);
  MvrLog::log(MvrLog::Terse, "\n");
}

