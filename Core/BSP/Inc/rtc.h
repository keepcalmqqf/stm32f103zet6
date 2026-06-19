/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   Board support driver for the STM32F103 internal real-time clock.
  *          Uses the external 32.768 kHz LSE crystal as the RTC clock source.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

/**
 * @brief  Initialize the RTC peripheral and start counting from the firmware
 *         build date/time. The LSE oscillator is enabled if not already running.
 * @retval HAL status.
 */
HAL_StatusTypeDef RTC_BspInit(void);

/**
 * @brief  Read the current RTC time.
 * @param  hours   pointer to store hours (0-23).
 * @param  minutes pointer to store minutes (0-59).
 * @param  seconds pointer to store seconds (0-59).
 * @retval true if the read succeeded, false otherwise.
 */
bool RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

/**
 * @brief  Set the RTC time.
 * @param  hours   hours to set (0-23).
 * @param  minutes minutes to set (0-59).
 * @param  seconds seconds to set (0-59).
 * @retval HAL status.
 */
HAL_StatusTypeDef RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

/**
 * @brief  Read the current RTC date.
 * @param  year  pointer to store year offset from 2000 (0-99).
 * @param  month pointer to store month (1-12).
 * @param  day   pointer to store day (1-31).
 * @retval true if the read succeeded, false otherwise.
 */
bool RTC_GetDate(uint8_t *year, uint8_t *month, uint8_t *day);

/**
 * @brief  Set the RTC date.
 * @param  year  year offset from 2000 (0-99).
 * @param  month month to set (1-12).
 * @param  day   day to set (1-31).
 * @retval HAL status.
 */
HAL_StatusTypeDef RTC_SetDate(uint8_t year, uint8_t month, uint8_t day);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H */
