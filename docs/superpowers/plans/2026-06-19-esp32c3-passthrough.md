# ESP32-C3 串口调试桥接实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在现有固件中新增 USART1↔USART2 字节级透明转发模块，启动完成 WiFi/NTP 后进入透传，方便 PC 串口助手直接调试 ESP32-C3 AT 固件。

**Architecture:** 新增 `app_passthrough` 模块负责两个方向的 ring buffer 与转发；USART1/2 RX 中断把字节写入对应 ring buffer；主循环调用 `App_Passthrough_Run()` 取出数据并通过阻塞发送转发。`app_system` 在 NTP 同步成功后启动透传，并禁用 USART1 日志输出。

**Tech Stack:** STM32 HAL, C11, arm-none-eabi-gcc, CMake

---

## 文件结构

| 文件 | 操作 | 职责 |
|------|------|------|
| `Core/App/Inc/app_passthrough.h` | 创建 | 模块接口与 ring buffer 大小配置 |
| `Core/App/Src/app_passthrough.c` | 创建 | ring buffer 实现、RX 中断入口、转发任务、PE4 使能 |
| `Core/App/Inc/app_config.h` | 修改 | 透传开关、PE4 引脚、日志控制 |
| `Core/Src/usart.c` | 修改 | 启动 USART1/2 RX 中断，导出句柄 |
| `Core/Inc/usart.h` | 修改 | 声明 `USART1_InitRxInterrupt()` / `USART2_InitRxInterrupt()` |
| `Core/Src/stm32f1xx_it.c` | 修改 | 转发 `USART1_IRQHandler` / `USART2_IRQHandler` 到 `app_passthrough` |
| `Core/App/Src/app_system.c` | 修改 | 初始化并在 NTP 成功后启动透传 |
| `Core/Src/syscalls.c` 或 `usart.c` | 修改 | `__io_putchar` 在透传模式下静默 |
| `CMakeLists.txt` | 修改 | 添加 `Core/App/Src/app_passthrough.c` 到构建 |

---

### Task 1: 创建 `app_passthrough.h`

**Files:**
- Create: `Core/App/Inc/app_passthrough.h`

- [ ] **Step 1: 编写头文件**

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_passthrough.h
  * @brief   USART1 <-> USART2 debug passthrough bridge.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_PASSTHROUGH_H
#define APP_PASSTHROUGH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化透传模块的 ring buffer 与 PE4 使能脚。
 * @note   应在 USART1/2 初始化之后调用。
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

/**
 * @brief 查询当前是否处于透传模式。
 */
