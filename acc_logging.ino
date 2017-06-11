//LIS3DH
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#define LIS3DH_CS 15
#define CLICKTHRESHHOLD 50
sensors_event_t myEvent;

//Feather M0
#include <math.h>
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

//Serial Plotter
#include "simplot.h"


//SD Card
#include <SD.h>
#define SD_CS 4
File dataFile;
String fileName = "accLog2.txt";

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

//--------ErrorNum---------
//LED Error blink
#define errLIS3DH 1
#define errNoSD 2
//-------------------------

//Initialisation
Adafruit_LIS3DH accSensor = Adafruit_LIS3DH(LIS3DH_CS);
uint8_t click;
#if defined(ARDUINO_ARCH_SAMD)
#define Serial Serial
#endif
#define ledGreen 8
#define ledRed 13
#define interruptPin 16
#define StateStandby 0
#define StateCalibration 1
#define StateRecord 2
#define StateError 3
uint8_t CurrState = 0;     //0 = Standby  // 1 = Calibration // 2 = record //  3 = error

//Timer
uint16_t sampleRate = 50; //Sample Rate of the record
bool state = 0; //timer test

//Variables
String xString = "";
String yString = "";
String zString = "";
uint32_t counter = 0;
int16_t myX = 0;
int16_t myY = 0;
int16_t myZ = 0;
uint8_t myXbinA = 0;
uint8_t myXbinB = 0;
uint8_t myYbinA = 0;
uint8_t myYbinB = 0;
uint8_t myZbinA = 0;
uint8_t myZbinB = 0;
const uint32_t myNumber = 6000;  //some value for the counter
uint8_t myAccbinint[myNumber];

//Calibration
uint16_t counterCalibration = 0;
const uint16_t counterCalibrationMax = 5000;
int16_t calibXMed = 0;
int16_t calibYMed = 0;
int16_t calibZMed = 0;


int test = 0;
/////////////////////////////////
void setup() {
	setupLED();
	setupInterrupt();
	//setupSerial();
	setupLIS3DH();
	//	setupLIS3DHClick();    //only a test, needs some improvement
	setupSD();
	setupRTC();

	//	deleteSDFile();
	//	dataFile = SD.open(fileName, FILE_WRITE);
	//	readSD();


	//Green LED, setup finished
	ledRedOff();
	ledGreenOn();


	startTimer(30);   //Only a test for the timer with 30 Hz (green LED blinking)



}

void loop() {








}

void calibration(){
	calcAccMedian();


}

double calcAngleEarth_XY(int vektor[3]){
	return 0; //---------------NOT FINISHED---------

}

double calcAngle(int vektor1[3],int vektor2[3]){
	int skalar = vektor1[0]*vektor2[0] + vektor1[1]*vektor2[1] +vektor1[2]*vektor2[2];
	int length1 = sqrt(vektor1[0]*vektor1[0]+vektor1[1]*vektor1[1]+vektor1[3]*vektor1[3]);
	int length2 = sqrt(vektor2[0]*vektor2[0]+vektor2[1]*vektor2[1]+vektor2[3]*vektor2[3]);
	double result = (double)skalar/(length1*length2);
	return asin(result);
}

void calcAccMedian(){
	int xSum = 0;
	int ySum = 0;
	int zSum = 0;
	for (uint16_t counterMed = 0; counterMed < counterCalibrationMax; counterMed++) {
		readAcc();
		xSum += accSensor.x;
		ySum += accSensor.y;
		zSum += accSensor.z;
	}
	calibXMed = xSum/counterCalibrationMax;
	calibYMed = ySum/counterCalibrationMax;
	calibZMed = zSum/counterCalibrationMax;
}

//Current state of the device
void setState(uint8_t stateToSet){
	CurrState = stateToSet;
}


void goSleep(){
	Serial.println("Going to sleep at: " + getCurrentTime());
	ledGreenOff();
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__WFI();
}


void wakeUp(){
	Serial.println("Wake up at: " + getCurrentTime());
	ledGreenOn();
}

