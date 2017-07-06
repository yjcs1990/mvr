/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/


#include "MvrExport.h"
#include "MvrSpeech.h"
#include "MvrConfig.h"
#include "mvriaInternal.h"


MVREXPORT MvrSpeechSynth::MvrSpeechSynth() : 
  mySpeakCB(this, &MvrSpeechSynth::speak), 
  myInitCB(this, &MvrSpeechSynth::init), 
  myInterruptCB(this, &MvrSpeechSynth::interrupt),
  myAudioPlaybackCB(0),
  myProcessConfigCB(this, &MvrSpeechSynth::processConfig)
{
  myProcessConfigCB.setName("MvrSpeechSynth");
}


MVREXPORT bool MvrSpeechSynth::init()
{
  return true;
}

MVREXPORT void MvrSpeechSynth::addToConfig(MvrConfig *config)
{
  addVoiceConfigParam(config);
  config->addProcessFileCB(&myProcessConfigCB, 100);
}

MVREXPORT MvrSpeechSynth::~MvrSpeechSynth()
{
}

MVREXPORT MvrRetFunctorC<bool, MvrSpeechSynth>* MvrSpeechSynth::getInitCallback(void) 
{
  return &myInitCB;
}

MVREXPORT MvrRetFunctor2C<bool, MvrSpeechSynth, const char*, const char*>* MvrSpeechSynth::getSpeakCallback(void) 
{
  return &mySpeakCB;
}


MVREXPORT MvrFunctorC<MvrSpeechSynth>*  MvrSpeechSynth::getInterruptCallback() 
{
  return &myInterruptCB;
}

MVREXPORT void MvrSpeechSynth::setAudioCallback(MvrRetFunctor2<bool, MvrTypes::Byte2*, int>* cb)
{
  myAudioPlaybackCB = cb;
}


MVREXPORT bool MvrSpeechSynth::speak(const char* text, const char* voiceParams) {
  return speak(text, voiceParams, NULL, 0);
}
  


bool MvrSpeechSynth::processConfig()
{
  setVoice(myConfigVoice);
  return true;
}

void MvrSpeechSynth::addVoiceConfigParam(MvrConfig *config)
{
  const char *current = getCurrentVoiceName();
  if(current)
  {
    strncpy(myConfigVoice, current, sizeof(myConfigVoice));
  }
  else
  {
    myConfigVoice[0] = 0;
  }
  std::string displayHint;
  std::list<std::string> voices = getVoiceNames();
  for(std::list<std::string>::const_iterator i = voices.begin(); i != voices.end(); i++)
  {
    if(i == voices.begin())
      displayHint = "Choices:";
    else
      displayHint += ";;";
    displayHint += *i;
  }
  config->addParam(
    MvrConfigArg("Voice", myConfigVoice, "Name of voice to use for speech synthesis", sizeof(myConfigVoice)), 
    "Speech Synthesis",
    MvrPriority::NORMAL,
    displayHint.c_str()
  );
}


