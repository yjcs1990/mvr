#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrNMEAParser.h"

#include <iostream>


//#define DEBUG_ARNMEAPARSER 1

#ifdef DEBUG_ARNMEAPARSER
void MvrNMEAParser_printBuf(FILE *fp, const char *data, int size) {  
  for(int i = 0; i < size; ++i) {
    if(data[i] == '\n')
      fprintf(fp, "[LF]");
    else if(data[i] == '\r')
      fprintf(fp, "[CR]");
    else if(data[i] < ' ' || data[i] > '~')  {
      fprintf(fp, "[0x%X]", data[i] & 0xff);    
    }
    else
      fputc(data[i], fp);    
  }
}
#endif

MVREXPORT MvrNMEAParser::MvrNMEAParser(const char *name) :
  myName(name),
  MaxNumFields(50),
  MaxFieldSize(128),
  ignoreChecksum(false),
  checksumBufOffset(0),
  inChecksum(false),
  inMessage(false),
  currentChecksum(0),
  gotCR(false)
{
  memset(checksumBuf, 0, 3);
}

MVREXPORT void MvrNMEAParser::addHandler(const char *message, MvrNMEAParser::Handler *handler)
{
  myHandlers[message] = handler;
}

MVREXPORT void MvrNMEAParser::removeHandler(const char *message)
{
  HandlerMap::iterator i = myHandlers.find(message);
  if(i != myHandlers.end()) myHandlers.erase(i);
}


void MvrNMEAParser::nextField()
{
  currentMessage.push_back(currentField);
  currentField = "";
  if (currentMessage.size() > MaxNumFields)
    endMessage();
}

void MvrNMEAParser::endMessage()
{
  inMessage = false;
  inChecksum = false;
  currentField = "";
  gotCR = false;
  currentMessage.clear();
}

void MvrNMEAParser::beginChecksum()
{
  checksumBufOffset = 0;
  inChecksum = true;
}

void MvrNMEAParser::beginMessage()
{
  currentMessageStarted.setToNow();
  currentMessage.clear();
  inChecksum = false;
  inMessage = true;
  currentField = "";
  gotCR = false;
  currentChecksum = 0;
  memset(checksumBuf, 0, sizeof(checksumBuf));
}


MVREXPORT int MvrNMEAParser::parse(MvrDeviceConnection *dev) 
{
  int n = dev->read(myReadBuffer, sizeof(myReadBuffer));
#ifdef DEBUG_ARNMEAPARSER
  std::cerr << "\t[MvrNMEAParser: read " << n << " bytes of data from device connection:\n\t";
  MvrNMEAParser_printBuf(stderr, myReadBuffer, n);
#endif
  if(n < 0) return ParseError;
  if(n == 0) return ParseFinished;
  return parse(myReadBuffer, n);
}

