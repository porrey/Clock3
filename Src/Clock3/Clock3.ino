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
// *** Board:       Arduino Pro or Pro Min
// *** Processor:   ATmega328P (3.3V, 8MHz)
// ***
// *** Required fuses:
// ***
// ***  Low:       0xE2
// ***  High:      0xDA
// ***  Extended:  0xFD (or 0x05)
// ***
// ***  -U lfuse:w:0xe2:m -U hfuse:w:0xda:m -U efuse:w:0xfd:m
// ***
// ******************************************************************

// ***
// *** Comment/uncomment this line to disable/enable debugging on the serial
// *** port. Connect RX from a TTL or FTDI cable to D7 (PD7). Note, this
// *** will use more program memory and more dynamic memory.
// ***
//#define DEBUG

#include <TimerOne.h>
#include <AceButton.h>
#include <EEPROM-Storage.h>
#include "LedMatrix.h"
#include "Other\ClockFont.h"
#include "GpsManager.h"
#include "BackgroundTone.h"
#include "TimeManager.h"
#include "BatteryMonitor.h"
#include "Mode.h"
#include "Strings.en-US.h"

using namespace ace_button;

// ***
// *** Buttons
// ***
#define MODE_BUTTON 2
#define SETUP_BUTTON 3

// ***
// *** GPS battery analog pin
// ***
#define GPS_BATTERY_PIN A0

// ***
// *** If debugging is enabled, The software serial port is created
// *** and the TRACE statements are mapped to Debug.print() and TRACELN
// *** are mapped to Debug.println().
// ***
#ifdef DEBUG
#include <SoftwareSerial.h>
#include "Other\memory.h"

// ***
// *** Define the serial port for displaying debug messages. Note we specify
// *** RX of -1 since we only send data and do not expect to receive any data.
// ***
SoftwareSerial Debug(-1, 7); // RX, TX

#define TRACE(x) Debug.print(x)
#define TRACELN(x) Debug.println(x)
#define TRACE_DATE(l, d) TraceDateTime(l, d)
#define TRACE_DETAILS() traceTimeDetails()
#else
#define TRACE(x)
#define TRACELN(x)
#define TRACE_DATE(l, d)
#define TRACE_DETAILS()
#endif

// ***
// *** Create variables to be stored in EEPROM. The first parameter is the
// *** address or location in EEPROM. The second parameter is the default
// *** value to return when the variable has not been initialized. Each
// *** variable requires enough bytes to hold the data type plus one additional
// *** byte for a checksum.
// ***
EEPROMStorage<uint8_t> _timeZoneId(0, 0);                       // This variable is stored in EEPROM at positions 0 and 1 (2 bytes).
EEPROMStorage<DstMode_t> _dstMode(2, DstMode_t::AUTO);          // This variable is stored in EEPROM at positions 2 and 3 (2 bytes).
EEPROMStorage<bool> _chime(4, true);                            // This variable is stored in EEPROM at positions 4 and 5 (2 bytes).
EEPROMStorage<bool> _twelveHour(6, true);                       // This variable is stored in EEPROM at positions 6 and 7 (2 bytes).

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
// *** Create an instance of Mode to track the clock mode
// *** specifying MODE_DISPLAY_TIME as the default mode
// *** and a 15 second timeout.
// ***
Mode _clockMode = Mode(Mode_t::MODE_DISPLAY_TIME, 15);

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
// *** Create an instance of battery monitor. The ADC on the ATmega328P
// *** is 10-bit and the reference voltage is 5.0V.
// ***
BatteryMonitor _batteryMonitor(10, 5.0);

// ***
// *** AMount of time for delayed text.
// ***
#define DISPLAY_TEXT_DELAY 650

