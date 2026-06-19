# ESP32-C3 串口调试桥接设计

## 1. 背景与目标

当前固件已通过 USART2（PA2/PA3）驱动 ESP32-C3 AT 固件完成 WiFi 连接与 NTP 校时。为了方便在桌面上直接调试 ESP32-C3 的 AT 指令行为，需要新增一层“调试桥接”：把 PC 调试串口 USART1（PA9/PA10）与 ESP32-C3 串口 USART2 透明地双向转发。

**核心目标**：
- 系统启动后仍先完成现有 WiFi/NTP 初始化流程。
- 初始化完成后，USART1 与 USART2 之间做字节级透明转发。
- PC 串口助手连接 PA9/PA10 即可直接向 ESP32-C3 发送 AT 指令并读取返回。

## 2. 设计决策

| 决策项 | 选择 | 理由 |
|--------|------|------|
| 透传模式 | 调试桥接（USART1 ↔ USART2） | 主要用于调试 AT 固件，不进入 TCP/UDP 透传模式 |
| 与现有功能共存 | 启动先完成 WiFi/NTP，再进入透传 | 保留当前能力，透传作为初始化后的最终状态 |
| 退出机制 | 不支持退出 | 方案最简；如需恢复日志/命令模式，可后续扩展 |
| 转发机制 | 中断 + 环形缓冲区 | 115200 全双工可靠，不丢字节，CPU 占用低 |
| printf 处理 | 进入透传后关闭 USART1 日志输出 | 避免日志字节污染透传数据流 |
| PE4 使能脚 | 在初始化阶段拉高 | 产品规格中 PE4 为 ESP32-C3 使能，当前代码未控制，需要补上 |

## 3. 架构与模块

```
┌─────────────┐         ┌─────────────────────┐         ┌─────────────┐
│   PC 串口   │◄───────►│  USART1  (PA9/PA10) │◄───────►│             │
│  (串口助手) │         │   调试/透传端口     │         │             │
└─────────────┘         └─────────────────────┘         │   STM32F1   │
                                                        │             │
┌─────────────┐         ┌─────────────────────┐         │ app_passthrough
│ ESP32-C3 AT │◄───────►│  USART2  (PA2/PA3)  │◄───────►│             │
│   固件      │         │   ESP32 通信端口    │         │             │
└─────────────┘         └─────────────────────┘         └─────────────┘
                                    ▲
                                    │ PE4 使能
                                    ▼
                            ┌───────────────┐
                            │  GPIO PE4     │
                            │ ESP32 EN      │
                            └───────────────┘
```

新增模块：
- `Core/App/Inc/app_passthrough.h`
- `Core/App/Src/app_passthrough.c`

修改模块：
- `Core/App/Src/app_system.c`：在 NTP 同步成功后启动透传，主循环调用转发任务
- `Core/App/Inc/app_config.h`：新增透传相关配置宏
- `Core/Src/usart.c` / `Core/Inc/usart.h`：开启 USART1/2 RX 中断，提供中断入口
- `Core/Src/stm32f1xx_it.c`：转发 USART1/2 中断到 `app_passthrough`

## 4. 数据流

### 4.1 PC → ESP32-C3

```
USART1 RX 中断触发
        │
        ▼
[ringbuf_u1_to_u2] 写入
        │
        ▼
App_Passthrough_Run() 从缓冲区读出
        │
        ▼
HAL_UART_Transmit(&huart2, ...) 发送给 ESP32-C3
```

### 4.2 ESP32-C3 → PC

```
USART2 RX 中断触发
        │
        ▼
[ringbuf_u2_to_u1] 写入
        │
        ▼
App_Passthrough_Run() 从缓冲区读出
        │
        ▼
HAL_UART_Transmit(&huart1, ...) 发送给 PC
```

## 5. 接口设计

```c
/* app_passthrough.h */

/**
 * @brief 初始化透传模块的 ring buffer 与中断状态。
 * @note  应在 USART1/2 初始化之后调用，但此时还不启用透传。
 */
void App_Passthrough_Init(void);

/**
 * @brief 启用透传模式。
 * @note  调用后 USART1 不再输出日志，所有字节均转发到/来自 ESP32-C3。
 */
void App_Passthrough_Start(void);

/**
 * @brief 透传主任务，从 ring buffer 中取出数据并完成转发。
 * @note  应在主循环中持续调用。
 */
void App_Passthrough_Run(void);

/**
 * @brief USART1 RX 中断回调处理入口。
 */
void App_Passthrough_OnUsart1Rx(uint8_t byte);

/**
 * @brief USART2 RX 中断回调处理入口。
 */
void App_Passthrough_OnUsart2Rx(uint8_t byte);
```

## 6. 配置项

在 `app_config.h` 中新增：

```c
/* 透传开关：1=启用，0=保持现有行为 */
#define APP_PASSTHROUGH_ENABLED      1

/* 每个方向环形缓冲区大小（字节） */
#define APP_PASSTHROUGH_BUF_SIZE     256U

/* ESP32-C3 使能引脚 */
#define APP_ESP_EN_GPIO_PORT         GPIOE
#define APP_ESP_EN_GPIO_PIN          GPIO_PIN_4
```

## 7. 与现有系统集成

### 7.1 启动流程

```
App_System_Init()
├── App_System_InitPeripherals()     (初始化 GPIO/FSMC/USART1/USART2)
├── App_System_InitBoard()           (LED、USART1 日志)
├── App_System_LcdDiagnostics()      (LCD 初始化)
├── App_System_InitRtcAndSync()      (RTC + WiFi/NTP)
│   └── App_TimeSync_SyncFromNetwork()
│       └── ESP_WiFiInit() / ESP_SyncNtpTime()
└── App_Passthrough_Start()          <-- 新增：启用透传
```

### 7.2 主循环

```c
void App_System_Run(void)
{
    while (1)
    {
        /* 现有 LVGL 时钟刷新保留 */
        App_UI_Handler();

        /* 新增：透传转发任务 */
        App_Passthrough_Run();

        HAL_Delay(ui_delay_ms);
    }
}
```

### 7.3 printf 行为

进入透传后，调用 `APP_LOG_DISABLE` 或在 `__io_putchar` 中判断透传状态，禁止向 USART1 输出任何日志字节，避免破坏 AT 数据流。

## 8. PE4 使能脚控制

当前代码未控制 PE4。实现时会在 `app_passthrough.c` 的 `App_Passthrough_Init()` 中配置 PE4 为推挽输出并置高，确保 ESP32-C3 始终处于使能状态。若后续需要复位 ESP32，可扩展为 `App_Passthrough_ResetModule()`。

## 9. 测试与验收标准

1. 上电后 LCD 正常显示时间，说明 WiFi/NTP 初始化仍工作。
2. PC 串口助手连接 PA9/PA10，波特率 115200 8N1。
3. 发送 `AT\r\n`，PC 能收到 `OK\r\n`。
4. 发送 `AT+CWMODE?\r\n`，PC 能收到正确响应。
5. 从 PC 连续发送 1KB 数据，STM32 转发到 ESP32；同时 ESP32 返回 1KB 数据，STM32 转发到 PC；双向均无丢字节。

## 10. 风险与注意事项

- 透传启动后 USART1 不再输出日志，调试新问题时需要临时禁用 `APP_PASSTHROUGH_ENABLED` 恢复日志模式。
- `HAL_UART_Transmit` 在主循环中发送，如果缓冲区写满且对端不读取，可能导致短暂阻塞；ring buffer 大小应大于一次典型 AT 响应长度。
- USART1/2 RX 中断优先级应相同，避免一个方向长期占用中断导致另一方向丢数据。
