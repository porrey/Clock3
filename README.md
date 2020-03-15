# Clock3
New firmware for the Spikenzie Labs Labs Solder:Time Desk Clock

![Clock](https://www.spikenzielabs.com/Catalog/index.php?main_page=popup_image&pID=842)

This firmware has been written from scratch using standard library's.


- A driver has been created for the LED matrix so it is compatible with the Adafruit GXF library.
- The Adafruit RTCLib library has been used for the DS1337 RTC.
- A font was created for the clock that works with the GFX library

In addition, support for capturing time from a GPS has been added.