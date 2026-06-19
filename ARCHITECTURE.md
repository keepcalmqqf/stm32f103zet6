# 架构与数据流

本文档描述 `stm32f103zet6` 固件工程的整体架构、模块分层、数据流与扩展方式。

---

## 1. 总体架构

本项目是**裸机固件**，基于 STM32 HAL 库与 CMake 构建，运行在 STM32F103ZET6（Cortex-M3，72 MHz）上。

```text
┌─────────────────────────────────────────────┐
│  Application (App/)                         │
│  - app_system.c   : 系统初始化编排 / 主循环 │
│  - app_ui.c       : LVGL 界面与显示逻辑     │
│  - app_time_sync.c: WiFi + NTP + RTC 同步   │
│  - app_passthrough.c: USART1↔USART2 透传桥接 │
│  - app_config.h   : 应用配置与日志抽象      │
├─────────────────────────────────────────────┤
│  main.c                                     │
│  - 仅保留 HAL 初始化、时钟配置、错误处理    │
│  - 调用 App_System_Init() / App_System_Run()│
├─────────────────────────────────────────────┤
│  Board Support (BSP)                        │
│  - led.c / ili9486.c / rtc.c                │
│  - lvgl_port_display.c / esp_wifi.c         │
│  - 封装硬件细节，提供可移植 API              │
├─────────────────────────────────────────────┤
│  Middlewares                                │
│  - Middlewares/LVGL/  (GUI 库)              │
├─────────────────────────────────────────────┤
│  STM32CubeMX Generated Layer                │
│  - gpio.c / fsmc.c / usart.c (MX_*)         │
│  - 时钟、中断、MSP、启动文件、链接脚本       │
├─────────────────────────────────────────────┤
│  STM32 HAL / CMSIS                          │
│  - Drivers/STM32F1xx_HAL_Driver             │
│  - Drivers/CMSIS                            │
└─────────────────────────────────────────────┘
```

---

## 2. 目录结构

```text
stm32f103zet6/
├── Core/
│   ├── Inc/              # CubeMX 生成头文件
│   │   ├── main.h        # 公共宏、Error_Handler 声明
│   │   ├── gpio.h        # GPIO 初始化头文件
│   │   ├── fsmc.h        # FSMC 初始化头文件
│   │   ├── usart.h       # USART1 初始化头文件
│   │   └── stm32f1xx_hal_conf.h  # HAL 配置文件
│   ├── Src/              # CubeMX 生成源文件
│   │   ├── main.c        # 应用入口与主循环
│   │   ├── gpio.c        # GPIO 初始化
│   │   ├── fsmc.c        # FSMC 初始化
│   │   ├── usart.c       # USART1 初始化
│   │   ├── stm32f1xx_it.c
│   │   └── stm32f1xx_hal_msp.c
│   ├── BSP/              # 板级支持包：用户硬件驱动
│   │   ├── Inc/
│   │   │   ├── led.h
│   │   │   ├── ili9486.h
│   │   │   ├── rtc.h
│   │   │   └── lvgl_port_display.h
│   │   └── Src/
│   │       ├── led.c
│   │       ├── ili9486.c
│   │       ├── rtc.c
│   │       └── lvgl_port_display.c
│   └── App/              # 应用层：业务逻辑与系统编排
│       ├── Inc/
│       │   ├── app_config.h        # 应用配置 / 日志宏
│       │   ├── app_system.h        # 初始化与主循环入口
│       │   ├── app_time_sync.h     # 网络时间同步抽象
│       │   ├── app_passthrough.h   # 串口透传桥接抽象
│       │   ├── app_ui.h            # LVGL 界面抽象
│       │   └── app_led_screen.h    # LED 与屏幕颜色映射
│       └── Src/
│           ├── app_system.c
│           ├── app_time_sync.c
│           ├── app_passthrough.c
│           ├── app_ui.c
│           └── app_led_screen.c
├── Middlewares/
│   └── LVGL/             # LVGL v9 图形库
├── cmake/
│   ├── gcc-arm-none-eabi.cmake   # 默认工具链
│   ├── starm-clang.cmake         # 备用 Clang 工具链
│   └── stm32cubemx/
│       └── CMakeLists.txt        # CubeMX 生成源文件列表
├── Drivers/
│   ├── CMSIS/            # 内核与外设寄存器定义
│   └── STM32F1xx_HAL_Driver/   # HAL 库源码
├── docs/                 # 项目文档
├── AGENTS.md             # AI 编码助手指引
├── ARCHITECTURE.md       # 架构与数据流
├── HARDWARE_PINOUT.md    # 核心板引脚分配
├── lv_conf.h             # LVGL 配置文件
├── CMakeLists.txt        # 根构建文件
├── CMakePresets.json     # Debug/Release 预设
├── stm32f103zet6.ioc     # STM32CubeMX 配置源文件
├── startup_stm32f103xe.s # 启动汇编
└── STM32F103XX_FLASH.ld  # 链接脚本
```

