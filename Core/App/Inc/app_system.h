/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_system.h
  * @brief   High-level application system orchestration.
  *          Initializes the board, runs time sync, and enters the main loop.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief  Full application initialization.
 * @retval true if all subsystems initialized correctly, false otherwise.
 * @note   On failure the caller should invoke Error_Handler().
 */
bool App_System_Init(void);

/**
 * @brief  Run the application main loop. Does not return.
 */
void App_System_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SYSTEM_H */
