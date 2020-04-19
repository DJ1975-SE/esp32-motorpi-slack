/*
 * Connect to Wifi and set time
 * 
 * display time and rssi on SSD1306 I2C display
 * 
 * Read temperature from DS18B20 via w1
 * Read temperature from MAX6675 via SPI
 * 
 * Write data back when wlan available
 * 
 * Scanning...
 * I2C device found at address 0x3C - display
 * I2C device found at address 0x68 - gyrometer MPU6050
 * I2C device found at address 0x76 - bme280
 * I2C device found at address 0x48 - ADS1115 with grounded ADDR
 * 
 * 
 * 
 * 
 * ToDo: 
 * Break out sensor reading in own thread
 * Write multiple/batched
 * bidirectional communication / slack?
 */

#define DEVICE "ESP32"

#include <WiFi.h>
#include "TimeLib.h"
#include <Adafruit_ADS1015.h>
#include "SSD1306Wire.h"        
#include <TFT_eSPI.h>           // Hardware-specific library
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <SPI.h>
#include <LinkedList.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include "credentials.h"
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <MD_MAX72xx.h>

// should be adjustable with a potentiometer
const int   loopWait = 1000;

// MAX6675 max SPI Clock is 4.3 MHz but does not seem to work, ST7735 runs by default on 27 MHz
// static const int spiClk = 1000000; // 1 MHz
static const int spiClk = 2000000; 

struct Measurement {
  int rssi;
  unsigned int freeheap;
  float w1_tempC;
  float max6675_1_tempC;
  float max6675_2_tempC;
  float max6675_3_tempC;
  float rpmfreq;
  float revfreq;
  unsigned int rpm;
  unsigned int revolutions;
  float bme280_airpressure;
  float bme280_humidity;
  float bme280_temperature;
  float bme280_altitude;
  float mpu6050_accel_x;
  float mpu6050_accel_y;
  float mpu6050_accel_z;
  float mpu6050_gyro_x;
  float mpu6050_gyro_y;
  float mpu6050_gyro_z;  
  float mpu6050_temperature;
  unsigned int analog0;
  unsigned int analog1;
  unsigned int analog2;
  unsigned int analog3;
  // We assume 0-5 volt
  float afrvoltage0;
  float voltage1;
  float voltage2;
  float voltage3;
  float calculatedafr;
  unsigned int sensorreadtime;
  wl_status_t wlanstatus;
  time_t timestamp;
  // debugging
  unsigned int readtime_w1_temp;
  unsigned int readtime_bme280;
  unsigned int readtime_mpu6050;
  unsigned int readtime_max6675_1;
  unsigned int readtime_max6675_2;
  unsigned int readtime_max6675_3;
  unsigned int readtime_mcp3008;
  unsigned int readtime_ads1115;
  unsigned int readtime_esp32;
  unsigned int readtime_interrupts;
  unsigned int writetime_display;
  unsigned int lastwritetime;
};

unsigned int roundedrpm=0;
char roundedafr[5];
unsigned long rpmpulses=0;
unsigned long revolutionpulses=0;
unsigned long oldrpmpulses=0;
unsigned long oldrevolutionpulses=0;

Adafruit_BME280 bme; // I2C
unsigned bme280status;

// MPU6050
Adafruit_MPU6050 mpu;
bool mpu6050failed=false;

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

#ifdef HAVE_W1
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
#endif

char timestring[50];
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

#ifdef HAVE_SSD1306
// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h
#endif
#ifdef HAVE_ST7735
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
#endif
#ifdef HAVE_ADS1115
//ADDR Pin - GND = I2C Address 0x48
Adafruit_ADS1115 ads1115(0x48);  /* Use this for the 16-bit version */
int16_t adc0;
int16_t adc1;
int16_t adc2;
int16_t adc3;
#endif

