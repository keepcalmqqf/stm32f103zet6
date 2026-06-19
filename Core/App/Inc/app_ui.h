/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_ui.h
  * @brief   Application UI abstraction. Hides LVGL widget details from the
  *          system orchestration code.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_UI_H
#define APP_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Initialize the LVGL library and create the application UI.
 * @param  ntp_synced true if the RTC has already been synchronized from NTP.
 * @retval true if initialization succeeded, false otherwise.
 */
bool App_UI_Init(bool ntp_synced);

/**
 * @brief  Update the on-screen clock with the given date and time.
 * @param  year   Full year (e.g. 2026).
 * @param  month  Month 1-12.
 * @param  day    Day 1-31.
 * @param  hour   Hour 0-23.
 * @param  minute Minute 0-59.
 * @param  second Second 0-59.
 */
void App_UI_UpdateClock(uint16_t year, uint8_t month, uint8_t day,
                        uint8_t hour, uint8_t minute, uint8_t second);

/**
 * @brief  Run one iteration of the LVGL timer handler.
 * @note   Caller is responsible for pacing this function (typically every 33 ms).
 */
void App_UI_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_UI_H */
