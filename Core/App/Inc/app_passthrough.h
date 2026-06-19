/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_passthrough.h
  * @brief   USART1 <-> USART2 debug passthrough bridge.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_PASSTHROUGH_H
#define APP_PASSTHROUGH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化透传模块的 ring buffer 与 PE4 使能脚。
 * @note  应在 USART1/2 初始化之后调用。
 */
void App_Passthrough_Init(void);

/**
 * @brief 启用透传模式。
 * @note  调用后 USART1 不再输出日志，所有字节均转发到/来自 ESP32-C3。
 */
void App_Passthrough_Start(void);

/**
 * @brief 透传主任务，从 ring buffer 中取出数据并完成转发。
 * @note  应在主循环中持续调用。
 */
void App_Passthrough_Run(void);

/**
 * @brief USART1 RX 中断回调处理入口。
 */
void App_Passthrough_OnUsart1Rx(uint8_t byte);

/**
 * @brief USART2 RX 中断回调处理入口。
 */
void App_Passthrough_OnUsart2Rx(uint8_t byte);

/**
 * @brief 查询当前是否处于透传模式。
 */
bool App_Passthrough_IsActive(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PASSTHROUGH_H */
