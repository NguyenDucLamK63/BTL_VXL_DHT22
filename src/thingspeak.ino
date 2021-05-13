#include "DHT.h"
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network passwordint keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

String myStatus = "";
#define DHTPIN 4 
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(115200);  //Initialize serial
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println(F("DHTxx test!"));

  dht.begin();
}

void loop() {
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);

   /*int number1 = random(0,100);
   int number2 = random(0,100);
   int number3 = random(0,100);
   int number4 = random(0,100);*/
  // set the fields with the values
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  //ThingSpeak.setField(3, number3);
  //ThingSpeak.setField(4, number4);

  // figure out the status message
  
  
  // write to the ThingSpeak channel
   ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  
  delay(2000); // Wait 20 seconds to update the channel again
}
