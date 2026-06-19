/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    esp_wifi.c
  * @brief   WiFi/NTP sync driver for an ESP32-C3 running AT firmware on USART2.
  *          - Assumes USART2 is already initialized at 115200 8N1.
  *          - Avoids printf/__io_putchar during AT traffic to prevent bus clashes.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "esp_wifi.h"
#include "usart.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Size of the incoming AT response accumulator. */
#define ESP_RX_BUF_SIZE 512U

/* Short inter-character guard used when polling for the next byte. */
#define ESP_RX_POLL_MS  10U

/* Time to wait for the ESP32-C3 AT firmware to boot after reset/power-on. */
#define ESP_BOOT_DELAY_MS 2000U

/* Once the RX line is idle for this long, assume the boot log has finished. */
#define ESP_BOOT_IDLE_MS  300U

/* Maximum time to keep draining the boot log, to avoid an infinite loop. */
#define ESP_BOOT_MAX_DRAIN_MS 5000U

/**
 * @brief  Discard bytes until the RX line has been idle for ESP_BOOT_IDLE_MS.
 *         This helps us wait out the ESP boot log before sending the first AT.
 */
static void esp_drain_rx_until_idle(void)
{
    uint32_t last_rx = HAL_GetTick();
    const uint32_t start = HAL_GetTick();
    while (((HAL_GetTick() - last_rx) < ESP_BOOT_IDLE_MS) &&
           ((HAL_GetTick() - start) < ESP_BOOT_MAX_DRAIN_MS))
    {
        uint8_t ch = 0U;
        if (HAL_UART_Receive(&huart2, &ch, 1U, ESP_RX_POLL_MS) == HAL_OK)
        {
            last_rx = HAL_GetTick();
        }
    }
}

/**
 * @brief  Print the raw ESP response for troubleshooting.
 * @note   Only call after an AT exchange has finished, otherwise the printf
 *         traffic on USART2 collides with the ESP traffic.
 */
static void esp_debug_dump(const char *label, const char *buf)
{
    printf("[ESP DBG] %s: [", label);
    for (const char *p = buf; *p != '\0'; p++)
    {
        const char c = *p;
        if ((c >= 0x20) && (c <= 0x7E))
        {
            printf("%c", c);
        }
        else
        {
            printf("\\x%02X", (unsigned char)c);
        }
    }
    printf("]\r\n");
}

/**
 * @brief  Send an AT command and wait for an expected substring in the response.
 * @param  cmd           Null-terminated command to send, including \r\n.
 * @param  expected      Substring that indicates success.
 * @param  timeout_ms    Maximum time to wait for @p expected.
 * @retval true if @p expected was seen, false on timeout.
 */
static bool esp_send_command(const char *cmd, const char *expected, uint32_t timeout_ms)
{
    const uint16_t cmd_len = (uint16_t)strlen(cmd);
    HAL_UART_Transmit(&huart2, (const uint8_t *)cmd, cmd_len, 1000U);

    char rx_buf[ESP_RX_BUF_SIZE];
    uint16_t idx = 0U;
    const uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint8_t ch = 0U;
        if (HAL_UART_Receive(&huart2, &ch, 1U, ESP_RX_POLL_MS) == HAL_OK)
        {
            if (idx < (ESP_RX_BUF_SIZE - 1U))
            {
                rx_buf[idx++] = (char)ch;
                rx_buf[idx] = '\0';
            }

            if (strstr(rx_buf, expected) != NULL)
            {
                return true;
            }
        }
    }

    esp_debug_dump("unexpected/no response", rx_buf);
    return false;
}

/**
 * @brief  Convert a 3-letter month abbreviation to 1-12.
 */
