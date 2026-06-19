/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_time_sync.h
  * @brief   Application-level time synchronization abstraction.
  *          Encapsulates the WiFi connection + NTP query + RTC update flow.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_TIME_SYNC_H
#define APP_TIME_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Try to connect to WiFi and sync the RTC from an NTP server.
 * @retval true if the RTC was updated with network time, false otherwise.
 * @note   Safe to call even when no module is attached: it simply returns false.
 */
bool App_TimeSync_SyncFromNetwork(void);

/**
 * @brief  Read the current RTC date and time.
 * @param  year   pointer to store full year (e.g. 2026).
 * @param  month  pointer to store month 1-12.
 * @param  day    pointer to store day 1-31.
 * @param  hour   pointer to store hour 0-23.
 * @param  minute pointer to store minute 0-59.
 * @param  second pointer to store second 0-59.
 * @retval true if the read succeeded, false otherwise.
 */
bool App_TimeSync_GetRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *day,
                                 uint8_t *hour, uint8_t *minute, uint8_t *second);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SYNC_H */