### 关键文件速查

| 路径 | 说明 | 是否由 CubeMX 生成 |
|------|------|-------------------|
| `stm32f103zet6.ioc` | STM32CubeMX 配置源文件，修改后必须重新生成代码 | 否（源文件） |
| `Core/Src/main.c` | 应用入口；仅调用 App_System_Init/Run 与错误处理 | 是 |
| `Core/Src/gpio.c` / `Inc/gpio.h` | GPIO 初始化（LED1/2/3 等） | 是 |
| `Core/Src/fsmc.c` / `Inc/fsmc.h` | FSMC 初始化，用于 16 位 8080 并行 LCD | 是 |
| `Core/Src/usart.c` / `Inc/usart.h` | USART1/USART2 初始化（`MX_*` 部分） | 是 |
| `Core/BSP/Src/esp_wifi.c` / `Inc/esp_wifi.h` | ESP32-C3 AT 命令驱动（WiFi / NTP） | 否 |
| `Core/App/Src/app_system.c` / `Inc/app_system.h` | 应用初始化编排与主循环 | 否 |
| `Core/App/Src/app_ui.c` / `Inc/app_ui.h` | LVGL 界面与开机画面 | 否 |
| `Core/App/Src/app_time_sync.c` / `Inc/app_time_sync.h` | WiFi + NTP → RTC 同步 | 否 |
| `Core/App/Src/app_passthrough.c` / `Inc/app_passthrough.h` | USART1↔USART2 透传桥接 | 否 |
| `Core/App/Inc/app_config.h` | WiFi 凭证、NTP 服务器、时区、日志宏、透传配置 | 否 |
| `Core/BSP/Src/led.c` / `Inc/led.h` | 用户 LED 抽象层 | 否 |
| `Core/BSP/Src/ili9486.c` / `Inc/ili9486.h` | ILI9486 LCD 驱动 | 否 |
| `Core/BSP/Src/rtc.c` / `Inc/rtc.h` | 实时时钟驱动（LSE 32.768 kHz） | 否 |
| `Core/BSP/Src/lvgl_port_display.c` / `Inc/lvgl_port_display.h` | LVGL 显示端口（ILI9486 flush_cb） | 否 |
| `Core/App/Src/app_led_screen.c` / `Inc/app_led_screen.h` | LED 与屏幕颜色映射业务逻辑 | 否 |
| `Middlewares/LVGL/` | LVGL v9.2 图形库 | 否（第三方库） |
| `lv_conf.h` | LVGL 配置文件 | 否 |
| `cmake/stm32cubemx/CMakeLists.txt` | CubeMX 生成的源文件列表 | 是（禁止手动编辑） |
| `CMakeLists.txt` | 根构建文件，添加用户源码/宏/头文件路径的位置 | 否 |
| `startup_stm32f103xe.s` | 启动汇编 | 是 |
| `STM32F103XX_FLASH.ld` | 链接脚本 | 是 |
| `HARDWARE_PINOUT.md` | 核心板完整引脚分配与外设说明 | 否 |
| `scripts/configure_fsmc_ioc.py` | 给 .ioc 注入 FSMC 配置的辅助脚本 | 否 |
| `scripts/cubemx_generate.script` | STM32CubeMX 命令行生成脚本 | 否 |
| `scripts/README.md` | 脚本目录说明 | 否 |

---

## 3. 启动流程

```text
复位向量
    │
    ▼
startup_stm32f103xe.s
    │   初始化 SP、.data/.bss、调用 SystemInit
    ▼
main()
    │
    ├── LED_EarlyInit()           # 早期 LED1 点亮，确认 MCU 已启动
    ├── HAL_Init()
    ├── SystemClock_Config()      # 72 MHz HSE + PLL
    └── App_System_Init()
            │
            ├── MX_GPIO_Init()            # LED 引脚
            ├── MX_FSMC_Init()            # FSMC Bank4 16-bit SRAM 模式
            ├── MX_USART1_UART_Init()     # 调试串口 115200 8N1
            ├── MX_USART2_UART_Init()     # ESP32-C3 AT 串口
            ├── LED_Init() / USART1_Init()
            ├── ILI9486_Init()            # LCD 初始化与诊断
            ├── RTC_BspInit()             # 启动 RTC
            ├── App_TimeSync_SyncFromNetwork()  # WiFi + NTP → RTC
            ├── App_UI_Init()             # LVGL + 时间标签
            └── App_Passthrough_Start()   # 启用 USART1/2 RX 中断，进入透传
    │
    └── App_System_Run() 主循环
            ├── App_TimeSync_GetRtcDateTime() 读取 RTC
            ├── App_UI_UpdateClock() 更新时间标签
            ├── App_UI_Handler() 刷新 LVGL
            ├── App_Passthrough_Run()     # 双向字节转发
            └── LED_ToggleNext()
```

