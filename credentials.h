#define WIFI_SSID "your-ssid"
#define WIFI_PASSWORD "your-password"
#define NTPSERVER1 "0.de.pool.ntp.org"
#define NTPSERVER2 "time.nist.gov"
#define NTPSERVER3 "0.se.pool.ntp.org"

// for cloud and local
#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_WRITE_PRECISION WritePrecision::S
#define INFLUXDB_BATCHSIZE 10
#define INFLUXDB_BUFFER 20

// for local v1
#define INFLUXDB_USER "username"
#define INFLUXDB_PASS "password"
#define INFLUXDB_DATABASE "influxdb"

// for cloud
#define INFLUXDB_ORG "normally your mail address"
#define INFLUXDB_TOKEN "your generated token"
#define INFLUXDB_BUCKET "your bucket"

// for slack connectivity
#define SLACK_SSL_FINGERPRINT "AC 95 5A 58 B8 4E 0B CD B3 97 D2 88 68 F5 CA C1 0A 81 E3 6E" // If Slack changes their SSL fingerprint, you would need to update this
#define SLACK_MAXCONNECTATTEMPTS 100

#define SLACK_BOT_TOKEN "your bot token" // Get token by creating new bot integration at https://my.slack.com/services/new/bot 
#define SLACK_BOT_ID "your bot id, look in debug for the ID"
#define SLACK_WEBHOOK "your bots webhook URL"
#define INFLUXDB_ROWNAME "your row name"

// Fingerprint of Certificate Authority of InfluxData Cloud 2 servers
// 2020.03.30 changed to letsencrypt, unable to figure out from where this fingerprint came
const char InfluxDbCloud2CAFingerprint[] PROGMEM = "9B:62:0A:63:8B:B1:D2:CA:5E:DF:42:6E:A3:EE:1F:19:36:48:71:1F";

//Linear calculation of AFR
#define AFRFACTOR 7.4

//Difference in hours to UTC (for display)
#define TZOFFSET 2

#define REALLY_UPLOAD 1
#define HAVE_INTERRUPTS 1
#define HAVE_SLACK 1

// I2C Devices
//#define HAVE_BME280   1
//#define HAVE_MPU6050  1
//#define HAVE_SSD1306   1
#define HAVE_ADS1115  1

// SPI Devices
#define HAVE_MAX6675  1
#define HAVE_ST7735   1
#define ST7735_START_LAYOUT driver
//#define HAVE_MCP3008  1
#define HAVE_MAX7219 1

//used for the alert
#define MAX7219_MICROBLINKS 12
//how long to display text message
#define MAX7219_TEXTTIME 20

//Pinout for ST7735 is in the "User_Setup.h" file in the TFT_eSPI library
//#define TFT_MISO 19
//#define TFT_MOSI 23
//#define TFT_SCLK 18
//#define TFT_CS   15  // Chip select control pin
//#define TFT_DC    4  // Data Command control pin

//HW & Pin for MAX7219
#define MAX7219_HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX7219_CS    16  // or SS
#define MAX7219_DEVICES   1

//Pin for MAX6675 CS
/*
#define MAX6675_1_CS 32
#define MAX6675_2_CS 33
#define MAX6675_3_CS 27
*/

// developing with a single sensor
#define MAX6675_1_CS 27
#define MAX6675_2_CS 27
#define MAX6675_3_CS 27

//Pin for RPM counter
#define RPMPIN 25

//Pin for revolution counter
#define REVPIN 26

//we get undefined values at start, limit what we write to the db
#define MAXRPM 50000
#define MAXREV 20000

// Data wire is connected to GPIO15
#define ONE_WIRE_BUS 27

// BME280 Calibration
#define SEALEVELPRESSURE_HPA (1013.25)

//MCP3008 SPI AD Converter
#define MCP3008_SPI_MAX_5V 3600000         ///< SPI MAX Value on 5V pin
#define MCP3008_SPI_MAX_3V 1350000         ///< SPI MAX Value on 3V pin
#define MCP3008_SPI_MAX MCP3008_SPI_MAX_5V ///< SPI MAX Value
#define MCP3008_SPI_ORDER MSBFIRST         ///<  SPI ORDER
#define MCP3008_SPI_MODE SPI_MODE0         ///< SPI MODE
#define MCP3008_VREF 5.1

//MCP3008 CS
#define MCP3008_CS 2



//ADS1115 I2C AD Converter
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
