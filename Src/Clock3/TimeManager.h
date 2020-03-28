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

class TimeManager
{
  public:
    // ***
    // *** A list of event IDs.
    // ***
    enum TIME_EVENT_ID
    {
      TIME_INITIALIZED = 0, TIME_NO_RTC = 1, TIME_TZ_OFFSET_CHANGED  = 2,
      TIME_DST_CHANGED = 3, TIME_MINUTE_CHANGED = 4
    };

    // ***
    // *** Definition for the event callback handler.
    // ***
    using TimeEvent = void (*)(TIME_EVENT_ID);

    // ***
    // ***
    // ***
    void begin(TimeEvent);
    void begin(int16_t, bool, TimeEvent);

    // ***
    // ***
    // ***
    void tickTock();

    // ***
    // *** Gets/sets the time zone offset.
    // ***
    void setTimeZoneOffset(int16_t);
    const int16_t getTimeZoneOffset();

    // ***
    // *** Gets/sets the daylight savings flag.
    // ***
    const bool getIsDst();
    void setIsDst(bool);

    // ***
    // *** Gets/sets UTC time.
    // ***
    const DateTime getUtcDateTime();
    void setUtcDateTime(const DateTime&);

    // ***
    // *** Gets the local time based on the current
    // ** time zone offset and DST flag.
    // ***
    const DateTime getLocalDateTime();

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

  protected:
    // ***
    // *** An instance of the the RTC_DS1307 library. The RTC in the Spikenzielabs clock
    // *** is a DS1337 but the interface for DS1307 works just as well.
    // ***
    RTC_DS1307 _rtc;

    // ***
    // *** Holds the time zone offset.
    // ***
    int16_t _tzOffset;

    // ***
    // *** Holds the DST flag.
    // ***
    bool _isDst;

    // ***
    // *** Tracks the last minute displayed so the display can
    // *** be updated once per minute.
    // ***
    int16_t _lastMinuteDisplayed = -1;

    // ***
    // *** The event callback handler.
    // ***
    TimeEvent _callback;
};
#endif
