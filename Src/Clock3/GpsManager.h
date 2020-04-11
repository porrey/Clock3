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
#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS.h>
#include <RTClib.h>

// ***
// *** Initialization strings for the GPS.
// ***
#define PMTK_SET_BAUD_9600 F("$PMTK251,9600*17")
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  F("$PMTK220,5000*1B")
#define PMTK_API_SET_FIX_CTL_1HZ  F("$PMTK300,1000,0,0,0,0*1C")
#define PMTK_SET_NMEA_OUTPUT_RMCGGA F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28")
#define PMTK_ENABLE_WAAS F("$PMTK301,2*2E")
#define PGCMD_ANTENNA F("$PGCMD,33,1*6C")

// ***
// *** A list of event IDs.
// ***
typedef enum GPS_EVENT_ID : uint8_t
{
  GPS_INITIALIZED = 0,
  GPS_FIX_CHANGED = 1
} GpsEventId_t;

class GpsManager
{
  public:
    // ***
    // *** Definition for the event callback handler.
    // ***
    using GpsEventHandler = void (*)(GpsEventId_t);

    // ***
    // *** Creates an instance of GpsManager with
    // *** the specified serial port.
    // ***
    GpsManager(Stream*);

    // ***
    // *** Initializes this instance with the given
    // *** event handler.
    // ***
    void begin(GpsEventHandler);

    // ***
    // *** Performs the background processing necessary to keep
    // *** the GPS date and time up to date.
    // ***
    void process();

    // ***
    // *** Returns the baud rate expected by the GPS module. This is
    // *** used by the caller to ensure the correct baud rate is set
    // *** on the serial port being passed to this instance.
    // ***
    const uint16_t getBaudRate();

    // ***
    // *** Indicates whether or not the GPS has a fix.
    // ***
    const bool hasFix();

    // ***
    // *** Gets the last date and time retrieved from
    // *** the GPS.
    // ***
    DateTime dateTime();

  protected:
    // ***
    // *** Holds a reference to the Serial port being used
    // *** to communicate with the GPS module.
    // ***
    Stream* _serialPort;

    // ***
    // *** Get/sets a value to indicate if we have fix
    // *** on the GPS. Once we have a fix, we can trust
    // *** the date and time value.
    // ***
    bool _hasFix = false;

    // ***
    // *** Used to changed the flag dn fire changed
    // *** event.
    // ***
    void setHasFix(bool);

    // ***
    // *** The instance of TinyGPS used to parse
    // *** the data coming rom the GPS module.
    // ***
    TinyGPS _gps = TinyGPS();

    // ***
    // *** Gets the date and time from the TinyGPS
    // *** library and converts it to a DateTime
    // *** structure.
    // ***
    void parseDateAndTime();

    // ***
    // *** Prevents renetering of process() method
    // *** while it is already running.
    // ***
    bool _processing = false;

    // ***
    // *** Holds the last date and time retrieved from
    // *** the GPS. This field is updated by a call to
    // *** parseDateAndTime().
    // ***
    DateTime _currentDateTime;

    // ***
    // *** The event callback handler.
    // ***
    GpsEventHandler _callback;
};
#endif
