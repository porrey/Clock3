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
#include <AceButton.h>
#include <EEPROM-Storage.h>
#include "LedMatrix.h"
#include "ClockFont.h"
#include "GpsManager.h"
#include "BackgroundTone.h"
#include "TimeManager.h"
#include "BatteryMonitor.h"
#include "Mode.h"
#include "Strings.en-US.h"

using namespace ace_button;

// ***
// *** Comment/Uncomment this line to disable/enable debugging on the serial
// *** port. Connect RX from a TTL or FTDI cable to D7 (PD7). Note, this
// *** will use more program memory and mor dynamic memory.
// ***
#define DEBUG

// ***
// *** If debugging is enabled, The software serial port is created
// *** and the TRACE statements are mapped to Debug.print() and TRACELN
// *** are mapped to Debug.println().
// ***
#ifdef DEBUG
  #include <SoftwareSerial.h>
  
  // ***
  // *** Define the serial port for displaying debug messages. Note we specify
  // *** RX of -1 since we only send data and do not expect to receive any data.
  // ***
  SoftwareSerial Debug(-1, 7); // RX, TX
  
  #define TRACE(x) Debug.print(x)
  #define TRACELN(x) Debug.println(x)
  #define TRACE_DATE(l, d) TraceDateTime(l, d)
#else
  #define TRACE(x)
  #define TRACELN(x)
  #define TRACE_DATE
#endif

// ***
// *** Create variables to be stored in EEPROM. The first parameter is the
// *** address or location in EEPROM. The second parameter is the default
// *** value to return when the variable has not been initialized. Each
// *** variable requires enough bytes to hold the data type plus one additional
// *** byte for a checksum.
// ***
EEPROMStorage<int16_t> _tzOffset(0, 0);     // This variable is stored in EEPROM at positions 0, 1 and 2 (3 bytes).
EEPROMStorage<bool> _isDst(3, false);       // This variable is stored in EEPROM at positions 3 and 4 (2 bytes).
EEPROMStorage<bool> _chime(5, false);       // This variable is stored in EEPROM at positions 5 and 5 (2 bytes).

// ***
// *** Create an instance of the GpsManager.
// ***
GpsManager _gpsManager = GpsManager(&Serial);

// ***
// *** Create an instance of the TimeManager.
// ***
TimeManager _timeManager = TimeManager();

// ***
// *** Create an instance of the Clock LED Display Matrix that is part
// *** of the Spikenzielabs clock kit found at
// *** https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock.
// ***
// *** The instance is created using a refresh of 50hz.
// ***
LedMatrix _display = LedMatrix(50);

// ***
// *** Create an instance of Mode to track the clock mode.
// ***
Mode _mode = Mode(Mode::MODE_DISPLAY_TIME);

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

// ***
// *** Create an instance of battery monitor.
// ***
BatteryMonitor _batteryMonitor;

// ***
// *** AMount of time for delayed text.
// ***
#define DISPLAY_TEXT_DELAY    650

