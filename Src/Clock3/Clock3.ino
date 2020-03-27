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

#include <TimerOne.h>
#include <SoftwareSerial.h>
#include <AceButton.h>
#include <EEPROM-Storage.h>
#include "LedMatrix.h"
#include "ClockFont.h"
#include "GpsManager.h"
#include "BackgroundTone.h"
#include "TimeManager.h"

using namespace ace_button;

// ***
// *** Create variables to be stored in EEPROM. The first parameter is the
// *** address or location in EEPROM. The second parameter is the default
// *** value to return when the variable has not been initialized. Each
// *** variable requires enough bytes to hold the data type plus one additional
// *** byte for a checksum.
// ***
EEPROMStorage<int16_t> _tzOffset(0, 0);   // This variable is stored in EEPROM at positions 0, 1 and 2 (3 bytes).
EEPROMStorage<bool> _isDst(3, 0);         // This variable is stored in EEPROM at positions 3 and 4 (2 bytes).

// ***
// *** Define the serial port for displaying debug messages. Note we specify
// *** RX of -1 since we only send data and do not expect to receive any data.
// ***
SoftwareSerial Debug(-1, 7); // RX, TX

// ***
// *** Create an instance of the GpsManager.
// ***
GpsManager _gpsManager = GpsManager(&Serial);

// ***
// *** Create an instance of the TimeManager.
// ***
TimeManager _timeManager;

// ***
// *** Create an instance of the Clock LED Display Matrix that is part
// *** of the Spikenzielabs clock kit found at
// *** https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock.
// ***
LedMatrix _display;

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
BackgroundTone _tone;

void setup()
{
  // ***
  // *** Initialize the default serial port.
  // ***
  Debug.begin(57600);
  Debug.println(F("Serial has been initialized."));

  // ***
  // *** Initialize the time manager.
  // ***
  _timeManager.begin(_tzOffset, _isDst, timeEvent);
  Debug.print(F("TZ Offset: ")); Debug.println(_timeManager.getTimeZoneOffset());
  Debug.print(F("DST: ")); Debug.println(_timeManager.getIsDst() ? F("Yes") : F("No"));
  debugDisplayDateTime(F("UTC Date/Time from RTC: "), _timeManager.getUtcDateTime());

  // ***
  // *** Initialize the LED matrix.
  // ***
  _display.begin(&ClockFont);
  Debug.println(F("The display has been initialized."));

  // ***
  // *** Initialize the button pins.
  // ***
  pinMode(MODE_BUTTON, INPUT);
  pinMode(SETUP_BUTTON, INPUT_PULLUP);
  Debug.println(F("Button pins have been initialized."));

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
  Debug.println(F("Button have been initialized."));

  // ***
  // *** Set up the timer for display refresh. Use the recommended
  // *** values based on refresh mode.
  // ***
  Timer1.initialize(_display.getRefreshDelay());
  Timer1.attachInterrupt(refreshDisplay);
  Debug.println(F("Timer1 has been initialized."));

  // ***
  // *** This will cause the battery voltage to
  // *** be written to the debug serial port.
  // ***
  batteryVoltage();

  // ***
  // *** Power on display test.
  // ***
  powerOnDisplayTest(_display);

  // ***
  // *** Show version number.
  // ***
  drawMomentaryTextCentered(_display, F("clk 3"), 2500, true);
  
  // ***
  // *** Set up the background tone generator.
  // ***
  _tone.begin(SETUP_BUTTON, backgroundToneEvent);
  _tone.play(BackgroundTone::STARTUP);
  Debug.println(F("Sound has been initialized."));

  // ***
  // *** Initialize the Serial port and GPS Manager.
  // ***
  Serial.begin(9600);
  _gpsManager.begin(gpsEvent);
  Debug.println(F("GPS has been initialized."));

  // ***
  // *** Display a message indicating that setup has completed.
  // ***
  Debug.println(F("Setup completed."));
}

