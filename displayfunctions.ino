/*
 * Code handling display. the code for SSD1306 is an old rest.
 * 20200405 focus on the ST7735
 */

void printOnDisplay(Measurement *Meas) {
//  struct tm timeinfo;
  int a;
  unsigned long startmeasMillis=millis();
  localTimeInTimeString();
#ifdef SSD1306
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, timestring);
  display.drawString(0, 10,"Heap " + String(ESP.getFreeHeap()));
  // strength, formula stolen
  unsigned int strength = 120 - ((5/3) * abs(Meas->rssi));
  // draw the percentage as String
  display.drawProgressBar(0, 22, 120, 10, strength);
//  display.setTextAlignment(TEXT_ALIGN_CENTER);
//  display.drawString(64,42, "RSSI: " + String(Meas->rssi));
//  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 34,"x:" + String(Meas->mpu6050_gyro_x) + " y:" + String(Meas->mpu6050_gyro_y) + " z:" + String(Meas->mpu6050_gyro_z));
  display.drawString(0, 44,String(Meas->w1_tempC) + "C " + String(Meas->spi_tempC) + "C " + String(Meas->mpu6050_temperature) + "C");
  display.drawString(0, 54,String(Meas->bme280_temperature) + "C " + String(Meas->bme280_humidity) + " rel%");
  display.display();
#endif
#ifdef HAVE_MAX7219
  max7219.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/8);
  max7219.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
//Logic for the LED Matrix
  switch(max7219_current_command)
  {
    case none:
        max7219.control(MD_MAX72XX::TEST, MD_MAX72XX::OFF);
        max7219.clear();
        break;
    case rightarrow:
        max7219.setBuffer(COL_SIZE-1, COL_SIZE, arrow);
        break;
    case leftarrow:
        max7219.setBuffer(COL_SIZE-1, COL_SIZE, arrow);
        max7219.transform(MD_MAX72XX::TFLR);
        break;
    case uparrow:
        max7219.setBuffer(COL_SIZE-1, COL_SIZE, arrow);
        max7219.transform(MD_MAX72XX::TFLR);
        max7219.transform(MD_MAX72XX::TRC);
        break;
    case downarrow:
        max7219.setBuffer(COL_SIZE-1, COL_SIZE, arrow);
        max7219.transform(MD_MAX72XX::TRC);
        break;
    case alert:
        max7219.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY);
        if(max7219_alertcounter > 1) {
          for (a=0; a<MAX7219_MICROBLINKS; a++) {
            if (a%2==0) {
              max7219.control(MD_MAX72XX::TEST, MD_MAX72XX::ON);
              delay(20);
            }
            else {
              max7219.control(MD_MAX72XX::TEST, MD_MAX72XX::OFF);
              delay(50);
            }
          }
          max7219_alertcounter--;
        }
        else{
          max7219_current_command=none;
        }
    default:
        max7219.control(MD_MAX72XX::TEST, MD_MAX72XX::OFF);
        max7219.clear();    
        break;
  }
#endif
#ifdef HAVE_ST7735
  if(max7219_current_command == text) {
    if (!clearedscreen_for_slacktext) {
      tft.fillScreen(TFT_BLACK);
      clearedscreen_for_slacktext=true;
    }
    tft.setTextWrap(false,false);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 0, 1);
    tft.setTextFont(1);
    tft.print(timestring);
    tft.println("                           ");
    tft.setTextColor(TFT_WHITE,TFT_DARKGREEN);
