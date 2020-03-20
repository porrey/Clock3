# Clock3
New firmware for the Spikenzie Labs Solder:Time Desk Clock

![Clock](https://github.com/porrey/Clock3/raw/master/Images/Clock.jpg)

This firmware has been written from scratch using standard libraries.


- A driver has been created for the LED matrix so it is compatible with the Adafruit GXF library.
- The Adafruit RTCLib library has been used for the DS1337 RTC.
- A font was created for the clock that works with the GFX library

In addition, support for capturing time from a GPS has been added.

# GFX Font Editor

There is also source code for a Windows 10 UWP application called GFX Font Editor written in c#. This application can be used to create and edit fonts for the Adafruit GFX library. This application is in the early stages of development. It currently only supports a character width of 8.

The font used in Clock3 was developed using this software.

![GFX Font Editor](https://github.com/porrey/Clock3/raw/master/Images/GfxEditor-ScreenShot.png)
