/*
  DS3231: Real-Time Clock. Simple example
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-ds3231.html
  GIT: https://github.com/UCTRONICS/UCTRONICS_Arduino_kits
  Web: www.uctronics.com 
*/

#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define countof(a) (sizeof(a) / sizeof(a[0]))
DateTime dt;

void setup()
{
    Serial.begin(9600);   
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.setDateTime(2017, 6, 13, 16, 29, 3,compiled);
}

void loop()
{
  dt = Rtc.DataTime();

  // For leading zero look to DS3231_dateformat example

  Serial.print("Raw data: ");
  Serial.print(dt.year);   Serial.print("-");
  Serial.print(dt.month);  Serial.print("-");
  Serial.print(dt.day);    Serial.print(" ");
  Serial.print(dt.hour);   Serial.print(":");
  Serial.print(dt.minute); Serial.print(":");
  Serial.print(dt.second); Serial.println("");

  delay(1000);
}
