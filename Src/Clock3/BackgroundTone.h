// ***
// *** Copyright(C) 2020, Daniel M. Porrey. All rights reserved.
// ***
// *** This program is free software: you can redistribute it and/or modify
// *** it under the terms of the GNU Lesser General Public License as published
// *** by the Free Software Foundation, either version 3 of the License, or
// *** (at your option) any later version.
// ***
// *** This program is distributed in the hope that it will be useful,
// *** but WITHOUT ANY WARRANTY; without even the implied warranty of
// *** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *** GNU Lesser General Public License for more details.
// ***
// *** You should have received a copy of the GNU Lesser General Public License
// *** along with this program. If not, see http://www.gnu.org/licenses/.
// ***
#ifndef BACKGROUND_TONE_H
#define BACKGROUND_TONE_H

#include <Arduino.h>
#include "MusicNotes.h"

#define NO_SEQUENCE 0xffff
#define END_OF_SEQUENCE 0xffff
#define REPEAT_SEQUENCE 0

// ***
// *** This represents a sequence of pitch/duration pairs to play
// *** melodies or sounds.
// ***
const static uint16_t _sequences[] PROGMEM = {
      /* Buzz -> start : 0 */
      NOTE_A2, 500, NOTE_REST, 300, NOTE_REST, REPEAT_SEQUENCE,
      /* Classic -> start : 6 */
      NOTE_A5, 50, NOTE_REST, 50,
      NOTE_A5, 50, NOTE_REST, 50,
      NOTE_A5, 50, NOTE_REST, 750,
      NOTE_REST, REPEAT_SEQUENCE,
      /* Chime -> start : 20 */
      NOTE_F4, 800, NOTE_A4, 800, NOTE_G4, 800, NOTE_C4, 1600, NOTE_REST, 200,
      NOTE_F4, 800, NOTE_G4, 800, NOTE_A4, 800, NOTE_F4, 1600, NOTE_REST, 200,
      NOTE_A4, 800, NOTE_F4, 800, NOTE_G4, 800, NOTE_C4, 1600, NOTE_REST, 200,
      NOTE_C4, 800, NOTE_G4, 800, NOTE_A4, 800, NOTE_F4, 1600, NOTE_REST, 200,
      NOTE_REST, END_OF_SEQUENCE };

class BackgroundTone
{
  public:
    // ***
    // *** Specifies a particular sequence (song or chime). The value
    // *** is the offset of the first note in the _sequences[] array.
    // ***
    enum SEQUENCE { BUZZ = 0, CLASSIC = 6, CHIME = 20 };

    // ***
    // *** Species an event ID for the callback.
    // ***
    enum SEQUENCE_EVENT_ID { SEQUENCE_STARTED = 0, SEQUENCE_COMPLETED = 1 };

    // ***
    // *** Definition for the event callback handler.
    // ***
    using BackgroundToneEvent = void (*)(SEQUENCE_EVENT_ID);

    // ***
    // *** Default constructor.
    // ***
    BackgroundTone() {};

    // ***
    // *** Initializes the player.
    // ***
    void begin(uint16_t, BackgroundToneEvent);

    // ***
    // *** Called in the loop to keep things moving.
    // ***
    void tick();

    // ***
    // *** Starts playing the specified sequence.
    // ***
    void play(SEQUENCE);

    // ***
    // *** Returns true if a sequence is playing.
    // ***
    const bool isPlaying();

    // ***
    // *** Stops a playing sequence. Some sequences have a fixed playing length
    // *** and will stop automatically. Other sequences are set to repeat. These
    // *** can only be stopped by caling this method.
    // ***
    void stop();

  protected:
    // ***
    // *** This is the current sequence being played. This is
    // *** set to NO_SEQUENCE when nothing is playing.
    // ***
    uint16_t _currentSequence = NO_SEQUENCE;

    // ***
    // *** Current note being played in the current sequence.
    // ***
    uint16_t _currentNoteIndex = END_OF_SEQUENCE;

    // ***
    // *** Gets the next note in the sequence.
    // ***
    void getNextNote(uint16_t&, uint16_t&);

    // ***
    // *** This is the pin used to playing the tone.
    // ***
    uint8_t _pin;

    // ***
    // *** Determines whether or not a sequence is currently playing.
    // ***
    bool _isPlaying = false;

    // ***
    // *** The event callback handler.
    // ***
    BackgroundToneEvent _callback;
};
#endif
