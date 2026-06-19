# STM32F103ZET6 核心板硬件接口说明

本文件根据 `STM32-F103ZET6开发板.pdf` 原理图整理，列出所有外设引脚分配、电源、存储与扩展接口，方便驱动开发与故障排查。

> **版本**：2026-06-19 修订版  
> **依据**：`STM32-F103ZET6开发板.pdf`（V1.0，2024-08-08）

---

## 1. 基本信息

| 项目 | 说明 |
|------|------|
| MCU | STM32F103ZET6（LQFP144，ARM Cortex-M3） |
| 主晶振 | 8 MHz HSE（OSC_IN/OSC_OUT = PD0/PD1） |
| 低速晶振 | 32.768 kHz LSE（PC14/PC15） |
| 系统时钟 | 72 MHz（HSE × 9 PLL） |
| 调试接口 | SWD（PA13/SWDIO、PA14/SWCLK），NRST 复位 |
| 供电 | DC 12 V → MT2492（5 V）→ AMS1117-3.3（3.3 V） |

### 电源树

```text
DC12V ──► MT2492 ──► 5V ──► AMS1117-3.3 ──► 3V3
              │                    │
            22uF                 10uF
            4.7uH                10uF
```

- `5V`：USB、蜂鸣器、部分接口
- `3V3`：MCU、存储器、外设

---

## 2. LED 指示灯

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| LED-1 | PA0 | 低电平点亮 |
| LED-2 | PA1 | 低电平点亮 |
| LED-3 | PA8 | 低电平点亮 |
| LED-0 | PG12（网络标签） | 原理图 U16.1 右侧网络标签将 PG12 标为 LED-0，但其 MCU 复用功能为 FSMC_NE4；当前固件未使用该 LED |

> ⚠️ **PG12 复用冲突**：原理图右侧网络标签将 PG12 标为 `LED-0`，但中间栏复用功能明确为 `FSMC_NE4`，且当前 LCD 驱动使用 PG12 作为片选。两者不能同时使用，当前固件优先使用 FSMC_NE4。

---

## 3. LCD 显示屏接口（3.5 寸 TFT / ILI9486）

采用 **16 位 8080 并行总线**，通过 FSMC 驱动。

### 3.1 FSMC 控制信号

| LCD 信号 | MCU 引脚 | FSMC 功能 | 说明 |
|----------|----------|-----------|------|
| CS | PG12 | FSMC_NE4 | 片选，低电平有效 |
| RS | PG0 | FSMC_A10 | 寄存器/数据选择 |
| WR | PD5 | FSMC_NWE | 写使能，低电平有效 |
| RD | PD4 | FSMC_NOE | 读使能，低电平有效 |

### 3.2 FSMC 数据总线（D0-D15）

| LCD 数据 | MCU 引脚 | LCD 数据 | MCU 引脚 |
|----------|----------|----------|----------|
| D0 | PD14 | D8 | PE11 |
| D1 | PD15 | D9 | PE12 |
| D2 | PD0 | D10 | PE13 |
| D3 | PD1 | D11 | PE14 |
| D4 | PE7 | D12 | PE15 |
| D5 | PE8 | D13 | PD8 |
| D6 | PE9 | D14 | PD9 |
| D7 | PE10 | D15 | PD10 |

### 3.3 LCD 辅助信号

| 功能 | MCU 引脚 | 说明 |
|------|----------|------|
| LCD-RST | PG15 | 硬件复位，低电平有效 |
| LCD-BG | PB0 | 背光控制，高电平打开 |

### 3.4 软件地址映射

- **命令口地址**：`0x6C000000`（FSMC Bank1 NE4，A10 = 0）
- **数据口地址**：`0x6C000800`（A10 = 1，16 位总线偏移为 `1 << 11`）

### 3.5 连接器 CN2 / HDR1 引脚

| 引脚 | 信号 | 引脚 | 信号 |
|------|------|------|------|
| 1 | FSMC-D0 | 2 | I2C1-SDA |
| 3 | FSMC-D1 | 4 | I2C1-SCL |
| 5 | FSMC-D2 | 6 | GT-INT |
| 7 | FSMC-D3 | 8 | LCD-BG |
| 9 | FSMC-D4 | 10 | LCD-RST |
| 11 | FSMC-D5 | 12 | FSMC-NE4 |
| 13 | FSMC-D6 | 14 | FSMC-A10 |
| 15 | FSMC-D7 | 16 | FSMC-NWE |
| 17 | FSMC-D8 | 18 | FSMC-NOE |
| 19 | FSMC-D9 | 20 | FSMC-D15 |
| 21 | FSMC-D10 | 22 | FSMC-D14 |
| 23 | FSMC-D11 | 24 | FSMC-D13 |
| 25 | FSMC-D12 | 26 | — |

