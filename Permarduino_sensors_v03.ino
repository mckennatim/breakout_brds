/*
*************************************
       PERMARDUINO SENSORS
 M.A. de Pablo & C. de Pablo, 2013-15
 
 Permafrost  and active layer thermal
 monitoring device based on Arduino
 
 -------------------------------------
 Version 3.0    2015-04-11   v20150411
 *************************************
 */

//  DEVICE CONFIGURATION
//  Libraries definition
#include <Wire.h>                                          // Library for I2C communications protocol
#include <DS3231.h>                                        // Library for DS3131 RTC management
//#include <eepromi2c.h>                                     // Library for 24LC256 EEPROM management

#include <OneWire.h>                                       // Library for 1-Wire communications protocol
#include <DallasTemperature.h>                             // Library for DS18B20 temperature sensor
#include <SD.h>                                            // Library for SD card management
#include <avr/sleep.h>                                     // Library for sleep and power save

//  Analogue pins definition
const byte BatteryPin = 0;                                 // Battery voltage - Analog pin 0
const byte SolarCellPin = 1;                               // Solar cell voltage - Analog pin 1
const byte DeviceConf = 2;                                 // Read device configuration - Analog pin 2
//const byte SDApin = 4;                                   // I2C communications port. Not required to be declared
//const byte SCLpin = 5;                                   // I2C communications port. Not required to be declared

//  Digital pins definition
//const byte RXpin = 0;
//const byte TXpin = 1;
//const byte RTCAlarmPin = 2;                              // Not required to be declared
//const byte TestButtonPin = 3;                            // Not required to be declared
const byte PowerPin = 4;                                   // Activation pin to power sensors - Digital pin 4
const byte AirTempPin = 5;                                 // Air temperature sensor - Digital pin 5
const byte SnowTempPin = 6;                                // Snow temperature sensors chain - Digital pin 6
const byte BTSTempPin = 7;                                 // Bottom snow temperature sensors - Digital pin 7
const byte SurfaceTempPin = 8;                             // Surface temperature sensor - Digital pin 8
const byte GroundTempPin = 9;                              // Ground temperature sensors chain- Digital pin 9
const byte chipSelect = 10;                                // SD Card configuration - Digital pin 10
//const byte MISOpin = 11;                                 // SPI communications port. Not required to be declared
//const byte MOSIpin = 12;                                 // SPI communications port. Not required to be declared
//const byte CSKpin = 13;                                  // SPI communications port. Not required to be declared
const byte TestLedPin = 17;                                // PermArduino working led - Analog pin 3

//  Variables definition
int Year;
byte Month;
byte Date;
byte Hour;
byte Minute;
byte Second;
boolean WorkNow = true;                                    // Cancel sleep cycle
unsigned int n = 0;                                        // Measurements counter
volatile byte e = 0;                                       // Error type
byte sn[5];                                                // Sensors in each bus
unsigned int batt;                                         // Store battery voltage
unsigned int solar;                                        // Store solar cell voltage
float temp;                                                // Store temperature data

// Constant definition
const int SwitchValues[32] = {
  0, 511, 416, 642, 351, 617, 500, 677,
  303, 600, 484, 670, 409, 639, 520, 686,
  267, 588, 474, 666, 399, 635, 517, 684,
  346, 615, 498, 676, 423, 644, 526, 688
};
const boolean Sensors[32][5] = {
  {0,0,0,0,0}, {1,0,0,0,0}, {0,1,0,0,0}, {1,1,0,0,0},
  {0,0,1,0,0}, {1,0,1,0,0}, {0,1,1,0,0}, {1,1,1,0,0},
  {0,0,0,1,0}, {1,0,0,1,0}, {0,1,0,1,0}, {1,1,0,1,0},
  {0,0,1,1,0}, {1,0,1,1,0}, {0,1,1,1,0}, {1,1,1,1,0},
  {0,0,0,0,1}, {1,0,0,0,1}, {0,1,0,0,1}, {1,1,0,0,1},
  {0,0,1,0,1}, {1,0,1,0,1}, {0,1,1,0,1}, {1,1,1,0,1},
  {0,0,0,1,1}, {1,0,0,1,1}, {0,1,0,1,1}, {1,1,0,1,1},
  {0,0,1,1,1}, {1,0,1,1,1}, {0,1,1,1,1}, {1,1,1,1,1}
};

/*
//  EEPROM data configuration
 struct config {
 String header;
 } config;
 */

