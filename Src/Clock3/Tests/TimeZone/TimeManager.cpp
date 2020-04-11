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
#include "TimeManager.h"

void TimeManager::begin(uint8_t currentTimeZoneId, DstMode_t dstMode, bool displayTwelveHourFormat, TimeEventHandler callback)
{
  // ***
  // *** Initialize the callback.
  // ***
  this->_callback = callback;

  // ***
  // *** Initialize the time zone ID.
  // ***
  this->_currentTimeZoneId = currentTimeZoneId;

  // ***
  // *** Initialize the DST mode.
  // ***
  this->_dstMode = dstMode;

  // ***
  // *** Initialize the displayTwelveHourFormat flag.
  // ***
  this->displayTwelveHourFormat = displayTwelveHourFormat;

  // ***
  // *** Start the RTC and fire the
  // *** appropriate event.
  // ***
  if (this->_rtc.begin())
  {
    this->_callback(TIME_INITIALIZED);
  }
  else
  {
    this->_callback(TIME_NO_RTC);
  }
}

void TimeManager::process()
{
  // ***
  // *** Get the current date and time from the RTC.
  // ***
  DateTime now = this->_rtc.now();

  // ***
  // *** Check if the minute has changed.
  // ***
  if (this->_lastMinuteDisplayed != now.minute())
  {
    // ***
    // *** Fire the event.
    // ***
    this->_callback(TIME_MINUTE_CHANGED);

    // ***
    // *** Track the time of the last update.
    // ***
    _lastMinuteDisplayed = now.minute();
  }
}

const uint8_t TimeManager::timeZoneId()
{
  // ***
  // *** Return the current time zone ID.
  // ***
  return this->_currentTimeZoneId;
}

void TimeManager::timeZoneId(uint8_t timeZoneId)
{
  // ***
  // *** Set the time zone ID.
  // ***
  this->_currentTimeZoneId = timeZoneId;

  // ***
  // *** Fire the event.
  // ***
  this->_callback(TIME_ZONE_CHANGED);
}

const char* TimeManager::timeZoneName()
{
  char* returnValue;

  // ***
  // *** Get the current timezone information.
  // ***
  TimeZone_t* timeZone = this->getTimeZone(this->_currentTimeZoneId);

  // ***
  // *** Check the current DST setting
  // ***
  if (this->isDst())
  {
    // ***
    // *** Use the DST name.
    // ***
    returnValue = timeZone->dName;
  }
  else
  {
    // ***
    // *** Use the standard name.
    // ***
    returnValue = timeZone->sName;
  }

  return returnValue;
}

const DstMode_t TimeManager::dstMode()
{
  return this->_dstMode;
}

void TimeManager::dstMode(DstMode_t dstMode)
{
  // ***
  // *** Set the DST mode.
  // ***
  this->_dstMode = dstMode;

  // ***
  // *** Fire the event.
  // ***
  this->_callback(DST_MODE_CHANGED);
}

const char* TimeManager::dstLabel()
{
  const char* returnValue;

  // ***
  // *** Return a text version of the
  // *** DST mode.
  // ***
  if (this->dstMode() == DstMode_t::YES)
  {
    returnValue = "Yes";
  }
  else if (this->dstMode() == DstMode_t::NO)
  {
    returnValue = "No";
  }
  else
  {
    returnValue = "Auto";
  }

  return returnValue;
}

const DstMode_t TimeManager::toggleDstMode()
{
  // ***
  // *** Change the DST mode to the
  // *** "next" value in tis order:
  // *** YES -> No -> AUTO
  // ***
  if (this->dstMode() == DstMode_t::YES)
  {
    this->dstMode(DstMode_t::NO);
  }
  else if (this->dstMode() == DstMode_t::NO)
  {
    this->dstMode(DstMode_t::AUTO);
  }
  else
  {
    this->dstMode(DstMode_t::YES);
  }

  return this->dstMode();
}

