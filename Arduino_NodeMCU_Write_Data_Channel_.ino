#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

  
char ssid[] = "AndroidAP";  // your network SSID (name) 
char pass[] = "robot2018";  // your network password

int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long pulseRateChannelNumber = 620080; //ThingSpeak channel ID
unsigned int pulseField = 1; //field corresponding to the pulse rate. For one patient, this is 1
unsigned int accelerometerField = 2;  //field corresponding to the accelerometer value
const char * myWriteAPIKey = "NPHOWFAKQ1V4IH3B"; //Write API key of the ThingSpeak channel

int pulseValue ;
int accelerometerValue = 0;

void setup() {
  Serial.begin(115200);  // Initialize serial

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {  //establishes connection to a wifi network

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  if (Serial.available())
  {
    String data = Serial.readString();  //reads data from the serial port, which is a string consisting of pulse rate and acceleroemter values
    //data is processed so that only the last pulse rate/accelerometer values are obtained. This is due to the fact that ThingSpeak doesn't 
    //allow updating the channel more than every 15 seconds so the data is accumulated
    
    int lastOpeningBracket = data.lastIndexOf('\n');
    int lastTwo = data.lastIndexOf('\n', lastOpeningBracket -1);
    String newString = data.substring(lastTwo+1, lastOpeningBracket);
    int delimiterIndex = newString.indexOf(';');
    String pulse_str = newString.substring(0, delimiterIndex);
    String accelerometer_str = newString.substring(delimiterIndex+1 );

    int pulseValue = pulse_str.toInt();
    int accelerometerValue = accelerometer_str.toInt();
    Serial.println(pulseValue); //prints the pulse rate and accelerometer values
    Serial.println(accelerometerValue);
  
    // Writes both pulse rate and accelerometer values to ThingSpeak to their corresponding fields
    ThingSpeak.setField(pulseField, pulseValue);
    ThingSpeak.setField(accelerometerField, accelerometerValue);
    ThingSpeak.writeFields(pulseRateChannelNumber, myWriteAPIKey);
    
    delay(5000); // Wait 5 seconds to update the channel again*/
  }
}
