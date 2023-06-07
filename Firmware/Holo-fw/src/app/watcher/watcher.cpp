#include "watcher.h"
#include "ui.h"
#include "ESP32Time.h"
#include "network.h"
#include "common.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

#include <HTTPClient.h>

#include "Time.h"
#include "TimeLib.h"
#include "WiFiUdp.h"

#include <esp32-hal-timer.h>
#include <map>

void watch_init();
void watch_release();
void set_focus_update(int v);
void update_time_status();

WiFiUDP ntpUDP;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
int daylightOffset_sec = 0;
const char* timezoneServiceURL = "http://worldtimeapi.org/api/ip"; // URL for IP-based timezone service

 
struct DateTime {
  int sec;
  int min;
  int hour;
  int wday;
  int mday;
  int mon;
  int year;
};


#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp"

struct WT_Config
{
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
};

struct WeatherAppRunData
{
    
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;
    unsigned int update_type; // 更新类型的标志位

    BaseType_t xReturned_task_task_update; // 更新数据的异步任务
    TaskHandle_t xHandle_task_task_update; // 更新数据的异步任务

    ESP32Time g_rtc; // 用于时间解码
    
};

static WT_Config cfg_data;
static WeatherAppRunData *run_data = NULL;

enum wea_event_Id
{
    UPDATE_NOW,
    UPDATE_NTP,
    UPDATE_DAILY
};

