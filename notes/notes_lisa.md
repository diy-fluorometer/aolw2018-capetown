* Acquire ESP32 from other group - EasiKit ESP32-C1
* Download, install and test ESP32 toolchain (https://esp-idf.readthedocs.io/en/latest/get-started/index.html#get-started-get-esp-idf)
* Download, install and test Eclipse IDE for ESP32 development (https://esp-idf.readthedocs.io/en/latest/get-started/eclipse-setup.html)
* First steps with programming the ESP32 (http://www.lucadentella.it/en/2016/12/22/esp32-4-flash-bootloader-e-freertos/)
* Then tried Arduino for ESP32 (https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/debian_ubuntu.md)
* Worked, Board: DOIT ESP32 Dev Kit, 115200 Baud
* --> tsl.begin() is not reliable if no sensor is present

The default pins are defined in variants/nodemcu/pins_arduino.h as SDA=4 and SCL=5, but those are not pins number but GPIO number, so since the pins are D1=5 and D2=4.
SCL -> D1
SDA -> D2

# Modifications to the FluoroControl code

* Control the device using a mini-HTTP server
* Added test endpoints `/test/start` and `/test/stop` for initial measurements.
  * `/test/start/` will turn on the blue LED, and start logging measurements to the serial console, in the format \<time-in-millis\>\\t\<run-name\>\\t\<sequence-in-run\>\\t\<value\>
  * `/test/stop` will make the device stop taking measurements and turn of the LED
