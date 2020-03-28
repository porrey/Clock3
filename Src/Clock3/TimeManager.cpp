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

void TimeManager::begin(TimeEvent callback)
{
  this->_callback = callback;

  if (this->_rtc.begin())
  {
    this->_callback(TIME_INITIALIZED);
  }
  else
  {
    this->_callback(TIME_NO_RTC);
  }
}

void TimeManager::begin(int16_t tzOffset, bool isDst, TimeEvent callback)
{
  this->_tzOffset = tzOffset;
  this->_isDst = isDst;
  this->begin(callback);
}

void TimeManager::tickTock()
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

const int16_t TimeManager::getTimeZoneOffset()
{
  return this->_tzOffset;
}

void TimeManager::setTimeZoneOffset(int16_t tzOffset)
{
  if (this->_tzOffset != tzOffset)
  {
    this->_tzOffset = tzOffset;
    this->_callback(TIME_TZ_OFFSET_CHANGED);
  }
}

const bool TimeManager::getIsDst()
{
  return _isDst;
}

void TimeManager::setIsDst(bool isDst)
{
  if (this->_isDst != isDst)
  {
    this->_isDst = isDst;
    this->_callback(TIME_DST_CHANGED);
  }
}

const DateTime TimeManager::getUtcDateTime()
{
  return this->_rtc.now();
}

void TimeManager::setUtcDateTime(const DateTime& dateTime)
{
  _rtc.adjust(dateTime);
}

const DateTime TimeManager::getLocalDateTime()
{
  DateTime now = this->getUtcDateTime();
  return DateTime(now.unixtime() + (this->_tzOffset * 3600) + (this->_isDst ? 3600 : 0));
}

const uint8_t TimeManager::localHour()
{
  return this->twentyFourToTwelve(this->getLocalDateTime().hour());
}

const uint8_t TimeManager::localMinute()
{
  return this->getLocalDateTime().minute();
}

const uint8_t TimeManager::twentyFourToTwelve(uint8_t hour)
{
  uint8_t returnValue = 0;

  if (hour == 0)
  {
    returnValue = 12;
  }
  else if (hour >= 1 && hour <= 12)
  {
    returnValue = hour;
  }
  else
  {
    returnValue = hour - 12;
  }

  return returnValue;
}

const bool TimeManager::isAm()
{
  return !this->isPm();
}

const bool TimeManager::isPm()
{
  return this->getLocalDateTime().hour() >= 12;
}
