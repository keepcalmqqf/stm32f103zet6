# 架构与数据流

本文档描述 `stm32f103zet6` 固件工程的整体架构、模块分层、数据流与扩展方式。

---

## 1. 总体架构

本项目是**裸机固件**，基于 STM32 HAL 库与 CMake 构建，运行在 STM32F103ZET6（Cortex-M3，72 MHz）上。

```text
┌─────────────────────────────────────────────┐
│  Application (main.c)                       │
│  - 初始化流程：HAL → 时钟 → GPIO/FSMC/USART │
│  - 业务循环：LED 轮询 + 日志输出             │
├─────────────────────────────────────────────┤
│  Board Support (BSP)                        │
│  - led.c / ili9486.c / usart.c              │
│  - 封装硬件细节，提供可移植 API              │
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
│   ├── Inc/              # 用户头文件
│   │   ├── main.h        # 公共宏、Error_Handler 声明
│   │   ├── led.h         # LED 抽象层
│   │   ├── usart.h       # USART1 驱动
│   │   └── ili9486.h     # ILI9486 LCD 驱动
│   └── Src/              # 用户源文件
│       ├── main.c        # 应用入口与主循环
│       ├── led.c         # LED 驱动实现
│       ├── usart.c       # USART1 + printf 重定向
│       └── ili9486.c     # FSMC 8080 并口 LCD 驱动
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
├── HARDWARE_PINOUT.md    # 核心板引脚分配
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
| `Core/Src/main.c` | 应用入口与主循环 | 是 |
| `Core/Src/gpio.c` / `Inc/gpio.h` | GPIO 初始化（LED1/2/3 等） | 是 |
| `Core/Src/fsmc.c` / `Inc/fsmc.h` | FSMC 初始化，用于 16 位 8080 并行 LCD | 是 |
| `Core/Src/usart.c` / `Inc/usart.h` | USART1 初始化（`MX_*` 部分） | 是 |
| `Core/Src/led.c` / `Inc/led.h` | 用户 LED 抽象层 | 否 |
| `Core/Src/ili9486.c` / `Inc/ili9486.h` | ILI9486 LCD 驱动 | 否 |
| `cmake/stm32cubemx/CMakeLists.txt` | CubeMX 生成的源文件列表 | 是（禁止手动编辑） |
| `CMakeLists.txt` | 根构建文件，添加用户源码/宏/头文件路径的位置 | 否 |
| `startup_stm32f103xe.s` | 启动汇编 | 是 |
| `STM32F103XX_FLASH.ld` | 链接脚本 | 是 |
| `HARDWARE_PINOUT.md` | 核心板完整引脚分配与外设说明 | 否 |
| `configure_fsmc_ioc.py` | 给 .ioc 注入 FSMC 配置的辅助脚本 | 否 |
| `cubemx_generate.script` | STM32CubeMX 命令行生成脚本 | 否 |

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
    ├── 早期 GPIO 测试（LED1 点亮，确认 MCU 已启动）
    ├── HAL_Init()
    ├── SystemClock_Config()      # 72 MHz HSE + PLL
    ├── MX_GPIO_Init()            # LED 引脚
    ├── MX_FSMC_Init()            # FSMC Bank4 16-bit SRAM 模式
    ├── MX_USART1_UART_Init()     # 115200 8N1
    ├── 用户外设初始化
    │   ├── LCD 背光 PB0、复位 PG15
    │   ├── LED_Init()
    │   ├── USART1_Init()
    │   └── ILI9486_Init()
    │
    └── while (1) 主循环
            ├── LED_ToggleNext()
            ├── printf 日志
            └── HAL_Delay(5000)
```

---

## 4. 模块说明

### 4.1 LED 抽象层 (`led.c` / `led.h`)

- 封装硬件极性：LED 为**低电平点亮**。
- 提供 `LED_On/Off/AllOff/ToggleNext` 等 API。
- 引脚映射来自 CubeMX 生成的 `main.h`（`LED1_Pin`、`LED2_Pin`、`LED3_Pin`）。

### 4.2 USART1 调试输出 (`usart.c` / `usart.h`)