//    tft.setTextFont(2);
    tft.setTextSize(2);
    tft.setTextWrap(true,true);
    tft.println(slack_textmessage);
    slack_textcounter--;
    if (slack_textcounter < 1) {
      max7219_current_command=none;
      tft.fillScreen(TFT_BLACK);
      tft.setTextFont(1);
      clearedscreen_for_slacktext=false;
    }  
  }
  else {
    switch (st7735_current_layout)
    {
      case original:
        tft.setTextWrap(false,false);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.print(timestring);
        tft.println("                           ");
        tft.print("Heap " + String(ESP.getFreeHeap()) + "  RSSI " + String(Meas->rssi));
        tft.println("                           ");
        tft.print("RPM:" + String(Meas->rpmfreq) + "Hz, AFR:"+ String(Meas->afrvoltage0) + "V");
        tft.println("                           ");
        tft.setTextColor(TFT_BLACK, TFT_YELLOW);
        sprintf(egpdisplaystring,"EGP: %03dC / %03dC / %03dC", int(Meas->max6675_1_tempC), int(Meas->max6675_2_tempC), int(Meas->max6675_3_tempC));
        tft.print(egpdisplaystring);
        tft.println("                           ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print("MPU  " + String(Meas->mpu6050_temperature) + "C BME  " + String(Meas->bme280_temperature) +"C");
        tft.println("                           ");
        tft.print(String(Meas->bme280_humidity) + " rel%  " + String(Meas->bme280_airpressure) + " hPa");
        tft.println("                           ");
        tft.println("                           ");
        tft.print("RPM:");
//        tft.setTextFont(2);
        tft.setTextSize(2);
        tft.print(" " + String(Meas->rpm));
        tft.println("              ");
//        tft.setTextFont(1);
        tft.setTextSize(1);
// We should catch HEAT < 0.5 V and ERR > 4 V
        tft.print("\nAFR:");
//        tft.setTextFont(2);
        tft.setTextSize(2);
        if (Meas->afrvoltage0 < 0.5) {
          tft.setTextColor(TFT_RED, TFT_WHITE);
          tft.print(" HEAT");    
        }
        else if (Meas->afrvoltage0 > 4.0) {
          tft.setTextColor(TFT_RED, TFT_WHITE);
          tft.print(" ERR");
        }
        else {
          tft.print(" " + String(Meas->calculatedafr));
        }
        tft.println("              ");
//        tft.setTextFont(1);
        tft.setTextSize(1);
        tft.setCursor(0, 10*10, 1);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.print("Sensors time: " + String(Meas->sensorreadtime) + " ms");
        tft.println("                           ");
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.print("Network time: " + String(Meas->lastwritetime) + " ms");
        tft.println("                           ");
        tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        tft.print("Display time: " + String(millis() - startmeasMillis) + " ms");
        tft.println("                           ");
        break;
      case driver:
        tft.setTextWrap(false,false);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.print(timestring);
        tft.println("                           ");
        tft.println("                           ");
        // We should round RPM to the next 100, for now we simply truncate
        roundedrpm = ((Meas->rpm)/100) * 100;
        tft.print("RPM:");
        tft.setTextFont(4);
//        tft.setTextSize(2);
        tft.print(" " + String(roundedrpm));
        tft.println("              ");
        tft.setTextFont(1);
//        tft.setTextSize(1);
        // We should catch HEAT < 0.5 V and ERR > 4 V
        tft.print("\nAFR:");
        tft.setTextFont(4);
//        tft.setTextSize(2);
        if (Meas->afrvoltage0 < 0.5) {
          tft.setTextColor(TFT_RED, TFT_WHITE);
          tft.print(" HEAT");    
        }
        else if (Meas->afrvoltage0 > 4.0) {
          tft.setTextColor(TFT_RED, TFT_WHITE);
          tft.print(" ERR");
        }
        else {
        // We should round AFR to the closest .1, for now we simply truncate
          sprintf(roundedafr,"%.1f", Meas->calculatedafr);
          tft.print(" ");
          tft.print(roundedafr);
        }
        tft.println("              ");
        tft.setTextFont(1);
//        tft.setTextSize(1);
//        tft.print("\nEGT:");
//        tft.setTextFont(2);
        tft.println("              ");
        tft.setTextSize(2);
        sprintf(egpdisplaystring,"%03d %03d %03d`C", int(Meas->max6675_1_tempC), int(Meas->max6675_2_tempC), int(Meas->max6675_3_tempC));
//        tft.print(" ");
        tft.print(egpdisplaystring);
        tft.println("              ");
        tft.setTextFont(1);
        tft.setTextSize(1);
        tft.setCursor(0, 12*10, 1);
        a=(Meas->sensorreadtime + Meas->lastwritetime + (millis() - startmeasMillis));
        tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        tft.print("Total time: " + String(a) + " ms");
        tft.println("                           ");
        break;
      default:
        // This should never happen, reset layout and wait for the next call
        st7735_current_layout=original;
      }
  }
#endif
}

void displayWhileWaitingForTime () {
#ifdef SSD1306
  display.clear();
  display.drawString(0, 10,String(WIFI_SSID) + ": " + String(WiFi.RSSI()) + "dBm");
  display.drawString(0, 20,"Heap : " + String(ESP.getFreeHeap()));
  display.drawString(0, 30,"Time Sync attempt " + String(timeiteration) + ".");
  display.drawString(0, 40,"Epoch : " + String(setuptime));
  display.display();
#endif
}


void printStringOnDisplay(char* inputstring) {
  localTimeInTimeString();
#ifdef HAVE_SSD1306
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, timestring);
  display.drawString(0, 20, inputstring);
  display.display();
#endif
#ifdef HAVE_ST7735
  // without wrap max strlen is "0 1 2 3 4 5 6 7 8 9 0 1 2 " = 26 chars with font 1
  // to avoid refreshing the whole screen we pad the string to that (otherwise a long
  // string will remain on the screen
  // cursor position is updated by the library
  int i;
  tft.setCursor(0, 0, 1);
  tft.print(timestring);
  tft.println("                           ");
  tft.print(inputstring);
  tft.println("                           ");
#endif
  Serial.println(timestring);
  Serial.println(inputstring);
}
void localTimeInTimeString() {
  time_t localepoch;
  time(&localepoch);
  localepoch = localepoch + (TZOFFSET * 60 * 60);
  sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", year(localepoch), month(localepoch), day(localepoch), hour(localepoch), minute(localepoch), second(localepoch));
}
