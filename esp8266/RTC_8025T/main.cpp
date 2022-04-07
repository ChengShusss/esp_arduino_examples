#include <Arduino.h>

#include "rtc.h"

#define SDA_PIN 4         // SDA引脚，默认gpio4(D2)
#define SCL_PIN 5         // SCL引脚，默认gpio5

RX8025 rtc;
DateTime datetime;

void setup(void)
{
  Serial.begin(9600);
  Serial.println("on the setup");
  
  // Wire初始化
  Wire.begin();
  rtc.RX8025_init();
  rtc.setRtcTime(35, 33, 0, 8, 4, 22);
}

void loop(void)
{
  Serial.print(rtc.getYear());
  Serial.print("年");
  Serial.print(rtc.getMonth());
  Serial.print("月");
  Serial.print(rtc.getDate());
  Serial.print("日");
  Serial.print(rtc.getHour());
  Serial.print(":");
  Serial.print(rtc.getMinute());
  Serial.print(":");
  Serial.println(rtc.getSecond());
  delay(1000);
  Serial.println(rtc.getUnixtime());
}