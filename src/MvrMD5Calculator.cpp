#include "MvrExport.h"
#include "MvrMD5Calculator.h"

#include "mvriaOSDef.h"
#include "mvriaInternal.h"

#include "MvrLog.h"

MVREXPORT MvrMD5Calculator::MvrMD5Calculator(MvrFunctor1<const char*> *secondFunctor) :
  myFunctor(this, &MvrMD5Calculator::append),
  mySecondFunctor(secondFunctor),
  myState(),
  myDigest(),
  myIsFinished(false)
{
  myDigest[0] = '\0';
  md5_init(&myState);
}

MVREXPORT MvrMD5Calculator::~MvrMD5Calculator()
{
}
  
MVREXPORT void MvrMD5Calculator::reset()
{
  myDigest[0] = '\0';
  md5_init(&myState);

  myIsFinished = false;
}

MVREXPORT MvrFunctor1<const char *> *MvrMD5Calculator::getFunctor()
{
  return &myFunctor;
}

MVREXPORT MvrFunctor1<const char *> *MvrMD5Calculator::getSecondFunctor()
{
  return mySecondFunctor;
}

MVREXPORT void MvrMD5Calculator::setSecondFunctor(MvrFunctor1<const char *> *secondFunctor)
{
  mySecondFunctor = secondFunctor;
}

MVREXPORT unsigned char *MvrMD5Calculator::getDigest()
{
  if (!myIsFinished) {
    md5_finish(&myState, myDigest);
    myIsFinished = true;
  }
  return myDigest;

} // end method getDigest


MVREXPORT void MvrMD5Calculator::toDisplay(const unsigned char *digestBuf,
                                         size_t digestLength,
                                         char *displayBuf,
                                         size_t displayLength)
{
  if ((displayBuf == NULL) || (digestBuf == NULL)) {
    return;
  }
  int j = 0;

  memset(displayBuf, 0, displayLength);

  for (int i = 0; 
        ((i < digestLength) && (j < displayLength - 1)); 
        i++) {

    j = i * 2;
    if (j > displayLength - 3) {
      break;
    }
    // Seems that we need the 3 to account for the end-of-string (because otherwise
    // (j + 1) holds the null char)
    snprintf(&displayBuf[j], 3, "%02x", digestBuf[i]);

    /***
    MvrLog::log(MvrLog::Normal,
               "MvrMD5Calculator::toDisplay()  i = %i  j = %i,  \"%02x\" = \"%x,%x\"",
               i, j, digestBuf[i], displayBuf[j], displayBuf[j + 1]);
    ***/
  } // end for each char

  displayBuf[displayLength - 1] = '\0'; 

  /***
  MvrLog::log(MvrLog::Normal,
             "MvrMD5Calculator::toDisplay() returns %s displayLength = %i",
             displayBuf,
             displayLength);
  ***/

} // end method toDisplay

MVREXPORT bool MvrMD5Calculator::calculateChecksum(const char *fileName,
                                                 unsigned char *md5DigestBuffer,
                                                 size_t md5DigestBufferLen)
{
  MvrTime calcTime;

  if (MvrUtil::isStrEmpty(fileName)) {
    // TODO MvrLog
    return false;
  }
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    // TODO MvrLog
    return false;
  }

  MvrMD5Calculator calculator;

  // TODO: Make this static and protect w/ mutex?
  char line[10000];
  bool ret = true;

  while (fgets(line, sizeof(line), file) != NULL)
  {
    calculator.append(line);
  }
  
  fclose(file);

  if (md5DigestBuffer != NULL) {
    if (md5DigestBufferLen != MvrMD5Calculator::DIGEST_LENGTH) {
      // log warning
    }
    memset(md5DigestBuffer, 0, md5DigestBufferLen);
    memcpy(md5DigestBuffer, calculator.getDigest(), 
          MvrUtil::findMin(md5DigestBufferLen, MvrMD5Calculator::DIGEST_LENGTH));
  }

  int elapsed = calcTime.mSecSince();
  
  MvrLog::log(MvrLog::Normal,
             "MvrMD5Calculator::calculateChecksum(%s) took %i msecs",
             fileName, elapsed);

  return true;


} // end method calculateChecksum



MVREXPORT void MvrMD5Calculator::append(const char *str)
{
  if (str == NULL) {
    MvrLog::log(MvrLog::Terse,
               "MvrMD5Calculator::append cannot append null string");
  }
  // YUCK to the cast!
  md5_append(&myState, (unsigned char *) str, strlen(str));

  if (mySecondFunctor != NULL) {
    mySecondFunctor->invoke(str);
  }

} // end method append

