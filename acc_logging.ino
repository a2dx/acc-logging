//LIS3DH
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define LIS3DH_CS 1
#define CLICKTHRESHHOLD 50
sensors_event_t myEvent;

//Feather M0
#include <math.h>
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

//Serial Plotter
#include "simplot.h"


//SD Card
//#include <SD.h>
#include <SdFat.h>   //Use SdFat library for performace!!!!!
#define SD_CS 4
SdFat SD;
File dataFile;
File dataTarget;


char const fileNameC[] = "REC1.bin";
char const fileNameC2[] = "REC2.bin";

const char* fileNametest = NULL;



String fileName = "ok.bin";
const String targetName ="ok.bin";

//const String fileName = "testrec77.dat";
//const String targetName ="testrec32.csv";

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

bool receivedSerialTime = false;

//--------ErrorNum---------
uint8_t currentError = 0;
//LED Error blink
#define errLIS3DH 1
#define errNoSD 2
#define errUnknown 3
//-------------------------

//-----Initialisation-------
Adafruit_LIS3DH accSensor = Adafruit_LIS3DH(LIS3DH_CS);
uint8_t click;
#if defined(ARDUINO_ARCH_SAMD)
#define Serial Serial
#endif
#define ledGreen 8
#define ledRed 13
#define StateStandby 0
#define StateCalibration 1
#define StateRecord 2
#define StateError 3
uint8_t CurrState = 0;     //0 = Standby  // 1 = Calibration // 2 = record //  3 = error

//------pin definition-----
#define interruptPinRecord 12   //A1    (Start/Stop Record)
#define interruptPinCalibration 16    //A2  (calibration)
#define LEDRecord 17
#define LEDCalibration 18
#define LEDBattery 19

//------buttons-----
const unsigned int DEBOUNCE_TIME = 400;

unsigned int counterButton = 0;
static unsigned long last_interrupt_timeRecord = 0;
static unsigned long last_interrupt_timeCalib = 0;
//-----Timer-----
const uint16_t sampleRate = 800; //Sample Rate of the record
bool state = 0; //timer test

//----Variables-----
uint8_t SDCounter = 0;
uint8_t testVar = 0;
byte byteArray[6];
String xString = "";
String yString = "";
String zString = "";
uint32_t counter = 0;
int16_t myX = 0;
int16_t myY = 0;
int16_t myZ = 0;
byte myXbinA = 0;
byte myXbinB = 0;
byte myYbinA = 0;
byte myYbinB = 0;
byte myZbinA = 0;
byte myZbinB = 0;
const uint32_t myNumber = 300;  //some value for the counter
uint8_t myAccbinint[myNumber];
uint8_t myAccbinintSD[myNumber];
bool writeSD = false;
String testFileName = "";
uint8_t recordcounter = 0;
uint16_t divider = 16380;
int testNumber = 0;

//if (range == LIS3DH_RANGE_16_G) divider = 1365; // different sensitivity at 16g
// if (range == LIS3DH_RANGE_8_G) divider = 4096;
// if (range == LIS3DH_RANGE_4_G) divider = 8190;
// if (range == LIS3DH_RANGE_2_G) divider = 16380;

//-----Calibration-------
uint16_t counterCalibration = 0;
const uint16_t counterCalibrationMax = 25000;
int16_t calibXMed = 0;
int16_t calibYMed = 0;
int16_t calibZMed = 0;
double rotAngleX = 0;
double rotAngleY = 0;

int test = 0;
/////////////////////////////////
void setup() {



	//Button record Ground
	pinMode(10, OUTPUT);
	digitalWrite(10, LOW);

	//Serial
	setupSerial();



	setupLED();
	Serial.println("Interrupt");
	setupInterrupt();
	Serial.println("Interrupt set");


	setupSD();
	setupLIS3DH();
	setupRTC();


	//waitForSetupRTCWithSerial();
	Serial.println("My new Time: " + getCurrentDateAndTime());
	//setupAkkuCheck();
	setStandbyState();

	ledGreenOff();


}

void loop() {

	//Recording
	if (CurrState == StateRecord) {
		startRecord();
	}
	//Calibration
	if (CurrState == StateCalibration) {
		calibration();
	}

	//Error message
	if (CurrState == StateError) {
		error(currentError);
	}

	//time to wake up  (needed for buttons)
	delay(300);
	ledBatteryOn();
	ledRedOn();
	Serial.println("go sleep");
	goSleep();
	ledRedOff();
	//time to wake up  (needed for buttons)
	delay(300);


}


