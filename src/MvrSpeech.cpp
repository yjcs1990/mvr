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