// ***
// *** setup() is called once when the microcontroller
// *** is first powered up.
// ***
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
  TRACE(F("Time Zone ID from EEPROM: ")); TRACELN(_timeZoneId);
  _timeManager.begin(_timeZoneId, _dstMode, _twelveHour, onTimeEvent);
  TRACE_DETAILS();

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
  // *** Configure the AceButtons using ButtonConfig by
  // *** assigning the event handler and setting the
  // *** required features.
  // ***
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(onButtonEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAll);

  // ***
  // *** Initialize the buttons.
  // ***
  _buttons[BUTTON_ID_MODE].init(MODE_BUTTON, HIGH, BUTTON_ID_MODE);
  _buttons[BUTTON_ID_SETUP].init(SETUP_BUTTON, HIGH, BUTTON_ID_SETUP);
  TRACELN(F("Button have been initialized."));

  // ***
  // *** Set up the timer for display refresh. Use the recommended
  // *** values based on refresh mode.
  // ***
  Timer1.initialize(_display.getRefreshDelay());
  Timer1.attachInterrupt(onRefreshDisplay);
  TRACELN(F("Timer1 has been initialized."));

  // ***
  // *** Power on display test. This will ensure all LEDs are
  // *** working and that the initialization of the display
  // *** driver is working.
  // ***
  _display.powerOnDisplayTest();

  // ***
  // *** Initialize the battery monitor and display
  // *** the current voltage.
  // ***
  _batteryMonitor.begin(GPS_BATTERY_PIN);
  TRACE(F("GPS Battery = ")); TRACE(_batteryMonitor.voltage()); TRACELN(F("v"));

  // ***
  // *** Set up the background tone generator.
  // ***
  _tone.begin(SETUP_BUTTON, onBackgroundToneEvent);
  TRACELN(F("Sound has been initialized."));

  // ***
  // *** Initialize the Serial port and GPS Manager.
  // ***
  Serial.begin(9600);
  _gpsManager.begin(onGpsEvent);

  // ***
  // *** Show version number.
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
// *** code to be run while other code is waiting. This will
// *** prevent valuable CPU cycles from being wasted. See hooks.c
// *** in the Arduino core. This is a very basic form of
// *** cooperative multitasking.
// ***
void yield()
{
  // ***
  // *** If there is a tone playing, the button events will
  // *** fire because the tone is played on the same pin as
  // *** the mode button.
  // ***
  if (!_tone.isPlaying())
  {
    // ***
    // *** Check the buttons to update their state.
    // ***
    _buttons[BUTTON_ID_MODE].check();
    _buttons[BUTTON_ID_SETUP].check();
  }

  // ***
  // *** Keep the background tone generator rolling...
  // ***
  _tone.process();

  // ***
  // *** Keep the time manager ticking...
  // ***
  _timeManager.process();

  // ***
  // *** Checks for mode timeout. Retuns true
  // *** whenever the mode is reset back to
  // *** the default mode.
  // ***
  if (_clockMode.process())
  {
    TRACELN(F("Mode changed to default."));
  }

  // ***
  // *** If there is a tone playing, the serial port will interfere
  // *** with the sound.
  // ***
  if (!_tone.isPlaying())
  {
    // ***
    // *** Process the GPS.
    // ***
    _gpsManager.process();
  }
}

// ***
// *** loop() is called by the Arduino internals again
// *** and again forever...
// ***
void loop()
{
  // ***
  // *** Check the current mode.
  // ***
  switch (_clockMode.mode())
  {
    // ***
    // *** Display the time.
    // ***
    case Mode_t::MODE_DISPLAY_TIME:
      {
        if (_clockMode.anyChanged())
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
          // *** Do not show the AM/PM or GPS Fix indicators
          // *** in 24-hour format.
          // ***
          if (_timeManager.displayTwelveHourFormat)
          {
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
            _display.drawPixel(18, 1, _gpsManager.hasFix() ? 1 : 0);
          }
        }
      }
      break;
    case Mode_t::MODE_TZ:
      {
        // ***
        // *** Display the current offset.
        // ***
        if (_clockMode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_TZ, DISPLAY_TEXT_DELAY, true);
        }

        if (_clockMode.anyChanged())
        {
          TimeZone_t* tz = TimeManager::getTimeZone(_timeManager.timeZoneId());

          if (_timeManager.isDst())
          {
            _display.drawTextCentered(tz->dName);
          }
          else
          {
            _display.drawTextCentered(tz->sName);
          }
        }
      }
      break;
    case Mode_t::MODE_DST:
      {
        // ***
        // *** Display the current DST mode.
        // ***
        if (_clockMode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_DST, DISPLAY_TEXT_DELAY, true);
        }

        if (_clockMode.anyChanged())
        {
          _display.drawTextCentered(_timeManager.dstLabel());
        }
      }
      break;
    case Mode_t::MODE_CHIME:
      {
        if (_clockMode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_CHIME, DISPLAY_TEXT_DELAY, true);
        }

        if (_clockMode.anyChanged())
        {
          displayBoolean(_display, _chime);
        }
      }
      break;
    case Mode_t::MODE_FORMAT:
      {
        if (_clockMode.modeChanged())
        {
          _display.drawMomentaryTextCentered(STRING_DISPLAY_FMT, DISPLAY_TEXT_DELAY, true);
        }

        if (_clockMode.anyChanged())
        {
          displayBoolean(_display, _twelveHour);
        }
      }
      break;
  }

  // ***
  // *** Reset mode and setup change flags.
  // ***
  _clockMode.reset();

  // ***
  // *** Use delay to allow some backround processing. Internally,
  // *** this calls yield();
  // ***
  delay(350);
}