---

## 4. 串口（UART/USART）

| 功能 | MCU 引脚 | 说明 |
|------|----------|------|
| UART1-TX | PA9 | USART1_TX，调试日志 |
| UART1-RX | PA10 | USART1_RX，调试日志 |
| UART2-TX | PA2 | USART2_TX，连接 ESP32-C3 U1RX |
| UART2-RX | PA3 | USART2_RX，连接 ESP32-C3 U1TX |
| UART5-TX | PC12 | UART5_TX |
| UART5-RX | PD2 | UART5_RX |

> 注：STM32F103 没有 UART6。PC6/PC7 在原理图中用于 SDIO_D6/D7，不是串口。

---

## 5. SPI 接口

### 5.1 SPI1（LoRa / 备用）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| SPI1-NSS | PA4 | 软件片选 |
| SPI1-SCK | PA5 | 时钟 |
| SPI1-MISO | PA6 | 主机输入 |
| SPI1-MOSI | PA7 | 主机输出 |

### 5.2 SPI2（W5500 以太网）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| W5500-CS | PB12 | SPI2_NSS |
| SPI2-SCK | PB13 | 时钟 |
| SPI2-MISO | PB14 | 主机输入 |
| SPI2-MOSI | PB15 | 主机输出 |
| W5500-RST | PD3 | 复位 |
| W5500-INT | PG6 | 中断 |

### 5.3 SPI3

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| SPI3-SCK | PB3 | 时钟 |
| SPI3-MISO | PB4 | 主机输入 |
| SPI3-MOSI | PB5 | 主机输出 |
| SPI3-NSS | PA15 | 软件片选 |

---

## 6. I2C 接口

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| I2C1-SDA | PB7 | 复用功能 I2C1_SDA |
| I2C1-SCL | PB6 | 复用功能 I2C1_SCL |
| I2C2-SDA | PB11 | 复用功能 I2C2_SDA，连接 EEPROM M24C02 |
| I2C2-SCL | PB10 | 复用功能 I2C2_SCL，连接 EEPROM M24C02 |

---

## 7. CAN 总线

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| CAN-RX | PB8 | CAN_RX |
| CAN-TX | PB9 | CAN_TX |

---

## 8. USB 接口

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| USB-DP | PA12 | USB_DP |
| USB-DN | PA11 | USB_DM |

---

## 9. SD 卡接口（SDIO）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| SDIO-CK | PC12 | SDIO_CLK |
| SDIO-CMD | PD2 | SDIO_CMD |
| SDIO-D0 | PC8 | SDIO_D0 |
| SDIO-D1 | PC9 | SDIO_D1 |
| SDIO-D2 | PC10 | SDIO_D2 |
| SDIO-D3 | PC11 | SDIO_D3 |

---

## 10. 存储器

### 10.1 SRAM（标称 SDRAM，实为异步 SRAM）

型号：IS62WV51216BLL-55TLI（512K × 16 bit）

| SRAM 信号 | MCU 引脚 | 说明 |
|-----------|----------|------|
| A0-A18 | PF0-PF5、PF12-PF15、PG0-PG5、PD11-PD13 | 地址总线 |
| D0-D15 | PD14-PD15、PD0-PD1、PE7-PE15、PD8-PD10 | 数据总线 |
| CS1# | PG10 | FSMC_NE3 |
| OE# | PD4 | FSMC_NOE |
| WE# | PD5 | FSMC_NWE |
| UB# | PE1 | FSMC_NBL1 |
| LB# | PE0 | FSMC_NBL0 |

> 注：SRAM 与 LCD 共用 FSMC 数据总线，片选不同（LCD 用 NE4，SRAM 用 NE3）。

### 10.2 EEPROM

型号：M24C02-WMN6TP（2 Kbit，I2C）

| EEPROM 信号 | MCU 引脚 | 说明 |
|-------------|----------|------|
| SCL | PB10 | I2C2_SCL |
| SDA | PB11 | I2C2_SDA |
| WC# | 3V3 | 写保护关闭 |

### 10.3 SPI Flash

