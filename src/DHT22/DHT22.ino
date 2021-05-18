#include "DHT.h"
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network passwordint keyIndex = 0;            // your network key Index number (needed only for WEP)
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";
// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
  function submitMessage() {
    alert("Submit Done");
    setTimeout(function(){ document.location.reload(false); }, 500);
  }
</script>
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <!-- <input type="submit" value="Submit"> -->
    input2: <input type="text" name="input2">
    <!-- <input type="submit" value="Submit"> -->
    input3: <input type="text" name="input3">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form>
</body></html>)rawliteral";
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

String myStatus = "";
#define DHTPIN 4 
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3F, 16, 2);
byte degree_symbol[8] = 
              {
                0b00111,
                0b00101,
                0b00111,
                0b00000,
                0b00000,
                0b00000,
                0b00000,
                0b00000
              };
void setup() {
  Serial.begin(115200);  //Initialize serial
  WiFi.mode(WIFI_STA);   
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage_1;
    String inputMessage_2;
    String inputMessage_3;
    //String inputParam_1;
    //String inputParam_2;
    //String inputParam_3;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)&& request->hasParam(PARAM_INPUT_2) && request->hasParam(PARAM_INPUT_3)) {
      inputMessage_1 = request->getParam(PARAM_INPUT_1)->value();
      //inputParam_1 = PARAM_INPUT_1;
      inputMessage_2 = request->getParam(PARAM_INPUT_2)->value();
      //inputParam_2 = PARAM_INPUT_2;
      inputMessage_3 = request->getParam(PARAM_INPUT_3)->value();
      //inputParam_3 = PARAM_INPUT_3;
    }
    else {
      inputMessage_1 = "No message sent";
      //inputParam_1 = "none";
      inputMessage_2 = "No message sent";
      //inputParam_2 = "none";
      inputMessage_3 = "No message sent";
      //inputParam_3 = "none";
    }
    //String val = “1234” ;
    float result_1 = inputMessage_1 . toFloat();
    float result_2 = inputMessage_2 . toFloat();
    float result_3 = inputMessage_3 . toFloat();
    Serial.println(result_1 + result_2 + result_3);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input 1 , input 2, intput 3 " 
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  
}
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println(F("DHTxx test!"));

  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degree_symbol);
  lcd.setCursor(0,0);
  lcd.print("  DHT22   with ");
  lcd.setCursor(0,1);
  lcd.print("  ESP32 DevKit ");
  delay(2000);
  lcd.clear();
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
  lcd.setCursor(0,0);
  lcd.print("Temp = ");
  lcd.print(t);
  lcd.write(0);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity = ");
  lcd.print(h);
  lcd.print("%");
  delay(1000);
  // set the fields with the values
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  
  delay(2000); // Wait 20 seconds to update the channel again
}
