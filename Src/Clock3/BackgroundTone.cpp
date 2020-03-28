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
#include "BackgroundTone.h"

void BackgroundTone::begin(uint16_t pin, BackgroundToneEvent callback)
{
  this->_pin = pin;
  this->_callback = callback;
}

void BackgroundTone::play(SEQUENCE sequence)
{
  this->_currentSequence = sequence;
  this->_currentNoteIndex = sequence;
  this->_isPlaying = true;
  this->_callback(SEQUENCE_STARTED);
}

const bool BackgroundTone::isPlaying()
{
  return this->_isPlaying;
}

void BackgroundTone::stop()
{
  this->_currentSequence = NO_SEQUENCE;
  this->_isPlaying = false;
  this->_callback(SEQUENCE_COMPLETED);
}

void BackgroundTone::tick()
{
  // ***
  // *** If the current track is END_OF_SEQUENCE, then no seuence
  // *** is currently being played.
  // ***
  if (this->_currentSequence != NO_SEQUENCE)
  {
    // ***
    // *** OCR2A is the count down register for the timer used to generate the tone. When
    // *** this value is zero, the current tone has completed.
    // ***
    if (OCR2A == 0)
    {
      // ***
      // *** Get the current note being played.
      // ***
      NOTE* note = this->getNextNote();

      if (note->duration != END_OF_SEQUENCE)
      {
        if (note->pitch != NOTE_REST)
        {
          // ***
          // *** Play the current note.
          // ***
          tone(this->_pin, note->pitch, note->duration);
        }
        else
        {
          // ***
          // *** Start a tone on a non-existent port to fire
          // *** the timer.
          // ***
          // *** TO DO: Replace this with a countdown on Timer2
          // ***
          tone(15, 100, note->duration);
        }
      }
    }
  }
}

const NOTE* BackgroundTone::getNextNote()
{
  NOTE* returnValue;

  // ***
  // *** Get the nxt note in the sequence.
  // ***
  returnValue = &this->_sequences[this->_currentNoteIndex];

  // ***
  // *** If the duration of the note is 0 then it repeats. If the value
  // *** is -1 then it has ended.
  // ***
  if (returnValue->duration == REPEAT_SEQUENCE)
  {
    // ***
    // *** This is a repeating sequence. Set the
    // *** next note to the start of the sequence.
    // ***
    this->_currentNoteIndex = this->_currentSequence;
    returnValue = &this->_sequences[this->_currentNoteIndex];
  }
  else if (returnValue->duration == END_OF_SEQUENCE)
  {
    this->_currentSequence = NO_SEQUENCE;
    this->_currentNoteIndex = END_OF_SEQUENCE;
    this->_isPlaying = false;
    this->_callback(SEQUENCE_COMPLETED);
  }
  else
  {
    // ***
    // *** Increment the note index.
    // ***
    this->_currentNoteIndex++;
  }

  return returnValue;
}