---

## 4. 模块说明

### 4.1 LED 抽象层 (`Core/BSP/Src/led.c` / `Core/BSP/Inc/led.h`)

- 封装硬件极性：LED 为**低电平点亮**。
- 提供 `LED_On/Off/AllOff/ToggleNext` 等 API。
- 引脚映射来自 CubeMX 生成的 `main.h`（`LED1_Pin`、`LED2_Pin`、`LED3_Pin`）。

### 4.2 USART1 调试输出 (`Core/Src/usart.c` / `Core/Inc/usart.h`)

- 配置：115200 8N1，TX=PA9，RX=PA10。
- `printf` 通过 `__io_putchar` 重定向到 USART1，方便无调试器时排查问题。
- 避免在 USART 中断服务程序中调用 `printf`。

### 4.3 ILI9486 LCD 驱动 (`Core/BSP/Src/ili9486.c` / `Core/BSP/Inc/ili9486.h`)

- 通过 **FSMC 16 位 8080 并行总线** 访问 ILI9486 控制器。
- 控制器目标：Z350IT002 3.5 寸屏，分辨率 320×480。
- 地址映射：
  - 命令口：`0x6C000000`（FSMC Bank1 NE4，A10 = 0）
  - 数据口：`0x6C000800`（A10 = 1，16 位总线偏移 `1 << 11`）
- 提供初始化、复位、读 ID、读状态、填色、画点、画线、设置旋转等 API。
- 颜色格式：RGB565。

### 4.4 FSMC 配置 (`Core/Src/fsmc.c` / `Core/Inc/fsmc.h`)

- 使用 **FSMC Bank4（NE4）**，16 位数据宽度，SRAM 模式。
- 地址/数据线按核心板原理图连接（详见 `HARDWARE_PINOUT.md`）。
- 读写时序分开配置（`ExtendedMode = ENABLE`），适配 LCD 8080 时序。
- `__HAL_AFIO_FSMCNADV_DISCONNECTED()` 断开 NADV，避免干扰。

### 4.5 RTC 实时时钟 (`Core/BSP/Src/rtc.c` / `Core/BSP/Inc/rtc.h`)

- 使用外部 32.768 kHz LSE 作为 RTC 时钟源。
- 首次上电以固件编译时间作为初始时间，VBAT 供电时可保持走时。
- 提供 `RTC_GetTime()` / `RTC_SetTime()` / `RTC_GetDate()` / `RTC_SetDate()` API。

### 4.6 ESP32-C3 AT 驱动 (`Core/BSP/Src/esp_wifi.c` / `Core/BSP/Inc/esp_wifi.h`)

- 通过 USART2（PA2/PA3，115200 8N1）与 ESP32-C3 AT 固件通信。
- `ESP_WiFiInit()`：连接指定 SSID 的 WiFi AP。
- `ESP_SyncNtpTime()`：配置 SNTP 并解析 `+CIPSNTPTIME:` 响应。
- 不依赖 `printf`，避免 AT 流量与调试串口冲突。

### 4.7 LVGL 显示端口 (`Core/BSP/Src/lvgl_port_display.c` / `Core/BSP/Inc/lvgl_port_display.h`)

- 把 ILI9486 封装为 LVGL v9 的显示驱动。
- 实现 `flush_cb`，通过 `ILI9486_SetAddressWindow()` + FSMC 数据口写入局部刷新像素。
- 使用 partial 渲染模式，单缓冲约 7.5 KB。

### 4.8 应用系统编排 (`Core/App/Src/app_system.c` / `Core/App/Inc/app_system.h`)

- 负责调用 CubeMX 生成的 `MX_*_Init()` 与用户 BSP 初始化。
- 运行主循环，协调 UI 刷新、RTC 读取、LED 指示。
- `main.c` 不直接处理业务细节，只保留 HAL/时钟/错误处理。

