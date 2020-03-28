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
#include "LedMatrix.h"

LedMatrix::LedMatrix() : Adafruit_GFX(COLUMNS, ROWS)
{
  this->setRefreshRate(DEFAULT_REFRESH_RATE);
  this->setTextSize(1);
  this->setTextWrap(false);
}

void LedMatrix::begin()
{
  // ***
  // *** Set the column select pins up for output.
  // ***
  DECODER_REGISTER = (DECODER_REGISTER & B11111111) | (_BV(DECODER_C) | _BV(DECODER_B) | _BV(DECODER_A));

  // ***
  // *** Set the row select pins up for output.
  // ***
  ROW_REGISTER = (ROW_REGISTER & B11111111) | (_BV(ROW_7) | _BV(ROW_6) | _BV(ROW_5) | _BV(ROW_4) | _BV(ROW_3) | _BV(ROW_2) | _BV(ROW_1));

  // ***
  // *** Set the chip select pins up for output.
  // ***
  CHIP_SELECT_REGISTER = (CHIP_SELECT_REGISTER & B11111111) | (_BV(CS_S2) | _BV(CS_S1));

  // ***
  // *** Reset the display matrix.
  // ***
  this->reset();
}

void LedMatrix::begin(const GFXfont* f)
{
  this->setFont(f);
  this->begin();
}

const uint8_t LedMatrix::getRefreshRate()
{
  return this->_refreshRate;
}

void LedMatrix::setRefreshRate(uint8_t refreshRate)
{
  this->_refreshRate = refreshRate;

  // ***
  // *** Returns number of microseconds.
  // ***
  // *** 1. Refresh rate = x screen updates per second
  // *** 2. Multiple refresh rate by number of columns to get columns per second.
  // *** 3. Divide 1 by columns per second to get seconds per column
  // *** 4. Multiple by 1000 to get milliseconds per column.
  // *** 5. Multiple by 1000 to microseconds per column.
  // ***
  this->_refreshDelay = (uint64_t)(1.0 / ((float)this->_refreshRate * (float)this->width()) * 1000.0 * 1000.0);
}

const uint32_t LedMatrix::getRefreshDelay()
{
  return this->_refreshDelay;
}

void LedMatrix::drawPixel(int16_t column, int16_t row, uint16_t color)
{
  if (color == 0)
  {
    // ***
    // *** Clear the bit for the given row.
    // ***
    CLEAR_BIT(this->_matrixBuffer[column], row);
  }
  else
  {
    // ***
    // *** Set the bit for the given row.
    // ***
    SET_BIT(this->_matrixBuffer[column], row);
  }

  // ***
  // *** Calculate the column compensation delay.
  // ***
  if (this->_matrixBuffer[column] != 0)
  {
    // ***
    // *** Get the count of bits that are set to 1 for the
    // *** column y.
    // ***
    uint8_t bitCount = this->getBitCount(this->_matrixBuffer[column]);

    // ***
    // *** Compensate COMP_DELAY_PER_BIT microseconds
    // *** per row NOT displayed.
    // ***
    uint64_t compensationDelay = (8 * COMP_DELAY_PER_BIT) - (COMP_DELAY_PER_BIT * bitCount);

    // ***
    // *** Store the value so the refresh routine does not have to
    // *** do this work during the refresh cycle.
    // ***
    this->_compensationDelay[column] = compensationDelay;
  }
}

void LedMatrix::refresh()
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
  // *** Check if the last column has been updated.
  // ***
  if (this->_currentColumn == this->width())
  {
    this->_currentColumn = 0;
  }
}

