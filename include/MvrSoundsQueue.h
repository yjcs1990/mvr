#ifndef MVRSOUNDSQUEUE_H
#define MVRSOUNDSQUEUE_H


#include "mvriaTypedefs.h"
#include "MvrASyncTask.h"
#include "MvrCondition.h"
#include "MvrFunctor.h"
#include "MvrSpeech.h"
#include <list>
#include <string>
#include <set>

/**
 * @brief This class manages a queue of items to play as WAV files or as text to 
 *  speak using a speech synthesizer.
 *
 * The actual playback of sound and speech is done through callbacks 
 * (which you can supply in the constructor or afterwards).
 * Some callbacks you can use are provided by classes like MvrSoundPlayer (for sound file
 * playback) and MvrFestival (from the MvrSpeechSynth_Festival library) and MvrCepstral 
 * (from the MvrSpeechSynth_Cepstral librrary) for speech synthesis. 
 *
 * Add sound files to the queue with play(), and text to speak with
 * speak(). Use runAsync() to run
 * the processing thread in the background, or run() to run synchronously in the
 * current thread.
 *
 *
 * @sa MvrSoundPlayer
 * @sa @ref soundsQueueExample.cpp
 *
  @ingroup UtilityClasses
 */ 

class MvrSoundsQueue: public MvrASyncTask
{
public:

  /** Opaque identifier for the type of an item in the sound queue. Not used 
   * during processing, but may be used to search for items in the queue. */
  enum ItemType { SPEECH, SOUND_FILE, SOUND_DATA, OTHER };

  /** Callback for playing a queue item. First argument is the "data", second is
   * item type-specific "parameters". Return true to continue processing more
   * callbacks in a list, false to cancel processing.
   */
  typedef MvrRetFunctor2<bool, const char*, const char*> PlayItemFunctor;
  typedef MvrFunctor InterruptItemFunctor;

  /** Callback types for determining whether to commence playing an item,
   * or skipping it.
   */
  typedef MvrRetFunctor<bool> PlaybackConditionFunctor;

  /** A sound item in the queue, with callbacks for dealing with the
   * item and the data to pass to those callbacks. 
   */
  class Item {
  public:
    std::string data;
    ItemType type;
    std::string params;
    int priority;
    std::list<InterruptItemFunctor*> interruptCallbacks;
    std::list<PlayItemFunctor*> playCallbacks;
    std::list<MvrFunctor*> doneCallbacks;
    std::list<PlaybackConditionFunctor*> playbackConditionCallbacks;

    MVREXPORT Item();
    MVREXPORT Item(std::string _data, ItemType _type, std::string _params = "", int priority = 0);
    MVREXPORT Item(std::string _data, ItemType _type, std::string _params, int priority, std::list<PlayItemFunctor*> callbacks); 

	  MVREXPORT Item(const MvrSoundsQueue::Item& toCopy);

    /** Note: does not compare priority! */
    bool operator==(const Item& other) const
    {
      return (other.type == type && other.params == params && other.data == data);
    }

    /** Called by sound queue to play this item by calling play callbacks. */
    void play();

    /** Called by sound queue to interrupt this item by calling interrupt
     * callbacks. */
    void interrupt();

    /** Called by sound queue thread after playing this item by calling done
     * callbacks. */
    void done();
  };


  MVREXPORT MvrSoundsQueue();

  /** @deprecated
   *  @see addInitCallback()
   *  @see setSpeakCallback()
   *  @see setInterruptSpeechCallback()
   *  @see setPlayFileCallback
   *  @see setInterruptFileCallback
   */
  MVREXPORT MvrSoundsQueue(MvrRetFunctor<bool> *speakInitCB, 
		    PlayItemFunctor *speakCB = 0, 
        InterruptItemFunctor *interruptSpeechCB = 0,
		    MvrRetFunctor<bool> *playInitCB = 0, 
		    PlayItemFunctor *playFileCB = 0,
        InterruptItemFunctor *interruptFileCB = 0);

  /** Set default speech and WAV file callbacks for use
   * by the convenience methods speak() and play().
   * Omit the last three arguments to use 
   * callbacks into MvrSoundPlayer.
   */
  MVREXPORT MvrSoundsQueue(MvrSpeechSynth* speechSynthesizer, 
		    MvrRetFunctor<bool> *playInitCB = 0, 
		    PlayItemFunctor *playFileCB = 0,
        InterruptItemFunctor *interruptFileCB = 0);

  MVREXPORT virtual ~MvrSoundsQueue();

  /** Add a callback to be called when the sound queue begins to run in its
   *  thread.  (For example, the speech synthesizers must be initialized in
   *  the same thread as they are used.)
   */
  MVREXPORT void addInitCallback(MvrRetFunctor<bool> *cb) {
    myInitCallbacks.push_back(cb);
  }