const DateTime TimeManager::utcDateTime()
{
  // ***
  // *** Get the current date an time from the RTC
  // *** in TC time.
  // ***
  return this->_rtc.now();
}

void TimeManager::utcDateTime(const DateTime& dateTime)
{
  // ***
  // *** Set the RTC date and time.
  // ***
  _rtc.adjust(dateTime);
}

const DateTime TimeManager::localDateTime()
{
  // ***
  // *** Calculate the local date and time based on
  // *** the current time zone.
  // ***
  return this->calculateLocalDateTime(this->timeOffset());
}

const int16_t TimeManager::timeOffset()
{
  int16_t returnValue = 0;

  // ***
  // *** Get the current timezone information.
  // ***
  TimeZone_t* tz = TimeManager::getTimeZone(this->timeZoneId());

  // ***
  // *** Check current DST.
  // ***
  if (this->isDst())
  {
    // ***
    // *** Use daylight savings offset.
    // ***
    returnValue = tz->dOffset;
  }
  else
  {
    // ***
    // *** Use standard offset.
    // ***
    returnValue = tz->sOffset;
  }

  return returnValue;
}

const bool TimeManager::isDst()
{
  bool returnValue = false;

  // ***
  // *** Check the DST mode.
  // ***
  if (this->dstMode() == DstMode_t::YES)
  {
    // ***
    // *** DST has been turned on manually.
    // ***
    returnValue = true;
  }
  else if (this->dstMode() == DstMode_t::NO)
  {
    // ***
    // *** DST has been turned off manually.
    // ***
    returnValue = false;
  }
  else
  {
    // ***
    // *** DST must be calculated based on the time
    // *** zone and current date and time.
    // ***
    returnValue = this->calculateDst();
  }

  return returnValue;
}

const uint8_t TimeManager::localHour()
{
  uint8_t returnValue = 0;

  // ***
  // *** Check the current format setting.
  // ***
  if (this->displayTwelveHourFormat)
  {
    // ***
    // *** Convert the current hour to 12-hour format.
    // ***
    returnValue = this->twentyFourToTwelve(this->localDateTime().hour());
  }
  else
  {
    // ***
    // *** Return standard 24-hour format.
    // ***
    returnValue = this->localDateTime().hour();
  }

  return returnValue;
}

const uint8_t TimeManager::localMinute()
{
  // ***
  // *** return the minute portion of the current time.
  // ***
  return this->localDateTime().minute();
}

const uint8_t TimeManager::twentyFourToTwelve(uint8_t hour)
{
  uint8_t returnValue = 0;

  if (hour == 0)
  {
    // ***
    // *** This is 12 am
    // ***
    returnValue = 12;
  }
  else if (hour >= 1 && hour <= 12)
  {
    // ***
    // *** During the am hours (and noon) the time does
    // *** not need to be converted.
    // ***
    returnValue = hour;
  }
  else
  {
    // ***
    // *** During the pm hours, subtract
    // *** 12 from the hour.
    // ***
    returnValue = hour - 12;
  }

  return returnValue;
}

const bool TimeManager::isAm()
{
  // ***
  // *** If it is not pm, it is am.
  // ***
  return !this->isPm();
}

const bool TimeManager::isPm()
{
  // ***
  // *** It is PM starting at 12 noon (12 pm) until 23
  // *** (11 pm). Hour will switch to 0 at midnight
  // *** which is 12 am.
  // ***
  return this->localDateTime().hour() >= 12;
}

const static uint8_t TimeManager::timeZoneCount()
{
  // ***
  // *** The number of time zones defined is the total size
  // *** of the array structure divided by the size of each
  // *** element in the array (in bytes).
  // ***
  return sizeof(_timeZones) / sizeof(TimeZone_t);
}

