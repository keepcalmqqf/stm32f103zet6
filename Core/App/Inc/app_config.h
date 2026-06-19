/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_config.h
  * @brief   Application-level configuration constants and logging macros.
  *          Centralizes tunable parameters so that main.c and business modules
  *          do not depend on specific hardware credentials or timing values.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/* Exported defines ----------------------------------------------------------*/

/* WiFi access point credentials used by the ESP32-C3 AT module. */
#define APP_WIFI_SSID       "Xiaomi_3799"
#define APP_WIFI_PASSWORD   "qwertyuiop"

/* NTP configuration. */
#define APP_NTP_TIMEZONE    8                         /* UTC+8 */
#define APP_NTP_SERVER_1    "ntp.aliyun.com"
#define APP_NTP_SERVER_2    "cn.ntp.org.cn"

/* UI refresh period (ms). Kept in sync with LVGL default refr period. */
#define APP_UI_REFRESH_MS   33U

/* Time update interval (ms). */
#define APP_TIME_UPDATE_MS  1000U

/* ESP32-C3 passthrough configuration. */
#define APP_PASSTHROUGH_ENABLED      1
#define APP_PASSTHROUGH_BUF_SIZE     256U

/* ESP32-C3 enable pin (active high). */
#define APP_ESP_EN_GPIO_PORT         GPIOE
#define APP_ESP_EN_GPIO_PIN          GPIO_PIN_4

/* Logging abstraction: route everything through printf on USART1.
   Disable by defining APP_LOG_DISABLE before including this header. */
#ifndef APP_LOG_DISABLE
  #define APP_LOG_INFO(fmt, ...)  printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
  #define APP_LOG_WARN(fmt, ...)  printf("[WARN] " fmt "\r\n", ##__VA_ARGS__)
  #define APP_LOG_ERROR(fmt, ...) printf("[ERROR] " fmt "\r\n", ##__VA_ARGS__)
#else
  #define APP_LOG_INFO(fmt, ...)  ((void)0U)
  #define APP_LOG_WARN(fmt, ...)  ((void)0U)
  #define APP_LOG_ERROR(fmt, ...) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
