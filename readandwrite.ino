/* Functions for reading the sensors 
 * and writing the info back to influxdb
 */


int writeAllData(Measurement *Meas)
{
  // Write to the DB
  row.clearFields();
  row.clearTags();
#ifdef HAVE_BME280
  row.addField("bme280_temperature", Meas->bme280_temperature);
  row.addField("bme280_humidity", Meas->bme280_humidity);
  row.addField("bme280_airpressure", Meas->bme280_airpressure);
  row.addField("readtime_bme280", Meas->readtime_bme280);
#endif
#ifdef HAVE_MPU6050
  row.addField("mpu6050_accel_x", Meas->mpu6050_accel_x);
  row.addField("mpu6050_accel_y", Meas->mpu6050_accel_y);
  row.addField("mpu6050_accel_z", Meas->mpu6050_accel_z);
  row.addField("mpu6050_gyro_x", Meas->mpu6050_gyro_x);
  row.addField("mpu6050_gyro_y", Meas->mpu6050_gyro_y);
  row.addField("mpu6050_gyro_z", Meas->mpu6050_gyro_z);
  row.addField("mpu6050_temperature",Meas->mpu6050_temperature);
  row.addField("readtime_mpu6050", Meas->readtime_mpu6050);
#endif
#ifdef HAVE_W1
  row.addField("w1_tempC", Meas->w1_tempC);
  row.addField("readtime_w1_temp", Meas->readtime_w1_temp);
#endif
#ifdef HAVE_MAX6675
  row.addField("max6675_1_tempC", Meas->max6675_1_tempC);
  row.addField("max6675_2_tempC", Meas->max6675_2_tempC);
  row.addField("max6675_3_tempC", Meas->max6675_2_tempC);
/*  
  row.addField("readtime_max6675_1", Meas->readtime_max6675_1);
  row.addField("readtime_max6675_2", Meas->readtime_max6675_2);
  row.addField("readtime_max6675_3", Meas->readtime_max6675_3);
*/
#endif
#ifdef HAVE_MCP3008
  row.addField("readtime_mcp3008", Meas->readtime_mcp3008);
#endif
#ifdef HAVE_ADS1115
  row.addField("readtime_ads1115", Meas->readtime_ads1115);
#endif
#ifdef HAVE_INTERRUPTS
  row.addField("rpm", Meas->rpm);
  row.addField("rpmfreq", Meas->rpmfreq);
  row.addField("revolutions", Meas->revolutions);
  row.addField("revfreq", Meas->revfreq);
  row.addField("readtime_interrupts", Meas->readtime_interrupts);
#endif
  row.addField("rssi", Meas->rssi);
  row.addField("freeheap", Meas->freeheap);
  row.addField("wlstatus", Meas->wlanstatus);
  row.addField("afrvoltage0", Meas->afrvoltage0);
  row.addField("analog0", Meas->analog0);
  row.addField("analog1", Meas->analog1);
  row.addField("analog2", Meas->analog2);
  row.addField("analog3", Meas->analog3);
  row.addField("calculatedafr", Meas->calculatedafr);
  row.addField("readtime_esp32", Meas->readtime_esp32);
  row.addField("sensorreadtime", Meas->sensorreadtime);
  row.addField("lastwritetime", Meas->lastwritetime);
  row.addTag("device", "ESP32");
  row.setTime(Meas->timestamp);
  startwrite=millis();
// Write to buffered influxclient
#ifdef REALLY_UPLOAD
  influxclient.writePoint(row);
//  Serial.print("Full buffer: ");
//  Serial.println(influxclient.isBufferFull() ? "Yes" : "No");
  if (WiFi.status() == WL_CONNECTED && influxclient.isBufferFull()) {
    Serial.println("Flushing data into InfluxDB");
    if (!influxclient.flushBuffer()) {
      Serial.print("InfluxDB flush failed: ");
      Serial.println(influxclient.getLastErrorMessage());
    }
  }
#endif
  return millis() - startwrite;
}


