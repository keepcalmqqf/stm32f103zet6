/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   USART1 driver header.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported defines ----------------------------------------------------------*/
#define USART1_BAUDRATE 115200U

/* Exported functions prototypes ---------------------------------------------*/
void USART1_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