// ***
// *** yield() is defined in the Arduino core and is called
// *** by various parts of the code such as delay(). This allows
// *** code to be run while other code is aiting for something to
// *** prevent valuable CPU cycles from being wasted. See hooks.c.
// ***
void yield()
{
  // ***
  // *** Keep the background tone generator rolling...
  // ***
  _tone.tick();

  // ***
  // *** Keep the time manager ticking...
  // ***
  _timeManager.tickTock();

  // ***
  // *** If there is a tone playing, the serial port will interfere
  // *** with it and the buttons do not need to work.
  // ***
  if (!_tone.isPlaying())
  {
    // ***
    // *** Process the GPS.
    // ***
    _gpsManager.process();

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
  // *** Check the current mode.
  // ***
  switch (_mode)
  {
    // ***
    // *** Display the time.
    // ***
    case MODE_DISPLAY_TIME:
      {
        if (_modeChanged || _setupChanged)
        {
          // ***
          // *** Update the displayed time.
          // ***
          updateTimeDisplay(_display);

          // ***
          // *** Update the AM/PM mark on the display.
          // ***
          updateAmPmDisplay(_display, _timeManager);

          // ***
          // *** Update the GPS fix mark on the display.
          // ***
          updateGpsFixDisplay(_display, _gpsManager);
        }
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
          displayTzOffset(_display, _tzOffset);
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
      {
        Debug.println(F("Track started. Buttons are temporarily disabled."));
      }
      break;
    case BackgroundTone::SEQUENCE_COMPLETED:
      {
        Debug.println(F("Track completed. Restoring buttons."));
        pinMode(SETUP_BUTTON, INPUT_PULLUP);
      }
      break;
  }
}

void gpsEvent(GpsManager::GPS_EVENT_ID eventId)
{
  switch (eventId)
  {
    case GpsManager::GPS_INITIALIZED:
      {
        Debug.println(F("GPS initialized."));
      }
      break;
    case GpsManager::GPS_FIX_CHANGED:
      {
        // ***
        // *** Update the display only when we are int he display
        // *** time mode.
        // ***
        if (_mode = MODE_DISPLAY_TIME)
        {
          // ***
          // *** Highlight the LED at x = 18 and y = 5
          // *** when the time is PM.
          // ***
          updateGpsFixDisplay(_display, _gpsManager);
          Debug.print(F("GPS Fix has changed: ")); Debug.println(_gpsManager.hasFix() ? F("Yes") : F("No"));
        }
      }
      break;
  }
}

void timeEvent(TimeManager::TIME_EVENT_ID eventId)
{
  switch (eventId)
  {
    case TimeManager::TIME_INITIALIZED:
      {
        Debug.println(F("The time manager has been initialized."));
      }
      break;
    case TimeManager::TIME_NO_RTC:
      {
        Debug.println(F("The timer manager could not find the RTC."));
      }
      break;
    case TimeManager::TIME_MINUTE_CHANGED:
      {
        Debug.println(F("Minute changed."));

        // ***
        // *** Check the current minute. Every hour at 15 minutes
        // *** past the hour, update the RTC from the GPS.
        // ***
        if (_timeManager.getUtcDateTime().minute() == 15)
        {
          if (_gpsManager.hasFix())
          {
            Debug.println(F("Updating RTC from GPS."));
            _timeManager.setUtcDateTime(_gpsManager.dateTime());
            debugDisplayDateTime("UTC Date/Time from GPS: ", _gpsManager.dateTime());
          }
        }

        // ***
        // *** Triggr a time update for the display.
        // ***
        _setupChanged = true;
      }
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
              _modeChanged = true;
              drawMomentaryTextCentered(_display, F("GPS"), 1500, true);

              if (_gpsManager.hasFix())
              {
                _timeManager.setUtcDateTime(_gpsManager.dateTime());
                Debug.println(F("RTC has been updated from the GPS."));
              }
              else
              {
                drawMomentaryTextCentered(_display, F("No Fix"), 1500, true);
                Debug.println(F("RTC could not be updated from the GPS; no fix."));
              }
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
                    _tzOffset++;

                    if (_tzOffset > 14)
                    {
                      _tzOffset = -14;
                    }

                    // ***
                    // *** Update the time manager.
                    // ***
                    _timeManager.setTimeZoneOffset(_tzOffset);

                    // ***
                    // *** Trigger setup change.
                    // ***
                    _setupChanged = true;

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    Debug.print(F("Changed TZ to ")); Debug.println(_tzOffset);
                  }
                  break;
                case MODE_DST:
                  {
                    // ***
                    // *** Toggle the DST flag.
                    // ***
                    _isDst = !_isDst;

                    // ***
                    // *** Update the time manager.
                    // ***
                    _timeManager.setIsDst(_isDst);

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

// ***
// *** Called by the Timer1 to refresh the display.
// ***
void refreshDisplay()
{
  // ***
  // *** Refresh the LED matrix.
  // ***
  _display.refresh();
}

void updateTimeDisplay(const LedMatrix& display)
{
  // ***
  // *** Clear the display.
  // ***
  display.clear();

  // ***
  // *** Format the date and time as time only.
  // ***
  char buffer[5];
  sprintf(buffer, "%01d:%02d", _timeManager.localHour(), _timeManager.localMinute());
  String time = String(buffer);

  // ***
  // *** Update the time on the display.
  // ***
  display.drawTextCentered(time);
  Debug.print(F("Display Time: ")); Debug.println(time);
}

void updateAmPmDisplay(const LedMatrix& display, const TimeManager& timeManager)
{
  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  display.drawPixel(18, 5, _timeManager.isPm() ? 1 : 0);
  Debug.print(F("AM/PM => ")); Debug.println(_timeManager.isPm() ? F("PM") : F("AM"));
}

void powerOnDisplayTest(const LedMatrix& display)
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

void displayVoltage(const LedMatrix& display, float value)
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

void updateGpsFixDisplay(const LedMatrix& display, const GpsManager& gps)
{
  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  display.drawPixel(18, 1, gps.hasFix() ? 1 : 0);
}

void displayTzOffset(const LedMatrix& display, int16_t value)
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

void displayBoolean(const LedMatrix& display, bool value)
{
  char buffer[3];
  sprintf(buffer, "%s", value ? "Yes" : "No");

  // ***
  // *** Update the time on the display.
  // ***
  display.drawTextCentered(String(buffer));
}

void drawMomentaryTextCentered(const LedMatrix& display, String text, uint64_t displayTime, bool resetAfter)
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

void debugDisplayDateTime(String label, const DateTime& dt)
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
  Debug.print(F("GPS Battery = ")); Debug.print(returnValue); Debug.println(F("v"));

  return returnValue;
}
