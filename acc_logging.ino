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
#include <SD.h>
#define SD_CS 4
File dataFile;
File dataTarget;


String fileName = "accraw47.csv";
String targetName ="acclog88.csv";

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
#define interruptPinRecord 15   //A1    (Start/Stop Record)
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
uint16_t sampleRate = 400; //Sample Rate of the record
bool state = 0; //timer test

//----Variables-----
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
const uint32_t myNumber = 300;  //some value for the counter
uint8_t myAccbinint[myNumber];
uint8_t myAccbinintSD[myNumber];
bool writeSD = false;
String testFileName = "";
uint8_t recordcounter = 0;
uint16_t divider = 1365;

//-----Calibration-------
uint16_t counterCalibration = 0;
const uint16_t counterCalibrationMax = 5000;
int16_t calibXMed = 0;
int16_t calibYMed = 0;
int16_t calibZMed = 0;
double rotAngleX = 0;
double rotAngleY = 0;

int test = 0;
/////////////////////////////////
void setup() {
	setupLED();
	setupInterrupt();
	setupSerial();
	setupLIS3DH();
	//	setupLIS3DHClick();    //only a test, needs some improvement
	setupSD();
	setupRTC();


	//deleteSDFile();
	//dataFile = SD.open(fileName, FILE_WRITE);
	//dataTarget = SD.open(targetName, FILE_WRITE);
	//dataFile.close();

	//Green LED, setup finished
	ledRedOff();
	ledGreenOff();


	//waitForSetupRTCWithSerial();
	Serial.println("My new Time: " + getCurrentDateAndTime());
	//setupAkkuCheck();
	setStandbyState();

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
	goSleep();
	ledBatteryOff();
	ledRedOff();
	//time to wake up  (needed for buttons)
	delay(300);

	//Power Optimisation reset
	accSensor.setDataRate(LIS3DH_DATARATE_400_HZ);
	//-----------


}


void startRecord(){

	ledRecordOn();

	//fileName = "REC" + String(recordcounter) + ".csv";
	fileName = getCurrentDateTimeFilename();
	dataFile = SD.open(fileName,FILE_WRITE);
	String xRot = String(radianToDegree(rotAngleX));
	String yRot = String(radianToDegree(rotAngleY));

	String firstLine = "Format: x,y,z ---- Start at: " + getCurrentTime() + "   SampleRate: "
			+ String(sampleRate) + "Hz    Calibration AngleX: "
			+ xRot + "     Calibration AngleY"   + yRot + "--- Divider: " + String(divider);

	dataFile.println(firstLine);

	Serial.println("Filename: " + String(dataFile.name()));
	Serial.println("Start record at: " + getCurrentTime());
	delay(10);


	startTimer(sampleRate);




	//String myfileName = getCurrentDateTimeFilename();



	while (CurrState == StateRecord){

		if(state == true) {
			//digitalWrite(LED_PIN,HIGH);
			ledGreenOn();
		} else {
			ledGreenOff();
			//digitalWrite(LED_PIN,LOW);
		}
		state = !state;



		//		if(writeSD){
		//			writeSD = false;
		//			//Serial.println("Array full");
		//			//counter = 0;
		//			for (uint16_t var = 0; var < myNumber; var++) {
		//				myAccbinintSD[var] = myAccbinint[var];
		//			}
		//			//Serial.println("writing array.... ");
		//			ledGreenOn();
		//
		//			dataFile.write(myAccbinintSD, myNumber);
		//			//dataFile.flush();
		//			//dataFile.close();
		//			ledGreenOff();
		//		}

	}
	Serial.println("Filename: " + String(dataFile.name()));
	disableTimer();
	Serial.println("Stop record at: " + getCurrentTime());
	delay(700);
	dataFile.close();
	delay(100);

	recordcounter++;
	//Serial.println("Filename: " + String(dataFile.name()));

	//	dataTarget = SD.open(targetName, FILE_WRITE);
	//	dataFile = SD.open(fileName);
	//
	//	copyDatToCsv();
	//
	//	dataFile.close();
	//	dataTarget.close();

	ledGreenOff();
	setStandbyState();
	ledRecordOff();
}

void setupAkkuCheck(){
	Serial.println("Akku interrupt setzen");
	rtc.setAlarmTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds()+10);
	rtc.enableAlarm(rtc.MATCH_HHMMSS);
	rtc.attachInterrupt(alarmCheckbattery);
}

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
	//Calibration Code

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

	accSensor.setRange(LIS3DH_RANGE_16_G);
	delay(2000);
	ledCalibrationOff();
	setStandbyState();
}

double radianToDegree(double rad){
	return rad*180/PI;
}

void rotationX(int16_t x,int16_t y,int16_t z){

	double myCos = cos(rotAngleX);
	double mySin = sin(rotAngleX);

	calibYMed = ((double)y)*myCos - ((double)z)*mySin;
	calibZMed = ((double)y)*mySin + ((double)z)*myCos;
}