void setupInterrupt(){
	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(interruptPin,wakeUp, LOW);
}


void setupLED(){
	pinMode(ledGreen, OUTPUT);
	pinMode(ledRed, OUTPUT);
}

void ledRedOn(){
	digitalWrite(ledRed, HIGH);

}
void ledRedOff(){
	digitalWrite(ledRed, LOW);
}


void ledGreenOn(){
	digitalWrite(ledGreen, HIGH);
}

void ledGreenOff(){
	digitalWrite(ledGreen, LOW);

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
		error(errLIS3DH);
	}
	accSensor.setRange(LIS3DH_RANGE_16_G);
	accSensor.setDataRate(LIS3DH_DATARATE_400_HZ);
	Serial.println("---LIS3DH initialized---");
}

//Read sensor data
void readAcc(){
	accSensor.read();
}


void setupSD(){
	if (!SD.begin(SD_CS)) {
		Serial.println("Card failed, or not present");
		error(errNoSD);
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
		Serial.println("1000 lines saved at: " + getCurrentTime());
		ledGreenOn();
		dataFile.flush();
		ledGreenOff();
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

void writeSensorBinToArray(){
	accSensor.read();
	myXbinA= accSensor.x >> 8;
	myXbinB= accSensor.x & 0x00FF;
	myYbinA= accSensor.y >> 8;
	myYbinB= accSensor.y & 0x00FF;
	myZbinA= accSensor.z >> 8;
	myZbinB= accSensor.z & 0x00FF;
	myAccbinint [0+counter] = myXbinA;
	myAccbinint [1+counter] = myXbinB;
	myAccbinint [2+counter] = myYbinA;
	myAccbinint [3+counter] = myYbinB;
	myAccbinint [4+counter] = myZbinA;
	myAccbinint [5+counter] = myZbinB;
	counter += 6 ;
	if(counter>=(myNumber-1)){
		dataFile.write(myAccbinint, myNumber);
		dataFile.close();
	};
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

// blink out an error code
void error(uint8_t errno) {
	while(1) {
		uint8_t i;
		for (i=0; i<errno; i++) {
			ledRedOn();
			delay(100);
			ledRedOff();
			delay(100);
		}
		for (i=errno; i<10; i++) {
			delay(200);
		}
	}
}

/////////////////////////Timer///////////////////////////////
void setTimerFrequency(int frequencyHz) {
	int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
	TcCount16* TC = (TcCount16*) TC3;
	// Make sure the count is in a proportional position to where it was
	// to prevent any jitter or disconnect when changing the compare value.
	TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
	TC->CC[0].reg = compareValue;
	Serial.println(TC->COUNT.reg);
	Serial.println(TC->CC[0].reg);
	while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) {
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
	while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

	TcCount16* TC = (TcCount16*) TC3;

	TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Use the 16-bit timer
	TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Use match mode so that the timer counter resets when the count matches the compare register
	TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Set prescaler to 1024
	TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	setTimerFrequency(frequencyHz);

	// Enable the compare interrupt
	TC->INTENSET.reg = 0;
	TC->INTENSET.bit.MC0 = 1;

	NVIC_EnableIRQ(TC3_IRQn);

	TC->CTRLA.reg |= TC_CTRLA_ENABLE;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

void disableTimer()
{

	TcCount16* TC = (TcCount16*) TC3;

	TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

//Reset TC3
void resetTimer()
{
	TcCount16* TC = (TcCount16*) TC3;
	TC->CTRLA.reg = TC_CTRLA_SWRST;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
	while (TC->CTRLA.bit.SWRST);
}


//Interrupt called by the Timer
void TC3_Handler() {
	TcCount16* TC = (TcCount16*) TC3;


	// INterrupt function here!!!
	if(state == true) {
		//digitalWrite(LED_PIN,HIGH);
		ledGreenOn();
	} else {
		ledGreenOff();
		//digitalWrite(LED_PIN,LOW);
	}
	state = !state;
	//---------------------


	TC->INTFLAG.bit.MC0 = 1;
}

