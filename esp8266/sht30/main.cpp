/*
  Display "flicker free" scrolling text and updating number

  Example for library:
  https://github.com/Bodmer/TFT_eSPI

  The sketch has been tested on a 320x240 ILI9341 based TFT, it
  could be adapted for other screen sizes.

  A Sprite is notionally an invisible graphics screen that is
  kept in the processors RAM. Graphics can be drawn into the
  Sprite just as it can be drawn directly to the screen. Once
  the Sprite is completed it can be plotted onto the screen in
  any position. If there is sufficient RAM then the Sprite can
  be the same size as the screen and used as a frame buffer.

  The Sprite occupies (2 * width * height) bytes.

  On a ESP8266 Sprite sizes up to 128 x 160 can be accommodated,
  this size requires 128*160*2 bytes (40kBytes) of RAM, this must be
  available or the processor will crash. You need to make the sprite
  small enough to fit, with RAM spare for any "local variables" that
  may be needed by your sketch and libraries.

  Created by Bodmer 15/11/17

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
*/

#include <ESP8266WiFi.h>
#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Wire.h>
#include <ESP8266HTTPClient.h>

#include "main.h"

const char *ssid = "Shadow-WLAN";
const char *password = "shadow2035";
#define TIME_HOST "http://www.beijing-time.org/t/time.asp"
unsigned long last_update_time_ms = 1 << 31;
unsigned long last_update_second_ms = 1 << 31;
float cTemp, humidity;

uint8_t datetime[] = {0, 0, 0, 0, 0, 0, 0};

TFT_eSPI tft = TFT_eSPI(); // Create object "tft"

TFT_eSprite img = TFT_eSprite(&tft); // Create Sprite object "img" with pointer to "tft" object
//                                    // the pointer is used by pushSprite() to push it onto the TFT

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup(void)
{
  // tft setting
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLUE);

  // Serial, iic init
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);

  // try to connect to wifi.
  WiFi.begin(ssid, password);
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
void loop()
{
  if (millis() < last_update_second_ms || millis() - 1000 > last_update_second_ms)
  {
    start_temp_measure();
    last_update_second_ms = millis();
    plot_time(datetime, 14, 24);
    plot_wifi(14, 0);

    // get and print temp & humidity data.
    get_temp_humidity(&cTemp, &humidity);
    plot_temp_humidity(cTemp, humidity, 14, 96);
  }

  // update time every 30s
  if (WiFi.status() == WL_CONNECTED)
  {
    if (millis() < last_update_time_ms || millis() - 30000 > last_update_time_ms)
    {
      get_time(datetime);
      last_update_time_ms = millis();
    }
  }
}

// #########################################################################
// Build the scrolling sprite image from scratch, draw text at x = xpos
// #########################################################################
void build_banner(String msg, int xpos)
{
  int h = IHEIGHT;

  // We could just use fillSprite(color) but lets be a bit more creative...

  // Fill with rainbow stripes
  while (h--)
    img.drawFastHLine(0, h, IWIDTH, rainbow(h * 4));

  // Draw some graphics, the text will apear to scroll over these
  img.fillRect(IWIDTH / 2 - 20, IHEIGHT / 2 - 10, 40, 20, TFT_YELLOW);
  img.fillCircle(IWIDTH / 2, IHEIGHT / 2, 10, TFT_ORANGE);

  // Now print text on top of the graphics
  img.setTextSize(1);          // Font size scaling is x1
  img.setTextFont(4);          // Font 4 selected
  img.setTextColor(TFT_BLACK); // Black text, no background colour
  img.setTextWrap(false);      // Turn of wrap so we can print past end of sprite

  // Need to print twice so text appears to wrap around at left and right edges
  img.setCursor(xpos, 2); // Print text at xpos
  img.print(msg);

  img.setCursor(xpos - IWIDTH, 2); // Print text at xpos - sprite width
  img.print(msg);
}

