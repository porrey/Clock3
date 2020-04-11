#include "TimeManager.h"
#include "Tests_TimeZoneId_00.h"
#include "Tests_TimeZoneId_01.h"
#include "Tests_TimeZoneId_02.h"
#include "Tests_TimeZoneId_03.h"
#include "Tests_TimeZoneId_04.h"
#include "Tests_TimeZoneId_05.h"
#include "Tests_TimeZoneId_06.h"
#include "Tests_TimeZoneId_07.h"
#include "Tests_TimeZoneId_08.h"
#include "Tests_TimeZoneId_09.h"
#include "Tests_TimeZoneId_10.h"

TimeManager _timeManager;

void setup()
{
  // ***
  // *** Initialize the serial port.
  // ***
  Serial.begin(115200);

  // ***
  // *** Initialize the time manager.
  // ***
  _timeManager.begin(0, DstMode_t::AUTO, true, onTimeEvent);

  // ***
  // *** Select the test to run. There is not
  // *** enough memory on an ATmega328P to run
  // *** all tests at once.
  // ***
  run_TimeZoneId_0_Tests(_timeManager);
  //run_TimeZoneId_1_Tests(_timeManager);
  //run_TimeZoneId_2_Tests(_timeManager);
  //run_TimeZoneId_3_Tests(_timeManager);
  //run_TimeZoneId_4_Tests(_timeManager);
  //run_TimeZoneId_5_Tests(_timeManager);
  //run_TimeZoneId_6_Tests(_timeManager);
  //run_TimeZoneId_7_Tests(_timeManager);
  //run_TimeZoneId_8_Tests(_timeManager);
  //run_TimeZoneId_9_Tests(_timeManager);
  //run_TimeZoneId_10_Tests(_timeManager);
}

void loop()
{

}

void onTimeEvent(TimeEventId_t eventId)
{

}
