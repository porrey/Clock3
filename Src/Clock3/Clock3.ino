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
#include "Tests.h"
#include "ClockMatrix.h"
#include "ClockFont.h"
#include "TimerOne.h"
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "Gps.h"
#include <EEPROM.h>
#include <AceButton.h>
using namespace ace_button;

// ***
// *** EEPROM addresses for settings that need to be saved
// *** when the clock is not powered on.
// ***
#define EEPROM_ADR_TZ   0
#define EEPROM_ADR_DST  2

// ***
// *** Define the serial port for displaying debug messages. When debugging
// *** on an Arduino, this should be the standard Serial port. Note we specify
// *** RX of 0 since we only send data and do not expect to receive any data.
// ***
SoftwareSerial Debug(0, 7); // RX, TX
//#define Debug Serial

// ***
// *** Define the Serial port used by the GPS. For the clock this should be the standard
// *** serial port. If debugging on an Arduino, this should use the soft serial port. When
// *** debugging on an UNO, only digital ports 2 and 3 will work for the SoftwareSerial
// *** as long as the buttons are not connected (the mode and setup buttons use these pins).
// ***
#define GpsSerial Serial
//SoftwareSerial GpsSerial(2, 3); // RX, TX

// ***
// *** An instance of the TinyGPS interface.
// ***
TinyGPS _gps;

// ***
// *** Create an instance of the Clock LED Display Matrix that is part
// *** of the Spikenzielabs clock kit found at
// *** https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock.
// *** Although the use INDIVIDUAL_LED mode would look nicer, it requires so
// *** much CPU time that it interferres with the serial port and stops the GPS
// *** from working. We will use FULL_COLUMN mode instead.
// ***
ClockLedMatrix _display = ClockLedMatrix(ClockLedMatrix::FULL_COLUMN);

// ***
// *** An instance of the the RTC_DS1307 library. The RTC in the Spikenzielabs clock
// *** is a DS1337 but the interface for DS1307 works just as well.
// ***
RTC_DS1307 _rtc;

// ***
// *** The time zone offset.
// ***
int16_t _tz_offset = -6;

// ***
// ***
// ***
bool _isDst = true;

// ***
// *** Tracks the last minute displayed so the display can
// *** be updated once per minute.
// ***
int16_t _lastMinuteDisplayed = -1;

// ***
// *** The time of the last GPS update.
// ***
DateTime _lastGpsUpdate = DateTime(1900, 1, 1, 0, 0, 0);
bool _gpsFix = false;

// ***
// *** Mode
// ***
bool _modeChanged = true;
uint8_t _mode = 0;

#define MODE_DISPLAY_TIME 0
#define MODE_TZ 1
#define MODE_DST 2

// ***
// *** Setup
// ***
bool _setupChanged = false;

// ***
// *** Create button objects for the mode and setup buttons.
// ***
#define MODE_BUTTON_ID 0
#define SETUP_BUTTON_ID 1

AceButton _buttons[2];

