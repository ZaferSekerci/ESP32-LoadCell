#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HX711.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeMono9pt7b.h>
#include "esp32-hal-cpu.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "sNilwAwLOEY69xx2LhQBw2QPDr8xMcpi";
BlynkTimer timer;

// WiFi bilgileri.
// Şifresiz wifi ise pass bölümünü "" şeklinde bırakın
char ssid[] = "Confectioner ek";
char pass[] = "artemispardus*123";

//5110 connections
#define RST 18
#define CE 5
#define DC 4
#define DIN 2
#define CLK2 15
//Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK2);
Adafruit_PCD8544 lcd = Adafruit_PCD8544(CLK2, DIN, DC, CE, RST); //RST, CE, DC, DIN, CLK2

//load cell library
#define CLK 25
#define DOUT 33
HX711 scale;

//variables for loadcell values
float load;
float tempLoad; // gecici ağırlık değişkeni
float difLoad;  // çıkan yada eklenen farkı tutan değişken

void sendSensor()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, load);
  Blynk.virtualWrite(V2, difLoad);
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V0 to a variable
  if (pinValue > 0)
  {
    tempLoad = load; // telefondaki sanal tuş basıldığında
  }
}

void updateLcd(int row, int col, float val, const char *name)
{
  lcd.setCursor(col, row);
  //lcd.clear(row, 0, 84);

  lcd.printf("%s %.2f \n", name, val);

  lcd.display();
}

void getLoad()
{
  //scale.read_average(10);
  load = scale.get_units(10);
}

void tare()
{
  scale.read_average(10);
  delay(300);
  scale.tare();
  delay(300);
  scale.read_average(10);
}

void setup()
{
  setCpuFrequencyMhz(80); //cpu hızını 80mhz e düşürüyoruz
  //Serial.begin(115200);

  pinMode(11, OUTPUT);
  //loadcell okuma başlatılıyor
  scale.begin(DOUT, CLK);
  //scale.read_average(20);
  scale.set_scale(); // Kalibrasyon böleni tek loadcell=151.8
  tare();
  //scale.set_offset();
  //scale.tare();           // dara alma komutu
  //delay(1000);
  //scale.read_average(20); // açılışta ortalama okuma yapma için

  //lcd ekran konstrast değeri
  lcd.begin();
  lcd.setContrast(50);
  lcd.setTextSize(1, 2);

  //blynk bağlantısı başlıyor
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  getLoad();
  updateLcd(0, 0, load, "YUK:");
  difLoad = load - tempLoad;
  updateLcd(20, 0, difLoad, "FARK:");
  lcd.clearDisplay();

  Blynk.run();
  timer.run();

  if (touchRead(13) < 30)
  {
    tempLoad = load;
  }
}