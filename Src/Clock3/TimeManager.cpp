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
  DateTime now = this->_rtc.now();

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

int16_t TimeManager::getTimeZoneOffset()
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

bool TimeManager::getIsDst()
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

DateTime TimeManager::getUtcDateTime()
{
  return this->_rtc.now();
}

void TimeManager::setUtcDateTime(const DateTime& dateTime)
{
  _rtc.adjust(dateTime);
}

DateTime TimeManager::getLocalDateTime()
{
  DateTime now = this->getUtcDateTime();
  return DateTime(now.unixtime() + (this->_tzOffset * 3600) + (this->_isDst ? 3600 : 0));
}

uint8_t TimeManager::localHour()
{
  return this->twentyFourToTwelve(this->getLocalDateTime().hour());
}

uint8_t TimeManager::localMinute()
{
  return this->getLocalDateTime().minute();
}

uint8_t TimeManager::twentyFourToTwelve(uint8_t hour)
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

bool TimeManager::isAm()
{
  return !this->isPm();
}

bool TimeManager::isPm()
{
  return this->getLocalDateTime().hour() >= 12;
}