// ***
// *** Event handler for the Background Tone.
// ***
void onBackgroundToneEvent(SequenceEventId_t eventId)
{
  switch (eventId)
  {
    case SequenceEventId_t::SEQUENCE_STARTED:
      {
        // ***
        // *** Reset the display.
        // ***
        _display.reset();

        TRACELN(F("Track started. Buttons are temporarily disabled."));
      }
      break;
    case SequenceEventId_t::SEQUENCE_COMPLETED:
      {
        // ***
        // *** Reconnect the setup button.
        // ***
        pinMode(SETUP_BUTTON, INPUT_PULLUP);
        TRACELN(F("Track completed. Restored buttons."));

        // ***
        // *** Force the display to dedraw.
        // ***
        _clockMode.setupChanged(true);
      }
      break;
  }
}

// ***
// *** Event handler for the GPS Manager.
// ***
void onGpsEvent(GpsEventId_t eventId)
{
  switch (eventId)
  {
    case GpsEventId_t::GPS_INITIALIZED:
      {
        // ***
        // *** Not used.
        // ***
        TRACELN(F("GPS has been initialized."));
      }
      break;
    case GpsEventId_t::GPS_FIX_CHANGED:
      {
        // ***
        // *** Update the display only when we are in the display
        // *** time mode.
        // ***
        if (_clockMode.mode() == Mode_t::MODE_DISPLAY_TIME)
        {
          _clockMode.modeChanged(true);
          TRACE_DETAILS();
        }
      }
      break;
  }
}

// ***
// *** Event handler for the Time Manager.
// ***
void onTimeEvent(TimeEventId_t eventId)
{
  switch (eventId)
  {
    case TimeEventId_t::TIME_INITIALIZED:
      {
        TRACELN(F("The time manager has been initialized."));
      }
      break;
    case TimeEventId_t::TIME_NO_RTC:
      {
        TRACELN(F("The timer manager could not find the RTC."));
      }
      break;
    case TimeEventId_t::TIME_ZONE_CHANGED:
      {
        TRACELN(F("Time zone changed."));
      }
      break;
    case TimeEventId_t::DST_MODE_CHANGED:
      {
        TRACELN(F("DST mode changed."));
      }
      break;
    case TimeEventId_t::TIME_MINUTE_CHANGED:
      {
        TRACELN(F("Minute changed."));

        // ***
        // *** Check the current minute.
        // ***
        if (_timeManager.utcDateTime().minute() == 0)
        {
          if (_chime)
          {
            if (_clockMode.mode() == Mode_t::MODE_DISPLAY_TIME)
            {
              // ***
              // *** The top of every hour.
              // ***
              _tone.play(Sequence_t::CHIME);
              TRACELN(F("Playing chime."));
            }
            else
            {
              TRACELN(F("Chime enabled; not in time display mode)."));
            }
          }
          else
          {
            TRACELN(F("Chime is disabled."));
          }
        }
        else if (_timeManager.utcDateTime().minute() == 30)
        {
          // ***
          // *** Every hour at 15 minutes past the hour,
          // *** update the RTC from the GPS.
          // ***
          if (_gpsManager.hasFix())
          {
            TRACELN(F("Updating RTC from GPS."));
            TRACE_DETAILS();
            _timeManager.utcDateTime(_gpsManager.dateTime());
          }
        }

        // ***
        // *** Trigger a time update for the display.
        // ***
        _clockMode.setupChanged(true);
        TRACE_DETAILS();
      }
      break;
  }
}

