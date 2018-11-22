#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

//char ssid[] = "HWU_Research";
//char ssid[] = "Sara's iPhone (2)";   // your network SSID (name) 
//char ssid[] = "iPhone de Linda";
char ssid[] = "AndroidAP";
//char pass[] = "Her1ot-W@tt";
//char pass[] = "g16xa5pah52ef";   // your network password
//char pass[] = "8i2nqx4kyce17";
char pass[] = "robot2018";

int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long pulseRateChannelNumber = 620080;
unsigned int myFieldNumber = 1;
const char * myWriteAPIKey = "NPHOWFAKQ1V4IH3B";

int pulseValue = 0;

void setup() {
  Serial.begin(115200);  // Initialize serial

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

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

  //pulseValue = Serial.read();
  //Serial.println("Pulse Rate Data: " + String(pulseValue));
  Serial.print(Serial.read());
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeField(pulseRateChannelNumber, myFieldNumber, pulseValue, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(5000); // Wait 20 seconds to update the channel again
}