void startRecord(){

	//Set Record LED
	ledRedOff();
	ledGreenOff();
	ledRecordOn();


	//Create Filename
	fileName = "REC1.bin";


	SDCounter = 1;
	//Check if filename exists
	while (SD.exists(fileName.c_str())){
		SDCounter++;
		fileName = "REC" + String(SDCounter) + ".bin";
	}

	//Create new File
	dataFile = SD.open(fileName,FILE_WRITE);
	delay(10);



	startTimer(sampleRate);


	//while during record, Recorded with interrupt
	while (CurrState == StateRecord){

		if(state == true) {
			//digitalWrite(LED_PIN,HIGH);
			ledGreenOn();
		} else {
			ledGreenOff();
			//digitalWrite(LED_PIN,LOW);
		}
		state = !state;

	}
	Serial.println("Filename: " + String(dataFile.name()));
	disableTimer();
	Serial.println("Stop record at: " + getCurrentTime());
	delay(700);
	dataFile.close();
	delay(100);

	recordcounter++;

	ledGreenOff();
	setStandbyState();
	ledRecordOff();
}

//Not testes
void setupAkkuCheck(){
	Serial.println("Akku interrupt setzen");
	rtc.setAlarmTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds()+10);
	rtc.enableAlarm(rtc.MATCH_HHMMSS);
	rtc.attachInterrupt(alarmCheckbattery);
}

//Not tested
void alarmCheckbattery(){
	//setupAkkuCheck();
	Serial.println("interrupt");
	float batteryVoltage = getBatteryVoltage();
	ledCalibrationOn();
	delay(500);
	ledCalibrationOff();
	Serial.println(String(batteryVoltage));
}




void calibration(){
	//Calibration Code calculates the orientation of the sensor and save it in rotAngleX and rotAngleY
	// use rotationX and rotationY function to calculate the coordinate Transformation
	ledCalibrationOn();
	accSensor.setRange(LIS3DH_RANGE_2_G);

	Serial.println("Iam Calibrating");
	calcAccMedian();
	Serial.println("Old x: " + String(calibXMed));
	Serial.println("Old y: " + String(calibYMed));
	Serial.println("Old z: " + String(calibZMed));
	int earthVector[3] = {calibXMed,calibYMed,calibZMed};

	calcAngleEarth_ZX(earthVector);
	calcAngleEarth_ZY(earthVector);

	Serial.println("RotAngleX: " + String(radianToDegree(rotAngleX)));
	Serial.println("RotAngleY: " + String(radianToDegree(rotAngleY)));
	rotationX(earthVector[0],earthVector[1],earthVector[2]);
	earthVector[0] = calibXMed;
	earthVector[1] = calibYMed;
	earthVector[2] = calibZMed;
	rotationY(earthVector[0],earthVector[1],earthVector[2]);

	Serial.println("new x: " + String(calibXMed));
	Serial.println("new y: " + String(calibYMed));
	Serial.println("new z: " + String(calibZMed));

	//After finished calibration
	ledCalibrationOff();
	setStandbyState();
}

//Rad to degree
double radianToDegree(double rad){
	return rad*180/PI;
}

//calculate the rotation in X
void rotationX(int16_t x,int16_t y,int16_t z){

	double myCos = cos(rotAngleX);
	double mySin = sin(rotAngleX);

	calibYMed = ((double)y)*myCos - ((double)z)*mySin;
	calibZMed = ((double)y)*mySin + ((double)z)*myCos;
}


//Calculate the rotation in Y
void rotationY(int16_t x,int16_t y,int16_t z){

	double myCos = cos(rotAngleY);
	double mySin = sin(rotAngleY);

	calibXMed = ((double)x)*myCos + ((double)z)*mySin;
	calibZMed = -((double)x)*mySin +((double)z)*myCos;
}

//Function for calibration
void calcAngleEarth_ZX(int vectorEarth[3]){
	int vectorZ[3] = {0,1,0};
	rotAngleX = calcAngle(vectorEarth, vectorZ);
}

//Function for calibration
void calcAngleEarth_ZY(int vectorEarth[3]){
	int vectorZ[3] = {1,0,0};
	rotAngleY = calcAngle(vectorEarth, vectorZ);
}

//Function for calibration
double calcAngle(int vektor1[3],int vektor2[3]){
	int skalar = vektor1[0]*vektor2[0] + vektor1[1]*vektor2[1] +vektor1[2]*vektor2[2];
	double length1 = sqrt(vektor1[0]*vektor1[0]+vektor1[1]*vektor1[1]+vektor1[2]*vektor1[2]);
	double length2 = sqrt(vektor2[0]*vektor2[0]+vektor2[1]*vektor2[1]+vektor2[2]*vektor2[2]);
	double result = (double)skalar/(length1*length2);
	//	Serial.println("Length2: "+ String(length1));
	//	Serial.println("Length1: "+ String(length2));
	//	Serial.println("Skalar: "+ String(skalar));
	//	Serial.println(String(result));
	return asin(result);
}

