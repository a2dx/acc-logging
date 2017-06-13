#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2017-06-13 12:28:39

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#define LIS3DH_CS 1
#define CLICKTHRESHHOLD 50
#include <math.h>
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#include "simplot.h"
#include <SD.h>
#define SD_CS 4
#include "RTCZero.h"
void setup() ;
void loop() ;
void startRecord();
void setupAkkuCheck();
void alarmCheckbattery();
void calibration();
double radianToDegree(double rad);
void rotationX(int16_t x,int16_t y,int16_t z);
void rotationY(int16_t x,int16_t y,int16_t z);
void calcAngleEarth_ZX(int vectorEarth[3]);
void calcAngleEarth_ZY(int vectorEarth[3]);
double calcAngle(int vektor1[3],int vektor2[3]);
void calcAccMedian();
void setRecordState();
void setCalibrationState();
void setErrorState();
void setStandbyState();
void goSleep();
void wakeUp();
void setupInterrupt();
void setupLED();
void ledRecordOn();
void ledRecordOff();
void ledCalibrationOn();
void ledCalibrationOff();
void ledBatteryOn();
void ledBatteryOff();
void ledRedOn();
void ledRedOff();
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
void writeSensorIntDataToSD();
void writeSensorBinToArray();
void copyDatToCsv();
void deleteSDFile();
void setupRTC();
void waitForSetupRTCWithSerial();
String getCurrentTime();
String getCurrentDateTimeFilename();
String getCurrentDate();
String getCurrentDateAndTime();
float getBatteryVoltage();
void error(uint8_t errno) ;
void setTimerFrequency(int frequencyHz) ;
void startTimer(int frequencyHz) ;
void disableTimer() ;
void resetTimer() ;
void TC3_Handler() ;
void start_stopRecordInterrupt();
void startCalibration();

#include "acc_logging.ino"


#endif