//  Log files configuration
File datafile;                                             // File to store the adquired data
char filename1[] = "Data.csv";                             // Name of the file to store the data
char symbols[] = ";:/ *";                                    // Constant symbols used during data record

//  Libraries configuration
DS3231 RTC;                                               // Configure library to read the Real Time Clock               
/*
OneWire oneWire1(AirTempPin);                              // Configure library to read air temperature sensors
DallasTemperature AirBus(&oneWire1);
OneWire oneWire2(SnowTempPin);                             // Configure library to read snow temperature sensors
DallasTemperature SnowBus(&oneWire2);
OneWire oneWire3(BTSTempPin);                             // Configure library to read bottom temperature os snow cover sensors
DallasTemperature BTSBus(&oneWire3);
OneWire oneWire4(SurfaceTempPin);                          // Configure library to read surface temperature sensors
DallasTemperature SurfaceBus(&oneWire4);
OneWire oneWire5(GroundTempPin);                           // Configure library to read ground temperature sensors
DallasTemperature GroundBus(&oneWire5);
*/
OneWire Wires[5] = { OneWire(AirTempPin), OneWire(SnowTempPin), OneWire(BTSTempPin), OneWire(SurfaceTempPin), OneWire(GroundTempPin)};
DallasTemperature SensorsBus[5] = {DallasTemperature(&Wires[0]), DallasTemperature(&Wires[1]), DallasTemperature(&Wires[2]), DallasTemperature(&Wires[3]), DallasTemperature(&Wires[4])};
boolean ActiveSensors[5]= {1,0,0,0,0};
const byte sensorsnumber[5] = {1,16,1,1,17};               // Number of temperature sensors in each bus
const byte Precision = 12;                                 // DS18B20 precision configuration to 12 bits (0.0625ºC)

