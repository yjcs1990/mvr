#ifndef MVRSOUNDPLAYER_H_
#define MVRSOUNDPLAYER_H_


#include "MvrFunctor.h"



/** 
 * @brief This class provides a cross-platform interface for playing short sound samples.
 * (Currently implemented for Windows and Linux).
 * @sa For I/O and network transfer of encoded audio, see the MvrNetAudio library.
 * @sa MvrSoundsQueue
 *
 * @note Uses an external program to play WAV files on Linux. If an environment
 * vmvriable named PLAY_WAV is set, that program is used, otherwise, 'play' from
 * the 'sox' toolset is used.  PLAY_WAV must contain one word (the command; no arguments)
 * A call to playWavFile() will return immediately after
 * 'play' has finished, even though Linux may still be playing back the sound data. In general,
 * this kind of thing is a problem, especially with speech recognition immediately after playing
 * a sound. Ideally, we should be able to truly block until the sound has finished playback.
 * Alas, it is not an ideal world. Another potential pitfall due to the use of
 * an external program invocation: the program you call must not attempt to
 * issue any output.  'play' from the 'sox' toolset automatically supresses
 * normal output if it isn't called from an interactive terminal, but it may
 * still issue some error messages, which will cause it to hang indefinately.
 *
 * The volume (level) of audio output from a robot is determined by two things:
 * the computer sound device mixer, and also the amplifier which drives the
 * speakers.  The computer's mixer can be adjusted through the operating system:
 * on Linux, you can use the 'aumix' program to adjust the Master and PCM
 * levels.  On Windows, use the Windows mixer program.  If on Linux, MvrSoundPlayer also
 * prodives the setVolume() method, which adjusts the volume of the sound before
 * it is played.
 *  
    @ingroup UtilityClasses
 */
class MvrSoundPlayer
{
 public:
  /** Play a WAV (Windows RIFF) file 
    * @note Uses an external program to play WAV files on Linux. If an environment
    * vmvriable named PLAY_WAV is set, that program is used, otherwise, 'play' from
    * the 'sox' toolset is used. See detailed note in the overview for this
    * cass.
    * @param filename Name of the file to play
    * @param params ignored
    */
  MVREXPORT static bool playWavFile(const char* filename, const char* params);

  MVREXPORT static bool playWavFile(const char* filename) { return playWavFile(filename, NULL); }

  /** Play a file in some native file format for the compilation platform. */
  MVREXPORT static bool playNativeFile(const char* filename, const char* params);

  /** Cancel (interrupt) any current sound or file playback. */
  MVREXPORT static void stopPlaying();

  /** Return the static functor for playWavFile */
  MVREXPORT static MvrRetFunctor2<bool, const char*, const char*> *getPlayWavFileCallback();

  /** Return the static functor for stopPlaying(). */
  MVREXPORT static MvrFunctor* getStopPlayingCallback();

  /** Play raw uncompressed PCM16 sound data. The format of this data is 
   *  numSamples samples of two bytes each. Each byte pair is a signed little endian
   *  integer.
   *  The sound will be played back at 16kHz, monaurally.
   *  @return false on error, true on success.
   */
  MVREXPORT static bool playSoundPCM16(char* data, int numSamples);

  /** Set a volume adjustment applied to all sounds right before playing.
     (So this adjusts the volume in addition to, not instead of, the
      computer audio mixer). 
      Any value less than or equal to 0 is no volume i.e. muted or no output.
      @linuxonly
  */
  MVREXPORT static void setVolume(double v);

  /**
      Set volume as a "percent" of normal, where 100% is normal or natural
      volume, 50% is increased by 50%, -50% is decreased by 50%, etc. (-100.0% is 
      no volume, or mute.)
      @linuxonly
  */
  MVREXPORT static void setVolumePercent(double pct);
 
protected:
  static int ourPlayChildPID; ///< Only used on Linux.
  static MvrGlobalRetFunctor2<bool, const char*, const char*> ourPlayWavFileCB;
  static MvrGlobalFunctor ourStopPlayingCB;
  static double ourVolume;
};
    
  
#endif // MVRSOUNDPLAYER_H_