void setup()
{
  // ***
  // *** Initialize the default serial port.
  // ***
  Debug.begin(115200);
  Debug.println(F("Serial has been initialized."));

  // ***
  // *** Initialize the RTC.
  // ***
  if (!_rtc.begin())
  {
    Debug.println(F("Couldn't find the RTC."));
  }
  else
  {
    Debug.println(F("The RTC has been initialized."));
  }

  // ***
  // *** Initialize the LED matrix.
  // ***
  _display.begin();

  // ***
  // *** Set the font.
  // ***
  _display.setTextWrap(false);
  _display.setFont(&ClockFont);
  _display.setTextSize(1);
  Debug.println(F("The display has been initialized."));

  // ***
  // *** Set up the timer for display refresh. Use the recommended
  // *** values based on refresh mode.
  // ***
  Timer1.initialize(_display.refreshDelay());
  Timer1.attachInterrupt(refreshDisplay);

  // ***
  // *** Initialize the GPS.
  // ***
  GpsSerial.begin(9600);


  GpsSerial.println(PMTK_SET_BAUD_9600); delay(250);
  GpsSerial.println(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); delay(250);
  GpsSerial.println(PMTK_API_SET_FIX_CTL_1HZ); delay(250);
  GpsSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA); delay(250);
  GpsSerial.println(PMTK_ENABLE_WAAS); delay(250);
  GpsSerial.println(PGCMD_NOANTENNA); delay(250);

  // ***
  // *** Check the RTC.
  // ***
  if (!_rtc.isrunning())
  {
    Debug.println(F("RTC is NOT running!"));
  }
  else
  {
    Debug.println(F("The RTC is running."));

    // ***
    // *** Read the date and time and display it
    // *** on the serial device.
    // ***
    DateTime time = _rtc.now();
    Debug.print(F("DateTime::TIMESTAMP_FULL: ")); Serial.println(time.timestamp(DateTime::TIMESTAMP_FULL));
  }

  // ***
  // *** Initialize the button pins.
  // ***
  pinMode(MODE_BUTTON, INPUT);
  pinMode(SETUP_BUTTON, INPUT_PULLUP);

  // ***
  // *** Configure the ButtonConfig with the event handler and
  // *** and the required features.
  // ***
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(buttonEventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  // ***
  // *** Initialize the buttons.
  // ***
  _buttons[MODE_BUTTON_ID].init(buttonConfig, MODE_BUTTON, HIGH, MODE_BUTTON_ID);
  _buttons[SETUP_BUTTON_ID].init(buttonConfig, SETUP_BUTTON, HIGH, SETUP_BUTTON_ID);

  // ***
  // *** Restore time zone offset and DST values from EEPROM.
  // ***
  EEPROM.get(EEPROM_ADR_TZ, _tz_offset);
  EEPROM.get(EEPROM_ADR_DST, _isDst);

  // ***
  // *** Power on display test.
  // ***
  powerOnDisplayTest();

  // ***
  // *** Show version number.
  // ***
  drawMomentaryTextCentered(_display, "clk 3", 2000, true);

  Debug.println(F("Setup completed."));
  Debug.println(F(""));
}

void loop()
{
  // ***
  // *** Check to see if it is time to update the RTC time
  // *** from the GPS data. We will update every hour
  // *** (3600 seconds).
  // ***
  if (checkIfTimeForRtcUpdateFromGps(_lastGpsUpdate, _gps, _rtc, 3600))
  {
    _lastGpsUpdate = _rtc.now();
    _gpsFix = true;
    Debug.println("GPS Fix = Yes");
  }

  // ***
  // *** Check the current mode.
  // ***
  switch (_mode)
  {
    // ***
    // *** Display the time.
    // ***
    case MODE_DISPLAY_TIME:
      {
        // ***
        // *** Update the displayed time.
        // ***
        if (updateTimeDisplay(_display, _rtc, _lastMinuteDisplayed, _tz_offset, _isDst, _modeChanged))
        {
          // ***
          // *** Track the time of the last update. We only change the display each minute.
          // ***
          _lastMinuteDisplayed = _rtc.now().minute();
        }

        // ***
        // *** Display the GPS fix indicator.
        // ***
        updateGpsFixDisplay(_gpsFix);
      }
      break;
    case MODE_TZ:
      {
        // ***
        // *** Display the current offset.
        // ***
        if (_modeChanged)
        {
          drawMomentaryTextCentered(_display, "TZ", 1000, true);
        }

        if (_modeChanged || _setupChanged)
        {
          displayOffset(_display, _tz_offset);
        }
      }
      break;
    case MODE_DST:
      {
        // ***
        // *** Display the current DST mode.
        // ***
        if (_modeChanged)
        {
          drawMomentaryTextCentered(_display, "DST", 1000, true);
        }

        if (_modeChanged || _setupChanged)
        {
          displayDst(_display, _isDst);
        }
      }
      break;
  }

  // ***
  // *** Reset mode and setup change flags.
  // ***
  _modeChanged = false;
  _setupChanged = false;

  // ***
  // *** Use smartDelay() instead of delay().
  // ***
  smartDelay(250);
}

// ***
// *** In this application, every CPU tick is critical. Instead
// *** of using delay() in the application, this routine simulates
// *** a delay but puts the microcontroller to work while
// *** the delay is in affect. This should not be used when the
// *** delay time needs to be accurate. The loop could run longer
// *** depending on the amount of data available on the serial port.
// ***
void smartDelay(uint64_t delayTime)
{
  // ***
  // *** Mark the start time.
  // ***
  uint64_t start = millis();

  // ***
  // *** Enter a loop.
  // ***
  do
  {
    // ***
    // *** Read data from the GPS while
    // *** it is available.
    // ***
    while (GpsSerial.available())
    {
      _gps.encode(GpsSerial.read());
    }

    // ***
    // *** Check the buttons to update their state.
    // ***
    _buttons[MODE_BUTTON_ID].check();
    _buttons[SETUP_BUTTON_ID].check();

  } while (millis() - start < delayTime);
}