static long long getWorldTime()
{
    HTTPClient http;
  int request_count = 0;
  String url = timezoneServiceURL;
  while(request_count<5)
  {
      http.begin(url);
      int httpResponseCode = http.GET();
      if (httpResponseCode == HTTP_CODE_OK) 
      {
        String payload = http.getString();
        // Parse JSON response
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        String utc_str = doc["utc_offset"].as<String>();
//        String utc_str = "+08:00";
        int hours, minutes;
        sscanf(utc_str.c_str(), "%d:%d", &hours, &minutes);
        daylightOffset_sec = hours * 3600 + minutes * 60;

        String utc_time_str = doc["unixtime"].as<String>();
        // run_data->preNetTimestamp = (atoll(utc_time_str.c_str()) + daylightOffset_sec)*1000;
        run_data->preNetTimestamp = (atoll(utc_time_str.c_str()))*1000;

        run_data->preLocalTimestamp = millis();


//        String utc_str = "+08:00";
        // int hours, minutes;
        // sscanf(utc_str.c_str(), "%d:%d", &hours, &minutes);
        // daylightOffset_sec = hours * 3600 + minutes * 60;
        // Serial.print("time area:");
        // Serial.println(*timezone);
        Serial.print("time offset:");
        Serial.println(daylightOffset_sec);     



        break;
      }
      else 
      {
        run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
        run_data->preLocalTimestamp = millis();
        Serial.print("Error getting wold wide time. HTTP error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      request_count++;
  }
  return run_data->preNetTimestamp;

}
void getTimezone(String* timezone) 
{
  HTTPClient http;
  int request_count = 0;
  String url = timezoneServiceURL;
  while(request_count<5)
  {
      http.begin(url);
      int httpResponseCode = http.GET();
      if (httpResponseCode == HTTP_CODE_OK) 
      {
        String payload = http.getString();
        // Parse JSON response
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        *timezone = doc["timezone"].as<String>();
        String utc_str = doc["utc_offset"].as<String>();
//        String utc_str = "+08:00";
        int hours, minutes;
        sscanf(utc_str.c_str(), "%d:%d", &hours, &minutes);
        daylightOffset_sec = hours * 3600 + minutes * 60;
        Serial.print("time area:");
        Serial.println(*timezone);
        Serial.print("time offset:");
        Serial.println(daylightOffset_sec);     
        break;
      }
      else 
      {
        Serial.print("Error getting timezone. HTTP error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      request_count++;
  }

}


static void task_update(void *parameter); // 异步更新任务

static long long get_timestamp()
{
    // 使用本地的机器时钟
    run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
    run_data->preLocalTimestamp = millis();
    return run_data->preNetTimestamp;
}

static long long get_timestamp(String url)
{
    if (WL_CONNECTED == WiFi.status())
    {
        String time = "";
        HTTPClient http;
        http.setTimeout(1000);
        http.begin(url);

        int httpCode = http.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                Serial.println(payload);
                int time_index = (payload.indexOf("data")) + 12;
                time = payload.substring(time_index, payload.length() - 3);
                // 以网络时间戳为准
                run_data->preNetTimestamp = atoll(time.c_str()) + run_data->errorNetTimestamp + TIMEZERO_OFFSIZE;
                run_data->preLocalTimestamp = millis();
            }
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            // 得不到网络时间戳时
            run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
            run_data->preLocalTimestamp = millis();
        }
        http.end();
        
    }
    else{
            run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
            run_data->preLocalTimestamp = millis();
    }
    return run_data->preNetTimestamp;
}


static void UpdateTime_RTC(long long timestamp)
{
    struct TimeStr t;
    run_data->g_rtc.setTime(timestamp / 1000);
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour(true);
    t.minute = run_data->g_rtc.getMinute();
    t.second = run_data->g_rtc.getSecond();
    t.weekday = run_data->g_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    display_time(t, LV_SCR_LOAD_ANIM_NONE);
}

//use the forigner timer server
static void UpdateTime_RTC_EN()
{
    // time_t now = time(nullptr);
    // tm* timeinfo = localtime(&now);

    // struct TimeStr t;
    
    // t.month = timeinfo->tm_mon;
    // t.day = timeinfo->tm_mday;
    // t.second= timeinfo->tm_sec;
    // t.minute = timeinfo->tm_min;
    // t.hour = timeinfo->tm_hour;
    // t.weekday = timeinfo->tm_wday;
    // display_time(t, LV_SCR_LOAD_ANIM_NONE);
    // if(timeinfo->tm_year>=2023)
    // {
    //     run_data->g_rtc.setTime(t.second, t.minute, t.hour, t.day, t.month, timeinfo->tm_year, 0);
    //     run_data->preNetTimestamp = run_data->g_rtc.getMillis();
    //     run_data->preLocalTimestamp = millis();
    // }

   UpdateTime_RTC(getWorldTime());


}

//初始化的时候通过网络获取时间
//每间隔15分钟，去同步一次网络时间
void watch_init()
{
    ui_init();
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    run_data->preNetTimestamp = 1679317985522; // 上一次的网络时间戳 初始化为2023-03-20 00:00:00
    run_data->errorNetTimestamp = 2;
    run_data->preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    run_data->clock_page = 0;
    run_data->preTimeMillis = 0;
    // 强制更新时间
    run_data->coactusUpdateFlag = 0x01;
    run_data->update_type = 0x00; 


}

void set_focus_update(int v)
{
    run_data->coactusUpdateFlag = v;
    #ifdef EN_TIME_ZONE
    if (WL_CONNECTED == WiFi.status())
    {
        String timezone;
        getTimezone(&timezone);
        // Set timezone
        setenv("TZ", timezone.c_str(), 1);
        tzset();
        
        // Get time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        while (!time(nullptr)) 
        {
            delay(1000);
        }
        Serial.println("Time is synchronized.");
    }

    #endif
}

void watch_release()
{
    ui_release();
    // free(run_data);

}

void update_time_status()
{
    //  if (run_data->clock_page == 0)
    {
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(900000, &run_data->preTimeMillis, false))
        {
            // 尝试同步网络上的时钟
            #ifdef ZH_TIME_ZONE
                UpdateTime_RTC(get_timestamp(TIME_API));
            #endif

            #ifdef EN_TIME_ZONE
                UpdateTime_RTC_EN();
            #endif
           
        }
        else if (millis() - run_data->preLocalTimestamp > 800)
        {
            UpdateTime_RTC(get_timestamp());
        }
        run_data->coactusUpdateFlag = 0x00; // 取消强制更新标志
        display_space();
        // Serial.println("updating...");
        delay(100);
    }
}




