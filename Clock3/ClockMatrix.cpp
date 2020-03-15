#include "ClockMatrix.h"

ClockLedMatrix::ClockLedMatrix() : Adafruit_GFX(COLUMNS, ROWS)
{
}

void ClockLedMatrix::begin()
{
  // ***
  // *** Set the column select pins up for output.
  // ***
  DECODER_REGISTER = (1 << DECODER_C) | (1 << DECODER_B) | (1 << DECODER_A);

  // ***
  // *** Set the row selection pins up for output.
  // ***
  ROW_REGISTER = (1 << ROW_7) | (1 << ROW_6) | (1 << ROW_5) | (1 << ROW_4) | (1 << ROW_3) | (1 << ROW_2) | (1 << ROW_1);

  // ***
  // *** Set the chip select pins up for output.
  // ***
  CHIP_SELECT_REGISTER = (1 << CS_S2) | (1 << CS_S1);

  // ***
  // *** Reset the display matrix.
  // ***
  this->reset();
}

void ClockLedMatrix::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (color == 0)
  {
    clearBit(this->_matrixBuffer[x], y);
  }
  else
  {
    setBit(this->_matrixBuffer[x], y);
  }
}

void ClockLedMatrix::drawColumn(uint8_t column, byte rows)
{
  if (column >= 0 && column < this->width())
  {
    // ***
    // *** Disable all decoders.
    // ***
    CHIP_SELECT_PORT = DECODERS_OFF;

    // ***
    // *** Set the rows. The correct bits are already set
    // ***
    ROW_PORT = (ROW_PORT & B10000000) | rows;

    // ***
    // *** Select the correct column. After taking the modulo of
    // *** the column number with 8, the last 3 bits give the value of
    // *** of the A, B and C channels. These 3 bits need to be
    // *** shifted into the correction positions for the decorder
    // *** port.
    // ***
    DECODER_PORT = (DECODER_PORT & B10001111) | ((column % 8) << 4);

    // ***
    // *** Select the correct column bank.
    // ***
    if (column >= 0 && column <= 7)
    {
      CHIP_SELECT_PORT = DECODER_1;
    }
    else if (column >= 8 && column <= 15)
    {
      CHIP_SELECT_PORT = DECODER_2;
    }
    else if (column >= 15 && column <= 19)
    {
      CHIP_SELECT_PORT = DECODER_3;
    }
  }
}

void ClockLedMatrix::reset()
{
  // ***
  // *** Disable all decoders.
  // ***
  CHIP_SELECT_PORT = DECODERS_OFF;

  // ***
  // *** All rows off.
  // ***
  ROW_PORT = ROW_PORT & 0b1000000;

  for (uint8_t column = 0; column < COLUMNS; column++)
  {
    this->_matrixBuffer[column] = 0;
  }

  // ***
  // *** Reset the row and column counter for the refresh.
  // ***
  this->_currentColumn = 0;

  // ***
  // *** Reset the cursor.
  // ***
  this->setCursor(0, 0);
}

void ClockLedMatrix::refresh()
{
  // ***
  // *** Draw the current column.
  // ***
  this->drawColumn(this->_currentColumn, this->_matrixBuffer[this->_currentColumn]);

  // ***
  // *** Increment the current column.
  // ***
  this->_currentColumn++;

  // ***
  // ***
  // ***
  if (this->_currentColumn == this->width())
  {
    this->_currentColumn = 0;
  }
}
