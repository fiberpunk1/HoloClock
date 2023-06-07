#include "common.h"
#include "network.h"
#include <EEPROM.h>

IMU mpu;
SdCard tf;
Pixel rgb;
// Config g_cfg;       // 全局配置文件
Network g_network;  // 网络连接
FlashFS g_flashCfg; // flash中的文件系统（替代原先的Preferences）
Display screen;     // 屏幕对象
Ambient ambLight;   // 光线传感器对象
int wifi_current_mode = 1; //wifi的模式，等于0说明进入了配网模式


void writeEEPROMString(int a,int b,String str)
{
    EEPROM.write(a, str.length());
    
    for (int i = 0; i < str.length(); i++)
    {
        EEPROM.write(b + i, str[i]);
    }
    EEPROM.commit();
}
String readEEPROMString(int a, int b)
{ 
    String data = "";
    for (int i = 0; i < a; i++)
    {
        data += char(EEPROM.read(b + i));
    }
    return data;
}

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}

#if GFX

#include <Arduino_GFX_Library.h>

Arduino_HWSPI *bus = new Arduino_HWSPI(TFT_DC /* DC */, TFT_CS /* CS */, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_ST7789 *tft = new Arduino_ST7789(bus, TFT_RST /* RST */, 3 /* rotation */, true /* IPS */,
                                         240 /* width */, 240 /* height */,
                                         0 /* col offset 1 */, 80 /* row offset 1 */);

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
TFT_eSPI *tft = new TFT_eSPI(SCREEN_HOR_RES, SCREEN_VER_RES);
#endif