bool App_Passthrough_IsActive(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PASSTHROUGH_H */
```

- [ ] **Step 2: Commit**

```bash
git add Core/App/Inc/app_passthrough.h
git commit -m "feat(passthrough): add app_passthrough.h interface"
```

---

### Task 2: 创建 `app_passthrough.c`

**Files:**
- Create: `Core/App/Src/app_passthrough.c`

- [ ] **Step 1: 编写实现**

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_passthrough.c
  * @brief   USART1 <-> USART2 byte-level transparent bridge.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_passthrough.h"
#include "app_config.h"
#include "usart.h"
#include "stm32f1xx_hal.h"

#ifndef APP_PASSTHROUGH_BUF_SIZE
#define APP_PASSTHROUGH_BUF_SIZE 256U
#endif

typedef struct
{
    uint8_t  buf[APP_PASSTHROUGH_BUF_SIZE];
    uint16_t head;
    uint16_t tail;
} PassthroughRingBuf_t;

static volatile PassthroughRingBuf_t s_u1_to_u2;
static volatile PassthroughRingBuf_t s_u2_to_u1;
static volatile bool s_active = false;

static bool ringbuf_push(volatile PassthroughRingBuf_t *rb, uint8_t byte)
{
    const uint16_t next_head = (uint16_t)((rb->head + 1U) % APP_PASSTHROUGH_BUF_SIZE);
    if (next_head == rb->tail)
    {
        return false; /* full */
    }
    rb->buf[rb->head] = byte;
    rb->head = next_head;
    return true;
}

static bool ringbuf_pop(volatile PassthroughRingBuf_t *rb, uint8_t *byte)
{
    if (rb->head == rb->tail)
    {
        return false; /* empty */
    }
    *byte = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1U) % APP_PASSTHROUGH_BUF_SIZE);
    return true;
}

static void esp_enable_pin_init(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = APP_ESP_EN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(APP_ESP_EN_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(APP_ESP_EN_GPIO_PORT, APP_ESP_EN_GPIO_PIN, GPIO_PIN_SET);
}

void App_Passthrough_Init(void)
{
    memset((void *)&s_u1_to_u2, 0, sizeof(s_u1_to_u2));
    memset((void *)&s_u2_to_u1, 0, sizeof(s_u2_to_u1));
    s_active = false;

    esp_enable_pin_init();
}

void App_Passthrough_Start(void)
{
    s_active = true;
}

bool App_Passthrough_IsActive(void)
{
    return s_active;
}

void App_Passthrough_OnUsart1Rx(uint8_t byte)
{
    if (s_active)
    {
        (void)ringbuf_push(&s_u1_to_u2, byte);
    }
}

void App_Passthrough_OnUsart2Rx(uint8_t byte)
{
    if (s_active)
    {
        (void)ringbuf_push(&s_u2_to_u1, byte);
    }
}

void App_Passthrough_Run(void)
{
    if (!s_active)
    {
        return;
    }

    uint8_t byte = 0U;

    /* PC -> ESP32-C3 */
    while (ringbuf_pop(&s_u1_to_u2, &byte))
    {
        (void)HAL_UART_Transmit(&huart2, &byte, 1U, HAL_MAX_DELAY);
    }

    /* ESP32-C3 -> PC */
    while (ringbuf_pop(&s_u2_to_u1, &byte))
    {
        (void)HAL_UART_Transmit(&huart1, &byte, 1U, HAL_MAX_DELAY);
    }
}
```

- [ ] **Step 2: Build 验证**

```bash
cmake --build --preset Debug
```

Expected: 构建成功，无新增 warning。

- [ ] **Step 3: Commit**

```bash
git add Core/App/Src/app_passthrough.c
git commit -m "feat(passthrough): add ring buffer, PE4 enable, and forward task"
```

---

### Task 3: 更新 `app_config.h`

**Files:**
- Modify: `Core/App/Inc/app_config.h`

- [ ] **Step 1: 在文件末尾、#endif 之前插入配置**

```c
/* ESP32-C3 passthrough configuration. */
#define APP_PASSTHROUGH_ENABLED      1
#define APP_PASSTHROUGH_BUF_SIZE     256U

/* ESP32-C3 enable pin (active high). */
#define APP_ESP_EN_GPIO_PORT         GPIOE
#define APP_ESP_EN_GPIO_PIN          GPIO_PIN_4
```

- [ ] **Step 2: Commit**

```bash
git add Core/App/Inc/app_config.h
git commit -m "config(passthrough): add passthrough and ESP enable pin settings"
```

---

### Task 4: 在 `usart.c` / `usart.h` 中启用 RX 中断

**Files:**
- Modify: `Core/Inc/usart.h`
- Modify: `Core/Src/usart.c`

- [ ] **Step 1: 在 `usart.h` 中声明中断启动函数**

在 `usart.h` 中 `MX_USART2_UART_Init` 声明之后添加：

```c
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

void USART1_Init(void);
void USART1_InitRxInterrupt(void);
void USART2_InitRxInterrupt(void);
```

- [ ] **Step 2: 在 `usart.c` 中实现中断启动函数**

在 `__io_putchar` 之前添加：

```c
void USART1_InitRxInterrupt(void)
{
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    HAL_NVIC_SetPriority(USART1_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void USART2_InitRxInterrupt(void)
{
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    HAL_NVIC_SetPriority(USART2_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}
```

- [ ] **Step 3: Commit**

```bash
git add Core/Inc/usart.h Core/Src/usart.c
git commit -m "feat(usart): enable USART1/2 RXNE interrupts for passthrough"
```

---

### Task 5: 在 `stm32f1xx_it.c` 中转发 USART 中断

**Files:**
- Modify: `Core/Src/stm32f1xx_it.c`

- [ ] **Step 1: 包含头文件**

在文件顶部包含：

```c
#include "app_passthrough.h"
```

- [ ] **Step 2: 修改 `USART1_IRQHandler`**

找到：

```c
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}
```

改为：

```c
void USART1_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
  {
    const uint8_t byte = (uint8_t)(huart1.Instance->DR & 0xFFU);
    App_Passthrough_OnUsart1Rx(byte);
  }

  HAL_UART_IRQHandler(&huart1);
}
```

- [ ] **Step 3: 修改 `USART2_IRQHandler`**

找到：

```c
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}
```

改为：

```c
void USART2_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
  {
    const uint8_t byte = (uint8_t)(huart2.Instance->DR & 0xFFU);
    App_Passthrough_OnUsart2Rx(byte);
  }

  HAL_UART_IRQHandler(&huart2);
}
```

- [ ] **Step 4: Commit**

```bash
git add Core/Src/stm32f1xx_it.c
git commit -m "feat(it): forward USART1/2 RX bytes to passthrough handler"
```

---

### Task 6: 在 `app_system.c` 中集成透传

**Files:**
- Modify: `Core/App/Src/app_system.c`

- [ ] **Step 1: 包含头文件**

在 `#include "usart.h"` 之后添加：

```c
#include "app_passthrough.h"
```

- [ ] **Step 2: 在 `App_System_InitBoard` 中初始化 RX 中断和透传模块**

在 `USART1_Init();` 之后添加：

```c
    USART1_InitRxInterrupt();
    USART2_InitRxInterrupt();
    App_Passthrough_Init();
```

- [ ] **Step 3: 在 NTP 同步成功后启动透传**

在 `App_System_InitRtcAndSync` 返回 true 后，调用：

```c
    if (ntp_synced)
    {
        App_Passthrough_Start();
        APP_LOG_INFO("Passthrough mode active");
    }
```

注意：这是进入透传前的最后一条日志。

- [ ] **Step 4: 在主循环中调用转发任务**

在 `App_System_Run` 中，在 `App_UI_Handler()` 之后添加：

```c
        App_Passthrough_Run();
```

- [ ] **Step 5: Commit**

```bash
git add Core/App/Src/app_system.c
git commit -m "feat(system): integrate passthrough init, start, and run loop"
```

---

### Task 7: 透传时禁用 `printf` 输出

**Files:**
- Modify: `Core/Src/usart.c`

- [ ] **Step 1: 修改 `__io_putchar`**

把：

```c
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1U, HAL_MAX_DELAY);
    return ch;
}
```

改为：

```c
int __io_putchar(int ch)
{
    if (App_Passthrough_IsActive())
    {
        return ch;
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1U, HAL_MAX_DELAY);
    return ch;
}
```

- [ ] **Step 2: 在 `usart.c` 顶部包含头文件**

在 `#include "usart.h"` 之后添加：

```c
#include "app_passthrough.h"
```

- [ ] **Step 3: Commit**

```bash
git add Core/Src/usart.c
git commit -m "feat(usart): suppress printf output to USART1 while passthrough active"
```

---

### Task 8: 更新 `CMakeLists.txt`

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 在 `target_sources` 中添加新源文件**

在 `Core/App/Src/app_system.c` 之后添加：

```c
    Core/App/Src/app_passthrough.c
```

- [ ] **Step 2: Commit**

```bash
git add CMakeLists.txt
git commit -m "build(cmake): add app_passthrough.c to target sources"
```

---

### Task 9: 构建与体积验证

**Files:**
- Verify: `build/Debug/stm32f103zet6.elf`

- [ ] **Step 1: 清理并构建**

```bash
cmake --preset Debug
cmake --build --preset Debug
```

Expected: 构建成功，无错误、无新增 warning。

- [ ] **Step 2: 检查固件体积**

```bash
arm-none-eabi-size build/Debug/stm32f103zet6.elf
```

Expected: 与修改前相比，Flash/RAM 增加在几百字节以内。

- [ ] **Step 3: Commit（如体积正常）**

```bash
git commit --allow-empty -m "build: verify passthrough feature builds and size is acceptable"
```

---

## 自我审查

### Spec 覆盖检查

| Spec 要求 | 对应任务 |
|-----------|----------|
| 启动先完成 WiFi/NTP，再进入透传 | Task 6 |
| USART1↔USART2 字节级透明转发 | Task 2, 4, 5 |
| 中断 + 环形缓冲区 | Task 2, 4 |
| 进入透传后关闭 printf | Task 7 |
| PE4 使能脚控制 | Task 2 |
| 配置集中管理 | Task 3 |
| 构建验证 | Task 9 |

### Placeholder 检查

- 无 TBD/TODO。
- 每个修改步骤均给出具体代码与命令。
- 函数签名在头文件与实现中一致。

### 类型一致性检查

- `App_Passthrough_IsActive()` 返回 `bool`，在 `usart.c` 中作为条件使用，一致。
- `ringbuf_push/pop` 使用 `volatile PassthroughRingBuf_t *`，与静态变量类型一致。
- USART 中断处理函数使用 `huart1.Instance->DR`，符合 STM32F1 寄存器访问方式。

---

## 执行交接

**Plan complete and saved to `docs/superpowers/plans/2026-06-19-esp32c3-passthrough.md`. Two execution options:**

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
