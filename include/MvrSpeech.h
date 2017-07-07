#ifndef MVRSPEECH_H
#define MVRSPEECH_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "MvrConfig.h"
#include <list>
#include <string>


/** @brief Abstract interface to speech synthesis.  
 *
 *  This class defines the abstract interface for speech synthesizers 
 *  used with Mvr.
 *  Implementations are provided in the separate MvrSpeechSynth_Cepstral and
 *  MvrSpeechSynth_Festival libraries.
 *  This class provides a common-denominator
 *  interface. Implementations (especially MvrCepstral) may support more
 *  features, or behave differently; refer to their documentation for more
 *  information.
 *
 *  This class registers one parameter with the global MvrConfig object in the
 *  "Speech Synthesis" section:  The parameter is named "voice" and sets the voice
 *  used for speech synthesis, if multiple voices is supported and the 
 *  new voice can be loaded. 
 */

class MvrSpeechSynth
{
public:

  /** Don't forget to call this from derived classes. */
  MVREXPORT MvrSpeechSynth();  

  MVREXPORT virtual ~MvrSpeechSynth();


  /** Perform  synthesizer initialization, if necessary. You must call
   * this method. 
   *
   * (Subclass implementations should call this method *after* initializing their 
   * speech engine.)
   */
  MVREXPORT virtual bool init(); 

  /** Use the given MvrConfig object to read parameters such as voice, speaking
   * rate, volume.
   */
  MVREXPORT virtual void addToConfig(MvrConfig *config);

  /** Speaks the given text. 
   * @param str The text to speak.
   * @param voiceParams Voice selection criteria expression
   *    (implementation-specific)
   * @param audioOutputCB If not NULL, send synthesized audio data to this
   *    callback (may be called several times). Otherwise, play using default
   *    AudioCallback, or directly out the speakers
   * @param sampleRate if given, temporarily use this sample rate for this
   *    speech, then restore. If 0, use current sample rate.
   */
  MVREXPORT virtual bool speak(const char *str, const char* voiceParams, MvrRetFunctor2<bool, MvrTypes::Byte2*, int>* audioOutputCB, unsigned short sampleRate = 0) = 0;

  /** Speaks the given text. 
   * @param str The text to speak.
   * @param voiceParams Voice selection criteria expression
   *    (implementation-specific)
   */
  MVREXPORT virtual bool speak(const char *str, const char* voiceParams = NULL);

  /** Speaks the given string, using current voice and output settings,
   *  taking varargs and a format string (like printf)
   */
  MVREXPORT virtual  bool speakf(const char* fmt, ...) = 0; 

  /** If any speech is currently ongoing, interrupt it. 
   */
  MVREXPORT virtual void interrupt() = 0; 

  /** @return a functor for init() to use with MvrSoundsQueue */
  MVREXPORT MvrRetFunctorC<bool, MvrSpeechSynth>* getInitCallback();

  /** @return a functor for speak() to use with MvrSoundsQueue */
  MVREXPORT MvrRetFunctor2C<bool, MvrSpeechSynth, const char*, const char*>* getSpeakCallback(void) ;


  /** @return a functor for interrupt() */
  MVREXPORT MvrFunctorC<MvrSpeechSynth>* getInterruptCallback();


  /** Instead of playing synthesized audio using the synthesizer's internal
   *  audio playback, call the given callback when a chunk of audio has
   *  been synthesized. Audio is passed to the callback in the first parameter
   *  as signed 16-bit samples (PCM16).  The sample rate is 16kHz but may be
   *  changed with setAudioSampleRate(). The second parameter is the number
   *  of samples.  The return value from the callback is ignored.
   */
  MVREXPORT void setAudioCallback(MvrRetFunctor2<bool, MvrTypes::Byte2*, int>* cb);


  /** Change audio sample rate (Hz). Normal rate is 16000 Hz.
   *  Suggested values are 8000, 16000, or 44400
   */
  MVREXPORT virtual void setAudioSampleRate(int rate) = 0;

  MVREXPORT virtual int getAudioSampleRate() = 0;

  /** Lock, if neccesary */
  MVREXPORT virtual void lock() { }
  /** Unlock, if neccesary */
  MVREXPORT virtual void unlock() { }

  /** Set the current voice by name.
   *  Replaces fully any previous required voice criteria.
   * @sa getVoiceNames
   */
  MVREXPORT virtual bool setVoice(const char* name) = 0;

  /** Get name of current voice, if set with setVoice (else returns NULL) */
  MVREXPORT virtual const char* getCurrentVoiceName() = 0;

  /** Return a list of available voice names, if possible. */
  MVREXPORT virtual std::list<std::string> getVoiceNames() = 0;

protected:
  MvrRetFunctor2C<bool, MvrSpeechSynth, const char*, const char*> mySpeakCB;
  MvrRetFunctorC<bool, MvrSpeechSynth> myInitCB;
  MvrFunctorC<MvrSpeechSynth> myInterruptCB;
  MvrRetFunctor2<bool, MvrTypes::Byte2*, int> *myAudioPlaybackCB; ///< If set, send audio to this callback instead of playing it directly
private:
  MvrRetFunctorC<bool, MvrSpeechSynth> myProcessConfigCB;
  char myConfigVoice[32];
  bool processConfig();
  void addVoiceConfigParam(MvrConfig *config);

};



#endif
