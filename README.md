# Clock3
New firmware for the Spikenzie Labs Solder:Time Desk Clock ([https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock](https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock)).

This firmware is named **Clock3** because there were two versions of the original firmware released by Spikenzielabs.

- [Solder:Time II Arduino Sketch v1.0](http://www.spikenzielabs.com/-Downloadables/STDESKCLOCK/STDC_SketchV1.zip)
- [Solder:Time II Arduino Sketch v1.1](http://www.spikenzielabs.com/Downloadables/ST_Two_Release_1.1.zip)

## Clock Display
The image below shows an assembled Spikenzielabs Solder:Time Desk Clock running the Clock3 firmware. The time display take as little space as needed and is centerd on the display due to the variable-width font. The LED in the upper right corner indicates that the GPS has a fix. The LED in the lower right corner indicates PM. 

![Clock](https://github.com/porrey/Clock3/raw/master/Images/Clock-small.jpg)

## Firmware
This firmware has been written from scratch using standard Arduino libraries. In doing this, I'm hoping this firmware makes it easier to allow other developers and makers to customize thier own clock.

- A **matrix driver** has been created for the LED matrix so it is compatible with the Adafruit GXF library.
- The **Adafruit RTCLib** library has been used for the DS1337 RTC.
- A **font** was created for the clock that works with the GFX library.
- Support for capturing time from a **GPS** has been added using the TinyGPS library. I'm using the [Adafruit Ultimate GPS Breakout - 66 channel w/10 Hz updates - Version 3](https://www.adafruit.com/product/746) in my clock. This code will work with any standard GPS.
- The buttons are managed using the **AceButton** library. This library provides support for more than one event type on a button such as press, release, long press and repeated press. This allows the buttons to provide more creative capabilities.
- The original **TimerOne** library has been kept for refreshing the display. The refresh has been optimized to allow a lower refresh rate by drawing one column at a time while maintaining a consistent LED brightness.
- The code has been broken out into modules/classes in an effort to make it easier to include or exclude capabilities.
- This library continues to support sound through the pizeo.
- Added optional **[Westminster Quarters](https://en.wikipedia.org/wiki/Westminster_Quarters)** (hourly only).
- Added US timezone selection.
- Added an auto DST (daylight savings) option for built-in US timezones.
- Select bewteen 12-hour or 24-hour display.


## GFX Font Editor

This repository also contains source code for a Windows 10 UWP application called GFX Font Editor written in c#. This application can be used to create and edit fonts for the Adafruit GFX library. This application is in the early stages of development. It currently only supports a character width of 8.

The font used in Clock3 was developed using this software.

![GFX Font Editor](https://github.com/porrey/Clock3/raw/master/Images/GfxEditor-ScreenShot.png)

## Library References:

1. [https://github.com/adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
2. [https://github.com/PaulStoffregen/TimerOne](https://github.com/PaulStoffregen/TimerOne)
3. [https://github.com/adafruit/RTClib](https://github.com/adafruit/RTClib)
4. [https://github.com/bxparks/AceButton](https://github.com/bxparks/AceButton)
5. [https://github.com/neosarchizo/TinyGPS](https://github.com/neosarchizo/TinyGPS)
6. [https://github.com/porrey/EEPROM-Storage](https://github.com/porrey/EEPROM-Storage)