  /** @deprecated */
  MVREXPORT void setSpeakInitCallback(MvrRetFunctor<bool> *cb) {
    addInitCallback(cb);
  }

  /** Add (a copy of) the given item to the queue. */
  MVREXPORT void addItem(MvrSoundsQueue::Item item);

  /** Create a new queue item with the given data and add to the queue. */
  MVREXPORT void addItem(ItemType type, const char* data, std::list<PlayItemFunctor*> callbacks, int priority = 0, const char* params = 0);

  /** Return true if all initialization callbacks have completed, false
   * otherwise.  
   */
  MVREXPORT bool isInitialized()
  {
    return myInitialized;
  }

  /** @deprecated
   *  @see isPlaying()
   */
  MVREXPORT bool isSpeakingOrPlaying(void) { return (myPlayingSomething); }

  /// Returns true if an item is currently being played.
  MVREXPORT bool isPlaying() { return myPlayingSomething; }


  /** @deprecated
   *  @return true if any queue item is being played.
   *  @see isPlaying()
   */
  MVREXPORT bool isSpeaking() { return myPlayingSomething; }


  /// Begin processing the sounds queue synchronously (in this thread; does not return)
  MVREXPORT void run(void) { runInThisThread(); }

  /// Begin processing the sounds queue in a background thread
  MVREXPORT void runAsync(void) { create(false); }

  /** Temporarily stop processing the sounds queue. (Any currently playing sound
      or speech utterance will finish. The sound device may remain open.)
  */
  MVREXPORT void pause();

  /// Resume processing the sounds queue
  MVREXPORT void resume() ;

  /** @return true if the queue is paused */
  MVREXPORT bool isPaused();

  /// If sound is currently being played or speech is being spoken, interrupt it. (but continue processing the queue). SoundFinished callbacks will not be called.
  MVREXPORT void interrupt();

  /** Empty the queue.  If a sound is currently playing, it will not be interrupted.
      QueueEmpty callbacks will be called. SoundFinished callbacks will not be
      called.
  */
  MVREXPORT void clearQueue();

  /**  End the processing thread.
   * This function is deprecated. Use stopRunning() instead.
   * @deprecated
      This shuts down the sound queue completely. To 
      temporarily stop the queue processing, use pause(). To interrupt the
      currently playing sound, use interrupt().
  */
  MVREXPORT void stop() ;

  /// Create and return a new a functor for pause(), which other modules can use to pause this sounds
  /// queue.
  MVREXPORT MvrFunctor* getPauseCallback() 
  {
    return new MvrFunctorC<MvrSoundsQueue>(this, &MvrSoundsQueue::pause);
  }

  /// Create and return a new functor for resume(), which other modules can use to resume this
  ///  sounds queue.
  MVREXPORT MvrFunctor* getResumeCallback() 
  {
    return new MvrFunctorC<MvrSoundsQueue>(this, &MvrSoundsQueue::resume);
  }


  /// Get the current size of the speech/sound playback queue.
  MVREXPORT size_t getCurrentQueueSize()
  {
    size_t size;
    lock();
    size = myQueue.size();
    unlock();
    return size;
  }

  /** Add a callback functor to be invoked when playback of one sound or speech utterance starts. */
  MVREXPORT void addSoundStartedCallback(MvrFunctor* f)
  {
    myStartPlaybackCBList.push_back(f);
  }

  /** Remove a callback functor to be invoked when playback one sound or speech utterance starts. */
  MVREXPORT void remSoundStartedCallback(MvrFunctor* f)
  {
    myStartPlaybackCBList.remove(f);
  }

  /** Add a callback functor to be invoked when plackback of one sound or speech
   * utterance finishes */
  MVREXPORT void addSoundFinishedCallback(MvrFunctor* f)
  {
    myEndPlaybackCBList.push_back(f);
  }

  /** Remove a callback functor to be invoked when plackback of one sound or
   * speech utterance finishes. */
  MVREXPORT void remSoundFinishedCallback(MvrFunctor* f)
  {
    myEndPlaybackCBList.remove(f);
  }

  /** Add a callback functor to be invoked when playback of one sound or speech utterance starts. */
  MVREXPORT void addSoundItemStartedCallback(
	  MvrFunctor1<MvrSoundsQueue::Item> *f)
  {
    myStartItemPlaybackCBList.push_back(f);
  }

  /** Remove a callback functor to be invoked when playback one sound or speech utterance starts. */
  MVREXPORT void remSoundItemStartedCallback(
	  MvrFunctor1<MvrSoundsQueue::Item> *f)
  {
    myStartItemPlaybackCBList.remove(f);
  }