static bool esp_parse_month(const char *mon, uint8_t *month)
{
    static const char *s_months[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    for (uint8_t i = 0U; i < 12U; i++)
    {
        if (strncmp(mon, s_months[i], 3U) == 0)
        {
            *month = (uint8_t)(i + 1U);
            return true;
        }
    }
    return false;
}

/**
 * @brief  Parse the ESP AT NTP time response:
 *         +CIPSNTPTIME:Mon Jan 18 11:28:21 2021
 */
static bool esp_parse_ntp_response(const char *response, ESP_DateTime_t *dt)
{
    const char *p = strstr(response, "+CIPSNTPTIME:");
    if (p == NULL)
    {
        return false;
    }

    /* Skip the prefix and the day-of-week token ("Mon "). */
    p += strlen("+CIPSNTPTIME:");
    while (*p != '\0' && *p != ' ')
    {
        p++;
    }
    if (*p == ' ')
    {
        p++;
    }

    char mon[4] = {0};
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int year = 0;

    if (sscanf(p, "%3s %d %d:%d:%d %d", mon, &day, &hour, &minute, &second, &year) != 6)
    {
        return false;
    }

    uint8_t month = 0U;
    if (!esp_parse_month(mon, &month))
    {
        return false;
    }

    dt->year   = (uint16_t)year;
    dt->month  = month;
    dt->day    = (uint8_t)day;
    dt->hour   = (uint8_t)hour;
    dt->minute = (uint8_t)minute;
    dt->second = (uint8_t)second;

    return true;
}

bool ESP_WiFiInit(const char *ssid, const char *password)
{
    if ((ssid == NULL) || (password == NULL))
    {
        return false;
    }

    /* Wait for the ESP32-C3 AT firmware to finish booting. */
    HAL_Delay(ESP_BOOT_DELAY_MS);
    esp_drain_rx_until_idle();

    /* Disable command echo to simplify parsing. */
    if (!esp_send_command("ATE0\r\n", "OK", 1000U))
    {
        return false;
    }

    /* Set station mode. */
    if (!esp_send_command("AT+CWMODE=1\r\n", "OK", 2000U))
    {
        return false;
    }

    /* Connect to the access point. This can take several seconds. */
    char cmd[128];
    (void)snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    if (!esp_send_command(cmd, "OK", 20000U))
    {
        return false;
    }

    return true;
}

bool ESP_SyncNtpTime(int8_t timezone, const char *ntp1, const char *ntp2, ESP_DateTime_t *dt)
{
    if ((ntp1 == NULL) || (dt == NULL))
    {
        return false;
    }

    /* Configure SNTP: enable, timezone, up to two servers. */
    char cmd[256];
    if (ntp2 != NULL)
    {
        (void)snprintf(cmd, sizeof(cmd),
                       "AT+CIPSNTPCFG=1,%d,\"%s\",\"%s\"\r\n",
                       (int)timezone, ntp1, ntp2);
    }
    else
    {
        (void)snprintf(cmd, sizeof(cmd),
                       "AT+CIPSNTPCFG=1,%d,\"%s\"\r\n",
                       (int)timezone, ntp1);
    }

    if (!esp_send_command(cmd, "OK", 5000U))
    {
        return false;
    }

    /* Give the module a moment to resolve and query the NTP server. */
    HAL_Delay(1000U);

    /* Query the current SNTP time. */
    HAL_UART_Transmit(&huart2, (const uint8_t *)"AT+CIPSNTPTIME?\r\n",
                      (uint16_t)strlen("AT+CIPSNTPTIME?\r\n"), 1000U);

    char rx_buf[ESP_RX_BUF_SIZE];
    uint16_t idx = 0U;
    const uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 5000U)
    {
        uint8_t ch = 0U;
        if (HAL_UART_Receive(&huart2, &ch, 1U, ESP_RX_POLL_MS) == HAL_OK)
        {
            if (idx < (ESP_RX_BUF_SIZE - 1U))
            {
                rx_buf[idx++] = (char)ch;
                rx_buf[idx] = '\0';
            }

            /* The response ends with "OK". */
            if (strstr(rx_buf, "OK") != NULL)
            {
                return esp_parse_ntp_response(rx_buf, dt);
            }
        }
    }

    return false;
}
