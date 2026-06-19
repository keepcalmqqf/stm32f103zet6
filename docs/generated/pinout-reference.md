# 生成引脚参考

> ⚠️ 本文件由 `HARDWARE_PINOUT.md` 派生，用于快速查阅。若硬件设计变更，请同步更新源文件。

---

## 基本信息

| 项目 | 说明 |
|------|------|
| MCU | STM32F103ZET6 |
| 主晶振 | 8 MHz HSE |
| 系统时钟 | 72 MHz |
| 调试 | SWD（PA13/PA14） |

## 关键外设引脚

| 功能 | 引脚 | 备注 |
|------|------|------|
| LED1 | PA0 | 低电平点亮 |
| LED2 | PA1 | 低电平点亮 |
| LED3 | PA8 | 低电平点亮 |
| LED0 | PG12 | 与 FSMC_NE4 冲突，当前固件未使用 |
| LCD CS | PG12 | FSMC_NE4 |
| LCD RS | PG0 | FSMC_A10 |
| LCD WR | PD5 | FSMC_NWE |
| LCD RD | PD4 | FSMC_NOE |
| LCD RST | PG15 | 低电平复位 |
| LCD BG | PB0 | 高电平打开背光 |
| USART1 TX | PA9 | 调试日志 |
| USART1 RX | PA10 | 调试日志 |
| USART2 TX | PA2 | 连接 ESP32-C3 |
| USART2 RX | PA3 | 连接 ESP32-C3 |
| W5500 CS | PB12 | SPI2_NSS |
| W5500 RST | PD3 | 复位 |
| W5500 INT | PG6 | 中断 |
| LoRa CS | PG14 | 片选 |
| LoRa RST | PG13 | 复位 |
| EEPROM SCL | PB10 | I2C2 |
| EEPROM SDA | PB11 | I2C2 |
| SPI Flash CS | PC13 | W25Q32 |
| NAND CE | PG9 | FSMC_NE2 |
| SDIO CK | PC12 | SD 卡时钟 |
| CAN RX | PB8 | — |
| CAN TX | PB9 | — |
| USB DP | PA12 | — |
| USB DM | PA11 | — |

完整引脚分配见 [`HARDWARE_PINOUT.md`](../../HARDWARE_PINOUT.md)。