  /** Add a callback functor to be invoked when plackback of one sound or speech
   * utterance finishes */
  MVREXPORT void addSoundItemFinishedCallback(
	  MvrFunctor1<MvrSoundsQueue::Item> *f)
  {
    myEndItemPlaybackCBList.push_back(f);
  }

  /** Remove a callback functor to be invoked when plackback of one sound or
   * speech utterance finishes. */
  MVREXPORT void remSoundItemFinishedCallback(
	  MvrFunctor1<MvrSoundsQueue::Item> *f)
  {
    myEndItemPlaybackCBList.remove(f);
  }

  /** Add a callback functor to be invoked when a the sound queue becomes
   * non-empty, that is, when a block of sounds/speech utterances begins.
   */
  MVREXPORT void addQueueNonemptyCallback(MvrFunctor* f)
  {
    myQueueNonemptyCallbacks.push_back(f);
  }

  /** Remove a functor added by addQueueNonemptyCallback(). */
  MVREXPORT void remQueueNonemptyCallback(MvrFunctor* f)
  {
    myQueueNonemptyCallbacks.remove(f);
  }

  /** Add a callback functor to be invoked when the sound queue becomes empty
   * and the last sound has finished playing,
   * that is, when a block of sounds/speech utterances ends. This will not
   * be called when the sound queue first begins running.
   */
  MVREXPORT void addQueueEmptyCallback(MvrFunctor* f)
  {
    myQueueEmptyCallbacks.push_back(f);
  }

  /** Remove a functor added by addQueueEmptyCallback() */
  MVREXPORT void remQueueEmptyCallback(MvrFunctor* f)
  {
    myQueueEmptyCallbacks.remove(f);
  }


  
  /** Find items waiting in the queue. This is mainly useful in finding 
   *  speech text.
   * @param item  Item to search for.  
   * @return A set of positions in the queue. 1 indicates the next sound that will
   * play, followed by 2, etc.
   * @note You have a potential race condition if an item is removed from the queue 
   * after this method returns, but before you on the information returned.
   * For best results, pause the sound queue while using this information.
   */
  MVREXPORT std::set<int> findPendingItems(const char* item);

  /** Remove pending items with the given data and type. */
  MVREXPORT void removePendingItems(const char* item, ItemType type);

  /** Remove pending items with the given data. */
  MVREXPORT void removePendingItems(const char* data);

  /** Remove pending items with a priority less than that given. */
  MVREXPORT void removePendingItems(int priority);

  /** Remove pending items with priority less the given priority and with the given type. */
  MVREXPORT void removePendingItems(int priority, ItemType type);

  /** Remove pending items with the given type. */
  MVREXPORT void removePendingItems(ItemType type);

  /** Remove pending and current items with a priority less than that given. */
  MVREXPORT void removeItems(int priority);

  /** Removes pending and current items with the same data and type as the given item **/
  MVREXPORT void removeItems(Item item);

  MVREXPORT std::string nextItem(ItemType type);
  MVREXPORT std::string nextItem(int priority);
  MVREXPORT std::string nextItem(ItemType type, int priority);

  /// Convenience methods for special speech synthesis and WAV file queue items: 
  //@{
  
  /**
   * As a convenience, you may set a "default" speech synthesis callback,
   * and then simply use the speak() method to add a speech item to the queue 
   * with those callbacks. 
   * @sa Item
   */
  MVREXPORT void setSpeakCallback(PlayItemFunctor *cb) {
    myDefaultSpeakCB = cb;
  }

  /** Set the "default" callback to interrupt a current speech utterance, used
   * by speak() 
   * @sa Item
   */
  MVREXPORT void setInterruptSpeechCallback(InterruptItemFunctor *cb) {
    myDefaultInterruptSpeechCB = cb;
  }

  /** As a convenience, you may set a "default" WAV file playback callback,
   * and then simply use the play() method to add the file to the queue with
   * this callback.
   * @sa Item
   */
  MVREXPORT void setPlayFileCallback(PlayItemFunctor *cb) {
    myDefaultPlayFileCB = cb;
  }

  /** @deprecated use setPlayFileCallback() */
  MVREXPORT void setPlayWavFileCallback(PlayItemFunctor* cb) {
    setPlayFileCallback(cb);
  }

  /** Set the "default" callback to interrupt current wav file playback, for use
   * by the play() convenience method. 
   * @sa Item
   */
  MVREXPORT void setInterruptFileCallback(InterruptItemFunctor *cb) {
    myDefaultInterruptFileCB = cb;
  }

  /** @deprecated use setInterruptFileCallback() */
  MVREXPORT void setInterruptWavFileCallback(InterruptItemFunctor* cb) {
    setInterruptFileCallback(cb);
  }
  