//  Sensors identification
uint8_t airsensors[1][8] = {                               // Air temperature sensors
  {0x28,0x25,0x30,0x09,0x06,0x00,0x00,0x85}                // Sensor at 160 cm heigth
};
uint8_t snowsensors[16][8] = {                             // Snow temperature sensors
  {0x28,0xBC,0x0F,0x60,0x05,0x00,0x00,0xAE},               // Sensor at 0 cm height
  {0x28,0xA2,0xF3,0x60,0x05,0x00,0x00,0x61},               // Sensor at 2.5 cm height
  {0x28,0xE2,0xA8,0x60,0x05,0x00,0x00,0xF1},               // Sensor at 5 cm height
  {0x28,0x46,0x10,0x61,0x05,0x00,0x00,0x8F},               // Sensor at 10 cm height
  {0x28,0xB6,0xFF,0x60,0x05,0x00,0x00,0xC7},               // Sensor at 20 cm height
  {0x28,0xC9,0xEF,0x60,0x05,0x00,0x00,0x07},               // Sensor at 30 cm height
  {0x28,0xF9,0xF5,0x60,0x05,0x00,0x00,0x2B},               // Sensor at 40 cm height
  {0x28,0x55,0xA4,0x60,0x05,0x00,0x00,0x52},               // Sensor at 50 cm height
  {0x28,0xF5,0x70,0x5D,0x05,0x00,0x00,0x9A},               // Sensor at 60 cm height
  {0x28,0xDD,0x74,0x60,0x05,0x00,0x00,0x47},               // Sensor at 70 cm height
  {0x28,0xE3,0x96,0x60,0x05,0x00,0x00,0xE0},               // Sensor at 80 cm height
  {0x28,0xD3,0xFB,0x60,0x05,0x00,0x00,0xF0},               // Sensor at 90 cm height
  {0x28,0x1B,0xEA,0x60,0x05,0x00,0x00,0x7F},               // Sensor at 100 cm height
  {0x28,0x67,0xB4,0x5F,0x05,0x00,0x00,0x7F},               // Sensor at 120 cm height
  {0x28,0x6F,0x09,0x61,0x05,0x00,0x00,0x20},               // Sensor at 140 cm height
  {0x28,0xDF,0x06,0x61,0x05,0x00,0x00,0x48}                // Sensor at 160 cm height
};
uint8_t BTSsensors[1][8] = {                               // Bottom Temperature of Snow Cover sensor
  {0x28,0x90,0xC1,0x0A,0x06,0x00,0x00,0x8A}                // Sensor at 2 cm heigth
};
uint8_t surfacesensors[1][8] = {                           // Surface temperatue sensors
  {0x28,0xFC,0x4F,0x20,0x05,0x00,0x00,0xD2}                // Sensor at 1 cm depth  
};
uint8_t groundsensors[17][8]={                             // Ground temperature sensors
  {0x28,0xF0,0xF0,0x60,0x05,0x00,0x00,0x6F},               // Sensor at 0 cm depth
  {0x28,0x98,0xF1,0x60,0x05,0x00,0x00,0xC0},               // Sensor at 2.5 cm depth
  {0x28,0x64,0xED,0x60,0x05,0x00,0x00,0x92},               // Sensor at 5 cm depth
  {0x28,0x42,0x0D,0x61,0x05,0x00,0x00,0xC3},               // Sensor at 10 cm depth
  {0x28,0x22,0xA1,0x60,0x05,0x00,0x00,0x9D},               // Sensor at 20 cm depth
  {0x28,0xE2,0x7D,0x61,0x05,0x00,0x00,0xF2},               // Sensor at 30 cm depth
  {0x28,0xAA,0x6D,0x61,0x05,0x00,0x00,0x5A},               // Sensor at 40 cm depth
  {0x28,0xEA,0x01,0x61,0x05,0x00,0x00,0x1F},               // Sensor at 50 cm depth
  {0x28,0x7E,0xB2,0x60,0x05,0x00,0x00,0xFC},               // Sensor at 60 cm depth
  {0x28,0x41,0xFD,0x60,0x05,0x00,0x00,0xB3},               // Sensor at 70 cm depth
  {0x28,0x91,0x7E,0x61,0x05,0x00,0x00,0x7D},               // Sensor at 80 cm depth
  {0x28,0x31,0x25,0x61,0x05,0x00,0x00,0xC4},               // Sensor at 90 cm depth
  {0x28,0x55,0x1E,0x61,0x05,0x00,0x00,0x2F},               // Sensor at 100 cm depth
  {0x28,0x8D,0x97,0x60,0x05,0x00,0x00,0xFD},               // Sensor at 110 cm depth
  {0x28,0x13,0xF0,0x60,0x05,0x00,0x00,0x1F},               // Sensor at 120 cm depth
  {0x28,0xD3,0xF4,0x60,0x05,0x00,0x00,0x9F},               // Sensor at 130 cm depth
  {0x28,0xA7,0x09,0x61,0x05,0x00,0x00,0x1E}                // Sensor at 140 cm depth
}; 
uint8_t (*SensorsID[5])[8] = {airsensors, snowsensors, BTSsensors, surfacesensors, groundsensors};

