#include "Tests.h"
#include "ClockMatrix.h"
#include "ClockFont.h"
#include "TimerOne.h"
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// ***
// *** Initialization strings for the GPS.
// ***
#define PMTK_SET_BAUD_9600 F("$PMTK251,9600*17")
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  F("$PMTK220,5000*1B")
#define PMTK_API_SET_FIX_CTL_1HZ  F("$PMTK300,1000,0,0,0,0*1C")
#define PMTK_SET_NMEA_OUTPUT_RMCGGA F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28")
#define PMTK_ENABLE_WAAS
#define PGCMD_ANTENNA F("$PGCMD,33,1*6C")
#define PGCMD_NOANTENNA F("$PGCMD,33,0*6D")

// ***
// *** Define the RX/TX pins for the debug serial port.
// ***
#define DEBUG_RX  PB7
#define DEBUG_TX  7
SoftwareSerial Debug(DEBUG_RX, DEBUG_TX); // RX, TX

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
// *** The time zone offset. The default is -6 since I live in Chicago.
// ***
volatile int8_t _tz_offset = -6;

// ***
// ***
// ***
volatile bool _isDst = true;

// ***
// *** Tracks the last minute displayed so the display can be updated once per minute.
// ***
int16_t _lastMinute = -1;

// ***
// *** A flag to indicate if time has been set.
// ***
bool _timeIsSet = false;

// ***
// *** The time of the last GPS update.
// ***
DateTime _lastUpdated;

// ***
// *** Mode
// ***
enum MODES
{
  MODE_DISPLAY_TIME = 0,
  MODE_SHOW_OFFSET = 1,
  MODE_SHOW_DST = 2
};

MODES _mode = MODE_DISPLAY_TIME;

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
  // *** Set up the timer for display refresh.
  // ***
  Timer1.initialize(250);
  Timer1.attachInterrupt(refreshSupply);

  // ***
  // *** Initialize the GPS.
  // ***
  Serial.begin(9600);

  Serial.println(PMTK_SET_BAUD_9600); delay(250);
  Serial.println(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); delay(250);
  Serial.println(PMTK_API_SET_FIX_CTL_1HZ); delay(250);
  Serial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA); delay(250);
  Serial.println(PMTK_ENABLE_WAAS); delay(250);
  Serial.println(PGCMD_NOANTENNA); delay(250);

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
    Debug.println(String(F("DateTime::TIMESTAMP_FULL:\t")) + time.timestamp(DateTime::TIMESTAMP_FULL));
  }

  // ***
  // *** Power on test.
  // ***
  powerOnTest();

  // ***
  // *** Display the GPS fix indicator.
  // ***
  updateGpsFixDisplay();
}

void loop()
{
  switch (_mode)
  {
    // ***
    // *** Display the time.
    // ***
    case MODE_DISPLAY_TIME:
      // ***
      // *** Update the displayed time.
      // ***
      updateTimeDisplay();

      // ***
      // *** Display the GPS fix indicator.
      // ***
      updateGpsFixDisplay();
      break;
    case MODE_SHOW_OFFSET:
      // ***
      // *** Display the current offset.
      // ***
      showOffset();
      break;
    case MODE_SHOW_DST:
      // ***
      // *** Display the current DST mode.
      // ***
      showDst();
      break;
  }

  // ***
  // *** Check to see if a GPS update is needed.
  // ***
  checkGpsTime();

  // ***
  // *** Short delay.
  // ***
  delay(250);
}

