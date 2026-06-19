/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_passthrough.c
  * @brief   USART1 <-> USART2 byte-level transparent bridge.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_passthrough.h"
#include "app_config.h"
#include "usart.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#ifndef APP_PASSTHROUGH_BUF_SIZE
#define APP_PASSTHROUGH_BUF_SIZE 256U
#endif

typedef struct
{
    uint8_t  buf[APP_PASSTHROUGH_BUF_SIZE];
    uint16_t head;
    uint16_t tail;
} PassthroughRingBuf_t;

static volatile PassthroughRingBuf_t s_u1_to_u2;
static volatile PassthroughRingBuf_t s_u2_to_u1;
static volatile bool s_active = false;

static bool ringbuf_push(volatile PassthroughRingBuf_t *rb, uint8_t byte)
{
    const uint16_t next_head = (uint16_t)((rb->head + 1U) % APP_PASSTHROUGH_BUF_SIZE);
    if (next_head == rb->tail)
    {
        return false; /* full */
    }
    rb->buf[rb->head] = byte;
    rb->head = next_head;
    return true;
}

static bool ringbuf_pop(volatile PassthroughRingBuf_t *rb, uint8_t *byte)
{
    if (rb->head == rb->tail)
    {
        return false; /* empty */
    }
    *byte = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1U) % APP_PASSTHROUGH_BUF_SIZE);
    return true;
}

static void esp_enable_pin_init(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = APP_ESP_EN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(APP_ESP_EN_GPIO_PORT, &GPIO_InitStruct);

    /* Hard reset the ESP32-C3: pull EN low for 100 ms, then release.
       This guarantees a clean power-on reset regardless of the pin's
       previous state. */
    HAL_GPIO_WritePin(APP_ESP_EN_GPIO_PORT, APP_ESP_EN_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(100U);
    HAL_GPIO_WritePin(APP_ESP_EN_GPIO_PORT, APP_ESP_EN_GPIO_PIN, GPIO_PIN_SET);
}

void App_Passthrough_Init(void)
{
    memset((void *)&s_u1_to_u2, 0, sizeof(s_u1_to_u2));
    memset((void *)&s_u2_to_u1, 0, sizeof(s_u2_to_u1));
    s_active = false;

    esp_enable_pin_init();
}

void App_Passthrough_Start(void)
{
    s_active = true;
}

bool App_Passthrough_IsActive(void)
{
    return s_active;
}

void App_Passthrough_OnUsart1Rx(uint8_t byte)
{
    if (s_active)
    {
        (void)ringbuf_push(&s_u1_to_u2, byte);
    }
}

void App_Passthrough_OnUsart2Rx(uint8_t byte)
{
    if (s_active)
    {
        (void)ringbuf_push(&s_u2_to_u1, byte);
    }
}

void App_Passthrough_Run(void)
{
    if (!s_active)
    {
        return;
    }

    uint8_t byte = 0U;

    /* PC -> ESP32-C3 */
    while (ringbuf_pop(&s_u1_to_u2, &byte))
    {
        (void)HAL_UART_Transmit(&huart2, &byte, 1U, HAL_MAX_DELAY);
    }

    /* ESP32-C3 -> PC */
    while (ringbuf_pop(&s_u2_to_u1, &byte))
    {
        (void)HAL_UART_Transmit(&huart1, &byte, 1U, HAL_MAX_DELAY);
    }
}
