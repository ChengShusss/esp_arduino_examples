#ifndef RX8025_h
#define RX8025_h

#define RX8025_SEC 0x0
#define RX8025_MIN 0x1
#define RX8025_HR 0x2
#define RX8025_WEEK 0x3
#define RX8025_DATE 0x4
#define RX8025_MTH 0x5
#define RX8025_YR 0x6
#define RX8025_Doffset 0x7
#define RX8025_AW_MIN 0x8
#define RX8025_AW_HR 0x9
#define RX8025_AW_WEEK 0xa
#define RX8025_AD_MIN 0xb
#define RX8025_AD_HR 0xc
#define RX8025_CTL1 0xd
#define RX8025_CTL2 0xE

#include <Arduino.h>
#include <Wire.h>
#include <time.h>
// DateTime (get everything at once) from JeeLabs / Adafruit
// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime
{
public:
  DateTime(uint32_t t = 0);
  DateTime(uint16_t year, uint8_t month, uint8_t day,
           uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
  DateTime(const char *date, const char *time);
  uint16_t year() const
  {
    return 2000 + yOff;
  }
  uint8_t month() const
  {
    return m;
  }
  uint8_t day() const
  {
    return d;
  }
  uint8_t hour() const
  {
    return hh;
  }
  uint8_t minute() const
  {
    return mm;
  }
  uint8_t second() const
  {
    return ss;
  }
  uint8_t dayOfTheWeek() const;

  // 32-bit times as seconds since 1/1/2000
  long secondstime() const;
  // 32-bit times as seconds since 1/1/1970
  // THE ABOVE COMMENT IS CORRECT FOR LOCAL TIME; TO USE THIS COMMAND TO
  // OBTAIN TRUE UNIX TIME SINCE EPOCH, YOU MUST CALL THIS COMMAND AFTER
  // SETTING YOUR CLOCK TO UTC
  uint32_t unixtime(void) const;

protected:
  uint8_t yOff, m, d, hh, mm, ss;
};

//判断年份是否是闰年
bool isleapYear(const uint8_t);

class RX8025
{
private:
  unsigned char RX8025_Control[2];
  /**
   * 获取寄存器数据
   * @return byte
   */
  byte getData(byte regaddr);

  /**
   * @brief 将十进制编码的二进制数转换为普通十进制数
   *
   * @param val
   * @return byte
   */
  byte decToBcd(byte val);

  /**
   * 将二进制编码的十进制数转换为普通十进制数
   * @param val
   * @return byte
   */
  byte bcdToDec(byte val);

public:
  RX8025(); // costruttore
  /**
   * 初始化
   */
  void RX8025_init(void);

  /**
   * 向时钟芯片设置时间
   * @param s  秒钟
   * @param m  分钟
   * @param h 时钟
   * @param d  天
   * @param mh 月
   * @param y 年
   */
  void setRtcTime(uint8_t s, uint8_t m, uint8_t h, uint8_t d, uint8_t mh, uint8_t y);

  /**
   * 获取秒钟
   */
  byte getSecond();
  /**
   * 获取分钟数
   * @return byte
   */
  byte getMinute();
  /**
   * 获取小时数
   * @return byte
   */
  byte getHour();
  /**
   * 获取星期数
   * @return byte
   */
  byte getDoW();
  /**
   * 获取日期
   * @return byte
   */
  byte getDate();
  /**
   * 获取月份
   * @return byte
   */
  byte getMonth();
  /**
   * 获取年份
   * @return byte
   */
  byte getYear();

  /**
   * 获取时间戳
   * @return long
   */
  long getUnixtime();
};

#endif