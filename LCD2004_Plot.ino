
#include <Wire.h>
#include <DS1307.h>
#include <LiquidCrystal.h>
#include <Adafruit_BMP085_U.h>
//#include <OneWire.h>
#include <DallasTemperature.h>
#include "plotSymb.h"
#include <SPI.h>
#include <SD.h>

#define UPDATE_TIME_MS 5000
#define PRES_STEP 100
#define PRES_GRAF_LEN 20

static float presArray[PRES_GRAF_LEN];
static byte cnt;
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DS1307 rtc;
Adafruit_BMP085_Unified bmp(10085);
OneWire oneWire(9);
DallasTemperature tempSensor(&oneWire, 9);
//byte tempSensorAddress[] = {0,0,0,0,0,0,0,0};

const int chipSelectSD = 10;

void setup() {
  //Serial.begin(9600);

  if (!SD.begin(chipSelectSD)) {
    //Serial.println("Card error!");    
  }
    
  rtc.begin();
  rtc.fillByYMD(2020,12,16);
  rtc.fillByHMS(23,59,45);
  rtc.fillDayOfWeek(THU);
  rtc.setTime();//write time to the RTC chip

  bmp.begin();
  
  lcd.begin(20, 4);

  lcd.createChar(0, gradus);
  for (byte i = 0; i < 7; i++) {
    lcd.createChar(i + 1, graf[i]);
  }
  
  for (byte i = 0; i < PRES_GRAF_LEN; i++) {
    presArray[i] = 101325.0f;
  }
  cnt = 0;

  tempSensor.begin();
  tempSensor.setResolution(11);
}

void loop() {
  tempSensor.requestTemperaturesByIndex(0);
  lcd.clear();
  lcd.setCursor(0,0);
  rtc.getTime();
  if(rtc.hour < 10)
    lcd.print("0");
  lcd.print(rtc.hour, DEC);
  lcd.print(":");
  if(rtc.minute < 10)
    lcd.print("0");
  lcd.print(rtc.minute, DEC);
  lcd.print(":");
  if(rtc.second < 10)
    lcd.print("0");
  lcd.print(rtc.second, DEC);

  lcd.print("  ");
  if(rtc.dayOfMonth < 10)
    lcd.print(" ");
  lcd.print(rtc.dayOfMonth, DEC);
  lcd.print(".");
  if(rtc.month < 10)
    lcd.print("0");
  lcd.print(rtc.month, DEC);
  lcd.print(".");
  lcd.print(rtc.year+2000, DEC);

  lcd.setCursor(0,1);
  lcd.print("T1:");
  float temp;
  bmp.getTemperature(&temp);
  char str[5];
  dtostrf(temp, 5, 1, str);
  lcd.print(str);
//  lcd.write((byte)0);
//  lcd.print("C");

  lcd.print(" T2:");
  //tempSensor.requestTemperatures();
  while(!tempSensor.isConversionComplete()) {
    delay(100);
  }
  float temp2 = tempSensor.getTempCByIndex(0);
  dtostrf(temp2, 5, 1, str);
  lcd.print(str);
//  lcd.write((byte)0);
//  lcd.print("C");

  lcd.setCursor(0,2);
  lcd.print("P:");
  float pres;
  bmp.getPressure(&pres);
  //char str[5];
  //dtostrf(pres/133.3f, 5, 1, str);
  lcd.print(pres);
  //lcd.print(" mmHg");
  //lcd.print(" Pa");

  presArray[cnt] = pres;
  cnt++;
  if (cnt == PRES_GRAF_LEN) {
    cnt = 0;
  }

  lcd.setCursor(0,3);
  float presMin = 3.4e38;
  for (byte i = 0; i < PRES_GRAF_LEN; i++) {
    if (presArray[i] < presMin) {
      presMin = presArray[i];
    }
  }
  for (byte i = 0; i < PRES_GRAF_LEN; i++) {
    unsigned int barVal = (unsigned int)(presArray[(i + cnt)%PRES_GRAF_LEN] - presMin) / PRES_STEP;
    if (barVal > 6) {
      barVal = 6;
    }
    lcd.write((byte)(barVal + 1));
  }


  File datafile = SD.open("test.txt", FILE_WRITE);
  if (!datafile) {
    //Serial.println("File open error!");
  }
  String dataStr = "";
  dataStr += String(rtc.year+2000);
  dataStr += String(";");
  dataStr += String(rtc.month);
  dataStr += String(";");
  dataStr += String(rtc.dayOfMonth);
  dataStr += String(";");
  dataStr += String(rtc.hour);
  dataStr += String(";");
  dataStr += String(rtc.minute);
  dataStr += String(";");
  dataStr += String(rtc.second);
  dataStr += String(";");
  dataStr += String(temp);
  dataStr += ";";
  dataStr += String(pres);
  datafile.println(dataStr);
  datafile.close();

  delay(UPDATE_TIME_MS);
}