//  DEVICE INITIALIZATION
void setup(){
  pinMode(PowerPin, OUTPUT);                               // Configure the power pin
  pinMode(TestLedPin, OUTPUT);                             // Configurate the working led
  pinMode(chipSelect, OUTPUT);                             // Configure SD card
  
  digitalWrite(TestLedPin, HIGH);                          // Turn off the test led
  
  Wire.begin();                                            // Starts I2C communications
  RTC.begin();                                             // Start the RTC                       
   if (! RTC.begin()){                                     // Check if RTC is running  
    e = 1;                                                 // Error #01: RTC is not running
    error();                                               // Blink the led and freeze the device
    return;
  }
  ReadTime();                                              // Read the present time from RTC
  if (Year < 2014) {                                       // Check if RTC is about up to date
    e = 2;                                                 // Error #02: RTC is out of date
    error();
    return;
  }
  
  if (! SD.begin(chipSelect)) {                            // Check if SD memory card is running
    e = 3;                                                 // Error #03: Unable to set SD card
    error();                                               // Blink the led and freeze the device
    return;
  }
  if (! SD.exists(filename1)) {                            // Check and/or create the logdata file on SD Card
    datafile = SD.open(filename1, FILE_WRITE);             // Create the datafile if it doesn't exist 
    datafile.close();
    if (! SD.exists(filename1)) {                          // Check if datafile was created
      e = 4;                                               // Error #04: Unable to create a file on SD card
      error();                                             // Blink the led and freeze the device
      return;
    }
    //eeRead(0,config);                                    // Read the header information from the EEPROM memory
    //delay(50);
    datafile = SD.open(filename1, FILE_WRITE);             // Open the datafile 
    if (datafile){
      //datafile.println(config.header);                   // Save the header in the datafile
      datafile.println("Permarduino_sensors");
      //datafile.print(F("No;Date;Time;InV(mV);InT(mC);Bat(mV);Sol(mV);Air1(C);Sn1(C);Sn2(C);Sn3(C);Sn4(C);Sn5(C);Sn6(C);Sn7(C);Sn8(C);Sn9(C);Sn10(C);Sn11(C);Sn12(C);Sn13(C);Sn14(C);Sn15(C);Sn16(C);"));
      //datafile.println(F("BTS1(C);Surf1(C);Gr1(C);Gr2(C);Gr3(C);Gr4(C);Gr5(C);Gr6(C);Gr7(C);Gr8(C);Gr9(C);Gr10(C)"));
      delay(30);
      datafile.close();                                    // Close the datafile
    }
    else {
      e = 5;                                               // Error #05: Unable to write on data file
      error();                                             // Blink the led and freeze the device
      return;
    }     
  }

  // Sensors initilization, configuration and test
  digitalWrite(PowerPin, HIGH);                            // Turn on the sensors
  delay(1000);
  
  SensorsConfig();                                       // Establish the device configuration
  delay(100);
  
  for (byte a = 0; a < 5; a++) {
    if (ActiveSensors[a] == true) {
       SensorsBus[a].begin();                              // Initialize temperature sensor bus
       sn[a] = SensorsBus[a].getDeviceCount();             // Establish the number of sensors on the temperature bus  
       if (! sn[a] == sensorsnumber[a]) {                  // Check if the number of sensors is correct in each bus
         e = 6;                                            // Error #06: Incorrect number of sensors
         error();
         return;
       }
       for (byte b = 0; b < sensorsnumber[a]; b++){                     
         SensorsBus[a].setResolution(SensorsID[a][b], Precision);    // Set the resolution of temperature sensors on the bus
       }
    }
  }
  
  /*
  SensorsBus[0].begin();
  //AirBus.begin();                                          
  //SnowBus.begin();                                         // Initialize snow temperature sensor bus
  SensorsBus[1].begin();
  SensorsBus[2].begin();
  SensorsBus[3].begin();
  SensorsBus[4].begin();
  //BTSBus.begin();                                          // Initialize BTS sensor bus
  //SurfaceBus.begin();                                      // Initialize surface temperature sensor bus
  //GroundBus.begin();                                       // Initialize ground temperature sensor bus
  sn[0] = SensorsBus[0].getDeviceCount();                         
  sn[1] = SensorsBus[1].getDeviceCount();                        // Establish the number of sensors on snow temperature bus
  sn[2] = SensorsBus[2].getDeviceCount();                         // Establish the number of sensors on BTS bus
  sn[3] = SensorsBus[3].getDeviceCount();                     // Establish the number of sensors on surface temperature bus
  sn[4] = SensorsBus[4].getDeviceCount();                      // Establish the number of sensors on ground temperature bus
  for (byte i = 0; i < 5; i++){                            
    if (! sn[i] == sensorsnumber[i]){
      e = 6;                                               
      error();                                             // Blink the led and freeze the device
      return;
    }
  }
  SensorsBus[0].setResolution(airsensors[0], Precision);          // Set the resolution of air temperature sensors
  for (byte i = 0; i < sn[1]; i++){                        // Set the resolution of snow temperature sensors
    SensorsBus[1].setResolution(snowsensors[i], Precision);
  }
  SensorsBus[2].setResolution(surfacesensors[0], Precision);      // Set the resolution of bottom snow cover temperature sensors
  SensorsBus[3].setResolution(surfacesensors[0], Precision);  // Set the resolution of surface temperature sensors
  for (byte i=0; i < sn[4]; i++){                          // Set the resolution of ground temperature sensors
    SensorsBus[4].setResolution(groundsensors[i], Precision);
  }
  */
  
  RecordData();                                            // Take an initial reading from sensors
  digitalWrite(PowerPin, LOW);                             // Turn off the sensors
  digitalWrite(TestLedPin, LOW);                           // Turn off the test led
  delay(500);
  CheckDevice();
  RTC.enableInterrupts(EveryHour);                         // Set the working alarm for the device (once each hour o'clok) 
  delay(500);
}


