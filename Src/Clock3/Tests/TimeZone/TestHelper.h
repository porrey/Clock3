#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "RTCLib.h"

void displayTestResult(const DateTime& dt, bool dst, bool expectedDst)
{
  char buffer[] = "MM-DD-YYYY hh:mm:ss";
  Serial.print("Local Dt/Tm: ");
  Serial.print(dt.toString(buffer));
  Serial.print("; DST => Actual = ");
  Serial.print(dst ? "Yes, " : "No, ");
  Serial.print("Expected = ");
  Serial.print(expectedDst ? "Yes => " : "No => ");
  Serial.println(dst == expectedDst ? "PASS" : "FAIL");
}

uint8_t test(const TimeManager& timeManager, uint8_t timeZoneId, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t seconds, bool expectedDst)
{
  uint8_t returnValue = 0;

  timeManager.timeZoneId(timeZoneId);
  DateTime dt = DateTime(year, month, day, hour, minute, seconds);
  timeManager.utcDateTime(dt);
  bool isDst = timeManager.isDst();
  returnValue  = (isDst == expectedDst) ? 0 : 1;

  if (returnValue == 1)
  {
    displayTestResult(timeManager.localDateTime(), isDst, expectedDst);
  }

  return returnValue;
}
#endif
