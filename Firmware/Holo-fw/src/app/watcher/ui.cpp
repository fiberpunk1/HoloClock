// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.3.4
// PROJECT: HoloTimer

#include "ui.h"
#include "ui_helpers.h"
#include "Arduino.h"

#define THEME_1
#define SPACE_60


#include "space_image-60.h"
#include "ui_img_1_png.h"


///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_Screen;
lv_obj_t * ui_TH1;
lv_obj_t * ui_TH2;
lv_obj_t * ui_M1;
lv_obj_t * ui_Labelline;
lv_obj_t * ui_LabelMouth;
lv_obj_t * ui_LabelDate;

lv_obj_t *spaceImg = NULL;

const void *manImage_map[] = {&man_0, &man_1, &man_2, &man_3, &man_4, &man_5, &man_6, &man_7, &man_8, &man_9};
static const char weekDayCh[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char mouthCh[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep","Oct", "Nov", "Dec"};
// ///////////////////// TEST LVGL SETTINGS ////////////////////
// #if LV_COLOR_DEPTH != 16
//     #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
// #endif
// #if LV_COLOR_16_SWAP !=1
//     #error "LV_COLOR_16_SWAP should be 1 to match SquareLine Studio's settings"
// #endif

void ui_init(void);
void ui_release();
void ui_change_sec(int);
void ui_change_min(int mins);
void ui_change_h(int h);
void ui_change_day(int day, int week);
void ui_chagne_mouth(int mouth);
void display_space();
///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////
void ui_Screen_screen_init(void)
{
    ui_Screen = lv_obj_create(NULL);
    lv_obj_add_flag(ui_Screen, LV_OBJ_FLAG_SCROLL_ONE);     /// Flags
    lv_obj_clear_flag(ui_Screen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_Screen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_Screen, &ui_img_1_png, LV_PART_MAIN|LV_STATE_DEFAULT);

    ui_TH1 = lv_label_create(ui_Screen);
    lv_obj_set_width(ui_TH1, TH1_W);
    lv_obj_set_height(ui_TH1, TH1_H);
    lv_obj_set_x(ui_TH1, TH1_X);
    lv_obj_set_y(ui_TH1, TH1_Y);
    lv_obj_set_align(ui_TH1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_TH1, "18");
    lv_obj_add_flag(ui_TH1, LV_OBJ_FLAG_SCROLL_ONE);     /// Flags
    lv_obj_set_style_text_color(ui_TH1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_TH1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_TH1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_TH2 = lv_label_create(ui_Screen);
    lv_obj_set_width(ui_TH2, TH2_W);
    lv_obj_set_height(ui_TH2, TH2_H);
    lv_obj_set_x(ui_TH2, TH2_X);
    lv_obj_set_y(ui_TH2, TH2_Y);
    lv_obj_set_align(ui_TH2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_TH2, "59");
    lv_obj_add_flag(ui_TH2, LV_OBJ_FLAG_SCROLL_ONE);     /// Flags
    lv_obj_set_style_text_color(ui_TH2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_TH2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_TH2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);  


    ui_M1 = lv_label_create(ui_Screen);
    lv_obj_set_width(ui_M1, M1_W);
    lv_obj_set_height(ui_M1, M1_H);
    lv_obj_set_x(ui_M1, M1_X);
    lv_obj_set_y(ui_M1, M1_Y);
    lv_obj_set_align(ui_M1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_M1, "25");
    lv_obj_add_flag(ui_M1, LV_OBJ_FLAG_SCROLL_ONE);     /// Flags
    lv_obj_set_style_text_color(ui_M1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_M1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_M1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);


    ui_LabelMouth = lv_label_create(ui_Screen);
    lv_obj_set_width(ui_LabelMouth, MOUTH_W);
    lv_obj_set_height(ui_LabelMouth, MOUTH_H);
    lv_obj_set_x(ui_LabelMouth, MOUTH_X);
    lv_obj_set_y(ui_LabelMouth, MOUTH_Y);
    lv_obj_set_align(ui_LabelMouth, LV_ALIGN_CENTER);
    lv_label_set_text(ui_LabelMouth, "May");
    lv_obj_set_style_text_color(ui_LabelMouth, lv_color_hex(MOUTH_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LabelMouth, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_LabelMouth, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelMouth, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_LabelDate = lv_label_create(ui_Screen);
    lv_obj_set_width(ui_LabelDate, DATE_W);
    lv_obj_set_height(ui_LabelDate, DATE_H);
    lv_obj_set_x(ui_LabelDate, DATE_X);
    lv_obj_set_y(ui_LabelDate, DATE_Y);
    lv_obj_set_align(ui_LabelDate, LV_ALIGN_CENTER);  
    lv_label_set_text(ui_LabelDate, "20");
    lv_obj_set_style_text_color(ui_LabelDate, lv_color_hex(DATE_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LabelDate, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_LabelDate, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelDate, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    spaceImg = lv_img_create(ui_Screen);
    lv_img_set_src(spaceImg, manImage_map[0]);
    // lv_img_set_zoom(spaceImg, 256/2);

#ifdef THEME_1
    lv_obj_set_style_text_font(ui_TH1, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TH2, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_M1, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(spaceImg, LV_ALIGN_CENTER, 105,10);
#endif


}

void display_space(void)
{
    static int _spaceIndex = 1;
    if (NULL != ui_Screen && lv_scr_act() == ui_Screen)
    {
        lv_img_set_src(spaceImg, manImage_map[_spaceIndex]);
        #ifdef BUTTERFLY_60
        _spaceIndex = (_spaceIndex + 1) % 10;
        #endif
        
        #ifndef BUTTERFLY_60
        _spaceIndex++;
        if(_spaceIndex>=10)
             _spaceIndex = 1;
        #endif
    }
    delay(100);
}


void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type)
{
    ui_change_sec(timeInfo.second);
    ui_change_min(timeInfo.minute);
    ui_change_h(timeInfo.hour);
    ui_change_day(timeInfo.day, timeInfo.weekday);
    ui_chagne_mouth(timeInfo.month);
}

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Screen_screen_init();
    lv_disp_load_scr(ui_Screen);
}

void ui_release()
{
    if (NULL != ui_Screen)
    {
        lv_obj_clean(ui_Screen); 
        ui_Screen = NULL;
        ui_TH1 = NULL;
        ui_TH2 = NULL;
        ui_M1 = NULL;
        ui_Labelline = NULL;
        ui_LabelMouth = NULL;
        ui_LabelDate = NULL;
    }

}

void ui_change_sec(int sec)
{
    String sec_str = "";
    if(sec<10)
        sec_str = "0"+String(sec);
    else
        sec_str = String(sec);
    lv_label_set_text(ui_M1, sec_str.c_str());
}

void ui_change_min(int mins)
{
    String sec_str = "";
    if(mins<10)
        sec_str = "0"+String(mins);
    else
        sec_str = String(mins);
    lv_label_set_text(ui_TH2, sec_str.c_str());
}

void ui_change_h(int h)
{
    String sec_str = "";
    sec_str = String(h);
    lv_label_set_text(ui_TH1, sec_str.c_str());
}

void ui_change_day(int day, int week)
{
    lv_label_set_text_fmt(ui_LabelDate, "%d %s", day, weekDayCh[week]);
}

void ui_chagne_mouth(int mouth)
{
    lv_label_set_text(ui_LabelMouth, mouthCh[mouth-1]);
}