- 配置：115200 8N1，TX=PA9，RX=PA10。
- `printf` 通过 `__io_putchar` 重定向到 USART1，方便无调试器时排查问题。
- 避免在 USART 中断服务程序中调用 `printf`。

### 4.3 ILI9486 LCD 驱动 (`ili9486.c` / `ili9486.h`)

- 通过 **FSMC 16 位 8080 并行总线** 访问 ILI9486 控制器。
- 控制器目标：Z350IT002 3.5 寸屏，分辨率 320×480。
- 地址映射：
  - 命令口：`0x6C000000`（FSMC Bank1 NE4，A10 = 0）
  - 数据口：`0x6C000800`（A10 = 1，16 位总线偏移 `1 << 11`）
- 提供初始化、复位、读 ID、读状态、填色、画点、画线、设置旋转等 API。
- 颜色格式：RGB565。

### 4.4 FSMC 配置 (`fsmc.c` / `fsmc.h`)

- 使用 **FSMC Bank4（NE4）**，16 位数据宽度，SRAM 模式。
- 地址/数据线按核心板原理图连接（详见 `HARDWARE_PINOUT.md`）。
- 读写时序分开配置（`ExtendedMode = ENABLE`），适配 LCD 8080 时序。
- `__HAL_AFIO_FSMCNADV_DISCONNECTED()` 断开 NADV，避免干扰。

---

## 5. 数据流示例

### 5.1 LCD 写像素

```text
App: ILI9486_DrawPixel(x, y, color)
    │
    ▼
ili9486.c: ILI9486_SetAddressWindow(...)  →  写 0x2A/0x2B 设置窗口
    │
    ▼
ili9486.c: ILI9486_WriteCmd(0x2C)        →  命令口地址 0x6C000000
    │
    ▼
ili9486.c: ILI9486_WriteData(color)      →  数据口地址 0x6C000800
    │
    ▼
FSMC 外设 → GPIO D/E/G → LCD 8080 接口
```

### 5.2 调试日志输出

```text
App: printf("...\r\n")
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

---

## 6. 代码组织原则

1. **生成代码与用户代码分离**
   - `Core/Src/main.c`、`gpio.c`、`fsmc.c` 等由 STM32CubeMX 生成。
   - 用户逻辑必须放在 `/* USER CODE BEGIN/END ... */` 保护块内，否则重新生成会被覆盖。

2. **新增模块放 `Core/Src/` 与 `Core/Inc/`**
   - 并在根 `CMakeLists.txt` 的 `target_sources(...)` 中注册。

3. **BSP 层封装硬件差异**
   - 应用代码不直接操作 GPIO 端口/引脚，而是通过 `led.h` 等抽象接口。

4. **避免在初始化阶段做阻塞性业务逻辑**
   - 初始化应快速完成，进入主循环后再执行周期性任务。

5. **禁止在中断里调用 HAL_Delay / printf**
   - 中断服务程序保持短小，必要时设置标志位，在主循环处理。

---

## 7. 扩展新外设的标准流程

1. 在 `stm32f103zet6.ioc` 中配置外设与引脚。
2. 用 STM32CubeMX 重新生成代码（`STM32CubeMX -s cubemx_generate.script`）。
3. 在 `Core/Inc/` 与 `Core/Src/` 创建新的 BSP 驱动文件。
4. 在根 `CMakeLists.txt` 的 `target_sources(...)` 中添加新 `.c`。
5. 在 `main.c` 的 `USER CODE BEGIN 2` 区域调用初始化函数。
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
| 程序卡死 | `Error_Handler()` 被触发 | 检查 HAL 初始化返回值与时钟配置 |

---

## 相关文档

- [设计原则](./docs/design-docs/core-beliefs.md)
- [构建与发布流程](./RELEASE.md)
- [硬件引脚分配](./HARDWARE_PINOUT.md)
- [测试规范](./docs/TESTING.md)
- [可靠性要求](./docs/RELIABILITY.md)
- [HAL 快速参考](./docs/references/stm32f1xx-hal-reference-llms.txt)
- [工具链快速参考](./docs/references/arm-toolchain-reference-llms.txt)