//calculates the median during the time (
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

//set the Current state of the device
void setRecordState(){
	CurrState = StateRecord;
}
void setCalibrationState(){
	CurrState = StateCalibration;
}
void setErrorState(){
	CurrState = StateError;
}
void setStandbyState(){
	CurrState = StateStandby;
}


//Set the Microcontroller in deep sleep
void goSleep(){
	//Power Optimisation
	Serial.println("Going to sleep at: " + getCurrentTime());
	ledRedOn();
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__WFI();
}


void wakeUp(){
	Serial.println("Wake up at: " + getCurrentTime());
	ledGreenOn();
}


//Setup interrupts
void setupInterrupt(){


	//Start/Stop Record
	pinMode(interruptPinRecord, INPUT_PULLUP);
	//delay(50);
	attachInterrupt(interruptPinRecord,start_stopRecordInterrupt, LOW);

	//If you have an implemented calibration butten
	//Start calibration
	//	pinMode(interruptPinCalibration, INPUT_PULLUP);
	//	//delay(50);
	//	attachInterrupt(interruptPinCalibration,startCalibration, LOW);

}

//Setup LED output pins
void setupLED(){
	pinMode(ledGreen, OUTPUT);
	pinMode(ledRed, OUTPUT);
	pinMode(LEDCalibration, OUTPUT);
	pinMode(LEDRecord, OUTPUT);
	pinMode(LEDBattery, OUTPUT);
}

