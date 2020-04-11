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
#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "RtcMemory.h"

#define MARCH 3
#define NOVEMBER 11
#define DST_TIME 2

// ***
// *** A list of event IDs.
// ***
typedef enum TIME_EVENT_ID : uint8_t
{
  TIME_INITIALIZED = 0,
  TIME_NO_RTC = 1,
  TIME_ZONE_CHANGED  = 2,
  DST_MODE_CHANGED  = 3,
  TIME_MINUTE_CHANGED = 4
} TimeEventId_t;

typedef enum DST_MODE : uint8_t
{
  YES = 0,
  NO = 1,
  AUTO = 2
} DstMode_t;

typedef struct TIME_ZONE
{
  uint8_t id;
  char sName[5];
  char dName[5];
  int16_t sOffset;
  int16_t dOffset;
  bool observesDst;
} TimeZone_t;

// ***
// *** US time zones (https://time.gov/).
// ***
const static TimeZone_t _timeZones[] = {
  {  0, "UTC\0", "UTC\0", 0, 0, false },      // Coordinated Universal Time
  {  1, "AKST\0", "AKDT\0", -9, -8, true },   // ALASKA STANDARD/DAYLIGHT TIME
  {  2, "HAST\0", "HADT\0", -10, -9, true },  // ALEUTIAN STANDARD/DAYLIGHT TIME
  {  3, "HST\0", "HST\0", -10, -10, false },  // HAWAII STANDARD TIME
  {  4, "SST\0", "SST\0", -11, -11, false },  // SAMOA STANDARD TIME
  {  5, "CHST\0", "CHST\0",  10, 10, false }, // CHAMORRO STANDARD TIME
  {  6, "PST\0", "PDT\0", -8, -7, true },     // PACIFIC STANDARD/DAYLIGHT TIME
  {  7, "MST\0", "MDT\0", -7, -6, true },     // MOUNTAIN STANDARD/DAYLIGHT TIME
  {  8, "CST\0", "CDT\0", -6, -5, true },     // CENTRAL STANDARD/DAYLIGHT TIME
  {  9, "EST\0", "EDT\0", -5, -4, true },     // EASTERN STANDARD/DAYLIGHT TIME
  { 10, "AST\0", "AST\0", -4, -4, false }     // PUERTO RICO ATLANTIC STANDARD TIME
};

class TimeManager
{
  public:
    // ***
    // *** Definition for the event callback handler.
    // ***
    using TimeEventHandler = void (*)(TimeEventId_t);

    // ***
    // *** Default constructor.
    // ***
    TimeManager() { };

    // ***
    // *** Initializes this instance.
    // ***
    void begin(uint8_t, DstMode_t, bool, TimeEventHandler);

    // ***
    // *** Background processing.
    // ***
    void process();

    // ***
    // *** Gets/sets the current time zone ID.
    // ***
    const uint8_t timeZoneId();
    void timeZoneId(uint8_t);

    // ***
    // *** Gets/sets the current DST mode.
    // ***
    const DstMode_t dstMode();
    void dstMode(DstMode_t);

    // ***
    // *** Returns a label that can be displayed
    // *** for the current DST mode.
    // ***
    const char* dstLabel();

    // ***
    // *** Changes the DST mode to the next value.
    // ***
    const DstMode_t toggleDstMode();

    // ***
    // *** Gets/sets UTC time.
    // ***
    const DateTime utcDateTime();
    void utcDateTime(const DateTime&);

    // ***
    // *** Gets the local time based on the current
    // ** time zone offset and DST flag.
    // ***
    const DateTime localDateTime();
    const DateTime localDateTime(int16_t, bool);

    // ***
    // *** Returns the current time offset
    // *** based on the current time zone
    // *** and DST.
    // ***
    const int16_t timeOffset();

    // ***
    // *** Returns name of current time zone.
    // ***
    const char* timeZoneName();

    // ***
    // *** Based on the current time zone and date and time, using
    // *** US rules, determine if it is currently DST or not.
    // ***
    // *** - DST begins at 2:00 a.m. on the second Sunday of March
    // *** - DST ends at 2:00 a.m. on the first Sunday of November
    // *** https://www.nist.gov/pml/time-and-frequency-division/popular-links/daylight-saving-time-dst
    // ***
    const bool isDst();

    // ***
    // *** Determines if the localHour() returns
    // *** 12 hour or 24 hour format.
    // ***
    bool displayTwelveHourFormat = true;

    // ***
    // *** Returns the current local hour value.
    // ***
    const uint8_t localHour();

    // ***
    // *** Returns the current local minute value.
    // ***
    const uint8_t localMinute();

    // ***
    // *** Converts a 24 hour value to a 12 hour value.
    // ***
    const uint8_t twentyFourToTwelve(uint8_t);

    // ***
    // *** Returns true when the current local time is AM.
    // ***
    const bool isAm();

    // ***
    // *** Returns true when the current local time is PM.
    // ***
    const bool isPm();

    // ***
    // *** Gets the time zone by ID.
    // ***
    const static TimeZone_t* getTimeZone(uint8_t timeZoneId);

    // ***
    // *** Returns the number of time zones defined.
    // ***
    const static uint8_t TimeManager::timeZoneCount();

  protected:
    // ***
    // *** An instance of the the RTC_DS1307 library. The
    // *** RTC in the Spikenzielabs clock is a DS1337 but
    // *** the interface for DS1307 works just as well.
    // ***
    RtcMemory _rtc;

    // ***
    // *** Tracks the last minute displayed so the display
    // *** can be updated once per minute.
    // ***
    int16_t _lastMinuteDisplayed = -1;

    // ***
    // *** The currnet time zone ID.
    // ***
    uint8_t _currentTimeZoneId = 0;

    // ***
    // *** Th current DST mode.
    // ***
    DstMode_t _dstMode = DstMode_t::AUTO;

    // ***
    // *** Calculates local date and time based on
    // *** DST and offset.
    // ***
    const DateTime calculateLocalDateTime(int16_t);

    // ***
    // *** Calculates DST based on the current date
    // *** and time.
    // ***
    bool calculateDst();

    // ***
    // *** Checks if the current date and time
    // *** is after the starting date and time
    // *** for DST.
    // ***
    bool isAfterDstStart(const DateTime& dt);

    // ***
    // *** Checks if the current date and time
    // *** is before the starting date and time
    // *** for DST.
    // ***
    bool isBeforeDstEnd(const DateTime& dt);

    // ***
    // *** The event callback handler.
    // ***
    TimeEventHandler _callback;
};
#endif
