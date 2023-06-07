#include "picture.h"
#include "picture_gui.h"
#include "common.h"
#include "../watcher/watcher.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>
#include "docoder.h"
#include "DMADrawer.h"


// 相册的持久化配置

#define MEDIA_CONFIG_PATH "/media.cfg"



#define MEDIA_PLAYER_APP_NAME "Media"

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOVIE_PATH "/movie"
#define NO_TRIGGER_ENTER_FREQ_160M 90000UL // 无操作规定时间后进入设置160M主频（90s）
#define NO_TRIGGER_ENTER_FREQ_80M 120000UL // 无操作规定时间后进入设置160M主频（120s）


ACTIVE_TYPE pre_statu;
uint8_t pre_play_type;//记录上一次播放的是图片还是视屏,0 蝴蝶壁纸, 1 时钟

void picture_init();
void picture_process(const ImuAction *act_info);


struct MP_Config
{
    uint8_t switchFlag; // 是否自动播放下一个（0不切换 1自动切换）
    uint8_t powerFlag;  // 功耗控制（0低发热 1性能优先）
};

struct MediaAppRunData
{
    PlayDocoderBase *player_docoder;
    unsigned long preTriggerKeyMillis; // 最近一回按键触发的时间戳
    int movie_pos_increate;
    File_Info *movie_file; // movie文件夹下的文件指针头
    File_Info *pfile;      // 指向当前播放的文件节点
    File file;
};

static MP_Config video_cfg_data;
static MediaAppRunData *video_run_data = NULL;
static std::vector<String> mp4_file_list;
static int current_video_index = 0;

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft->height())
        return 0;

    // This function will clip the image block rendering automatically at the TFT boundaries
    tft->pushImage(x, y, w, h, bitmap);

    // This might work instead if you adapt the sketch to use the Adafruit_GFX library
    // tft.drawRGBBitmap(x, y, bitmap, w, h);

    // Return 1 to decode next block
    return 1;
}

File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    // 得到 p_cur_file 的下一个 类型为FILE_TYPE_FILE 的文件（即下一个非文件夹文件）
    if (NULL == p_cur_file)
    {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file)
    {
        if (FILE_TYPE_FILE == pfile->file_type)
        {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    return pfile;
}

static void release_player_docoder(void)
{
    // 释放具体的播放对象
    if (NULL != video_run_data->player_docoder)
    {
        delete video_run_data->player_docoder;
        video_run_data->player_docoder = NULL;
    }
}

void video_run_init()
{
    video_run_data = (MediaAppRunData *)calloc(1, sizeof(MediaAppRunData));
    video_run_data->player_docoder = NULL;
    video_run_data->movie_pos_increate = 1;
    video_run_data->movie_file = NULL; // movie文件夹下的文件指针头
    video_run_data->pfile = NULL;      // 指向当前播放的文件节点
    video_run_data->preTriggerKeyMillis = millis();
}

//初始化一个文件解码器
static bool video_start(bool create_new, String filename)
{
    video_run_init();
    video_run_data->file = SPIFFS.open(filename);
    // 直接解码mjpeg格式的视频
    Serial.print(F("before release the player decoder...")); 
    video_run_data->player_docoder = new MjpegPlayDocoder(&video_run_data->file, true);
    Serial.print(F("MJPEG video start --------> "));  
    Serial.println(filename);
    return true;
}


//获取所有的目录信息，每个目录对应一个打印文件
void update_all_video()
{
    mp4_file_list.clear();
    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while(file)
    {

        String file_name = String(file.name());
        Serial.println(file_name);
        if(file_name.endsWith(".mjpeg"))
        {
            mp4_file_list.push_back(file_name);
        }
        

        file = root.openNextFile();
        delay(10);
    }
}

void picture_init()
{
    // photo_gui_init();
    watch_init();
    video_run_init();

    pre_play_type = 1;  //默认设置为时钟
    set_focus_update(1);

    tft->setSwapBytes(true); // We need to swap the colour bytes (endianess)

    update_all_video();
    TJpgDec.setJpgScale(1);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);

}


void video_check_start()
{
    if(mp4_file_list.size()>0)
    {
        String p_current_file = mp4_file_list[current_video_index];
        if(p_current_file.endsWith(".mjpeg") || p_current_file.endsWith(".MJPEG"))
        {
            Serial.println("Here in video check start...");
            Serial.println(p_current_file);
            release_player_docoder();
            delay(30);
            if(video_run_data->file.available())
            {
                Serial.println("Now let's close the file");
                video_run_data->file.close(); 
            }
            Serial.println("Here in video close file");
            video_start(true, p_current_file);
        }
    }
   
}


void picture_process(const ImuAction *act_info)
{
    //往右切换为时钟状态
    if (TURN_RIGHT == act_info->active)
    {
        //如果之前已经是时钟，那就不需要做什么
        if(act_info->active==pre_statu)
        {
            
        }
        else
        {
            //先释放蝴蝶播放的资源
            if(mp4_file_list.size()>0)
            {
                release_player_docoder();
                if(video_run_data->file != NULL)
                    video_run_data->file.close(); 
                tft->fillScreen(TFT_BLACK);
                TJpgDec.setJpgScale(1);
                TJpgDec.setCallback(tft_output);
            }
            Serial.print("Before watch init");
            //再进行时钟初始化
            watch_init();
            Serial.print("After watch init");
            
        }
        
        pre_play_type = 1;
        
    }
    //往左切换为蝴蝶状态，如果已经是蝴蝶状态，就切换蝴蝶的样式
    else if (TURN_LEFT == act_info->active)
    {
        if(act_info->active==pre_statu)
        {
            if(mp4_file_list.size()>0)
            {
                //如果已经是蝴蝶显示状态，那么就需要切换视屏
                current_video_index += 1;
                current_video_index = (current_video_index % mp4_file_list.size());
                Serial.print("current video index:");
                Serial.println(current_video_index);
            }

            
        }
        else
        {
            if(mp4_file_list.size()>0)
            {
                //如果不是蝴蝶状态，那么需要先释放时钟资源，再初始化蝴蝶的资源
                watch_release();
                current_video_index = 0;
            }

        }
        //这里提前释放播放视屏的资源
        video_check_start();
        pre_play_type = 0;
        
    }

    if(pre_play_type)
    {
        //设置时钟相关的动作
        update_time_status();
    }
    else
    {
        if(mp4_file_list.size()>0)
        {
            String p_current_file = mp4_file_list[current_video_index];
            if(p_current_file.endsWith(".mjpeg") || p_current_file.endsWith(".MJPEG"))
            {
                //在这里播放视屏
                if (video_run_data->file.available())
                {
                    // 播放一帧数据
                    video_run_data->player_docoder->video_play_screen();
                }
                else
                {
                    release_player_docoder();
                    video_run_data->file.close();
                    video_start(true, p_current_file);
                }
                
            }
            else
            {
                pre_play_type = 1;
            }
        }

    }
    
    pre_statu = act_info->active; 
    
}

void picture_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

int picture_exit_callback(void *param)
{
    photo_gui_del();
    return 0;
}
