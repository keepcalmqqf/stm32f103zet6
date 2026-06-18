/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    led.c
  * @brief   LED abstraction layer implementation.
  *          Encapsulates GPIO port/pin mapping and active-low polarity so that
  *          application code does not depend on hardware details.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "led.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} LED_Pin_t;

/* Private variables ---------------------------------------------------------*/
static const LED_Pin_t led_pins[LED_COUNT] =
{
    {LED1_GPIO_Port, LED1_Pin},
    {LED2_GPIO_Port, LED2_Pin},
    {LED3_GPIO_Port, LED3_Pin},
};

static uint8_t led_current = 0;

/* Private function prototypes -----------------------------------------------*/
static void LED_Write(uint8_t index, GPIO_PinState state);

/* Private user code ---------------------------------------------------------*/
static void LED_Write(uint8_t index, GPIO_PinState state)
{
    if (index < LED_COUNT)
    {
        HAL_GPIO_WritePin(led_pins[index].port, led_pins[index].pin, state);
    }
}

/* Exported functions --------------------------------------------------------*/
void LED_Init(void)
{
    led_current = 0;
    LED_AllOff();
}

void LED_AllOff(void)
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        LED_Write(i, GPIO_PIN_SET); /* Active low: SET = off */
    }
}

void LED_On(uint8_t index)
{
    LED_Write(index, GPIO_PIN_RESET); /* Active low: RESET = on */
}

void LED_Off(uint8_t index)
{
    LED_Write(index, GPIO_PIN_SET); /* Active low: SET = off */
}

uint8_t LED_ToggleNext(void)
{
    LED_AllOff();
    LED_On(led_current);
    uint8_t on_index = led_current;
    led_current = (led_current + 1) % LED_COUNT;
    return on_index;
}
