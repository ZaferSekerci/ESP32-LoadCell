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
char auth[] = "sNilwAwLOEY69xx2LhQBw2QPDr8xMcpi"; //blynkden alınacak key
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
#define lcdCLK 15
//Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK2);
Adafruit_PCD8544 lcd = Adafruit_PCD8544(lcdCLK, DIN, DC, CE, RST); //RST, CE, DC, DIN, CLK2

//load cell library
#define CLK 25
#define DOUT 33
#define DOUT2 32
#define DOUT3 35
#define DOUT4 34

HX711 scale;
HX711 scale2;
HX711 scale3;
HX711 scale4;
float calibration_factor = 103.7; //hesaplanan kalibrasyon böleni

//variables for loadcell values
float load;     //asıl ağırlığımızı tuttuğumuz kısım
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
  int pinValue = param.asInt();
  if (pinValue > 0)
  {
    tempLoad = load; // telefondaki sanal tuş basıldığında
  }
}

void updateLcd(int row, int col, float val, const char *name)
{
  lcd.setCursor(col, row);             //imleci istenen yere getirme
  lcd.printf("%s %.2f \n", name, val); //gelen değerleri yazma
  lcd.display();                       // değişiklikleri onaylama
}

void getLoad()
{
  load = 0;
  load = scale.get_units(5) + scale2.get_units(5) + scale3.get_units(5) + scale4.get_units(5); //4 hücreden 5er adet ortalama değerler okuyup toplamını load a yazıyoruz
}

void tare()
{
  scale.read_average(10); // 10 adet değer okuyup ortalamasını alıyoruz
  delay(100);             //değerlerin kararlı hale gelmesi için 100ms bekleme
  scale.tare();           //dara alma işlemi gerçekleşiyor
                          //yukarıdaki işlem diğer tüm hücreler için tekrarlanıyor
  scale2.read_average(10);
  delay(100);
  scale2.tare();

  scale3.read_average(10);
  delay(100);
  scale3.tare();

  scale4.read_average(10);
  delay(100);
  scale4.tare();
}

void setup()
{
  setCpuFrequencyMhz(80); //cpu hızını 80mhz e düşürüyoruz
  Serial.begin(115200);   //bilgisayardan haberleşme için

  scale.begin(DOUT, CLK);
  scale2.begin(DOUT2, CLK);
  scale3.begin(DOUT3, CLK);
  scale4.begin(DOUT4, CLK);

  scale.set_scale(calibration_factor);
  scale2.set_scale(calibration_factor);
  scale3.set_scale(calibration_factor);
  scale4.set_scale(calibration_factor);
  tare(); // tüm hücrelerin darasını alan metod

  lcd.begin();                          // lcd ekran başlatılıyor
  lcd.setContrast(50);                  //lcd ekran konstrast değeri
  lcd.setTextSize(1, 2);                // ekran yazı boyutu ayarı
  lcd.clearDisplay();                   // açılışta ekranı temizleme
  Blynk.begin(auth, ssid, pass);        //blynk bağlantısı başlıyor
  timer.setInterval(1000L, sendSensor); //blynk için bir sayaç
}

void loop()
{
  getLoad();                          //ağırlık değişkenini güncelleyen metod
  updateLcd(0, 0, load, "YUK:");      // 0 - 0 koordinatlarından başlayarak lcd ekrana load değişkenini yazdırma
  difLoad = load - tempLoad;          // eklenen yada çıkarılan değeri hesaplamak için
  updateLcd(20, 0, difLoad, "FARK:"); //20 - 0 koordinatlarından başlayarak lcd ekrana load değişkenini yazdırma
  lcd.clearDisplay();                 //gösterimler ardından ekranı temizlemek için

  Blynk.run(); //blynk sistemini çalıştırıyoruz
  timer.run(); //blynk sayacını çalıştırıyoruz

  if (touchRead(13) < 30)
  {
    tempLoad = load;
  }
  /* Kalibrasyon için ayrı blok
  scale.set_scale(calibration_factor);
  scale2.set_scale(calibration_factor);
  scale3.set_scale(calibration_factor);
  scale4.set_scale(calibration_factor);

  Serial.println(scale.get_units(5) + scale2.get_units(5) + scale3.get_units(5) + scale4.get_units(5));
  Serial.print(" calibration_factor: ");
  Serial.println(calibration_factor);

  if (Serial.available()) // okunan değer takip edilerek + - ile optimum değer bulunarak en tepedeki değişken güncellenecek
  {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 0.1;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 0.1;
  }
  */
}