//  MAIN DEVICE PROGRAM
void loop(){
  ReadTime();                                              // Read the present time                               // Check the present time
  if (WorkNow == false){                                   // Shows device status if button was pressed
    CheckDevice();                                         // Show device status and higher error code
    delay(100);
  }
  if (WorkNow == true){
  //if ((minute == 0) && (WorkNow == true)){               // Measure if it is the time o'clok and the test button was not pressed
    RecordData();                                          // Read the sensors and save the data
    delay(100);
    WorkNow = false;                                       // Made the device be ready for the next measurement
  }
  
  attachInterrupt(0, WakeUp, FALLING);                     // Activate again the interrupt from the RTC alarm
  attachInterrupt(1, CheckNow, CHANGE);                    // Activate again the interrupt from the test button
  RTC.clearINTStatus();
  
  delay(300);
  sleepNow();                                              // Send the arduino to sleep and power down mode until next measurement or device test
}


//  PROCEDURES
// Device configuration
void SensorsConfig(){
  int readings[35];                               // the readings from the analog input
  byte index = 0;                                  // the index of the current reading
  int total = 0;                                  // the running total
  int average = 0;                                // the average
  for (int thisReading = 0; thisReading < 35; thisReading++)// initialize all the readings to 0: 
    readings[thisReading] = 0;
  for (byte i = 0; i < 35; i++) {      
    total = total - readings[index];                        // subtract the last reading         
    readings[index] = analogRead(DeviceConf);                // read from the sensor 
    total = total + readings[index];                        // add the reading to the total       
    index = index + 1;                                     // advance to the next position in the array:                     
    if (index >= 35){                                      // if we're at the end of the array...            
      index = 0;                                           // ...wrap around to the beginning:                          
    }
    average = total / 35;                                  // calculate the average:        
    delay(20);                                             // delay in between reads for stability 
  }
  for (byte i = 0; i < 32; i++) {
    if (average == SwitchValues[i]) {
      ActiveSensors[0] = Sensors[i][0];
      ActiveSensors[1] = Sensors[i][1];
      ActiveSensors[2] = Sensors[i][2];
      ActiveSensors[3] = Sensors[i][3];
      ActiveSensors[4] = Sensors[i][4];
      break;
    }
  }
  delay(100);
  return;
}

// Sensors reading
void RecordData(){
  digitalWrite(PowerPin, HIGH);                            // Turn on the sensor's power
  digitalWrite(TestLedPin, HIGH);                          // Turn on the led marking "Measurements in progress"
  delay(1000);                                             // Give time to sensors to turn on and stabilize their reading
  n = n + 1;                                               // Update measurement counter
  SD.begin(chipSelect);
  datafile = SD.open(filename1, FILE_WRITE);               // Open the file to store the data
  if (datafile){
    printDigits(Date);                                     // Save present day
    datafile.print(symbols[2]);
    printDigits(Month);                                    // Save present month
    datafile.print(symbols[2]);
    datafile.print(Year);                                  // Save present year
    if (Year < 2014) {                                     // Check if RTC is up to date
      e = 2;                                               // Error #02: RTC is out of date
    }
    datafile.print(symbols[3]);
    printDigits(Hour);                                     // Save present hour
    datafile.print(symbols[1]);
    printDigits(Minute);                                   // Save present minute
    datafile.print(symbols[1]);
    printDigits(Second);                                   // Save present second
    delay(10);
    datafile.print(symbols[0]);
    datafile.print(n);                                     // Save measurement number (counter)
    datafile.print(symbols[0]); 
    datafile.print(readVcc());                             // Read and save battery voltage
    datafile.print(symbols[0]);
    batt = analogRead(BatteryPin);                         // Read the battery voltage
    datafile.print(batt);
    datafile.print(symbols[0]);
    datafile.print(2 * (readVcc() * batt) / 1023);         // Save the battery voltage          
    datafile.print(symbols[0]);
    solar = analogRead(SolarCellPin);                      // Read the solar cell voltage
    datafile.print(solar);
    datafile.print(symbols[0]);
    datafile.print(0.00666 * solar);
    datafile.print(symbols[0]);
    delay(10);
    temp = readTemp(),2;
    datafile.print(temp);                                           // Read and save inner temperature
    datafile.print(symbols[0]);
    RTC.convertTemperature();
    temp = RTC.getTemperature();
    datafile.print(temp);
    datafile.print(symbols[0]);
    delay(10);
    
    for (byte c = 0; c < 5; c++) {
      if (ActiveSensors[c] == true){
        SensorsBus[c].requestTemperatures();                          // Request temperatures from Air temperature bus
        for (byte d = 0; d < sensorsnumber[c]; d++) {
          temp = SensorsBus[c].getTempC(SensorsID[c][d]);                 // Read Air temperature sensor
          datafile.print(temp);                                         // Save Air temperature data
          datafile.print(symbols[0]);
          if ((temp == -127)||(temp == 85)){                            // Check for errors on Air temperature sensors
            e = 7;                                                      // Error #07: problems reading Air temperature sensors
          }
          delay(10);
        }
      }
      else {
        for (byte f = 0; f < sensorsnumber[c]; f++) {
          datafile.print(symbols[4]);
          datafile.print(symbols[0]);
        }
      }
      delay(10);
    }
    delay(10);
    datafile.println(e);                                   // Save the higher error code obtained
    delay(100);                                            // Give time to save all the data
    datafile.close();                                      // Close data file
  }
  else {                                                   // Call error procedure if not able to save the data
    e = 4;                                                 // Error #04: Unable to create a file on SD card
    error();                                               // Blink the led and freeze the device
    return;
  }
  digitalWrite(PowerPin, LOW);                             // Turn off the sensor's power
  digitalWrite(TestLedPin, LOW);                           // Turn off the led marking "Measurements in progress"
}


