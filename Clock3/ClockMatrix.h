#ifndef CLOCK_MATRIX_H
#define CLOCK_MATRIX_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "MyPins.h"
#include "Bitwise.h"

// ***
// *** This device is fixed at 7 rows and 20 columns.
// ***
#define ROWS       7
#define COLUMNS   20

class ClockLedMatrix : public Adafruit_GFX
{
  public:
    ClockLedMatrix();
    void begin();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void refresh();
    void reset();

  protected:
    // ***
    // *** Each byte is used to represent the rows for the given column (the
    // *** index of the array). eacg bit represents the row. The LSB is row 1,
    // *** the second bit is row 2 and so on. The MSB is not used since there
    // *** 8 bits and only 7 rows.
    // ***
    uint8_t _matrixBuffer[COLUMNS];

    // ***
    // *** Represents the current colum being displayed during the refresh cycle.
    // ***
    uint8_t _currentColumn;

    // ***
    // *** Draws the specified column where the bits rows determine
    // *** which LEDs in the row are on.
    // ***
    void drawColumn(uint8_t column, byte rows);
};
#endif
