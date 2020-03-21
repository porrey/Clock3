# Clock3
New firmware for the Spikenzie Labs Solder:Time Desk Clock ([https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock](https://www.spikenzielabs.com/Catalog/watches-clocks/solder-time-desk-clock)).

![Clock](https://github.com/porrey/Clock3/raw/master/Images/Clock.jpg)

This firmware has been written from scratch using standard Arduino libraries.

- A driver has been created for the LED matrix, so it is compatible with the Adafruit GXF library.
- The Adafruit RTCLib library has been used for the DS1337 RTC.
- A font was created for the clock that works with the GFX library.
- Support for capturing time from a GPS has been added using the TinyGPS library. I'm using the [Adafruit Ultimate GPS Breakout - 66 channel w/10 Hz updates - Version 3](https://www.adafruit.com/product/746) in my clock. This code will work with any standard GPS.
- The buttons are managed using the AceButton library. This library provides support for more than one event type on a button such as press, release, long press and repeated press. This allows the buttons to provide more creative capabilities.
- The original TimerOne library has been kept for refreshing the display

# GFX Font Editor

This repository also contains source code for a Windows 10 UWP application called GFX Font Editor written in c#. This application can be used to create and edit fonts for the Adafruit GFX library. This application is in the early stages of development. It currently only supports a character width of 8.

The font used in Clock3 was developed using this software.

![GFX Font Editor](https://github.com/porrey/Clock3/raw/master/Images/GfxEditor-ScreenShot.png)

## Library References:

1. [https://github.com/adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
2. [https://github.com/PaulStoffregen/TimerOne](https://github.com/PaulStoffregen/TimerOne)
3. [https://github.com/adafruit/RTClib](https://github.com/adafruit/RTClib)
4. [https://github.com/bxparks/AceButton](https://github.com/bxparks/AceButton)
5. [https://github.com/neosarchizo/TinyGPS](https://github.com/neosarchizo/TinyGPS)