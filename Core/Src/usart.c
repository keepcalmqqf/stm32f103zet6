/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART1 driver implementation (register-level).
  *          - 115200 8N1, TX=PA9, RX=PA10
  *          - printf redirected to USART1 via __io_putchar
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define USART1_BRR_VALUE    (72000000U / USART1_BAUDRATE)   /* APB2 = 72 MHz */

/* Private function prototypes -----------------------------------------------*/
static void USART1_GPIO_Init(void);

/* Private user code ---------------------------------------------------------*/
static void USART1_GPIO_Init(void)
{
    /* Enable clocks for GPIOA and USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9  (USART1_TX): Alternate function push-pull, 50 MHz */
    GPIOA->CRH &= ~(0x0FU << 4U);
    GPIOA->CRH |=  (0x0BU << 4U);   /* CNF=10, MODE=11 */

    /* PA10 (USART1_RX): Floating input */
    GPIOA->CRH &= ~(0x0FU << 8U);
    GPIOA->CRH |=  (0x04U << 8U);   /* CNF=01, MODE=00 */
}

/* Exported functions --------------------------------------------------------*/
void USART1_Init(void)
{
    USART1_GPIO_Init();

    /* Configure baud rate and enable transmitter/receiver/USART */
    USART1->BRR = USART1_BRR_VALUE;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

/* Redirect printf/putchar to USART1 */
int __io_putchar(int ch)
{
    /* Wait until TX empty */
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
    }
    USART1->DR = (uint8_t)ch;
    return ch;
}
