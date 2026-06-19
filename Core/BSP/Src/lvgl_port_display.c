/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lvgl_port_display.c
  * @brief   LVGL v9 display port for the ILI9486 320x480 LCD over FSMC.
  *          Uses partial rendering with a single small draw buffer.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "lvgl_port_display.h"
#include "ili9486.h"
#include "lvgl.h"

/* Horizontal resolution must match the current screen orientation. */
#define DISP_HOR_RES 320U
#define DISP_VER_RES 480U

/* Partial buffer: one tile of 1/20 screen height to save RAM. */
#define DRAW_BUF_LINES (DISP_VER_RES / 20U)
#define DRAW_BUF_SIZE  (DISP_HOR_RES * DRAW_BUF_LINES)

static lv_color_t s_draw_buf[DRAW_BUF_SIZE];

/**
 * @brief  LVGL flush callback: copy the rendered pixels to the LCD.
 */
static void disp_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    const uint16_t x0 = (uint16_t)area->x1;
    const uint16_t y0 = (uint16_t)area->y1;
    const uint16_t x1 = (uint16_t)area->x2;
    const uint16_t y1 = (uint16_t)area->y2;

    ILI9486_SetAddressWindow(x0, y0, x1, y1);

    const uint32_t pixel_count = (uint32_t)(x1 - x0 + 1) * (uint32_t)(y1 - y0 + 1);
    const uint16_t *pixels = (const uint16_t *)px_map;

    for (uint32_t i = 0; i < pixel_count; i++)
    {
        ILI9486_DATA_ADDR = pixels[i];
    }

    lv_display_flush_ready(display);
}

void LVGL_PortDisplayInit(uint32_t hor_res, uint32_t ver_res)
{
    lv_display_t *display = lv_display_create((int32_t)hor_res, (int32_t)ver_res);
    lv_display_set_flush_cb(display, disp_flush_cb);
    lv_display_set_buffers(display,
                           s_draw_buf,
                           NULL,
                           sizeof(s_draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
}
