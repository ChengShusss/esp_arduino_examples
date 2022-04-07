
#include "rtc.h"
// These included for the DateTime class inclusion; will try to find a way to
// not need them in the future...
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
// Changed the following to work on 1.0
//#include "WProgram.h"
#include <Arduino.h>

// 8025I2C地址
#define RX8025_address 0x32
// 日期起始时间(这里为啥要减掉八个小时,因为用的日期所在时区和国内时区相差8小时,所以需要减掉八小时的时区时间)
#define SECONDS_FROM_1970_TO_2000 946684800 - (8 * 60 * 60)
//
static const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**
   自2000/01/01起的天数，2001年有效。。2099
*/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && isleapYear(y))
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
  return ((days * 24L + h) * 60 + m) * 60 + s;
}

DateTime::DateTime(uint32_t t)
{
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

  ss = t % 60;
  t /= 60;
  mm = t % 60;
  t /= 60;
  hh = t % 24;
  uint16_t days = t / 24;
  uint8_t leap;
  for (yOff = 0;; ++yOff)
  {
    leap = isleapYear(yOff);
    if (days < 365 + leap)
      break;
    days -= 365 + leap;
  }
  for (m = 1;; ++m)
  {
    uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
    if (leap && m == 2)
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  d = days + 1;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
  if (year >= 2000)
  {
    year -= 2000;
  }
  yOff = year;
  m = month;
  d = day;
  hh = hour;
  mm = min;
  ss = sec;
}

// supported formats are date "Mmm dd yyyy" and time "hh:mm:ss" (same as __DATE__ and __TIME__)
DateTime::DateTime(const char *date, const char *time)
{
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  static char buff[4] = {'0', '0', '0', '0'};
  int y;
  sscanf(date, "%s %c %d", buff, &d, &y);
  yOff = y >= 2000 ? y - 2000 : y;
  m = (strstr(month_names, buff) - month_names) / 3 + 1;
  sscanf(time, "%c:%c:%c", &hh, &mm, &ss);
}

// UNIX time: IS CORRECT ONLY WHEN SET TO UTC!!!
uint32_t DateTime::unixtime(void) const
{
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2long(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000
  return t;
}

// Slightly modified from JeeLabs / Ladyada
// Get all date/time at once to avoid rollover (e.g., minute/second don't match)
static uint8_t bcd2bin(uint8_t val)
{
  return val - 6 * (val >> 4);
}
// Commented to avoid compiler warnings, but keeping in case we want this
// eventually
static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

/**
   判断是否是闰年
*/
bool isleapYear(const uint8_t y)
{
  //检查是否可以被4整除
  if (y & 3)
  {
    return false;
  }
  // 仅在第一次失败时检查其他
  return (y % 100 || y % 400 == 0);
}

RX8025::RX8025() // costruttore
{
  RX8025_Control[0] = 0x20;
  RX8025_Control[1] = 0x00;
}

/**
 * 向时钟芯片设置时间
 * @param s  秒钟
 * @param m  分钟
 * @param h 时钟
 * @param d  天
 * @param mh 月
 * @param y 年
 */
void RX8025::setRtcTime(uint8_t s, uint8_t m, uint8_t h, uint8_t d, uint8_t mh, uint8_t y)
{
  // 使用指定的地址开始向I2C从设备进行传输。
  Wire.beginTransmission(RX8025_address);
  Wire.write((byte)0x00);
  Wire.write(decToBcd(s));
  Wire.write(decToBcd(m));
  Wire.write(decToBcd(h));
  Wire.write(0x1);
  Wire.write(decToBcd(d));
  Wire.write(decToBcd(mh));
  Wire.write(decToBcd(y));
  // 停止与从机的数据传输
  Wire.endTransmission();
}

/**
 * 获取寄存器里面的数据
 * @param regaddr
 * @return byte
 */
byte RX8025::getData(byte regaddr)
{
  // 使用指定的地址开始向I2C从设备进行传输。
  Wire.beginTransmission(RX8025_address);
  Wire.write(regaddr);
  // 停止与从机的数据传输
  Wire.endTransmission();
  // 由主设备用来向从设备请求字节。
  Wire.requestFrom(RX8025_address, 1);
  // 读取数据
  return Wire.read();
}

/**
 * @brief 初始化函数
 *
 */
void RX8025::RX8025_init(void)
{
  // 使用指定的地址开始向I2C从设备进行传输。
  Wire.beginTransmission(RX8025_address);
  Wire.write(0xe0);
  for (unsigned char i = 0; i < 2; i++)
  {
    Wire.write(RX8025_Control[i]);
  }
  // 使用指定的地址开始向I2C从设备进行传输。
  Wire.endTransmission();
}

/**
 * @brief 将十进制编码的二进制数转换为普通十进制数
 *
 * @param val
 * @return byte
 */
byte RX8025::decToBcd(byte val)
{
  // 将十进制编码的二进制数转换为普通十进制数
  return ((val / 10 * 16) + (val % 10));
}

/**
 * 将二进制编码的十进制数转换为普通十进制数
 *
 * @param val
 * @return byte
 */
byte RX8025::bcdToDec(byte val)
{
  // 将二进制编码的十进制数转换为普通十进制数
  return ((val / 16 * 10) + (val % 16));
}

/**
 * 获取秒钟
 */
byte RX8025::getSecond()
{
  byte buff = getData(RX8025_SEC);
  return bcdToDec(buff & 0x7f);
}

/**
 * 获取分钟数
 * @return byte
 */
byte RX8025::getMinute()
{
  byte buff = getData(RX8025_MIN);
  return bcdToDec(buff & 0x7f);
}

/**
 * 获取小时数
 * @return byte
 */
byte RX8025::getHour()
{
  byte buff = getData(RX8025_HR);
  return bcdToDec(buff & 0x3f);
}

/**
 * 获取星期数
 * @return byte
 */
byte RX8025::getDoW()
{
  byte buff = getData(RX8025_WEEK);
  return bcdToDec(buff & 0x07);
}

/**
 * 获取日期
 * @return byte
 */
byte RX8025::getDate()
{
  byte buff = getData(RX8025_DATE);
  return bcdToDec(buff & 0x3f);
}

/**
 * 获取月份
 * @return byte
 */
byte RX8025::getMonth()
{
  byte buff = getData(RX8025_MTH);
  return bcdToDec(buff & 0x1f);
}

/**
 * 获取年份
 * @return byte
 */
byte RX8025::getYear()
{
  byte buff = getData(RX8025_YR);
  return bcdToDec(buff & 0xff);
}

long RX8025::getUnixtime()
{
  // 使用指定的地址开始向I2C从设备进行传输。
  Wire.beginTransmission(RX8025_address);
  Wire.write(0x00);
  // 停止与从机的数据传输
  Wire.endTransmission();
  // 由主设备用来向从设备请求字节。
  Wire.requestFrom(RX8025_address, 7);
  // 读取数据
  uint16_t ss = bcdToDec(Wire.read() & 0x7F);
  uint16_t mm = bcdToDec(Wire.read() & 0x7f);
  uint16_t hh = bcdToDec(Wire.read() & 0x3f);
  Wire.read();
  uint16_t d = bcdToDec(Wire.read() & 0x3f);
  uint16_t m = bcdToDec(Wire.read() & 0x1f);
  uint16_t y = bcdToDec(Wire.read() & 0xff) + 2000;
  return DateTime(y, m, d, hh, mm, ss).unixtime();
}