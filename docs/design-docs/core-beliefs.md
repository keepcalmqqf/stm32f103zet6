# 核心理念

本文档定义 `stm32f103zet6` 固件项目的核心信念。所有设计决策、代码评审与自动化流程都应遵循这些原则。

---

## 1. 生成代码与用户代码严格分离

- `Core/Src/main.c`、`gpio.c`、`fsmc.c` 等由 STM32CubeMX 生成。
- 用户逻辑必须写在 `/* USER CODE BEGIN/END ... */` 保护块内。
- 任何需要“在重新生成后仍然保留”的代码，都必须位于保护块或独立用户文件中。

## 2. 硬件细节应被封装，不应散落在业务代码中

- 应用代码通过 `led.h`、`ili9486.h` 等 BSP 接口访问硬件。
- GPIO 端口、引脚、极性等硬件细节由 CubeMX 生成的 `main.h` 或 BSP 层定义。
- 禁止在 `main()` 之外直接写 `HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, ...)`。

## 3. `AGENTS.md` 是地图，`docs/` 是记录系统

- `AGENTS.md` 只保留入口、高频命令和禁止事项。
- 详细架构、设计决策、流程规范必须下沉到 `docs/` 对应文件。
- 文档之间必须交叉链接，避免信息孤岛。

## 4. 构建是唯一的自动化验证手段

- 本仓库没有单元测试框架；干净构建即是最小质量门。
- 每次提交前必须执行 `cmake --build --preset Debug`。
- 发布前必须检查 `arm-none-eabi-size` 输出，确认 Flash / RAM 余量。

## 5. 优先可读性与可维护性，而非过度优化

- 使用清晰的命名、注释和函数拆分。
- 避免难以验证的汇编级微优化，除非有明确的性能需求与测量数据。
- 保持 C11 标准与 HAL 抽象，不直接操作寄存器，除非 HAL 无法满足时控要求。

## 6. 文档必须与代码同步演化

- 修改流程、命令、架构或接口时，必须同步更新对应文档。
- 过时的文档比没有文档更危险；发现过时应立即标记或修复。

---

## 决策记录

| 日期 | 决策 | 理由 | 相关文件 |
|------|------|------|----------|
| 2026-06-19 | 采用 STM32CubeMX + CMake 预设构建 | 统一构建环境，支持命令行与 IDE | `CMakeLists.txt`, `CMakePresets.json` |
| 2026-06-19 | FSMC 使用 SRAM 模式驱动 LCD | 避免 NOR CFI 命令干扰 8080 并口屏 | `Core/Src/fsmc.c`, `HARDWARE_PINOUT.md` |
| 2026-06-19 | printf 重定向到 USART1 | 无调试器时仍可排查问题 | `Core/Src/usart.c` |
| 2026-06-19 | 通过 ESP32-C3 AT 固件获取网络时间 | 板载无以太网，利用现有 WiFi/蓝牙模块同步 NTP | `Core/BSP/Src/esp_wifi.c`, `Core/App/Src/app_time_sync.c` |
| 2026-06-19 | RTC 驱动扩展日期支持 | NTP 返回完整日期，RTC 需要同时保存年月日 | `Core/BSP/Src/rtc.c`, `Core/BSP/Inc/rtc.h` |
| 2026-06-19 | 应用层拆分为 system / ui / time_sync / config | 降低 main.c 耦合，提高内聚与可维护性 | `Core/App/Src/app_*.c`, `Core/App/Inc/app_*.h`, `ARCHITECTURE.md` |
