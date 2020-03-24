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
#ifndef CLOCK_MATRIX_H
#define CLOCK_MATRIX_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "MyPins.h"
#include "Bitwise.h"
#include "gfxfont.h"

// **************************************************************************** //
// ******************** Borrowed from Adafruit GFX library ******************** //
// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {
#ifdef __AVR__
  return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
  // expression in __AVR__ section may generate "dereferencing type-punned
  // pointer will break strict-aliasing rules" warning In fact, on other
  // platforms (such as STM32) there is no need to do this pointer magic as
  // program memory may be read in a usual way So expression may be simplified
  return gfxFont->glyph + c;
#endif //__AVR__
}
// **************************************************************************** //
// **************************************************************************** //

// ***
// *** Define the default refresh interval in microseconds. 50 seems
// *** to be the sweet spot.
// ***
#define DEFAULT_REFRESH_RATE 50

// ***
// *** Define the compensation delay per bit in microseconds.
// ***
#define COMP_DELAY_PER_BIT 100

// ***
// *** The maximum delay to apply when brigtness is 0.
// ***
#define MAX_PWM_DELAY 500

// ***
// *** This device is fixed at 7 rows and 20 columns.
// ***
#define ROWS       7
#define COLUMNS   20

class ClockLedMatrix : public Adafruit_GFX
{
  public:
    // ***
    // *** Create a default instance.
    // ***
    ClockLedMatrix();

    // ***
    // *** Initialize the display.
    // ***
    void begin();

    // ***
    // *** Get/set the refresh of the display.
    // ***
    uint8_t getRefreshRate();
    void setRefreshRate(uint8_t);

    // ***
    // *** Implements drawPixel for this display enabling all of the GFX
    // *** capabilities.
    // ***
    void drawPixel(int16_t, int16_t, uint16_t);

    // ***
    // *** Performs a single LED write. In FULL_COLUMN mode this will turn
    // *** on the next full column of LEDs. In INDIVIDUAL_LED this will only
    // *** turn on one LED at a time. This needs to be called repeatedly at
    // *** a high rate for the display to look normal. If this not called
    // *** enough the screen will flash.
    // ***
    void refresh();

    // ***
    // *** Gets the recommended time between refreshes based on the selected mode.
    // ***
    uint32_t getRefreshDelay();

    // ***
    // *** Resets and clears the entire display.
    // ***
    void reset();

    // ***
    // *** Clears the rows and columns by settign all colos to 0.
    // ***
    void clear();

    // ***
    // *** Get the width of text for this display.
    // ***
    uint16_t getTextWidth(String);

    // ***
    // *** Draws a string centered on the display.
    // ***
    void drawTextCentered(String);

    // ***
    // *** This routine will lop through the time of day.
    // ***
    void testDisplay(uint16_t);

  protected:
    // ***
    // *** This specifies the number of times per second the
    // *** entire screen is refreshed.
    // ***
    uint8_t _refreshRate = DEFAULT_REFRESH_RATE;

    // ***
    // *** Number of microseconds of delay between each column draw to simulate
    // *** the specified refresh rate. This is used by the external timer.
    // ***
    uint64_t _refreshDelay;

    // ***
    // *** Each byte is used to represent the rows for the given column (the
    // *** index of the array). eacg bit represents the row. The LSB is row 1,
    // *** the second bit is row 2 and so on. The MSB is not used since there
    // *** 8 bits and only 7 rows.
    // ***
    uint8_t _matrixBuffer[COLUMNS];

    // ***
    // *** Represents the current colum being displayed during the refresh cycle. This
    // *** value is used in both FULL_COLUMN and INDIVIDUAL_LED refresh modes.
    // ***
    uint8_t _currentColumn;

    // ***
    // *** Represents the current row being displayed during the refresh cycle. This
    // *** value is only used when the refresh mode is INDIVIDUAL_LED.
    // ***
    uint8_t _currentRow;

    // ***
    // *** The amount of compensation needed for a particular column based on
    // *** the number of LEDs in the column that are turned on.
    // ***
    uint64_t _compensationDelay[COLUMNS];

    // ***
    // *** Draws the specified column where the bits in the second
    // *** paramter determine which LEDs in the row are on.
    // ***
    void drawColumn(uint8_t, uint8_t);

    // ***
    // *** Counts the number of bits that are '1'.
    // ***
    uint8_t getBitCount(uint8_t);
};
#endif
