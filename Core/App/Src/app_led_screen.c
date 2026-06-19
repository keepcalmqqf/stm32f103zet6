/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_led_screen.c
  * @brief   Application-level mapping between LEDs and LCD screen colors.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_led_screen.h"
#include "ili9486.h"
#include "led.h"

/* Private variables ---------------------------------------------------------*/
static const uint16_t led_screen_colors[LED_COUNT] =
{
    ILI9486_YELLOW, /* LED1 -> yellow */
    ILI9486_BLUE,   /* LED2 -> blue */
    ILI9486_GREEN,  /* LED3 -> green */
};

static const char *led_color_names[LED_COUNT] =
{
    "YELLOW",
    "BLUE",
    "GREEN",
};

/* Exported functions --------------------------------------------------------*/
uint16_t AppLedScreen_GetColor(uint8_t led_index)
{
    if (led_index >= LED_COUNT)
    {
        return ILI9486_BLACK;
    }
    return led_screen_colors[led_index];
}

const char *AppLedScreen_GetName(uint8_t led_index)
{
    if (led_index >= LED_COUNT)
    {
        return "UNKNOWN";
    }
    return led_color_names[led_index];
}