void onButtonEvent(AceButton* button, uint8_t eventType, uint8_t state)
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
          case AceButton::kEventPressed:
            {
              TRACELN(F("Mode button was pressed"));
            }
            break;
          case AceButton::kEventReleased:
            {
              TRACELN(F("Mode button was released"));
            }
            break;
          case AceButton::kEventLongPressed:
            {
              TRACELN(F("Mode button was long-pressed"));
              modeButtonLongPressed();
            }
            break;
          case AceButton::kEventClicked:
            {
              TRACELN(F("Mode button was clicked"));
              modeButtonClicked();
            }
            break;
          case AceButton::kEventRepeatPressed:
            {
              TRACELN(F("Mode button was repeat-pressed"));
            }
            break;
          case AceButton::kEventDoubleClicked:
            {
              TRACELN(F("Mode button was double-clicked"));
              modeButtonDoubleClicked();
            }
            break;
        }
      }
      break;
    case BUTTON_ID_SETUP:
      {
        switch (eventType)
        {
          case AceButton::kEventPressed:
            {
              TRACELN(F("Setup button was pressed"));
            }
            break;
          case AceButton::kEventReleased:
            {
              TRACELN(F("Setup button was released"));
            }
            break;
          case AceButton::kEventLongPressed:
            {
              TRACELN(F("Setup button was long pressed."));
            }
            break;
          case AceButton::kEventClicked:
            {
              TRACELN(F("Setup button was clicked"));
              setupButtonClicked();
            }
            break;
          case AceButton::kEventRepeatPressed:
            {
              TRACELN(F("Setup button was repeat-pressed"));
            }
            break;
          case AceButton::kEventDoubleClicked:
            {
              TRACELN(F("Setup button was double-clicked"));
              setupButtonDoubleClicked();
            }
            break;
        }
      }
      break;
  }
}

void modeButtonLongPressed()
{
  // ***
  // *** A long press of the mode button will cause the time
  // *** to update from the GPS.
  // ***
  _display.drawMomentaryTextCentered(STRING_DISPLAY_GPS, DISPLAY_TEXT_DELAY * 2, true);

  // ***
  // *** Update only when there is a GPS fix.
  // ***
  if (_gpsManager.hasFix())
  {
    TRACELN(F("Updating RTC from GPS."));
    TRACE_DETAILS();
    _timeManager.utcDateTime(_gpsManager.dateTime());
  }
  else
  {
    TRACELN(F("RTC could not be updated from the GPS; no fix."));
    _display.drawMomentaryTextCentered(STRING_DISPLAY_NO_FIX, DISPLAY_TEXT_DELAY * 2, true);
  }

  // ***
  // *** Force an update of the time display.
  // ***
  _clockMode.modeChanged(true);
}

void modeButtonClicked()
{
  // ***
  // *** Increment the mode and set the mode
  // *** changed flag.
  // ***
  _clockMode.increment();
  TRACE(F("Mode = ")); TRACELN(_clockMode.mode());
}

