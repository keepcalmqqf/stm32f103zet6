/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ili9486.c
  * @brief   ILI9486 16-bit parallel LCD driver over FSMC (8080 interface).
  *          Target: Z350IT002 module, 320x480, controller ILI9486.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ili9486.h"
#include "main.h"

/* ILI9486 command set */
#define ILI9486_NOP        0x00
#define ILI9486_SWRESET    0x01
#define ILI9486_RDDID      0x04
#define ILI9486_RDDST      0x09
#define ILI9486_SLPIN      0x10
#define ILI9486_SLPOUT     0x11
#define ILI9486_PTLON      0x12
#define ILI9486_NORON      0x13
#define ILI9486_INVOFF     0x20
#define ILI9486_INVON      0x21
#define ILI9486_DISPOFF    0x28
#define ILI9486_DISPON     0x29
#define ILI9486_CASET      0x2A
#define ILI9486_PASET      0x2B
#define ILI9486_RAMWR      0x2C
#define ILI9486_RAMRD      0x2E
#define ILI9486_PTLAR      0x30
#define ILI9486_VSCRDEF    0x33
#define ILI9486_MADCTL     0x36
#define ILI9486_VSCRSADD   0x37
#define ILI9486_PIXFMT     0x3A
#define ILI9486_WRDISBV    0x51
#define ILI9486_RDDISBV    0x52
#define ILI9486_WRCTRLD    0x53

/* Memory access control bits */
#define ILI9486_MADCTL_MY  0x80
#define ILI9486_MADCTL_MX  0x40
#define ILI9486_MADCTL_MV  0x20
#define ILI9486_MADCTL_ML  0x10
#define ILI9486_MADCTL_RGB 0x00
#define ILI9486_MADCTL_BGR 0x08

/* Public functions */

/**
  * @brief  Low-level write of an 8-bit command.
  */
void ILI9486_WriteCmd(uint8_t cmd)
{
    ILI9486_CMD_ADDR = cmd;
}

/**
  * @brief  Low-level write of 16-bit pixel/data.
  */
void ILI9486_WriteData(uint16_t data)
{
    ILI9486_DATA_ADDR = data;
}

/**
  * @brief  Low-level read of 16-bit data from controller.
  */
static uint16_t ILI9486_ReadData(void)
{
    return ILI9486_DATA_ADDR;
}

/**
  * @brief  Read controller ID (register 0xD3).
  * @retval 24-bit ID, or 0 if read fails.
  */
uint32_t ILI9486_ReadID(void)
{
    ILI9486_WriteCmd(0xD3);
    (void)ILI9486_ReadData(); /* dummy read */
    uint8_t id1 = (uint8_t)ILI9486_ReadData();
    uint8_t id2 = (uint8_t)ILI9486_ReadData();
    uint8_t id3 = (uint8_t)ILI9486_ReadData();
    return ((uint32_t)id1 << 16U) | ((uint32_t)id2 << 8U) | id3;
}

/**
  * @brief  Read display status (register 0x09).
  */
uint32_t ILI9486_ReadStatus(void)
{
    ILI9486_WriteCmd(0x09);
    (void)ILI9486_ReadData(); /* dummy read */
    uint16_t s1 = ILI9486_ReadData();
    uint16_t s2 = ILI9486_ReadData();
    uint16_t s3 = ILI9486_ReadData();
    uint16_t s4 = ILI9486_ReadData();
    return ((uint32_t)s1 << 24U) | ((uint32_t)s2 << 16U) | ((uint32_t)s3 << 8U) | s4;
}

/**
  * @brief  Turn on LCD backlight and configure the control pin.
  *         Backlight is active high on PB0.
  */