### 4.9 应用 UI (`Core/App/Src/app_ui.c` / `Core/App/Inc/app_ui.h`)

- 封装 LVGL 控件细节，对外提供 `App_UI_Init()` 与 `App_UI_UpdateClock()`。
- 开机画面（蓝/红/绿/黑）也封装在此，避免 `main.c` 直接操作 LCD。

### 4.10 网络时间同步 (`Core/App/Src/app_time_sync.c` / `Core/App/Inc/app_time_sync.h`)

- 封装"WiFi 连接 + NTP 查询 + RTC 写入"完整流程。
- 配置来自 `app_config.h`（SSID、密码、NTP 服务器、时区）。
- 同步失败时不会阻塞系统，继续使用 RTC 时间。

### 4.11 串口透传桥接 (`Core/App/Src/app_passthrough.c` / `Core/App/Inc/app_passthrough.h`)

- NTP 成功后，把 USART1（PA9/PA10）与 USART2（PA2/PA3）做字节级透明转发。
- 使用两个 ring buffer 与 USART1/2 RX 中断实现全双工转发。
- 控制 ESP32-C3 的 PE4 使能脚（高电平有效），并在 AT 命令阶段前做一次硬件复位。
- 透传激活后，`__io_putchar` 静默，避免日志字节污染 AT 数据流。

### 4.12 应用配置 (`Core/App/Inc/app_config.h`)

- 集中管理可调参数：WiFi 凭证、NTP 服务器、时区、刷新周期。
- 提供 `APP_LOG_*` 宏作为日志抽象，默认通过 USART1 输出。

---

## 5. 数据流示例

### 5.1 LVGL 刷新时间标签

```text
App_System_Run(): 每秒触发
    │
    ▼
app_time_sync.c: App_TimeSync_GetRtcDateTime()
    │
    ▼
rtc.c: RTC_GetTime() / RTC_GetDate()
    │
    ▼
app_ui.c: App_UI_UpdateClock(year, month, day, hour, minute, second)
    │
    ▼
lvgl: 在 draw_buf 中渲染文字像素
    │
    ▼
lvgl_port_display.c: flush_cb(area, px_map)
    │
    ▼
ili9486.c: ILI9486_SetAddressWindow(x0, y0, x1, y1)
    │
    ▼
FSMC 数据口 0x6C000800 → 写入 RGB565 像素
    │
    ▼
LCD 8080 接口显示更新
```

### 5.2 调试日志输出

```text
App: APP_LOG_INFO("...")
    │
    ▼
app_config.h: printf("[INFO] ...\r\n")
    │
    ▼
newlib: _write / __io_putchar
    │
    ▼
usart.c: HAL_UART_Transmit(&huart1, ...)
    │
    ▼
USART1 TX → PA9 → USB-TTL 串口工具
```

### 5.3 NTP 时间同步

```text
App_System_Init()
    │
    ▼
app_time_sync.c: App_TimeSync_SyncFromNetwork()
    │
    ├── esp_wifi.c: ESP_WiFiInit(ssid, password)
    │       │
    │       ▼
    │   USART2 TX → PA2 → ESP32-C3 GPIO6 (U1RX)
    │   USART2 RX ← PA3 ← ESP32-C3 GPIO7 (U1TX)
    │
    └── esp_wifi.c: ESP_SyncNtpTime(timezone, ntp1, ntp2, &dt)
            │
            ▼
        app_time_sync.c: RTC_SetTime() / RTC_SetDate()
            │
            ▼
        rtc.c: 写入 STM32 RTC 计数器
```

### 5.4 串口透传

```text
PC 串口助手
    │
    ▼
USART1 RX 中断 (PA10)
    │
    ▼
app_passthrough.c: ringbuf (U1→U2)
    │
    ▼
App_Passthrough_Run()
    │
    ▼
USART2 TX (PA2) → ESP32-C3 GPIO6 (U1RX)

ESP32-C3 GPIO7 (U1TX) → USART2 RX (PA3)
    │
    ▼
app_passthrough.c: ringbuf (U2→U1)
    │
    ▼
App_Passthrough_Run()
    │
    ▼
USART1 TX (PA9) → PC 串口助手
```

---

## 6. 代码组织原则

1. **生成代码与用户代码分离**
   - `Core/Src/main.c`、`gpio.c`、`fsmc.c` 等由 STM32CubeMX 生成。
   - 用户逻辑必须放在 `/* USER CODE BEGIN/END ... */` 保护块内，否则重新生成会被覆盖。

