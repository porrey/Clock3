#include "BackgroundTone.h"

void BackgroundTone::begin(uint16_t pin, BackgroundToneEvent callback)
{
  this->_pin = pin;
  this->_callback = callback;
}

void BackgroundTone::play(SEQUENCE sequence)
{
  switch (sequence)
  {
    case STARTUP:
      this->_currentSequence = 0;
      break;
    case SHUTDOWN:
      this->_currentSequence = 1;
      break;
    case BUZZ:
      this->_currentSequence = 2;
      break;
    case CLASSIC:
      this->_currentSequence = 3;
      break;
  }

  this->_currentSequence = sequence;
  this->_currentNoteIndex = 0;
}

bool BackgroundTone::isPlaying()
{
  return (this->_currentSequence != NO_SEQUENCE);
}

void BackgroundTone::stop()
{
  this->_currentSequence = NO_SEQUENCE;
}

void BackgroundTone::tick()
{
  // ***
  // *** If the current track is END_OF_SEQUENCE, then nothing is
  // *** currently being played.
  // ***
  if (this->_currentSequence != NO_SEQUENCE)
  {
    if (!this->_sequenceStarted)
    {
      this->_sequenceStarted = true;
      this->_callback(SEQUENCE_STARTED);
    }

    // ***
    // *** Get the current not being played.
    // ***
    NOTE* note = this->getNote();

    // ***
    // *** OCR2A is the count down register for the timer used to generate the tone. When
    // *** this value is zero, the curren tone has completed. If the current note is a rest,
    // *** the timer is not used.
    // ***
    if (OCR2A == 0)
    {
      if (note->duration != END_OF_SEQUENCE)
      {
        if (note->pitch != NOTE_REST)
        {
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

      // ***
      // *** Increment the note index.
      // ***
      this->_currentNoteIndex++;
    }
  }
  else
  {
    if (this->_sequenceStarted)
    {
      this->_sequenceStarted = false;
      this->_callback(SEQUENCE_COMPLETED);
    }
  }
}

const NOTE* BackgroundTone::getNote()
{
  NOTE* returnValue;

  // ***
  // *** Get the nxt note in the sequence.
  // ***
  returnValue = &this->_sequences[this->_currentSequence][this->_currentNoteIndex];

  // ***
  // *** If the duration of the note is 0 then it repeats. If the value
  // *** is -1 then it has ended.
  // ***
  if (returnValue->duration == REPEAT_SEQUENCE)
  {
    // ***
    // *** This is a repeating sequence. Get the first
    // *** note of the sequence.
    // ***
    this->_currentNoteIndex = 0;
    returnValue = &this->_sequences[this->_currentSequence][this->_currentNoteIndex];
  }
  else if (returnValue->duration == END_OF_SEQUENCE)
  {
    this->_currentSequence = NO_SEQUENCE;
  }

  return returnValue;
}
