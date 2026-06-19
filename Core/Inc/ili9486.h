/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ili9486.h
  * @brief   ILI9486 16-bit parallel LCD driver over FSMC (8080 interface).
  *          Target: Z350IT002 module, 320x480, controller ILI9486.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ILI9486_H
#define __ILI9486_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

/* LCD geometry */
#define ILI9486_WIDTH   320U
#define ILI9486_HEIGHT  480U

/* LCD control pins: backlight (PB0, active high) and reset (PG15, active low) */
#define LCD_BG_Pin       GPIO_PIN_0
#define LCD_BG_GPIO_Port GPIOB
#define LCD_RST_Pin       GPIO_PIN_15
#define LCD_RST_GPIO_Port GPIOG

/* FSMC Bank1 NE4 base address and A10-based RS offset (per board schematic) */
#define ILI9486_BASE_ADDR   0x6C000000U
#define ILI9486_RS_OFFSET   (1U << 11U)   /* A10 -> byte offset for 16-bit bus */

#define ILI9486_CMD_ADDR    (*(volatile uint16_t *)(ILI9486_BASE_ADDR))
#define ILI9486_DATA_ADDR   (*(volatile uint16_t *)(ILI9486_BASE_ADDR + ILI9486_RS_OFFSET))

/* RGB565 color helpers */
#define ILI9486_RGB(r, g, b) \
    ((((uint16_t)(r) & 0xF8U) << 8U) | \
     (((uint16_t)(g) & 0xFCU) << 3U) | \
     (((uint16_t)(b) & 0xF8U) >> 3U))

#define ILI9486_BLACK   0x0000U
#define ILI9486_WHITE   0xFFFFU
#define ILI9486_RED     0xF800U
#define ILI9486_GREEN   0x07E0U
#define ILI9486_BLUE    0x001FU
#define ILI9486_YELLOW  0xFFE0U
#define ILI9486_CYAN    0x07FFU
#define ILI9486_MAGENTA 0xF81FU

void ILI9486_Init(void);
void ILI9486_Reset(void);
void ILI9486_WriteCmd(uint8_t cmd);
void ILI9486_WriteData(uint16_t data);
uint32_t ILI9486_ReadID(void);
uint32_t ILI9486_ReadStatus(void);
void ILI9486_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9486_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9486_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9486_FillScreen(uint16_t color);
void ILI9486_DrawHorizontalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
void ILI9486_DrawVerticalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
void ILI9486_InvertDisplay(bool invert);
void ILI9486_SetRotation(uint8_t rotation);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9486_H */
