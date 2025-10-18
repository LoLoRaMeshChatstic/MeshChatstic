#pragma once

// LVGL Configuration para ESP32-S3 sin PSRAM
// Heltec V3 + ILI9225 (176x220) + Device-UI
// Optimizado para RAM limitada (~248 KB disponible)

#define LV_USE_STDLIB_MALLOC    0
#define LV_MEM_CUSTOM           1
#define LV_MEM_SIZE             (32U * 1024U)

#define LV_COLOR_DEPTH          16
#define LV_COLOR_16_SWAP        0
#define LV_DPI_DEF              130
#define LV_IMG_CACHE_DEF_SIZE   0

#define LV_USE_USER_DATA        0
#define LV_USE_LOG              0
#define LV_USE_ASSERT_MEM       0
#define LV_USE_ASSERT_OBJ       0
#define LV_USE_PERF_MONITOR     0
#define LV_USE_MEM_MONITOR      0
#define LV_USE_SYSMON           0
#define LV_USE_PROFILER         0

#define LV_USE_ANIMATION        1
#define LV_USE_SHADOW           0
#define LV_USE_GRADIENT         0
#define LV_USE_IMG_TRANSFORM    0
#define LV_USE_IMG_SCALE        0
#define LV_USE_ARC              0
#define LV_USE_CANVAS           0
#define LV_USE_CHART            0
#define LV_USE_METER            0
#define LV_USE_TILEVIEW         0
#define LV_USE_MSGBOX           0
#define LV_USE_DRAW_MASKS       0
#define LV_USE_LAYER            0
#define LV_DRAW_SW_COMPLEX      0
#define LV_USE_LARGE_COORD      0

#define LV_FONT_MONTSERRAT_12   1
#define LV_FONT_DEFAULT         &lv_font_montserrat_12

#define LV_USE_FLEX             1
#define LV_USE_GRID             0
#define LV_USE_THEME_DEFAULT    1
#define LV_USE_THEME_BASIC      0
