#include <SPIFFS.h>
#include <esp32-hal.h>
#include <esp32-hal-timer.h>


#include <WiFi.h>      
#include <WiFiMulti.h> 
#include <WebServer.h> 
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <EEPROM.h>
WebServer fiber_server(81);
WebServer config_wifi_server(80);

#include "driver/lv_port_indev.h"
#include "driver/lv_port_fs.h"

#include "common.h"

#include "app/picture/picture.h"
#include "app/watcher/watcher.h"
#include "wificonfig.h"

extern String wifi_config_string;

SysUtilConfig sys_cfg;
SysMpuConfig mpu_cfg;
RgbConfig g_rgb_cfg;

static bool isCheckAction = false;
ImuAction *act_info; // 存放mpu6050返回的数据
File uploadFile;
bool isSDCardInserted = false;

void wifi_init();

TimerHandle_t xTimerAction = NULL;
void actionCheckHandle(TimerHandle_t xTimer)
{
    // 标志需要检测动作
    isCheckAction = true;
}
void returnOK() 
{
  fiber_server.send(200, "text/plain", "");
}

void returnFail(String msg) 
{
  fiber_server.send(500, "text/plain", msg + "\r\n");
}
String readConfig(File& file)
{
  String ret="";
  if(file.available())
  {
    ret = file.readStringUntil('\n');
    ret.replace("\r", "");
  }
  
  return ret;
}
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;


    for (int i = 0; i <= maxIndex && found <= index; i++) 
    {
        if (data.charAt(i) == separator || i == maxIndex) 
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void wifi_check()
{
  //1.check SD card
  #if 1
  if(isSDCardInserted)
  {
    File configFile = SD.open("/config.txt", FILE_READ);
    bool t_check = false;
    if (!configFile) 
    {
      Serial.println("Failed to open config file");
      t_check = true;
      // return;
    }
    while (configFile.available()) 
    {
      if(t_check)
        break;
      String line = configFile.readStringUntil('\n');
      line.trim(); 
      int separatorIndex = line.indexOf(':');
      if (separatorIndex != -1) 
      {
        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);
        if (key == "ssid") 
        {
          Serial.print("Connecting to ");
          Serial.println(value);
          writeEEPROMString(1,30,value);
          delay(100);


        } 
        else if (key == "pass_word") 
        {
          Serial.print("Connecting to ");
          writeEEPROMString(5,60,value);
          delay(100);
        }
      }
    }
    configFile.close();
  }
  else
  {
    Serial.println("SD card not found!");
  }

  #endif

  //2.if no sd card, then just read wifi from the eeprom
  wifi_init();
}

void wifi_init()
{
    // File config_file;
    // config_file = SD.open("/config.txt",FILE_READ);
    // String wifi_name = "Fiberpunk";
    // String wifi_psd = "happybird";

    String wifi_name = "";
    String wifi_psd = "";

    wifi_name = readEEPROMString(EEPROM.read(1),30);
    Serial.print("read name from eeprom:");
    Serial.println(wifi_name);
    delay(100);

    wifi_psd = readEEPROMString(EEPROM.read(5),60);
    Serial.print("read psd from eeprom:");
    Serial.println(wifi_psd);
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);
    delay(100);
    WiFi.setTxPower(WIFI_POWER_5dBm);
    WiFi.begin(wifi_name.c_str(), wifi_psd.c_str());
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) 
    {
        rgb.setBrightness(0).setRGB(0, 64, 64);
        //totaly wait 10 seconds
        delay(500);
        Serial.print("connect...");
        rgb.setBrightness(0.2).setRGB(128, 0, 0);
    }
    if (i == 21) 
    {
        Serial.print("connect failed!");
        rgb.setBrightness(0.1).setRGB(128, 0, 0);
        set_focus_update(1);
        wifi_current_mode = 1;
        // wifi_current_mode = 0;

    }
    else{
        rgb.setBrightness(0.1).setRGB(0, 150, 0);
        Serial.print("connect successful!");
        String ip = "Beam-Holo:" + WiFi.localIP().toString();
        Serial.print(ip);
        set_focus_update(1);
        wifi_current_mode = 1;
    }

}