型号：W25Q32JVSSIQ（32 Mbit，SPI）

| Flash 信号 | MCU 引脚 | 说明 |
|------------|----------|------|
| CS# | PC13 | W25Q-CS |
| DO / IO1 | PA6 | SPI1_MISO |
| DI / IO0 | PA7 | SPI1_MOSI |
| CLK | PA5 | SPI1_SCK |
| WP# / IO2 | 3V3 | 写保护关闭 |
| HOLD# / IO3 | 3V3 | 保持禁用 |

### 10.4 NAND Flash

型号：W29N01HVSINA（1 Gbit）

| NAND 信号 | MCU 引脚 | 说明 |
|-----------|----------|------|
| IO0-IO7 | PD14-PD15、PD0-PD1、PE7-PE10 | FSMC_D0-D7 |
| #CE | PG9 | FSMC_NE2 |
| #WE | PD5 | FSMC_NWE |
| #RE | PD4 | FSMC_NOE |
| CLE | PD12 | FSMC_A17 |
| ALE | PD11 | FSMC_A16 |
| RY/#BY | PD6 | FSMC_NWAIT |

---

## 11. 触摸屏（XPT2046 / 电阻屏）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| T_CLK | PB13 | SPI2_SCK |
| T_CS | PB12 | SPI2_NSS |
| T_MOSI | PB15 | SPI2_MOSI |
| T_MISO | PB14 | SPI2_MISO |
| T_PEN | PC5 | 触摸中断，低电平有效 |
| T_BUSY | PC4 | 忙信号 |

> 注：XPT2046 与 SPI2 共用，需软件控制 T_CS；与 W5500 共用 SPI2 时注意片选互斥。

---

## 12. 按键（KEY）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| KEY-1 | PF8 | 输入按键 |
| KEY-2 | PF9 | 输入按键 |
| KEY-3 | PF10 | 输入按键 |
| KEY-4 | PF11 | 输入按键 |

---

## 13. 蜂鸣器与 WS2812

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| BUZZER-EN | PB1 | 高电平使能蜂鸣器 |
| LED-0 / WS2812 DIN | PG11 | WS2812 单线数据 |

---

## 14. 4-20 mA 输入与温度传感器

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| 4-20MA-1 | PF6 | 经 150 Ω + 3.3 V 稳压管 → ADC3_IN4 |
| 4-20MA-2 | PF7 | 经 150 Ω + 3.3 V 稳压管 → ADC3_IN5 |
| 18B20-DATA | PC3 | 单总线温度传感器 |

---

## 15. 以太网（W5500）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| W5500-CS | PB12 | SPI2_NSS |
| SPI2-SCK | PB13 | 时钟 |
| SPI2-MISO | PB14 | 主机输入 |
| SPI2-MOSI | PB15 | 主机输出 |
| W5500-RST | PD3 | 复位 |
| W5500-INT | PG6 | 中断 |
| ACTLED | — | HR911105A 内置 |
| LINKLED | — | HR911105A 内置 |

---

## 16. LoRa 模块（LLCC68）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| LORA-CS | PG14 | 片选 |
| LORA-RST | PG13 | 复位 |
| LORA-BUSY | PE2 | 忙信号 |
| LORA-TXEN | PE6 | 发送使能 |
| LORA-RXEN | PE5 | 接收使能 |
| SPI1-SCK | PA5 | 时钟 |
| SPI1-MISO | PA6 | 主机输入 |
| SPI1-MOSI | PA7 | 主机输出 |

---

## 17. ESP32-C3（WiFi / BLE）

| 功能 | MCU 引脚 | 备注 |
|------|----------|------|
| ESP32-EN | PE4 | 使能 |
| UART2-TX | PA2 | STM32 TX → ESP32 U1RX |
| UART2-RX | PA3 | STM32 RX ← ESP32 U1TX |

---

## 18. 调试与程序下载接口（CN1）

| 引脚 | 信号 | 说明 |
|------|------|------|
| 1 | 3V3 | 电源 |
| 2 | — | — |
| 3 | — | — |
| 4 | — | — |
| 5 | SWDIO | PA13 |
| 6 | UART1-RX | PA10 |
| 7 | SWCLK | PA14 |
| 8 | UART1-TX | PA9 |
| ... | — | — |

---

## 19. 时钟与复位

