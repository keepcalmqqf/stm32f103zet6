/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "ili9486.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* Early LED test: turn LED1 on before any init to confirm MCU is running */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* LCD backlight control: PB0, active high */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct_B = {0};
  GPIO_InitStruct_B.Pin = LCD_BG_Pin;
  GPIO_InitStruct_B.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_B.Pull = GPIO_NOPULL;
  GPIO_InitStruct_B.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BG_GPIO_Port, &GPIO_InitStruct_B);
  HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_SET);

  /* LCD hardware reset: PG15, active low */
  GPIO_InitStruct_B.Pin = LCD_RST_Pin;
  GPIO_InitStruct_B.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_B.Pull = GPIO_NOPULL;
  GPIO_InitStruct_B.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct_B);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);

  LED_Init();
  USART1_Init();
  printf("STM32F103ZET6 USART1 initialized: %lu baud\r\n", (unsigned long)USART1_BAUDRATE);

  /* Diagnostic: blink LED2 to confirm we reach here */
  for (int i = 0; i < 6; i++)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
    HAL_Delay(100);
  }
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

  /* Step 1: FSMC init already done above */
  printf("FSMC init done\r\n");

  /* Diagnostic: blink LED3 to confirm FSMC init passed */
  for (int i = 0; i < 6; i++)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
    HAL_Delay(100);
  }
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

  /* Step 2: test LCD reset command only */
  ILI9486_Reset();
  uint32_t id_after_reset = ILI9486_ReadID();
  printf("LCD reset done, ID after reset: 0x%06lX\r\n", id_after_reset);

  /* Step 3: full LCD init */
  ILI9486_Init();
  uint32_t id_after_init = ILI9486_ReadID();
  uint32_t status = ILI9486_ReadStatus();
  printf("ILI9486 LCD initialized over FSMC, ID: 0x%06lX, status: 0x%08lX\r\n",
         id_after_init, status);

  ILI9486_FillScreen(ILI9486_BLUE);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_RED);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_GREEN);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_BLACK);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Keep the LCD background color in sync with the currently lit LED.
       NOTE: if the physical LED colors on your board differ from the
       mapping below, adjust the led_screen_colors table. */
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

    uint8_t led_index = LED_ToggleNext();

    /* Brief blackout to make the transition visible. */
    ILI9486_FillScreen(ILI9486_BLACK);
    HAL_Delay(200);

    ILI9486_FillScreen(led_screen_colors[led_index]);
    printf("LED %u ON -> screen %s (0x%04X)\r\n",
           (unsigned int)(led_index + 1U),
           led_color_names[led_index],
           (unsigned int)led_screen_colors[led_index]);
    HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