void rotationY(int16_t x,int16_t y,int16_t z){

	double myCos = cos(rotAngleY);
	double mySin = sin(rotAngleY);

	calibXMed = ((double)x)*myCos + ((double)z)*mySin;
	calibZMed = -((double)x)*mySin +((double)z)*myCos;
}


void calcAngleEarth_ZX(int vectorEarth[3]){
	int vectorZ[3] = {0,1,0};
	rotAngleX = calcAngle(vectorEarth, vectorZ);
}

void calcAngleEarth_ZY(int vectorEarth[3]){
	int vectorZ[3] = {1,0,0};
	rotAngleY = calcAngle(vectorEarth, vectorZ);
}


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


void goSleep(){

	//Power Optimisation
	accSensor.setDataRate(LIS3DH_DATARATE_POWERDOWN);
	Serial.println("Going to sleep at: " + getCurrentTime());
	ledRedOn();
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__WFI();
}


void wakeUp(){
	Serial.println("Wake up at: " + getCurrentTime());
	ledGreenOn();
}

void setupInterrupt(){

	//Start/Stop Record
	pinMode(interruptPinRecord, INPUT_PULLUP);
	attachInterrupt(interruptPinRecord,start_stopRecordInterrupt, LOW);

	//Start calibration
	pinMode(interruptPinCalibration, INPUT_PULLUP);
	attachInterrupt(interruptPinCalibration,startCalibration, LOW);

}


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
	myX = accSensor.x;
	myY = accSensor.y;
	myZ = accSensor.z;
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
	//accSensor.getEvent(&myEvent);
	xString = String(accSensor.x);
	yString = String(accSensor.y);
	zString = String(accSensor.z);
	writeSDStringln(xString + "," + yString + "," + zString);

}

void writeSensorIntDataToSD(){
	accSensor.read();
	accSensor.getEvent(&myEvent);
	writeSDStringln(String(accSensor.x) + "," + String(accSensor.y) + "," + String(accSensor.z));

}

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
	//	if(counter>=(myNumber-1)){
	//		dataFile.write(myAccbinint, myNumber);
	//		dataFile.close();
	//	};
}

void copyDatToCsv(){
	uint8_t x1csv;
	uint8_t x2csv;
	uint8_t y1csv;
	uint8_t y2csv;
	uint8_t z1csv;
	uint8_t z2csv;
	uint16_t xcsv;
	uint16_t ycsv;
	uint16_t zcsv;

	//Serial.println("Size: " + String(dataFile.size()));
	while(dataFile.available()){
		//.println("Position: " + String(dataFile.position()));
		x1csv = dataFile.read();
		x2csv = dataFile.read();
		y1csv = dataFile.read();
		y2csv = dataFile.read();
		z1csv = dataFile.read();
		z2csv = dataFile.read();
		xcsv = ((uint16_t)x1csv << 8)|(x2csv & 0xFF);
		ycsv = ((uint16_t)y1csv << 8)|(y2csv & 0xFF);
		zcsv = ((uint16_t)z1csv << 8)|(z2csv & 0xFF);
		dataTarget.println((String)xcsv + "," + (String)ycsv + "," + (String)zcsv);
	}
	Serial.println("done writing csv file");
}



void deleteSDFile(){
	SD.remove(fileName);
	SD.remove(targetName);
	dataFile = SD.open(fileName, FILE_WRITE);
	dataTarget = SD.open(targetName, FILE_WRITE);
	dataFile.close();
	dataTarget.close();
}


void setupRTC(){
	rtc.begin(); // initialize RTC
	rtc.setTime(hours, minutes, seconds);
	rtc.setDate(day, month, year);
	Serial.println("Start time: " + getCurrentDateAndTime());
}



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
		} else {
			ledRecordOff();
			ledBatteryOff();
			ledCalibrationOff();

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
	String complete = myTime +".csv";

	return complete;
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

	writeSensorStrDataToSD();


	// INterrupt function here!!!
	//	readAcc();
	//	myXbinA= myX >> 8;
	//	myXbinB= myX & 0x00FF;
	//	myYbinA= myY >> 8;
	//	myYbinB= myY & 0x00FF;
	//	myZbinA= myZ >> 8;
	//	myZbinB= myZ & 0x00FF;
	//	myAccbinint [0+counter] = myXbinA;
	//	myAccbinint [1+counter] = myXbinB;
	//	myAccbinint [2+counter] = myYbinA;
	//	myAccbinint [3+counter] = myYbinB;
	//	myAccbinint [4+counter] = myZbinA;
	//	myAccbinint [5+counter] = myZbinB;
	//	counter += 6 ;
	//	if (counter+5 >= myNumber) {
	//		counter = 0;
	//		//		for (uint16_t var = 0; var < myNumber; var++) {
	//		//			myAccbinintSD[var] = myAccbinint[var];
	//		//		}
	//		writeSD = true;

	//}
	//
	//
	//
	//	if(state == true) {
	//		//digitalWrite(LED_PIN,HIGH);
	//		ledGreenOn();
	//	} else {
	//		ledGreenOff();
	//		//digitalWrite(LED_PIN,LOW);
	//	}
	//	state = !state;
	//---------------------


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
