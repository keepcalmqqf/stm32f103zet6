/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_ui.c
  * @brief   LVGL-based application UI: splash screen and clock display.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_ui.h"
#include "app_config.h"
#include "ili9486.h"
#include "lvgl.h"
#include "lvgl_port_display.h"
#include <stdio.h>

/* Single UI widget exposed internally. */
static lv_obj_t *s_time_label = NULL;

/* Show a brief color sequence to confirm LCD is alive. */
static void App_UI_ShowSplash(void)
{
    ILI9486_FillScreen(ILI9486_BLUE);
    HAL_Delay(500);
    ILI9486_FillScreen(ILI9486_RED);
    HAL_Delay(500);
    ILI9486_FillScreen(ILI9486_GREEN);
    HAL_Delay(500);
    ILI9486_FillScreen(ILI9486_BLACK);
}

bool App_UI_Init(bool ntp_synced)
{
    App_UI_ShowSplash();

    lv_init();
    lv_tick_set_cb(HAL_GetTick);
    LVGL_PortDisplayInit(ILI9486_WIDTH, ILI9486_HEIGHT);

    s_time_label = lv_label_create(lv_screen_active());
    if (s_time_label == NULL)
    {
        APP_LOG_ERROR("Failed to create time label");
        return false;
    }

    lv_obj_set_style_text_font(s_time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(s_time_label, lv_color_hex(0x00FF00), 0);
    lv_obj_align(s_time_label, LV_ALIGN_CENTER, 0, 0);

    if (ntp_synced)
    {
        lv_label_set_text(s_time_label, "--:--:--");
    }
    else
    {
        lv_label_set_text(s_time_label, "RTC 00:00:00");
    }

    APP_LOG_INFO("LVGL UI initialized");
    return true;
}

void App_UI_UpdateClock(uint16_t year, uint8_t month, uint8_t day,
                        uint8_t hour, uint8_t minute, uint8_t second)
{
    if (s_time_label == NULL)
    {
        return;
    }

    char time_buf[32];
    (void)snprintf(time_buf, sizeof(time_buf),
                   "%04u-%02u-%02u\n%02u:%02u:%02u",
                   (unsigned int)year,
                   (unsigned int)month,
                   (unsigned int)day,
                   (unsigned int)hour,
                   (unsigned int)minute,
                   (unsigned int)second);
    lv_label_set_text(s_time_label, time_buf);
}

void App_UI_Handler(void)
{
    lv_timer_handler();
}
