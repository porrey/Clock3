#ifndef BACKGROUND_TONE_H
#define BACKGROUND_TONE_H

#include <Arduino.h>
#include "MusicNotes.h"

#define NO_SEQUENCE -1
#define END_OF_SEQUENCE 999999
#define REPEAT_SEQUENCE 0

// ***
// *** Represents a single note within
// *** a sequence.
// ***
struct NOTE
{
  uint16_t pitch;
  uint32_t duration;
};

class BackgroundTone
{
  public:
    enum SEQUENCE
    {
      STARTUP = 0, SHUTDOWN = 1, BUZZ = 2, CLASSIC = 3
    };

    enum SEQUENCE_EVENT_ID
    {
      SEQUENCE_STARTED = 0, SEQUENCE_COMPLETED = 1
    };

    using BackgroundToneEvent = void (*)(SEQUENCE_EVENT_ID);

    // ***
    // *** Initializes the player.
    // ***
    void begin(uint16_t, BackgroundToneEvent);

    // ***
    // *** Called in the loop to keep things moving.
    // ***
    void tick();

    // ***
    // *** STarts playing the specified sequence.
    // ***
    void play(SEQUENCE);

    // ***
    // *** Returns true if a sequence is playing.
    // ***
    bool isPlaying();

    // ***
    // *** Stops a playing sequence. Some sequences have a fixed playing length
    // *** and will stop automatically. Other sequences are set to reapeat. These
    // *** can only be stopped by caling this method.
    // ***
    void stop();

  protected:
    const NOTE _startup[7] = { {NOTE_C4, 200}, {NOTE_D4, 200}, {NOTE_E4, 200}, {NOTE_F4, 200}, {NOTE_G4, 900}, {NOTE_REST, 250}, {NOTE_REST, END_OF_SEQUENCE} };
    const NOTE _shutDown[7] = { {NOTE_G4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_D4, 200}, {NOTE_C4, 900}, {NOTE_REST, 250}, {NOTE_REST, END_OF_SEQUENCE} };
    const NOTE _annoyingBuzz[3] = { {NOTE_A2, 500}, {NOTE_REST, 300}, {NOTE_REST, REPEAT_SEQUENCE} };
    const NOTE _classic[7] = { {NOTE_A5, 50}, {NOTE_REST, 50}, {NOTE_A5, 50}, {NOTE_REST, 50}, {NOTE_A5, 50}, {NOTE_REST, 750}, {NOTE_REST, REPEAT_SEQUENCE} };

    // ***
    // *** A list of all sequences.
    // ***
    const NOTE* _sequences[4] { _startup, _shutDown, _annoyingBuzz, _classic };

    // ***
    // *** This is the current sequence being played. This is
    // *** set to NO_SEQUENCE when nothing is playing.
    // ***
    int16_t _currentSequence = NO_SEQUENCE;

    // ***
    // *** Current note being played in the current sequence.
    // ***
    int16_t _currentNoteIndex = END_OF_SEQUENCE;

    // ***
    // *** Gets the next note in the sequence.
    // ***
    const NOTE* getNote();

    // ***
    // *** This is the pin used to playing the tone.
    // ***
    uint8_t _pin;

    // ***
    // *** Marks the start of the sequence for the events.
    // ***
    bool _sequenceStarted = false;

    // ***
    // *** The event callback handler.
    // ***
    BackgroundToneEvent _callback;
};
#endif
