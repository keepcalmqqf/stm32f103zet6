# 固件能力清单

本文档描述当前 `stm32f103zet6` 固件已实现的功能与验收标准。

---

## 已验证能力

| ID | 能力 | 验收标准 | 相关文件 |
|----|------|----------|----------|
| CAP-001 | 系统启动与 72 MHz 时钟配置 | `SystemClock_Config()` 成功，LED1 在 `HAL_Init()` 前点亮 | `Core/Src/main.c` |
| CAP-002 | LED 轮询指示 | 主循环每秒切换 LED1/2/3，作为运行状态指示 | `Core/BSP/Src/led.c` |
| CAP-003 | USART1 调试输出 | 115200 8N1，`printf` 可输出到串口助手 | `Core/Src/usart.c` |
| CAP-004 | FSMC 初始化 | `MX_FSMC_Init()` 成功，FSMC Bank4 16-bit SRAM 模式 | `Core/Src/fsmc.c` |
| CAP-005 | ILI9486 LCD 初始化与刷屏 | LCD 依次显示蓝、红、绿、黑，读 ID 返回有效值 | `Core/BSP/Src/ili9486.c` |
| CAP-006 | RTC 实时时钟 | LSE 32.768 kHz 时钟，可读取/设置时、分、秒 | `Core/BSP/Src/rtc.c` |
| CAP-007 | LVGL 实时时间显示 | 屏幕居中显示绿色 `HH:MM:SS`，每秒刷新 | `Core/BSP/Src/lvgl_port_display.c`、`Core/Src/main.c` |
| CAP-013 | ESP32-C3 串口透传 | NTP 成功后 USART1（PA9/PA10）与 USART2（PA2/PA3）双向透明转发；PC 发送 `AT\r\n` 能收到 `OK` | `Core/App/Src/app_passthrough.c`、`Core/BSP/Src/esp_wifi.c` |

---

## 开发中 / 规划中能力

| ID | 能力 | 状态 | 备注 |
|----|------|------|------|
| CAP-008 | XPT2046 触摸屏驱动 | 📋 规划中 | 与 W5500 共用 SPI2，注意片选互斥 |
| CAP-009 | SD 卡文件系统 | 📋 规划中 | SDIO 引脚已分配 |
| CAP-010 | 按键输入处理 | 📋 规划中 | KEY1-4 引脚 PF8/PF9/PF10/PF11 |
| CAP-011 | W5500 以太网 | 📋 规划中 | SPI2，PB12-PB15/PD3/PG6 |
| CAP-012 | LoRa（LLCC68）| 📋 规划中 | SPI1，PG13-PG14/PE2/PE5/PE6 |
| CAP-014 | 蜂鸣器控制 | 📋 规划中 | PB1 |
| CAP-015 | 4-20 mA 采集 | 📋 规划中 | PF6/PF7 |
| CAP-016 | DS18B20 温度采集 | 📋 规划中 | PC3 |
| CAP-017 | SPI Flash（W25Q32）| 📋 规划中 | SPI1 PA4/PA5/PA6/PA7，PC13 片选 |
| CAP-018 | EEPROM（M24C02）| 📋 规划中 | I2C2 PB10/PB11 |
| CAP-019 | NAND Flash（W29N01）| 📋 规划中 | FSMC NE2，8-bit 数据 |

---

## 验证方法

- 每次构建后烧录到目标板。
- 观察 LED 每秒切换一次。
- 观察 LCD 以绿色大字显示 `HH:MM:SS`，每秒刷新。
- 通过 USB-TTL 连接 PA9/PA10，检查串口日志中的 RTC 时间输出。
- NTP 成功后，通过 USB-TTL 连接 PA9/PA10 发送 `AT\r\n`，确认收到 ESP32-C3 返回的 `OK`。
