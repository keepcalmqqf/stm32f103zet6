/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    led.h
  * @brief   LED abstraction layer header.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ----------------------------------------------------------*/
#define LED_COUNT 3

/* Exported functions prototypes ---------------------------------------------*/
void LED_Init(void);
void LED_AllOff(void);
void LED_On(uint8_t index);
void LED_Off(uint8_t index);
void LED_ToggleNext(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */
