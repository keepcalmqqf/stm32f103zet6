/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   Board support driver for the STM32F103 internal real-time clock.
  *          Uses the external 32.768 kHz LSE crystal as the RTC clock source.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "rtc.h"

/* RTC peripheral handle. */
static RTC_HandleTypeDef hrtc = {0};

/**
 * @brief  Parse the hour from the __TIME__ macro ("HH:MM:SS").
 */
static uint8_t rtc_parse_build_hour(void)
{
    return (uint8_t)(((__TIME__[0] - '0') * 10U) + (__TIME__[1] - '0'));
}

/**
 * @brief  Parse the minute from the __TIME__ macro.
 */
static uint8_t rtc_parse_build_minute(void)
{
    return (uint8_t)(((__TIME__[3] - '0') * 10U) + (__TIME__[4] - '0'));
}

/**
 * @brief  Parse the second from the __TIME__ macro.
 */
static uint8_t rtc_parse_build_second(void)
{
    return (uint8_t)(((__TIME__[6] - '0') * 10U) + (__TIME__[7] - '0'));
}

HAL_StatusTypeDef RTC_BspInit(void)
{
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;

    /* Enable PWR and BKP clocks so we can access the backup domain. */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    /* Allow write access to the backup domain (RTC registers live there). */
    HAL_PWR_EnableBkUpAccess();

    /* If the RTC is already running (e.g. preserved by VBAT), just re-attach
       the HAL handle and leave the counter value untouched. */
    if ((RCC->BDCR & RCC_BDCR_RTCEN) != 0U)
    {
        return HAL_RTC_Init(&hrtc);
    }

    /* Start the external 32.768 kHz oscillator. */
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    osc.LSEState = RCC_LSE_ON;
    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&osc);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Select LSE as the RTC clock source and enable the RTC. */
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
    __HAL_RCC_RTC_ENABLE();

    status = HAL_RTC_Init(&hrtc);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Load an initial time from the firmware build timestamp. */
    return RTC_SetTime(rtc_parse_build_hour(),
                       rtc_parse_build_minute(),
                       rtc_parse_build_second());
}

bool RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    if ((hours == NULL) || (minutes == NULL) || (seconds == NULL))
    {
        return false;
    }

    RTC_TimeTypeDef sTime = {0};
    HAL_StatusTypeDef status = HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    if (status != HAL_OK)
    {
        return false;
    }

    *hours = sTime.Hours;
    *minutes = sTime.Minutes;
    *seconds = sTime.Seconds;
    return true;
}

HAL_StatusTypeDef RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = seconds;

    return HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}
