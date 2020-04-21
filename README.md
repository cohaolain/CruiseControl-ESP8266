# ESP8266 Cruise Control (GPS/OLED)

This is a project I'm working on so I can have cruise control _(ish)_ in my car.

It uses GPS in order to monitor the speed of your vehicle,
and shows your speed on the OLED display.  
It allows you to set your desired speed limit using two push buttons.  
If you speed, the device screams - the pitch changing with your speeding.

I will write a better description at some point.

## Hardware

-   ESP8266
    -   I used a Heltec WiFi Kit 8
        -   Which the manufacturer & retailer have about 5 completely different pinout diagrams for.
        -   Eventually found a correct one after much digging 😤.
    -   Not actually using the WiFi capabilities of this myself, _yet_.
-   OLED display
    -   Ideally compatible with `u8g2`.
        The WiFi Kit 8 already has such a display (128x32), quite handy.
        -   It just took a while to figure out which pinout diagram to use 😤.
-   GPS module
    -   I used a module with a NEO-6M.
        -   I _had_ bought a MakerHawk module that claimed to have a NEO-6M.
            It actually turned out to be a NEO-7.  
            I wanted to use the module's EEPROM to store our GPS receiver settings,
            but the newer version doesn't support EEPROM.  
            The NEO-7 module still had the EEPROM soldered on,
            but it was inaccessible. So I'd to return that.  
            Then I got myself a shiny new NEO-6M module!
    -   EEPROM is useful so we
        -   Don't need to connect the RX pin of the GPS module
            (I ran out of easily usable pins on my ESP8266)
        -   Don't need to rely on the BBR (battery-backed RAM)
    -   The default settings are changed so that we
        -   Have a higher navigation refresh rate (5hz compared to 1hz)
        -   Use the automotive navigation profile
        -   Turn off the flashing light (timepulse)
        -   Increase the baud rate (9600 to 19200)
        -   Change what data the module actually provides
    -   Breadboards, pushbuttons, a buzzer, wires, resistors etc.

Will include a schematic in the future.

## Libraries

-   **SoftwareSerial**
    -   For communicating with the GPS module
-   [**TinyGPS++**](https://github.com/mikalhart/TinyGPSPlus)
    -   For dealing with all that GPS data
-   [**u8g2**](https://github.com/olikraus/u8g2/wiki/fntlistall)
    -   For all our display interfacing needs
-   [**ESP8266WiFi**](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
    -   For (_now_), this is just to turn off the WiFi capabilities of the ESP8266.

## More

Eventually....