| 功能 | MCU 引脚 | 说明 |
|------|----------|------|
| OSC_IN | PD0 | 8 MHz HSE 输入 |
| OSC_OUT | PD1 | 8 MHz HSE 输出 |
| PC14-OSC32_IN | PC14 | 32.768 kHz LSE 输入 |
| PC15-OSC32_OUT | PC15 | 32.768 kHz LSE 输出 |
| NRST | NRST | 外部复位，低电平有效 |
| BOOT0 | BOOT0 | 启动模式选择 |
| BOOT1 | PB2 | 启动模式选择 |

---

## 20. 驱动开发注意事项

1. **LCD 必须通过 FSMC Bank4 + A10 访问**，地址为 `0x6C000000` / `0x6C000800`。
2. **LCD 背光 `PB0` 必须置高**，否则屏幕全黑。
3. **LCD 复位 `PG15` 必须给低电平复位脉冲**（拉低 ≥1 ms，再拉高 ≥120 ms）。
4. **FSMC 建议使用 SRAM 模式**，不要用 NOR 模式，避免 NOR CFI 命令干扰 LCD。
5. **PA13/PA14 默认用于 SWD 调试**，不要随意复用为普通 GPIO。
6. **SPI2 被 W5500 与 XPT2046 共用**，使用时必须做好片选互斥。
7. **PD0/PD1 同时是 HSE 晶振引脚和 FSMC_D2/D3**，启用 HSE 后不能同时作为 GPIO/FSMC 数据脚使用。当前 `.ioc` 同时配置了 HSE 与 FSMC_D2/D3，存在硬件冲突，需要设计层面确认。

---

## 21. 原理图核对结论

根据 `STM32-F103ZET6开发板.pdf` MCU 页（U16.1）直接核对，得出以下结论：

### 已确认

- **PD0 = OSC_IN，PD1 = OSC_OUT**：原理图晶振 X2（8 MHz）连接至 MCU 引脚 23/24，即 PD0/PD1。
- **PD0/PD1 同时被标为 FSMC_D2/D3**：原理图 MCU 引脚标注 `PD0/FSMC_D2`、`PD1/FSMC_D3`，与晶振功能冲突。
- **PG12 被标为 LED-0**：原理图 U16.1 右侧 net label 将 PG12 标为 `LED-0`。

### 原理图标注错误

U16.1 MCU 符号中，**引脚旁的复用功能文字（中间栏）是正确的**，但**右侧网络标签存在多处错误**。应以中间栏复用功能为准，网络标签仅作参考。

| 右侧网络标签 | 中间栏复用功能 | 数据手册实际复用 | 结论 |
|-------------|--------------|-----------------|------|
| PG13 = FSMC-NE4 | PG13/FSMC_A24 | PG13 = FSMC_A24 | **网络标签错误** |
| PG12 = LED-0 | PG12/FSMC_NE4 | PG12 = FSMC_NE4 | **网络标签错误** |
| PG3 = FSMC-A10 | PG3/FSMC_A13 | PG3 = FSMC_A13 | **网络标签错误** |
| PG0 = FSMC-A8 | PG0/FSMC_A10 | PG0 = FSMC_A10 | **网络标签错误** |

STM32F103ZET6 的 FSMC 复用功能是固定的：
- **FSMC_NE4 只能出现在 PG12**
- **FSMC_A10 只能出现在 PG0**

### 设计冲突

1. **PG12 复用冲突**：
   - 原理图将 PG12 标为 `LED-0`。
   - 但 PG12 的 MCU 复用功能为 `FSMC_NE4`，当前 LCD 驱动也使用 PG12 作为片选。
   - **结论**：PG12 不能同时驱动 LED-0 和 LCD CS。当前固件优先使用 FSMC_NE4。

2. **PD0/PD1 复用冲突**：
   - 8 MHz HSE 晶振与 FSMC_D2/D3 不能同时使用。
   - 当前 `.ioc` 同时启用 HSE 并配置 FSMC_D2/D3，**存在硬件冲突**，必须选择：
     1. 关闭 HSE，改用 HSI，保留 16 位 FSMC；或
     2. 保留 HSE，放弃 FSMC_D2/D3（即无法使用 16 位 FSMC LCD）。

### 本文件采用映射

本文件及当前固件采用与 STM32 数据手册一致的映射：

- **FSMC_NE4 → PG12**
- **FSMC_A10 → PG0**

---

*文档生成时间：2026-06-19*  
*依据：`STM32-F103ZET6开发板.pdf`*