void setupButtonClicked()
{
  switch (_clockMode.mode())
  {
    case Mode_t::MODE_TZ:
      {
        // ***
        // *** Increment the TZ offset
        // *** by 60 minutes.
        // ***
        _timeZoneId++;

        if (_timeZoneId > TimeManager::timeZoneCount())
        {
          _timeZoneId = 0;
        }

        // ***
        // *** Update the time manager.
        // ***
        _timeManager.timeZoneId(_timeZoneId);

        // ***
        // *** Trigger setup change.
        // ***
        _clockMode.setupChanged(true);

        // ***
        // *** Write the new value to the serial port.
        // ***
        TRACE(F("Changed Time Zone ID to ")); TRACELN(_timeZoneId);
      }
      break;
    case Mode_t::MODE_DST:
      {
        // ***
        // *** Toggle the DST mode.
        // ***
        _dstMode = _timeManager.toggleDstMode();

        // ***
        // *** Trigger setup change.
        // ***
        _clockMode.setupChanged(true);

        // ***
        // *** Write the new value to the serial port.
        // ***
        TRACE(F("Changed DST Mode to ")); TRACELN(_timeManager.dstLabel());
      }
      break;
    case Mode_t::MODE_CHIME:
      {
        // ***
        // *** Toggle the chime flag.
        // ***
        _chime = !_chime;

        // ***
        // *** Trigger setup change.
        // ***
        _clockMode.setupChanged(true);

        // ***
        // *** Write the new value to the serial port.
        // ***
        TRACE(F("Changed Chime to ")); TRACELN(_chime ? "Yes" : "No");
      }
      break;
    case Mode_t::MODE_FORMAT:
      {
        // ***
        // *** Toggle the twelve hour format flag.
        // ***
        _twelveHour = !_twelveHour;

        // ***
        // *** Update the time manager.
        // ***
        // ***
        _timeManager.displayTwelveHourFormat = _twelveHour;

        // ***
        // *** Trigger setup change.
        // ***
        _clockMode.setupChanged(true);

        // ***
        // *** Write the new value to the serial port.
        // ***
        TRACE(F("Changed twelve hour to ")); TRACELN(_twelveHour ? "Yes" : "No");
      }
      break;
  }
}

void setupButtonDoubleClicked()
{
  // ***
  // *** Only allowed when in display time mode.
  // ***
  if (_clockMode.mode() == Mode_t::MODE_DISPLAY_TIME)
  {
    _display.drawMomentaryTextCentered(STRING_DISPLAY_BATTERY, DISPLAY_TEXT_DELAY, true);

    // ***
    // *** Get the GPS backup battery voltage.
    // ***
    float voltage = _batteryMonitor.voltage();

    // ***
    // *** Convert the float value to a string.
    // ***
    char buffer[4];
    dtostrf(voltage, 3, 1, buffer);

    // ***
    // *** Format the string for display.
    // ***
    char str[5];
    sprintf(str, FORMAT_VOLTAGE, buffer);

    // ***
    // *** Display the string.
    // ***
    _display.drawMomentaryTextCentered(str, DISPLAY_TEXT_DELAY * 3, true);

    // ***
    // *** Force a redraw.
    // ***
    _clockMode.setupChanged(true);
  }
}

void modeButtonDoubleClicked()
{
  // ***
  // *** Only allowed when in display time mode.
  // ***
  if (_clockMode.mode() == Mode_t::MODE_DISPLAY_TIME)
  {
    _tone.play(Sequence_t::CHIME);
  }
}

// ***
// *** Called by the Timer1 to refresh the display.
// ***
void onRefreshDisplay()
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

// ***
// *** Displays the time zone offset centered on
// *** the display.
// ***
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

// ***
// *** Displays a boolean value using the strings 'Yes' and
// *** 'No'. The text is centered on the display.
// ***
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
// ***
// *** Used to display a DateTime value when debugging.
// ***
void TraceDateTime(String label, const DateTime& dt)
{
  char buffer[] = "MM-DD-YYYY hh:mm:ss";
  TRACE(label); TRACELN(dt.toString(buffer));
}

void traceTimeDetails()
{
  TRACELN();
  TRACE(F("Time Zone [")); TRACE(_timeManager.timeZoneId()); TRACE(F("]: ")); TRACELN(_timeManager.timeZoneName());
  TRACE(F("TZ Offset: ")); TRACELN(_timeManager.timeOffset());
  TRACE(F("DST Mode: ")); TRACELN(_timeManager.dstLabel());
  TRACE(F("DST: ")); TRACELN(_timeManager.isDst() ? F("Yes") : F("No") );
  TRACE_DATE(F("UTC Dt/Tm [RTC]: "), _timeManager.utcDateTime());
  TRACE_DATE(F("Local Dt/Tm [RTC]: "), _timeManager.localDateTime());
  TRACE(F("GPS Fix: ")); TRACELN(_gpsManager.hasFix() ? F("Yes") : F("No"));
  TRACE_DATE("UTC Dt/Tm [GPS]: ", _gpsManager.dateTime());
  TRACE(F("Free memory = ")); TRACELN(freeMemory());
  TRACELN();
}
#endif
