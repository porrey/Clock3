#ifndef GPS_H
#define GPS_H

// ***
// *** Initialization strings for the GPS.
// ***
#define PMTK_SET_BAUD_9600 F("$PMTK251,9600*17")
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ  F("$PMTK220,5000*1B")
#define PMTK_API_SET_FIX_CTL_1HZ  F("$PMTK300,1000,0,0,0,0*1C")
#define PMTK_SET_NMEA_OUTPUT_RMCGGA F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28")
#define PMTK_ENABLE_WAAS F("$PMTK301,2*2E")
#define PGCMD_ANTENNA F("$PGCMD,33,1*6C")
#define PGCMD_NOANTENNA F("$PGCMD,33,0*6D")

#endif