void buttonEventHandler(AceButton* button, uint8_t eventType, uint8_t state)
{
  // ***
  // *** Get the ID of the button that was pressed.
  // ***
  uint8_t id = button->getId();

  switch (id)
  {
    case MODE_BUTTON_ID:
      {
        switch (eventType)
        {
          case AceButton::kEventLongPressed:
            {
              // ***
              // *** A long press of the mode button will cause the time
              // *** to update from the GPS.
              // ***
              Debug.println(F("GPS"));
              drawMomentaryTextCentered(_display, "GPS", 1500, true);
              _gpsFix = false;
              _lastGpsUpdate = DateTime(1900, 1, 1, 0, 0, 0);
              _modeChanged = true;
            }
            break;
          case AceButton::kEventReleased:
            {
              // ***
              // *** Increment the mode and set the mode
              // *** changed flag.
              // ***
              _mode = (++_mode) % 3;
              _modeChanged = true;
              Debug.print(F("Mode = ")); Debug.println(_mode);
            }
            break;
        }
      }
      break;
    case SETUP_BUTTON_ID:
      {
        switch (eventType)
        {
          case AceButton::kEventLongPressed:
            {
              Debug.println(F("NOT USED YET"));
            }
            break;
          case AceButton::kEventReleased:
            {
              switch (_mode)
              {
                case MODE_TZ:
                  {
                    // ***
                    // *** Save the value to EEPROM only 
                    // *** when the button is released.
                    // ***
                    EEPROM.put(EEPROM_ADR_TZ, _tz_offset);
                  }
                case MODE_DST:
                  {
                    // ***
                    // *** Save the value to EEPROM only 
                    // *** when the button is released.
                    // ***
                    EEPROM.put(EEPROM_ADR_DST, _isDst);
                  }
              }
            }
            break;
          case AceButton::kEventPressed:
          case AceButton::kEventRepeatPressed:
            {
              switch (_mode)
              {
                case MODE_TZ:
                  {
                    // ***
                    // *** Increment the TZ offset
                    // *** by 60 minutes.
                    // ***
                    _tz_offset++;

                    if (_tz_offset > 14)
                    {
                      _tz_offset = -14;
                    }

                    _setupChanged = true;
                    Debug.print(F("TZ = ")); Debug.println(_tz_offset);
                  }
                  break;
                case MODE_DST:
                  {
                    // ***
                    // *** Toggle the DST flag.
                    // ***
                    _isDst = !_isDst;
                    _setupChanged = true;
                    Debug.print(F("DST = ")); Debug.println(_isDst);
                  }
                  break;
              }
            }
            break;
        }
      }
      break;
  }
}

