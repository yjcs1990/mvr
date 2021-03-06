#include "MvrExport.h"
#include "MvrSoundPlayer.h"
#include "MvrLog.h"
#include "mvriaUtil.h"
#include <string.h>
#include <errno.h>

int MvrSoundPlayer::ourPlayChildPID = -1;
MvrGlobalRetFunctor2<bool, const char*, const char*> MvrSoundPlayer::ourPlayWavFileCB(&MvrSoundPlayer::playWavFile);
MvrGlobalFunctor MvrSoundPlayer::ourStopPlayingCB(&MvrSoundPlayer::stopPlaying);
double MvrSoundPlayer::ourVolume = 1.0;

MVREXPORT MvrRetFunctor2<bool, const char*, const char*>* MvrSoundPlayer::getPlayWavFileCallback() 
{
  return &ourPlayWavFileCB;
}


MVREXPORT MvrFunctor* MvrSoundPlayer::getStopPlayingCallback()
{
  return &ourStopPlayingCB;
}


#ifdef WIN32

      /* Windows: */

#include <assert.h>

MVREXPORT bool MvrSoundPlayer::playWavFile(const char* filename, const char* params) 
{
  return (PlaySound(filename, NULL, SND_FILENAME) == TRUE);
}

MVREXPORT bool MvrSoundPlayer::playNativeFile(const char* filename, const char* params)
{
  /* WAV is the Windows native format */
  return playWavFile(filename, 0);
}

MVREXPORT void MvrSoundPlayer::stopPlaying()
{
  PlaySound(NULL, NULL, NULL);
}


MVREXPORT bool MvrSoundPlayer::playSoundPCM16(char* data, int numSamples)
{
  MvrLog::log(MvrLog::Terse, "INTERNAL ERROR: MvrSoundPlayer::playSoundPCM16() is not implemented for Windows yet! Bug reed@activmedia.com about it!");
  assert(false);

  return false;
}

#else

      /* Linux: */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/soundcard.h>
//#include <linux/soundcard.h>
#include <unistd.h>
#include <errno.h>



bool MvrSoundPlayer::playNativeFile(const char* filename, const char* params)
{
  int snd_fd = MvrUtil::open("/dev/dsp", O_WRONLY); // | O_NONBLOCK);
  if(snd_fd < 0) {
    return false;
  }
  int file_fd = MvrUtil::open(filename, O_RDONLY);
  if(file_fd < 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSoundPlayer::playNativeFile: open failed");
    return false;
  }
  int len;
  const int buflen = 512;
  char buf[buflen];
  while((len = read(file_fd, buf, buflen)) > 0)
  {
    if (write(snd_fd, buf, len) != len) {
      MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSoundPlayer::playNativeFile: write failed");
    }
  }
  close(file_fd);
  close(snd_fd);
  return true;
}

bool MvrSoundPlayer::playWavFile(const char* filename, const char* params)
{
  MvrArgumentBuilder builder;
  //builder.addPlain("sleep .35; play");
  builder.addPlain("play");
  builder.add("-v %.2f", ourVolume);
  builder.addPlain(filename);
  builder.addPlain(params);
  MvrLog::log(MvrLog::Normal, "MvrSoundPlayer: Playing file \"%s\" with \"%s\"", 
	     filename, builder.getFullString());
  
  int ret;
  if ((ret = system(builder.getFullString())) != -1)
  {
    MvrLog::log(MvrLog::Normal, "MvrSoundPlayer: Played file \"%s\" with \"%s\" (got %d)", 
	       filename, builder.getFullString(), ret);
    return true;
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSoundPlayer::playWaveFile: system call failed");
    return false;
  }


  /*
    This was an old mechanism for doing a fork then exec in order to
    mimic a system call, so that it could have the child PID for
    killing... however in our embedded linux killing the play command
    doesn't also kill the sox command, so the kill doesn't work at
    all... and the fork() would also reserve a lot of memory for the new process
    ...
   so it was replaced with the above system call

  const char* prog = NULL;
  prog = getenv("PLAY_WAV");
  if(prog == NULL)
    prog = "play";
  char volstr[4];
  if(strcmp(prog, "play") == 0)
  {
    snprintf(volstr, 4, "%f", ourVolume);
  }  

  MvrLog::log(MvrLog::Normal, "MvrSoundPlayer: Playing file \"%s\" using playback program \"%s\" with argument: -v %s", filename, prog, volstr);
  ourPlayChildPID = fork();

  //ourPlayChildPID = vfork(); // XXX rh experimental, avoids the memory copy cost of fork()
  // NOTE: after vfork() you can ONLY safely use an exec function or _exit() in the
  // child process. Any other call, if it modifies memory still shared by the
  // parent, will result in problems.
  if(ourPlayChildPID == -1) 
  {
    MvrLog::log(MvrLog::Terse, "MvrSoundPlayer: error forking! (%d: %s)", errno, 
      (errno == EAGAIN) ? "EAGAIN reached process limit, or insufficient memory to copy page tables" : 
        ( (errno == ENOMEM) ? "ENOMEM out of kernel memory" : "unknown error" ) );

    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSoundPlayer::playWaveFile: fork failed");
    return false;
  }
  if(ourPlayChildPID == 0)
  {
    // child process: execute sox
    int r = -1;
    r = execlp(prog, prog, "-v", volstr, filename, (char*)0);
    if(r < 0)
    {
      int err = errno;
      const char *errstr = strerror(err);
      printf("MvrSoundPlayer (child process): Error executing Wav file playback program \"%s %s\" (%d: %s)\n", prog, filename, err, errstr);
      //_exit(-1);   // need to use _exit with vfork
      exit(-1);
    }
  } 
  // parent process: wait for child to finish
  MvrLog::log(MvrLog::Verbose, "MvrSoundPlayer: created child process %d to play wav file \"%s\".", 
      ourPlayChildPID, filename);
  int status;
  waitpid(ourPlayChildPID, &status, 0);
  if(WEXITSTATUS(status) != 0) {
    MvrLog::log(MvrLog::Terse, "MvrSoundPlayer: Error: Wav file playback program \"%s\" with file \"%s\" exited with error code %d.", prog, filename, WEXITSTATUS(status));
    ourPlayChildPID = -1;
    return false;
  }
  MvrLog::log(MvrLog::Verbose, "MvrSoundPlayer: child process %d finished.", ourPlayChildPID);
  ourPlayChildPID = -1;
  return true;
  */
}



