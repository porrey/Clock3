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
#include "ClockMatrix.h"

ClockLedMatrix::ClockLedMatrix() : Adafruit_GFX(COLUMNS, ROWS)
{
  this->refreshMode = ClockLedMatrix::FULL_COLUMN;
}

ClockLedMatrix::ClockLedMatrix(RefreshMode refreshMode) : Adafruit_GFX(COLUMNS, ROWS)
{
  this->refreshMode = refreshMode;
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

uint16_t ClockLedMatrix::refreshDelay()
{
  uint16_t returnValue = 0;

  if (this->refreshMode == FULL_COLUMN)
  {
    returnValue = 1000;
  }
  else
  {
    returnValue = 150;
  }

  Serial.print("Refresh rate = "); Serial.println(returnValue);

  return returnValue;
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

void ClockLedMatrix::drawColumn(uint8_t column, uint8_t rows)
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
    if (column <= 7)
    {
      CHIP_SELECT_PORT = DECODER_1;
    }
    else if (column <= 15)
    {
      CHIP_SELECT_PORT = DECODER_2;
    }
    else if (column <= 19)
    {
      CHIP_SELECT_PORT = DECODER_3;
    }
  }

  // ***
  // *** Increment the current column.
  // ***
  this->_currentColumn++;

  // ***
  // *** Check if the last column has been updated.
  // ***
  if (this->_currentColumn == this->width())
  {
    this->_currentColumn = 0;
  }
}

void ClockLedMatrix::drawLed(uint8_t column, uint8_t row)
{
  if (column >= 0 && column < this->width())
  {
    // ***
    // *** Disable all decoders.
    // ***
    CHIP_SELECT_PORT = DECODERS_OFF;

    // ***
    // *** Set the rows. The mask selects the current bit.
    // ***
    uint8_t mask = (1 << row);
    uint8_t bits = this->_matrixBuffer[this->_currentColumn] & mask;
    ROW_PORT = (ROW_PORT & B10000000) | bits;

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
    if (column <= 7)
    {
      CHIP_SELECT_PORT = DECODER_1;
    }
    else if (column <= 15)
    {
      CHIP_SELECT_PORT = DECODER_2;
    }
    else if (column <= 19)
    {
      CHIP_SELECT_PORT = DECODER_3;
    }
  }

  // ***
  // *** Increment the current row.
  // ***
  this->_currentRow++;

  // ***
  // *** Check if the last row has been updated.
  // ***
  if (this->_currentRow == this->height())
  {
    this->_currentRow = 0;

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
      this->_currentRow = 0;
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
  this->_currentRow = 0;
  this->_currentColumn = 0;

  // ***
  // *** Reset the cursor.
  // ***
  this->setCursor(0, 0);
}

void ClockLedMatrix::refresh()
{
  if (this->refreshMode == ClockLedMatrix::FULL_COLUMN)
  {
    // ***
    // *** Draw the current column.
    // ***
    this->drawColumn(this->_currentColumn, this->_matrixBuffer[this->_currentColumn]);
  }
  else
  {
    // ***
    // *** Draw the current column.
    // ***
    this->drawLed(this->_currentColumn, this->_currentRow);
  }
}

uint16_t ClockLedMatrix::getTextWidth(String text)
{
  // ***
  // *** Since this display is one line onyl and is using a
  // *** specially designed font. This is a simplified calculation
  // *** that only uses the xAdvance value in each character
  // *** to determine width.
  // ***
  uint16_t returnValue = 0;

  uint8_t first = pgm_read_byte(&gfxFont->first);
  uint8_t last = pgm_read_byte(&gfxFont->last);
  char c;
  const char *str = const_cast<char *>(text.c_str());

  while ((c = *str++))
  {
    if ((c >= first) && (c <= last))
    {
      GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
      uint8_t xa = pgm_read_byte(&glyph->xAdvance);
      returnValue += xa;
    }
  }

  return returnValue;
}
