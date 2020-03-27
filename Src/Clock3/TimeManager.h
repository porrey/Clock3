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
    // ***
    // ***
    void setTimeZoneOffset(int16_t);
    int16_t getTimeZoneOffset();

    // ***
    // ***
    // ***
    bool getIsDst();
    void setIsDst(bool);

    // ***
    // ***
    // ***
    DateTime getUtcDateTime();
    void setUtcDateTime(const DateTime&);

    // ***
    // ***
    // ***
    DateTime getLocalDateTime();

    // ***
    // ***
    // ***
    uint8_t localHour();

    // ***
    // ***
    // ***
    uint8_t localMinute();

    // ***
    // ***
    // ***
    uint8_t twentyFourToTwelve(uint8_t);

    // ***
    // ***
    // ***
    bool isAm();

    // ***
    // ***
    // ***
    bool isPm();

  protected:
    // ***
    // *** An instance of the the RTC_DS1307 library. The RTC in the Spikenzielabs clock
    // *** is a DS1337 but the interface for DS1307 works just as well.
    // ***
    RTC_DS1307 _rtc;

    // ***
    // ***
    // ***
    int16_t _tzOffset;

    // ***
    // ***
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