const static TimeZone_t* TimeManager::getTimeZone(uint8_t timeZoneId)
{
  TimeZone_t* returnValue = &_timeZones[0];

  // ***
  // *** Check that the Id is less than the total
  // *** number of time ones defined.
  // ***
  if (timeZoneId < TimeManager::timeZoneCount())
  {
    // ***
    // *** Return the timezone by ID. The time zones
    // *** are indexed in the array by ID.
    // ***
    returnValue = &_timeZones[timeZoneId];
  }

  return returnValue;
}

const DateTime TimeManager::calculateLocalDateTime(int16_t offset)
{
  // ***
  // *** Get the current UTC date and time.
  // ***
  DateTime now = this->utcDateTime();

  // ***
  // *** Add the appropriate number of hours based
  // *** on the specified offset (could be a negative
  // *** number that results in subtracting hours).
  // ***
  return DateTime(now.unixtime() + (offset * 3600));
}

bool TimeManager::calculateDst()
{
  bool returnValue = false;

  // ***
  // *** Get the current timezone information.
  // ***
  TimeZone_t* tz = TimeManager::getTimeZone(this->timeZoneId());

  if (tz->observesDst)
  {
    // ***
    // *** Get the local date and time as if it is
    // *** currently standard time since DST
    // *** changes over based on standard time.
    // ***
    DateTime standardLocal = this->calculateLocalDateTime(tz->sOffset);

    if (this->isAfterDstStart(standardLocal))
    {
      // ***
      // *** Get the local date and time as if it is
      // *** currently daylight savings time since DST
      // *** changes back based on daylight savings time.
      // ***
      DateTime daylightSavingsLocal = this->calculateLocalDateTime(tz->dOffset);

      if (this->isBeforeDstEnd(daylightSavingsLocal))
      {
        returnValue = true;
      }
    }
  }

  return returnValue;
}

bool TimeManager::isAfterDstStart(const DateTime& dt)
{
  bool returnValue = false;

  // ***
  // *** Check if we are within the daylight savings months.
  // ***
  if (dt.month() >= MARCH)
  {
    if (dt.month() == MARCH)
    {
      // ***
      // *** Create the first day of March.
      // ***
      DateTime s = DateTime(dt.year(), MARCH, 1, 0, 0, 0);

      // ***
      // *** Calculate the second Sunday in March.
      // ***
      uint8_t secondSundayInMarch = s.dayOfTheWeek() == 0 ? s.day() + 7 : 7 + s.day() + (7 - s.dayOfTheWeek());

      // ***
      // *** Determine if we are after the second
      // *** Sunday of March.
      // ***
      if (dt.day() > secondSundayInMarch)
      {
        returnValue = true;
      }
      else if (dt.day() == secondSundayInMarch)
      {
        // ***
        // *** If it is after 2:00 am it is DST.
        // ***
        if (dt.hour() >= DST_TIME)
        {
          returnValue = true;
        }
      }
    }
    else
    {
      returnValue = true;
    }
  }

  return returnValue;
}

bool TimeManager::isBeforeDstEnd(const DateTime& dt)
{
  bool returnValue = false;

  // ***
  // *** Check if we are within the daylight savings months.
  // ***
  if (dt.month() <= NOVEMBER)
  {
    // ***
    // *** Determine if we are before the first
    // *** Sunday of November.
    // ***
    if (dt.month() == NOVEMBER)
    {
      // ***
      // *** Create the first day of November.
      // ***
      DateTime s = DateTime(dt.year(), NOVEMBER, 1, 0, 0, 0);

      // ***
      // *** Calculate the first Sunday in November.
      // ***
      uint8_t firstSundayInNovember = s.dayOfTheWeek() == 0 ? s.day() : s.day() + (7 - s.dayOfTheWeek());
      
      if (dt.day() < firstSundayInNovember)
      {
        returnValue = true;
      }
      else if (dt.day() == firstSundayInNovember)
      {
        // ***
        // *** If it is before 2:00 am it is DST.
        // ***
        returnValue = dt.hour() < DST_TIME;
      }
    }
    else
    {
      returnValue = true;
    }
  }

  return returnValue;
}
