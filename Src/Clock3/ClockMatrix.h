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
// *** This device is fixed at 7 rows and 20 columns.
// ***
#define ROWS       7
#define COLUMNS   20

class ClockLedMatrix : public Adafruit_GFX
{
  public:
    // ***
    // *** Specifies how the display is refreshed.
    // ***
    enum RefreshMode
    {
      // ***
      // *** FULL_COLUMN mode updates the display
      // *** a full column at a time. This mode is faster and can use a lower refresh rate
      // *** but individual LED brightness can vary depending onhow many LEDs are on in a
      // *** given column.
      // ***
      FULL_COLUMN  = 0,
      // ***
      // *** INDIVIDUAL_LED mode is slower and requires a higher refresh rate
      // *** but results in consistent LED brightness.
      // ***
      INDIVIDUAL_LED = 1
    };

    // ***
    // ***
    // ***
    ClockLedMatrix();

    // ***
    // ***
    // ***
    ClockLedMatrix(RefreshMode refreshMode);

    // ***
    // ***
    // ***
    void begin();

    // ***
    // ***
    // ***
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    // ***
    // *** Indicates how the display is refreshed. FULL_COLUMN mode updates the display
    // *** a full column at a time. This mode is faster and can use a lower refresh rate
    // *** but individual LED brightness can vary depending onhow many LEDs are on in a
    // *** given column. INDIVIDUAL_LED mode is slower and requires a higher refresh rate
    // *** but results in consistent LED brightness.
    // ***
    RefreshMode refreshMode;

    // ***
    // ***
    // ***
    void refresh();

    // ***
    // ***
    // ***
    uint16_t refreshDelay();

    // ***
    // ***
    // ***
    void reset();
    
    // ***
    // *** Get the width of text for this display.
    // ***
    uint16_t getTextWidth(String text);
    
  protected:
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
    // *** Draws the specified column where the bits rows determine
    // *** which LEDs in the row are on.
    // ***
    void drawColumn(uint8_t column, uint8_t rows);

    // ***
    // *** Draws the specified LED for the given column and row.
    // ***
    void drawLed(uint8_t column, uint8_t row);
};
#endif
