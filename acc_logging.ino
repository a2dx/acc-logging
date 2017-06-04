//LIS3DH
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#define LIS3DH_CS 15
#define CLICKTHRESHHOLD 50
sensors_event_t myEvent;

//Feather M0
//#define VBATPIN A7    // Battery Voltage on Pin A7

//Serial Plotter
#include "simplot.h"


//SD Card
#include <SD.h>
#define SD_CS 4
File dataFile;
String fileName = "accLog.txt";

//Real Time Clock
#include "RTCZero.h"
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 13;
const byte month = 6;
const byte year = 17;

uint8_t currHour = hours;
uint8_t currMinute = minutes;
uint8_t currSecond = seconds;

uint8_t currYear = year;
uint8_t currMonth = month;
uint8_t currDay = day;



//Initialisation
Adafruit_LIS3DH accSensor = Adafruit_LIS3DH(LIS3DH_CS);
uint8_t click;
#if defined(ARDUINO_ARCH_SAMD)
#define Serial Serial
#endif


//Variables
String xString = "";
String yString = "";
String zString = "";
uint32_t counter = 0;
uint32_t myNumber = 1000;  //some value for the counter
int16_t myX = 0;
int16_t myY = 0;
int16_t myZ = 0;


void setup() {

	setupSerial();
	setupLIS3DH();
	setupLIS3DHClick();
	setupSD();
	setupRTC();



	deleteSDFile();
	dataFile = SD.open(fileName, FILE_WRITE);
	readSD();


	//Green LED, setup finished
	digitalWrite(8, HIGH);
	pinMode(8, OUTPUT);



}

void loop() {
	//---------------------------------
	safeBufferCounterToSD();







	//---------------------------------
}





//Plot in Sloeber via Serial USB
void plotSerial(int16_t plotValue){
	plot1(Serial, plotValue);
}

//Plot 3 values in Sloeber via Serial USB
void plot3Serial(int16_t plotValue1,int16_t plotValue2,int16_t plotValue3){
	plot3(Serial, plotValue1,plotValue2,plotValue3);
}


//Setup serial
void setupSerial(){
	Serial.begin(115200);
	// wait for serial port to connect.
	while (!Serial);
}

//Setup LIS3DH CLick
void setupLIS3DHClick(){
	accSensor.setClick(2, CLICKTHRESHHOLD);
}

//Display Tick via serial USB
void displayClickSerial(){
	click = accSensor.getClick();
	if (click == 0) return;
	if (! (click & 0x30)) return;
	Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
	if (click & 0x10) Serial.print(" single click");
	if (click & 0x20) Serial.print(" double click");
	Serial.println();
}




//Setup for +-16 Range and 400 MHz Datarate
void setupLIS3DH(){


	if (! accSensor.begin(0x18)) {   // number is for i2c Initialisation
		Serial.println("Couldnt start");   //--------------xxxxxxx--------
		while (1);
	}

	accSensor.setRange(LIS3DH_RANGE_16_G);
	accSensor.setDataRate(LIS3DH_DATARATE_400_HZ);
	Serial.println("---LIS3DH initialized---");

}


void setupSD(){
	if (!SD.begin(SD_CS)) {
		Serial.println("Card failed, or not present");
		return;
	}
	Serial.println("---SD Card initialized----");
}

void writeSDStringln(String str){
	dataFile.println(str);
}


void readSD(){
	dataFile.close();
	dataFile = SD.open(fileName);
	if (dataFile) {
		Serial.println("-------Read from: " + fileName + ":");
		// read from the file
		while (dataFile.available()) {
			Serial.write(dataFile.read());
		}
		// close the file:
		Serial.println("-------Read  end");
		dataFile.close();
		dataFile = SD.open(fileName, FILE_WRITE);
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening");
	}
}

void safeBufferCounterToSD(){
	writeSensorStrDataToSD();
	counter++;
	if (counter == myNumber){
		dataFile.flush();
		Serial.println("1000 lines saved at: " + getCurrentTime());
		Serial.println("Battery Voltage: " + String(getBatteryVoltage()) +"V");
		counter = 0;

	}

}

void writeSensorStrDataToSD(){
	accSensor.read();
	accSensor.getEvent(&myEvent);
	xString = myEvent.acceleration.x;
	yString = myEvent.acceleration.y;
	zString = myEvent.acceleration.z;
	writeSDStringln(xString + "," + yString + "," + zString);

}

void deleteSDFile(){
	SD.remove(fileName);
	dataFile = SD.open(fileName, FILE_WRITE);
	dataFile.close();
}


void setupRTC(){
	rtc.begin(); // initialize RTC
	rtc.setTime(hours, minutes, seconds);
	rtc.setDate(day, month, year);
	Serial.println("Start time: " + getCurrentDateAndTime());

}



String getCurrentTime(){
	String str = "";
	String myTime = "";
	currHour = rtc.getHours();
	currMinute = rtc.getMinutes();
	currSecond = rtc.getSeconds();

	if (currHour < 10) str = "0";
	myTime += str + currHour  + ":";
	str = "";

	if (currMinute < 10) str = "0";
	myTime += str + currMinute + ":";
	str = "";

	if (currSecond < 10) str = "0";
	myTime += str + currSecond;
	str = "";

	return myTime;
}

String getCurrentDate(){
	String str = "";
	String myDate = "";
	currYear = rtc.getYear();
	currMonth = rtc.getMonth();
	currDay = rtc.getDay();

	if (currDay < 10) str = "0";
	myDate += str + currDay  + "/";
	str = "";

	if (currMonth < 10) str = "0";
	myDate += str + currMonth + "/";
	str = "";

	if (currYear < 10) str = "0";
	myDate += str + currYear;
	str = "";

	return myDate;
}

String getCurrentDateAndTime(){
	return getCurrentDate() +" " + getCurrentTime();
}

float getBatteryVoltage(){
	float vbat = analogRead(A7);
	vbat *= 2;    // we divided by 2, so multiply back
	vbat *= 3.3;  // Multiply by 3.3V, our reference voltage
	vbat /= 1024; // convert to voltage
	return vbat;
}

