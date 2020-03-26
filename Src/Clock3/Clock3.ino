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

// ******************************************************************
// ***
// *** Required fuses:
// ***
// *** Low:       0xE2
// *** High:      0xDA
// *** Extended:  0xFD (or 0x05)
// ***
// *** -U lfuse:w:0xe2:m -U hfuse:w:0xda:m -U efuse:w:0xfd:m
// ***
// ******************************************************************

#include "ClockMatrix.h"
#include "ClockFont.h"
#include <TimerOne.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <AceButton.h>
#include "Gps.h"
#include "BackgroundTone.h"
#include <EEPROM-Storage.h>

using namespace ace_button;

// ***
// *** Create variables to be stored in EEPROM. The first parameter is the
// *** address or location in EEPROM. The second parameter is the default
// *** value to return when the variable has not been initialized. Each
// *** variable requires enough bytes to hold the data type plus one additional
// *** byte for a checksum.
// ***
EEPROMStorage<int16_t> _tz_offset(0, 0);    // This variable is stored in EEPROM at positions 0, 1 and 2 (3 bytes).
EEPROMStorage<bool> _isDst(3, 0);           // This variable is stored in EEPROM at positions 3 and 4 (2 bytes).

// ***
// *** Define the serial port for displaying debug messages. When debugging
// *** on an Arduino, this should be the standard Serial port. Note we specify
// *** RX of -1 since we only send data and do not expect to receive any data.
// ***
SoftwareSerial Debug(-1, 7); // RX, TX

// ***
// *** Define the Serial port used by the GPS. For the clock this should be the standard
// *** serial port. If debugging on an Arduino, this should use the soft serial port. When
// *** debugging on an UNO, only digital ports 2 and 3 will work for the SoftwareSerial
// *** as long as the buttons are not connected (the mode and setup buttons use these pins).
// ***
#define GpsSerial Serial

// ***
// *** An instance of the TinyGPS interface.
// ***
TinyGPS _gps;

// ***
// *** Create an instance of the Clock LED Display Matrix that is part
// *** of the Spikenzielabs clock kit found at
// *** https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock.
// ***
ClockLedMatrix _display = ClockLedMatrix();

// ***
// *** An instance of the the RTC_DS1307 library. The RTC in the Spikenzielabs clock
// *** is a DS1337 but the interface for DS1307 works just as well.
// ***
RTC_DS1307 _rtc;

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
// *** Mode parameters.
// ***
bool _modeChanged = true;
uint8_t _mode = 0;

#define MODE_DISPLAY_TIME       0
#define MODE_TZ                 1
#define MODE_DST                2
#define MODE_BATTERY            3
#define MODE_MAX                4
#define MODE_DISPLAY_DELAY    650

// ***
// *** Indicates/triggers a setup value change.
// ***
bool _setupChanged = false;

// ***
// *** Define the IDs for the the mode and setup buttons.
// ***
#define BUTTON_ID_MODE 0
#define BUTTON_ID_SETUP 1

// ***
// *** Create button objects for the mode and setup buttons.
// ***
AceButton _buttons[2];

// ***
// *** The built-in speaker is on the same pin as the setup button.
// ***
BackgroundTone _tone = BackgroundTone();

void setup()
{
  // ***
  // *** Initialize the default serial port.
  // ***
  Debug.begin(57600);
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
  _display.setFont(&ClockFont);
  Debug.println(F("The display has been initialized."));

  // ***
  // *** Initialize the GPS.
  // ***
  GpsSerial.begin(9600);

  GpsSerial.println(PMTK_SET_BAUD_9600); delay(250);
  GpsSerial.println(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); delay(250);
  GpsSerial.println(PMTK_API_SET_FIX_CTL_1HZ); delay(250);
  GpsSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA); delay(250);
  GpsSerial.println(PMTK_ENABLE_WAAS); delay(250);
  GpsSerial.println(PGCMD_ANTENNA); delay(250);

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
    debugDisplayDateTime(F("UTC Date/Time from RTC: "), _rtc.now());
  }

  // ***
  // *** Initialize the button pins.
  // ***
  pinMode(MODE_BUTTON, INPUT);
  pinMode(SETUP_BUTTON, INPUT_PULLUP);

  // ***
  // *** Set up the timer for display refresh. Use the recommended
  // *** values based on refresh mode.
  // ***
  Timer1.initialize(_display.getRefreshDelay());
  Timer1.attachInterrupt(refreshDisplay);
  Debug.println(F("Timer1 has been initialized."));

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
  _buttons[BUTTON_ID_MODE].init(buttonConfig, MODE_BUTTON, HIGH, BUTTON_ID_MODE);
  _buttons[BUTTON_ID_SETUP].init(buttonConfig, SETUP_BUTTON, HIGH, BUTTON_ID_SETUP);

  // ***
  // *** Power on display test.
  // ***
  powerOnDisplayTest(_display);

  // ***
  // *** Show version number.
  // ***
  drawMomentaryTextCentered(_display, F("clk 3"), 2500, true);

  // ***
  // *** This will cause the battery voltage to
  // *** be written to the debug serial port.
  // ***
  batteryVoltage();

  // ***
  // *** Set up the background tone generator.
  // ***
  _tone.begin(SETUP_BUTTON, backgroundToneEvent);
  _tone.play(BackgroundTone::STARTUP);

  // ***
  // *** Display a message indicating that setup has completed.
  // ***
  Debug.println(F("Setup completed."));
  Debug.println(F(""));

  delay(1000);
}