//------LED-----
void ledRecordOn(){
	digitalWrite(LEDRecord, HIGH);
}
void ledRecordOff(){
	digitalWrite(LEDRecord, LOW);
}
void ledCalibrationOn(){
	digitalWrite(LEDCalibration, HIGH);
}
void ledCalibrationOff(){
	digitalWrite(LEDCalibration, LOW);
}
void ledBatteryOn(){
	digitalWrite(LEDBattery, HIGH);
}
void ledBatteryOff(){
	digitalWrite(LEDBattery, LOW);
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
//----------------------

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
	//while (!Serial);
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



//Setup LIS3DH
void setupLIS3DH(){
	if (! accSensor.begin(0x18)) {   // number is for i2c Initialisation
		error(errLIS3DH);
	}
	accSensor.mySetupLoggingSD();
	Serial.println("---LIS3DH initialized---");
}

//Read sensor data and save
void readAcc(){
	accSensor.read();
	myX = accSensor.x;
	myY = accSensor.y;
	myZ = accSensor.z;
}


//Setup SD
void setupSD(){
	if (!SD.begin(SD_CS)) {
		Serial.println("Card failed, or not present");
		error(errNoSD);
	}
	Serial.println("---SD Card initialized----");
}

//Write given String to SD
void writeSDStringln(String str){
	dataFile.println(str);
}

//Read SD File
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


//Write Sensor data to SD in String
void writeSensorIntDataToSD(){
	accSensor.read();
	writeSDStringln(String(accSensor.x) + "," + String(accSensor.y) + "," + String(accSensor.z));

}

//Write Sensor data to SD in binary
void writeSensorBinToArray(){
	readAcc();
	myXbinA= myX >> 8;
	myXbinB= myX & 0x00FF;
	myYbinA= myY >> 8;
	myYbinB= myY & 0x00FF;
	myZbinA= myZ >> 8;
	myZbinB= myZ & 0x00FF;
	myAccbinint [0+counter] = myXbinA;
	myAccbinint [1+counter] = myXbinB;
	myAccbinint [2+counter] = myYbinA;
	myAccbinint [3+counter] = myYbinB;
	myAccbinint [4+counter] = myZbinA;
	myAccbinint [5+counter] = myZbinB;
	counter += 6 ;
	if (counter+5 >= myNumber) {
		counter = 0;
		writeSD = true;
	}

}


//Read and save LIs3Dh date to BIN on SD Card
void binToSD(){

	accSensor.read();

	//Save in Array
	byteArray[1] = accSensor.x >> 8;
	byteArray[0] = accSensor.x & 0x00FF;
	byteArray[3] = accSensor.y >> 8;
	byteArray[2] = accSensor.y & 0x00FF;
	byteArray[5] = accSensor.z >> 8;
	byteArray[4] = accSensor.z & 0x00FF;


	//Write to SD
	dataFile.write(byteArray,6);

}


//Setup Real Time Clock
void setupRTC(){
	rtc.begin(); // initialize RTC
	rtc.setTime(hours, minutes, seconds);
	rtc.setDate(day, month, year);
	Serial.println("Start time: " + getCurrentDateAndTime());
}


//Setup Time with serial USB in this format: yymmddhhmmss
void waitForSetupRTCWithSerial(){
	Serial.print("Type the current time in this format: yymmddhhmmss");
	//Format:  yymmddhhmmss
	uint8_t counter = 0;
	uint8_t previousNum =0;

	while(!receivedSerialTime){

		if(state == true) {

			ledRecordOn();
			ledBatteryOn();
			ledCalibrationOn();
			ledRedOn();
			ledGreenOn();
		} else {
			ledRecordOff();
			ledBatteryOff();
			ledCalibrationOff();
			ledRedOff();
			ledGreenOff();

		}
		state = !state;
		delay(200);
		// send data only when you receive data:
		if (Serial.available() > 0) {
			counter++;
			// read the incoming byte:
			uint8_t incomingByte;
			incomingByte = Serial.read()-48;


			//odd number
			if (counter&1) {
				incomingByte = incomingByte*10;
			}

			//Save year:
			if (counter*(counter-1) == 2) {
				rtc.setYear(incomingByte+previousNum);
			}
			//Save month:
			if (counter*(counter-1) == 12) {
				rtc.setMonth(incomingByte+previousNum);
			}
			//Save day:
			if (counter*(counter-1) == 30) {
				rtc.setDay(incomingByte+previousNum);
			}
			//Save hours:
			if (counter*(counter-1) == 56) {
				rtc.setHours(incomingByte+previousNum);
			}
			//Save minutes:
			if (counter*(counter-1) == 90) {
				rtc.setMinutes(incomingByte+previousNum);
			}
			//Save seconds:
			if (counter*(counter-1) == 132) {
				rtc.setSeconds(incomingByte+previousNum);
			}
			previousNum = incomingByte;
		}
		if (counter == 12){
			receivedSerialTime = true;
		}


	}
	ledRecordOff();
	ledBatteryOff();
	ledCalibrationOff();

}

//Return the current time in Str
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

//Return the current time In String
String getCurrentDateTimeFilename(){
	String str = "";
	String myTime = "";
	currHour = rtc.getHours();
	currMinute = rtc.getMinutes();
	currSecond = rtc.getSeconds();

	if (currHour < 10) str = "0";
	myTime += str + currHour;
	str = "";

	if (currMinute < 10) str = "0";
	myTime += str + currMinute;
	str = "";

	if (currSecond < 10) str = "0";
	myTime += str + currSecond;
	str = "";

	String myDate = "";
	currYear = rtc.getYear();
	currMonth = rtc.getMonth();
	currDay = rtc.getDay();

	if (currDay < 10) str = "0";
	myDate += str + currDay ;
	str = "";

	if (currMonth < 10) str = "0";
	myDate += str + currMonth;
	str = "";

	if (currYear < 10) str = "0";
	myDate += str + currYear;
	str = "";

	//String complete = "ACC"+ myDate +"_"+ myTime +".dat";
	String complete = myTime +".bin";

	return complete;
}

//Return the current Date
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

//Returns the current date and time
String getCurrentDateAndTime(){
	return getCurrentDate() +" " + getCurrentTime();
}

//return the volatge battery
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
	while (TC->STATUS.bit.SYNCBUSY == 1);
}

//Start timer Interrupt with the given frequency
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

//Disable timer Interrupt
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

	//Save data in binary
	binToSD();

	TC->INTFLAG.bit.MC0 = 1;
}

//Interrupt functions
//A2 pin  (Pin number: 16)
void start_stopRecordInterrupt(){
	unsigned long interrupt_time = millis();
	if (interrupt_time - last_interrupt_timeRecord > DEBOUNCE_TIME) {
		if (CurrState == StateRecord) {
			setStandbyState();
			//disableTimer();
		}else if (CurrState == StateStandby) {
			setRecordState();
		}
	}
	last_interrupt_timeRecord = interrupt_time;

}
//A3 pin  (Pin number: 17)
void startCalibration(){
	unsigned long interrupt_time = millis();
	if (interrupt_time - last_interrupt_timeCalib > DEBOUNCE_TIME) {
		if (CurrState == StateStandby) {
			setCalibrationState();
		}
	}
	last_interrupt_timeCalib = interrupt_time;
}
