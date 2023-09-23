#include <Adafruit_GPS.h>
//#include <Adafruit_PMTK.h>
//#include <NMEA_data.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
//#include <avr/sleep.h>

// Reads temp and GPS data and records to an SD card!
// NOTE: INCOMPLETE CODE, NOT FUNCTIONAL
SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);
#define GPSECHO  true // Echoes GPS readings into serial monitor
//#define LOG_FIXONLY true // Will only log data when GPS has a fix if true. Turn to false for debugging purposes.
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
//#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
//bool usingInterrupt = false;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
const int chipSelect = 10;
File Tdata;
void setup(void)
{
  // start serial port
 Serial.begin(115200);
 delay(5000);
 Serial.print(F("Initializing SD card..."));
 //pinMode(chipSelect, OUTPUT); // gps example told me to set chipselect as output

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F("SD card initialized."));
  delay(1000);

  // Start up the library and GPS
  GPS.begin(9600);
  if (!GPS.begin(9600)) {
    Serial.print(F("GPS Failed to start"));
    while(1);
  }
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 100 millihertz (once every 10 seconds), 1Hz or 5Hz update rate
  GPS.sendCommand(PGCMD_NOANTENNA);
  // Ask for firmware version
  //mySerial.println(PMTK_Q_RELEASE);
  delay(500);
  Serial.print(F("GPS ready --"));
}
//uint32_t timer = millis();


void loop(void)
{ 
  while(!GPS.fix) {
    Serial.print("No GPS signal found!");
    Serial.println(GPS.fix);
    Serial.println(GPS.hour);
    delay(1000);
    //while(1);
  }


  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print(F("Requesting temperatures..."));
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(F("DONE"));
  delay(1000);
  float temp = sensors.getTempCByIndex(0); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  //float temp2 = sensors.getTempCByIndex(1);
  //float temp3 = sensors.getTempCByIndex(2);
  //float temp = ((temp1 + temp2 + temp3) / 3);
  char c = GPS.read();
  if ((c) && (GPSECHO))
    Serial.write(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another

  }

  String Time = (String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds));
  Tdata = SD.open("testdata.txt", FILE_WRITE);
  delay(1000);
  

  if (Tdata&&(GPS.fix)) { //only continues if file opens and the GPS has a fix
  
    delay(1000);
    
    // if you want to debug, this is a good time to do it!
    Serial.print(Time);
    Serial.print(F(", "));
    Serial.print(temp);
    Serial.print(F(", "));
    Serial.print(GPS.latitude);
    Serial.print(F(", "));
    Serial.print(GPS.longitude);
    Tdata.print(Time);
    Tdata.print(", ");
    Tdata.print(temp); // writes temperature to the file
    Tdata.print(", ");
    Tdata.print(GPS.latitude);
    Tdata.print(", ");
    Tdata.print(GPS.longitude);
    Tdata.print("\n"); // goes to new line
    Tdata.close(); // closes file

  }
  
}