// ***
// *** yield() is defined in th Arduino core and is called
// *** by various parts of the code such as delay(). This allows
// *** code to be run while other code is aiting for something to
// *** prevent valuable CPU cycls from being wasted. See hooks.c.
// ***
void yield()
{
  // ***
  // *** Keep the background tone generator rolling...
  // ***
  _tone.tick();

  // ***
  // *** If there is a tone playinng, the serial port will interfere
  // *** with it and the buttons do not need to work.
  // ***
  if (!_tone.isPlaying())
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
    _buttons[BUTTON_ID_MODE].check();
    _buttons[BUTTON_ID_SETUP].check();
  }
}

void loop()
{
  // ***
  // *** Check to see if it is time to update the RTC time
  // *** from the GPS data. We will update every hour
  // *** (3600 seconds).
  // ***
  if (!_tone.isPlaying())
  {
    if (updateRtcFromGps(_lastGpsUpdate, _gps, _rtc, 3600))
    {
      _lastGpsUpdate = _rtc.now();
      _gpsFix = true;
      Debug.println(F("GPS Fix = Yes"));
    }
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
        updateGpsFixDisplay(_display, _gpsFix);
      }
      break;
    case MODE_TZ:
      {
        // ***
        // *** Display the current offset.
        // ***
        if (_modeChanged)
        {
          drawMomentaryTextCentered(_display, F("TZ"), MODE_DISPLAY_DELAY, true);
        }

        if (_modeChanged || _setupChanged)
        {
          displayTzOffset(_display, _tz_offset);
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
          drawMomentaryTextCentered(_display, F("DST"), MODE_DISPLAY_DELAY, true);
        }

        if (_modeChanged || _setupChanged)
        {
          displayBoolean(_display, _isDst);
        }
      }
      break;
    case MODE_BATTERY:
      {
        if (_modeChanged)
        {
          drawMomentaryTextCentered(_display, F("Bat V"), MODE_DISPLAY_DELAY, true);
        }

        if (_modeChanged || _setupChanged)
        {
          float voltage = batteryVoltage();
          displayVoltage(_display, voltage);
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
  // *** Use delay to allow some backround processing.
  // ***
  delay(250);
}

void backgroundToneEvent(BackgroundTone::SEQUENCE_EVENT_ID eventId)
{
  switch (eventId)
  {
    case BackgroundTone::SEQUENCE_STARTED:
      Debug.println(F("Track started. Buttons are temporarily disabled."));
      break;
    case BackgroundTone::SEQUENCE_COMPLETED:
      Debug.println(F("Track completed. Restoring buttons."));
      pinMode(SETUP_BUTTON, INPUT_PULLUP);
      break;
  }
}

void buttonEventHandler(AceButton * button, uint8_t eventType, uint8_t state)
{
  // ***
  // *** Get the ID of the button that was pressed.
  // ***
  uint8_t id = button->getId();

  switch (id)
  {
    case BUTTON_ID_MODE:
      {
        switch (eventType)
        {
          case AceButton::kEventLongPressed:
            {
              // ***
              // *** A long press of the mode button will cause the time
              // *** to update from the GPS.
              // ***
              drawMomentaryTextCentered(_display, F("GPS"), 1500, true);
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
              _mode = (++_mode) % MODE_MAX;
              _modeChanged = true;
              Debug.print(F("Mode = ")); Debug.println(_mode);
            }
            break;
        }
      }
      break;
    case BUTTON_ID_SETUP:
      {
        switch (eventType)
        {
          case AceButton::kEventLongPressed:
            {
              Debug.println(F("NOT USED YET"));
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

                    // ***
                    // *** Trigger setup change.
                    // ***
                    _setupChanged = true;

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    Debug.print(F("Changed TZ to ")); Debug.println(_tz_offset);
                  }
                  break;
                case MODE_DST:
                  {
                    // ***
                    // *** Toggle the DST flag.
                    // ***
                    _isDst = !_isDst;

                    // ***
                    // *** Trigger setup change.
                    // ***
                    _setupChanged = true;

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    Debug.print(F("Changed DST to ")); Debug.println(_isDst ? "Yes" : "No");
                  }
                  break;
                case MODE_BATTERY:
                  {
                    _setupChanged = true;
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

bool updateTimeDisplay(const ClockLedMatrix & display, const RTC_DS1307 & rtc, int16_t lastMinuteDisplayed, int16_t tz_offset, bool isDst, bool force)
{
  bool returnValue = false;

  DateTime now = rtc.now();

  if (force | lastMinuteDisplayed != now.minute())
  {
    debugDisplayDateTime("UTC Date/Time from RTC: ", now);

    // ***
    // *** Clear the display.
    // ***
    display.clear();

    // ***
    // *** Adjust for the current time zone.
    // ***
    Debug.print(F("Time Zone Offset: ")); Debug.println(tz_offset);
    Debug.print(F("DST: ")); Debug.println(isDst ? F("Yes") : F("No"));
    DateTime localNow = DateTime(now.unixtime() + (tz_offset * 3600) + (isDst ? 3600 : 0));
    debugDisplayDateTime("Local Date/Time from RTC: ", localNow);

    // ***
    // *** Get a string version of the time.
    // ***
    String time = dateTimeToTimeString(&localNow);
    Debug.print(F("Time Display: ")); Debug.println(time);

    // ***
    // *** Update the time on the display.
    // ***
    display.drawTextCentered(time);

    // ***
    // *** Update the AM/PM mark on the display.
    // ***
    updateAmPmDisplay(_display, &localNow);

    Debug.println(F(""));

    returnValue = true;
  }

  return returnValue;
}

void updateGpsFixDisplay(const ClockLedMatrix & display, bool gpsHasTime)
{
  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  display.drawPixel(18, 1, gpsHasTime ? 1 : 0);
}

void updateAmPmDisplay(const ClockLedMatrix & display, DateTime * now)
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
  display.drawPixel(18, 5, pm ? 1 : 0);
}

bool updateRtcFromGps(DateTime lastGpsUpdate, const TinyGPS & gps, const RTC_DS1307 & rtc, uint64_t updateInterval)
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

// ***
// *** Called by the timer.
// ***
void refreshDisplay()
{
  // ***
  // *** Refresh the LED matrix.
  // ***
  _display.refresh();
}

void powerOnDisplayTest(const ClockLedMatrix & display)
{
  // ***
  // *** Light each LED.
  // ***
  for (uint8_t x = 0; x < display.width(); x++)
  {
    for (uint8_t y = 0; y < display.height(); y++)
    {
      display.drawPixel(x, y, 1);
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
  display.clear();
}

bool setDateAndTimeFromGps(const TinyGPS & gps, const RTC_DS1307 & rtc)
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
    debugDisplayDateTime(F("UTC Date/Time from GPS: "), dt);

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

String dateTimeToTimeString(DateTime * now)
{
  uint8_t hour = now->hour() > 12 ? now->hour() - 12 : now->hour();
  char buffer[5];
  sprintf(buffer, "%01d:%02d", hour, now->minute());
  return String(buffer);
}

void displayVoltage(const ClockLedMatrix & display, float value)
{
  // ***
  // *** Convert the float value to a string.
  // ***
  char buffer[3];
  dtostrf(value, 3, 1, buffer);

  // ***
  // *** Format the string for display.
  // ***
  char str[3];
  sprintf(str, "%sv", buffer);

  // ***
  // *** Display the string.
  // ***
  display.drawTextCentered(String(str));
}

void displayTzOffset(const ClockLedMatrix & display, int16_t value)
{
  // ***
  // *** Format the string for display.
  // ***
  char buffer[3];
  sprintf(buffer, "%d", value);

  // ***
  // *** Display the string.
  // ***
  display.drawTextCentered(String(buffer));
}

void displayBoolean(const ClockLedMatrix & display, bool value)
{
  char buffer[3];
  sprintf(buffer, "%s", value ? "Yes" : "No");

  // ***
  // *** Update the time on the display.
  // ***
  display.drawTextCentered(String(buffer));
}

void drawMomentaryTextCentered(const ClockLedMatrix & display, String text, uint64_t displayTime, bool resetAfter)
{
  // ***
  // *** Draw the text centered.
  // ***
  display.drawTextCentered(text);

  // ***
  // *** Use delay to pause the text.
  // ***
  delay(displayTime);

  // ***
  // *** Clear the display if specified.
  // ***
  if (resetAfter)
  {
    display.clear();
  }
}

void debugDisplayDateTime(String label, DateTime dt)
{
  char buffer[] = "MM-DD-YYYY hh:mm:ss";
  Debug.print(label); Debug.println(dt.toString(buffer));
}

float batteryVoltage()
{
  float returnValue = 0.0;

  // ***
  // *** Get the GPS battery voltage.
  // ***
  uint16_t value = analogRead(A0);
  returnValue = 0.0049 * value;
  Debug.print("GPS Battery = "); Debug.print(returnValue); Debug.println("v");

  return returnValue;
}