bool updateTimeDisplay(const ClockLedMatrix& display, const RTC_DS1307& rtc, int16_t lastMinuteDisplayed, int16_t tz_offset, bool isDst, bool force)
{
  bool returnValue = false;

  DateTime now = rtc.now();

  if (force | lastMinuteDisplayed != now.minute())
  {
    Debug.print(F("RTC Date/Time: ")); Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Clear the display.
    // ***
    display.reset();

    // ***
    // *** Adjust for the current time zone.
    // ***
    Debug.print(F("Time Zone Offset: ")); Serial.println(tz_offset);
    Debug.print(F("DST: ")); Serial.println(isDst ? F("Yes") : F("No"));
    DateTime localNow = DateTime(now.unixtime() + (tz_offset * 3600) + (isDst ? 3600 : 0));
    Debug.print(F("Local Date/Time: ")); Serial.println(localNow.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Get a string version of the time.
    // ***
    String time = dateTimeToTimeString(&localNow);
    Debug.print(F("Time Display: ")); Debug.println(time);

    // ***
    // *** Update the time on the display.
    // ***
    drawTextCentered(display, time);

    // ***
    // *** Update the AM/PM mark on the display.
    // ***
    updateAmPmDisplay(&localNow);

    Debug.println(F(""));

    returnValue = true;
  }

  return returnValue;
}

void updateGpsFixDisplay(bool gpsHasTime)
{
  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  _display.drawPixel(18, 1, gpsHasTime ? 1 : 0);
}

void updateAmPmDisplay(DateTime* now)
{
  // ***
  // ** Check if it is PM.
  // ***
  bool pm = now->hour() >= 12;
  Debug.print(F("AM/PM => ")); Debug.println(pm ? F("PM") : F("AM"));

  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  _display.drawPixel(18, 5, pm ? 1 : 0);
}

bool checkIfTimeForRtcUpdateFromGps(DateTime lastGpsUpdate, const TinyGPS& gps, const RTC_DS1307& rtc, uint64_t updateInterval)
{
  bool returnValue = false;

  // ***
  // *** Get the number of seconds since the last GPS update.
  // ***
  uint32_t secondsSinceLastGpsUpdate = rtc.now().unixtime() - lastGpsUpdate.unixtime();

  // ***
  // *** Check if the time needs to be set from the GPS signal.
  // ***
  if (secondsSinceLastGpsUpdate > updateInterval)
  {
    Debug.println(F("Triggering RTC update from GPS."));

    // ***
    // *** Try to get the time from GPS and set the clock.
    // ***
    returnValue = setDateAndTimeFromGps(gps, rtc);
  }

  return returnValue;
}

void refreshDisplay()
{
  // ***
  // *** Refresh the LED matrix.
  // ***
  _display.refresh();
}

void powerOnDisplayTest()
{
  // ***
  // *** Light each LED.
  // ***
  for (uint8_t y = 0; y < _display.height(); y++)
  {
    for (uint8_t x = 0; x < _display.width(); x++)
    {
      _display.drawPixel(x, y, 1);
      smartDelay(1);
    }
  }

  // ***
  // *** Pause for 2 seconds by reading GPS data.
  // ***
  smartDelay(2000);

  // ***
  // *** Reset the display.
  // ***
  _display.reset();
}

bool setDateAndTimeFromGps(const TinyGPS& gps, const RTC_DS1307& rtc)
{
  bool returnValue = false;

  Debug.println(F("Getting time from GPS."));

  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;

  // ***
  // *** Get the date and time from the GPS data.
  // ***
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  // ***
  // *** Check the age to make sure the date and time are valid.
  // ***
  if (age != TinyGPS::GPS_INVALID_AGE)
  {
    // ***
    // *** Create  UTC date and time instance.
    // ***
    DateTime dt = DateTime(year, month, day, hour, minute, second);
    Debug.print(F("GPS/UTC Date/Time: ")); Serial.println(dt.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Set the time on the RTC.
    // ***
    // ***
    Debug.println(F("RTC time has been set."));
    rtc.adjust(dt);

    // ***
    // *** Return true to indicate the clock was set.
    // ***
    returnValue = true;
  }
  else
  {
    Debug.print(F("Date and Time not available. Age = ")); Debug.println(age);
    returnValue = false;
  }

  Debug.println(F(""));

  return returnValue;
}

String dateTimeToTimeString(DateTime* now)
{
  uint8_t hour = now->hour() > 12 ? now->hour() - 12 : now->hour();
  char buffer[5];
  sprintf(buffer, "%01d:%02d", hour, now->minute());
  return String(buffer);
}

void displayOffset(const ClockLedMatrix& display, int16_t tz_offset)
{
  display.reset();
  display.setCursor(0, 6);
  display.println(tz_offset);
}

void displayDst(const ClockLedMatrix& display, bool isDst)
{
  char buffer[3];
  sprintf(buffer, "%s", isDst ? "Yes" : "No");

  // ***
  // *** Update the time on the display.
  // ***
  drawTextCentered(display, String(buffer));
}

void drawTextCentered(const ClockLedMatrix& display, String text)
{
  display.reset();

  // ***
  // *** Calculate the width and left position.
  // ***
  int len = text.length();
  int width = len * 3;
  int16_t left = ((int16_t)((display.width() - width) / 2.0)) - 1;
  display.setCursor(left, display.height() - 1);
  display.println(text);
}

void drawMomentaryTextCentered(const ClockLedMatrix& display, String text, uint64_t displayTime, bool resetAfter)
{
  drawTextCentered(display, text);

  delay(displayTime);

  if (resetAfter)
  {
    display.reset();
  }
}