void readAllData(Measurement *Meas, int lastwritetime) 
{   
  //Timestamp of the measurement
  time(&Meas->timestamp);
  startMillis=millis();
  unsigned long startmeasMillis=millis();
#ifdef HAVE_W1
  // W1 Sensor
//  && M.w1_tempC != DEVICE_DISCONNECTED_C
  startmeasMillis=millis();
  sensors.requestTemperatures();
  Meas->w1_tempC = sensors.getTempC(W1_SENSORADDR_1);
  Meas->readtime_w1_temp=millis()-startmeasMillis;
#endif

#ifdef HAVE_MAX6675
  // SPI High temp sensor
  startmeasMillis=millis();
  Meas->max6675_1_tempC = readSPImax6675(MAX6675_1_CS);
  // for statistics/debugging, can be removed
  Meas->readtime_max6675_1=millis()-startmeasMillis;

  startmeasMillis=millis();
  Meas->max6675_2_tempC = readSPImax6675(MAX6675_2_CS);
  // for statistics/debugging, can be removed
  Meas->readtime_max6675_2=millis()-startmeasMillis;

  startmeasMillis=millis();
  Meas->max6675_3_tempC = readSPImax6675(MAX6675_3_CS);
  // for statistics/debugging, can be removed
  Meas->readtime_max6675_3=millis()-startmeasMillis;
#endif

  // ESP32
  startmeasMillis=millis();
  Meas->freeheap = ESP.getFreeHeap();
  Meas->wlanstatus = WiFi.status();
  // Assume connectivity is lost
  Meas->rssi = -120;
  if (Meas->wlanstatus == WL_CONNECTED) {
    Meas->rssi = WiFi.RSSI();
  }
  Meas->readtime_esp32=millis()-startmeasMillis;

#ifdef HAVE_INTERRUPTS
  //Calculate RPM, based on rpmpulses
  //Read diff in time and pulses
  startmeasMillis=millis();
  unsigned int diffpulses=rpmpulses-oldrpmpulses;
  oldrpmpulses=rpmpulses;
  unsigned int diffmillis=millis()-rpmmillis;
  rpmmillis=millis();
  Meas->rpmfreq = (float(diffpulses*1000)/diffmillis);
  // We have Frequency, RPM is simply 60*Freq
  Meas->rpm = int(60 * float(diffpulses*1000)/diffmillis);
  if (Meas->rpm > MAXREV) {
    Meas->rpm=0;  
  }
  //Calculate Revolution frequency, based on rpmpulses
  //Read diff in time and pulses
  startmeasMillis=millis();
  diffpulses=revolutionpulses-oldrevolutionpulses;
  oldrevolutionpulses=revolutionpulses;
  diffmillis=millis()-revolutionmillis;
  revolutionmillis=millis();
  Meas->revfreq = (float(diffpulses*1000)/diffmillis);
  // We have Frequency, RPM is simply 60*Freq
  Meas->revolutions = int(60 * float(diffpulses*1000)/diffmillis);
  if (Meas->revolutions > MAXREV) {
    Meas->revolutions=0;  
  }
  Meas->readtime_interrupts=millis()-startmeasMillis;
#endif

#ifdef HAVE_MCP3008
  //AD Converter 0-5 Volts
  startmeasMillis=millis();
  Meas->analog0 = readSPImcp3008(0);
  //The value is 0-1023 in reference to Vref, which is 5.1 Volt
  Meas->afrvoltage0 = ((Meas->analog0 * MCP3008_VREF) / 1024);
  Meas->readtime_mcp3008=millis()-startmeasMillis;
#endif

#ifdef HAVE_ADS1115
  //AD Converter 0-5 Volts, 16 bit resolution
  startmeasMillis=millis();
  adc0=ads1115.readADC_SingleEnded(0);
  adc1=ads1115.readADC_SingleEnded(1);
  adc2=ads1115.readADC_SingleEnded(2);
  adc3=ads1115.readADC_SingleEnded(3);
  Meas->analog0 = int(adc0);
  Meas->analog1 = int(adc1);
  Meas->analog2 = int(adc2);
  Meas->analog3 = int(adc3);
  // 1 bit = 0.1875mV
  Meas->afrvoltage0 = (adc0 * 0.1875)/1000.0;
  // 1 to 3 volt; simply multiply with AFRFACTOR
  Meas->calculatedafr = Meas->afrvoltage0 * AFRFACTOR;
  Meas->readtime_ads1115=millis()-startmeasMillis;
#endif

#ifdef HAVE_BME280
  //BME280
  startmeasMillis=millis();
  Meas->bme280_temperature = bme.readTemperature();
  Meas->bme280_humidity = bme.readHumidity();
  Meas->bme280_airpressure = bme.readPressure() / 100.0F;
  Meas->bme280_altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Meas->readtime_bme280=millis()-startmeasMillis;
#endif

#ifdef HAVE_MPU6050
  //MPU6050
  if(!mpu6050failed) {
    startmeasMillis=millis();
    sensors_event_t a, g, mputemp;
    // acceleration and gyro register mismatch?
    mpu.getEvent(&g, &a, &mputemp);
    Meas->mpu6050_accel_x = a.acceleration.x;
    Meas->mpu6050_accel_y = a.acceleration.y;
    Meas->mpu6050_accel_z = a.acceleration.z;
    Meas->mpu6050_gyro_x = g.gyro.x;
    Meas->mpu6050_gyro_y = g.gyro.y;
    Meas->mpu6050_gyro_z = g.gyro.z;
    Meas->mpu6050_temperature = mputemp.temperature;
    Meas->readtime_mpu6050=millis()-startmeasMillis;
  }
#endif
  
  Meas->lastwritetime = lastwritetime;
  Meas->sensorreadtime = millis() - startMillis;

//debug for display
/*
  Meas->rpm=12345;
  Meas->afrvoltage0=3;
  Meas->calculatedafr=Meas->afrvoltage0 * AFRFACTOR;
*/  
  return;
}