void MvrSoundPlayer::stopPlaying()
{

  MvrLog::log(MvrLog::Normal, 
	     "MvrSoundPlayer::stopPlaying: killing play and sox");

  // so if the system call below is "killall -9 play; killall -9 sox"
  // then on linux 2.4 kernels a sound played immediately afterwards
  // will simply not play and return (cause that's what play does if
  // it can't play)
  int ret;

  if ((ret = system("killall play; killall -9 sox")) != -1)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrSoundPlayer::stopPlaying: killed play and sox (got %d)", 
	       ret);
    return;
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, 
			  "MvrSoundPlayer::stopPlaying: system call failed");
    return;
  }


  /* This was for old mechanism in playWavFile but since it doesn't
   * work, it was replaced with the above

  // Kill a child processes (created by playWavFile) if it exists.
  if(ourPlayChildPID > 0)
  {
    MvrLog::log(MvrLog::Verbose, "MvrSoundPlayer: Sending SIGTERM to child process %d.", ourPlayChildPID);
    kill(ourPlayChildPID, SIGTERM);
  }
  */
}


bool MvrSoundPlayer::playSoundPCM16(char* data, int numSamples)
{
  //MvrLog::log(MvrLog::Normal, "MvrSoundPlayer::playSoundPCM16[linux]: opening sound device.");
  int fd = MvrUtil::open("/dev/dsp", O_WRONLY); // | O_NONBLOCK);
  if(fd < 0)
    return false;
  int arg = AFMT_S16_LE;
  if(ioctl(fd, SNDCTL_DSP_SETFMT, &arg) != 0)
  {
    close(fd);
    return false;
  }
  arg = 0;
  if(ioctl(fd, SNDCTL_DSP_STEREO, &arg) != 0)
  {
    close(fd);
    return false;
  }
  arg = 16000;
  if(ioctl(fd, SNDCTL_DSP_SPEED, &arg) != 0)
  {
    close(fd);
    return false;
  }
  //MvrLog::log(MvrLog::Normal, "MvrSoundPlayer::playSoundPCM16[linux]: writing %d bytes to sound device.", 2*numSamples);
  int r;
  if((r = write(fd, data, 2*numSamples) < 0))
  {
    close(fd);
    return false;
  }
  close(fd);
  MvrLog::log(MvrLog::Verbose, "MvrSoundPlayer::playSoundPCM16[linux]: finished playing sound. (wrote %d bytes of 16-bit monaural signed sound data to /dev/dsp)", r);
  return true;
}



#endif  // ifdef WIN32

MVREXPORT void MvrSoundPlayer::setVolumePercent(double pct)
{
  setVolume(1.0 + (pct / 100.0));
}

MVREXPORT void MvrSoundPlayer::setVolume(double v)
{
  if(v < 0) ourVolume = 0;
  else ourVolume = v;
}