void setup()
{
#ifdef DEBUG
  // ***
  // *** Initialize the default serial port.
  // ***
  Debug.begin(57600);
  TRACELN(F("Serial has been initialized."));
#endif

  // ***
  // *** Initialize the time manager.
  // ***
  _timeManager.begin(_tzOffset, _isDst, timeEvent);
  TRACE(F("TZ Offset: ")); TRACELN(_timeManager.getTimeZoneOffset());
  TRACE(F("DST: ")); TRACELN(_timeManager.getIsDst() ? F("Yes") : F("No"));
  TRACE_DATE(F("UTC Date/Time from RTC: "), _timeManager.getUtcDateTime());

  // ***
  // *** Initialize the LED matrix.
  // ***
  _display.begin(&ClockFont);
  TRACELN(F("The display has been initialized."));

  // ***
  // *** Initialize the button pins.
  // ***
  pinMode(MODE_BUTTON, INPUT);
  pinMode(SETUP_BUTTON, INPUT_PULLUP);
  TRACELN(F("Button pins have been initialized."));

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
  TRACELN(F("Button have been initialized."));

  // ***
  // *** Set up the timer for display refresh. Use the recommended
  // *** values based on refresh mode.
  // ***
  Timer1.initialize(_display.getRefreshDelay());
  Timer1.attachInterrupt(refreshDisplay);
  TRACELN(F("Timer1 has been initialized."));

  // ***
  // *** Power on display test.
  // ***
  _display.powerOnDisplayTest();

  // ***
  // *** Initialize the battery monitor and display
  // *** the current voltage.
  // ***
  _batteryMonitor.begin(A0);
  TRACE(F("GPS Battery = ")); TRACE(_batteryMonitor.voltage()); TRACELN(F("v"));

  // ***
  // *** Set up the background tone generator.
  // ***
  _tone.begin(SETUP_BUTTON, backgroundToneEvent);
  TRACELN(F("Sound has been initialized."));

  // ***
  // *** Initialize the Serial port and GPS Manager.
  // ***
  Serial.begin(9600);
  _gpsManager.begin(gpsEvent);

  // ***
  // *** Show version number.ngs.en-US
  // ***
  _display.drawMomentaryTextCentered(STRING_DISPLAY_VERSION, DISPLAY_TEXT_DELAY * 3, true);

  // ***
  // *** Display a message indicating that setup has completed.
  // ***
  TRACELN(F("Setup completed."));
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
  switch (_mode.getMode())
  {
    // ***
    // *** Display the time.
    // ***
    case Mode::MODE_DISPLAY_TIME:
      {
        if (_mode.anyChanged())
        {
          // ***
          // *** Update the displayed time. Format the date and
          // *** time as time only.
          // ***
          char buffer[5];
          sprintf(buffer, FORMAT_TIME, _timeManager.localHour(), _timeManager.localMinute());
          _display.drawTextCentered(buffer);
          TRACE(F("Display Time: ")); TRACELN(buffer);

          // ***
          // *** Update the AM/PM mark on the display.
          // *** Highlight the LED at x = 18 and y = 5
          // *** when the time is PM.
          // ***
          _display.drawPixel(18, 5, _timeManager.isPm() ? 1 : 0);
          TRACE(F("AM/PM => ")); TRACELN(_timeManager.isPm() ? F("PM") : F("AM"));

          // ***
          // *** Update the GPS fix mark on the display.
          // *** Highlight the LED at x = 18 and y = 5
          // *** when the time is PM.
          // ***
          _display.drawPixel(18, 1, gps.hasFix() ? 1 : 0);
        }
      }
      break;
    case Mode::MODE_TZ:
      {
        // ***
        // *** Display the current offset.
        // ***
        if (_mode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_TZ, DISPLAY_TEXT_DELAY, true);
        }

        if (_mode.anyChanged())
        {
          displayTzOffset(_display, _tzOffset);
        }
      }
      break;
    case Mode::MODE_DST:
      {
        // ***
        // *** Display the current DST mode.
        // ***
        if (_mode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_DST, DISPLAY_TEXT_DELAY, true);
        }

        if (_mode.anyChanged())
        {
          displayBoolean(_display, _isDst);
        }
      }
      break;
    case Mode::MODE_CHIME:
      {
        if (_mode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_CHIME, DISPLAY_TEXT_DELAY, true);
        }

        if (_mode.anyChanged())
        {
          displayBoolean(_display, _chime);
        }
      }
      break;
    case Mode::MODE_BATTERY:
      {
        if (_mode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_BATTERY, DISPLAY_TEXT_DELAY, true);
        }

        if (_mode.anyChanged())
        {
          float voltage = _batteryMonitor.voltage();
          displayVoltage(_display, voltage);
        }
      }
      break;
  }

  // ***
  // *** Reset mode and setup change flags.
  // ***
  _mode.reset();

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
        TRACELN(F("Track started. Buttons are temporarily disabled."));
      }
      break;
    case BackgroundTone::SEQUENCE_COMPLETED:
      {
        // ***
        // *** Recconect the buttons.
        // ***
        pinMode(SETUP_BUTTON, INPUT_PULLUP);
        TRACELN(F("Track completed. Restored buttons."));
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
        // ***
        // *** Not used.
        // ***
        TRACELN(F("GPS has been initialized."));
      }
      break;
    case GpsManager::GPS_FIX_CHANGED:
      {
        // ***
        // *** Update the display only when we are in the display
        // *** time mode.
        // ***
        if (_mode.getMode() == Mode::MODE_DISPLAY_TIME)
        {
          _mode.modeChanged(true);
          TRACE(F("GPS Fix has changed: ")); TRACELN(_gpsManager.hasFix() ? F("Yes") : F("No"));
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
        TRACELN(F("The time manager has been initialized."));
      }
      break;
    case TimeManager::TIME_NO_RTC:
      {
        TRACELN(F("The timer manager could not find the RTC."));
      }
      break;
    case TimeManager::TIME_MINUTE_CHANGED:
      {
        TRACELN(F("Minute changed."));

        // ***
        // *** Check the current minute.
        // ***
        if (_timeManager.getUtcDateTime().minute() == 0)
        {
          if (_chime && _mode.getMode() == Mode::MODE_DISPLAY_TIME)
          {
            // ***
            // *** Clear the display.
            // ***
            _display.clear();

            // ***
            // *** The top of every hour.
            // ***
            _tone.play(BackgroundTone::CHIME);
            TRACELN(F("Playing chime."));
          }
        }
        else if (_timeManager.getUtcDateTime().minute() == 30)
        {
          // ***
          // *** Every hour at 15 minutes past the hour,
          // *** update the RTC from the GPS.
          // ***
          if (_gpsManager.hasFix())
          {
            TRACELN(F("Updating RTC from GPS."));
            TRACE_DATE("UTC Date/Time from RTC: ", _timeManager.getUtcDateTime());
            TRACE_DATE("UTC Date/Time from GPS: ", _gpsManager.dateTime());
            _timeManager.setUtcDateTime(_gpsManager.dateTime());
          }
        }

        // ***
        // *** Trigger a time update for the display.
        // ***
        _mode.setupChanged(true);
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
              _mode.modeChanged(true);
              _display.drawMomentaryTextCentered(STRING_DISPLAY_GPS, DISPLAY_TEXT_DELAY * 2, true);

              // ***
              // *** Update only when there is a GPS fix.
              // ***
              if (_gpsManager.hasFix())
              {
                TRACELN(F("Updating RTC from GPS."));
                TRACE_DATE("UTC Date/Time from RTC: ", _timeManager.getUtcDateTime());
                TRACE_DATE("UTC Date/Time from GPS: ", _gpsManager.dateTime());
                _timeManager.setUtcDateTime(_gpsManager.dateTime());
              }
              else
              {
                _display.drawMomentaryTextCentered(STRING_DISPLAY_NO_FIX, DISPLAY_TEXT_DELAY * 2, true);
                TRACELN(F("RTC could not be updated from the GPS; no fix."));
              }
            }
            break;
          case AceButton::kEventReleased:
            {
              // ***
              // *** Increment the mode and set the mode
              // *** changed flag.
              // ***
              _mode.increment();
              //_modeChanged = true;
              TRACE(F("Mode = ")); TRACELN(_mode.getMode());
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
              TRACELN(F("Setup button was long pressed."));
            }
            break;
          case AceButton::kEventReleased:
          case AceButton::kEventRepeatPressed:
            {
              switch (_mode.getMode())
              {
                case Mode::MODE_TZ:
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
                    _mode.setupChanged(true);

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    TRACE(F("Changed TZ to ")); TRACELN(_tzOffset);
                  }
                  break;
                case Mode::MODE_DST:
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
                    _mode.setupChanged(true);

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    TRACE(F("Changed DST to ")); TRACELN(_isDst ? "Yes" : "No");
                  }
                  break;
                case Mode::MODE_CHIME:
                  {
                    // ***
                    // *** Toggle the chime flag.
                    // ***
                    _chime = !_chime;

                    // ***
                    // *** Trigger setup change.
                    // ***
                    _mode.setupChanged(true);

                    // ***
                    // *** Write the new value to the serial port.
                    // ***
                    TRACE(F("Changed Chime to ")); TRACELN(_chime ? "Yes" : "No");
                  }
                  break;
                case Mode::MODE_BATTERY:
                  {
                    _mode.setupChanged(true);
                    TRACELN("Battery");
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
  // *** If the screen is refreshed while the tone is
  // *** playing, the tone will not play smoothly. Hopefully,
  // *** I will find a better way to handle this...
  // ***
  if (!_tone.isPlaying())
  {
    // ***
    // *** Refresh the LED matrix.
    // ***
    _display.refresh();
  }
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
  sprintf(str, FORMAT_VOLTAGE, buffer);

  // ***
  // *** Display the string.
  // ***
  display.drawTextCentered(str);
}

void displayTzOffset(const LedMatrix& display, int16_t value)
{
  // ***
  // *** Format the string for display.
  // ***
  char buffer[3];
  sprintf(buffer, FORMAT_NUMBER, value);

  // ***
  // *** Display the string.
  // ***
  display.drawTextCentered(buffer);
}

void displayBoolean(const LedMatrix& display, bool value)
{
  char buffer[3];
  sprintf(buffer, FORMAT_STRING, value ? STRING_DISPLAY_YES : STRING_DISPLAY_NO);

  // ***
  // *** Update the time on the display.
  // ***
  display.drawTextCentered(buffer);
}

#ifdef DEBUG
void TraceDateTime(String label, const DateTime& dt)
{
  char buffer[] = "MM-DD-YYYY hh:mm:ss";
  TRACE(label); TRACELN(dt.toString(buffer));
}
#endif