// Inner voltmeter
long readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // Read 1.1V reference against AVcc
  delay(2);                                                // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);                                     // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;                              // Back-calculate AVcc in mV
  return result;
}

// Microcontroler temperature
double readTemp(void) {
  unsigned int wADC;
  double t;
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));           // Set the internal reference and mux.
  ADCSRA |= _BV(ADEN);                                     // enable the ADC
  delay(20);                                               // wait for voltages to become stable.
  ADCSRA |= _BV(ADSC);                                     // Start the ADC
  while (bit_is_set(ADCSRA,ADSC));                         // Detect end-of-conversion
  wADC = ADCW;                                             // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  t = (wADC - 324.31 ) / 1.22;                             // The offset of 324.31 could be wrong. It is just an indication.
  return (t);                                              // The returned temperature is in degrees Celcius.
}

//  Stops the sleep process
void WakeUp(){
  noInterrupts();                                          // Disable interrupts to not disrupt the sensor readings
  WorkNow = true;                                          // The device is ready to take new sensors readings
}

void CheckNow(){
  noInterrupts();                                          // Disable interrupts to not disrupt the device checking
  WorkNow = false;                                         // The device could be checked now
}

// Read present time and dat
void ReadTime(){
  DateTime now = RTC.now();
  Year = now.year(), DEC;
  Month = now.month(), DEC;
  Date = now.date(), DEC;
  Hour = now.hour(), DEC;
  Minute = now.minute(), DEC;
  Second = now.second(), DEC;
}

// Set the arduino to sleep mode
void sleepNow(){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);                   // sleep mode is set here
  sleep_enable();                                        // enables the sleep bit in the mcucr register
                                                         // so sleep is possible. just a safety pin  
  sleep_mode();                                          // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();                                       // first thing after waking from sleep:
                                                         // disable sleep... 
}

// Shows the device status by a blinking led
void CheckDevice(){
  blinkLed(TestLedPin, 3, 500);
  delay(100);
  blinkLed(TestLedPin, e, 200);
}

// Blink a led
void blinkLed(byte Pin, int numBlinks, int blinkRate) {
  for (int i=0; i < numBlinks; i++) {
    digitalWrite(Pin, HIGH);   
    delay(blinkRate);                
    digitalWrite(Pin, LOW);   
    delay(blinkRate);
  }
}

// Error control
void error() {
  while(1){                                              // Freeze the program because of the error
    blinkLed(TestLedPin, e, 200);
    delay(1000);
  }                                                
}

//  Utility function to print numbers leading 0
void printDigits(int digits){
  if(digits < 10){
    datafile.print('0');
  }
  datafile.print(digits);
}

/*
// Restart the program from beginning
 void softReset() {
 asm volatile ("  jmp 0");  
 } 
 */

/* Error codes
 00 No errors
 01 RTC is not running
 02 RTC is not up to date
 03 Unable to set SD card
 04 Unable to create a file on SD card
 05 Unable to write on data file
 06 Incorrect number of sensors
 07 Problems reading Air temperature sensors
 08 Problems reading Snow temperature sensors
 09 Problems reading Bottom snow cover temperature sensors
 10 Problems reading Surface teperature sensors
 11 Problems reading Ground temperature sensors
 */


