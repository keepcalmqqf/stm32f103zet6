/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_led_screen.h
  * @brief   Application-level mapping between LEDs and LCD screen colors.
  *          Keeps the demo business rule (which LED maps to which screen color)
  *          out of main.c.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_LED_SCREEN_H
#define APP_LED_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported functions prototypes ---------------------------------------------*/
uint16_t AppLedScreen_GetColor(uint8_t led_index);
const char *AppLedScreen_GetName(uint8_t led_index);

#ifdef __cplusplus
}
#endif

#endif /* APP_LED_SCREEN_H */