void fbhandleFileUpload() 
{
  if (fiber_server.uri() != "/edit") 
  {
    return;
  }
  HTTPUpload& upload = fiber_server.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    if (SPIFFS.exists((char *)upload.filename.c_str())) 
    {
      SPIFFS.remove((char *)upload.filename.c_str());
    }
    uploadFile = SPIFFS.open(upload.filename.c_str(), FILE_WRITE);
    // DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    if (uploadFile) 
    {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    // DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) 
  {
    if (uploadFile) 
    {
      uploadFile.close();
    }
    // DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void deleteRecursive(String path) 
{
  File file = SPIFFS.open((char *)path.c_str());
  if (!file.isDirectory()) 
  {
    file.close();
    SPIFFS.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while (true) 
  {
    File entry = file.openNextFile();
    if (!entry) 
    {
      break;
    }
    String entryPath = entry.name();
    if (entry.isDirectory()) 
    {
      entry.close();
      deleteRecursive(entryPath);
    } else 
    {
      entry.close();
      SPIFFS.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SPIFFS.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete() 
{
  if (fiber_server.args() == 0) 
  {
    return returnFail("BAD ARGS");
  }
  // String path = server.arg(0);
  String path = fiber_server.arg("path");
  // DBG_OUTPUT_PORT.print(path);
  if (path == "/" || !SPIFFS.exists((char *)path.c_str())) 
  {
    returnFail("No SPIFFS Card");
  }
  deleteRecursive(path);
  returnOK();
}
void printDirectory() 
{
  if (!fiber_server.hasArg("dir")) 
  {
    return returnFail("BAD ARGS");
  }
  String path = fiber_server.arg("dir");
  if (path != "/" && !SPIFFS.exists((char *)path.c_str())) 
  {
    return returnFail("No SPIFFS Card!");
  }
  File dir = SPIFFS.open((char *)path.c_str());
  path = String();
  if (!dir.isDirectory()) 
  {
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  fiber_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  fiber_server.send(200, "text/json", "");
  WiFiClient client = fiber_server.client();

  fiber_server.sendContent("[");
  for(int cnt = 0; true; ++cnt)
   {
    File entry = dir.openNextFile();
    if(!entry) 
    {
      break;
    }

    String output;
    if(cnt > 0)
    {
      output = ',';
    }

    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += entry.name();
    output += "\"";
    output += "}";
    fiber_server.sendContent(output);
    entry.close();
  }
  fiber_server.sendContent("]");
  dir.close();
}
void handleCreate() 
{
  if (!fiber_server.hasArg("dirname")) 
  {
        return returnFail("No file");
  }
  String path = fiber_server.arg("dirname");
  if (path == "/" || SPIFFS.exists((char *)path.c_str())) 
  {
    returnFail("Dir existed");
  }
  SPIFFS.mkdir((char *)path.c_str());
  picture_init();
  returnOK();
}
void updateStatus()
{   
  fiber_server.send(200, "text/plain", "ok");

}

void reportDevice()
{
  String ip = "Fiberpunk:" + WiFi.localIP().toString();
  Serial.print(ip);
  fiber_server.send(200, "text/plain",ip);
}

void handleNotFound()
{
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += config_wifi_server.uri();
  message += "\nMethod: ";
  message += (config_wifi_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += config_wifi_server.args();
  message += "\n";
  for (uint8_t i = 0; i < config_wifi_server.args(); i++) {
    message += " NAME:" + config_wifi_server.argName(i) + "\n VALUE:" + config_wifi_server.arg(i) + "\n";
  }
  config_wifi_server.send(404, "text/plain", message);
}

void handleWiFiList() 
{
  String wifiList = "[";

  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++) {
    if (i > 0) {
      wifiList += ",";
    }
    wifiList += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + "}";
    Serial.print("Wifi ssid:");
    Serial.println(WiFi.SSID(i));
  }

  wifiList += "]";
  config_wifi_server.send(200, "application/json", wifiList);
}
void handleRoot() 
{
  Serial.println("in the config index page");
  config_wifi_server.send(200, "text/html", wifi_config_string);
}

void handleConfig() {
  if (config_wifi_server.method() == HTTP_POST) {
    String ssid = config_wifi_server.arg("ssid");
    String password = config_wifi_server.arg("password");
    WiFi.softAPdisconnect(true); // 断开AP模式
    WiFi.mode(WIFI_MODE_STA); // 切换到STA模式
    WiFi.begin(ssid.c_str(), password.c_str());
    int i=0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) 
    {
        rgb.setBrightness(0).setRGB(0, 64, 64);
        //totaly wait 10 seconds
        delay(500);
        Serial.print("connect...");
        rgb.setBrightness(0.2).setRGB(128, 0, 0);
    }
    if (i == 21) 
    {
        Serial.print("connect failed!");
        rgb.setBrightness(0.1).setRGB(128, 0, 0);
        wifi_current_mode = 0;

    }
    else{
        rgb.setBrightness(0.1).setRGB(0, 150, 0);
        //保存密码到eeprom
        writeEEPROMString(1,30,ssid);
        delay(100);
        writeEEPROMString(5,60,password);
        delay(100);
        Serial.print("connect successful!");
        wifi_current_mode = 1;
        String ip = "Beam-Holo:" + WiFi.localIP().toString();
        Serial.print(ip);
        set_focus_update(1);
    }
    config_wifi_server.send(200, "text/html", "<html><body><h1>WiFi Configuration</h1><p>Connecting to WiFi network...</p></body></html>");

    
  } else {
    config_wifi_server.send(200, "text/html", "<html><body><form method=\"post\" action=\"/config\"><label for=\"ssid\">WiFi SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\" required><label for=\"password\">WiFi Password:</label><input type=\"password\" id=\"password\" name=\"password\" required><input type=\"submit\" value=\"Connect\"></form></body></html>");
  }
}

void wifi_config_mode_init()
{
  
  WiFi.disconnect();
  delay(100);
  WiFi.setTxPower(WIFI_POWER_5dBm);
  WiFi.mode(WIFI_MODE_AP); 
  WiFi.softAP("Fiberpunk_holo_ap", "fiberpunk");
  // IPAddress Ip(192, 168, 88, 88);    //setto IP Access Point same as gateway
  // IPAddress NMask(255, 255, 255, 0);
  // WiFi.softAPConfig(Ip, Ip, NMask);
  config_wifi_server.on("/", HTTP_GET,handleRoot);
  config_wifi_server.on("/wifi-list", HTTP_GET,handleWiFiList);
  config_wifi_server.on("/connect", handleConfig);
  config_wifi_server.onNotFound(handleNotFound);
  config_wifi_server.begin();

}

void setup()
{
    Serial.begin(115200);

    Serial.println(F("\nFiberpunk Holo \n"));
    Serial.flush();
    // MAC ID可用作芯片唯一标识
    Serial.print(F("ChipID(EfuseMac): "));
    Serial.println(ESP.getEfuseMac());
    EEPROM.begin(256);

    // 需要放在Setup里初始化
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    if(SD.begin())
    {
      isSDCardInserted = true;
    }

        /*** Init screen ***/
    screen.init(4,95);
    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.05).setRGB(0, 64, 64);

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();

    mpu.init(0, 1,&mpu_cfg);
        /*** 以此作为MPU6050初始化完成的标志 ***/
    RgbConfig *rgb_cfg = &g_rgb_cfg;
    // 初始化RGB灯 HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            rgb_cfg->min_value_0, rgb_cfg->min_value_1, rgb_cfg->min_value_2,
                            rgb_cfg->max_value_0, rgb_cfg->max_value_1, rgb_cfg->max_value_2,
                            rgb_cfg->step_0, rgb_cfg->step_1, rgb_cfg->step_2,
                            rgb_cfg->min_brightness, rgb_cfg->max_brightness,
                            rgb_cfg->brightness_step, rgb_cfg->time};

    // rgb_thread_init(&rgb_setting);

    act_info = mpu.getAction();
    // 定义一个mpu6050的动作检测定时器
    xTimerAction = xTimerCreate("Action Check",
                                200 / portTICK_PERIOD_MS,
                                pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);

    // lv_port_fs_init();
    lv_fs_fatfs_init();
    wifi_current_mode = 1;
    picture_init();
    wifi_init();
    
    Serial.println("init picture finish");

    fiber_server.on("/status", HTTP_GET, updateStatus);
    fiber_server.on("/find", HTTP_GET, reportDevice); 
    fiber_server.on("/list", HTTP_GET, printDirectory);
    fiber_server.on("/create", HTTP_GET, handleCreate);
    fiber_server.on("/delete", HTTP_GET, handleDelete);
    fiber_server.on("/edit", HTTP_POST, []() {
    returnOK();
  }, fbhandleFileUpload);

    fiber_server.begin();
}



void loop()
{
    if(wifi_current_mode)
    {
      fiber_server.handleClient();
    }
    else
    {
      config_wifi_server.handleClient();
    }
    screen.routine();
    if (isCheckAction)
    {
        isCheckAction = false;
        act_info = mpu.getAction();

        if(act_info->active==3)
        {
          delay(300);
          if(act_info->active==3)
          {
            //进入配网模式
            
             if(wifi_current_mode)
             {
                wifi_current_mode = 0;
                rgb.setBrightness(0.1).setRGB(0, 0, 150);
                Serial.println("Enter wifi config AP mode..");
                wifi_config_mode_init();
             }
          }
        }

        //前倾斜
        if(act_info->active==4)
        {
          delay(300);
          if(act_info->active==4)
          {
            //重新连接一次网络
            // wifi_init();
            if(WiFi.status() != WL_CONNECTED)
            {
              wifi_check();
            }
            
          }
        }

    }
    picture_process(act_info);
    act_info->active = ACTIVE_TYPE::UNKNOWN;
    act_info->isValid = 0;
}