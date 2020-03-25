#ifndef SHARED_PIN_TONE_H
#define SHARED_PIN_TONE_H

#include <Arduino.h>
#include "MusicNotes.h"

class SharedPinTone
{
  public:
    void begin(uint16_t, uint16_t);
    void play(uint16_t, uint64_t);
    void startUpSound();
    void shutDownSound();
    void annoyingBuzz();
    void classicAlert();

  protected:
    uint16_t _pin;
    uint16_t _fallbackMode;

    void setSharedPin();
    void resetSharedPin();
};
#endif
