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
#include "app_led_screen.h"
#include "rtc.h"
#include "lvgl.h"
#include "lvgl_port_display.h"
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
static lv_obj_t *s_time_label = NULL;
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
  /* Early sanity indicator: turn LED1 on before HAL_Init. */
  LED_EarlyInit();
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
  LED_Init();
  USART1_Init();
  printf("STM32F103ZET6 USART1 initialized: %lu baud\r\n", (unsigned long)USART1_BAUDRATE);

  /* Diagnostic: blink LED2 and LED3 to confirm we reached init. */
  LED_Blink(1, 6, 200);
  LED_Blink(2, 6, 200);

  ILI9486_Init();
  uint32_t lcd_id = ILI9486_ReadID();
  uint32_t lcd_status = ILI9486_ReadStatus();
  printf("ILI9486 LCD initialized over FSMC, ID: 0x%06lX, status: 0x%08lX\r\n",
         lcd_id, lcd_status);

  ILI9486_FillScreen(ILI9486_BLUE);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_RED);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_GREEN);
  HAL_Delay(500);
  ILI9486_FillScreen(ILI9486_BLACK);

  HAL_StatusTypeDef rtc_status = RTC_BspInit();
  printf("RTC_BspInit status: %d\r\n", (int)rtc_status);

  lv_init();
  lv_tick_set_cb(HAL_GetTick);
  LVGL_PortDisplayInit(ILI9486_WIDTH, ILI9486_HEIGHT);

  s_time_label = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(s_time_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(s_time_label, lv_color_hex(0x00FF00), 0);
  lv_obj_align(s_time_label, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(s_time_label, "00:00:00");
  printf("LVGL initialized with ILI9486 display port\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_rtc_update = 0;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    const uint32_t now = HAL_GetTick();

    if ((now - last_rtc_update) >= 1000U)
    {
      uint8_t hours = 0;
      uint8_t minutes = 0;
      uint8_t seconds = 0;

      if (RTC_GetTime(&hours, &minutes, &seconds) && (s_time_label != NULL))
      {
        char time_buf[16];
        snprintf(time_buf, sizeof(time_buf), "%02u:%02u:%02u",
                 (unsigned int)hours,
                 (unsigned int)minutes,
                 (unsigned int)seconds);
        lv_label_set_text(s_time_label, time_buf);

        printf("RTC time: %s\r\n", time_buf);
      }

      (void)LED_ToggleNext();
      last_rtc_update = now;
    }

    lv_timer_handler();
    HAL_Delay(LV_DEF_REFR_PERIOD);
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