// #########################################################################
// plot temp & humidity at (x, y), would occupy an area of 100 * 30
// #########################################################################
void plot_temp_humidity(int temp, int humidity, int x, int y)
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
  img.createSprite(100, 30);

  // Fill it with black
  img.fillSprite(TFT_BLACK);

  // Set the font parameters
  img.setTextSize(1);
  img.setTextFont(2);
  img.setTextColor(TFT_WHITE);

  img.print(" H: ");
  img.print(humidity);
  img.println(" %");

  img.print(" T: ");
  img.print(temp);
  img.print(" 'C");

  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  img.pushSprite(x, y);

  // Delete sprite to free up the RAM
  img.deleteSprite();
}

// #########################################################################
// plot temp & humidity at (x, y), would occupy an area of 100 * 30
// #########################################################################
void plot_time(uint8_t dt[], int x, int y)
{

  img.createSprite(100, 50);
  img.fillSprite(TFT_DARKCYAN);
  img.setCursor(2, 2);
  img.setTextFont(6);
  img.setTextColor(TFT_ORANGE);

  if (dt[4] < 10)
  {
    img.print('0');
  }
  img.print(dt[4]);
  // img.print(':');
  img.setTextFont(4);
  img.setTextColor(TFT_WHITE);
  if (dt[5] < 10)
  {
    img.print('0');
  }
  img.print(dt[5]);

  img.pushSprite(x, y);
  img.deleteSprite();
}

// #########################################################################
// plot wifi status, would occupy an area of 100 * 20
// #########################################################################
void plot_wifi(int x, int y)
{
  img.createSprite(100, 12);
  img.fillSprite(TFT_DARKGREY);
  img.setCursor(2, 2);
  img.setTextFont(1);

  if (WiFi.status() == WL_CONNECTED)
  {
    img.setTextColor(TFT_BLUE);
    img.print("Online");
  }
  else
  {
    img.setTextColor(TFT_RED);
    img.print("Offline");
  }
  img.pushSprite(x, y);
  img.deleteSprite();
}

// #########################################################################
// Get time from NTS(network time server), by TIME_HOST
// #########################################################################
bool get_time(uint8_t dt[])
{

  uint8_t i = 0;
  uint8_t p = 0;
  // Get time string, check if it is valid
  String payload = http_GET_Request(TIME_HOST);
  if (payload.indexOf("nyear") == -1)
  {
    return false;
  }

  // Split every time segment into dt, by y-m-d-h-m-s
  i = 10;
  while (p <= 6)
  {
    while (payload[i] < '0' || payload[i] > '9')
      i++;
    while (payload[i] >= '0' && payload[i] <= '9')
    {
      dt[p] = (dt[p] * 10 + payload[i] - '0') % 100;
      i++;
    }
    p++;
  }

  return true;
}

// #########################################################################
// Send start command to sht30.
// #########################################################################
void start_temp_measure(void)
{
  // start IIC communication
  Wire.beginTransmission(Addr_SHT30);
  // sent start command 0x2C06
  Wire.write(0x2C);
  Wire.write(0x06);
  // stop IIC
  Wire.endTransmission();
  // Recommand to wait several ms.
}

// #########################################################################
// read temp and humidity, must be called after start measure.
// #########################################################################
bool get_temp_humidity(float *cTemp, float *humidity)
{
  unsigned int data[6] = {0, 0, 0, 0, 0, 0}; //存储获取到的六个数据
  //请求获取6字节的数据，然后会存到8266的内存里
  Wire.requestFrom(Addr_SHT30, 6);

  //读取6字节的数据
  //这六个字节分别为：温度8位高数据，温度8位低数据，温度8位CRC校验数据
  //               湿度8位高数据，湿度8位低数据，湿度8位CRC校验数据
  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }

  //然后计算得到的数据，要转化为摄氏度、华氏度、相对湿度
  *cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
  // float fTemp = (cTemp * 1.8) + 32;
  *humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

  //在串口里输出得到的数据
  Serial.print("相对湿度：");
  Serial.print(*humidity);
  Serial.println(" %RH");
  Serial.print("摄氏度温度：");
  Serial.print(*cTemp);
  Serial.println(" C");

  return true;
}

// #########################################################################
// Get content of a page from server by http requests
// #########################################################################
String http_GET_Request(const char *serverName)
{
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0)
  {
    payload = http.getString();
  }
  else
  {
    // Serial.print("Error code: ");
    // Serial.println(httpResponseCode);
    payload = "";
  }
  // Free resources
  http.end();

  return payload;
}