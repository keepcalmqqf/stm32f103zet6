/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    esp_wifi.h
  * @brief   WiFi/NTP sync driver for an ESP32-C3 (AT firmware) connected to USART1.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ESP_WIFI_H
#define __ESP_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Date/time structure used for NTP results.
 */
typedef struct
{
    uint16_t year;   /**< Full year, e.g. 2026. */
    uint8_t  month;  /**< 1-12. */
    uint8_t  day;    /**< 1-31. */
    uint8_t  hour;   /**< 0-23. */
    uint8_t  minute; /**< 0-59. */
    uint8_t  second; /**< 0-59. */
} ESP_DateTime_t;

/**
 * @brief  Initialize the ESP32-C3 module and connect to the configured AP.
 * @param  ssid     WiFi SSID (null-terminated).
 * @param  password WiFi password (null-terminated).
 * @retval true if the module responds and connects to the AP, false otherwise.
 * @note   Blocks for several seconds while the WiFi connection is established.
 */
bool ESP_WiFiInit(const char *ssid, const char *password);

/**
 * @brief  Query an NTP server and fill the provided date/time structure.
 * @param  timezone  Time zone offset from UTC in hours (e.g. 8 for UTC+8, -5 for UTC-5).
 * @param  ntp1      Primary NTP server host name (null-terminated).
 * @param  ntp2      Secondary NTP server host name (null-terminated); may be NULL.
 * @param  dt        Pointer to store the parsed date/time.
 * @retval true if the NTP query succeeded and @p dt was filled, false otherwise.
 * @pre    ESP_WiFiInit() must have returned true.
 */
bool ESP_SyncNtpTime(int8_t timezone, const char *ntp1, const char *ntp2, ESP_DateTime_t *dt);

#ifdef __cplusplus
}
#endif

#endif /* __ESP_WIFI_H */
