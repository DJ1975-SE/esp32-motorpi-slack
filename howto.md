### How to

* Install arduino (https://www.arduino.cc)
* Install support for the ESP32 (https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md). Basically this means add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json as an additional Board Manager URL, and then pick the right board. Description & Video also under https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
* Choose the right Board (in my case ESP32 Dev Module, default settings seems OK to me)
* Choose the right Port (in my case /dev/ttyUSB1)
* Via "Manage Libraries" Install the following libraries (and their dependencies, IDE will ask latest at BME280, just choose to "install all"):
** TFT_eSPI by Bodmer (https://github.com/Bodmer/TFT_eSPI) - *important use version 2.1.9*
** ESP8266 Influxdb by Tobias Schruerg, Influxdata (https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino)
** Adafruit ADS1X15 by Adafruit (https://github.com/adafruit/Adafruit_ADS1X15)
** Adafruit BME280 Library by Adafruit (https://github.com/adafruit/Adafruit_BME280_Library)
** LinkedList by Ivan Seidel (https://github.com/ivanseidel/LinkedList)
** ESP8266 and ESP32 OLED driver for SSD1306 displays by Thingpulse (https://github.com/ThingPulse/esp8266-oled-ssd1306)
** ArduinoJson by Benoit Blanchon (https://github.com/bblanchon/ArduinoJson)
** Websockets by Markus Sattler (https://github.com/Links2004/arduinoWebSockets)
** MD_MAX72XX by majicDesigns (https://github.com/MajicDesigns/MD_MAX72XX)
** Adafruit MPU6050 by Adafruit (https://github.com/adafruit/Adafruit_MPU6050)
* Set the parameters for the ST7735 display:
** go into the libraries/TFT_eSPI folder and open the file "User_Setup.h"
** change the driver (#define ST7735_DRIVER)
** set the size (#define TFT_WIDTH 128, #define TFT_HEIGHT 160)
** set the display type (#define ST7735_REDTAB - still not right though)
** comment the pin settings for NodeMCU
** set the pins under "...for ESP32 Dev Board..."
*** #define TFT_MISO 19
*** #define TFT_MOSI 23
*** #define TFT_SCLK 18
*** #define TFT_CS   15
*** #define TFT_DC    4
*** #define TFT_RST  -1 
** under Section 3 keep "LOAD_GLCD" and "LOAD_FONT2" but comment the rest
** comment "#define SMOOTH_FONT"
* open the sketch
* at the top right open the "serial monitor" (or use "Tools/Serial Monitor" in the menu) and set the speed to 115200. If this fails, the wrong port is probably used or permissions are missing. It is not possible to upload the binary unless this is fixed first.
* hit the "upload" button (or use "Sketch/Upload" in the menu)




