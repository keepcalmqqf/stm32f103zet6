/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_system.c
  * @brief   System-level initialization and main loop.
  *          Coordinates BSP drivers, UI, and time sync without knowing their
  *          internal implementation details.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_system.h"
#include "app_config.h"
#include "app_time_sync.h"
#include "app_passthrough.h"
#include "app_ui.h"
#include "fsmc.h"
#include "gpio.h"
#include "led.h"
#include "ili9486.h"
#include "rtc.h"
#include "usart.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
static bool App_System_InitPeripherals(void);
static bool App_System_InitBoard(void);
static bool App_System_InitRtcAndSync(bool *ntp_synced);
static void App_System_LcdDiagnostics(void);

static bool App_System_InitPeripherals(void)
{
    MX_GPIO_Init();
    MX_FSMC_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    return true;
}

static bool App_System_InitBoard(void)
{
    LED_Init();
    USART1_Init();
    APP_LOG_INFO("USART1 initialized: %lu baud", (unsigned long)USART1_BAUDRATE);

    App_Passthrough_Init();
    APP_LOG_INFO("Passthrough bridge initialized");

    /* Diagnostic: blink LED2 and LED3 to confirm we reached init. */
    LED_Blink(1, 6, 200);
    LED_Blink(2, 6, 200);

    return true;
}

static void App_System_LcdDiagnostics(void)
{
    ILI9486_Init();

    const uint32_t lcd_id = ILI9486_ReadID();
    const uint32_t lcd_status = ILI9486_ReadStatus();
    APP_LOG_INFO("ILI9486 LCD initialized over FSMC, ID: 0x%06lX, status: 0x%08lX",
                 lcd_id, lcd_status);
}

static bool App_System_InitRtcAndSync(bool *ntp_synced)
{
    HAL_StatusTypeDef rtc_status = RTC_BspInit();
    APP_LOG_INFO("RTC_BspInit status: %d", (int)rtc_status);

    if (rtc_status != HAL_OK)
    {
        return false;
    }

    /* Attempt network sync; if it fails we still continue with RTC time. */
    *ntp_synced = App_TimeSync_SyncFromNetwork();
    return true;
}

bool App_System_Init(void)
{
    if (!App_System_InitPeripherals())
    {
        return false;
    }

    if (!App_System_InitBoard())
    {
        return false;
    }

    App_System_LcdDiagnostics();

    bool ntp_synced = false;
    if (!App_System_InitRtcAndSync(&ntp_synced))
    {
        return false;
    }

    if (!App_UI_Init(ntp_synced))
    {
        return false;
    }

    APP_LOG_INFO("Application initialized successfully");

    if (ntp_synced)
    {
        APP_LOG_INFO("Entering passthrough mode");
        App_Passthrough_Start();
    }

    return true;
}

void App_System_Run(void)
{
    uint32_t last_time_update = 0U;

    while (1)
    {
        const uint32_t now = HAL_GetTick();

        if ((now - last_time_update) >= APP_TIME_UPDATE_MS)
        {
            uint16_t year = 0;
            uint8_t month = 0;
            uint8_t day = 0;
            uint8_t hour = 0;
            uint8_t minute = 0;
            uint8_t second = 0;

            if (App_TimeSync_GetRtcDateTime(&year, &month, &day, &hour, &minute, &second))
            {
                App_UI_UpdateClock(year, month, day, hour, minute, second);
                APP_LOG_INFO("RTC: %04u-%02u-%02u %02u:%02u:%02u",
                             (unsigned int)year, (unsigned int)month, (unsigned int)day,
                             (unsigned int)hour, (unsigned int)minute, (unsigned int)second);
            }

            (void)LED_ToggleNext();
            last_time_update = now;
        }

        const uint32_t ui_delay_ms = App_UI_Handler();
        App_Passthrough_Run();
        HAL_Delay(ui_delay_ms);
    }
}