#ifdef HAVE_MAX7219
MD_MAX72XX max7219 = MD_MAX72XX(MAX7219_HARDWARE_TYPE, MAX7219_CS, MAX7219_DEVICES);                      // SPI hardware interface
uint8_t arrow[COL_SIZE] =
{
    0b00001000,
    0b00011100,
    0b00111110,
    0b01111111,
    0b00011100,
    0b00011100,
    0b00011100,
    0b00000000
};
#endif
enum max7219command {
  none,
  rightarrow,
  leftarrow,
  uparrow,
  downarrow,
  alert,
  text
};
max7219command max7219_current_command = none;

enum st7735_layout {
  original,
  driver,
  technical
};

st7735_layout st7735_current_layout = ST7735_START_LAYOUT;

// Create the influxdb object
//InfluxDBClient influxclient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
InfluxDBClient influxclient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// This is the only place we save data, buffer is in the influx class
Measurement curM;

// amateurish way to figure out if we have an ntp sync
# define FEBRUARY2020 1581627570
unsigned int timeiteration = 0;
time_t setuptime;

// telemetry
unsigned long startMillis=millis();

Point row(INFLUXDB_ROWNAME);

void IRAM_ATTR rpminterruptserviceroutine() {
  rpmpulses++;
}
void IRAM_ATTR revolutioninterruptserviceroutine() {
  revolutionpulses++;
}
unsigned long int rpmmillis;
unsigned long int revolutionmillis;
unsigned long int lastwritetime=0;
unsigned long int startwrite;
int startloop;
int waittime;
unsigned int last_writetime=0;

bool slack_connected = false;
unsigned long slack_lastPing = 0;
unsigned long slack_startconnect = 0;
int slack_attempts;
long slack_nextCmdId = 1;
WebSocketsClient slack_webSocket;

bool clearedscreen_for_slacktext=false;

bool max7219_blink = false;
int slack_textcounter=0;
int max7219_alertcounter=0;
int max7219_curcol=0;
char slack_textmessage[50];
char egpdisplaystring[30];

void setup() {
    Serial.begin(115200);
    delay(1);
    time(&setuptime);
    // Initialising the UI will init the display too.
    Serial.print(String(setuptime));
    Serial.println("Initializing display");
#ifdef HAVE_SSD1306
    display.init();
    display.flipScreenVertically();
#endif
#ifdef HAVE_ST7735
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 0, 1);
    tft.setTextWrap(false,false);
