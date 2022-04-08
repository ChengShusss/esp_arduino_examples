#ifndef _MAIN_
#define _MAIN_

// Size of sprite image for the scrolling text, this requires ~14 Kbytes of RAM
#define IWIDTH 128
#define IHEIGHT 30

#define Addr_SHT30 0x44

#define SDA_PIN 4 //引脚接法在这里
#define SCL_PIN 5

// Pause in milliseconds to set scroll speed
#define WAIT 0

#include <TFT_eSPI.h> 

extern TFT_eSPI tft; 

extern TFT_eSprite img; 


// ============================================
// funcs to plot items.
unsigned int rainbow(byte value);

void build_banner(String msg, int xpos);

void plot_temp_humidity(int t, int h, int x, int y);

void plot_time(uint8_t dt[], int x, int y);

void plot_wifi(int x, int y);

// ============================================
// funcs to get infomation.
bool get_time(uint8_t dt[]);

void start_temp_measure(void);
bool get_temp_humidity(float *cTemp, float *humidity);

String http_GET_Request(const char* serverName);


#endif