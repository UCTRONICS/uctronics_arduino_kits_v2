/***********************************************************
File name: Lesson_24_real_time_clock_module.ino
Description: Calibration clock.
Website: www.uctronics.com 
E-mail: sales@uctronics.com 
Author: Lee
Date: 2017/06/12
***********************************************************/
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(14,15,16); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
DateTime dt;

void setup() 
{
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.setDateTime(2021,4,13,13,49,0,compiled);
}

void loop() 
{

}
