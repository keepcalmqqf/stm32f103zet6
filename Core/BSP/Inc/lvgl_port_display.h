/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lvgl_port_display.h
  * @brief   LVGL v9 display port for the ILI9486 320x480 LCD over FSMC.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LVGL_PORT_DISPLAY_H
#define __LVGL_PORT_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Initialize LVGL display driver and bind it to the ILI9486 LCD.
 * @param  hor_res horizontal resolution in pixels.
 * @param  ver_res vertical resolution in pixels.
 */
void LVGL_PortDisplayInit(uint32_t hor_res, uint32_t ver_res);

#ifdef __cplusplus
}
#endif

#endif /* __LVGL_PORT_DISPLAY_H */
