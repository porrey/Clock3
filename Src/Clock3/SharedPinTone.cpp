#include "SharedPinTone.h"

void SharedPinTone::begin(uint16_t pin, uint16_t fallbackMode)
{
  this->_pin = pin;
  this->_fallbackMode = fallbackMode;
}

void SharedPinTone::setSharedPin()
{
  pinMode(this->_pin, OUTPUT);
}

void SharedPinTone::resetSharedPin()
{
  // ***
  // *** Restore the button.
  // ***
  digitalWrite(this->_pin, HIGH);
  pinMode(this->_pin, this->_fallbackMode);
}

void SharedPinTone::play(uint16_t frequency, uint64_t duration)
{
  this->setSharedPin();
  
  // ***
  // *** Play the tone.
  // ***
  tone(this->_pin, frequency, duration);
  delay(duration);
  noTone(this->_pin);

  this->resetSharedPin();
}

void SharedPinTone::startUpSound()
{
  this->setSharedPin();
  
  tone(this->_pin, NOTE_C4, 200);
  delay(200);
  tone(this->_pin, NOTE_D4, 200);
  delay(200);
  tone(this->_pin, NOTE_E4, 200);
  delay(200);
  tone(this->_pin, NOTE_F4, 200);
  delay(200);
  tone(this->_pin, NOTE_G4, 900);
  delay(900);
  noTone(this->_pin);

  this->resetSharedPin();
}

void SharedPinTone::shutDownSound()
{
  this->setSharedPin();
  
  tone(this->_pin, NOTE_G4, 200);
  delay(200);
  tone(this->_pin, NOTE_F4, 200);
  delay(200);
  tone(this->_pin, NOTE_E4, 200);
  delay(200);
  tone(this->_pin, NOTE_D4, 200);
  delay(200);
  tone(this->_pin, NOTE_C4, 900);
  delay(900);
  noTone(this->_pin);

  this->resetSharedPin();
}

void SharedPinTone::annoyingBuzz()
{
  this->setSharedPin();
  
  tone(this->_pin, NOTE_GS2, 300);
  delay(300);
  noTone(this->_pin);
  delay(300);

  this->resetSharedPin();
}

void SharedPinTone::classicAlert()
{
  this->setSharedPin();
  
  for (int i = 0; i < 4; i++)
  {
    tone(this->_pin, NOTE_A5, 50);
    delay(50);
    noTone(this->_pin);
    delay(50);
  }

  delay(750);

  this->resetSharedPin();
}
