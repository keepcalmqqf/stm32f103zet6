/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_time_sync.c
  * @brief   Connect to WiFi, query NTP, and update the STM32 RTC.
  *          Keeps network credentials and AT-command details out of main.c.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_time_sync.h"
#include "app_config.h"
#include "esp_wifi.h"
#include "rtc.h"
#include <stdio.h>

bool App_TimeSync_SyncFromNetwork(void)
{
    APP_LOG_INFO("Starting network time sync...");

    if (!ESP_WiFiInit(APP_WIFI_SSID, APP_WIFI_PASSWORD))
    {
        APP_LOG_WARN("WiFi connection failed");
        return false;
    }

    ESP_DateTime_t dt = {0};
    if (!ESP_SyncNtpTime(APP_NTP_TIMEZONE, APP_NTP_SERVER_1, APP_NTP_SERVER_2, &dt))
    {
        APP_LOG_WARN("NTP sync failed");
        return false;
    }

    if (RTC_SetTime(dt.hour, dt.minute, dt.second) != HAL_OK)
    {
        APP_LOG_ERROR("RTC_SetTime failed");
        return false;
    }

    if (RTC_SetDate((uint8_t)(dt.year - 2000U), dt.month, dt.day) != HAL_OK)
    {
        APP_LOG_ERROR("RTC_SetDate failed");
        return false;
    }

    APP_LOG_INFO("NTP synced: %04u-%02u-%02u %02u:%02u:%02u",
                 (unsigned int)dt.year, (unsigned int)dt.month, (unsigned int)dt.day,
                 (unsigned int)dt.hour, (unsigned int)dt.minute, (unsigned int)dt.second);
    return true;
}

bool App_TimeSync_GetRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *day,
                                 uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    if ((year == NULL) || (month == NULL) || (day == NULL) ||
        (hour == NULL) || (minute == NULL) || (second == NULL))
    {
        return false;
    }

    uint8_t y = 0;
    uint8_t m = 0;
    uint8_t d = 0;

    if (!RTC_GetTime(hour, minute, second) || !RTC_GetDate(&y, &m, &d))
    {
        return false;
    }

    *year  = (uint16_t)(y + 2000U);
    *month = m;
    *day   = d;
    return true;
}