void LedMatrix::drawColumn(uint8_t column, uint8_t rows)
{
  // ***
  // *** Disable all decoders. This turn all LEDs off.
  // ***
  CHIP_SELECT_PORT = DECODERS_OFF;

  // ***
  // *** Since this method is called in rapid succession, we can use a delay here to simulate
  // *** PWM on the display. A longer delay here makes the display look dimmer. Depending on
  // *** the number of rows that are turned on, a delay is calculated to compensate for when
  // *** a column has less LEDs lit up and therefore looks brighter. The less rows displayed,
  // *** the longer the delay (more time spent off).
  // ***
  delayMicroseconds(this->_compensationDelay[column]);

  // ***
  // *** Set the rows. The correct bits are already set
  // *** in the rows parameter when the drawPixel method
  // *** was called.
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

uint8_t LedMatrix::getBitCount(uint8_t rows)
{
  uint8_t returnValue = 0;

  for (int b = 0; b < 8; b++)
  {
    returnValue += (B00000001 & rows);
    rows = rows >> 1;
  }

  return returnValue;
}

void LedMatrix::reset()
{
  // ***
  // *** Disable all decoders.
  // ***
  CHIP_SELECT_PORT = DECODERS_OFF;

  // ***
  // *** All rows off.
  // ***
  ROW_PORT = ROW_PORT & 0b1000000;

  // ***
  // *** Reset all row bits.
  // ***
  this->clear();

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

void LedMatrix::clear()
{
  // ***
  // *** Reset all row bits.
  // ***
  for (uint8_t column = 0; column < COLUMNS; column++)
  {
    this->_matrixBuffer[column] = 0;
    this->_compensationDelay[column] = 0;
  }
}

const uint16_t LedMatrix::getTextWidth(const char* text)
{
  // ***
  // *** Since this display is one line only and is using a
  // *** specially designed font. This is a simplified calculation
  // *** that only uses the xAdvance value in each character
  // *** to determine width.
  // ***
  uint16_t returnValue = 0;

  uint8_t first = pgm_read_byte(&gfxFont->first);
  uint8_t last = pgm_read_byte(&gfxFont->last);
  char c;

  while ((c = *text++))
  {
    if ((c >= first) && (c <= last))
    {
      GFXglyph* glyph = pgm_read_glyph_ptr(gfxFont, c - first);
      uint8_t xa = pgm_read_byte(&glyph->xAdvance);
      returnValue += xa;
    }
  }

  return returnValue;
}

void LedMatrix::drawTextCentered(const char* text)
{
  // ***
  // *** Clear the display.
  // ***
  this->clear();

  // ***
  // *** Calculate the width of the text. Remove the
  // *** 1 pixel space at the end of the last character.
  // ***
  float textWidth = this->getTextWidth(text) - 1;

  // ***
  // *** Calculate the left position by dividing the difference
  // *** between the screen width and the text width by 2.
  // ***
  float left = (this->width() - textWidth) / 2.0;

  // ***
  // *** Set the cursor at the calculated left position and the
  // *** bottom of the display.
  // ***
  this->setCursor(left, this->height() - 1);

  // ***
  // *** Display the text.
  // ***
  this->print(text);
}

void LedMatrix::testDisplay(uint16_t delayTime)
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
      //String time = String(buffer);

      this->reset();
      this->drawTextCentered(buffer);
      this->drawPixel(18, 5, pm ? 1 : 0);

      delay(delayTime);
    }
  }
}

void LedMatrix::powerOnDisplayTest()
{
  // ***
  // *** Light each LED.
  // ***
  for (uint8_t x = 0; x < this->width(); x++)
  {
    for (uint8_t y = 0; y < this->height(); y++)
    {
      this->drawPixel(x, y, 1);
      delay(10);
    }
  }

  // ***
  // *** Pause for 2 seconds to show each
  // *** LED is working.
  // ***
  delay(2000);

  // ***
  // *** Clear the display.
  // ***
  this->clear();
}

void LedMatrix::drawMomentaryTextCentered(const char* text, uint64_t displayTime, bool resetAfter)
{
  // ***
  // *** Draw the text centered.
  // ***
  this->drawTextCentered(text);

  // ***
  // *** Use delay to pause the text.
  // ***
  delay(displayTime);

  // ***
  // *** Clear the display if specified.
  // ***
  if (resetAfter)
  {
    this->clear();
  }
}
