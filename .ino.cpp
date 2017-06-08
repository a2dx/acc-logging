#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2017-06-08 15:15:37

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#define LIS3DH_CS 15
#define CLICKTHRESHHOLD 50
#include "simplot.h"
#include <SD.h>
#define SD_CS 4
#include "RTCZero.h"
void setup() ;
void loop() ;
void setupLED();
void ledGreenOn();
void ledGreenOff();
void plotSerial(int16_t plotValue);
void plot3Serial(int16_t plotValue1,int16_t plotValue2,int16_t plotValue3);
void setupSerial();
void setupLIS3DHClick();
void displayClickSerial();
void setupLIS3DH();
void readAcc();
void setupSD();
void writeSDStringln(String str);
void readSD();
void safeBufferCounterToSD();
void writeSensorStrDataToSD();
void deleteSDFile();
void setupRTC();
String getCurrentTime();
String getCurrentDate();
String getCurrentDateAndTime();
float getBatteryVoltage();

#include "acc_logging.ino"


#endif
