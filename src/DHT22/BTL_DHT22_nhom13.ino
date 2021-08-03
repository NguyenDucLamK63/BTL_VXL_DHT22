#include "DHT.h"  // gọi thư viện DHT22
#include <WiFi.h> //Gọi thư viện để kết nối WiFi
#include <Wire.h> //Gọi thư viện để đi dây cho LCD
#include <WiFiClient.h> // Gọi thư viện WiFiclient
#include "ThingSpeak.h" // Thư viện để gửi dữ liệu lên Thingspeak
#include "Timer.h"  //Dùng thư viện timer để ngắt
#include "LiquidCrystal_I2C.h" //Thư viện để kết nối và hiển thị LCD
#include <BlynkSimpleEsp32.h>

#define SECRET_SSID "Canh Lam_2.4G"    // Biến lưu tên WiFi
#define SECRET_PASS "duclam2k"         // Biến lưu mật khẩu WiFi
#define SECRET_CH_ID 1462888           // Biến lưu địa chỉ ID của ThingSpeak
#define SECRET_WRITE_APIKEY "8MZORTFQOAQY4Y45"   //Biến lưu mã API Key của ThingSpeak
#define DHTPIN 4 //Define chân DHT22 ở chân số 4 của ESP32
#define DHTTYPE DHT22 //Kiểu cảm biến là DHT22

char auth[] = "pKm-ol8DBwL4L5pNITC-MKAFFJ21sDoC"; // Biến lưu mã Auth Token để kết nối với app Blink
WiFiClient  client;  //Client bắt đầu được yêu cầu
//Lưu ID và API key của ThingSpeak vào biến mới
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";
LiquidCrystal_I2C lcd(0x27, 16, 2); //Kết nối với LCD sau khi biết địa chỉ của nó
DHT dht(DHTPIN, DHTTYPE);         //Khai báo dht
Timer timer;                      //khai báo timer
BlynkTimer blynkTimer;            //Khai báo timer cho blynk
float h ;                         //Biến lưu giá trị độ ẩm
float t ;                         //Biến lưu giá trị nhiệt độ
int PIN = 2;                   //Chân điều khiển bật tắt tưới tiêu
void setup() {
  // setup chân bật tắt tưới tiêu
  pinMode(PIN, OUTPUT);
  pinMode(PIN, HIGH);
  Serial.begin(115200);  // Khởi tạo Serial
  ThingSpeak.begin(client);  // Khởi tạo ThingSpeak
  Serial.println(F("DHT22 test!"));  // In giá trị DHT22 test!
  dht.begin();             //DHT22 bắt đầu hoạt động
  //Cứ sau 5 giây, WiFi kiểm tra kết nối lại 1 lần
  timer.every(5000, connectWiFi);
  //Cứ sau 1 giây, DHT22 sẽ đọc dữ liệu 1 lần
  timer.every(1000, readDHT);
  //Kết nối với app blink và cứ sau 1 giây thì gửi dữ liệu 1 lần
  blynkTimer.setInterval(1000L, sendBlynk);
  Blynk.begin(auth, SECRET_SSID, SECRET_PASS);
  //Cứ sau 5 giây, LCD để cập nhật dữ liệu để hiển thị 1 lần
  timer.every(5000, DisplayLCD);
  //Cứ sau 20 giây,ThingSpeak sẽ cập nhật dữ liệu 1 lần
  timer.every(20000, sendToThingspeak);
}

void loop() {
  timer.update(); // Update timer
  // Chạy app blink
  Blynk.run();
  blynkTimer.run();
  //khi nhiệt độ lớn hơn 30 độ C và độ ẩm nhỏ hơn 75% thì máy bơm hoạt động
  if(t >= 50 && h <= 10 )
  {
  digitalWrite(PIN,LOW);
  } 
  else
  {
  digitalWrite(PIN,HIGH);
  }   
}

// Hàm kết nối WiFi
void connectWiFi() {
  Serial.println("WIFI");
  WiFi.mode(WIFI_STA);
  // Nếu WiFi không được kết nối với ESP32
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    Serial.print(".");
    Serial.println("\nWiFi Connected.");
    Serial.println(WiFi.localIP());
  }
}

//Hàm đọc và in dữ liệu DHT22 lên serial monitor
void readDHT() {
  h = dht.readHumidity(); // Đọc dữ liệu độ ẩm
  t = dht.readTemperature(); // Đọc dữ liệu nhiệt độ
  // Kiểm tra xem DHT22 có đọc được dữ liệu hay không
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // In dữ liệu lên serial monitor
  Serial.print(F("Độ ẩm : "));
  Serial.print(h);
  Serial.print(F("%  Nhiệt độ : "));
  Serial.println(t);
}

//Hàm gửi dữ liệu lên app blink
void sendBlynk() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Blynk.virtualWrite(V5, t); // Kết nối chân V5 trên app Blink để gửi nhiệt độ
  Blynk.virtualWrite(V6, h); // Kết nối chân V5 trên app Blink để gửi độ ẩm
}

//Hiển thị dữ liệu nhiệt độ độ ẩm lên LCD
void DisplayLCD() {
  //Kết nối các chân của I2C với ESP32
  Wire.begin(2, 0) ;
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Nhiet do: ");
  lcd.print(t);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Do am: ");
  lcd.print(h);
  lcd.print("%");
}

//Hàm gửi dữ liệu lên ThingSpeak
void sendToThingspeak() {
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  //Hiển thị vào kênh mình đã tạo
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
}
