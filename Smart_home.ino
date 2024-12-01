/*

 * + Đo các thông số nhiệt độ và độ ẩm, bụi của nhà thông qua thiết bị điện thoại.
+ Bật/ tắt, điều khiển các thiết bị điện thông qua điện thoại. 
+ Bật điện khi trời tối, tắt điện khi trời sáng. 
+ Vườn thông minh: có cảm biến độ ẩm đất. 
Khi độ ẩm đất xuống thấp hệ thống tự động bật máy bơm, bơm nước vườn cây. 
+ Hệ thống rò rỉ khí ga: Khi ga bị rò rỉ hoặc nhiệt độ cao hệ thống sẽ kêu lên gửi thông báo. 
+ Giàn phơi tự động: Khi có mưa giàn phơi tự động kéo giàn phơi ra để phơi quần ào. Khi trời nắng giàn phơi thu vào.
 */
 #include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Địa chỉ I2C của màn hình LCD 2004 (thường là 0x27 hoặc 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Khai báo màn hình 20x4  
#define BLYNK_PRINT Serial
#define BLYNK_AUTH_TOKEN  "BUdRcutM0gS-FW5rbNtlwY4tAt24TPGQ"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char ssid[] = "HAN";
char pass[] = "26081991";
bool tt = false, tt1 = false;
#include <ESP32_Servo.h> 

Servo gianphoi;
Servo cong;
Servo thang;
#include "DHT.h"
#define DHTPIN 15 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE); 
#define denpk 25
#define quatpk 26

#define denpn 27
#define dennb 14

#define denhl 4
#define cbquang 17

#define cbdat 36
#define bom 16

#define cbgas 18
#define coi 5

#define cbmua 19
#define servo 23
#define thongbao 2
float Tset = 40;

#include "SharpGP2Y10.h"
#include <PMsensor.h>
PMsensor PM;
int voPin = 39;
int ledPin = 13;
float mdbui = 0;
#define thoathiem 33
BLYNK_WRITE(V3)
{
  digitalWrite(denpk, param.asInt());
}
BLYNK_WRITE(V4)
{
  digitalWrite(quatpk, param.asInt());
}
BLYNK_WRITE(V5)
{
  digitalWrite(denpn, param.asInt());
}
BLYNK_WRITE(V6)
{
  digitalWrite(dennb, param.asInt());
}
BLYNK_WRITE(V8)
{
  cong.write(param.asInt());
}
long timeloa;
void setup() {
  Serial.begin(9600);
  /////(infrared LED pin, sensor pin)  /////
  PM.init(ledPin, voPin);
  lcd.init();
  lcd.backlight(); // Bật đèn nền
  lcd.setCursor(0, 0);
  lcd.print("   DANG KHOI DONG   ");
  Serial.println(F("DHTxx test!"));
  gianphoi.attach(servo, 500, 2400);
  pinMode(thoathiem, OUTPUT);
  digitalWrite(thoathiem, HIGH);
  cong.attach(12, 500, 2400);
  thang.attach(32, 500, 2400);
  gianphoi.write(0);
  cong.write(95);
  thang.write(95);
  dht.begin();
  pinMode(denpk, OUTPUT);
  pinMode(quatpk, OUTPUT);
  pinMode(dennb, OUTPUT);
  pinMode(denpn, OUTPUT);
  pinMode(denhl, OUTPUT);
  pinMode(bom, OUTPUT);
  pinMode(coi, OUTPUT);
  digitalWrite(bom, HIGH);
  pinMode(cbquang, INPUT_PULLUP);
  pinMode(cbgas, INPUT_PULLUP);
  pinMode(cbmua, INPUT_PULLUP);
  pinMode(thongbao, INPUT_PULLUP);
  pinMode(cbdat, INPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass,"blynk-server.com",8080);
  delay(1000);
  Blynk.run();
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V4, 0);
  Blynk.virtualWrite(V5, 0);
  Blynk.virtualWrite(V6, 0);
  Blynk.virtualWrite(V8, 0);
  lcd.clear();
}
void loop(){
  Blynk.run();
  ////////////////Đọc cảm biến nhiệt độ, độ ẩm
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  lcd.setCursor(0, 0);
  lcd.print("T:     *C ");
  lcd.setCursor(4, 0);
  lcd.print(t);
  lcd.setCursor(10, 0);
  lcd.print("H:     %  ");
  lcd.setCursor(13, 0);
  lcd.print(h);
  if(t >= Tset){
    if(millis() - timeloa <= 3000){
    digitalWrite(coi, HIGH);}
    else digitalWrite(coi, LOW);
    if(tt == false){
        Blynk.notify("CANH BAO NHIET DO QUA CAO");
        lcd.setCursor(0, 2);
  lcd.print("  NHIET DO QUA CAO  ");
    }
    tt = true;
  }
  else timeloa = millis();
  ///////////////Đọc cảm biến quang
  digitalWrite(denhl, digitalRead(cbquang));

  ///////////////Đọc c4095ảm biến độ ẩm đất
  int val = map(analogRead(cbdat),2047,4095,100,0);
  lcd.setCursor(0, 1);
  lcd.print("M:    %");
  lcd.setCursor(3, 1);
  lcd.print(val);
  Blynk.virtualWrite(V2, val);
  if(val <= 60) digitalWrite(bom, LOW);
  if(val >= 90) digitalWrite(bom, HIGH);

  ///////////////Đọc cảm biến khí gas
  int val1 = digitalRead(cbgas);
  
  if(val1 == 0){
    digitalWrite(coi, HIGH);
    digitalWrite(thoathiem, LOW);
    Blynk.virtualWrite(V8, 0);
    thang.write(0);
    cong.write(0);
    if(tt == false){
        Blynk.notify("PHAT HIEN RO RI KHI GAS");
        lcd.setCursor(0, 2);
  lcd.print("   RO RI KHI GAS   ");
    }
    tt = true;
  }
  else {
    digitalWrite(thoathiem, HIGH);
    thang.write(95);
  }
  Serial.println(digitalRead(thongbao));
  if(digitalRead(thongbao) == 0 && tt1 == true){
    Blynk.notify("CANH BAO CO NGUOI DOT NHAP");
    tt1= false;
  }
  else tt1 = true;
   if( val1 && t <= Tset){
    lcd.setCursor(0, 2);
  lcd.print("    BINH THUONG    ");
    digitalWrite(coi, LOW);
    tt = false;
  }
  
  
  ///////////////Đọc cảm biến mưa
  if(digitalRead(cbmua) == 1){
    gianphoi.write(45);  
  }
  else gianphoi.write(0); 
////////////////Đọc cảm biến mật độ bụi
  float pm = PM.read();
  pm = pm + 40;
  Serial.print("PM2.5: ");
  Serial.print(pm);
  Serial.println(" ppm");
   lcd.setCursor(8, 1);
  lcd.print("PM:         ");
  lcd.setCursor(11, 1);
  lcd.print(pm);
   Blynk.virtualWrite(V7, pm);
}