static void ILI9486_PowerOn(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LCD_BG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BG_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Hardware reset of the display controller.
  *         Reset line is active low on PG15.
  */
static void ILI9486_HardwareReset(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LCD_RST_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
}

/**
  * @brief  Software reset of the display controller.
  */
void ILI9486_Reset(void)
{
    ILI9486_WriteCmd(ILI9486_SWRESET);
    HAL_Delay(120);
}

/**
  * @brief  Initialize ILI9486 for 16-bit 8080 interface.
  */
void ILI9486_Init(void)
{
    ILI9486_PowerOn();
    ILI9486_HardwareReset();
    ILI9486_Reset();

    /* Sleep out (0x11) */
    ILI9486_WriteCmd(ILI9486_SLPOUT);
    HAL_Delay(20);

    /* Interface pixel format (0x3A): 16 bits/pixel */
    ILI9486_WriteCmd(ILI9486_PIXFMT);
    ILI9486_WriteData(0x55);
    HAL_Delay(10);

    /* Power control 3 (0xC2) for normal mode */
    ILI9486_WriteCmd(0xC2);
    ILI9486_WriteData(0x44);

    /* VCOM control (0xC5) */
    ILI9486_WriteCmd(0xC5);
    ILI9486_WriteData(0x00);
    ILI9486_WriteData(0x00);
    ILI9486_WriteData(0x00);
    ILI9486_WriteData(0x00);

    /* Positive gamma control (0xE0) */
    ILI9486_WriteCmd(0xE0);
    ILI9486_WriteData(0x0F);
    ILI9486_WriteData(0x1F);
    ILI9486_WriteData(0x1C);
    ILI9486_WriteData(0x0C);
    ILI9486_WriteData(0x0F);
    ILI9486_WriteData(0x08);
    ILI9486_WriteData(0x48);
    ILI9486_WriteData(0x98);
    ILI9486_WriteData(0x37);
    ILI9486_WriteData(0x0A);
    ILI9486_WriteData(0x13);
    ILI9486_WriteData(0x04);
    ILI9486_WriteData(0x11);
    ILI9486_WriteData(0x0D);
    ILI9486_WriteData(0x00);

    /* Negative gamma control (0xE1) */
    ILI9486_WriteCmd(0xE1);
    ILI9486_WriteData(0x0F);
    ILI9486_WriteData(0x32);
    ILI9486_WriteData(0x2E);
    ILI9486_WriteData(0x0B);
    ILI9486_WriteData(0x0D);
    ILI9486_WriteData(0x05);
    ILI9486_WriteData(0x47);
    ILI9486_WriteData(0x75);
    ILI9486_WriteData(0x37);
    ILI9486_WriteData(0x06);
    ILI9486_WriteData(0x10);
    ILI9486_WriteData(0x03);
    ILI9486_WriteData(0x24);
    ILI9486_WriteData(0x20);
    ILI9486_WriteData(0x00);

    /* Display inversion control (0xB4) */
    ILI9486_WriteCmd(0xB4);
    ILI9486_WriteData(0x02);

    /* Display function control (0xB6) */
    ILI9486_WriteCmd(0xB6);
    ILI9486_WriteData(0x02);
    ILI9486_WriteData(0x02);
    ILI9486_WriteData(0x3B);

    /* Entry mode set (0xB7) */
    ILI9486_WriteCmd(0xB7);
    ILI9486_WriteData(0x06);

    /* Memory access control (0x36): RGB order, landscape */
    ILI9486_SetRotation(0);

    /* Display on (0x29) */
    ILI9486_WriteCmd(ILI9486_DISPON);
    HAL_Delay(10);
}

/**
  * @brief  Set the GRAM address window for subsequent pixel writes.
  */
void ILI9486_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    ILI9486_WriteCmd(ILI9486_CASET);
    ILI9486_WriteData(x0 >> 8);
    ILI9486_WriteData(x0 & 0xFF);
    ILI9486_WriteData(x1 >> 8);
    ILI9486_WriteData(x1 & 0xFF);

    ILI9486_WriteCmd(ILI9486_PASET);
    ILI9486_WriteData(y0 >> 8);
    ILI9486_WriteData(y0 & 0xFF);
    ILI9486_WriteData(y1 >> 8);
    ILI9486_WriteData(y1 & 0xFF);

    ILI9486_WriteCmd(ILI9486_RAMWR);
}

/**
  * @brief  Draw a single pixel.
  */
void ILI9486_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ILI9486_WIDTH || y >= ILI9486_HEIGHT)
    {
        return;
    }

    ILI9486_SetAddressWindow(x, y, x, y);
    ILI9486_DATA_ADDR = color;
}

/**
  * @brief  Fill a rectangle with a solid color.
  */
void ILI9486_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t pixels;
    uint16_t x1, y1;

    if ((x >= ILI9486_WIDTH) || (y >= ILI9486_HEIGHT))
    {
        return;
    }

    x1 = x + w - 1;
    y1 = y + h - 1;
    if (x1 >= ILI9486_WIDTH)
    {
        x1 = ILI9486_WIDTH - 1;
    }
    if (y1 >= ILI9486_HEIGHT)
    {
        y1 = ILI9486_HEIGHT - 1;
    }

    ILI9486_SetAddressWindow(x, y, x1, y1);

    pixels = (uint32_t)(x1 - x + 1) * (uint32_t)(y1 - y + 1);
    for (uint32_t i = 0; i < pixels; i++)
    {
        ILI9486_DATA_ADDR = color;
    }
}

/**
  * @brief  Fill the entire screen.
  */
void ILI9486_FillScreen(uint16_t color)
{
    ILI9486_FillRect(0, 0, ILI9486_WIDTH, ILI9486_HEIGHT, color);
}

/**
  * @brief  Draw horizontal line.
  */
void ILI9486_DrawHorizontalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((y >= ILI9486_HEIGHT) || (x >= ILI9486_WIDTH))
    {
        return;
    }
    if ((x + len - 1) >= ILI9486_WIDTH)
    {
        len = ILI9486_WIDTH - x;
    }
    ILI9486_FillRect(x, y, len, 1, color);
}

/**
  * @brief  Draw vertical line.
  */
void ILI9486_DrawVerticalLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((x >= ILI9486_WIDTH) || (y >= ILI9486_HEIGHT))
    {
        return;
    }
    if ((y + len - 1) >= ILI9486_HEIGHT)
    {
        len = ILI9486_HEIGHT - y;
    }
    ILI9486_FillRect(x, y, 1, len, color);
}

/**
  * @brief  Invert display colors.
  */
void ILI9486_InvertDisplay(bool invert)
{
    ILI9486_WriteCmd(invert ? ILI9486_INVON : ILI9486_INVOFF);
}

/**
  * @brief  Set display orientation.
  * @param  rotation 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
  */
void ILI9486_SetRotation(uint8_t rotation)
{
    uint8_t madctl = ILI9486_MADCTL_BGR | ILI9486_MADCTL_MX;

    switch (rotation & 0x03U)
    {
        case 0:
            madctl = ILI9486_MADCTL_BGR | ILI9486_MADCTL_MX;
            break;
        case 1:
            madctl = ILI9486_MADCTL_BGR | ILI9486_MADCTL_MV;
            break;
        case 2:
            madctl = ILI9486_MADCTL_BGR | ILI9486_MADCTL_MY;
            break;
        case 3:
            madctl = ILI9486_MADCTL_BGR | ILI9486_MADCTL_MX |
                     ILI9486_MADCTL_MY | ILI9486_MADCTL_MV;
            break;
        default:
            break;
    }

    ILI9486_WriteCmd(ILI9486_MADCTL);
    ILI9486_WriteData(madctl);
}
