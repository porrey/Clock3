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