2. **新增模块按分层放置**
   - 硬件驱动（BSP）放 `Core/BSP/Src/` 与 `Core/BSP/Inc/`。
   - 应用逻辑放 `Core/App/Src/` 与 `Core/App/Inc/`。
   - 配置与日志抽象放 `Core/App/Inc/`（如 `app_config.h`）。
   - 第三方库放 `Middlewares/`。
   - 在根 `CMakeLists.txt` 的 `target_sources(...)` 与 `target_include_directories(...)` 中注册。

3. **BSP 层封装硬件差异**
   - 应用代码不直接操作 GPIO 端口/引脚，而是通过 `led.h`、`ili9486.h` 等抽象接口。
   - AT 模块等外部器件驱动也放在 BSP 层（如 `esp_wifi.c`），应用层通过简单 API 调用。

3. **应用层再拆分**
   - `app_system.c` 只做系统编排，不处理具体业务。
   - `app_ui.c` 只处理界面，`app_time_sync.c` 只处理网络时间同步。
   - `app_config.h` 集中管理可配置参数，避免硬编码散落在各文件。

4. **避免在初始化阶段做阻塞性业务逻辑**
   - 初始化应快速完成，进入主循环后再执行周期性任务。

5. **配置与日志应集中抽象**
   - 业务常量（WiFi 凭证、服务器地址、时区）集中在 `app_config.h`。
   - 日志通过 `APP_LOG_*` 宏输出，便于统一关闭或切换输出通道。

6. **禁止在中断里调用 HAL_Delay / printf**
   - 中断服务程序保持短小，必要时设置标志位，在主循环处理。

---

## 7. 扩展新外设的标准流程

1. 在 `stm32f103zet6.ioc` 中配置外设与引脚。
2. 用 STM32CubeMX 重新生成代码（`STM32CubeMX -s scripts/cubemx_generate.script`）。
3. 在 `Core/BSP/Inc/` 与 `Core/BSP/Src/` 创建新的 BSP 驱动文件。
4. 在根 `CMakeLists.txt` 的 `target_sources(...)` 中添加新 `.c`。
5. 在 `app_system.c` 的 `App_System_Init()` 中调用新的初始化函数；`main.c` 保持精简，只负责 HAL/时钟/错误处理。
6. 构建并验证：

   ```bash
   cmake --build --preset Debug
   arm-none-eabi-size build/Debug/stm32f103zet6.elf
   ```

---

## 8. 常见问题

| 现象 | 可能原因 | 排查方向 |
|------|----------|----------|
| LCD 全黑 | PB0 背光未拉高 | 确认 `HAL_GPIO_WritePin(LCD_BG_GPIO_Port, LCD_BG_Pin, GPIO_PIN_SET)` |
| LCD 初始化失败 | FSMC 时序或地址错误 | 核对 `HARDWARE_PINOUT.md` 中的 NE4/A10 映射 |
| 串口无输出 | USART1 未初始化或波特率不匹配 | 确认 `MX_USART1_UART_Init()` 已调用，波特率 115200 |
| 程序卡死 | `Error_Handler()` 被触发 | 检查 HAL 初始化返回值与时钟配置；LED1 闪烁次数对应 `g_fatal_error` |
| NTP 时间未同步 | WiFi 未连接或 NTP 服务器不可达 | 查看 USART1 日志；检查 `app_config.h` 中 SSID/密码/服务器 |
| ESP32-C3 AT 无响应 / 乱码 | USART2 RX 中断与 `HAL_UART_Receive` 轮询冲突，或波特率不匹配 | 确认 `MX_USART2_UART_Init()` 波特率为 115200；RX 中断只在 NTP 成功后启用 |
| 透传时 USART1 无输出 | 透传激活后 `printf` 已静默 | 正常现象；如需日志，临时关闭 `APP_PASSTHROUGH_ENABLED` |
| CubeMX 重新生成后 FSMC 死机 | `HAL_NOR_MODULE_ENABLED` 被启用 | 改回 `HAL_SRAM_MODULE_ENABLED`，见 `Core/Inc/stm32f1xx_hal_conf.h` |

---

## 相关文档

- [设计原则](./docs/design-docs/core-beliefs.md)
- [硬件引脚分配](./HARDWARE_PINOUT.md)
- [测试规范](./docs/TESTING.md)
- [可靠性要求](./docs/RELIABILITY.md)
- [HAL 快速参考](./docs/references/stm32f1xx-hal-reference-llms.txt)
- [工具链快速参考](./docs/references/arm-toolchain-reference-llms.txt)