  /** Add text string item to the queue for speech synthesis. The text will be
   * sent to the "speak" callback, if set, otherwise the SynthesiseToSound and
   * PlaySynthesizedSound callbacks are used, if set.
   */
  MVREXPORT void speak(const char *str);

  /** Add a sound file to the queue for default sound file playback.  */
  MVREXPORT void play(const char *filename);
  
#if !(defined(WIN32) && defined(_MANAGED)) && !defined(SWIG)

  /** Add a formatted text string (like printf) to the queue configured for default speech
   * synthesis. When reached in the queue while running, the text will be 
   * sent to the "speak" callback, if set, otherwise, the "SythesizeToSound" and
   * "PlaySynthesizedSound" callbacks will be used (if they are set.)
   *
   * @param fmt Format string. 
   * @param ... Mvrguments to format into the format string.
   */
  MVREXPORT void speakf(const char *fmt, ...);


  /** Speak with alternate voice. */
  MVREXPORT void speakWithVoice(const char* voice, const char* fmt, ...);

  /** Speak with alternate priority. */
  MVREXPORT void speakWithPriority(int priority, const char* fmt, ...);

  /** Add a sound file to the queue for default sound file playback.
   *
   * @param filename_fmt  Format string for determining the filename of the WAV
   * file, same as printf().
   * @param ... If given, arguments to format into the format string.
   */
  MVREXPORT void playf(const char *filename_fmt, ...);

#endif // MS Managed C++ or SWIG

  /** Return an item set up for speech with previously set default speech
   * callbacks.
   * @param speech Text to speak (optional)
   * @see setSpeechCallback()
   * @see addItem()
   */
  MVREXPORT MvrSoundsQueue::Item createDefaultSpeechItem(const char* speech = 0);

  /** Return an item set up for sound file playback with previously set default
   * play callbacks.
   * @param filename Filename to set (optional)
   * @see setPlayFileCallback()
   * @see addItem()
   */
  MVREXPORT MvrSoundsQueue::Item createDefaultFileItem(const char* filename = 0);

  //@}

  /** Set a playback condition functor used for default speech and sound file
   * items. Set to NULL to clear. */
  MVREXPORT void setDefaultPlayConditionCB(PlaybackConditionFunctor* f) {
    myDefaultPlayConditionCB = f;
  }
  
  /// main function for thread
  /** @internal */
  MVREXPORT virtual void *runThread(void *arg);

protected:
  bool myInitialized;
  std::list<Item> myQueue;
  MvrMutex myQueueMutex;
  void lock() {
    myQueueMutex.lock();
  }
  void unlock() {
    myQueueMutex.unlock();
  }
  bool tryLock() {
    return myQueueMutex.tryLock();
  }

  /// Functors to invoke when we start running in our thread
  std::list< MvrRetFunctor<bool>* > myInitCallbacks;

  bool myPlayingSomething;
  Item myLastItem; ///< The current or most recent item played.

  /// Used by speak() and play() convenience methods:
  //@{
  PlayItemFunctor *myDefaultSpeakCB;
  InterruptItemFunctor *myDefaultInterruptSpeechCB;
  PlayItemFunctor *myDefaultPlayFileCB;
  InterruptItemFunctor *myDefaultInterruptFileCB;
  //@}
 
  int myPauseRequestCount;  ///< Semaphore for doing pausing and resuming of the queue
  MvrCondition myPausedCondition;    ///< Condition variable used by the thread to pause

  /// Functors to invoke during queue processing
  //@{
  std::list<MvrFunctor*> myStartPlaybackCBList;
  std::list<MvrFunctor*> myEndPlaybackCBList;
  std::list<MvrFunctor1<MvrSoundsQueue::Item> *> myStartItemPlaybackCBList;
  std::list<MvrFunctor1<MvrSoundsQueue::Item> *> myEndItemPlaybackCBList;
  std::list<MvrFunctor*> myQueueNonemptyCallbacks;
  std::list<MvrFunctor*> myQueueEmptyCallbacks;
  //@}
  
  PlaybackConditionFunctor* myDefaultPlayConditionCB; ///< Used when creating default configured speech and sound file items

  /// Invoke each functor in a list
  void invokeCallbacks(const std::list<MvrFunctor*>& lst);

  /// Invoke each functor in a list, but stop if any of them return false.
  void invokeCallbacks(const std::list<MvrRetFunctor<bool>*>& lst);

  /**  push item onto queue 
   * @sa addItem()
   */
  //@{
  void pushQueueItem(Item item);
  void pushQueueItem_NoLock(Item item);
  //@}

  /** Pop item from queue and return it */
  //@{
  Item popQueueItem();
  Item popQueueItem_NoLock();
  //@}

};

#endif  // MVRSOUNDSQUEUE_H

