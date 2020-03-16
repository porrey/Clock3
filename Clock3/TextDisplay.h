#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

void drawTextCentered(String text)
{
  // ***
  // *** Calculate the width and left position.
  // ***
  int len = text.length();
  int width = len * 3;
  int16_t left = ((int16_t)((this->width() - width) / 2.0)) - 1;
  this->setCursor(left, this->height() - 1);
  this->println(text);
}

void drawMomentaryTextCentered(String text, uint64_t displayTime, bool resetAfter)
{
  drawTextCenetered(text);
  displayTime(displayTime);

  if (resetAfter)
  {
    _display.reset();
  }
}
#endif
