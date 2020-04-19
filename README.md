# Read values with an ESP32 and store them in a cloudbased influxdb

This is code in development which reads various sensors connected to an ESP32 and stores them locally, and whenever there is Wifi connectivity, it dumps it off in an influxdb.

The code is poor in the following ways:

* no peer review
* mostly assuming sensors are always OK (for both SPI and I2C that is OK generally, but it still can freeze on bootup)
* superhappy fun time with global variables, break statements etc
* designed during creation
* possibly buggy, no real testing yet

## Data Storage

* Influxdatas https://www.influxdata.com/ cloud offer

## To-do list

* Split code on different cores, perhaps breaking out the sensor reading on the free core
* Fix issue when recovery from missing connectivity, until local buffer has been written away no new values are being read
* A ton of other things
* be more efficient with RAM (datatypes in struct)

# Changelog

* 20200419: Fixed README, local time in display
* 20200403: Fix certificate Verification from influx, added Slack support, reorganized code, added howto
* 20200313: Removed own linked list / rely on influxdb client, adaptive loop wait time, improved reconnect after wifi loss, removed w1 sensors since they are slow, changed to ADS1115 A/D Converter, added RPM and Velocity pulse inputs, configurable pinout (within limits of hardware). Code changed to be modular if not all sensors are available. Added 2xMAX6675 high temperature sensor code (can easily be extended to more). COLOUR super-cyber-flashy TFT output with direct RPM / AFR values

# Slack integration

Write message directly to slack bot. Commands understood:

* text (string up to 50 chars) = Displays this text in ST7735 display for 20 seconds
* layout (1 or 2) = switch display between "driver" and "original"
* disp up = shows an up arrow on MAX7219 LED Matrix
* disp down = shows a down arrow on MAX7219 LED Matrix
* disp right = shows a right arrow on MAX7219 LED Matrix
* disp left = shows a left arrow on MAX7219 LED Matrix
* alert = blinks the MAX7219 as in "attention"

If the slack bot doesn't understand it replies back with the syntax. If it understands, it writes back what it will do prefixed with "ACK:".

## Pinout / Wiring

(for ST7735 display: Pinout is defined in User_Setup.h)

* ESP GPIO18 = SPI CLK (MCP3008, MAX6675, ST7735, MAX7219)
* ESP GPIO19 = SPI Data MISO (MCP3008, MAX6675, MAX7219)
* ESP GPIO23 = SPI Data MOSI (MCP3008, ST7735)
* ESP GPIO32  = SPI CS for MAX6675
* ESP GPIO33  = SPI CS for MAX6675
* ESP GPIO27  = SPI CS for MAX6675
* ESP GPIOX  = SPI CS for MCP3008
* ESP GPIO15 = SPI CS for ST7735
* ESP GPIO16 = SPI CS for MAX7219
* ESP GPIO4  = Data Command for ST7735
* ESP GPIO22 = I2C CLK (MPU6050, BME280, SSD1306, ADS1115)
* ESP GPIO21 = I2C SDA (MPU6050, BME280, SSD1306, ADS1115)
* ESP GPIO26 = Wheel Revolution Pulse input, ground via 10kOhm
* ESP GPIO25 = Ignition Pulse input (RPM), ground via 10kOhm
* +3V3 via 330Ohm R = RESET for ST7735

The MCP3008 and ADS1115 have a +5 Vin (Vin cannot be lower than Vref), the rest is running on 3.3 V

