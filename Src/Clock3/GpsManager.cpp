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
#include "GpsManager.h"

GpsManager::GpsManager(Stream* serialPort)
{
  this->_serialPort = serialPort;
}

void GpsManager::begin(GpsEventHandler callback)
{
  this->_callback = callback;

  // ***
  // *** Initialize the GPS.
  // ***
  this->_serialPort->println(PMTK_SET_BAUD_9600); delay(250);
  this->_serialPort->println(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); delay(250);
  this->_serialPort->println(PMTK_API_SET_FIX_CTL_1HZ); delay(250);
  this->_serialPort->println(PMTK_SET_NMEA_OUTPUT_RMCGGA); delay(250);
  this->_serialPort->println(PMTK_ENABLE_WAAS); delay(250);
  this->_serialPort->println(PGCMD_ANTENNA); delay(250);

  this->_callback(GpsEventId_t::GPS_INITIALIZED);
}

const uint16_t GpsManager::getBaudRate()
{
  return 9600;
}

void GpsManager::process()
{
  // ***
  // *** Only enter once. Due to calling yield(), it is possible
  // *** reneter this method over and over again either causing
  // *** a stack overflow or some kind of break down of the space
  // *** and time vortex.
  // ***
  if (!_processing)
  {
    // ***
    // *** Set the flag to prevent re-entry.
    // ***
    this->_processing = true;

    while (this->_serialPort->available())
    {
      // ***
      // *** Rad the data from the GPS and pass it to
      // *** the TinyGPS library for parsing.
      // ***
      this->_gps.encode(this->_serialPort->read());

      // ***
      // *** Call yield to allow other background
      // *** processes to run.
      // ***
      yield();
    }

    // ***
    // *** Parse the date and time from the TinyGPS library.
    // ***
    this->parseDateAndTime();

    // ***
    // *** Reset the flag.
    // ***
    _processing = false;
  }
}

const bool GpsManager::hasFix()
{
  return this->_hasFix;
}

void GpsManager::setHasFix(bool hasFix)
{
  // ***
  // *** Check if the new value has changed.
  // ***
  if (this->_hasFix != hasFix)
  {
    // ***
    // *** Change the value.
    // ***
    this->_hasFix = hasFix;

    // ***
    // *** Fire the event.
    // ***
    this->_callback(GpsEventId_t::GPS_FIX_CHANGED);
  }
}

const DateTime GpsManager::dateTime()
{
  // ***
  // *** Return the internal value.
  // ***
  return this->_currentDateTime;
}

void GpsManager::parseDateAndTime()
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;

  // ***
  // *** Get the date and time from the GPS data.
  // ***
  this->_gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  // ***
  // *** Check the age to make sure the date and time are valid.
  // ***
  if (age != TinyGPS::GPS_INVALID_AGE)
  {
    // ***
    // *** Create  UTC date and time instance.
    // ***
    this->_currentDateTime = DateTime(year, month, day, hour, minute, second);
    this->setHasFix(true);
  }
  else
  {
    // ***
    // *** We may still have a fix, but set this to false to indicate
    // *** the current date and time cannot be trusted.
    // ***
    this->setHasFix(false);
  }
}
