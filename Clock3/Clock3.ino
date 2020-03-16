#include "Tests.h"
#include "ClockMatrix.h"
#include "ClockFont.h"
#include "TimerOne.h"
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <AceButton.h>
using namespace ace_button;

// ***
// *** Initialization strings for the GPS.
// ***
#define PMTK_SET_BAUD_9600 F("$PMTK251,9600*17")
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  F("$PMTK220,5000*1B")
#define PMTK_API_SET_FIX_CTL_1HZ  F("$PMTK300,1000,0,0,0,0*1C")
#define PMTK_SET_NMEA_OUTPUT_RMCGGA F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28")
#define PMTK_ENABLE_WAAS F("$PMTK301,2*2E")
#define PGCMD_ANTENNA F("$PGCMD,33,1*6C")
#define PGCMD_NOANTENNA F("$PGCMD,33,0*6D")

// ***
// *** Define the serial port for displaying debug messages. When debugging
// *** on an Arduino, this should be the standard Serial port. Note we specify
// *** RX of 0 since we only send data and do not expect to receive any data.
// ***
SoftwareSerial Debug(0, 7); // RX, TX
//#define Debug Serial

// ***
// *** Define the Serial port used by the GPS. For the clock this should be the standard
// *** serial port. If debugging on an Arduino, this should use the soft serial port.
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
int8_t _tz_offset = -5;
//uint8_t _tz_change_direction = -1;

// ***
// ***
// ***
bool _isDst = false;

// ***
// *** Tracks the last minute displayed so the display can be updated once per minute.
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
bool _modeChanged = false;
uint8_t _mode = 0;

#define MODE_DISPLAY_TIME 0
#define MODE_TZ 1
#define MODE_DST 2

// ***
// *** Setup
// ***
bool _setupChanged = false;

// ***
// *** Create a button object for the mode button.
// ***
#define MODE_BUTTON_ID 0
#define SETUP_BUTTON_ID 1
//AceButton _buttons[2];

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
  //pinMode(MODE_BUTTON, INPUT_PULLUP);
  //digitalWrite(MODE_BUTTON, HIGH);
  //pinMode(SETUP_BUTTON, INPUT_PULLUP);
  //digitalWrite(SETUP_BUTTON, HIGH);

  // ***
  // *** Initialize the buttons.
  // ***
  //_buttons[MODE_BUTTON_ID].init(MODE_BUTTON, HIGH, MODE_BUTTON_ID);
  //_buttons[SETUP_BUTTON_ID].init(SETUP_BUTTON, HIGH, SETUP_BUTTON_ID);

  // ***
  // *** Configure the ButtonConfig with the event handler.
  // ***
  //ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  //buttonConfig->setEventHandler(buttonEventHandler);
  //buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  //buttonConfig->setFeature(ButtonConfig::kFeatureClick);

  // ***
  // *** Power on test.
  // ***
  powerOnTest(_gps);

  // ***
  // *** Show version number.
  // ***
  drawMomentaryTextCentered(_display, "ver 3", 2000, true);

  Debug.println(F("Setup completed."));
  Debug.println(F(""));
}

void buttonEventHandler(AceButton* button, uint8_t eventType, uint8_t state)
{
  uint8_t id = button->getId();

  if (eventType == ButtonConfig::kFeatureLongPress)
  {
    if (id == MODE_BUTTON_ID)
    {
      drawMomentaryTextCentered(_display, "GPS", 1500, true);
    }
  }
  else if (eventType == ButtonConfig::kFeatureClick)
  {
    if (id == MODE_BUTTON_ID)
    {
      // ***
      // *** Increment the mode and set the mode
      // *** changed flag.
      // ***
      _mode = (++_mode) % 3;
      _modeChanged = true;
    }
    else
    {
      switch (_mode)
      {
        case MODE_TZ:
          // ***
          // *** Toggle the DST flag.
          // ***
          _isDst = !_isDst;
          break;
        case MODE_DST:
          // ***
          // *** Increment the TZ offset
          // *** by 30 minutes.
          // ***
          //_tz_offset += (_tz_change_direction * .5);
          break;
      }
    }
  }
}

void loop()
{
  // ***
  // *** Check the buttons to update their state.
  // ***
  //_buttons[MODE_BUTTON_ID].check();
  //_buttons[SETUP_BUTTON_ID].check();

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
  // *** If the mode has changed, reset the display.
  // ***
  if (_modeChanged)
  {
    _display.reset();
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
        if (updateTimeDisplay(_display, _rtc, _lastMinuteDisplayed, _modeChanged))
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

        showOffset(_display);
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

        showDst(_display);
      }
      break;
  }

  // ***
  // *** Reset mode and setup change flags.
  // ***
  _modeChanged = false;
  _setupChanged = false;

  // ***
  // *** Read data from the GPS for up to 750ms. This
  // *** is the "loop delay".
  // ***
  readGpsData(_gps, 750);
}

bool updateTimeDisplay(const ClockLedMatrix& display, const RTC_DS1307& rtc, int16_t lastMinute, bool force)
{
  bool returnValue = false;

  DateTime now = rtc.now();

  if (force | lastMinute != now.minute())
  {
    Debug.print(F("RTC Date/Time: ")); Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Clear the display.
    // ***
    display.reset();

    // ***
    // *** Adjust for the current time zone.
    // ***
    Debug.print("Time Zone Offset: "); Serial.println(_tz_offset);
    Debug.print("DST: "); Serial.println(_isDst ? "Yes" : "No");
    DateTime localNow = DateTime(now.unixtime() + (_tz_offset * 3600));
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
  Debug.print("AM/PM => "); Debug.println(pm ? "PM" : "AM");

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

void powerOnTest(const TinyGPS& gps)
{
  // ***
  // *** Light each LED.
  // ***
  for (uint8_t y = 0; y < _display.height(); y++)
  {
    for (uint8_t x = 0; x < _display.width(); x++)
    {
      _display.drawPixel(x, y, 1);
      delay(5);
    }
  }

  // ***
  // *** Pause for 2 seconds by reading GPS data.
  // ***
  readGpsData(gps, 2000);

  // ***
  // *** Reset the display.
  // ***
  _display.reset();
}

void readGpsData(const TinyGPS& gps, uint64_t readWaitTime)
{
  uint64_t start = millis();

  do
  {
    while (GpsSerial.available())
    {
      gps.encode(GpsSerial.read());
    }
  } while (millis() - start < readWaitTime);
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
    Debug.println("RTC time has been set.");
    rtc.adjust(dt);

    // ***
    // *** Return true to indicate the clock was set.
    // ***
    returnValue = true;
  }
  else
  {
    Debug.print("Date and Time not available. Age = "); Debug.println(age);
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

void showOffset(const ClockLedMatrix& display)
{
  display.setCursor(0, 6);
  display.println(_tz_offset);
}

void showDst(const ClockLedMatrix& display)
{
  char buffer[3];
  sprintf(buffer, "%s", _isDst ? "Yes" : "No");

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