void updateTimeDisplay()
{
  DateTime now = _rtc.now();

  if (_lastMinute != now.minute())
  {
    // ***
    // *** Get the number of seconds since the last GPS update.
    // ***
    uint32_t secondsSinceLastGpsUpdate = now.unixtime() - _lastUpdated.unixtime();

    // ***
    // *** Clear the display.
    // ***
    _display.reset();

    // ***
    // *** Get a string version of the time.
    // ***
    String time = getTimeString(&now);
    Debug.print(F("Time: ")); Serial.println(time);

    // ***
    // *** The time displayed will be centered. Calculate the width
    // *** and left position.
    // ***
    int len = time.length();
    int width = len * 3;
    int16_t left = ((int16_t)((_display.width() - width) / 2.0)) - 1;

    // ***
    // *** Set the cursor to center the time.
    // ***
    _display.setCursor(left, 6);

    // ***
    // *** Update the time on the display.
    // ***
    _display.println(time);

    // ***
    // *** Update the AM/PM mark on the display.
    // ***
    updateAmPmDisplay(&now);

    // ***
    // *** Track the time of the last update. We only change the display each minute.
    // ***
    _lastMinute = now.minute();

    // ***
    // *** Trigger a GPS read every hour.
    // ***
    if (secondsSinceLastGpsUpdate > 3600)
    {
      Debug.println(F("Triggering GPS update."));
      _timeIsSet = false;
    }
  }
}

void updateGpsFixDisplay()
{
  // ***
  // ** Check if we have a GPS fix.
  // ***
  bool fix = _timeIsSet;

  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  _display.drawPixel(18, 1, fix ? 1 : 0);
}

void updateAmPmDisplay(DateTime* now)
{
  // ***
  // ** Check if it is PM.
  // ***
  bool pm = now->hour() >= 12;

  // ***
  // *** Highlight the LED at x = 18 and y = 5
  // *** when the time is PM.
  // ***
  _display.drawPixel(18, 5, pm ? 1 : 0);
}

void checkGpsTime()
{
  // ***
  // *** Check if the time needs to be set from the GPS signal.
  // ***
  if (!_timeIsSet)
  {
    // ***
    // *** Get data for the GPS. Let it read for about 2
    // *** seconds; should be enough.
    // ***
    getGpsData(_gps, 2000);

    // ***
    // *** Try to get the time from GPS and set the clock.
    // ***
    if (setDateAndTime(_gps, _rtc, _tz_offset))
    {
      _lastUpdated = _rtc.now();
      _timeIsSet = true;
    }
  }
}

void refreshSupply()
{
  // ***
  // *** Refresh the LED matrix.
  // ***
  _display.refresh();
}

void powerOnTest()
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
  // *** Pause.
  // ***
  delay(1000);

  // ***
  // *** Reset the display.
  // ***
  _display.reset();
}

void getGpsData(const TinyGPS& gps, uint64_t readTime)
{
  uint64_t start = millis();

  do
  {
    while (Serial.available())
    {
      _gps.encode(Serial.read());
    }
  } while (millis() - start < readTime);
}

bool setDateAndTime(const TinyGPS& gps, const RTC_DS1307& rtc, int tz_offset)
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
    Debug.println(String(F("GPS/UTC Date/Time:\t")) + dt.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Adjust for the current time zone.
    // ***
    DateTime dtAdjusted = DateTime(dt.unixtime() + (tz_offset * 3600));
    Debug.println(String(F("Local Date/Time:\t")) + dtAdjusted.timestamp(DateTime::TIMESTAMP_FULL));

    // ***
    // *** Set the time on the RTC.
    // ***
    // ***
    Debug.println("RTC time has been set.");
    rtc.adjust(dtAdjusted);

    // ***
    // *** Return true to indicate the clock was set.
    // ***
    returnValue = true;
  }
  else
  {
    Debug.println("Date and Time not available.");
    returnValue = false;
  }

  return returnValue;
}

String getTimeString(DateTime* now)
{
  uint8_t hour = now->hour() > 12 ? now->hour() - 12 : now->hour();
  char buffer[5];
  sprintf(buffer, "%01d:%02d", hour, now->minute());
  return String(buffer);
}

void showOffset()
{
  char buffer[9];
  sprintf(buffer, "TZO: %01d", _tz_offset);

  // ***
  // *** Set the cursor to center the time.
  // ***
  _display.setCursor(1, 6);

  // ***
  // *** Update the time on the display.
  // ***
  _display.println(buffer);
}

void showDst()
{
  char buffer[9];
  sprintf(buffer, "DST: %s", _isDst ? "Y" : "N");

  // ***
  // *** Set the cursor to center the time.
  // ***
  _display.setCursor(1, 6);

  // ***
  // *** Update the time on the display.
  // ***
  _display.println(buffer);
}
