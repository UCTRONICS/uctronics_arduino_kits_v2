/*
  DS3231: Real-Time Clock. Date Format
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
    Serial.begin(57600);   
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.setDateTime(2017, 6, 13, 16, 29, 3,compiled);
}

void loop()
{
  char buffer[150]={0};
  dt = Rtc.DataTime();
  Serial.print("Long number format:          ");
  Rtc.dateFormat("d-m-Y H:i:s", dt,buffer);
  Serial.println(buffer);  
  Serial.print("Long format with month name: ");
  Rtc.dateFormat("d F Y H:i:s",  dt,buffer);
  Serial.println(buffer);  
  Serial.print("Short format witch 12h mode: ");
  Rtc.dateFormat("jS M y, h:ia",  dt,buffer);
  Serial.println(buffer); 
  Serial.print("Today is:                    ");
  Rtc.dateFormat("l, z",  dt,buffer);
  Serial.print(buffer); 
  Serial.println(" days of the year.");
  Serial.print("Actual month has:            ");
  Rtc.dateFormat("t",  dt,buffer);
  Serial.print(buffer); 
  Serial.println(" days.");
  Serial.print("Unixtime:                    ");
  Rtc.dateFormat("U",  dt,buffer);      
  Serial.println(buffer); 
  delay(1000);
}