#endif
    printStringOnDisplay("Connecting to Wifi");
    // connect to the wifi network

    Serial.println();
    time(&setuptime);
    Serial.print(String(setuptime));
    Serial.print(" Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    printStringOnDisplay("Wifi connected");
    Serial.println("");
    time(&setuptime);
    Serial.print(String(setuptime));
    Serial.println(" WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("ESP Mac Address: ");
    Serial.println(WiFi.macAddress());

    // there is no point in writing data unless we have a proper time.
    // halt until good time
    printStringOnDisplay("Setting time ");
    configTime(gmtOffset_sec, daylightOffset_sec, NTPSERVER1, NTPSERVER2, NTPSERVER3);
    time(&setuptime);
    while (setuptime < FEBRUARY2020) {
      displayWhileWaitingForTime();
      timeiteration++;
      Serial.println("Time attempt " + String(timeiteration) + " " + String(setuptime));
      configTime(gmtOffset_sec, daylightOffset_sec, NTPSERVER1, NTPSERVER2, NTPSERVER3);
      time(&setuptime);
      printStringOnDisplay("Waiting for time");
      delay(1000);
    } 
#ifdef HAVE_MAX7219
    pinMode(MAX7219_CS, OUTPUT); //VSPI SS for first SPI device (MAX6675)
    max7219.begin();
    max7219.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/4);
    max7219.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
    max7219.clear();
#endif
    //configure the influxdb connection, seconds resolution and write 30 datapoints each time
    printStringOnDisplay("connecting to influx");
    influxclient.setWriteOptions(INFLUXDB_WRITE_PRECISION,INFLUXDB_BATCHSIZE,INFLUXDB_BUFFER);

#ifdef HAVE_W1
    // initialize w1 bus
    printStringOnDisplay("init w1 bus, ");
    sensors.begin();
#endif
#ifdef HAVE_BME280
    printStringOnDisplay("init BME280 i2c, 0x76, ");
    // default address for the BME280 is in this library 0x60
    bme280status = bme.begin(0x76);
    if (!bme280status) {
      printStringOnDisplay("BME280 failed, ");
    }
#endif
#ifdef HAVE_ADS1115
    printStringOnDisplay("init ADS1115 i2c, 0x48, ");
    ads1115.begin();
#endif
#ifdef HAVE_MPU6050
    // initialize MPU6050
    printStringOnDisplay("init MPU6050 i2c, 0x68");
    if (!mpu.begin()) {
      printStringOnDisplay("MPU6050 failed");
      mpu6050failed=true;
    }
    MPU6050calibration();
#endif
    //initialise instance of the SPIClass attached to VSPI (perhaps we need HSPI later)
    printStringOnDisplay("init SPI bus ");
    vspi = new SPIClass(VSPI);
    //initialise vspi with default pins
    //SCLK = 18, MISO = 19, MOSI = 23, SS = 5
    vspi->begin();
    //set up slave select pins as outputs as the Arduino API
    //doesn't handle automatically pulling SS low
    pinMode(MAX6675_1_CS, OUTPUT); //VSPI SS for first SPI device (MAX6675)
    pinMode(MAX6675_2_CS, OUTPUT); //VSPI SS for first SPI device (MAX6675)
    pinMode(MAX6675_3_CS, OUTPUT); //VSPI SS for first SPI device (MAX6675)
//    pinMode(2, OUTPUT); //VSPI SS for second SPI device


    pinMode(RPMPIN, INPUT); 
    pinMode(REVPIN, INPUT);
#ifdef HAVE_INTERRUPTS
    // create interrupt routine for RPMs
    attachInterrupt(RPMPIN, rpminterruptserviceroutine, RISING);
    // create interrupt routine for Revolutions
    attachInterrupt(REVPIN, revolutioninterruptserviceroutine, RISING);
#endif
#ifdef HAVE_SLACK
    slack_startconnect=millis();
    printStringOnDisplay("connecting to slack");    
    slack_connected = connectToSlack();
    slack_attempts = 0;
    while (!slack_connected && slack_attempts<SLACK_MAXCONNECTATTEMPTS){
          printStringOnDisplay("waiting for slack");
          Serial.println("waiting for slack:" + String(int(millis()-slack_startconnect)));
          slack_webSocket.loop();
          delay(500);
          slack_attempts++;
    }
#endif
    printStringOnDisplay("setup finished");
    rpmmillis=millis();
    oldrpmpulses=rpmpulses;
    revolutionmillis=millis();
    oldrevolutionpulses=revolutionpulses;
}

void loop() {
  startloop=millis();
  readAllData(&curM, last_writetime);
  printOnDisplay(&curM);
  last_writetime=writeAllData(&curM);
  if (curM.wlanstatus != WL_CONNECTED) {
    Serial.print("Wifi lost, WiFi.status = ");
    Serial.println(wl_status_to_string(curM.wlanstatus));
    printStringOnDisplay("Wifi Lost, reconnecting");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  else 
  {
#ifdef HAVE_SLACK
    slack_webSocket.loop();
#endif
    // We have connectivity, sync time and flush the buffer

//    configTime(gmtOffset_sec, daylightOffset_sec, NTPSERVER);
    // pop all measurements and write them to the db, verify that we have connectivity while we do it
    // since we pop first we lose a measurement if the wifi is lost while emptying the list
//    printAllInfo(&curM);
  }

  waittime=loopWait - (millis()-startloop);
  if (waittime > 0) {
    delay(waittime);
  }
}