MVREXPORT int MvrNMEAParser::parse(const char *buf, int n)
{
  int result = 0;
  if (n < 0) 
  {
    return result|ParseError;
  }

  if (n == 0) 
  {
    return result|ParseFinished;
  }

#ifdef DEBUG_ARNMEAPARSER
  std::cerr << "\t[MvrNMEAParser: given " << n << " bytes of data.]\n";
  std::cerr << "\t[MvrNMEAParser: parsing chunk \"";
  MvrNMEAParser_printBuf(stderr, buf, n);
  std::cerr << "\"]\n";
#endif


  for (int i = 0; i < n; i++)
  {
    // Check for message start
    if (buf[i] == '$')
    {
      beginMessage();
      continue;
    }

    // Otherwise, we must be in a sentece to do anything
    if (!inMessage)
      continue;

    // Reached the CR at the end?
    if (buf[i] == '\r') 
    {
      gotCR = true;
      continue;
    }
  
    // Reached the Newline at the end?
    if (buf[i] == '\n') 
    {
      if (gotCR) 
      {
        // Got both CR and LF?-- then end of message

        if(!inChecksum)
        {
          // checksum should have preceded.
          MvrLog::log(MvrLog::Terse, "MvrNMEAParser: Missing checksum.");
          result |= ParseError;
          endMessage();
          continue;
        }

        // got CRLF but there was no data. Ignore.
        if(currentMessage.size() == 0)
        {
          endMessage();
          continue;
        }
        

        // ok:
        Message msg;
        msg.message = &currentMessage;
        msg.timeParseStarted = currentMessageStarted;
        msg.prefix = currentMessage[0].substr(0, 2);
        // TODO should we check for an accepted set of prefixes? (e.g. GP, GN,
        // GL, GB, BD, HC, PG, etc.)
        msg.id = currentMessage[0].substr(2);
#ifdef DEBUG_ARNMEAPARSER
        fprintf(stderr, "\t[MvrNMEAPArser: Input message has system prefix %s with message ID %s]\n", msg.prefix.c_str(), msg.id.c_str());
#endif
        std::string lastprefix = myLastPrefix[msg.id];
        if(lastprefix != "" && lastprefix != msg.prefix)
        {
          const char *id = msg.id.c_str();
          const char *p = msg.prefix.c_str();
          const char *lp = lastprefix.c_str();
          MvrLog::log(MvrLog::Normal, "MvrNMEAParser: Warning: Got duplicate %s message with prefix %s (previous prefix was %s).  Data from %s%s will replace %s%s.", id, p, lp, p, id, lp, id);
        }
        HandlerMap::iterator h = myHandlers.find(msg.id);
        if (h != myHandlers.end()) 
        {
#ifdef DEBUG_ARNMEAPARSER
          fprintf(stderr, "\t[MvrNMEAParser: Got complete message, calling handler for %s...]\n", msg.id.c_str());
#endif
          if(h->second)
          {
            h->second->invoke(msg);
            result |= ParseUpdated;
          }
          else
          {
            MvrLog::log(MvrLog::Terse, "MvrNMEAParser Internal Error: NULL handler functor for message %s!\n", msg.id.c_str());
          }
        }
#ifdef DEBUG_ARNMEAPARSER
        else
        {
          fprintf(stderr, "\t[MvrNMEAParser: Have no message handler for %s (%s).]\n", msg.id.c_str(), currentMessage[0].c_str());
        }
#endif
      }
      else
      {
        MvrLog::log(MvrLog::Normal, "MvrNMEAParser: syntax error, \\n without \\r.");
        result |= ParseError;
      }

      endMessage();
      continue;
    }

    // Are we in the final checksum field?
    if (inChecksum)
    {
      checksumBuf[checksumBufOffset++] = buf[i];
      if (checksumBufOffset > 1)   // two bytes of checksum
      {
        int checksumRec = (int) strtol(checksumBuf, NULL, 16);
        if (checksumRec != currentChecksum) 
        {
          MvrLog::log(MvrLog::Normal, "%s: Warning: Skipping message with incorrect checksum.", myName);

          // reconstruct message to log:
          std::string nmeaText = "";
          for(MessageVector::const_iterator i = currentMessage.begin(); i != currentMessage.end(); ++i)
          {
            if(i != currentMessage.begin()) nmeaText += ",";
            nmeaText += *i;
          }
          MvrLog::log(MvrLog::Normal, "%s: Message provided checksum \"%s\" = 0x%x (%d). Calculated checksum is 0x%x (%d).  NMEA message contents were: \"%s\"", myName, checksumBuf, checksumRec, checksumRec, currentChecksum, currentChecksum, nmeaText.c_str());

          // abort the message and start looking for the next one.
          result |= ParseError;
          endMessage();
        }
      }
      continue;
    }


    // Got to the checksum?
    if (buf[i] == '*')
    {
      nextField();
      if (!ignoreChecksum)
        beginChecksum();
      continue;
    }

    // Every byte in a message between $ and * XORs to form the
    // checksum:
    currentChecksum ^= buf[i];

    // Time to start a new field?
    if (buf[i] == ',')
    {
      nextField();
      continue;
    }


    // Else, we must be in the middle of a field
    // TODO we could use strchr to look ahead in the buf 
    // for the end of the field (',' or '*') or end of the buf, and copy more
    // than one byte at a time.
    currentField += buf[i];
    if (currentField.size() > MaxFieldSize)
    {
      endMessage();
      continue;
    }
  }

  return result;
}



