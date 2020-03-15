#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>
#include "ClockMatrix.h"

void testTimeDisplay(ClockLedMatrix *clock, uint16_t delayTime)
{
  // ***
  // *** Loop through hours 1 to 24;
  // ***
  for (uint8_t hour = 1; hour <= 24; hour++)
  {
    bool pm = hour >= 12;
    uint8_t h = hour > 12 ? hour - 12 : hour;

    // ***
    // *** Lop through minutes 0 to 59.
    // **
    for (uint8_t minute = 0; minute < 60; minute++)
    {
      char buffer[5];
      sprintf(buffer, "%01d:%02d", h, minute);
      String time = String(buffer);

      int len = time.length();
      Serial.print("Length: "); Serial.println(len);
      int width = len * 3;
      Serial.print("Width: "); Serial.println(width);
      int16_t left = ((int16_t)((clock->width() - width) / 2.0)) - 1;
      Serial.print("Left: "); Serial.println(left);

      clock->reset();
      clock->setCursor(left, 6);
      clock->println(time);
      clock->drawPixel(18, 5, pm ? 1 : 0);

      delay(delayTime);
    }
  }
}

#endif
