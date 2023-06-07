// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.3.4
// PROJECT: HoloTimer

#ifndef _HOLOTIMER_UI_H
#define _HOLOTIMER_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t * ui_Screen;
extern lv_obj_t * ui_TH1;
extern lv_obj_t * ui_TH2;
extern lv_obj_t * ui_M1;
extern lv_obj_t * ui_Labelline;
extern lv_obj_t * ui_LabelMouth;
extern lv_obj_t * ui_LabelDate;

struct TimeStr
{
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;
};


extern void ui_change_sec(int);
extern void ui_change_min(int mins);
extern void ui_change_h(int h);
extern void ui_change_day(int day, int week);
extern void ui_chagne_mouth(int mouth);

extern void ui_init(void);
extern void ui_release();
extern void display_space();
extern void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
